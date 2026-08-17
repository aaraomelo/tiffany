#!/usr/bin/env python3
"""citacoes.py — as contagens que os papers citam batem com o que os medidores medem?

Um paper escreve `\\code{tests/reta.c} ($19{:}0$)` e o medidor cresce para 37. O número
fica no texto e não se actualiza sozinho: é a referência escrita à mão, um andar acima do
código. Encontradas sete assim, e o padrão é sempre o mesmo — a citação é o número de
NASCIMENTO do medidor. O `tests/conservacao_metrica.c` nasceu com 11 asserções (commit
df1316a), foi a 13, depois a 14, e três papers continuavam a dizer 11.

E há um caso que diz tudo: o `tests/reta.c` aparecia DUAS VEZES no mesmo geometrico.tex,
uma com 37 e outra com 19. Quem actualizou, actualizou um sítio.

O PADRÃO É ESTREITO DE PROPÓSITO: só `${N}{:}{M}$`, que é a notação da casa para a
contagem. Um regex mais largo apanha `$1{,}4\\times10^{-17}$`, o intervalo `$[0,1[$`, a
lista `$2,4,8,16,32$` e a fracção contínua `$[1,1,1,\\dots]$` — dezasseis falsos positivos
na primeira tentativa, e publicá-los seria a definição da consulta a passar por facto.

    python3 tools/citacoes.py          confere e lista as divergências
    python3 tools/citacoes.py -v       mostra também as que batem
"""
import re, os, sys, glob, subprocess, tempfile

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
# só a notação de contagem da casa: $N{:}M$
CIT = re.compile(r'\\code\{tests/([a-z_0-9\\]+)\.c\}[^\n]{0,120}?\$(\d+)\{:\}(\d+)\$', re.S)
DOCS = ['papers/*.tex', 'teoria.tex', 'catalogo.tex', 'enredo.tex']


def citacoes():
    """(medidor, ok, falhas) -> [onde]"""
    out = {}
    for padrao in DOCS:
        for f in glob.glob(os.path.join(RAIZ, padrao)):
            try:
                s = open(f, encoding='utf-8', errors='replace').read()
            except OSError:
                continue
            for m in CIT.finditer(s):
                nome = m.group(1).replace('\\_', '_')
                linha = s[:m.start()].count('\n') + 1
                chave = (nome, int(m.group(2)), int(m.group(3)))
                out.setdefault(chave, []).append(f"{os.path.relpath(f, RAIZ)}:{linha}")
    return out


def mede(nome, tmp):
    """corre o medidor e devolve (ok, falhas), ou None se não compila"""
    fonte = os.path.join(RAIZ, 'tests', nome + '.c')
    if not os.path.exists(fonte):
        return None
    exe = os.path.join(tmp, nome)
    r = subprocess.run(['cc', '-O1', '-I', os.path.join(RAIZ, 'lib'),
                        '-I', os.path.join(RAIZ, 'tests'), '-o', exe, fonte, '-lm'],
                       capture_output=True)
    if r.returncode:
        return None
    try:
        p = subprocess.run([exe], capture_output=True, text=True, timeout=180,
                           cwd=os.path.join(RAIZ, 'tests'), stdin=subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        return None
    ok = sum(1 for l in p.stdout.split('\n') if l.startswith('#UNIT ok'))
    ma = sum(1 for l in p.stdout.split('\n') if l.startswith('#UNIT falha'))
    return ok, ma


def main():
    verbose = '-v' in sys.argv
    cits = citacoes()
    nomes = sorted({c[0] for c in cits})
    print(f"{len(cits)} citações com contagem, em {len(nomes)} medidores. A correr...\n")
    reais = {}
    with tempfile.TemporaryDirectory() as tmp:
        for n in nomes:
            reais[n] = mede(n, tmp)
    bate = mal = sem = 0
    for (nome, a, b), ondes in sorted(cits.items()):
        r = reais.get(nome)
        if r is None:
            print(f"  [sem medida]  tests/{nome}.c   citado {a}:{b}   em {ondes[0]}")
            sem += 1
        elif (a, b) == r:
            bate += 1
            if verbose:
                print(f"  ok            tests/{nome}.c   {a}:{b}   ({len(ondes)} citação/ões)")
        else:
            mal += 1
            print(f"  DIVERGE       tests/{nome}.c   o paper diz {a}:{b}, mede {r[0]}:{r[1]}")
            for o in ondes:
                print(f"                  {o}")
    print(f"\n  batem: {bate}   divergem: {mal}   sem medida: {sem}")
    if mal:
        print("\n  Uma divergência é quase sempre o número de NASCIMENTO do medidor: ele cresceu")
        print("  e o texto ficou. Corrigir TODAS as ocorrências, não a primeira — o reta.c tinha")
        print("  duas no mesmo ficheiro, com números diferentes.")
    return 1 if mal else 0


if __name__ == '__main__':
    sys.exit(main())
