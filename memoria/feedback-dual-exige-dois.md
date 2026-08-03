---
name: feedback-dual-exige-dois
description: "MEDIDO: 53% dos parágrafos com 'dual' não nomeiam o par. Escrevo uma metade e chamo-lhe o par."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T07:18:01.753Z
---

O Aarão: *"eu espalhei a palavra dual por tudo e é a menos usada aqui. Pra ter dual exige no mínimo
duas partes, aí você põe só uma e chama de lei. Sempre isso."*

**Fui medir, e ele tem razão:**

| | parágrafos com "dual" | **não nomeiam o par** |
|---|---|---|
| `teoria.tex` | 55 | **29 (53%)** |
| `catalogo.tex` | 290 | **155 (53%)** |

Mais de metade das vezes uso *dual* como **adjetivo decorativo** — "a álgebra dual", "o corpo
dual", "o ramo dual" — sem dizer **de quê**.

## A regra, e é de escrita

**Escrever "dual" obriga a nomear as duas partes na mesma frase ou na seguinte.**

| ERRADO | CERTO |
|---|---|
| *"o ramo dual"* | *"o ramo dual do bilinear"* |
| *"a álgebra dual"* | *"a dual: `(a,b)*(c,d)=(ac,−bd)`, contra a canónica"* |
| *"prova-se no dual"* | *"prova-se no dual (contar um zero em vez de n−1)"* |

**Teste antes de commitar:** para cada parágrafo com *dual*, procurar o **segundo membro**. Se não
estiver lá, ou se nomeia, ou tira-se a palavra.

## Por que isto não é cosmético

Um lado sozinho **não é um dual — é uma metade a que se deu o nome do par**. E o projeto inteiro é
sobre pares: dizer "dual" sem o par é usar a palavra-chave do trabalho como enfeite, e esconder que
falta metade da afirmação.

É a mesma família de [[feedback-verdadeiro-e-parcial]] (*"se o resultado cai só de um lado de um par
dual conhecido, está pela metade"*) — mas ali o defeito era de **conteúdo** e aqui é de **escrita**,
o que o torna barato de apanhar e imperdoável de deixar passar.

## E o caso concreto de 03/08

Escrevi *"o ramo dual"* na tabela dos dois ramos de Gentil sem dizer dual **de quê**. Corrigido para
*"o ramo dual do bilinear"*, e a tabela ganhou a linha que faltava: *o que mede* contra *o que
ordena*. Ver [[feedback-o-sujeito-da-frase]], que é o defeito irmão — ali eu punha o clássico como
sujeito, aqui ponho meia estrutura como se fosse a estrutura.
