---
name: project-o-fecho-do-dual-lagrange
description: "A casa tinha o split directo/cruzado num paper e N(xy)=N(x)N(y) noutro, e nunca escreveu que são a MESMA frase — Lagrange fecha-os, e o degrau 4 de Hurwitz sai daí."
metadata: 
  node_type: memory
  type: project
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T17:58:48.007Z
---

Construí Λ² (a cunha, a parte ANTISSIMÉTRICA) e deixei o produto interno do outro lado.
O Aarão: «le as algebras de gentil dual de hurwitz e **fecha o dual** [...] gentil conserva
norma». Tinha razão — eu estava a medir **metade de um par**, outra vez.

A equação que os fecha é uma linha, exata em inteiros:

    ⟨u,v⟩² + ‖u∧v‖² = N(u)·N(v)          (Lagrange)
    directo² + cruzado² = a norma conservada

**A casa já tinha as duas metades, em ficheiros diferentes, sem a frase que as junta:**

- `papers/corpo_analitico.tex` §637-640 — «o DIRECTO ⟨a,b⟩ = cos θ é a parte SIMÉTRICA, a
  potência ACTIVA; o CRUZADO ‖a∧b‖ = sin θ é a ANTISSIMÉTRICA, a REACTIVA que roda e volta
  (o torque É o produto cruzado)», com fp = cos θ e tan φ = cruzado/directo.
- `tests/hurwitz.c` §H2 — N(xy) = N(x)N(y) em 1,2,4,8, e onde se parte.
- E ninguém escreveu que a segunda **É** a primeira: Lagrange é cos²θ + sin²θ = 1 com a
  norma por dentro, medida ao QUADRADO (a raiz não se tira).

## As três consequências, e nenhuma é decorativa

1. **Cauchy–Schwarz deixa de ser desigualdade.** ⟨u,v⟩² ≤ N(u)N(v) é Lagrange com
   ‖u∧v‖² ≥ 0, e a FOLGA é exatamente o cruzado. Medido: 117649 pares, a folga É o
   cruzado em todos. A desigualdade é uma igualdade a que se apagou um termo.

2. **A conservação da norma OBRIGA a dimensão 4.** Para u,v puros em dim 3 o produto é
   uv = (−⟨u,v⟩, u×v): um escalar e um vetor. O escalar NÃO CABE em dim 3. Hurwitz não é
   um teto posto de fora — é onde o directo arranja lugar ao lado do cruzado.

3. **A estrela.** O `thm:central` do corpo estelar diz que a bijeção dual Gentil↔Hurwitz
   É A ESTRELA, com ν∘ν = id resíduo 0. O ⋆ de Hodge tem a mesma propriedade entre Λᵏ e
   Λⁿ⁻ᵏ. **Mas o «Hodge» dos papers é a CONJECTURA** (ciclos algébricos, o milénio) —
   mesmo nome, outro objeto, e não se juntam.

## O gume, e o seu limite dito à frente

«A soma de k quadrados fecha para o produto?» — o buscador acha a testemunha
**exatamente em k = 3** (`2 · 14 = 28`, e 28 = 4·7 não é soma de três quadrados) e volta
vazio em 1, 2 e 4. Os degraus não são citados: são o resultado da procura.

**E para k ≥ 4 a tese é SEMPRE VERDADEIRA** (todo natural é soma de 4 quadrados), logo
correr em 5, 6, 7 mediria o vazio — [[feedback-varrer-onde-nada-pode-falhar]]. O que
exclui 5,6,7 é a BILINEARIDADE, e essa é o teorema, não um número.

Realizado em `lib/exterior.h`, `tests/hurwitz.c` §H6–H7 e `banco/conversa.c` §C40 +
«exterior 1..15». Ver [[project-fator-de-potencia]], [[feedback-medir-so-metade-do-par]],
[[feedback-dual-exige-dois]], [[project-transformada-universal]].
