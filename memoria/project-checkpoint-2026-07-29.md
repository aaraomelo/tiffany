---
name: project-checkpoint-2026-07-29
description: Onde o tiffany parou em 29/07/2026 e o que vem amanhã (a assistente) — ler ao retomar
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-07-30T02:28:47.991Z
---

Checkpoint fechado com o Aarão em **29/07/2026**, no fim de uma sessão longa. Tudo commitado, três
repos limpos do meu lado (`tiffany` em `0059afb`, `broca-so` em `f77c3a8`, `chess` em `0e1e012` — os
10 arquivos sujos do `broca-so` são trabalho dele, não tocar).

**Estado:** 3 papers (`teoria` 14 pág, `tiffany` 6, `microprocessador` 6), 0 pendências de referência.
Bateria em **49 medidores — 47 verdes, 2 negativos por projeto, 0 falhas** (`./tools/bateria.sh`, que
extrai a lista dos próprios papers). `corpo_analitico.tex` do `broca-so` em 32 pág (Partes XV e XVI).

**A última linha de raciocínio da sessão** (três medidores encadeados, todos resíduo 0):
`significado.c` → a janela `1 < classes < pontos` com lei que **conserva** (a borda) ·
`instrumento.c` → sem lei o grupo é `S_N`; com lei sobra o centralizador `d^c·c!`, e as medidas formam
**retículo** de refinamentos (a medida escolhe a resolução, não fabrica o substrato) ·
`antissimetrico.c` → a assimetria que basta é a **anti**: `ω(u,u)=0` e não-degenerada, única classe em
toda dimensão par e nula em `n=1`, `ℂ` é o primeiro lugar onde há algo, e a lei de potência é
`det = Pf²` / módulo `|x|^d`.

**Amanhã (30/07) volta para a ASSISTENTE** — decisão dele, explícita. O que está em aberto lá:

- O §6 do `tiffany.tex` é um resultado **negativo honesto**: a tradução por *embedding* exato não
  fecha (contradição interna), e a causa é **contagem, não ordem**. `tatoeba/ancora.c` e
  `tatoeba/homogeneo.c` devolvem `1` **por projeto** — é resultado, não quebra; não "consertar".
- O teto do descascamento é **identidade do método**, não falta de esforço. O único lugar onde pode
  haver relaxação é o **modo analógico** — é para lá que aponta.
- `d ≥ E/V` é o limite estabelecido para o modelo do operador (`tatoeba/operador.c`).
- `tatoeba/tiffany.c` é REPL: rodar sempre com `</dev/null`, senão pendura até o `timeout`.
- Argumentos obrigatórios (senão o medidor sai `1`/`2` calado e parece falha): `neuronio` e
  `neuronio_analog` pedem caminho de `.tex`; `linear` e `venom` pedem `.pgm`; `ancora` pede
  `pares.tsv 20000`; `homogeneo`/`embedding` pedem `pares.tsv`; `operador` pede `pares.tsv 6 0 0 1`.
  O mapa está no `args()` do `tools/bateria.sh`.

Regra que vale sempre aqui: cada afirmação vira medidor em C com **resíduo 0 ou falha**, e nada de RAM
— ver [[feedback-nunca-usar-ram]].
