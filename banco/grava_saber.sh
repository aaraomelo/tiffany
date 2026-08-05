#!/bin/bash
# grava_saber.sh — ELE GRAVA O QUE SABE NO BANCO. E o banco devolve. Ponto.
#
# O Aarão: "não faz sentido esse seu 'não completa', porque é o que o sistema FAZ: gravar um
# texto e recuperar. O sistema é reversível. Manda ele gravar o que ele sabe no banco e pronto."
#
# E ELE TEM RAZÃO CONTRA O QUE EU MEDI. No `cifrando.c` fui perguntar se a cifra se PREVÊ a si
# própria — uma pergunta sobre periodicidade, que Lagrange responde com "só se for quadrático".
# Mas o sistema nunca precisou disso: ele **grava e recupera**, e a reversibilidade já estava
# medida no `plugue.sh` (ler∘escrever = id, resíduo 0).
#
# *Autocompletar, aqui, é o banco devolver o que lá foi posto — não a cifra adivinhar o futuro.*
#
#   ele DIZ      →  o texto
#   nós CIFRAMOS →  a cifra do texto é o ENDEREÇO (o slot onde ele fica)
#   o banco      →  guarda, e devolve
#   a medida     →  o que sai é byte a byte o que entrou, ou não é
set -e
GER=${GER:-qwen2.5:1.5b}
BANCO=${BANCO:-/tmp/banco_saber.dat}
IDX=${IDX:-/tmp/banco_saber.idx}
curl -s -m 5 localhost:11434/api/tags >/dev/null || { echo "  o doador não responde."; exit 2; }
cc -O2 -std=c99 -I../lib -I. erg.c -o /tmp/erg_bin 2>/dev/null
: > "$IDX"
/tmp/erg_bin zera "$BANCO" 8192
echo "  o doador: $GER   ·   o banco: $BANCO (8192 slots)"
python3 - "$GER" "$BANCO" "$IDX" <<'PY'
import json, urllib.request, sys, struct, time
GER, BANCO, IDX = sys.argv[1], sys.argv[2], sys.argv[3]
def api(r,c):
    d=json.dumps(c).encode()
    q=urllib.request.Request(f"http://localhost:11434/api/{r}",d,{"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(q,timeout=300).read())
def fala(p,n=90):
    return api("generate",{"model":GER,"prompt":p,"stream":False,
        "options":{"temperature":0,"seed":7,"num_predict":n}})["response"].strip().replace("\n"," ")
def cifra_de(bs, m=6):
    """A CIFRA do texto: Euclides sobre dois inteiros tirados dos bytes. É o ENDEREÇO."""
    a = sum(b*(i+1) for i,b in enumerate(bs)) or 1
    b = sum(b*b for b in bs) or 1
    out=[]
    while b and len(out)<m:
        q=a//b; a,b=b,a-q*b; out.append(q)
    return out

TEMAS = ["fração contínua","razão áurea","transformada de Fourier","determinante",
         "sequência de Fibonacci","corpo algébrico","involução","período de Pisano",
         "trie","entropia","contrato inteligente","efeito Seebeck"]
SLOT=16
prox = 1                      # o slot 0 fica para o cabeçalho
t0=time.time()
with open(BANCO,"r+b") as fb, open(IDX,"w") as fi:
    for i,tema in enumerate(TEMAS):
        txt = fala(f"Responda em uma frase, sem preâmbulo: O que é {tema}?")
        bs  = txt.encode("utf-8")
        cif = cifra_de(bs)
        # GRAVAR: o texto vai para slots consecutivos, 16 bytes cada, a partir de `prox`
        base = prox
        n = 0
        for k in range(0, len(bs), SLOT):
            pedaco = bs[k:k+SLOT].ljust(SLOT, b"\0")
            fb.seek((base+n)*SLOT); fb.write(pedaco); n += 1
        prox += n
        fi.write(f"{tema}\t{base}\t{n}\t{len(bs)}\t{' '.join(map(str,cif))}\n")
        print(f"  {i+1:2}/{len(TEMAS)}  slot {base:4}  {n:2} slots  {len(bs):3}B  cifra [{' '.join(map(str,cif))}]  {txt[:38]}",flush=True)
print(f"  gravadas {len(TEMAS)} afirmações em {prox-1} slots, {time.time()-t0:.1f}s")
PY
