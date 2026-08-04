#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
sql.py — SQL emitindo pelo CHESSC, com a memória no DISCO.

Reescrita do tools/sql.c. O que muda não é o front-end — o analisador, a árvore do WHERE, a
contração tensorial e a canonização continuam os mesmos, e são a parte que vale. O que muda é
a SAÍDA: em vez do meu montador escrevendo bytes com pwrite, emite-se o pseudo-assembly
universal e entrega-se ao chessc (chess/sandbox/chessc.py).

Três coisas que isso corrige, e nenhuma é estética:

  1. A MULTIPLICAÇÃO deixa de ser minha. Eu emitia soma repetida — linear no VALOR. O corpo ℚ
     do projeto usa double-and-add — linear nos BITS —, e o kernel.py nomeia o meu erro:
     "eu tinha escrito ⌊num/den⌋ (double-and-add) em C: o bispo a jogar como torre". Para a·90
     são 7 voltas em vez de 90. A peça está atestada em
     chess/sandbox/tecnicas/regua_racional_chessc.py (interp ≡ WASM ≡ Dafny, resíduo 0).

  2. Os RÓTULOS são simbólicos. Todo o cuidado que tomei com saltos de ±127 bytes era limitação
     do meu montador, não da linguagem: o chessc aceita `JZ FIM`, `JMP TOPO`.

  3. E o WHERE passa a ser PROVÁVEL, não só executável: chessc.emit_dafny devolve as leis, e
     chessc.emit_wasm devolve o metal. A contração deixa de ser "eu conferi".

SEM RAM: o emit_c do chessc gera `u64 mem[64]` — um vetor em memória. Aqui ele é pós-processado
para que cada acesso vire pread/pwrite no arquivo .mem. Some o teto de 64 slots junto: o arquivo
é a memória, e a tabela nunca é carregada.

    python3 sql.py <base> "CREATE TABLE t (a,b,c)"
    python3 sql.py <base> "INSERT INTO t VALUES (7,10,20)"
    python3 sql.py <base> "SELECT * FROM t WHERE (a+b)*(a-b) > 0"
    python3 sql.py <base> --dafny "SELECT * FROM t WHERE a = 3"    # a prova, pelo chessc
    python3 sql.py teste
