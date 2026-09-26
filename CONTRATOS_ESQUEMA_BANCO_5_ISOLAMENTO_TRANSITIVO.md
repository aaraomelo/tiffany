# CONTRATOS DO BANCO GRANDE — 5. ISOLAMENTO TRANSITIVO (CONGELADO)

> **Estado: FECHADO — Opção T-Iso, 2026-09-26.** O isolamento por inquilino é aplicado **à tabela base da consulta** e **não** é transportado para as tabelas de junção/subconsulta. Limite medido, não bug. Sem alteração de código, sem reatestado.
>
> Precedência: `PROTOCOLO_MATRICIAL_BIG_PICKLE.md` §11–§12. **Sem interface com a `Qz`/Mat2Q**.
>
> Nenhuma alteração de código: a opção descreve o comportamento medido; as assinaturas de `banco/sql` e `pgwire` mantêm-se.

---

## A. Mapeamento do caminho actual (medido)

### A.1 Onde o isolamento é aplicado
A política de inquilino (Contrato 3, `S_RLS`) é erosionada **num único** sítio: `banco/sql.c:8953`, dentro de `varre`, sobre o `S_MATCH` **da tabela que está a ser varrida** (a tabela base da consulta). Rege `SELECT`/`UPDATE`/`DELETE` (Contrato 3).

### A.2 O que acontece numa junção
`j_carrega_direita` (`sql.c:7337-7368`) carrega a tabela da **direita** de um JOIN ou de um `IN (SELECT…)`:
- abre a tabela da direita (`usa_tabela`, `:7340`);
- percorre **todas** as linhas vivas `S_VIVO` (`:7349-7350`) e indexa-as (`:7361-7362`);
- **não verifica `S_RLS`, nem o inquilino da sessão, nem `app.tenant_id` em lado nenhum** deste laço.

O mesmo `j_carrega_direita` serve o JOIN (`:9307`) e a subconsulta `IN` (`:8885`). Consequência directa: **a tabela da direita entra na junção/subconsulta sem o filtro de inquilino da tabela base.**

### A.3 A terceira tabela (A→B→C)
O parser compõe **dois JOIN** como "a saída do primeiro é a entrada do segundo" (`:7787-7794`; segunda tabela em `j2_*`, `:7803-7807`). Cada um dos cortes chama `j_carrega_direita` sobre a sua tabela direita — nenhuma delas filtrada por inquilino.

### A.4 A erosão RLS corre depois, uma vez
A erosão (`:8953`) actua sobre o `S_MATCH` já composto. Ela limita **as linhas da tabela base que sobrevivem**, mas não reindexa nem restringe as linhas que as tabelas de junção já começaram a servir.

---

## B. Os cinco casos, medidos

| Caso | Comportamento medido | Fonte |
|------|------------------------|-------|
| **Acesso directo** (`SELECT … FROM a`) | Filtrado por inquilino. | `:8953` |
| **Relação A→B** (`a JOIN b`) | `a` filtrada; `b` **não filtrada** na junção. | `:7349-7364` |
| **Relação A→B→C** (dois JOIN) | `a` filtrada; `b` e `c` **não filtradas**. | `:7787-7807`, `:7349` |
| **Ausência de vínculo** (`IN (SELECT…)`, coluna ausente) | Célula ausente não é chave nem entra na árvore (`S_PRES`); linha sem o campo não casa. | `:7357-7362`, `:8868` |
| **Múltiplos vínculos** (várias linhas de `b` casam com uma de `a`) | Todas as linhas vivas de `b` que casam são servidas; a que for de outro inquilino **é lida** se casar pela chave não-tenant. | `:7349-7364` |

---

## C. O invariante efetivamente garantido hoje

> **O isolamento por inquilino restringe as linhas da TABELA BASE da consulta. Ele não é transportado para as tabelas de junção/subconsulta; a exposição de dados de outro inquilino depende, por isso, de a chave de junção cruzar inquilinos.**

Isto **não** é, por si só, uma fuga: a linha de `a` que sobreviveu ao filtro tem de casar com a linha de `b` pela chave do `ON`; se essa chave for o próprio identificador global (`id`), o `b` que casa é o do mesmo inquilino por construção. A **fronteira** é precisamente esta: **a segurança do join depende de a chave `ON` não cruzar inquilinos.** Isto é uma propriedade do *schema do cliente*, não do motor, e é onde a decisão tem de ser explícita.

O que o motor **não** garante hoje: que uma junção nunca exponha uma linha de outro inquilino. Essa garantia não existe e **não é simulada**.

---

## D. Decisão conceptual (isolada) — CONGELADA (Opção T-Iso)

> **T-Iso — Preservar a semântica medida:** o isolamento rege a tabela base; **não** é transitivo através de junções/subconsultas. Isto é uma **fronteira documentada**, não um defeito. A segurança de uma junção passa a ser responsabilidade do **schema do cliente** (chave `ON` que não cruza inquilinos) e, quando se precisar de isolamento transitivo garantido pelo motor, isso constitui **nova capacidade**, com contrato e medidor próprios.

Ressalva (mesma forma dos anteriores): isto fixa **o que o motor mede**, e **não** afirma que o isolamento transitivo seja desnecessário em todo o sistema. Um ERP cujas chaves de junção cruzuem inquilinos, ou que exija isolamento por omissão mesmo com chaves erradas, precisa da capacidade nova (T-Iso-B).

Opções registadas:
- **T-Iso-B (propagar a política de inquilino às tabelas de junção/subconsulta)** — **capacidade nova** de segurança. Alteraria `j_carrega_direita` para filtrar por inquilino; exigiria medidor dedicado (o caso "múltiplos vínculos" de B) e mudaria o comportamento de consultas hoje verdes. Não é equivalência; não se faz sem necessidade demonstrada.
- **T-Iso-C (assumir que a chave `ON` nunca cruza inquilinos, como invariante do schema)** — sem custo, mas **tem de ser escrito no schema do cliente**, e não deixado implícito.

Contrapartida que o contrato assume: o motor **não garante** isolamento transitivo. Quem precisar declara-o (T-Iso-B) ou garante por chave (T-Iso-C, com medidor no cliente).

## E. Medidores correspondentes

- O caminho de leitura por inquilino está medido em `banco/sql` (VERDE 98/98) e no `pgwire` (VERDE).
- O comportamento de junção **não** tem medidor dedicado de tenant hoje. Para T-Iso **não é necessário** (o motor fica intacto); para T-Iso-B seria obrigatório (o caso "múltiplos vínculos" seria o teste).

## F. Registo

- Contrato **fechado** a 2026-09-26, Opção **T-Iso**, sem alteração de código.
- Restam **2** decisões de schema: embedding (coluna `vector`) e forma do `UNIQUE`.
