#!/usr/bin/env bash
# cresce_tatoeba.sh — amostra casual de tiffany-tatoeba/por.tsv → corpus + base.
#
#   bash tools/cresce_tatoeba.sh [base]
#   default base: banco/.fala/reino
#
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
B="${1:-$ROOT/banco/.fala/reino}"
TSV="${TATOEBA_TSV:-$ROOT/../tiffany-tatoeba/por.tsv}"

[ -x "$CV" ] || {
  cc -O2 -std=c99 -w -I"$ROOT/lib" -o "$CV" "$ROOT/banco/conversa.c" -lm
}
mkdir -p "$B"
[ -f "$TSV" ] || { echo "falta $TSV"; exit 1; }

python3 - "$ROOT" "$TSV" <<'PY'
import re, hashlib, sys
from pathlib import Path
root, tsv = Path(sys.argv[1]), Path(sys.argv[2])
corpus = root / "lib/classe/corpus_frases_pt.txt"
tex = root / "corpus/fala/tatoeba_casual.tex"

def responde(q: str) -> str:
    ql = q.lower()
    if re.search(r"\b(como estás|como vai|tudo bem)\b", ql):
        return "Estou bem, obrigado. E tu?"
    if re.search(r"\b(quem és|como te chamas)\b", ql):
        return "Sou a assistente — conversa leve e teoria quando pedires."
    if re.search(r"\b(obrigad)", ql):
        return "De nada. Estou por aqui."
    if re.search(r"\b(bom dia|boa tarde|boa noite|olá|oi)\b", ql):
        return "Olá! Em que posso ajudar?"
    if q.strip().endswith("?"):
        return "Boa pergunta. Posso responder em tom casual ou subir à teoria — o que preferes?"
    return "Percebi. Queres continuar em conversa leve ou falar de matemática?"

cands_q, cands_c, cands_n = [], [], []
with tsv.open(encoding="utf-8", errors="replace") as f:
    for line in f:
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 3:
            continue
        s = parts[2].strip()
        if not (4 <= len(s) <= 72):
            continue
        if re.search(r"[&<>{}\[\]\\|@#$%^*=_]|https?://|\d{4,}", s):
            continue
        if not re.search(r"[a-záéíóúãõâêôç]", s, re.I):
            continue
        if s.endswith("?"):
            cands_q.append(s)
        elif re.match(
            r"^(Bom|Boa|Olá|Oi|Obrigad|Tchau|Até|Como|Quem|O que|Por que)", s, re.I
        ):
            cands_c.append(s)
        else:
            cands_n.append(s)

def amostra(xs, mod, lim):
    out, seen = [], set()
    for s in xs:
        h = int(hashlib.md5(s.encode()).hexdigest(), 16)
        if h % mod:
            continue
        k = s.lower()
        if k in seen:
            continue
        seen.add(k)
        out.append(s)
        if len(out) >= lim:
            break
    return out

sel_q = amostra(cands_q, 11, 60)
sel_c = amostra(cands_c, 3, 40)
sel_n = amostra(cands_n, 97, 120)
sel = sel_c + sel_q + sel_n

exist, seen = [], set()
if corpus.exists():
    for lin in corpus.read_text(encoding="utf-8").splitlines():
        if lin.startswith("#"):
            exist.append(lin)
            continue
        if not lin.strip():
            continue
        k = lin.strip().lower()
        if k in seen:
            continue
        seen.add(k)
        exist.append(lin.strip())

added = 0
for s in sel:
    k = s.lower()
    if k in seen:
        continue
    seen.add(k)
    exist.append(s)
    added += 1

header = [l for l in exist if l.startswith("#")] or [
    "# corpus_frases_pt.txt — conversa.tex + tatoeba + órbita",
    "# Uma frase por linha.",
]
body = [l for l in exist if not l.startswith("#")][:400]
corpus.write_text("\n".join(header + body) + "\n", encoding="utf-8")

secs = []
for s in sel_c + sel_q:
    tit = s.replace("{", " ").replace("}", " ")
    if len(tit) > 70:
        tit = tit[:67] + "..."
    secs.append(f"\\section{{{tit}}}\n{responde(s)}\n")

tex.write_text(
    "\\documentclass{article}\n\\begin{document}\n"
    "% tatoeba_casual.tex — amostra PT do tiffany-tatoeba/por.tsv\n"
    + "\n".join(secs)
    + "\\end{document}\n",
    encoding="utf-8",
)
print(f"  corpus: {len(body)} frases (+{added} novas desta corrida)")
print(f"  tex: {len(secs)} secções → corpus/fala/tatoeba_casual.tex")
PY

echo "indexa tex vivo (inclui tatoeba_casual.tex) → $B"
export TIFFANY_ROOT="$ROOT"
bash "$D/indexa_tex_vivo.sh" "$B" >/dev/null
echo "tatoeba: base=$B (tex vivo — sem duplicar corpo)"
"$CV" "$B" responde "bom dia" 2>&1 | head -2
