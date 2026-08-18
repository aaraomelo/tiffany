#!/usr/bin/env python3
"""Quais doubles ALIMENTAM uma condição `ok(...)` — o que conta na migração.

Um `double` declarado e só impresso não decide nada. O que interessa é o que entra na
CONDIÇÃO, porque é aí que ele traz um limiar de borla.

    python3 tools/doubles_cond.py [ficheiro.c ...]

Sem argumentos, varre tests/*.c e banco/*.c e imprime o total por ficheiro.

O `\\b` sozinho não chega: `\\bt\\b` casa com o `t` de `t.instrucoes`, que é o campo LONG de
uma struct — e o `tikz.c` apareceu com nove doubles que não tinha. O nome só conta se não
for seguido de `.`, `->` ou `(`.
"""
import re, sys, glob

def usados(src):
    decl = set(m.group(1) for m in re.finditer(r'\b(?:double|float)\s+([A-Za-z_]\w*)', src))
    achados = []
    for mo in re.finditer(r'\bok\s*\(', src):
        i = mo.end(); p = 1; j = i
        while j < len(src) and p > 0:
            p += (src[j] == '(') - (src[j] == ')'); j += 1
        a = src[i:j]; k = a.rfind('"')
        cond = re.sub(r'/\*.*?\*/', ' ', a[k+1:] if k > 0 else a, flags=re.S)
        u = set()
        for n in decl:
            # o nome NÃO conta se for `n.campo`, `n->campo` ou `n(...)`, nem se for o CAMPO
            # de outra coisa (`e2.a`, `q->c`) — nesses casos o que está na condição é um
            # membro de struct que só partilha o nome com o double. Fechar só um dos lados
            # deixa passar metade: `\bt\b` apanhava `t.instrucoes`, e `\ba\b` apanhava `e2.a`.
            if re.search(r'(?<![\w.>])\b' + re.escape(n) + r'\b(?![\w.]|\s*(?:->|\())', cond):
                u.add(n)
        if u:
            achados.append((src[:mo.start()].count('\n') + 1, sorted(u), ' '.join(cond.split())[:80]))
    return achados

alvos = sys.argv[1:] or sorted(glob.glob('tests/*.c')) + sorted(glob.glob('banco/*.c'))
tot = 0; nf = 0
for f in alvos:
    src = open(f, encoding='utf-8', errors='replace').read()
    ach = usados(src)
    nomes = set()
    for _, u, _ in ach: nomes |= set(u)
    if not nomes: continue
    nf += 1; tot += len(nomes)
    if len(alvos) <= 4:
        print(f"── {f}: {len(nomes)} ──")
        for ln, u, c in ach: print(f"   L{ln:4d}  {u}  {c}")
    else:
        print(f"  {len(nomes):3d}  {f}")
print(f"\n  TOTAL: {tot} doubles a alimentar condições, em {nf} ficheiros")
