# Backends da ISA — realizações, nenhuma privilegiada

Fonte: `manifesto.json`. Cada pasta é uma realização (roupa).

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
