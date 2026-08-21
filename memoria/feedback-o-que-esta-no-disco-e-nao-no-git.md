---
name: feedback-o-que-esta-no-disco-e-nao-no-git
description: A bateria pode estar verde sobre uma árvore que o git não reproduz — três headers do banco viviam só no disco desta máquina.
metadata: 
  node_type: memory
  type: feedback
  originSessionId: cee0686a-c852-4d69-8fca-ca206e1fba24
  modified: 2026-08-20T23:43:36.965Z
---

Ao preparar um checkpoint: `banco/sql.c` está na bateria, inclui `banco/sql_api.h`,
e esse ficheiro **não existia no git**. O `.gitignore` tinha `banco/*` com excepção
para `.c`, `.sql`, `.sh`, `.txt` — e não para `.h`. Os três headers dos Trios
PG1–PG4, declarados FECHADOS, viviam só no disco desta máquina.

**A bateria estava verde sobre uma árvore que o git não reproduz.** Aqui compila;
num clone não existe; e nada falha deste lado.

**Why:** é a órfã do [[feedback-assercoes-vazias]] noutro andar. O `#include` é uma
REFERÊNCIA, e uma referência cujo alvo não está no repositório é exactamente o
`\ref` sem `\label` que o `tests/refs.c` já persegue: o compilador não se queixa,
porque o disco tem o ficheiro. O verde mede a máquina, não a obra.

**How to apply:** antes de declarar um trio fechado, exportar o que o git TEM e
compilar a partir daí — `git ls-files -z | tar --null -T - -cf -` para uma pasta
limpa. E a medida ficou permanente em `tests/refs.c` §R5: todo `#include "x"` de um
ficheiro rastreado aponta para um ficheiro que o git também tem, com controlo
negativo.

E o costume do lado da ferramenta: a §R5 nasceu a acusar **88** e 85 eram falso
positivo meu — `banco/../lib/disco.h` não é `lib/disco.h` sem colapsar os `X/..`.
Normalizado, são 5. Duas réguas independentes (C e Python) deram o mesmo número
antes de eu acreditar nele. Ver [[feedback-a-definicao-do-extractor]].
