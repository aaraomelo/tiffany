---
name: feedback-inventei-o-requisito
description: "Concluí «às matrizes falta uma ORDEM». Ninguém pediu ordem. Inventei o requisito, não o cumpri, e reportei a falta como resultado."
metadata:
  node_type: memory
  type: feedback
---

Medi que o AGM de matrizes fecha onde elas comutam, e terminei com:

> «o que a dimensão traz é o caso não comutativo, e aí **falta a ORDEM**».

Os números estão certos — a ordem de Loewner é parcial, `diag(2,0)` e `diag(0,2)`
são incomparáveis, `AB` não é simétrica quando não comutam. **A conclusão é que
está errada.** O revisor:

> «A matriz **não ordena**. Ela **compõe** ordens.»
> «A ordem diz onde cada coisa está; a matriz diz como as ordens agem umas sobre
> as outras.»
> «Procurar uma ordem para matrizes é pô-las de volta numa reta depois de elas
> terem acabado de ganhar várias direções.»

**E a casa já tinha isto medido.** `tests/corpo_viveiro.c`: *«o índice é a
ESTANTE, não o que está nela»* — o retículo das dimensões (∨=lcm, ∧=gcd) de um
lado, o corpo do outro. `tests/corpodecorpos.c` e `tests/viveiro.c` medem os ℝⁿ
como elementos, com ⊕ (dim a+b) e ⊗ (dim a·b), e nenhum deles pede ordem total.

**Why:** eu tinha uma máquina (o corte, que precisa de ordem linear) e ao levá-la
para um objeto novo perguntei *«o objeto tem o que a minha máquina pede?»* em vez
de *«que máquina este objeto pede?»*. A falta que reportei era a da minha
ferramenta, não a do objeto — e escrita como resultado ela vira uma afirmação
sobre a matemática.

**How to apply:**
1. Antes de escrever «falta X», perguntar **quem pediu X**. Se a resposta for «a
   técnica que eu trouxe», o que há é um limite de alcance da técnica — e
   escreve-se assim: *«o corte aplica-se aqui e não ali»*, não *«ali falta
   ordem»*.
2. O enquadramento é parte do resultado. Os números de §W87 sobreviveram
   intactos à correção; o que mudou foi a frase — e era a frase que estava a
   afirmar algo falso.
3. Da família de [[feedback-a-ausencia-e-deliberada]] («o que falta é decisão,
   não lacuna») e de [[feedback-a-base-incompleta]] («um ponto fora do campo NÃO
   pede régua nova»). É a terceira vez que trato um limite da minha régua como
   um defeito do objeto.

**E o revisor corrigiu-me TRÊS vezes o mesmo tipo de erro, em cadeia:**

1. «falta uma ordem às matrizes» → ordem não era requisito;
2. «as matrizes não podem ser ordenadas» → **podem** (Loewner parcial,
   lexicográfica total). O teorema proíbe *achatar preservando vizinhança*, não
   ordenar;
3. «a matriz não se lineariza» → **lineariza-se**; a bijeção `I^n ↔ I` existe,
   é explícita e reversível. O que não se preserva é a vizinhança.

Os NÚMEROS sobreviveram intactos às três correções. O que mudou foi sempre a
frase — e era sempre a frase que afirmava algo falso. **O enquadramento é parte
do resultado**, e o meu erro recorrente é enunciar um limite da minha técnica
como uma propriedade do objeto.

O teorema que sobreviveu chama-se agora `thm:viz-nao-iso` — o nome anterior
(`thm:naolineariza`) dizia mais do que o enunciado, e o nome é onde a
extrapolação se esconde primeiro.
