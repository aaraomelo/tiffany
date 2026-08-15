---
name: project-teoria-dos-numeros
description: O andar de teoria dos números na assistente — os 17 exercícios do eval.txt, e a descoberta que os organiza: Euclides = MDC = Bézout = FC é a mesma órbita.
metadata:
  type: project
---

Os dezassete exercícios de teoria dos números do `eval.txt` correm na assistente
(`numeros` dá o índice, `numeros 5` ou `lema de euclides` corre o exercício), commit
`7cadf01`, `lib/numeros.h` + §C32. Zero doubles.

**A frase que organiza o andar, e é descoberta e não resumo:**

> Euclides = MDC = Bézout = FC — «diferentes saídas da MESMA órbita»

É literal. A descida `a = bq + r` produz, do **mesmo rastro**: o gcd (o último resto não
nulo), os coeficientes de Bézout (subindo a cadeia) e os termos da fração contínua (os
quocientes). Por isso a lib nova é pequena — o `iz_gcd` já era o Euclides estendido e o
`nt_fatora` já dava a fatoração. Faltava só a FC dos racionais, o inverso modular, o φ, o
μ e o Chinês. É a [[feedback-a-base-ja-existe]] a funcionar a favor, pela primeira vez.

**O que este andar ensina sobre a casa**: a mesma órbita já estava em três sítios sem se
saber — no inversor, no `lado` da cifra, no Euclides do gcd. Liga ao
[[project-o-real-e-o-corte]], onde a FC é uma das quatro portas.

**A asserção minha que caiu, e estava certa a cair**: escrevi «todo convergente é a melhor
aproximação do seu denominador» e o medidor derrubou — sempre em `j = 0`. O convergente de
ordem zero é `⌊x⌋/1`, e para 5/3 ≈ 1,667 o mais perto com denominador 1 é **2**, não 1.
Não alarguei o enunciado para o salvar: nomeei a exceção e medi-a nos dois sentidos — de
ordem ≥ 1 zero falhas, e a de ordem 0 falha **exatamente** quando a parte fracionária passa
de 1/2 (547 falham, 1020 não, critério certo nos 1567). É o padrão que ele quer: a exceção
não se esconde, mede-se.

Os gumes ficaram todos onde ele os pediu: o caso com `a|c` sem `a|b`; o lema de Euclides a
FALHAR em 1223 casos com composto; Fermat a falhar em n = 15; o Chinês recusado sem
coprimalidade; e a família da diofantina medida como **completa** — «descreva todas» não é
gerar algumas, é não deixar nenhuma escapar.

E o que ele previu no fim: «o andar vai começar a revelar sozinho as conexões com inversão
de Möbius, funções multiplicativas, Euler, primos e Dirichlet». O μ já lá está com a
identidade do cancelamento; a inversão de Möbius propriamente dita ainda não.
