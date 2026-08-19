#!/usr/bin/env python3
"""doubles_todos.py — A RÉGUA NOVA: TODOS os doubles são alvo, e o repo INTEIRO.

A régua velha perguntava «alimenta uma condição `ok(...)`?». Ela fechou em 0 e o
trabalho que ela media acabou. Esta pergunta é outra, e é a do Corpo Algébrico:

    NÃO HÁ MOTIVO PARA DOUBLES.

    A cadeia da descida é double -> long -> int -> uint8 -> bit, e em nenhum degrau
    se perde a matemática (`algebrico sec:do-bit`, tests/cadeia.c 7:0). O real NÃO
    está nesta cadeia: R alcanca-se pelo CORTE (`algebrico thm:corte`), que é
    PERPENDICULAR à descida e corre em inteiros — `a/b < sigma` parte-se em
    `2a-mb < 0` ou `(2a-mb)^2 < b^2 D`, sem perda. E a largura do encaixe que
    alcança R é `1/(U_k U_{k+1})` (`algebrico thm:encaixe`): exacta, a partir de
    inteiros, e SEM AVALIAR NENHUMA RAIZ.

    Logo o double não é um degrau para R. Ele está do mesmo lado que o bit — «o bit
    não está mais longe de R do que o double estava: estão os dois do mesmo lado, e
    nenhum lá chega» — e é o lado caro: não compra R e traz um limiar.

E PORQUE É QUE ESTA FERRAMENTA EXISTE, se já havia três. Porque as três tinham o
mesmo buraco, e ele custou 23 doubles em condição que a casa dava por fechados:

  · audita_tipos.sh e triagem_limiares.sh varrem `lib/*.h tests/*.c tools/*.c banco/*.c`
  · doubles_cond.py sem argumentos varre `tests/*.c banco/*.c`
  · bateria.sh compila `tests/ banco/ tools/ tatoeba/`
  -> `integration/*.c` não está em NENHUMA das quatro, e é lá que vivem os 23.

  · e nenhuma das três conta `float`: `grep -w double` não o vê. São 255 ocorrências
    em .c/.h, 76 ficheiros — llm.c 47, banco/pinos.c 36, gguf.c 18.

Aqui o escopo é `git ls-files`, e o tipo é `double|float`. Um alvo que uma régua não
vê não é um alvo a menos: é um alvo que ninguém sabe que existe.

    python3 tools/doubles_todos.py           o total, e a frente de trabalho
    python3 tools/doubles_todos.py -v        todos os ficheiros, um por linha
    python3 tools/doubles_todos.py f.c ...   só estes, com as linhas

CONTA-SE O CÓDIGO, E NÃO O TEXTO. Os comentários que EXPLICAM a migração falam do
tipo sem o ter, e quem escreve mais sobre ela pareceria migrar menos: são 333
ocorrências e 89 ficheiros inteiros. O `despe` apaga comentários e literais de texto
preservando as quebras de linha, para os números de linha não se moverem.

E AS DUAS CONTAGENS NÃO SE MISTURAM NA MESMA FRASE: ocorrências do tipo e nomes
declarados são réguas diferentes, e vêm em colunas diferentes.
"""
import re, sys, subprocess, collections

# ── despir: o código, e não o texto ────────────────────────────────────────────
def despe(s):
    out, i, n = [], 0, len(s)
    while i < n:
        c = s[i]
        if c == '/' and i + 1 < n and s[i + 1] == '*':
            j = s.find('*/', i + 2); j = n if j < 0 else j + 2
        elif c == '/' and i + 1 < n and s[i + 1] == '/':
            j = s.find('\n', i); j = n if j < 0 else j
        elif c in '"\'':
            j, q = i + 1, c
            while j < n and s[j] != q:
                j += 2 if s[j] == '\\' else 1
            j = min(j + 1, n)
        else:
            out.append(c); i += 1; continue
        out.append(''.join(ch if ch == '\n' else ' ' for ch in s[i:j]))
        i = j
    return ''.join(out)

TIPO = re.compile(r'\b(?:double|float)\b')
DECL = re.compile(r'\b(?:double|float)\s+(?:\*\s*)?([A-Za-z_]\w*)')
LIM  = re.compile(r'\b1[eE]-\d+')

