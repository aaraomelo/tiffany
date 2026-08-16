---
name: project-escada-paga-uma-fibra
description: Cada andar torna UMA fibra total e paga noutra — e os `if` da aritmética são o preço de um andar, não defeitos.
metadata:
  type: project
---

**A LEI DA ESCADA.** Cada andar torna uma fibra total e paga noutra, e não há andar onde
tudo seja total:

| andar | fica total | paga-se |
|---|---|---|
| ℕ | a soma | a subtracção (`a+x=b` só se `b≥a`) |
| ℤ | a subtracção | o sinal |
| ℚ | a divisão | o zero (`0⁻¹`) |
| ℙ¹ | a inversão | a soma (`∞+∞`) |

**«Não eliminamos as excepções; descobrimos em que andar elas são o preço.»** Isso muda o
estatuto de tudo o que a casa escrevia como excepção: deixam de ser defeitos e passam a ser
uma quantidade, que se conta. O `0⁻¹` não sobrevivia à escada — era o preço de UM andar.

**OS `if` SÃO O PREÇO, E CADA UM TEM UM ABSORVEDOR COM NOME.** Os dez `if` do
`racionais.h` testam três coisas — sinal, zero, tecto — e as três vêm da NORMALIZAÇÃO.
Normaliza-se porque em ℚ os números crescem; em `𝔽ₚ` nada cresce, logo não se normaliza, e
sem normalizar: o sinal não existe (tipo), o zero é a troca `[q:p]` (representação e
dualidade), o tecto não tem para onde ir (tipo), o `[0:0]` fica no enunciado (domínio).
Contagem **na fonte**: 10 → 8 → 14 → **0** (`lib/sem_ramo.h`).

A disciplina, do eval: **não eliminar `if` por estética — só quando a condição é absorvida
por primitiva, tipo, domínio, dualidade ou representação; senão só se desloca a excepção.**
E a prova de que nada foi deslocado é comparar EXAUSTIVAMENTE com a versão ramificada:
16128 comparações, zero divergências. E o gume: há um `if` que NÃO se tira — passar do par
ao índice precisa de saber se `q = 0`, porque o índice do ∞ é convenção, não valor do
corpo. Por isso essa função existe para IMPRIMIR, não para calcular.

**A EXAUSTÃO, E O ENCAIXE.** 127 é primo E é o topo do `int8_t`, logo `𝔽₁₂₇` cabe inteiro
no tipo e `|ℙ¹(𝔽₁₂₇)| = 128` — os valores não negativos, com `∞ = [1:0]` a ocupar um lugar
como os outros. A regra do eval: **«não representar o infinito como um número grande —
representá-lo como o dual projectivo de zero».** Aí a inversão é bijectiva nos 128, o gato
é bijecção para todo metal, e a órbita FECHA sempre (períodos 6–128) — logo **não existe
prova por crescimento**, que era a classe de defeito do dia inteiro.

**E A ARITMÉTICA NATURAL.** Em ℕ um racional É uma sequência de naturais, e
`|pₙqₙ₊₁ − pₙ₊₁qₙ| = 1` (74651 pares) dá a forma fechada `1/(qₙqₙ₊₁)`. O módulo de Cauchy
deixa de ser busca na órbita: `qₙ·qₙ₊₁·a > b`, uma comparação de naturais. Formar a
diferença custa o QUADRADO — é a conta que se escolhe que decide o tamanho dos números.

`tests/aritmetica.c` (6:0) · `tests/exaustao.c` (7:0) · `tests/sem_ramo.c` (6:0) ·
`tests/projetiva.c` (6:0). No Universal: `thm:fibra-por-andar`, `thm:sem-ramo`,
`cor:exaustao`, `cor:zero-infinito`. Ver [[project-teorema-do-gato]],
[[feedback-saturacao-nao-e-resultado]], [[feedback-o-write-diz-updated]].
