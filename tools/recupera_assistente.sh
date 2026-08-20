#!/usr/bin/env bash
# recupera_assistente.sh — Recupera o corpus dos repos e deixa a assistente funcional.
#
# Fontes (já existem; não se inventa):
#   corpus/fala/*.tex          conversa casual + árvores de fundação
#   corpus/docs/*.tex          torre, partitura, medida…
#   tools/casual.sh            sementes banais + conversa.tex
#   tools/fundacao.sh          escada Corpos→…→Peano
#   papers/arquitetura.tex     aranha / dragão / idiomas (opcional)
#
#   cd tiffany && bash tools/recupera_assistente.sh [base]
#   default: banco/.fala/reino
#
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
B="${1:-$ROOT/banco/.fala/reino}"

[ -x "$CV" ] || {
  echo "compila conversa…"
  cc -O2 -std=c99 -w -I"$ROOT/lib" -o "$CV" "$ROOT/banco/conversa.c" -lm
}
mkdir -p "$B"

aprende(){ "$CV" "$B" aprende "$1" "$2" >/dev/null; }

echo "=== recupera assistente → $B ==="

# 1) Casual (cumprimentos)
bash "$D/casual.sh" "$B"

# 1b) Operar o sistema (bateria, painel, tex vivo, …) — não TeX
bash "$D/semear.sh" "$B"

# 2) Fundação (teoria mínima operacional)
if [ -x "$D/fundacao.sh" ]; then
  bash "$D/fundacao.sh" "$B" || true
fi

# 3) Índice vivo dos .tex (SÓ títulos → @TEX; corpo NÃO se copia)
#    Se o ficheiro mudar, responde() lê o actual.
echo "indexa tex vivo (sem duplicar corpo)…"
export TIFFANY_ROOT="$ROOT"
python3 "$D/indexa_tex_vivo.py" 2>/dev/null | "$CV" "$B" - >/dev/null || true

# 4) Docs/papers já cobertos pelo índice vivo — NÃO ingerir corpos
echo "  … papers/corpus/docs/teoria via @TEX (leitura ao vivo)"

# 5) Ponte operacional (semear cobre SQL/bateria); títulos de teoria vêm do índice vivo
aprende "idioma português" "álgebra sobre Word_8: Σ, monoide Σ*, léxico dual, regras. lib/portugues.h; IMPORT IDIOMA pt"
aprende "traz os idiomas" "português, inglês, espanhol — mesma tubagem. Detalhe: papers/arquitetura.tex (tex vivo)"

# 6) Exporta frases casuais para a órbita (corpus_frases_pt.txt)
python3 - "$ROOT/corpus/fala/conversa.tex" "$ROOT/lib/classe/corpus_frases_pt.txt" <<'PY'
import re, sys
from pathlib import Path
src, dst = Path(sys.argv[1]), Path(sys.argv[2])
txt = src.read_text(encoding="utf-8", errors="replace")
txt = re.sub(r"(?m)(?<!\\)%.*$", "", txt)
partes = re.split(r"\\(?:sub)*section\*?\{", txt)
frases = []
# títulos = falas casuais
for corpo in partes[1:]:
    fecha = corpo.find("}")
    if fecha < 0: continue
    tit = corpo[:fecha]
    tit = re.sub(r"\\[a-zA-Z]+\*?", " ", tit)
    tit = tit.replace("{", " ").replace("}", " ")
    tit = re.sub(r"\s+", " ", tit).strip()
    if 3 <= len(tit) <= 80 and re.search(r"[a-záéíóúãõâêôç]", tit, re.I):
        frases.append(tit)
# respostas curtas também (primeira frase do corpo)
for corpo in partes[1:]:
    fecha = corpo.find("}")
    if fecha < 0: continue
    resto = corpo[fecha+1:]
    resto = re.sub(r"\\[a-zA-Z]+\*?(\[[^\]]*\])?(\{[^}]*\})?", " ", resto)
    resto = resto.replace("{", " ").replace("}", " ").replace("$", "")
    resto = re.sub(r"\s+", " ", resto).strip()
    m = re.match(r"(.{15,100}?[.!?…])", resto)
    if m:
        s = m.group(1).strip()
        if re.search(r"[a-záéíóúãõâêôç]", s, re.I):
            frases.append(s)
# léxico / órbita
extra = [
    "o ouro do rei é álgebra e órbita do dragão",
    "a prata e o ouro da casa do rei",
    "álgebra do dragão na órbita do ouro",
    "o que é a aranha",
    "curva do dragão",
    "órbita de uma frase",
]
frases = extra + frases
# únicos, estáveis
seen, out = set(), []
for f in frases:
    k = f.lower()
    if k in seen: continue
    seen.add(k); out.append(f)
    if len(out) >= 200: break
dst.write_text(
    "# corpus_frases_pt.txt — recuperado de corpus/fala/conversa.tex + órbita\n"
    "# Uma frase por linha.\n"
    + "\n".join(out) + "\n",
    encoding="utf-8",
)
print(f"  export: {len(out)} frases → {dst}")
PY

echo
echo "=== smoke ==="
for q in "bom dia" "oi" "quem és tu" "estou cansado" "mostra a fundação" "o que é o dragão" "como corro a bateria" "como opero o sistema"; do
  echo "Q: $q"
  TIFFANY_ROOT="$ROOT" "$CV" "$B" responde "$q" 2>&1 | head -2
  echo
done
echo "base pronta: $B"
echo "uso: banco/bin/conversa $B responde \"…\""

# 7) Órbitas no banco (opcional — se sql compilar)
if [ -f "$ROOT/lib/classe/corpus_frases_pt.txt" ]; then
  bash "$D/indexa_orbitas.sh" /tmp/tiffany_orbita_idx >/tmp/recupera_ix.out 2>&1 \
    && echo "órbitas: $(grep -c '^orbita ' "$ROOT/lib/classe/corpus_orbitas_pt.txt" 2>/dev/null || echo 0) indexadas" \
    || echo "órbitas: (indexa_orbitas.sh falhou — ver /tmp/recupera_ix.out)"
fi