# E UM LIMIAR SÓ CONTA SE DECIDIR. Contar `1e-N` dá 244 no repo e a triagem chama
# «decoração» a 104 deles — mas 186 são VALOR e não régua: SIGMA_SB, K_B, H_PLANCK,
# um passo de grelha, uma escala em nanómetros. O `radiacao.c` aparecia com 13 e doze
# eram metros, coulombs e kelvin. A separação é do STATEMENT, não do literal:
#
#   VALOR   inicialização sem comparação          double vol = 1e-9;      -> não é alvo
#   GUARDA  o eps somado a um denominador, ou o    x/(fabs(q)+1e-30)       -> não decide
#           ramo de um ?: contra divisão por zero  t>0 ? t : 1e-9
#   RÉGUA   o literal está num statement que        if(fabs(a-b) < 1e-9)   -> É ALVO
#           COMPARA — e essa decide se algo passa
#
# Medido assim: 244 = 186 valor + 7 guarda + 51 régua, e das 51 só 32 vivem em tests/
# (as outras 22 estão em integration/, que não corre — §1.6 do cursor).
CMP   = re.compile(r'[<>]=?|[!=]=')
GUARD = re.compile(r'\?[^:]*:\s*[\d.]*[eE]-\d+\s*\)|\+\s*[\d.]*[eE]-\d+\s*\)')
# E O LITERAL TEM DE COMEÇAR POR DÍGITO. `\d*` deixava o `\d*` vazio casar, e então
# `fabs(E-1)` contava como limiar (o `E-1` é uma SUBTRACÇÃO) e `ESP_NV[g[i].e-16]`
# também (o `.e` é um CAMPO menos 16). Eram 21 dos sítios que a régua acusava, e o
# lookbehind fecha os dois: um literal em notação científica começa por dígito, e
# esse dígito não vem depois de letra, sublinhado ou ponto.
LIMQ  = re.compile(r'(?<![\w.])\d+(?:\.\d+)?[eE]-\d+')

def limiares(src):
    """(valor, guarda, regua) — e só a régua é alvo."""
    v = g = r = 0
    for L in src.split('\n'):
        for m in LIMQ.finditer(L):
            if not CMP.search(L): v += 1
            elif GUARD.search(L[max(0, m.start()-40):m.end()+10]): g += 1
            else: r += 1
    return v, g, r

# As quatro classes do `audita_tipos.sh`, e a ordem de ataque sai delas.
def classe(L):
    if re.search(r'typedef|struct|\}\s*[A-Za-z_]+;', L):            return 'estrutura'
    if re.search(r'printf|%[0-9.]*[fge]', L):                       return 'apresentacao'
    if re.search(r'1e-|fabs|\bEPS\b|\beps\b|\btol|TOLER', L):        return 'medicao'
    return 'realizacao'

# O que ALIMENTA uma condição — a régua velha, que continua a ser a mais grave.
def em_ok(src):
    decl = set(DECL.findall(src)); out = []
    for mo in re.finditer(r'\b(?:ok|tique)\s*\(', src):
        i = mo.end(); p = 1; j = i
        while j < len(src) and p > 0:
            p += (src[j] == '(') - (src[j] == ')'); j += 1
        a = src[i:j]; k = a.rfind('"')
        cond = a[k + 1:] if k > 0 else a
        # o nome NÃO conta se for `n.campo`, `n->campo`, `n(...)` nem CAMPO de outro
        u = {n for n in decl
             if re.search(r'(?<![\w.>])' + re.escape(n) + r'\b(?![\w.]|\s*(?:->|\())', cond)}
        if u:
            out.append((src[:mo.start()].count('\n') + 1, sorted(u), ' '.join(cond.split())[:80]))
    return out

# O `#` do Python não é o `//` do C, mas a régua tem de ser UMA: despir C e não despir
# JS/PY dava duas contagens para o mesmo objecto, e os 10 limiares em comentário do
# calculo2/hurwitz/rn/selo saíam do total enquanto os do rede_dual.js entravam.
def despe_py(s):
    s = re.sub(r'"""(?:.|\n)*?"""|\'\'\'(?:.|\n)*?\'\'\'',
               lambda m: ''.join(c if c == '\n' else ' ' for c in m.group()), s)
    s = re.sub(r'"(?:[^"\\\n]|\\.)*"|\'(?:[^\'\\\n]|\\.)*\'', ' ', s)
    return re.sub(r'#[^\n]*', ' ', s)

# E O QUE ESTÁ DENTRO DE UMA CRASE NÃO É JAVASCRIPT. Contar `float` em .js dava 149
# ocorrências e NENHUMA calculava nada: são GLSL de shader (app/src/*_campo.js — o tipo
# é da GPU, não uma escolha minha), fonte C dentro das strings de teste do tradutor
# (tests/traduz_volta.js) e o regex das próprias ferramentas. JS não TEM o tipo. Inflar
# o alvo com isto seria publicar a definição da minha consulta em vez de um facto sobre
# o repo — sai para uma coluna própria, contado e nomeado.
CRASE = re.compile(r'`(?:[^`\\]|\\.)*`', re.S)

def varre(f):
    try:
        raw = open(f, encoding='utf-8', errors='replace').read()
    except OSError:
        return None
    if f.endswith('.py'):
        src, emb = despe_py(raw), 0
    elif f.endswith('.js'):
        emb = sum(len(TIPO.findall(m.group())) for m in CRASE.finditer(raw))
        src = despe(CRASE.sub(lambda m: ''.join(c if c == '\n' else ' ' for c in m.group()), raw))
    else:
        src, emb = despe(raw), 0
    linhas = [(i + 1, L) for i, L in enumerate(src.split('\n')) if TIPO.search(L)]
    cls = collections.Counter()
    for _, L in linhas:
        cls[classe(L)] += len(TIPO.findall(L))
    v, g, r = limiares(src)
    return dict(f=f, tipo=len(TIPO.findall(src)), nomes=len(set(DECL.findall(src))),
                lim=len(LIM.findall(src)), cls=cls, linhas=linhas, emb=emb,
                lim_valor=v, lim_guarda=g, lim_regua=r,
                ok=em_ok(src) if f.endswith(('.c', '.h')) else [])

