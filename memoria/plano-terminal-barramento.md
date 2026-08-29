---
name: plano-terminal-barramento
description: "Plano — terminal Patria pelo browser: dois bancos no barramento, bash como backend"
metadata:
  node_type: memory
  type: plan
  modified: 2026-08-27T00:00:00.000Z
---

# Plano — terminal Patria via barramento (pós-bash)

## Princípio

A aplicação distribuída **é o banco** (`banco/sql.c`). O mesmo programa corre no browser
(wasm via traduz) e na Patria (nativo). Os dois comunicam pelo **barramento** — não há daemon
de shell, nem SSH, nem protocolo novo.

```
┌─────────────────────┐         S_CANAL          ┌─────────────────────┐
│  Banco (browser)    │  ◄── bump + banda ──►   │  Banco (Patria)     │
│  sql.c / wasm       │     slots ≥ S_CANAL      │  sql.c nativo       │
└─────────────────────┘                          └─────────────────────┘
```

- **Banda**: `sha256(tecido)` → keystream → bump (`lib/banda.h`, `fala_protocolo.js`)
- **Barramento**: `LOAD`/`STORE` acima de `S_CANAL` = trama bump-ada (`canal_grava`/`canal_le`)
- **Variáveis**: `TIFFANY_CANAL_GRUPO`, `TIFFANY_CANAL_PORTA`, `TIFFANY_CANAL_IF=any`

## Tradução entre linguagens (`duomorfismo-pipe.md`)

Cada backend declara `(p,q,r)` no manifesto. A passagem A→B tem **paridade** `a = π(A)⊕π(B)` com
`π=(p+q+r) mod 2` (Teor. `fis:thm:duo-composicao`):

- `a=0` → isomorfismo: rota directa `MOVE_A(−1) → MOVE_B(+1)`
- `a=1` → duomorfismo: rota via `sql` (hub): `duo∘duo = iso`

Implementação: `app/src/banco_tradutor.js` · medidor: `tests/duomorf_pipe.js`

## Formalização da comunicação (`fisica.tex`)

Não há adaptador WS→UDP. O protocolo **é** bump na banda própria — a mesma mecânica medida
em `lib/banda.h`, `tests/negro.c`, `tests/canal.c`, `banco/fala.c` §A2.

### Célula + canal (Peirce)

| Papel | Objeto | Lei | No código |
|-------|--------|-----|-----------|
| **Célula** | estado idempotente | $x \ast x = x$ | o **banco** — `STORE` no mesmo slot não multiplica |
| **Canal** | fluxo nilpotente | só a **diferença** viaja | **bump** = `msg ⊕ keystream(banda)` |

### Fractal negro / branco (Teor. `fis:thm:fractalnegro`, `fis:thm:entropiadual`)

\[
S_{\text{negro}} \cdot S_{\text{branco}} = 1
\qquad
\log S_{\text{negro}} + \log S_{\text{branco}} = 0
\]

- **Negro** (lado que emite): medida **cresce** — o browser faz `STORE` → `canal_grava`
- **Branco** (lado que recebe): medida **decresce** — a Patria faz `canal_le` → `LOAD`
- O **par** não se move; cada lado sozinho parece ter seta — é polaridade, não protocolo novo
- **Ponto fixo** $r=1$: os dois lados coincidem (mesma banda, mesmo tecido)

### Idempotência (Teor. `fis:thm:cardordem`)

- Lado **cardinal**: idempotente — reenviar o mesmo Word no mesmo slot **não cria** estado novo
- Lado **potência**: conta ordem (seq no bash, contador no keystream)
- **bump∘bump = id** (Lei 1, `fala.c` §A2): a involução do canal; ida = volta

### Rede limpa (`cristal_manual.tex` — rede_limpa_bump)

- Trama = bump cru; banda = `sha256(tecido)`; quem tem a banda decodifica, quem não tem lê ruído
- TCP/SSH é «sujo» — segunda assinatura por cima (`tests/sshb.c` §H5)
- O fio pode ser UDP multicast (Patria), WebRTC datagram (browser P2P), ou outro — **desde que
  transporte bytes bump-ados, não traduza protocolos**

