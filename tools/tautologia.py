#!/usr/bin/env python3
"""O detector da asserção que não pode falhar — a forma que a leitura não apanha.

O defeito: uma variável da condição foi DEFINIDA, poucas linhas acima, em função de outra
variável da mesma condição. Aí `X == f(Y)` seguido de `X == Y` (ou `X > Y`) não mede nada —
relê a definição. Foi assim que passaram:

    long ops_dobra = 2L*k;   ok(..., ops_dobra == 2L*k && k > 0);        (plugs.c)
    long a = razao - U;      ok(..., razao == U && a == 0 && ...);       (estrela.c)
    janela_fechada = janela_aberta*ganho;  ok(..., janela_fechada > janela_aberta);  (simula.c)

Nenhum destes é visível a ler a condição sozinha: só olhando para o que a define.

    python3 tools/tautologia.py [ficheiro.c ...]

Sem argumentos varre tests/*.c e banco/*.c. Cada achado é um CANDIDATO, não um veredito:
chamadas com parâmetros de saída (`f(x, &y)` seguido de `x == y`) aparecem e são legítimas.
O teste final é sempre o mesmo — mutar e ver se cai.
"""
import re, sys, glob

def condicao(src, mo):
    i = mo.end(); p = 1; j = i
    while j < len(src) and p > 0:
        p += (src[j] == '(') - (src[j] == ')'); j += 1
    a = src[i:j]; k = a.rfind('"')
    c = (a[k+1:] if k > 0 else a).strip().lstrip(',').strip()
    # os COMENTÁRIOS saem: um /* ... */ dentro da condição trazia as palavras da nota para
    # dentro da análise, e o detector passava a acusar o texto em vez do código
    return re.sub(r'/\*.*?\*/', ' ', c, flags=re.S).strip()

RESERVADAS = {'long','int','double','float','sizeof','fabs','ok','NULL','const','if','for',
              'while','return','char','void','static','unsigned','memcmp','printf'}

def varre(f):
    src = open(f, encoding='utf-8', errors='replace').read()
    L = src.split('\n'); out = []
    for mo in re.finditer(r'\bok\s*\(', src):
        cond = condicao(src, mo)
        ln = src[:mo.start()].count('\n') + 1
        nomes = {n for n in re.findall(r'\b([A-Za-z_]\w*)\b', cond)} - RESERVADAS
        if len(nomes) < 2: continue
        for li in range(max(0, ln-31), ln-1):
            m = re.match(r'\s*(?:const\s+)?(?:double|float|long|int|)\s*([A-Za-z_]\w*)\s*=\s*([^;]+);\s*(?:/\*.*)?$', L[li])
            if not m: continue
            X, expr = m.group(1), m.group(2)
            if X not in nomes: continue
            # `ok(..., x == y)` também casa com o padrão `nome = ...`: se a expressão começa
            # por `=`, aquilo era uma COMPARAÇÃO e não uma atribuição
            if expr.lstrip().startswith('='): continue
            if re.search(r',\s*[A-Za-z_]\w*\s*=', expr): continue     # declaração múltipla
            outros = {n for n in re.findall(r'\b([A-Za-z_]\w*)\b', expr) if n in nomes and n != X}
            # e os PARÂMETROS DE SAÍDA saem: `f(a, &b)` seguido de `a == b` é a rotina a
            # devolver duas coisas, não a condição a reler-se. São a maioria dos falsos
            # positivos, e distinguem-se pelo `&`.
            outros = {o for o in outros if not re.search(r'&\s*' + re.escape(o) + r'\b', expr)}
            for o in outros:
                perto = r'[^;]{0,40}'
                if re.search(rf'\b{re.escape(X)}\b\s*(==|!=|<=|>=|<|>)\s*{perto}\b{re.escape(o)}\b', cond) or \
                   re.search(rf'\b{re.escape(o)}\b{perto}(==|!=|<=|>=|<|>)\s*[^;]{{0,20}}\b{re.escape(X)}\b', cond):
                    out.append((ln, li+1, X, expr.strip()[:60], o, ' '.join(cond.split())[:80]))
                    break
            else: continue
            break
    return out

alvos = sys.argv[1:] or sorted(glob.glob('tests/*.c')) + sorted(glob.glob('banco/*.c'))
n = 0
for f in alvos:
    for ln, li, X, e, o, c in varre(f):
        n += 1
        print(f"  {f}:{ln}   (definido na linha {li})")
        print(f"     {X} = {e}      ← e a condição compara {X} com {o}")
        print(f"     {c}\n")
print(f"  {n} candidato(s). Cada um confirma-se MUTANDO: se o medidor não cai, a condição não media.")
