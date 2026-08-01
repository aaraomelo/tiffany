---
name: project-checkpoint-2026-08-01-noite
description: "Checkpoint 01/08/2026 (noite) — a álgebra global, as EDs, o corpo diferencial (Fourier/Mellin/Pontryagin), e cinco correções do Aarão que eram todas a mesma: eu a classificar o objeto errado"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T12:39:47.410Z
---

# Checkpoint 01/08/2026, noite — tiffany

Estado: **bateria 183 total, 181 verdes, 0 falhas**; `teoria.tex` **86 páginas, 0 pendências**;
corpus **382 pares**; 304 commits. Tudo empurrado.
Continuação de [[project-checkpoint-2026-08-01]].

## A cadeia que se fechou

Cada passo usou a máquina do anterior, e **nenhum precisou de máquina nova**:

| passo | e o que se descobriu |
|---|---|
| álgebra global `R^n` | o **corpo é o argumento** — o `i` deixou de estar cravado e virou a borda `s²=−1` |
| EDs 2.ª ordem | **a característica É a borda**, com `D` no lugar do `σ` |
| não homogéneas | a **ressonância é a raiz dupla** outra vez — o mesmo denominador a anular-se |
| sistemas | a de 2.ª ordem **já era** um, com a companion (o gato); e `(B,C) = (−tr, det)` |
| grau `n` | a classificação generaliza pela **assinatura** `(r,s)` |
| corpo diferencial | a cifra é a **reta**, a deformação leva ao **círculo**, e o flip é **Fourier** |
| polinomial | por **dobra**: Sturm (Euclides) e enumeração finita — as irracionais **não se calculam** |
| forma polar | `log z = log r + iθ` — **Mellin no real, Fourier no imaginário** |
| `R^n` em 4 peças | e a **não-comutatividade é o cruzado**: `ab − ba = 2(a×b)` |

## As três coisas que mais valeram

**A assinatura `(r,s)` é a classificação, e o Δ é o caso `n=2`.** Há `⌊n/2⌋+1` assinaturas em
grau `n`; em grau 2 são duas (mais o degenerado) e cabem no **sinal de um número**. *É o mesmo
corpo, nem sequer outra roupa — só outra interpretação*, e o §2 da teoria já o dizia.

**Os dois produtos são as duas metades de qualquer bilinear.** Simétrica = o **direto** (não vê a
ordem, escalar, existe sempre); antissimétrica = o **cruzado** (é a ordem, vetor, obstrução em
dim ≠ 1,3,7). E são a **soma e a multiplicação do corpo de corpos** — o `viveiro.tex` já o tinha:
balancear sobre o comum **é** a ação, e `a·b = lcm·gcd` diz que o tensorial cru são `gcd` cópias.

**O contrato reduziu-se.** A cláusula `Π` está certa como *facto* e errada como *cláusula*: o
caractere **não se declara, calcula-se** de `⊕` (ℤ/n tem `n`, e saem de `n`). *O que era
verificação passou a construção* — e isso é sempre o sinal de se ter percebido a peça.

## As cinco correções do Aarão, e são todas a mesma

1. *"generaliza pq é o mesmo corpo, só outra interpretação"* — eu ia escrever que as três classes
   **não** generalizam, olhando para a **lista de raízes** em vez do **corpo**.
2. *"o i está na família do R^n"* — chamei família à **parametrização** `x^n−mx^{n−1}−1`, quando a
   família é a **construção** `Z[x]/(p)`.
3. *"tá tendo iteração aí, precisa ser dobra"* — Durand-Kerner, 5000 passos. Trocado por **Sturm**,
   que é Euclides, que é a cifra.
4. *"Pontryagin é o produto cruzado"* — eu escrevi **cartesiano**, misturando o grupo das
   **coordenadas** (direto, comuta) com o das **transformações** (afim, cruzado).
5. *"vê se o corpo diferencial fechou"* — eu chamara **dual** ao `∫`, e ele não involui. Não
   faltava *completar* o par: faltava **não lhe chamar dualidade**.

**Todas são a mesma:** eu classifico o objeto errado, ou dou-lhe o nome errado, e depois concluo
que a estrutura falha. A estrutura nunca falhou — falhou o meu apontar.

## E os defeitos que apanhei sozinho

- uma **tolerância de 0,2** escondia que o valor medido era o **símbolo exato** da diferença
  central (`i·sen(ωh)/h`), e não `iω` com erro. *Margem larga dá verde e esconde.*
- a apresentação **mentia sobre os lados** em `x²=x²+1`: a diferença é linear, os lados não são
  retas, e escrever `1·x+0` para o `x²` era a saída a mentir com a resposta certa.
- chamei **acoplamento** ao que era **amortecimento** — não mudava os números, mudava o que
  significam.

## O que amarra tudo, e apareceu três vezes hoje

**Onde a ordem importa, sobra sempre um termo — e esse termo é a estrutura, não o defeito.**
É o mesmo desenho no Leibniz (dois termos), no `D∘∫ ≠ ∫∘D` (o núcleo), e no produto cruzado
(a ação). Três sítios que eu tratava como separados.

## Aberto

- o `l[8192]` do socket ainda aterra em RAM antes do banco;
- o martelo fatiado não substituiu o `sha256` do `OP_MARTELO`;
- a assistente resolve EDs lineares de 2.ª ordem, sistemas 2×2 e polinomiais de qualquer grau;
  EDs de ordem `n` e sistemas `m×m` estão medidos em `geral.c` mas não ligados à porta.
