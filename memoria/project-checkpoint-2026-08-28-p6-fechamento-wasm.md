---
name: project-checkpoint-2026-08-28-p6-fechamento-wasm
description: "28/08 — P6 fecha a arquitectura; combinacao.tex fundido em redes.tex; próximo: motor wasm cliente e servidor."
metadata:
  type: project
---

# 28/08 — P6 FECHA; VOLTA AO MOTOR WASM

Checkpoint para retomada: ler **este ficheiro**, `papers/redes.tex`,
`lib/arena_combinacao.mjs`, `tests/redes_combinacao.js` — **não** memória de chat.

`papers/combinacao.tex` **não existe**. O conteúdo está em `papers/redes.tex`.

## Estado teórico

| Item | Estatuto |
|------|----------|
| **P6** | **Postulado de fechamento da arquitectura.** Autoriza \(X\times X\to X\). Não se mede. |
| \(C=W\) | Hebb: \(W_{ij}=\sum_p\xi_i^p\xi_j^p\) |
| Hopfield dual | memória da volta: \(\lambda^++\lambda^-=0\) |
| overwrite | Hebb de capacidade 1 (corolário, não outra lei) |
| \(\tau, D_{\mathrm{can}}, \operatorname{prof}, u\) | **demonstrados** — não autorizam \(C\) |
| lexMax / lexMin | **não promover** a \(C\) |
| `fisica.tex` EM / `fis:obs:cisao` | **não reabrir** |

Cadeia:

\[
\underbrace{\tau,\; D_{\mathrm{can}},\; \operatorname{prof},\; u}_{\text{demonstrados}}
\quad\longrightarrow\quad
\underbrace{C}_{\text{postulado}}
\quad\longrightarrow\quad
\underbrace{\text{realizações físicas}}_{\text{testáveis}}.
\]

Negar P6 não refuta um corolário: nega a operação de composição.

## Estado experimental (realização, não lei)

| Item | Estatuto |
|------|----------|
| C0–L7, M1–M2, \(E_\partial\) | medido; host **não** nomeia \(C\) |
| Bateria | `node tests/redes_combinacao.js` → **`#TOTAL 274 0`** |
| `tools/auditoria_rg6.bat` | chama a bateria |

## Próximo: motor wasm no cliente e no servidor

O paper da combinação **fechou**. A retomada é o motor:

- **Cliente (browser):** `app/` — Vite, `app/banco/`, `app/src/banco_*.js`, `celula_wasm.js`, `canal_browser.js`, `terminal.js`, `c_wasm_node.js` / `c_wasm_shell.js`
- **Servidor (metal/Patria):** `banco/sql.c`, `banco/erg.c`, `banco/bash.c`, `banco/node.c`, `banco/powershell.c`, `banco/canal_patria.c`
- **Backends:** `conecthus/backends/{bash,node,powershell,shell,html,css,js}/` + `manifesto.json`
- **Tradução C→wasm:** `lib/wasm_erg.mjs`, `lib/c_asm_shell.mjs`, `tools/traduz.c`, `tools/wasm_erg.c`
- **Plano do barramento:** `memoria/plano-terminal-barramento.md`

Não reabrir P6 nem lexMax. Avaliar realizações **contra** o postulado.

## Como verificar

```bat
cmd /c "call tools\env_node.bat && node tests\redes_combinacao.js"
```

Esperado: `#TOTAL 274 0`.

Papers: `pdflatex papers/redes.tex` (dois passos).

## Ficheiros âncora

```
papers/redes.tex                 — P6 fechamento; §Combinação
psi/multifocal.tex               — P6 alinhado (não eixo III)
fisica.tex                       — relógio dual; não reabrir EM
lib/arena_combinacao.mjs         — realização C0–L7
tests/redes_combinacao.js        — 274 asserts
app/src/                         — motor wasm no browser
banco/*.c                        — motor no metal
conecthus/backends/manifesto.json
memoria/plano-terminal-barramento.md
```