# ── escopo: o repo inteiro, e não um glob ─────────────────────────────────────
args = [a for a in sys.argv[1:] if not a.startswith('-')]
verboso = '-v' in sys.argv
if args:
    alvos = args
else:
    alvos = [f for f in subprocess.run(['git', 'ls-files'], capture_output=True, text=True)
             .stdout.split() if f.endswith(('.c', '.h', '.js', '.py'))]

# E O FILTRO CONTA OS LIMIARES TAMBÉM. Filtrar por `tipo or ok` deixava de fora
# `tests/rede_dual.js` — 10 limiares `1e-N` e zero doubles —, e a coluna dos limiares
# saía subcontada sem ninguém dar por isso: um ficheiro sem double pode estar cheio de
# régua minha, e a régua é o defeito, não o tipo.
rows = [r for r in (varre(f) for f in alvos) if r and (r['tipo'] or r['ok'] or r['lim'] or r['emb'])]

if args and len(args) <= 4:
    for r in rows:
        print(f"── {r['f']}: {r['tipo']} ocorrências, {r['nomes']} nomes, {r['lim']} limiares")
        for ln, L in r['linhas']:
            print(f"   L{ln:<5d} [{classe(L)[:5]}] {L.strip()[:96]}")
        for ln, u, c in r['ok']:
            print(f"   L{ln:<5d} !! EM CONDIÇÃO {u}  {c}")
    sys.exit(0)

C  = [r for r in rows if r['f'].endswith(('.c', '.h'))]
JS = [r for r in rows if not r['f'].endswith(('.c', '.h'))]

def total(sel, nome):
    t = sum(r['tipo'] for r in sel); nm = sum(r['nomes'] for r in sel)
    lm = sum(r['lim'] for r in sel)
    k = sum(len({x for _, u, _ in r['ok'] for x in u}) for r in sel)
    eb = sum(r['emb'] for r in sel)
    c = collections.Counter()
    for r in sel: c.update(r['cls'])
    print(f"  {nome:<22} {t:>6} ocorr.  {nm:>5} nomes  {lm:>4} limiares  {k:>3} em ok()   "
          f"[estr {c['estrutura']} · real {c['realizacao']} · MED {c['medicao']} · apr {c['apresentacao']}]")
    if eb:
        print(f"  {'':<22} {eb:>6} a MAIS dentro de crases — GLSL de shader e fonte C em strings de teste,"
              f" que NÃO são deste alvo")
    lv = sum(r['lim_valor'] for r in sel); lg = sum(r['lim_guarda'] for r in sel)
    lr = sum(r['lim_regua'] for r in sel)
    print(f"  {'':<22} {'':>6} limiares por CAUSA: {lr} régua (alvo) · {lg} guarda · {lv} valor físico")
    return t, k

print("O QUE FALTA ELIMINAR — todos são alvo, e o escopo é `git ls-files`")
print()
tc, kc = total(C,  f"C/H ({len(C)} fich.)")
tj, kj = total(JS, f"JS/PY ({len(JS)} fich.)")
print()
print(f"  ALVO TOTAL: {tc + tj} ocorrências de double/float, {kc + kj} delas dentro de uma condição.")
print()
print("  A ordem de ataque: MEDIÇÃO primeiro (é um limiar meu dentro da conta),")
print("  ESTRUTURA depois (decide a representação de tudo o resto), REALIZAÇÃO a seguir,")
print("  APRESENTAÇÃO por fim — e ela também sai, porque o que se entrega é a PALAVRA")
print("  (`analitico cor:entrega`), não um decimal.")

if kc + kj:
    print("\n\nOS QUE ESTÃO DENTRO DE UMA CONDIÇÃO — estes primeiro, sem excepção")
    for r in sorted(rows, key=lambda r: -len({x for _, u, _ in r['ok'] for x in u})):
        ns = {x for _, u, _ in r['ok'] for x in u}
        if not ns: continue
        print(f"\n  ── {r['f']}: {len(ns)} {sorted(ns)}")
        for ln, u, c in r['ok']:
            print(f"     L{ln:<5d} {u}  {c}")

print("\n\nA FRENTE DE TRABALHO — por ocorrências" + ("" if verboso else " (top 40; `-v` dá todos)"))
print(f"  {'ficheiro':<40}{'ocorr':>7}{'nomes':>7}{'MED':>6}{'1e-N':>6}{'ok()':>6}")
for r in sorted(rows, key=lambda r: -r['tipo'])[:None if verboso else 40]:
    k = len({x for _, u, _ in r['ok'] for x in u})
    print(f"  {r['f']:<40}{r['tipo']:>7}{r['nomes']:>7}{r['cls']['medicao']:>6}{r['lim']:>6}{k:>6}")
