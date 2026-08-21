---
name: feedback-varrer-onde-o-defeito-nao-vive-colunas
description: Duas regiões de slots sobrepunham-se nas colunas 4 a 7 — e todas as tabelas dos medidores têm três colunas.
metadata: 
  node_type: memory
  type: feedback
  originSessionId: cee0686a-c852-4d69-8fca-ca206e1fba24
  modified: 2026-08-20T23:43:52.328Z
---

No `banco/sql.c`: `S_CORPO 60..67` (o corpo declarado de cada coluna) e
`S_EXPR 64..191` (os temporários da árvore do WHERE) **sobrepunham-se em 64..67**.
Numa tabela com cinco colunas ou mais, o primeiro `SELECT` com expressão escrevia o
temporário por cima da declaração — **e ficava no disco**. Medido: uma coluna
`MORFICO(6)`, o par (3,6), voltava (1,0), que é RACIONAL. A partir daí a aritmética
daquela coluna é a álgebra errada, para sempre, e nada o diz.

Ninguém lá chegou porque **todas as tabelas dos medidores têm três colunas**.

**Why:** é [[feedback-varrer-onde-nada-pode-falhar]] com um parâmetro que eu nunca
tinha pensado como regime — o NÚMERO DE COLUNAS. O medidor do corpo da coluna
existia, media os quatro tipos, e media-os numa tabela de quatro: o defeito começa
na quinta. Um medidor pode estar certo, completo na sua tese, e nunca tocar no sítio.

**How to apply:** quando um mapa reserva regiões por índice (`BASE + i`), escrever
as fronteiras lado a lado e conferir que não se cruzam — é aritmética, não leitura.
E o medidor tem de correr no regime onde o defeito PODE viver: aqui, seis colunas.
O gume prova-o dos dois lados — repor a sobreposição faz cair, e reduzir o medidor a
três colunas faz cair também: a escolha do regime é parte da medida.

O achado veio do gume de OUTRA coisa (a fachada pqlike, que exigia que um erro
chegasse como erro). Ver [[feedback-dois-caminhos]]: os piores defeitos aparecem
quando se obriga dois caminhos a concordar.
