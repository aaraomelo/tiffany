# Backends da ISA — realizações, nenhuma privilegiada

Fonte: `manifesto.json`. Cada pasta é uma realização (roupa).

A ULA e o gato (GOLD) vivem em `banco/sql.c` — o motor. Os backends **não**
copiam a ULA: traduzem / MOVE. O censo das Partes está em `manifesto.corpos`
(língua ≠ palco ≠ corpo canónico). Órbitas Hopfield: sql / latex / node.
Shells ingeridos (canal, não Hopfield): node / bash / powershell — o pleno
resolve o binário em `banco/tiffany_shell.h` (`TIFFANY_NODE` / `TIFFANY_BASH` /
`TIFFANY_PWSH`), não no PATH do utilizador.

```
*.c  →  tools/traduz  →  assets/figuras/wasm/*.wasm
*.claim → lang/parse_claim → core/execute → ClaimResult
```

```bash
bash tools/sobe_backends_wasm.sh
node tests/backends_wasm.js
cc -O2 -std=c99 -Ilib -Iconecthus/lang -Iconecthus/core \
   tests/claim_ir.c conecthus/lang/parse_claim.c conecthus/core/execute.c \
   -o /tmp/claim_ir && /tmp/claim_ir
```

- `claims/*.claim` — IR (incl. `pipeline.claim` = PipelineClosure)
- `lang/` — parser; `core/` — executor + **controlo** (histerese acima)
- `latex/claim_to_tex.c` — projeção LaTeX sem Result
- Escada: … + volta na estação (`estacao.claim`, chip E)
