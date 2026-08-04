---
name: project-a-lei-em-dois-niveis
description: "A dualidade promovida a LEI (primeira e segunda, com prova), a descoberta de que as equações já estão publicadas, e a contribuição real: a ANÁLISE da família de potência"
metadata:
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-04T04:15:36.952Z
---

## AS DUAS LEIS, que é onde isto chegou

> **Primeira Lei — toda representação tem dual.** `ρ*(g) = (ρ(g)⁻¹)^T` é representação e
> `(ρ*)* = ρ`. **A prova não usa hipótese alguma sobre G ou V** — por isso é lei e não
> propriedade. (A transposição inverte a ordem, a inversão inverte-a outra vez: cancelam-se.)
>
> **Segunda Lei — exigir que o passo SEJA o dual determina a estrutura.** Derivar dá
> `b²−nb−1=0`; iterar dá `g^(k+1)=id`, uma ordem finita. *A primeira dá o dual; a segunda dá
> a forma.*

Medido: homomorfismo em 14400 pares de GL₂(ℤ), involução em 232 matrizes — zero falhas. E **o
dicionário entra exatamente aí**: `ν(M) = adj(M)^T/det`, logo é `|det|=1` que a faz ficar em ℤ —
que é o que o dicionário palavra→matriz já dá. Das 1880 matrizes com `|det|≠1`, **todas as 1880**
têm dual com denominador.

## A CORREÇÃO DE LEITURA QUE MUDA O SENTIDO INTEIRO

Eu escrevera "a condição para fechar" e listara convexo/compacto/comutativo como **restrições**.
O Aarão: ***"Colocação errada. Não é condição para fechar — é O QUE FECHA. Sempre fecha."***

A coluna certa é **o que a fecha**: o valor, a classe fundamental, a aresta, a incidência, a
convexidade, Haar, o produto, a norma — **todos são O INVARIANTE, a coisa que a troca não move**.
A bidualidade não precisa de licença: fecha porque há sempre algo que não se moveu. O que varia é
**quanto do objeto o invariante alcança** — em dimensão infinita a volta dá-se na mesma, mas chega
a um sítio maior. Por isso o corpo é finito: não para a dualidade existir, mas para o invariante
cobrir o objeto inteiro.

---

Madrugada de 04/08. O Aarão pediu-me para verificar que `f' = f⁻¹` também vale, eu medi que
**não coexiste** com `f = f⁻¹` (na família potência `b=±1` contra `b=φ`) e reportei-o. Ele
respondeu com a correção que reorganizou tudo:

> **"Então são DUAS. `f = f⁻¹` e `f' = f⁻¹`. Essa é a bidualidade."**

Eu estava a exigir que UMA função cumprisse as duas. São duas peças, e o que interessa é que a
segunda **fatoriza** na primeira.

```
nível 0    f  = f⁻¹      involução, dualidade pura, Möbius involutiva (traço 0)
nível 1    f' = f⁻¹      bidualidade: a Möbius de Fibonacci, e o ouro
nível n    f^(n) = f⁻¹   a família metálica
```

E a fórmula **contém** o nível 0: `b² − nb − 1 = 0` com **n = 0** dá `b = ±1`, e `b = −1` é
`f = a/x`, que é a Möbius de traço zero. A involução não é um caso à parte — é o termo n=0.

## A LIÇÃO MAIOR DO DIA: verificar a literatura ANTES de reivindicar

Apresentei ao Aarão a fatorização `M_Fib = J·i` como achado. **É um teorema de 1967**
(Wonenburger 1966, Đoković 1967: uma matriz é produto de duas involuções sse é semelhante à sua
inversa). E `f' = f⁻¹ ⟹ φ` está publicado (J. D. Cook). Só descobri porque o Aarão perguntou
*"acredito que nesse ponto nossa contribuição deve ser apenas organizacional, o que acha?"* —
e eu fui verificar em vez de concordar.

**O enquadramento é dele e é o certo:** *"elas aparecem LARGADAS na literatura. Juntas, e
criamos a análise. A análise completa é nossa contribuição, além de toda a síntese."*

## A ANÁLISE, que é o que é nosso (`tests/escada.c`, 19 asserções, resíduo 0)

- o coeficiente **nunca falha**, e a razão é uma desigualdade de INTEIROS: `σ_n > n` ⟹ todos os
  fatores de `(b)_n` positivos. Não é cálculo caso a caso, é `n²+4 > n²`.
- por Vieta, `σσ' = −1` para todo n — **as raízes SÃO o par da alfândega**, lido nos coeficientes.
- **dicotomia par/ímpar**: `(σ')_n` tem sinal `(−1)^n` ⟹ n par tem DUAS soluções reais, n ímpar UMA.
- `(σ_n)_n` fecha em `Z[σ]`, e para **n ímpar o termo constante é ZERO**: σ, 1+σ, 3σ, 4+8σ, 35σ…

## O TEOREMA: a reversibilidade força Pisot

```
f^(n)=f⁻¹ ⟹ b(b−n)=1 ⟹ termo independente −1 ⟹ |N(σ)|=1 ⟹ σ é Pisot
```

Toda a família metálica é Pisot, sem exceção. E o critério em grau 2 é uma **desigualdade de
inteiros**: `x²−Ax+B` é Pisot ⟺ `−A−1 < B < A−1` (6348 pares, zero discordâncias contra o
cálculo do conjugado). **Alcance de um lado só, dito no texto:** unidade ⟹ Pisot, mas Pisot ⇏
unidade (111 contraexemplos). E a tensão: em grau 2 força sempre; em grau n falha desde n=6.

## E A BIDUALIDADE COMPLETOU A CADEIA — o Aarão apanhou-me outra vez

Escrevi que as transições dimensionais **não** são bijeções. *"Usa a bidualidade nesses casos de
transição dimensional."* Estava a olhar um lado só de um par, que é o erro contra o qual tenho
nota escrita ([[feedback-dual-exige-dois]]).

**A bijeção não é com um conjunto, é com o PAR: `A ↦ A × A*`.** Cayley–Dickson É essa operação
iterada, e a dimensão **conta quantas vezes se dualizou**. O traço sozinho perde (124 bordas em
13 traços, **111 distinções**); com o par `(t,N)` são 124 para 124, zero colapso.

**Mas Hilbert continua a falhar, e por OUTRA razão** — obstrução topológica (continuidade), não
algébrica. *A dualidade repõe o que a álgebra perde; não repõe o que a topologia proíbe.*

## O PAR ITERAR/DERIVAR, que o catálogo já tinha e não juntara

`F³ = F⁻¹` (a transformada, ordem 4) e `Frob^(n−1) = Frob⁻¹` são a MESMA forma `g^k = g⁻¹`, com o
expoente a contar **iterações**. E os dois sentidos dão coisas opostas:

| o expoente conta | vira | e dá |
|---|---|---|
| iterações | `g^(k+1) = id` | uma **ORDEM** — finita, discreta |
| derivações | `b² − nb − 1 = 0` | uma **BORDA** — irracional, contínua |

É Pontryagin escrito **na própria lei**. E arruma o n≡5: em grau 2 os dois lados nunca se
encontram (σ_m > 1 real, nunca tem ordem); é preciso **subir de grau**, e é lá que entra o sexto
ciclotómico. Não é defeito do caso — é o único sítio onde a lei responde das duas formas.

Ligado a [[project-dualidade-memoria-da-divisao]] e [[project-pisot-rouche-dual]].
