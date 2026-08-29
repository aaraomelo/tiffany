---
name: project-checkpoint-2026-08-29-linguas-isa
description: "29/08 — 19 línguas da ISA ingeridas no motor pela mesma MOVE; asm é degrau ERG, não língua."
metadata:
  type: project
---

# 29/08 — 19 LÍNGUAS, UM ESTÔMAGO

Checkpoint para retomada: ler **este ficheiro**, `conecthus/backends/manifesto.json`
(`corpos.motor.linguagens_isa`), `tests/manifesto_corpos.js` — **não** memória de chat.

Commit: `fcf7a57c` (`master`).
Remoto: `https://github.com/aaraomelo/tiffany.git`

\[
\boxed{
19\ \text{línguas}
\;\xrightarrow{\mathrm{MOVE}}\;
\text{arena}
\;\xrightarrow{\text{hub sql}}
\text{mesma máquina}
}
\]

\[
\boxed{\text{língua}\neq\text{palco}\neq\text{corpo canónico}}
\]

## Estado

| Item | Estatuto |
|------|----------|
| Motor | `banco/sql.c` — interface_padrao=sql |
| Ingerido | 19 chaves de `linguagens[]` |
| `asm` | degrau de `isa.cadeia` (ERG-64); **não** entra em `linguagens[]` |
| `js` ≠ `node` | DOM/escapar vs interpretar/canal |
| Shells | canal (`node`/`bash`/`powershell`); **não** órbita Hopfield |
| Hopfield \(\mathcal{O}\) | `{sql, latex, node}` — não cresce |
| p,q,r | intactos |
| claim `.erg` | `nao localizada` (`wasm_erg` recusa div≠256) — não se inventa cadeia |
| Hospedeiro opcional | `TIFFANY_PTX` / `TIFFANY_LLC` / …; ausente ⇒ `nao localizada` |
| Fractal / EM | Alonzo; EM sem corpo — **não reabrir I0** |

Cadeia de uma língua (quando o ficheiro existe):

```
fonte.c  --traduz-->  lingua.wasm  --wasm_erg-->  celula.erg
```

Pleno: html/css/js = o próprio `.wasm` no DISCO; shells = `banco/*.c` + `tiffany_shell.h`;
ptx/glsl/llvm = emissor na arena.

## Medidores

| Medidor | Resultado |
|---------|-----------|
| `tests/manifesto_corpos.js` | `#TOTAL 43 0` |
| `tests/duomorf_pipe.js` | `53/53` |
| `tests/orbitas_hopfield.js` | `28/28` |
| `tests/backends_wasm.js` §W11 | 19/19 `absorcao.move` |
| `tools/bateria.sh --corpos` | lista `linguagens_isa` + `asm=isa` |

```bat
cmd /c "call tools\env_node.bat && node tests\manifesto_corpos.js && node tests\duomorf_pipe.js && node tests\orbitas_hopfield.js"
```

## Fora deste commit

- `.wasm` e fitas `.bin` (gerados; não entram nesta vaga)
- stub vazio `conecthus/backends/claim/celula.erg` (cadeia claim = `nao localizada`)

## Ficheiros âncora

```
conecthus/backends/manifesto.json   — linguagens_isa; pipe 19; arestas via sql
banco/sql.c                         — motor
banco/tiffany_shell.h               — node/bash/pwsh + hospedeiros opcionais
app/src/banco_absorve.js            — qualquer língua do manifesto
tests/manifesto_corpos.js           — §C11 censo ISA
tests/duomorf_pipe.js               — arestas; pqr intacto
tests/backends_wasm.js              — §W11 MOVE por língua
```

Não promover ptx/glsl/llvm a Parte. Não fundir `wasm` língua com artefactos `*.wasm`.
