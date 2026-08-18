---
name: project-a-cuspide-e-o-trial
description: a cúspide é disc = 0 (as folhas colidem); τ = sign(disc) É o trial e indexa a dimensão τ+1; cada racional ancora uma cúspide e o gume é atravessá-la
metadata:
  type: project
---

**A CÚSPIDE é `disc = tr² − 4·det = 0`** — onde as duas folhas da transformada universal COLIDEM. É a mesma quantidade que em [[project-o-real-e-o-corte]] dizia «o objecto é racional, é um PONTO e não um corte»; um andar acima diz «o operador está na fronteira entre fechar e não fechar».

**τ = sign(disc) ∈ {−1, 0, +1} É O TRIAL** (a lei L_3, ordem 3), e a cúspide é o ponto do meio. O Aarão corrigiu-me duas vezes aqui: «a cúspide tem 3 pontos, isso definia passagem pela dimensão» e «a cúspide é trial, **não tem nada de instável**» — eu tinha escrito «instabilidade estrutural», que era invenção minha.

**τ indexa a DIMENSÃO**: nº de folhas reais = τ+1 ∈ {0,1,2}. Em τ=0 o traço é PAR, λ = tr/2 é inteiro, e o espaço próprio colapsa de 2 para 1. **E isto é a VERIFICAÇÃO**: τ é o sinal de um inteiro, SEM TECTO — ao contrário de medir a ordem por iteração, que precisa de um tecto à mão ([[feedback-o-tecto-do-array]]).

**Cada racional ancora uma cúspide**: `P(p,q) = [1−pq, p²; −q², 1+pq]` tem det 1, disc 0, e fixa [p:q]. Logo ℚ **é** o conjunto das cúspides ([[project-descida-racionais-pontos-fixos]]), e **o corte é o complemento delas**. O FRACTAL é a órbita: toda cúspide se alcança de [1:0] pelos geradores — a auto-semelhança É a acção do grupo —, e o passo é a mediante de Farey (que fica ENTRE; a diferença é vizinha mas é o PAI).

**O GUME É A TRAVESSIA DE τ=0.** Fora, mutar só derruba se mudar τ de lado — **e é isto que dá conteúdo ao gume que não morde** ([[feedback-o-ramo-que-nunca-corre]]): não é fraco, ficou do mesmo lado. Com passo unitário não há como saltar por cima.

**A correcção de sítio no quadro do toro**: τ=−1 tem ordens 3,4,6 (trial, Viviani, hexal) e mais nenhuma; τ=0 tem 1 e 2, e são **só ±I**; τ=+1 não tem nenhuma. Cantor/Julia é o espelho ν²=id, e a única involução de det 1 é −I: **o espelho vive NA cúspide, não dentro do toro**.

**Na continuação** ([[project-calculo-2-3]]): os polos de −log(1−mx−x²) são as folhas, e em τ=0 colidem num polo DUPLO. A base das soluções muda: {σ^k, σ†^k} → {λ^k, k·λ^k}. **O k que aparece é o mesmo k da volta t_k = k·c_k** — é a ordem do polo a mostrar-se no coeficiente.

Medido: `tests/cuspide.c` (8:0), `thm:cuspide` + `def:cuspide` + `cor:cuspide-continuacao` em geometrico.
