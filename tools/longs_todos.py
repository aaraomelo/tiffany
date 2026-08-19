#!/usr/bin/env python3
"""longs_todos.py — A RÉGUA PARA long/long long/__int128/unsigned long long.

Espelho (por espírito) de `tools/doubles_todos.py`, mas agora o alvo é:
  · matar 64 bits no código de produção (long long, ull, __int128)
  · descer de long (64) para int32/duais e depois para int16 (16 bits)

Escopo:
  · por defeito: `git ls-files` para *.c e *.h
  · argumentos: lista de ficheiros a inspecionar

Saída:
  · TOTAL (ocorrências) por tipo
  · uma “frente” ordenada por ocorrências de tokens 64-bit
  · contagem de SCALE/fixo_mil/parse_dec6 (para lembrar que escala é fronteira I/O)

Nota:
  · a régua despira comentários e literais (string/char) para não contar texto.
"""

import re
import sys
import subprocess
import collections


def despe(s: str) -> str:
    """Despira comentários C/C++ e strings/chars (mantém quebras de linha)."""
    out, i, n = [], 0, len(s)
    while i < n:
        c = s[i]
        if c == "/" and i + 1 < n and s[i + 1] == "*":
            j = s.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(ch if ch == "\n" else " " for ch in s[i:j]))
            i = j
            continue
        if c == "/" and i + 1 < n and s[i + 1] == "/":
            j = s.find("\n", i)
            j = n if j < 0 else j
            out.append("".join(ch if ch == "\n" else " " for ch in s[i:j]))
            i = j
            continue
        if c in "\"'":
            q = c
            j = i + 1
            while j < n and s[j] != q:
                j += 2 if s[j] == "\\" else 1
            j = min(j + 1, n)
            out.append("".join(ch if ch == "\n" else " " for ch in s[i:j]))
            i = j
            continue
        out.append(c)
        i += 1
    return "".join(out)


T_LL = re.compile(r"\blong\s+long\b")
T_ULL = re.compile(r"\bunsigned\s+long\s+long\b")
T_I128 = re.compile(r"\b__int128\b")
T_LONG = re.compile(r"\blong\b")
T_INT32 = re.compile(r"\b(?:int32_t|uint32_t|int32|uint32)\b")
T_INT16 = re.compile(r"\b(?:int16_t|uint16_t|int16|uint16)\b")

# escalas e pontos de conversão: conta-se ocorrência, mas não se decide aqui.
SCALE = re.compile(r"\b(?:SCALE|EMB_S)\b\s*=?\s*([0-9]+)")
FIXO_MIL = re.compile(r"\bfixo_mil\b")
PARSE_DEC6 = re.compile(r"\bparse_dec6\b")
STR2DBL = re.compile(r"\bstr2dbl\b")
RT_LE_DEC = re.compile(r"\brt_le_decimal\b")
RT_ESCREVE_DEC = re.compile(r"\brt_escreve_decimal\b")


def counts_for(src: str):
    src = despe(src)

    ll = len(T_LL.findall(src))
    ull = len(T_ULL.findall(src))
    i128 = len(T_I128.findall(src))

    # long simples: conta long token e subtrai as ocorrências “long” que pertencem
    # aos tipos compostos (long long / unsigned long long).
    all_long = len(T_LONG.findall(src))
    # long long tem 2 × "long"; unsigned long long tem 2 × "long" também.
    long_bare = all_long - 2 * ll - 2 * ull

    i32 = len(T_INT32.findall(src))
    i16 = len(T_INT16.findall(src))

    scales = 0
    for _ in SCALE.finditer(src):
        scales += 1
    scales += len(FIXO_MIL.findall(src))
    scales += len(PARSE_DEC6.findall(src))
    scales += len(STR2DBL.findall(src)) * 0  # str2dbl é fase “double”, mantemos ignorado aqui
    scales += len(RT_LE_DEC.findall(src))
    scales += len(RT_ESCREVE_DEC.findall(src))

    return {
        "long": long_bare,
        "long long": ll,
        "unsigned long long": ull,
        "__int128": i128,
        "int32/uint32": i32,
        "int16/uint16": i16,
        "escala_fronteira": scales,
    }


def classify(path: str) -> str:
    """Classificação mínima (sem decidir migração, só apontando o contexto)."""
    base = path.split("/")[-1]
    if base == "dual32.c" or base.endswith("dual32.c"):
        return "TRAVA dual32 (medidor)"
    if base == "le_num.h" or base.endswith("le_num.h"):
        return "LIB le_num (fronteira I/O via bits)"
    if "dual32.h" in path or base == "dual32.h":
        return "PAR dual32 (estrutura/ordem)"
    return "ALVO provável"


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("-")]
    verbose = "-v" in sys.argv

    if args:
        alvos = args
    else:
        # escopo: git ls-files
        txt = subprocess.run(
            ["git", "ls-files"], capture_output=True, text=True, check=False
        ).stdout
        alvos = [f for f in txt.split() if f.endswith((".c", ".h"))]

    rows = []
    total = collections.Counter()

    for f in alvos:
        with open(f, encoding="utf-8", errors="replace") as fp:
            raw = fp.read()
        c = counts_for(raw)
        for k, v in c.items():
            total[k] += v
        c["f"] = f
        c["cls"] = classify(f)
        rows.append(c)

    print("TOTAIS (ocorrências em *.c/*.h, despidas de strings/comentários):")
    for k in ["long", "long long", "unsigned long long", "__int128", "int32/uint32", "int16/uint16"]:
        print(f"  {k:<20} {total[k]:>6}")
    print(f"  {'escala_fronteira':<20} {total['escala_fronteira']:>6}")
    print()

    # frente: ordenar por 64-bit “porções”
    def score(r):
        return r["long long"] + r["unsigned long long"] + r["__int128"]

    rows_sorted = sorted(rows, key=score, reverse=True)
    print("FRENTE (top 40 por 64-bit):")
    for r in rows_sorted[:None if verbose else 40]:
        if score(r) == 0 and not verbose:
            break
        cls = r["cls"]
        print(
            f"  {r['f']:<40}  "
            f"ll={r['long long']:<3} ull={r['unsigned long long']:<3} i128={r['__int128']:<3}  "
            f"long={r['long']:<5}  cls={cls}"
        )

    print()
    print("DICA: a migração usa dual32.h para produtos 32×32, e só deixa __int128")
    print("      como TRAVA/medidor. SCALE/fixo_mil/parse_dec6 são fronteira I/O.")


if __name__ == "__main__":
    main()

