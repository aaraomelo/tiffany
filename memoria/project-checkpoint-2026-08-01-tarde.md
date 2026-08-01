---
name: project-checkpoint-2026-08-01-tarde
description: "Checkpoint 01/08/2026 (tarde) — o corpo transistor: onde vive o operador, os dois regimes, e o microcontrolador que roda"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T14:59:26.375Z
---

# Checkpoint 01/08/2026 (tarde) — tiffany

Continuação do [[project-checkpoint-2026-08-01-manha]]. Quatro commits,
`bea3ec8`→`f23e353`. **Bateria: 193 medidores, 191 verdes, 0 falhas.** `teoria.tex` 93 páginas.
Corpus 392 pares. 40 commits no dia.

## O arco: da teoria ao silício, e ele fecha

O Aarão levou o sistema até ao metal em três passos, e o resultado é que **a tríade do contrato
tem encapsulamento e três pernas**.

**`tools/eletrico.c` + `eletrico.h` — o corpo transistor.** A fonte é
`chess/sandbox/corpo_transistor.tex`. A tríade encarnada: a **SOMA** ⊕ é **Kirchhoff** (o
resistor, série soma `Z`, paralelo soma `Y`); o **PRODUTO** ⊗ é o **ganho** (o divisor, e compor
divisores multiplica); o **OPERADOR** Π é o **TRANSISTOR**. E *"é onde vive o operador"* é
**literal**: `Π = exp∘Σ∘log` **é** a equação de Shockley, e mede-se `I(V₁+V₂) = I(V₁)·I(V₂)/Is` —
a soma de tensões vira produto de correntes. As multiplicidades: `L` é `+1`, `R` é `0`, `C` é
`−1`, e `L ⋈ C` soma 0 com média geométrica `√(L/C) = Z₀`. O RLC cai na **mesma borda das EDs**,
com o mesmo `Δ` e as mesmas três classes — o crítico é a raiz dupla, `ε²=0`. E §E7 valida pelos
**dois caminhos**: forma fechada contra integração no tempo.

**`tools/amplifica.c` — os dois regimes.** A **mesma** equação, e o que muda é onde se opera. A
janela ativa vale exatamente `VT·ln(99) = 118,8 mV`. **Dentro**: `gm = Ic/VT` é a **derivada**, e
*amplificar É linearizar* — é o corpo `ε²=0`, com o ganho a ser o `f'(a)`. A realimentação troca
o ganho por uma **razão** (a sensibilidade dividida por `1+A'β`), o mesmo movimento da ponte de
Wheatstone. **Fora**: a porta lógica, e ela **amputa** (801 níveis → 3, zona morta 1%). *O digital
não é mais preciso que o analógico — é mais surdo.* E as portas **SÃO GF(2)**: `AND` é o produto,
`XOR` é a soma, `NOT` é a dobra. De Morgan é a dualidade `∧ ⋈ ∨`, involução. NAND é universal,
medido **construindo**.

**`tools/mcu.c` — o microcontrolador multifractal, e ele RODA.** Não mede peças soltas: monta a
máquina e executa. Clock = o astável (dois caminhos), ALU = Joaquim ⊕ e Yasmin ⊗ **só de NAND**
(65536 pares), memória = o endereço **É** o caminho na árvore, barramento casado (`Γ=0`, e o
ganho máximo cai no mesmo ponto). O **HALT é o ponto fixo** — mil pulsos depois de parado não
movem nada. O bustrofédon tem passo unitário (`N²−1`) contra o raster (`2N(N−1)`, salto `N`). E
§U8 executa `1+2+…+n` e o fatorial, validados contra a conta direta.

**A cadeia de oito elos, e nenhum é postulado:**
`Shockley → chaveia → NAND → XOR → somador → multiplicador → ALU → ciclo → programa`

## Na assistente

`conversa.c` foi de 26 para 30 unidades. Portas novas: `rlc`, `serie`, `paralelo`, `divisor`,
`ressonancia`, `wheatstone`, `transistor`, `amplificador`, `ganho`, `logica`, `porta`, `somador`
— cada uma com o **negativo medido** (a fala portuguesa vai ao corpus, não ao resolvedor).

## Onde as fontes estão

- `chess/sandbox/corpo_transistor.tex` — a tríade eletrónica, as multiplicidades, o
  microcontrolador, o bustrofédon, e as três questões (receptor=dual, linha casada=resíduo 0,
  modulação=operador)
- `chess/elementares/corpo_transistor.py` e `sandbox/tecnicas/*.py` — os certificados citados

## O que se repetiu, e é o assunto do feedback

**Três números escritos de cabeça numa sessão** — `"6 cobertas"` (eram 7), `"menos de 0,01%"`
(era 0,0333%), `"salto N−1"` (era N). Ver [[feedback-assercoes-vazias]], que ganhou a secção do
antídoto: **medir a LEI, não a constante**. E uma vez não conferi o total da bateria ao rodar um
medidor sozinho — a lição que já estava em [[feedback-dois-caminhos]] e que eu tinha acabado de
reescrever.
