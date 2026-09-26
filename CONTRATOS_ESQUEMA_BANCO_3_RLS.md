# CONTRATOS DO BANCO GRANDE — 3. RLS ALÉM DE `SELECT` (CONGELADO)

> **Estado: FECHADO — Opção R-A, 2026-09-26.** A política rege `SELECT`, `UPDATE` e `DELETE`; o `INSERT` não é regido. Congela-se **a semântica medida**; não se introduz `WITH CHECK` nem qualquer capacidade nova.
>
> Precedência: `PROTOCOLO_MATRICIAL_BIG_PICKLE.md` §11–§12. **Não há interface com a `Qz`/Mat2Q**: é um contrato de schema.
>
> Nenhuma alteração de código, nenhum reatestado: R-A descreve o comportamento já medido, logo as assinaturas de `banco/sql` e `pgwire` mantêm-se.

---

## A. O que o motor faz hoje (medido, não presumido)

### A.1 A política é uma erosão do campo visível
`banco/sql.c:913-926`: o isolamento é descrito como a **erosão** do `S_MATCH` (o campo das linhas que passaram o `WHERE`), e não como maquinaria de filtros. O `S_RLS` guarda a marca, `S_RLSCOL` a coluna que isola, `S_TENANT` o inquilino corrente, `S_BYPASS` a excepção.

### A.2 Onde a política é consultada
`S_RLS` é lido em **um único** sítio: `sql.c:8953`, dentro de `varre` — a função que serve **as três** acções:
- `ACAO_MARCA` → `SELECT`
- `ACAO_SET` → `UPDATE`
- `ACAO_APAGA` → `DELETE`
- (o `INSERT` segue outro caminho, `sql_insere`, e não passa por `varre`.)

A erosão corre **depois** do `WHERE` do cliente e **antes** de a resposta sair (`:8942`), e filtra por inquilino usando `app.tenant_id` (`:8959`).

### A.3 A evidência decisiva: `UPDATE` e `DELETE` também são regidos
O laço de escrita em `aplica_diario` (`sql.c:6098-6121`) emite, **para cada linha**, o teste `S_MATCH AND S_BITM` e só escreve/apaga quando é verdadeiro. Como `S_MATCH` é **precisamente** o campo que a política erodiu, a política governa também `UPDATE` e `DELETE` — não apenas a leitura. O mesmo vale para o `μ` (`:6023`) e para a escrita do `SET` (`:6052`), que saltam linhas não marcadas.

### A.4 As excepções e recusas que já existem
- `bypass` explícito: `SET app.bypass_rls = 'on'` salta a erosão (`:8961`). É **declarado**, nunca implícito.
- **Linha sem `tenantId` não passa** (`:8969-8971`): a ausência do campo não é do inquilino que pergunta.
- `WITH CHECK`/`USING` não são implementados; a política é um `USING` de leitura propagado às escritas (A.3), não um `WITH CHECK` de inserção.

---

## B. O que o medido NÃO cobre (limites explícitos)

1. **`INSERT` não é regido**: como não passa por `varre`, a política não limita o que um inquilino pode inserir — nem impede inserir uma linha cujo `tenantId` seja de outro. Esta é uma **ausência declarada**, medida, não um bug escondido.
2. **A coluna que isola é `tenantId` pelo nome** (`:15331-15333`): se a tabela não tiver essa coluna, a política **não liga** (`:15342-15344`).
3. **A semântica é "a linha tem de ser do inquilino da sessão"**, com base no valor bruto (`S_PRES` + `celula_qz` do campo). Não há predicados richer, nem `USING` composto, nem políticas por operação.

---

## C. A decisão conceptual (isolada) — CONGELADA (Opção R-A)

> **R-A — RLS preservado conforme a semântica medida:** a política existente rege `SELECT`, `UPDATE` e `DELETE`; `INSERT` permanece fora do mecanismo actual. A extensão ao `INSERT`, caso desejada, constitui **nova capacidade** e deverá possuir **contrato e medidor próprios**.

Ressalva (mesma forma exigida no temporal): isto é **preservação do que o motor mede e garante**, não um juízo de que o modelo do Postgres deva ser copiado. Em particular, **não** se moderniza a política para `USING`/`WITH CHECK` completas só porque o PostgreSQL as tem: isso introduziria semântica nova antes de existir necessidade demonstrada.

Opções registadas:
- **R-B (estender a `INSERT` via `WITH CHECK`)** — capacidade nova (validar `tenantId` à inserção). Exigiria medidor novo; não é equivalência.
- **R-C (políticas `USING`/`WITH CHECK` completas, por operação)** — reescrita de âmbito largo; sem evidência de necessidade.

Contrapartida que o contrato assume: o `INSERT` cross-tenant **não é bloqueado** por esta política. Um sistema que precise de o impedir tem de o declarar como capacidade separada (R-B), com o seu medidor.

## D. Medidores correspondentes

O isolamento é medido no percurso de leitura por `banco/sql` (que está VERDE 98/98). A propagação a `UPDATE`/`DELETE` segue da evidência estrutural de A.3 (o mesmo `S_MATCH` governa o laço de escrita). Nenhum medidor novo é necessário para R-A; R-B exigiria um teste dedicado de `INSERT` cross-tenant.

## E. Registo

- Contrato **fechado** a 2026-09-26, Opção **R-A**, sem alteração de código.
- Restam **4** decisões de schema: `UUID→TEXT`, isolamento transitivo, embedding e forma do `UNIQUE`.