"""
from __future__ import annotations

import os
import struct
import subprocess
import sys
import tempfile

_CHESSC_DIR = "/home/aaraolopes/Documentos/chess/sandbox"
sys.path.insert(0, _CHESSC_DIR)
import chessc  # noqa: E402

# ── o mapa da memória (slots de 8 bytes; o disco é a memória) ────────────────
S_CAT, S_ZERO, S_UM, S_TMP, S_CONTA = 0, 1, 2, 3, 4
S_ACC, S_PROD, S_MASK = 5, 6, 7
S_V = 9             # o valor do SET
S_NROWS = 8         # o contador de linhas — NÃO no slot 1, que é o zero das constantes
S_MUL = 10          # 10..15  o rascunho do double-and-add
S_K = 20            # 20..35  as constantes dos átomos
S_COND = 40         # 40..55  o resultado de cada átomo
S_EXPR = 60         # 60..75  os temporários da árvore
S_MATCH = 256       # 256..511  o bitmap do resultado
S_VIVO = 512        # 512..1023
S_LINHAS = 1024
NCOL, KGRAU = 6, 3
NI = NCOL + 1
SLOT = 8

# ── o tensor de multi-índice (idêntico ao do sql.c: a posição ordenada é o endereço) ──


def mi_cod(d):
    d = sorted(d)
    r = 0
    for t in reversed(d):
        r = r * NI + t
    return r


def mi_de(cod):
    d = []
    for _ in range(KGRAU):
        d.append(cod % NI)
        cod //= NI
    return d


def mi_grau(cod):
    return sum(1 for x in mi_de(cod) if x)


class Tensor:
    __slots__ = ("c",)

    def __init__(self):
        self.c = {}

    def mon(self, d, k):
        if k == 0:
            return
        cod = mi_cod(list(d) + [0] * (KGRAU - len(d)))
        self.c[cod] = self.c.get(cod, 0) + k
        if self.c[cod] == 0:
            del self.c[cod]

    def const(self, k):
        self.mon([], k)

    def var(self, col):
        self.mon([col + 1], 1)

    def soma(self, o, s=1):
        r = Tensor()
        r.c = dict(self.c)
        for cod, v in o.c.items():
            r.c[cod] = r.c.get(cod, 0) + s * v
            if r.c[cod] == 0:
                del r.c[cod]
        return r

    def mul(self, o):
        r = Tensor()
        for ca, va in self.c.items():
            da = [x for x in mi_de(ca) if x]
            for cb, vb in o.c.items():
                db = [x for x in mi_de(cb) if x]
                if len(da) + len(db) > KGRAU:
                    return None                      # passa do grau: recusa, não trunca
                r.mon(da + db, va * vb)
        return r

    def constante(self):
        return all(mi_grau(c) == 0 for c in self.c)

    def chave(self):
        return tuple(sorted(self.c.items()))


# ── o analisador ────────────────────────────────────────────────────────────
class Fim(Exception):
    pass


class Lex:
    def __init__(self, s):
        self.s, self.i = s, 0

    def pula(self):
        while self.i < len(self.s) and self.s[self.i].isspace():
            self.i += 1

    def olha(self):
        self.pula()
        return self.s[self.i] if self.i < len(self.s) else ""

    def come(self, c):
        self.pula()
        if self.s.startswith(c, self.i):
            self.i += len(c)
            return True
        return False

    def palavra(self, w):
        self.pula()
        n = len(w)
        if self.s[self.i:self.i + n].upper() == w and (
                self.i + n >= len(self.s) or not self.s[self.i + n].isalnum()):
            self.i += n
            return True
        return False

    def ident(self):
        self.pula()
        j = self.i
        if self.olha() == "*":
            self.i += 1
            return "*"
        while self.i < len(self.s) and (self.s[self.i].isalnum() or self.s[self.i] == "_"):
            self.i += 1
        return self.s[j:self.i]

    def numero(self):
        self.pula()
        j = self.i
        if self.olha() == "-":
            self.i += 1
        if self.i >= len(self.s) or not self.s[self.i].isdigit():
            self.i = j
            return None
        while self.i < len(self.s) and self.s[self.i].isdigit():
            self.i += 1
        return int(self.s[j:self.i])


def num_fator(lx):
    if lx.come("("):
        t = num_soma(lx)
        if not lx.come(")"):
            raise Fim()
        return t
    n = lx.numero()
    t = Tensor()
    if n is not None:
        t.const(n)
        return t
    nome = lx.ident()
    if not nome or not nome[0].isalpha():
        raise Fim()
    col = ord(nome[0]) - ord("a")
    if not 0 <= col < NCOL:
        raise Fim()
    t.var(col)
    return t


def num_produto(lx):
    t = num_fator(lx)
    while lx.come("*"):
        u = num_fator(lx)
        r = t.mul(u)
        if r is None:
            raise Fim()
        t = r
    return t


def num_soma(lx):
    neg = lx.come("-")
    if not neg:
        lx.come("+")
    t = num_produto(lx)
    if neg:
        t = Tensor().soma(t, -1)
    while True:
        if lx.come("+"):
            t = t.soma(num_produto(lx), 1)
        elif lx.come("-"):
            t = t.soma(num_produto(lx), -1)
        else:
            return t


class No:
    def __init__(self, tipo, esq=None, dir=None, v=None, op=None, nega=0):
        self.tipo, self.esq, self.dir = tipo, esq, dir
        self.v, self.op, self.nega = v, op, nega
        self.decidido, self.valor, self.atomo = False, 0, -1


def bool_fator(lx):
    salvo = lx.i
    try:
        L = num_soma(lx)
        lx.pula()
        op, nega = None, 0
        if lx.come("!="):
            op, nega = "=", 1
        elif lx.come("<="):
            op, nega = ">", 1
        elif lx.come(">="):
            op, nega = "<", 1
        elif lx.come("="):
            op = "="
        elif lx.come("<"):
            op = "<"
        elif lx.come(">"):
            op = ">"
        if op is None:
            raise Fim()
        R = num_soma(lx)
    except Fim:
        lx.i = salvo                      # o '(' era grupo booleano, não fator numérico
        if lx.come("("):
            e = bool_expr(lx)
            if not lx.come(")"):
                raise Fim()
            return e
        raise
    v = L.soma(R, -1)                     # L op R  ⟺  (L−R) op 0
    if op == "<":                         # canoniza: v<0 ⟺ (−v)>0, logo o '<' some
        v, op = Tensor().soma(v, -1), ">"
    elif op == "=":                       # v=0 ⟺ (−v)=0: fixa o sinal
        prim = next((v.c[c] for c in sorted(v.c) if mi_grau(c)), 0) or v.c.get(0, 0)
        if prim < 0:
            v = Tensor().soma(v, -1)
    n = No("cond", v=v, op=op, nega=nega)
    if v.constante():
        d = v.c.get(0, 0)
        vale = (d == 0) if op == "=" else (d < 0) if op == "<" else (d > 0)
        n.decidido, n.valor = True, (not vale) if nega else vale
    return n


def bool_termo(lx):
    e = bool_fator(lx)
    while lx.palavra("AND"):
        e = No("and", e, bool_fator(lx))
    return e


def bool_expr(lx):
    e = bool_termo(lx)
    while lx.palavra("OR"):
        e = No("or", e, bool_termo(lx))
    return e


# ── a contração: normaliza a árvore e junta os átomos ────────────────────────
def sig(n):
    if n.tipo == "cond":
        return ("c", n.op, n.nega, n.decidido, n.valor, n.v.chave())
    s1, s2 = sig(n.esq), sig(n.dir)
    if repr(s1) > repr(s2):
        n.esq, n.dir = n.dir, n.esq
        s1, s2 = s2, s1
    return (n.tipo, s1, s2)


def normaliza(n):
    if n.tipo == "cond":
        return n
    n.esq, n.dir = normaliza(n.esq), normaliza(n.dir)
    s1, s2 = sig(n.esq), sig(n.dir)
    if s1 == s2:
        return n.esq                       # A op A = A
    if repr(s1) > repr(s2):
        n.esq, n.dir = n.dir, n.esq
    return n


def junta(n, atomos):
    if n.tipo == "cond":
        k = sig(n)
        for j, a in enumerate(atomos):
            if a[0] == k:
                n.atomo = j
                return
        atomos.append((k, n))
        n.atomo = len(atomos) - 1
        return
    junta(n.esq, atomos)
    junta(n.dir, atomos)


# ── o disco: os slots vivem no arquivo, e nada é carregado ───────────────────
class Base:
    def __init__(self, nome, criar=False):
        self.caminho = nome + ".mem"
        if criar or not os.path.exists(self.caminho):
            open(self.caminho, "ab").close()
        self.f = open(self.caminho, "r+b")

    def le(self, slot):
        self.f.seek(slot * SLOT)
        b = self.f.read(SLOT)
        return struct.unpack("<q", b)[0] if len(b) == SLOT else 0

    def grava(self, slot, v):
        self.f.seek(0, 2)
        if self.f.tell() < (slot + 1) * SLOT:
            self.f.write(b"\0" * ((slot + 1) * SLOT - self.f.tell()))
        self.f.seek(slot * SLOT)
        self.f.write(struct.pack("<q", v & 0xFFFFFFFFFFFFFFFF))

    def fecha(self):
        self.f.flush()
        os.fsync(self.f.fileno())
        self.f.close()


# ── o montador: pseudo-assembly do chessc, com rótulos simbólicos ────────────
class Asm:
    def __init__(self):
        self.dados, self.linhas, self.n = {}, [], 0

    def data(self, slot, v):
        self.dados[slot] = v

    def i(self, texto, rot=None):
        self.linhas.append(("%s:" % rot if rot else "        ") + " " + texto)

    def rotulo(self, pref):
        self.n += 1
        return "%s%d" % (pref, self.n)

    def copia(self, de, para):
        """põe o conteúdo de um slot noutro: STORE grava R, então soma-se com zero."""
        self.i("LOAD %d" % de)
        self.i("LOAD %d" % S_ZERO)
        self.i("ADD")
        self.i("STORE %d" % para)

    def texto(self):
        d = "\n".join(".data %d %d" % (s, v) for s, v in sorted(self.dados.items()))
        return d + "\n" + "\n".join(self.linhas) + "\n        HALT\n"

    # ⊗ double-and-add: linear nos BITS, não no valor — a peça do corpo ℚ
    #   (chess/sandbox/tecnicas/regua_racional_chessc.py, interp ≡ WASM ≡ Dafny, resíduo 0).
    #   A versão anterior desta função emitia a cópia final DUAS vezes, uma delas inalcançável
    #   depois do JMP, e ainda deixava uma linha morta. Funcionava por acidente. Agora o laço
    #   tem uma saída só, no rótulo FIM, e a cópia acontece uma vez.
    def mult(self, dest, X, Y):
        acc, mul, masc, cont, bit = S_MUL, S_MUL + 1, S_MUL + 2, S_MUL + 3, S_MUL + 4
        topo, salta, fim = self.rotulo("M"), self.rotulo("MS"), self.rotulo("MF")
        self.data(S_MUL + 5, 1)
        self.data(S_MUL + 6, 64)
        self.copia(S_ZERO, acc)
        self.copia(X, mul)
        self.copia(Y, S_TMP)
        self.copia(S_MUL + 5, masc)
        self.copia(S_MUL + 6, cont)

        self.i("LOAD %d" % S_TMP, topo)        # o bit corrente do multiplicador
        self.i("LOAD %d" % masc)
        self.i("AND")
        self.i("STORE %d" % bit)
        self.i("LOAD %d" % bit)
        self.i("LOAD %d" % S_ZERO)
        self.i("CMP")                          # FL_ZERO sse o bit está apagado
        self.i("JZ %s" % salta)
        self.i("LOAD %d" % acc)                # aceso: acumula
        self.i("LOAD %d" % mul)
        self.i("ADD")
        self.i("STORE %d" % acc)

        self.i("LOAD %d" % masc, salta)        # dobra a máscara
        self.i("LOAD %d" % masc)
        self.i("ADD")
        self.i("STORE %d" % masc)
        self.i("LOAD %d" % mul)                # e dobra o multiplicando
        self.i("LOAD %d" % mul)
        self.i("ADD")
        self.i("STORE %d" % mul)
        self.i("LOAD %d" % S_UM)               # cont −= 1  (A=cont, B=um ⟹ R = cont−1)
        self.i("LOAD %d" % cont)
        self.i("SUB")
        self.i("STORE %d" % cont)
        self.i("LOAD %d" % cont)
        self.i("LOAD %d" % S_ZERO)
        self.i("CMP")
        self.i("JZ %s" % fim)
        self.i("JMP %s" % topo)

        self.i("LOAD %d" % S_ZERO, fim)        # a saída ÚNICA do laço
        self.copia(acc, dest)


def emit_atomo(a, atomo, j, linha, ncols):
    """avalia um átomo (um tensor comparado com 0) na linha dada."""
    dest = S_COND + j
    _, n = atomo
    if n.v.constante():
        return
    a.data(S_K + j, n.v.c.get(0, 0))
    a.copia(S_K + j, S_ACC)
    for cod in sorted(n.v.c):
        c = n.v.c[cod]
        g = mi_grau(cod)
        if g == 0:
            continue
        cols = [x - 1 for x in mi_de(cod) if x]
        if any(x >= ncols for x in cols):
            continue
        if g == 1:
            termo = S_LINHAS + linha * ncols + cols[0]
        else:
            a.copia(S_LINHAS + linha * ncols + cols[0], S_PROD)
            for cc in cols[1:]:
                a.mult(S_PROD, S_PROD, S_LINHAS + linha * ncols + cc)
            termo = S_PROD
        for _ in range(abs(c)):
            if c > 0:
                a.i("LOAD %d" % S_ACC)
                a.i("LOAD %d" % termo)
                a.i("ADD")
            else:
                a.i("LOAD %d" % termo)
                a.i("LOAD %d" % S_ACC)
                a.i("SUB")
            a.i("STORE %d" % S_ACC)
    # a comparação é sempre com ZERO — a contração já passou tudo para um lado
    fim, casou = a.rotulo("C"), a.rotulo("CS")
    a.copia(S_ZERO, dest)
    if n.op == "=":
        a.i("LOAD %d" % S_ACC)
        a.i("LOAD %d" % S_ZERO)
        a.i("CMP")
        a.i("JZ %s" % casou)
        a.i("JMP %s" % fim)
    else:                                   # '>' : o bit de SINAL de (0 − acc)
        # a máscara isola o bit 63. Eu tinha escrito -1 aqui, que são TODOS os bits ligados:
        # o AND devolvia o próprio valor e o teste dava sempre verdadeiro. Erro de
        # transcrição do sql.c, onde a máscara era 1<<63.
        a.data(S_MASK, 1 << 63)
        a.i("LOAD %d" % S_ACC)
        a.i("LOAD %d" % S_ZERO)
        a.i("SUB")                          # R = 0 − acc  → negativo se acc > 0
        a.i("STORE %d" % S_TMP)
        a.i("LOAD %d" % S_TMP)
        a.i("LOAD %d" % S_MASK)             # a máscara: só o bit de sinal
        a.i("AND")
        a.i("STORE %d" % S_TMP)
        a.i("LOAD %d" % S_TMP)
        a.i("LOAD %d" % S_ZERO)
        a.i("CMP")
        a.i("JZ %s" % fim)                  # zero ⟹ acc ≤ 0
        a.i("JMP %s" % casou)
    a.i("LOAD %d" % S_ZERO, casou)
    a.copia(S_UM, dest)
    a.i("LOAD %d" % S_ZERO, fim)
    if n.nega:                              # !=, <=, >= : XOR com 1
        a.i("LOAD %d" % dest)
        a.i("LOAD %d" % S_UM)
        a.i("XOR")
        a.i("STORE %d" % dest)


def emit_arvore(a, n, dest):
    if n.tipo == "cond":
        a.copia(S_UM if n.valor else S_ZERO, dest) if n.decidido else \
            a.copia(S_COND + n.atomo, dest)
        return
    de, dd = dest + 1, dest + 2
    emit_arvore(a, n.esq, de)
    emit_arvore(a, n.dir, dd)
    a.i("LOAD %d" % de)
    a.i("LOAD %d" % dd)
    a.i("AND" if n.tipo == "and" else "OR")
    a.i("STORE %d" % dest)


# ── o disco COMO memória: pós-processa o C do chessc ─────────────────────────
_RUNTIME = r'''
/* --- a memória é o DISCO: cada acesso é pread/pwrite, e nada é carregado. --- */
#include <unistd.h>
#include <fcntl.h>
static int FMEM = -1;
static u64 ml(long s){ u64 v=0; if(pread(FMEM,&v,8,s*8)!=8) v=0; return v; }
static void ms(long s, u64 v){ pwrite(FMEM,&v,8,s*8); }
'''


def compila_e_roda(asm_txt, caminho_mem):
    """emite C pelo chessc e troca mem[] por pread/pwrite — o disco continua sendo a memória."""
    c = chessc.emit_c(asm_txt)
    c = c.replace("    u64 A=0,B=0,R=0; int Z=0; u64 mem[64]={0};",
                  "    u64 A=0,B=0,R=0; int Z=0;\n"
                  '    FMEM = open("%s", O_RDWR|O_CREAT, 0644); if(FMEM<0) return 2;' % caminho_mem)
    c = c.replace("typedef uint64_t u64;", "typedef uint64_t u64;" + _RUNTIME, 1)
    saida, pulando = [], False
    for ln in c.split("\n"):
        if "{ int etotal" in ln:            # o relatório de cristalização do chessc: fora
            pulando = True
        if pulando:
            if "FALHOU" in ln:
                pulando = False
            continue
        s = ln.strip()
        if s.startswith("mem[") and "]=" in s and "ULL;" in s:      # os .data
            slot = s[4:s.index("]")]
            val = s[s.index("=") + 1:s.rindex(";")]
            saida.append("    ms(%s, %s);" % (slot, val))
        elif "A=mem[" in ln:
            saida.append(ln.replace("A=mem[", "A=ml(").replace("];", ");"))
        elif "mem[" in ln and "]=R;" in ln:
            slot = ln[ln.index("mem[") + 4:ln.index("]=R;")]
            saida.append("    ms(%s, R);" % slot)
        elif "mem[7]" in ln or "mem[8]" in ln:
            saida.append(ln.replace("mem[7]", "ml(7)").replace("mem[8]", "ml(8)"))
        else:
            saida.append(ln)
    c = "\n".join(saida)

    with tempfile.TemporaryDirectory() as td:
        fc, fx = os.path.join(td, "p.c"), os.path.join(td, "p")
        open(fc, "w").write(c)
        r = subprocess.run(["cc", "-O2", "-w", "-o", fx, fc], capture_output=True, text=True)
        if r.returncode:
            print(r.stderr[:1200])
            raise SystemExit("o C gerado nao compilou")
        subprocess.run([fx], check=False)
    return len(asm_txt.split("\n"))


# ── os comandos ─────────────────────────────────────────────────────────────
def cria(b, lx):
    lx.ident()
    if not lx.come("("):
        return False
    n = 0
    while True:
        if not lx.ident():
            break
        n += 1
        if not lx.come(","):
            break
    b.grava(S_CAT, n)
    b.grava(S_NROWS, 0)
    print("tabela criada: %d colunas" % n)
    return True


def insere(b, lx):
    lx.palavra("INTO")
    lx.ident()
    lx.palavra("VALUES")
    lx.come("(")
    ncols, nrows = b.le(S_CAT), b.le(S_NROWS)
    vs = []
    while True:
        v = lx.numero()
        if v is None:
            break
        vs.append(v)
        if not lx.come(","):
            break
    if len(vs) != ncols:
        print("erro: a tabela tem %d colunas, vieram %d" % (ncols, len(vs)))
        return False
    a = Asm()
    a.data(S_ZERO, 0)
    for j, v in enumerate(vs):
        a.data(S_K + j, v)
        a.copia(S_K + j, S_LINHAS + nrows * ncols + j)
    a.data(S_UM, 1)
    a.copia(S_UM, S_VIVO + nrows)
    b.fecha()
    linhas = compila_e_roda(a.texto(), b.caminho)
    b.__init__(b.caminho.rsplit(".mem", 1)[0])
    b.grava(S_NROWS, nrows + 1)
    print("1 linha inserida — %d linhas de ISA; agora %d linhas" % (linhas, nrows + 1))
    return True


# as três ações que uma varredura pode ter na linha que casa
MARCA, SET, APAGA = "marca", "set", "apaga"


def varre(b, lx, acao, dafny=False):
    col_set, valor = 0, 0
    if acao == MARCA:
        lx.ident()
        if not lx.palavra("FROM"):
            return False
        lx.ident()
    elif acao == SET:
        lx.ident()                                  # a tabela
        if not lx.palavra("SET"):
            return False
        alvo = lx.ident()
        if not lx.come("="):
            return False
        valor = lx.numero()
        if valor is None:
            return False
        col_set = ord(alvo[0]) - ord("a")
    else:
        if not lx.palavra("FROM"):
            return False
        lx.ident()
    raiz, atomos = None, []
    if lx.palavra("WHERE"):
        raiz = normaliza(bool_expr(lx))
        junta(raiz, atomos)
    ncols, nrows = b.le(S_CAT), b.le(S_NROWS)
    if nrows <= 0:
        print("(vazio)")
        return True
    for i in range(nrows):
        b.grava(S_MATCH + i, 0)
    b.grava(S_CONTA, 0)
    a = Asm()
    a.data(S_ZERO, 0)
    a.data(S_UM, 1)
    if acao == SET:
        a.data(S_V, valor)
    for i in range(nrows):
        if raiz is None:
            a.copia(S_UM, S_EXPR)
        else:
            for j, at in enumerate(atomos):
                emit_atomo(a, at, j, i, ncols)
            emit_arvore(a, raiz, S_EXPR)
        a.i("LOAD %d" % S_EXPR)               # e a linha tem de estar viva
        a.i("LOAD %d" % (S_VIVO + i))
        a.i("AND")
        a.i("STORE %d" % S_EXPR)
        pula = a.rotulo("L")
        a.i("LOAD %d" % S_EXPR)
        a.i("LOAD %d" % S_ZERO)
        a.i("CMP")
        a.i("JZ %s" % pula)
        if acao == MARCA:
            a.copia(S_UM, S_MATCH + i)              # marca a linha no bitmap
        elif acao == SET:
            a.copia(S_V, S_LINHAS + i * ncols + col_set)   # troca a coluna
        else:
            a.copia(S_ZERO, S_VIVO + i)             # a linha deixa de existir
        a.i("LOAD %d" % S_CONTA)
        a.i("LOAD %d" % S_UM)
        a.i("ADD")
        a.i("STORE %d" % S_CONTA)
        a.i("LOAD %d" % S_ZERO, pula)
    txt = a.texto()
    if dafny:
        print(chessc.emit_dafny(txt)[:1500])
        return True
    b.fecha()
    linhas = compila_e_roda(txt, b.caminho)
    b.__init__(b.caminho.rsplit(".mem", 1)[0])
    achou = b.le(S_CONTA)
    nome = {MARCA: "lida(s)", SET: "atualizada(s)", APAGA: "apagada(s)"}[acao]
    print("-- %d linhas de ISA, %d átomo(s), %d linha(s) %s" % (linhas, len(atomos), achou, nome))
    if acao != MARCA:
        return True
    for i in range(nrows):
        if b.le(S_MATCH + i) == 0:
            continue
        print("   " + " | ".join(str(b.le(S_LINHAS + i * ncols + j)) for j in range(ncols)))
    return True


def executa(b, sql, dafny=False):
    lx = Lex(sql)
    try:
        if lx.palavra("CREATE"):
            lx.palavra("TABLE")
            return cria(b, lx)
        if lx.palavra("INSERT"):
            return insere(b, lx)
        if lx.palavra("SELECT"):
            return varre(b, lx, MARCA, dafny)
        if lx.palavra("UPDATE"):
            return varre(b, lx, SET, dafny)
        if lx.palavra("DELETE"):
            return varre(b, lx, APAGA, dafny)
    except Fim:
        print("nao entendi: %s" % sql)
        return False
    print("nao entendi: %s" % sql)
    return False


def teste():
    base = "/tmp/sqlpy_teste"
    for ext in (".mem",):
        if os.path.exists(base + ext):
            os.unlink(base + ext)
    b = Base(base, criar=True)
    print("\n=== SQL PELO CHESSC, memória no disco ===================================\n")
    for q in ["CREATE TABLE t (a,b,c)",
              "INSERT INTO t VALUES (7,10,20)",
              "INSERT INTO t VALUES (3,30,40)",
              "INSERT INTO t VALUES (7,50,60)",
              "INSERT INTO t VALUES (9,70,80)"]:
        print("$ " + q)
        executa(b, q)
    for q in ["SELECT * FROM t",
              "SELECT * FROM t WHERE a = 7",
              "SELECT * FROM t WHERE a = 3 OR a = 9",
              "SELECT * FROM t WHERE (a+b)*(a-b) > 0",
              "SELECT * FROM t WHERE a*a - b*b > 0"]:
        print("\n$ " + q)
        executa(b, q)
    b.fecha()
    print()


if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1] == "teste":
        teste()
    elif len(sys.argv) >= 3:
        dafny = "--dafny" in sys.argv
        args = [x for x in sys.argv[1:] if x != "--dafny"]
        bb = Base(args[0], criar=True)
        executa(bb, args[1], dafny)
        bb.fecha()
    else:
        print(__doc__)
