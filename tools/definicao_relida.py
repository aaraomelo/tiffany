#!/usr/bin/env python3
# definicao_relida.py — A ASSERÇÃO QUE VERIFICA A PRÓPRIA DEFINIÇÃO.
#
# Em 17/08 apanhei este padrão CINCO vezes, em cinco ficheiros diferentes, e é sempre a
# mesma forma: uma variável é definida por uma expressão, e uma asserção mais abaixo
# verifica essa mesma expressão. A conta fecha por construção, o limiar mede só o
# arredondamento da reescrita, e nenhuma entrada a pode derrubar.
#
#     forca.c        V = Pi*S            depois  razao = V/(Pi*S)          → 1
#     cosmico.c      Qf = Q - W          depois  W + Qf == Q               → W+Q−W
#     tikz.c         #define TFIM (N*H)  depois  fabs(N*H - TFIM) < 1e-12  → x−x
#     spline.c       w = av*s/upem       depois  w12/w10 == 1.2            → 12/10
#     encanamento.c  snr_saida = snr/F   depois  snr/snr_saida == F        → F
#
# O `gume.py` não os apanha (mutar o operador derruba a asserção na mesma) e o
# `residuo_zero.py` só apanha os que imprimem um resíduo. Este procura a FORMA.
#
# COMO: para cada asserção, extrai os identificadores da condição; para cada um, procura a
# sua definição no mesmo bloco; e assinala quando a definição de um deles MENCIONA outro
# que aparece do outro lado da comparação. É uma heurística — dá falsos positivos, e por
# isso ela reporta e não julga, como as outras duas ferramentas da casa.
#
#   uso:  python3 tools/definicao_relida.py [ficheiro.c ...]

import re, sys, glob, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IDENT = re.compile(r'\b[A-Za-z_][A-Za-z0-9_]*\b')
PALAVRA_C = {'if','for','while','return','int','long','double','float','const','static',
             'void','char','sizeof','struct','else','break','continue','unsigned','fabs',
             'sqrt','printf','ok','conclui','tique','do','switch','case','default','cabs',
             'creal','cimag','exp','log','sin','cos','pow','hypot','floor','ceil','atan'}

def parte_virgulas(txt):
    """separa `a = 0, b = f(x,y), c = 2` nas suas partes, sem partir dentro de parênteses.
    Sem isto, uma declaração múltipla — `int a = 0, b = 0;` — faz a definição de `a`
    parecer mencionar `b`, e a lista enche-se de ruído: a primeira versão desta ferramenta
    deu 250 candidatos e quase todos eram isso."""
    partes, prof, ini = [], 0, 0
    for i, c in enumerate(txt):
        if c in '([{': prof += 1
        elif c in ')]}': prof -= 1
        elif c == ',' and prof == 0:
            partes.append(txt[ini:i]); ini = i+1
    partes.append(txt[ini:])
    return [p.strip() for p in partes if p.strip()]

def defs_do_ficheiro(linhas):
    """{nome: (linha, expressão)} — a PRIMEIRA definição vista de cada identificador."""
    d = {}
    for i, L in enumerate(linhas):
        t = L.strip()
        if t.startswith('*') or t.startswith('//') or t.startswith('/*'):
            continue                                   # comentário não define nada
        for mo in re.finditer(r'#define\s+([A-Za-z_]\w*)\s+(.+)', L):
            d.setdefault(mo.group(1), (i+1, mo.group(2).strip()))
        corpo = L.split('/*')[0]
        # tira o tipo da frente, se houver, e parte pelas vírgulas de topo
        corpo = re.sub(r'^\s*(?:const\s+)?(?:unsigned\s+)?'
                       r'(?:double|float|long|int|char|size_t)\s+', '', corpo)
        for parte in parte_virgulas(corpo.rstrip(';')):
            mo = re.match(r'^([A-Za-z_]\w*)\s*=\s*([^=].*)$', parte)
            if not mo: continue
            nome, expr = mo.group(1), mo.group(2).strip()
            if nome in PALAVRA_C: continue
            # uma inicialização a constante não é uma definição interessante
            if re.fullmatch(r'[-+]?[0-9.]+[eE]?[-+]?[0-9]*[LlUuFf]?', expr): continue
            d.setdefault(nome, (i+1, expr))
    return d

def analisa(caminho):
    fonte = open(caminho, encoding='utf-8', errors='replace').read()
    linhas = fonte.split('\n')
    defs = defs_do_ficheiro(linhas)
    achados = []
    for mo in re.finditer(r'\bok\s*\(', fonte):
        i = mo.end(); prof = 1; j = i
        while j < len(fonte) and prof > 0:
            if fonte[j] == '(': prof += 1
            elif fonte[j] == ')': prof -= 1
            j += 1
        arg = fonte[i:j]
        k = arg.rfind('"')
        if k <= 0: continue
        txt, cond = arg[:k+1], arg[k+1:]
        nomes = [n for n in set(IDENT.findall(cond)) if n not in PALAVRA_C and n in defs]
        ln_ok = fonte[:mo.start()].count('\n') + 1
        for n in nomes:
            ln_def, expr = defs[n]
            if ln_def >= ln_ok: continue
            outros = [o for o in nomes if o != n]
            # a definição de `n` menciona outro nome que também está na condição?
            usados = [o for o in outros if re.search(r'\b'+re.escape(o)+r'\b', expr)]
            # …e esse outro aparece do OUTRO lado da comparação que `n`?
            if usados:
                achados.append((ln_ok, n, ln_def, expr[:56], usados[:3],
                                re.sub(r'\s+', ' ', txt)[1:72]))
    return achados

def main():
    alvos = sys.argv[1:] or sorted(glob.glob(os.path.join(RAIZ, 'tests', '*.c')))
    print("  A ASSERÇÃO QUE VERIFICA A PRÓPRIA DEFINIÇÃO — onde olhar")
    print("  (heurística: reporta e não julga. A pergunta é sempre a mesma —")
    print("   esta condição pode dar outra coisa em ALGUMA entrada?)\n")
    n = 0
    for alvo in alvos:
        a = analisa(alvo)
        if not a: continue
        print(f"  ── {os.path.basename(alvo)}")
        vistos = set()
        for ln_ok, nome, ln_def, expr, usados, txt in a:
            chave = (ln_ok, nome)
            if chave in vistos: continue
            vistos.add(chave); n += 1
            print(f"     linha {ln_ok}: `{nome}` (definido na {ln_def} como `{expr}`)")
            print(f"        e a condição também usa {usados} — que está na definição")
            print(f"        «{txt}»")
        print()
    print(f"  candidatos: {n}")
    return 0

if __name__ == '__main__':
    sys.exit(main())
