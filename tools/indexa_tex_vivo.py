#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""indexa_tex_vivo.py — índice FINO: título → @TEX caminho (sem corpo).

O corpus NÃO se duplica. A base só guarda o ponteiro; `conversa responde`
expande @TEX lendo o .tex AGORA — se o ficheiro mudar, a resposta muda.

    python3 tools/indexa_tex_vivo.py | banco/bin/conversa <base> -
    python3 tools/indexa_tex_vivo.py --lista   # só lista, não emite APRENDE
"""
from __future__ import annotations
import re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# Fontes vivas do repo (documentos — não conversa casuais inventadas)
FONTES = [
    ROOT / "teoria.tex",
    ROOT / "catalogo.tex",
    *sorted((ROOT / "papers").glob("*.tex")),
    *sorted((ROOT / "corpus" / "docs").glob("*.tex")),
    *sorted((ROOT / "corpus" / "fala").glob("*.tex")),
]

SKIP = {"estilo.tex", "gkcapa.tex"}  # estilo/capa — sem secções de conteúdo


def limpa_titulo(t: str) -> str:
    t = re.sub(r"\\[a-zA-Z]+\*?(?:\[[^\]]*\])?(?:\{[^}]*\})?", " ", t)
    t = t.replace("{", " ").replace("}", " ").replace("$", "")
    t = re.sub(r"\s+", " ", t).strip()
    return t


def secoes(cam: Path):
    txt = cam.read_text(encoding="utf-8", errors="replace")
    txt = re.sub(r"(?m)(?<!\\)%.*$", "", txt)
    for m in re.finditer(r"\\(?:sub)*section\*?\{", txt):
        i = m.end()
        depth = 1
        j = i
        while j < len(txt) and depth:
            if txt[j] == "{":
                depth += 1
            elif txt[j] == "}":
                depth -= 1
            j += 1
        tit = limpa_titulo(txt[i : j - 1])
        if 4 <= len(tit) <= 120:
            yield tit


def rel(cam: Path) -> str:
    try:
        return str(cam.relative_to(ROOT))
    except ValueError:
        return str(cam)


def main() -> int:
    lista = "--lista" in sys.argv
    n = 0
    vistos = set()
    for cam in FONTES:
        if not cam.is_file() or cam.name in SKIP:
            continue
        r = rel(cam)
        for tit in secoes(cam):
            chave = tit.lower()
            # um título = uma entrada; primeira fonte ganha (não duplica corpo)
            if chave in vistos:
                continue
            vistos.add(chave)
            marc = f"@TEX {r}"
            if lista:
                print(f"{tit}\t{r}")
            else:
                t = tit.replace("'", "").replace('"', "")
                print(f"APRENDE '{t}' '{marc}'")
            n += 1
    print(f"-- {n} título(s) indexados (ponteiro @TEX, sem corpo)", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
