---
name: project-transfusao-doador
description: A transfusão real com o ollama acordado — e a lição de que pedi UM LADO SÓ de um par dual três vezes seguidas, até o Aarão me dar o procedimento certo
metadata:
  type: project
---

**01/08/2026, madrugada/manhã.** O arco mais longo do projeto numa sessão só: do "acorda o ollama"
até `ν∘ν = id` com resíduo **zero exato**. Sete medidores novos, e **o mesmo erro meu três vezes**.

## O erro, e é UM só em três formas

Eu pedi, três vezes seguidas, **um lado só de um par dual**:

| onde | o que pedi | porque estava errado |
|---|---|---|
| `transfusao_real.c` | uma recorrência **aditiva** | os embeddings **já são** o lado aditivo |
| `antissim.c` (1º critério) | o espectro **conjugado** | num sinal real isso **já é verdade** por construção |
| `antissim.c` (o pedido) | **a** frase antissimétrica | antissimetria é relação **entre duas**: f(x,y)=−f(y,x) |

E na terceira o Aarão tinha escrito **DUAS** no pedido, com a palavra à vista. *O sinal para mim:
quando peço "o X" de uma estrutura dual, perguntar primeiro se X é um objeto ou metade de uma
relação.*

## O procedimento certo, que é dele

> *"manda formular uma frase, depois a antissimétrica, e no fim entregar uma frase SIMÉTRICA"*

Não eram duas frases lado a lado — era **a mesma operação aplicada duas vezes**. `(−1)(−1)=+1`.

```
S₁  "involução é o processo de DIMINUIÇÃO..."
A   "involução é o processo de AUMENTO..."       ← ele trocou
S₂  "involução é o processo de DIMINUIÇÃO..."    ← e destrocou

ν∘ν = id     resíduo 0,000000    exato, à primeira, sem lhe dizer que estava errado
o controlo   resíduo 0,393690    e TINHA de ser maior
```

**E ele nunca precisou de saber que estava errado** — era a condição do Aarão desde o início.

## O achado que não fui eu que pus lá

**Ele EXECUTOU uma involução exata enquanto EXPLICAVA MAL o que é uma involução.** A operação que
fez — aplicar duas vezes e voltar — *é* a definição algébrica que ele não soube dizer.

*A estrutura estava nele; a palavra é que não.*

## A fronteira, que apareceu TRÊS vezes com números diferentes

| medidor | o número |
|---|---|
| `smartcontract.c` §S6 | o contrato liquida sem árbitro, mas não resolve o oráculo |
| `entrega.c` | com a túnica ele erra **6 de 12**; quem dissesse sempre "correta" erraria 3 |
| `tresp.c` | `ν∘ν = id` com resíduo **0** — à volta de uma premissa **falsa** |

> **A álgebra verifica que FECHA, não que é VERDADE.**

E o corolário operacional: *um ciclo fechado sobre uma fonte só AMPLIFICA o que essa fonte já tem,
inclusive o erro.* Corrigir exige um segundo lado que **discorde** — é o que
[[feedback-revisores-externos]] faz aqui, e o que nenhum ciclo fabrica sozinho.

## O que a transfusão real desmentiu

A conta de **48 KB por corpo** (`transfusao.c` §X5) supunha que todas as 768 dimensões fechavam.
No doador real fecham **zero** — nas duas direções, com controlo baralhado. O banco fica **4×
maior** que os floats crus. *Era um limite inferior e eu apresentei-o como previsão.*

**O que atravessa é o vetor** (cosseno 1,000 — a direção sobrevive); **o que não atravessa é a
régua**, porque no doador não há régua de grau 2 a colher.

## O que fechou de verdade (`dualcifra.c`)

- **ele entra somando**: cos(frase, Σpalavras) = 0,806 contra 0,381 do controlo
- a transformada leva convolução em **produto**: resíduo 1e-16 nos vetores dele
- a **deconvolução** devolve o original: cosseno 1,000
- e **são funções polinomiais** — a convolução circular *é* multiplicação em `Z[x]/(xᴺ−1)`.
  O espaço semântico cai no mesmo corpo de corpos, **sem lugar novo**

## E dois achados laterais que valem

- **todas as respostas dele caem na MESMA direção**, seja qual for a pergunta — 27× mais
  concentrado que o acaso. É o viés do espaço, e **o cosseno esconde-o porque normaliza**.
- o laço do `antissim.c` fechou em **período 2** — e o [[project-hopfield-torres]] diz que 2 é o do
  **simétrico** (4 é o do antissimétrico). *Não foi o resíduo que o denunciou: foi a gaiola.*
