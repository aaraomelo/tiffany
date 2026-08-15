---
name: project-gume-automatico
description: O gume deixou de ser escrito à mão e passou a ser uma BUSCA — retirar a hipótese e procurar o objeto onde a tese também cai.
metadata:
  type: project
---

Commit `c1cb539`, `lib/linear.h` + §C37. O `eval.txt` pediu uma coisa que **não é
conteúdo, é mecanismo**:

> «um gume obrigatório em cada teorema: **se a hipótese for retirada, procurar
> automaticamente um contra-exemplo**. É fazer o motor descobrir **qual hipótese está
> carregando cada teorema**.»

`gume_matriz(n, lim, hip, tese, &contra)` varre o espaço, salta os objetos onde a hipótese
vale, e devolve o **primeiro** onde a tese também cai. Achou nos três testados: det ≠ 0 ⟹
invertível (passo 1), colunas LI ⟹ núcleo trivial (passo 1), simétrica ⟹ comuta com a
transposta (passo 4).

**E isto é o que eu vinha a fazer à mão a sessão toda** — ℤ₁₂ abeliano não exercitava a
normalidade ([[feedback-varrer-onde-nada-pode-falhar]]), ℤₘ finito fazia corpo e domínio
coincidirem. A regra que eu tinha aprendido virou função.

## O erro que interessa: o controlo do buscador

Pus o controlo como `tese = hipótese`. Mas assim **todo** objeto que falha a hipótese é
«contra-exemplo» — o buscador acha sempre e o controlo não controla nada. Um contador que
dispara sempre é igual a um contador avariado.

O controlo certo é uma **tese que vale SEMPRE** (`det A = det Aᵀ`): aí não pode existir
contra-exemplo, e **vir vazio** é o que prova que o buscador não inventa. É o mesmo padrão
do detetor de choques que precisava de um caso forjado ([[project-o-real-e-o-corte]]) — mas
aqui do lado oposto: ali fazia-se disparar, aqui garante-se que NÃO dispara.

**Regra geral que daqui sai**: um buscador de contra-exemplos precisa de DOIS controlos —
um caso onde tem de achar, e um caso onde tem de **não** achar. Só com os dois é que ele
está a decidir.

## A redundância que ensina

O filtro `if(hip(&A)) continue` é **semântico, não operacional** — medido: apagá-lo não
muda um resultado. Se o teorema é verdadeiro, `hip ⟹ tese`, logo nenhum objeto com a
hipótese verdadeira falha a tese, e a linha seguinte já os exclui. Ele **declara** que se
procura fora da hipótese, e passaria a ser operacional no dia em que o «teorema» fosse
falso — que é justamente quando queremos saber. Ver [[feedback-o-ramo-que-nunca-corre]].