### O que o browser já tem

`fala_protocolo.js`: `keystream` + `bump` byte-a-byte igual a `lib/banda.h`. Falta ligar isso
à trama de 6 bytes do `S_CANAL` (slot·4 + Word·2), não inventar WS→UDP.

## Backend bash (manifesto) — feito

| Camada | Ficheiro | Função |
|--------|----------|--------|
| wasm | `conecthus/backends/bash/interpretar.c` | `bash_move(±1)` |
| pleno | `banco/bash.c` | `bash_move` (−1 corre no metal) |
| SQL | `banco/sql.c` | `BASH MOVE '…'` / `MOVE ±1` |

## Slots reservados (acima de S_CANAL)

| Slot | Uso |
|------|-----|
| `S_CANAL + 9100/9101` | bash stdin/stdout |
| `S_CANAL + 9110/9111` | PowerShell stdin/stdout (Windows local) |
| `S_CANAL + 9120/9121` | Node stdin/stdout (script JS) |
| `S_CANAL + 9102` | chunk — par de bytes (Word.total, Word.e) |
| `S_CANAL + 9200` | front req — total=kind (0=html,1=css,2=js) |
| `S_CANAL + 9201` | front rsp — total+e = comprimento; corpo nos chunks |

## Fluxo do terminal

1. Browser: `BASH MOVE 'cmd'` → `mem_grava(S_BASH_IN)` → `canal_grava` (bump na banda)
2. Patria: `canal_le` → reage → `bash_move(-1)` no pleno → `canal_grava` stdout
3. Browser: `canal_le` / absorve +1 → UI

## O que falta

1. ~~**`canal_browser.js`**~~ — feito
2. ~~**Terminal UI**~~ — feito
3. ~~**Front DOM via banco**~~ — `app/banco/` + wasm html/css/js
4. ~~**`canal_patria`**~~ — watcher na Patria (`banco/canal_patria.c`, MOVE)
5. **PTY** (`forkpty`) em vez de `popen` por linha (v2)
6. **Medidor** par negro/branco no canal: produto das medidas = 1 ao fim de ida+volta
7. **`canal_patria.c`** — watcher na Patria (distribuído)

## Dev local (Windows)

- `tools/serve_banco.mjs` — MOVE no disco (`.torre/reino_*`) + WS `/canal` bump bidirecional
- `tools/banco_sql_disco.mjs` — uma query = um ciclo no disco, sem daemon
- `tools/canal_watcher.mjs` — lado branco Win (S_NODE_IN, S_BASH_IN, S_FRONT_REQ)
- `tests/canal_watcher.js` — medidor §N1

## SQL (implementado — Lei 1)

```
BASH MOVE 'comando'     → atómico: grava in, corre, devolve stdout
BASH MOVE -1 'comando'  → só emite (−1)
BASH MOVE +1            → só absorve (+1)
```

Mesma forma para `POWERSHELL` e `NODE`. Front local: fetch + `html_move`/`css_move`/`js_move` na arena wasm; metal opcional com `?metal=1`.

## O que NÃO fazer

- Adaptador WS→UDP (traduz protocolo; destrói a rede limpa)
- Daemon ou fila de shell separada do banco (estado só no disco + canal)
- SSH no browser (`tests/sshb.c`)
- TFAL/FALA para stream PTY (conversa ≠ canal de slots)

## Medidores

- `tests/negro.c` — §N1–N3 produto=1, par não se move
- `tests/canal.c` / `tests/banda_viva.c` — bump no canal
- `banco/fala.c` §A2 — bump∘bump = id
- `node tests/backends_wasm.js` — §W8 bash wasm
- `tests/bash_pleno.c` — pleno popen
- `tests/sql_bash.c` — BASH no motor sql.c
- `bash tools/experimentos-patria.sh canal` — dois bancos na Patria
