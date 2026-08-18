#!/usr/bin/env bash
# limpa_secoes_corpus.sh — tira prefixos ") " / "1) " / "- " dos títulos \\section
# e re-aprende nas bandas com o título limpo.
#
#   ../tools/limpa_secoes_corpus.sh .fala/<hex> [tex…]
#   ../tools/limpa_secoes_corpus.sh .fala/<hex>   # todos conversa_arvore*.tex + gabarito
#
set -euo pipefail
B="${1:?uso: ./limpa_secoes_corpus.sh <base> [tex…]}"
shift || true
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
[ -x "$CV" ] || { echo "falta $CV"; exit 1; }

if [ "$#" -eq 0 ]; then
  set -- "$ROOT"/corpus/fala/conversa_arvore*.tex "$ROOT"/corpus/fala/conversa_gabarito.tex
fi

python3 - "$CV" "$B" "$@" <<'PY'
import re, sys, subprocess
cv, base = sys.argv[1], sys.argv[2]
texs = sys.argv[3:]

def limpa_tit(t):
    t = t.strip()
    # repete até estabilizar: "1) ", ") ", "- ", "* ", "• "
    for _ in range(6):
        n = re.sub(r'^[\d\*\#•\-–—\.\)\(]+\s*', '', t).strip()
        n = re.sub(r'^\)\s*', '', n).strip()
        if n == t:
            break
        t = n
    return t

def limpa_corpo(txt):
    def repl(m):
        tit = m.group(1)
        limpo = limpa_tit(tit)
        return '\\section{' + limpo + '}'
    return re.sub(r'\\section\{([^}]*)\}', repl, txt)

n_tex = 0
n_apr = 0
for path in texs:
    try:
        raw = open(path, encoding='utf-8').read()
    except FileNotFoundError:
        continue
    novo = limpa_corpo(raw)
    if novo != raw:
        open(path, 'w', encoding='utf-8').write(novo)
        n_tex += 1
        print(f'limpo tex: {path}')
    # re-aprende pares com título limpo
    parts = re.split(r'\\(?:sub)*section\*?\{', novo)
    for corpo in parts[1:]:
        fecha = corpo.find('}')
        if fecha < 0:
            continue
        tit = limpa_tit(corpo[:fecha])
        resto = corpo[fecha+1:]
        resto = re.sub(r'\\[a-zA-Z]+\*?\{?', ' ', resto)
        resto = resto.replace('}', ' ').replace('{', ' ')
        resto = re.sub(r'\s+', ' ', resto).strip()
        if len(tit) < 4 or len(resto) < 20:
            continue
        # se o título sujo ainda existir no corpus, o limpo sobrescreve o caminho certo
        r = resto.replace("'", '').replace('"', '')[:1200]
        t = tit.replace("'", '').replace('"', '')
        subprocess.run([cv, base, 'aprende', t, r], check=False, stdout=subprocess.DEVNULL)
        n_apr += 1
        # também aprende a variante suja → mesma resposta (redirect)
        sujo = ') ' + t
        subprocess.run([cv, base, 'aprende', sujo, r], check=False, stdout=subprocess.DEVNULL)

print(f'ficheiros tex alterados: {n_tex}; pares re-aprendidos: {n_apr}')
# verificação
import pathlib
rest = 0
for p in pathlib.Path(path).parent.glob('conversa*.tex') if False else []:
    pass
for path in texs:
    try:
        raw = open(path, encoding='utf-8').read()
    except FileNotFoundError:
        continue
    rest += len(re.findall(r'\\section\{\) ', raw))
print(f'secções ainda com ") " : {rest}')
PY
