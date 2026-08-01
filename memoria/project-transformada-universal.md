---
name: project-transformada-universal
description: "A transformada universal (tools/universal.c): o √N é ELA, os três √N são um só pela ortogonalidade, R^n é uma realização, e o corpo universal é AUTODUAL"
metadata:
  type: project
---

# A transformada universal — `tools/universal.c`

01/08/2026, commit `5dc2ae9`. **224 medidores, 222 verdes, 0 falhas.** Atualizados os três, como o
Aarão pediu: o medidor, o `catalogo.tex` e o `teoria.tex`.

## Ela já estava no enredo — e recuperar vale mais que reinventar

`chess/sandbox/geracao_energia.tex`:

> *"a transformada universal o diagonaliza (`F(a ⊛ b) = F(a)F(b)`, resíduo 0)"*

**É a única coisa que se lhe pede**, e daí sai tudo o resto. E é isso que a torna *universal*: não
depende do corpo, só de haver um produto invariante por deslocamento. Medido em `N=4,8,16,32`,
resíduo `7e-15`.

## O √N é ELA — e os três √N são um só

A afirmação do Aarão (*"raiz de N é a transformada universal"*) é testável, e passa. Varrendo o
expoente `p` em `F_jk = ω^jk · N^-p` **em vez de o pôr à mão**, só `p = 1/2` conserva a norma — e só
com ele `F† = F⁻¹`.

    1. normalizar a transformada      2,4e-16
    2. normalizar a base (Hadamard)   0,0
    3. promediar N medidas (ruído)    1,0e-02

**Não são três coeficientes que coincidem: é UM.** A razão é a **ortogonalidade** — somar `N`
independentes é somar `N` direções perpendiculares, e Pitágoras conta isso como `√N`; a transformada
que usa essa base conserva essa norma (Parseval). *Promediar melhorar por √N não é um acaso
estatístico: é Parseval.*

## R^n é uma REALIZAÇÃO

Multiplicar em `Z_p[x]/(x^N−1)` **é** convoluir os coeficientes — as N casas batem. **A borda
`σⁿ = mσⁿ⁻¹+1` só troca a REDUÇÃO, não a convolução** — e por isso a transformada continua a
diagonalizar e a régua `(B,C)` continua a mesma. *R^n não é outra teoria: é o corpo universal com
uma redução escolhida.*

## E o corpo universal é AUTODUAL (verificado a pedido)

    F⁴ = id                  ordem 4  —  o i, que RODA
    F² = a reflexão n → −n   ordem 2  —  o J, que ESPELHA
    e há vetores IGUAIS à sua própria transformada

Os pontos fixos constroem-se somando a órbita `a + Fa + F²a + F³a`, e a **gaussiana** é o exemplo
clássico (1,74% no discreto, exato no contínuo).

**O que autodual significa aqui, e é preciso:** *a transformada não sai do corpo*. Um corpo que
precisasse de outro para se transformar não seria universal — teria de haver um terceiro para fechar,
e depois um quarto. E fecha com o §B12: `F` tem ordem 4 (roda) e `F²` ordem 2 (espelha) — **o J vive
dentro do i**.

## A deconvolução

Existe **sse nenhuma casa de `F` se anula**, e isso é decidível numa varredura. O núcleo constante
anula 15 de 16 — *é o que não tem dual, e fica na garrafa* ([[project-headjack-dual]]).

## O método que se confirmou

**Recuperar do enredo em vez de reinventar.** A definição estava escrita, com o nome certo, e o meu
trabalho foi *medi-la* — não descobri a transformada universal, verifiquei-a. Isto repete o que
aconteceu com a ISA de pilha e com os cinco domínios: **o repo sabe mais do que eu me lembro, e a
primeira coisa a fazer é procurar lá.**
