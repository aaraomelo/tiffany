---
name: project-condicao-pisot-n-menor-t
description: O encaixe fecha ⟺ n ≤ t. A unidade era SUFICIENTE e eu tinha escrito «necessária»
metadata:
  node_type: memory
  type: project
---

16/08/2026. O `geometrico.tex` §24 dizia **«a hipótese da unidade é necessária ao encaixe»**, e
provava-o com `x² − 2x − 4` (|det| = 4, e o mecanismo quebra). **O contraexemplo está certo; a
conclusão estava a mais.** O que a derruba é uma linha:

```
x² − 3x − 3   tem |det| = 3 ≠ 1   E É PISOT   (|σ†| = 0,7913)
```

## A condição verdadeira, e sai inteira

Para `x² − tx − n` com `t, n ≥ 1`:

```
|σ†| < 1  ⟺  √(t²+4n) < t+2  ⟺  t²+4n < (t+2)²  ⟺  n ≤ t
```

**O termo constante não excede o traço.** A unidade (`n = 1`) satisfaz para TODO `t` — é por isso
que a família metálica é toda Pisot, e é por isso que ela é o **extremo** e não a regra. E o
determinante **sozinho não classifica**: `x²−3x−3` e `x²−2x−4` falham ambos a unidade e caem de
lados opostos.

## A fronteira é onde o irracional ACABA

`n = t+1` ⟹ `D = (t+2)²` é quadrado perfeito ⟹ `|σ†| = 1` exacto, e o polinómio **factoriza**:

```
x² − tx − (t+1) = (x − (t+1))(x + 1)
```

A raiz de módulo 1 é `−1`, o polinómio é **redutível**, e não há irracional nenhum para encaixar.
*A condição não corta o contínuo ao meio: corta exactamente onde ele deixa de existir.*

## E o vazamento mede-se por SINAL, não por limiar

`σ^k` e `(σ†)^k` são as raízes de `y² − t_k y + (−n)^k`, com `t_k` o traço inteiro de Newton.
Como `σ^k > 1`:

```
|(σ†)^k| < 1  ⟺  f(1) = 1 − t_k + (−n)^k < 0   E   f(−1) = 1 + t_k + (−n)^k > 0
```

Teste de sinal sobre inteiros, **onde um limiar teria escolhido um número**.
`tests/condicao_pisot.c` (4:0): 8400 pares, 60 traços, 97 andares.

**E são DUAS exigências, não uma:** `n ≤ t` dá o **vazamento zero** (a face dual encolhe);
`|N(σ)| = 1` dá o **comprimento** `1/(q_k q_{k+1})` do intervalo. Eu tinha-as fundido numa.

Ver [[project-a-reta-construida]] e [[feedback-o-escopo-da-afirmacao]].
