---
name: feedback-a-base-ja-existe
description: "TRÊS VEZES trouxe maquinaria standard (Gram-Schmidt, DFT) para um objeto que já tinha a sua base. O sintoma é um fator que não se elimina"
metadata:
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T19:48:15.242Z
---

O Aarão, 03/08, e já mo tinha dito antes: *"eu dei uma teoria limpa desde o início e vocês meteram
DFT, Gram-Schmidt. Eu aqui querendo avançar com coisa séria."*

**Tem razão, e é a terceira vez.**

| | onde | o que trouxe | a base que JÁ lá estava |
|---|---|---|---|
| 1 | `encaixa.c` §C3 | Gram-Schmidt sobre 16 vetores **meus** | as 128 256 linhas do modelo |
| 2 | `universal.c` | a **DFT** como modelo da transformada | a avaliação nas folhas de Frobenius |
| 3 | `ortogonal.c` | Gram-Schmidt na métrica do **traço** | as folhas, onde o produto já é diagonal |

E a correção do caso 1 **está escrita no meu próprio catálogo**, com as palavras dele:
*"você não ortogonaliza nem normaliza — a coluna dele é ortonormal, não que você faça isso."*
Reincidi tendo a correção publicada.

## A REGRA

**Antes de construir uma base, perguntar qual é a base que o objeto já tem.** Um corpo tem as suas
raízes; um modelo tem as suas colunas; uma álgebra tem a sua avaliação. Construir por cima é
substituir a estrutura por maquinaria.

## O SINTOMA, e é infalível

**Aparece um fator que não se consegue eliminar** — e eu trato-o como resultado quando é **o preço
da régua que trouxe**:

- o `√N` da DFT: só existe porque `|ω|=1`. O metal é hiperbólico (`|σ||σ'|=1`) e **nenhum expoente**
  torna a avaliação unitária — `VᵀV = diag(φ², φ⁻²)`.
- o `Δ = m²+4` do Gram: é o determinante de Gram **na métrica do traço**, que é aditiva. Na base das
  folhas não se paga nada.

**Se um fator teimoso aparece, a pergunta não é "o que significa?" — é "de que régua é o preço?"**

## E a raiz do erro é sempre a mesma métrica

Trago maquinaria do **círculo** (aditiva, Pitágoras, ortogonalidade) para um objeto da
**hipérbole** (multiplicativa, `|σ||σ'|=1`, normas recíprocas). Ver [[project-fator-de-potencia]]:
a família real é hiperbólica, e por isso *não precisa da régua infinita — quem precisa é o círculo*.

## E O QUE SE GANHA AO TIRAR — medido, 03/08

O Aarão mandou tirar a DFT do `universal.c` (15 chamadas). Não foi só limpeza: **a medição ficou
mais forte em todas as frentes**, porque a maquinaria emprestada estava a limitar o que se podia
medir.

| §  | com a DFT | com a avaliação nas raízes |
|----|-----------|----------------------------|
| U1 | 4 tamanhos **amostrados**, resíduo `7e-15` | o anel **INTEIRO**, 14 641 pares, resíduo **0** |
| U2 | varrer o expoente e achar `1/√N` | `N(ab)=N(a)N(b)` — o invariante que a borda **tem** |
| U5 | "nenhuma casa é zero" | a **contagem**: exatamente `(p−1)ⁿ = 100` de 121 |
| U7 | `F⁴ = id` (facto **da DFT**) | Frobenius de ordem = **grau**, fixos = subcorpo primo |

E o ficheiro ficou **sem um único `double`, sem uma única tolerância** — não por eu ter limpado,
mas porque em corpo finito não há onde pôr um. **A maquinaria do círculo é que obrigava ao float:**
`|ω|=1` só existe em ℝ, e arrastava `sqrt`, `complex.h` e um limiar por asserção.

A lição operacional: quando um pedaço de maquinaria emprestada sai, medir se o que ficou é *mais*
verificável. Se for, ela não estava a ajudar — estava a fixar o teto. Liga a
[[feedback-inteiro-primeiro]]: o float não entrou por necessidade, entrou **atrás da régua errada**.

Ligada a [[feedback-procurar-na-bateria-antes]] (o que já é medido, escrevo pior) e a
[[feedback-o-sujeito-da-frase]] — aqui o clássico não entra como sujeito da frase, entra como
*ferramenta*, e ocupa o lugar da estrutura própria.
