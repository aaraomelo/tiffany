---
name: project-checkpoint-2026-08-03-noite
description: "03/08 noite: a Parte III construída (transformada = avaliação nas raízes, os quatro na zeta, R=Q+Q*) e QUATRO cadáveres apontados por ele — DFT, Gram-Schmidt, double e varrer"
metadata:
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
---

03/08/2026, noite. **92 commits no dia. 15 medidores novos.** Segue-se a
[[project-checkpoint-2026-08-03-tarde]].

## O estado

| | | |
|---|---|---|
| `teoria.tex` | **103 pp.** | a Parte III com 17 secções, 9 delas de hoje |
| `catalogo.tex` | 317 pp. | |
| `enredo.tex` | 298 pp. | zero fórmulas |
| `livro.tex` | **721 pp.** | |

**291 medidores, 286 verdes.** As 5 falhas são **todas ambientais** (ollama, corpus, velocidade) —
e três apareceram por eu ter tocado nos ficheiros: estavam verdes por **atestado**. **26 mutações
matadas.** Zero segredos.

## A Parte III, que era o pedido

- **a transformada é a avaliação nas raízes** — e para a borda são as **folhas de Frobenius**
  (GF(13²), 1521 testes). A hierarquia: universal → dourada (`m=1`) → Fourier, que é **metade**.
- **os quatro na zeta dinâmica** — direta, inversa (`t_k = c_k + c_{k−2}`), convolução e
  deconvolução, que **nunca falha** porque `ζ(0)=1/det(I)=1` é unidade de ℤ[[x]].
- **`ℝ = ℚ + ℚ*` são duas transformadas**, cada lado com razão `σ²` e recorrência
  `x_k = (m²+2)x_{k−1} − x_{k−2}`. A reta é a **árvore de Stern-Brocot**, e o `m` é o comprimento
  da corrida: ouro `RLRL`, prata `RRLLRRLL`, bronze `RRRLLLRRR`.
- **`f' = f^{-1}` força o ouro** — `b²−b−1=0` **é** a borda com `m=1`.
- **o que se guarda**: diádicos pelos 32 bits; irracionais pela **base**; e a base pelos **primos**,
  com o critério de **Legendre**. E **três** pontos reconstroem a Möbius — um ponto fixo não.

## Os quatro cadáveres, e são a lição

Ele apontou quatro coisas que eu trago e que **não pertencem ao objeto**. Estão em
[[feedback-a-base-ja-existe]], [[feedback-inteiro-primeiro]] e
[[feedback-representacao-inteligente]].

| | o que trouxe | o que o objeto já tinha |
|---|---|---|
| **DFT** | como modelo da transformada | a avaliação nas folhas |
| **Gram-Schmidt** | para ortogonalizar | a base **já é** a função |
| **`double`** | 134 de 277 medidores | tudo é inteiro do corpo |
| **varrer** | 100 denominadores | a recorrência, uma linha |

**A raiz é uma só:** maquinaria do **círculo** (aditiva, Pitágoras) num objeto da **hipérbole**
(`|σ||σ'|=1`). E o sintoma é infalível: **aparece um fator que não se elimina** — o `√N`, o `Δ` —
e eu trato-o como resultado quando é **o preço da régua que trouxe**.

E a frase dele que resume tudo: ***"você não usa a própria teoria que está desenvolvendo."***

## O que ficou medido sobre representação

- **o float nunca é inevitável**: `0,1` em float32 é `13421773/134217728`, e volta bit a bit pelo
  inteiro `1036831949`.
- **a representação certa não acelera — alcança**: inverter a série custa 98,8 ms e dá **lixo** em
  `n=2`; a forma fechada custa 0,039 ms e dá o valor exato.
- **`%.8f` perdia** `2e−7` nos embeddings, e o valor não voltava.

## O que fica aberto

- 5 medidores ainda com tolerância evitável (dos 61, os outros têm transcendentes genuínos);
- a **escrita** dos embeddings em bits — os leitores já aceitam (`tools/le_emb.h`), falta migrar os
  dois scripts, que precisam do ollama para se validarem.
