---
name: feedback-nao-rodar-a-bateria-toda
description: "Nunca correr a bateria inteira para validar uma mudança pequena — torra o PC do Aarão; correr só o medidor afectado"
metadata:
  type: feedback
---

**«nao torra a porra do meu pc nao roda bateria nehuma... nao faz sentido rodando o
inferno todo pra validar uma merdinha»**

São ~530 sementes e 5090 unidades. Correr `tools/bateria.sh` (ou `--refaz`, que é
pior) para confirmar uma mudança de três linhas é gastar a máquina dele por nada.

**Why:** o custo cai todo no processador do Aarão, e ele vê-o aquecer. Uma mudança
localizada tem um medidor localizado; a bateria inteira só é devida quando a mudança
atravessa o repo (e mesmo aí, é ele que decide).

**How to apply:**

1. **Compilar e correr SÓ o medidor afectado.** Mexi no `banco/sql.c` do lado do
   protocolo → `tests/pgwire.c`. Mexi na cifra → `tests/indexa_orbitas.c`. Um
   comando, um binário.
2. **`--reatesta <nome>`** quando é mesmo preciso passar pela bateria: reabre UM
   medidor, não os 529.
3. **Nunca `: > tools/atestados.txt`** (destrói facto) e **nunca `--refaz` por
   iniciativa própria** — reabre TODAS.
4. **O gume também é local:** mutar e correr o medidor daquele ficheiro, não a
   bateria.

E o irmão disto: **parar de fazer experimentos para descobrir o que já está escrito
nos papers.** Ver [[feedback-ler-a-teoria-antes-de-experimentar]].
