---
name: feedback-a-tautologia-dentro-da-correccao
description: Escrevi uma tautologia DENTRO da correcção de uma tautologia — os dois lados a mesma expressão, com comentários diferentes.
metadata:
  type: feedback
---

A criticar o `bidual.c` §B5 por `long bidual = distintos;` — uma atribuição que
não podia falhar —, escrevi no meu medidor da naturalidade:

```c
int esq = chiY[k][ fmap[x] ];      /* ev_{f(x)}(chi) */
int dir = chiY[k][ fmap[x] ];      /* = (chi∘f)(x)   */
if(esq != dir) falhas_nat++;
```

**Os dois lados são a mesma expressão.** O que os fazia parecer diferentes eram
os COMENTÁRIOS: cada um nomeava um caminho matemático distinto, e nenhum dos
dois estava no código.

**Why:** quando dois objectos são iguais POR TEOREMA, a tradução directa para
código dá a mesma expressão — e o teorema deixa de ser medido. O sinal de alarme
é precisamente o par de comentários a explicar que os lados são diferentes: se
preciso de explicar, o código não mostra.

**How to apply:** cada lado tem de passar por uma OPERAÇÃO que o outro não faz.
No caso: o esquerdo avalia em Y; o direito constrói χ∘f, **procura o índice** na
lista de X̂ (é isso que f̂ é) e avalia a tabela encontrada. Se f̂ não existisse, a
busca devolvia −1 e o teste caía — conta-se isso à parte. Regra: **se os dois
lados da comparação são a mesma expressão, apaga-se um e o teste não muda.**
Testar isso é gratuito. Ver [[feedback-assercoes-vazias]],
[[feedback-a-referencia-escrita-a-mao]], [[feedback-normalizar-nao-e-medir]].
