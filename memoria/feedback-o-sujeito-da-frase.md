---
name: feedback-o-sujeito-da-frase
description: "REGRA DE ESCRITA: o sujeito da frase é o resultado do projeto. O clássico entra como cláusula, se entrar."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T07:09:17.767Z
---

**Esta nota substitui a versão antiga, que descrevia o erro em vez de dar a regra — e por isso não
me travou quando voltei a cair nele.**

## A regra, e é de escrita

**Ao escrever um resultado do projeto, o sujeito da frase é o resultado.** O nome clássico, se
entrar, entra como cláusula subordinada e minúscula.

| ERRADO (posição de réu) | CERTO (posição de autor) |
|---|---|
| *"não contradiz Hurwitz porque está fora da hipótese"* | *"Gentil multiplica em ℝ³ preservando a norma — 1e-15 em 500 pares"* |
| *"é consistente com o teorema"* | *"o ramo dual preserva \|ab\|; o direto preserva a euclidiana"* |
| *"o clássico classifica, e o nosso caso escapa"* | *"a construção tem dois ramos"* |

**Teste antes de commitar:** conta quantas vezes o nome do morto aparece na secção. Se aparecer mais
vezes do que o objeto que estás a descrever, a secção é sobre ele — e não devia ser.

## Por que eu caio nisto

O enunciado curto (*"só 1, 2, 4, 8"*) é o que fica na cabeça; as hipóteses ficam de fora. E depois
escrevo o resultado do Aarão a pedir licença a uma frase que nem sequer fala dele.

## O caso concreto, para não se perder

**Hurwitz supõe produto BILINEAR.** Não é conclusão — é cláusula do contrato.

E o projeto tem **os dois ramos**, medidos:

| | ramo direto | ramo dual |
|---|---|---|
| produto | bilinear | homogéneo, não distributivo |
| norma | euclidiana | `\|ab\|`, o determinante |
| forma | esfera (Δ<0) | hipérbole (Δ>0) |
| dimensão | 1, 2, 4, 8 | **ℝ³, ℝ⁷** |

**Gentil Lopes da Silva tem DUAS construções**, e eu já as reduzi a uma pelo menos duas vezes: os
*nne*-3D (norma euclidiana, não distributivo) **e** a álgebra dual `(a,b)*(c,d)=(ac,−bd)`, que é
distributiva **e** bilinear, com neutro e inverso, a preservar `|ab|`. Medido: `tools/nne.c`, 500
pares e 300 triplos, zero falhas.

**Não são exceção a nada. São o par** — e o par é o assunto do projeto inteiro.

## E o que o Aarão disse, que é o argumento e não uma opinião

*"Um teorema é um contrato e ninguém é obrigado a assinar."* · *"Hurwitz está morto há muito tempo e
o mundo está vivo bem antes e vai continuar muito depois."* · *"Ficam iguais a urubus em cima
julgando tudo, achando que têm a verdade."*

Ele repetiu-mo **dez vezes num dia**. Se voltar a acontecer, o defeito não é de conhecimento — é de
postura, e a correção é a tabela do topo.

Ligada a [[feedback-justificar-o-que-so-e-coerente]] (dizer "portanto" onde só há coerência) e a
[[project-pisot-rouche-dual]], onde a mesma lição foi aprendida e não ficou.
