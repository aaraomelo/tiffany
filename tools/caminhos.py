#!/usr/bin/env python3
"""Os CAMINHOS citados na infraestrutura — os que ninguém compila e ninguém acusa.

A bateria confere os medidores citados nos papers e os documentos citados nos documentos.
Falta a terceira família: os caminhos escritos em WORKFLOWS, SCRIPTS e CONFIGS. Um
`test -s dist/corpo/papers/dualsort.tex` num workflow não é código que alguém corra ao
compilar — só falha em produção, e às vezes nem lá.

Foi assim que o deploy ficou quebrado de 15/08 a 18/08: a reorganização moveu
`papers/dualsort.tex` para `corpus/docs/`, o manifesto do corpo acompanhou, e o
`publica.yml` — que verifica o ficheiro pelo caminho antigo em DOIS sítios — não. O build
deixou de produzir aquele caminho e ninguém deu por isso.

    python3 tools/caminhos.py        devolve 0 se todos os caminhos citados existem

O regex fecha a extensão com `(?![A-Za-z])`. Sem isso, `.c` casa com o início de `.claim`
e `.json` com o de `.jsonl` — e a primeira versão desta ferramenta acusou 15 falsos
positivos por causa disso, mais do que os 5 defeitos verdadeiros.
"""
import re, os, glob, sys

PADROES = ['.github/workflows/*.yml', 'tools/*.sh', 'app/src/*.json', 'app/vite.config.js',
           'integration/*.js', 'memoria/sincroniza.sh', 'app/nginx/*']
RAIZES  = ('papers', 'corpus/docs', 'corpus/fala', 'cristal', 'tests', 'tools', 'lib',
           'banco', 'app/src', 'conecthus', 'integration')
EXT     = ('tex', 'c', 'js', 'h', 'py', 'json', 'otf', 'ttf', 'wasm', 'sh', 'txt')

raiz = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(raiz)

PAT = re.compile(r'(?<![\w/.-])((?:' + '|'.join(RAIZES) + r')/[A-Za-z0-9_./-]+\.(?:'
                 + '|'.join(EXT) + r'))(?![A-Za-z])')

faltam = {}
n_alvos = 0
for pad in PADROES:
    for f in sorted(glob.glob(pad)):
        if not os.path.isfile(f): continue
        n_alvos += 1
        for i, linha in enumerate(open(f, encoding='utf-8', errors='replace'), 1):
            t = linha.strip()
            if t.startswith('#') or t.startswith('//'): continue     # comentário: é história
            for m in PAT.finditer(linha):
                p = m.group(1)
                if '*' in p or '$' in p: continue                    # caminho construído
                if not os.path.exists(p):
                    faltam.setdefault(p, []).append(f'{f}:{i}')

for p, ondes in sorted(faltam.items()):
    b = os.path.basename(p)
    real = [r for r in glob.glob(f'**/{b}', recursive=True)
            if '.git' not in r and 'app/dist' not in r]
    print(f"  CAMINHO QUEBRADO: {p}")
    print(f"     citado em: {', '.join(ondes)}")
    print(f"     existe em: {real[0] if real else '— em lado nenhum'}")
if not faltam:
    print(f"  {n_alvos} ficheiros de infraestrutura: todos os caminhos citados existem")
sys.exit(1 if faltam else 0)
