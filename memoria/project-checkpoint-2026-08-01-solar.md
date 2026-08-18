---
name: project-checkpoint-2026-08-01-solar
description: "Checkpoint 01/08/2026 (o corpo solar) — a alfândega, a bateria que retém o que não tem dual, a eficiência áurea autodual, e a garrafa como sala de espera"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T15:42:52.165Z
---

# Checkpoint 01/08/2026 (o corpo solar) — tiffany

Continuação do [[project-checkpoint-2026-08-01-maquinas]]. Dois commits, `bd39e32` e `361e934`.
**Bateria: 198 medidores, 196 verdes, 0 falhas.** `teoria.tex` 97 páginas. Corpus 397 pares.
45 commits no dia.

## A lei que liga tudo: a alfândega

Do enredo (`reino_dourado_enredo.tex`, `\part{A Alfândega Dimensional}`): *"toda fronteira cobra,
e esta cobra na única moeda que existe: o **inverso**. O que tem dual atravessa inteiro; o que não
tem fica retido — e o que fica retido, **arde**. Daí a luz."*

E o Aarão completou-a: **"o que é reversível entra no circuito, o que não é fica na garrafa ATÉ
TER DUAL"** — logo a garrafa **não é um cemitério, é uma sala de espera**.

## `tools/solar.c` — o corpo solar

- **A bateria É a alfândega com terminais.** Entra dual (a carga reverte), sai dual (a descarga
  reverte), e o retido é exatamente `2I²Rt`. *"Armazenador de não-dualidade" é exato nos dois
  sentidos*: o que ela entrega é o dual, o que ela retém é o que não tem.
- **A garrafa de Koch da fonte**: harmónicos de Fibonacci com amplitudes `φ^{-j}`, e daí
  `THD² = Σφ^{-2k} = 1/φ` **fechado**, por `φ² = φ+1`.
- **A eficiência é ÁUREA e AUTODUAL**: `FP = φ^{-1/2}` e `FP² = 1/φ = THD²`. *A perda e o ganho
  são o mesmo número, e encontram-se no vinco.* Teto de 78,6% na fundamental; casando N níveis,
  `η → 100%`.
- **O eixo preditivo**: a `PA_m` tem a `m`-ésima diferença constante, e o Teorema da Unificação
  dá predição por fórmula **fechada** — exata até `h=20`. *Ela não itera, logo não acumula.*
- **O ónus**: `ν` racional **fecha**, `ν` irracional **nunca fecha**. *Uma lei, três balcões* —
  o período que não existe, o algoritmo que não existe (§T2), e o `I²R`.

## `tools/koch.c` — completar É ficar reversível

- A garrafa tem **borda infinita em espaço finito** (perímetro por `4/3`, falta de área por
  `4/9`, exato) e dimensão `log4/log3`. **É essa a forma da assinatura**: informação finita
  nomeando alcance infinito.
- **A reversibilidade é uma ESCADA, não um interruptor.** `25 → 40 → 64 → 96 → 144 → 192 → 256`,
  e com **6 bits** a obra volta inteira — *a assinatura vale 2 bits*.
- **E é a mesma escada de três sítios**: os bits da semente, os harmónicos casados (§S6) e os
  níveis do multinível (§M6). *Uma lei, três balcões* — outra vez.
- **Julia cai sem forçar**: sob `z↦z²`, `|z|<1` **extingue** (nilpotente, glacial), `|z|=1`
  **permanece** (idempotente, tropical), e a **fronteira entre os dois É o conjunto de Julia**.
  É a tabela do `kernel/corpo_tropical.tex` — `e²=e` contra `n²=0`.
- **Cantor é a garrafa vista do avesso**: medida zero, cardinalidade contínua.
- **O circuito só enche, a garrafa só esvazia**: `4 → 8 → 16 → … → 256`, dobrando a cada bit.
- **SOLAR ⋈ LUNAR** fecham por **Peirce** (`e+(1−e)=1`, `e(1−e)=0`). Sol é fonte e emite; Lua é
  espelho e reflete. **Não são dois corpos — são o corpo diferencial visto dos dois lados da sua
  dobra**, e é por isso que ele é a instância máxima (§M6): auto-dual, no vinco.

## Onde as fontes estão

- `chess/sandbox/circuito_solar.tex` — a bateria de Koch, o painel casado, a eficiência áurea
- `chess/sandbox/corpo_analitico.tex` — as estrelas irracionais, o ónus
- `kernel/corpo_tropical.tex` (§ tabela, linha ~66) e `kernel/corpo_glacial.tex` — o par
  quente/frio, ponto a ponto, e o fecho por Peirce
- `reino_dourado_enredo.tex` `\part{A Alfândega Dimensional}` (~8059)

## O que fica aberto

O `paper_H` §E7–E12: `PG_m` (progressões geométricas de ordem m), a ordem `m` **contínua** por
interpolação, e `μ`/`β` adaptativos. Medi só o `PA_m` e o Teorema da Unificação.
