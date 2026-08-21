---
name: project-checkpoint-2026-08-21-pg
description: 21/08 — o banco fala Postgres e o psql real liga-se; o ROLLBACK desfaz pelo levantamento; e o aranha.tex fica travado.
metadata:
  type: project
---

**21/08/2026** — `7d571b7 → 2ffc454`, com push. Bateria **529 : 529, 0 falhas**.

**O BANCO FALA POSTGRES.** FEBE escrito do zero (`banco/pgwire.c`,
`lib/pgwire.h`, `banco/pgcat.h`, `banco/pgfunc.h`), sem libpq. **O psql 16.14
real liga-se** — via `docker run --rm --network host postgres:16 psql`, porque
o ERP já tinha quatro Postgres em contentores e eu tinha dado «não há cliente
real» como limite quando o limite era não ter olhado. Faz o ciclo todo: `\l`,
`\dt`, `\d`, CREATE/INSERT/UPDATE/DELETE, `count(*)`, SHOW, SET, transacções.

**TRÊS DEFEITOS QUE SÓ O CLIENTE REAL ENCONTROU** — invisíveis enquanto quem
perguntava éramos nós:
- o `RowDescription` anunciava **tudo como int4**, incluindo texto: o valor
  chegava certo (o wire é texto) e o **tipo** ia errado;
- `SQL_OUT_MAX_COLS` era **8** e o `\d` pede **13** colunas lidas por posição;
- as funções-script tinham caminho relativo e **só funcionavam no directório
  certo** — apanhado pela bateria, não pelo medidor.

**O ROLLBACK DESFAZ, E DESFAZ PELO LEVANTAMENTO.** Sondagem no psql mostrou que
ele devolvia a tag **sem desfazer nada** — perder dados sem ver um erro. A
resposta não foi inventar transacções: **escrever é quocientar**, e guardar o
par (endereço, valor anterior) é a folha. Desfaz-se lendo a pilha **ao
contrário** (chega ao `k=1`), com tecto declarado: cheia, **recusa** em vez de
desfazer metade. E o motor mede-se como realização: `∑G = |I| = 127` sobre 47
slots (`tests/pgwire.c` §W16).

**O `papers/aranha.tex` FICA TRAVADO** — ver [[feedback-o-aranha-tex-esta-travado]].
E o `cursor.txt` foi reescrito de 3588 linhas de diário para 118 de **estado e
directrizes**, para outro agente continuar.

**O que fica fora, medido e não estimado:** o ERP tem 77 modelos Prisma, 45
enums, UUID, Decimal, pgvector e RLS — **migrar o ERP não é o próximo passo**.
Ver [[project-erp-em-plataforma]].
