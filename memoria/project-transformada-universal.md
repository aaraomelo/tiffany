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

## E DOIS REVISORES DERRUBARAM METADE — e a correção ficou melhor

**O primeiro:** *"a borda troca a redução, não a convolução — POR ISSO a transformada continua a
diagonalizar"* é **FALSO**. `Z_p[x]/(p_n)` com `p_n` irredutível **é um corpo**: só tem idempotentes
triviais, logo nenhuma diagonalização por caracteres. A DFT diagonaliza `x^N−1` porque ele **cinde**;
a borda não cinde. *E a reordenação tinha promovido essa frase a afirmação fundadora do paper.*

**A forma verdadeira é melhor:** a transformada é a **avaliação nas raízes**, e para a borda são as
conjugadas de **Frobenius** — as *folhas* do §7. Medido em `GF(13²)`: 1521 testes, 0 falhas. *A
inversa pelos conjugados (§6) e as folhas (§7) passam a ser COROLÁRIOS.*

**O segundo, e é o mais duro:** o **`√N` não sobrevive à correção**. As raízes do metal não estão no
círculo (`φ`, `1/φ`), e **nenhum expoente** torna a matriz de avaliação unitária (`VᵀV =
diag(φ²,φ⁻²)` — ortogonal mas *não isotrópica*).

**E a resolução é do Aarão, em duas frases:**

> *"cai na base ortonormal dual. na cifra."*
> *"o raiz de N é justamente a base da agulha que mede sem invadir o invariante"*

As normas não são **iguais** — são **RECÍPROCAS**: `|σ|·|σ'| = 1` exato, desvio `1,3e-15`.

    o OBJETO   o metal, hiperbólico, norma MULTIPLICATIVA (produto = 1)  ← a cifra
    a AGULHA   a projeção ortogonal, norma ADITIVA (Pitágoras, √N)       ← o instrumento

E *"medir sem invadir"* mede-se: a projeção é **idempotente** e o resto fica **perpendicular**. *A
tensão não é contradição: é a dualidade entre os dois — e é por viverem em normas diferentes que uma
pode medir a outra sem a tocar.*

## O que fica por fazer (do relatório do segundo revisor)

- a **ordem 4 não é universal**: em char 2 cai para 2, e na transformada corrigida o operador é
  Frobenius, com ordem `n`
- a **condição de deconvolução é vazia em `R^n`**: num corpo todo não-nulo inverte (`converte.c` §C½
  mede-o: 2000/2000)
- há **DUAS convoluções com um nome**: a do grupo aditivo (`p^n` pontos, auto-dual) e a do índice
  (`N` pontos, exige `x^N−1`). *O repo já registou este erro exato uma vez:* «misturei o grupo das
  coordenadas com o grupo das transformações»
- o cluster Pontryagin (`transformada.c`, `dualidade.c`, `pontryagin.c`, `trio.c`) está citado **só
  no `viveiro.tex`** — zero no catálogo. *Por isso a transformada foi reescrita de raiz em vez de
  recolhida*
- `tools/formalizador.py` **é um medidor e é invisível dos dois lados** — tem `return 0 if residuo
  == 0`, não é citado, e o `bateria.sh` só lista `*.c` e `morfico.py` à mão
- e a **definição de abertura já exclui `R^n`**: multiplicar por `σ` com a borda é deslocamento **com
  realimentação** — um LFSR, não um shift

## O método que se confirmou

**Recuperar do enredo em vez de reinventar.** A definição estava escrita, com o nome certo, e o meu
trabalho foi *medi-la* — não descobri a transformada universal, verifiquei-a. Isto repete o que
aconteceu com a ISA de pilha e com os cinco domínios: **o repo sabe mais do que eu me lembro, e a
primeira coisa a fazer é procurar lá.**
