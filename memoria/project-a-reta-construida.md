---
name: project-a-reta-construida
description: "A construção geométrica de ℝ pela espinha bit → 8 leis → dual → Pisot → corte, e a razão de o corte fechar: |det| = 1, a área conserva-se."
metadata: 
  node_type: memory
  type: project
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-16T04:06:23.523Z
---

O Aarão inverteu o `geometrico.tex` (16/08): ele deixa de ser um paper paralelo a explicar
geometria e passa a ser **a realização geométrica da construção já fechada no Universal**,
com a espinha

    bit → 8 leis → 𝔽₂⁸ → dual → metálico/Pisot → corte → ℝ

e a tese: **«não usamos a reta para construir os números; construímos a reta a partir do
processo que produz os números.»**

**A REGRA EDITORIAL É O QUE DÁ A FORMA:** *nenhuma definição geométrica anterior à
construção do corte*. Foi ela que forçou o paper todo — reta, círculo e polígono só
aparecem depois do §corte. E uma regra dessas obriga a **nomear as próprias excepções**:
a distância no cubo 𝔽₂⁸ (valores em {0..8}, é contagem de bits) e a área/comprimento do
Gato (**nomeiam um DETERMINANTE, que numa matriz inteira é um inteiro**). Sem esse
parágrafo o argumento seria circular — construir ℝ usando área.

**O ELO NOVO, e é o coração:** **a completude é consequência da CONSERVAÇÃO DA ÁREA.**
`det A_m = −1` ⟹ `|det| = |σσ†| = 1` em todo o andar; o quadrado unitário vai num
paralelogramo de comprimento σᵏ e largura |σ†|ᵏ, e **comprimento × largura = 1**. Como o
comprimento diverge, **a largura TEM de ir a zero**. O corte não fecha por axioma
acrescentado — fecha porque a área não pode aumentar nem diminuir. É o [[project-teorema-do-gato]]
a fazer trabalho, não a ser citado.

**E a aproximação é PRODUZIDA, não escolhida:** `σᵏ + (σ†)ᵏ = t_k` é inteiro, logo
`dist(σᵏ,ℤ) = |σ†|ᵏ` — e o critério decide-se em **inteiros puros**,
`(t_k−1)² < ΔF_k² < (t_k+1)²`, que pela identidade `t_k² − ΔF_k² = 4(−1)^k` se reduz a
`t_k ≥ 2` (k ímpar) / `t_k ≥ 3` (k par). Os dois caminhos concordam nos 131 andares.
**A hipótese não é vazia: falha em EXACTAMENTE UM andar** — o ouro no primeiro degrau,
|φ†| = 0,618 > 1/2, e aí o inteiro mais próximo de φ é 2, não 1. É o mesmo m do único
degrau plano (F₁ = F₂ = 1).

**O CONTROLO é o que dá conteúdo a tudo:** `x² − 2x − 4` tem recorrência inteira e traço
inteiro, mas |σ†| = √5 > 1 e det = (−4)^k. O critério falha nos 12 andares e |det| = 1 em
nenhum. **Uma recorrência inteira qualquer não dá a reta: tem de ser unidade E Pisot.**
Sem ele, o teorema podia estar a medir uma propriedade de qualquer recorrência.

**O que a medição apanhou em mim:** o sinal de Cassini trocado (0 de 115 — invisível na
leitura, total na medição); o determinante do controlo sem o factor 4 (companheira com
entrada 4, não 1); e **DOIS números escritos à mão nas asserções** (`potk > 300` quando
eram 264, `casos > 200` quando eram 131) — corrigidos para condições **estruturais**
(mínimo de andares por m), que é o que não envelhece. E um terceiro na prosa: «19 3928»
onde eram 19 360.

E a bateria colhe o veredito **da saída**: a minha linha `§G2 … onde isso FALHA` pôs a
palavra FALHA no relatório e o medidor saiu «VERDE … — FALHA». Ver [[feedback-o-exit-sombreado]].

`tests/geometria_real.c` (9:0) · `papers/geometrico.tex` reescrito, 172 → 513 linhas, 4 → 7
páginas. As duas peças do paper antigo sobrevivem **a jusante** do corte: Arquimedes das
áreas e a torre normalizada passam de axioma a proposições, porque ambas pressupõem área,
que pressupõe o corte. Ver [[project-checkpoint-2026-08-14-curadoria]], [[project-o-real-e-o-corte]].
