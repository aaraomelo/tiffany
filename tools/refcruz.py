#!/usr/bin/env python3
"""As REFERÊNCIAS CRUZADAS entre documentos — que o LaTeX não vê.

`\\ref{x}` dentro de um documento é conferida pelo pdflatex: se o label não existe, ele
avisa. Mas uma referência a um resultado de OUTRO documento não é um `\\ref` — é texto,
escrito `\\code{topologico thm:det-volume}`. Ninguém a verifica, e ela apodrece em silêncio
quando o teorema muda de nome ou de casa.

Confere três coisas, e a terceira é a que a separação exige:

  1. o documento nomeado EXISTE                     (topologico, analitico, …)
  2. o label existe NAQUELE documento               (e não noutro qualquer)
  3. a DIRECÇÃO é legítima pela separação:
       centro → lado    é REALIZAÇÃO   (o Universal aponta o que os lados realizam)
       lado → centro    é ORIGEM       (o lado diz de que peça canónica parte)
       lado → lado      é o PAR        (τ=−1 ↔ τ=+1, a reflexão) — legítimo, mas raro:
                        se for muito, é sinal de que um lado está a fundar-se no outro
                        em vez de se fundar no centro

    python3 tools/refcruz.py            confere e devolve 0 se está tudo alinhado
"""
import re, sys, os

DOCS = {'algebrico':'papers/corpo_algebrico.tex', 'topologico':'papers/corpo_topologico.tex',
        'analitico':'papers/corpo_analitico.tex', 'computacional':'papers/corpo_computacional.tex',
        'teoria':'teoria.tex', 'catalogo':'catalogo.tex', 'enredo':'enredo.tex'}
CENTRO = 'algebrico'
# A TRÍADE É DE TRÊS. `computacional` NÃO é um lado: é uma realização que vive DENTRO do
# τ=−1 (lê-se pelo que consome — do centro e do endereçamento, e nada do τ=+1). Contá-lo
# como lado era criar a quarta casa que a arquitectura recusa.
LADOS  = {'topologico', 'analitico'}
REALIZ = {'computacional': 'topologico'}   # realização : polo onde vive
PARTICULARES = {'teoria', 'catalogo', 'enredo'}   # segundo escalão: mais específicos

raiz = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(raiz)

labels = {}
for nome, f in DOCS.items():
    if not os.path.exists(f): continue
    for m in re.finditer(r'\\label\{([^}]+)\}', open(f, encoding='utf-8').read()):
        labels.setdefault(m.group(1), set()).add(nome)

CIT = re.compile(r'\\code\{([a-z_]+)[ ~]+((?:thm|def|sec|cor|prop|lem|sub|eq|fig|tab):[a-zA-Z0-9_\\-]+)\}')

def faixa_dividas(src):
    """onde começa e acaba a §sec:dividas — as citações lá dentro são inventário."""
    i = src.find(r'\label{sec:dividas}')
    if i < 0: return (-1, -1)
    j = src.find(r'\section', i + 20)
    return (i, j if j > 0 else len(src))
mortas, erradas, direccoes, divida = [], [], {}, []
for nome, f in DOCS.items():
    if not os.path.exists(f): continue
    _src = open(f, encoding='utf-8').read()
    _di, _df = faixa_dividas(_src)
    dentro_das_dividas = lambda pos: _di >= 0 and _di <= pos <= _df
    for m in CIT.finditer(_src):
        alvo, lab = m.group(1), m.group(2).replace('\\_', '_')
        if alvo not in DOCS:
            mortas.append((nome, alvo, lab, 'documento não existe')); continue
        if alvo not in labels.get(lab, set()):
            onde = ', '.join(sorted(labels.get(lab, set()))) or 'nenhum'
            erradas.append((nome, alvo, lab, f'o label vive em: {onde}')); continue
        # A SECÇÃO QUE CONTA A DÍVIDA NÃO É DÍVIDA. Ela nomeia as peças para dizer que elas
        # estão fora — citar para inventariar não é fundar-se. Sem esta excepção, o próprio
        # inventário aparecia no relatório como seis das nove dívidas que ele lista.
        if nome == CENTRO and dentro_das_dividas(m.start()):
            direccoes['inventário (a §sec:dividas a nomear-se)'] = \
                direccoes.get('inventário (a §sec:dividas a nomear-se)', 0) + 1
            continue
        if nome in REALIZ:                       k = f'realização ({nome}, dentro do {REALIZ[nome]}) → {alvo}'
        elif nome == CENTRO and alvo in LADOS:   k = 'centro → lado (realização)'
        elif alvo in REALIZ:                     k = f'{nome} → realização {alvo}'
        elif nome in LADOS and alvo == CENTRO:    k = 'lado → centro (origem)'
        elif nome in LADOS and alvo in LADOS:     k = 'lado → lado (o PAR)'
        elif nome == CENTRO and alvo in PARTICULARES:
            # O CENTRO A FUNDAR-SE FORA DE SI. Não é o mesmo que citar uma realização:
            # ali a seta vai do centro para fora e diz ONDE a peça se realiza; aqui vai
            # buscar FUNDAMENTO a um documento que devia ser mais particular que ele.
            # É a dívida da separação, e está contada em `corpo_algebrico.tex` §sec:dividas.
            k = f'DÍVIDA: centro → {alvo} (funda-se fora)'
            divida.append((nome, alvo, lab))
        else:                                     k = f'{nome} → {alvo}'
        direccoes[k] = direccoes.get(k, 0) + 1

for t in mortas + erradas:
    print(f"  REFERENCIA CRUZADA QUEBRADA: {t[0]} cita \\code{{{t[1]} {t[2]}}} — {t[3]}")
if not (mortas or erradas):
    print(f"  {sum(direccoes.values())} referências cruzadas, todas apontam para labels que existem")
for k in sorted(direccoes, key=lambda x: -direccoes[x]):
    print(f"     {direccoes[k]:3d}  {k}")
if divida:
    alvos = sorted({t[2] for t in divida})
    print(f"\n  DÍVIDA: {len(divida)} citações em que o CENTRO se funda fora de si,"
          f" sobre {len(alvos)} peças — contadas em corpo_algebrico.tex §sec:dividas:")
    for a_ in alvos: print(f"     {a_}")
sys.exit(1 if (mortas or erradas) else 0)
