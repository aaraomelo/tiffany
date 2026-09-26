# CONTRATOS DO BANCO GRANDE — 6. EMBEDDING (CONGELADO)

> **Estado: FECHADO — Opção EV-A, 2026-09-26.** `vector`/`halfvec`/`tsvector` guardam como TEXT (`CORPO_TEXTO`). Nenhuma operação semântica de vetor. Sem alteração de código, sem reatestado.
>
> Precedência: `PROTOCOLO_MATRICIAL_BIG_PICKLE.md` §11–§12. Sem interface com Qz/Mat2Q: contrato de schema.
>
> Nenhuma alteração de código: EV-A descreve comportamento já medido; assinaturas de `banco/sql` e `pgwire` mantêm-se.

---

## A. O que o motor faz hoje (medido)

### A.1 vector / halfvec / tsvector são TEXT
`banco/sql.c:3243-3248` — os três tipos entram na mesma lista de `BYTEA`/`BLOB`/`TEXT` e recebem `CORPO_TEXTO`. A coluna guarda o endereço no pool; a cadeia vive na gaveta.

### A.2 Aviso documentado
O motor imprime: *"a coluna é X, de uma extensão não carregada --- ela GUARDA, e as operações dela não existem aqui"*. Não rejeita a coluna. Não valida formato de vetor. Não oferece operadores de distância, HNSW ou GIN.

### A.3 TEXT[] segue o mesmo princípio
`sql.c:3258-3267` — array guarda a cadeia com elementos; operações de array não existem.

---

## B. Invariantes que permanecem verdadeiros

1. `vector`/`halfvec`/`tsvector`/`TEXT[]` → `CORPO_TEXTO`.
2. Nenhuma operação semântica existente no motor para estes tipos.
3. Aviso impresso no CREATE; não rejeita a coluna.
4. Guardar não é saber operar — princípio consistente com UUID→TEXT.

---

## C. A decisão conceptual (isolada) — CONGELADA (Opção EV-A)

> **EV-A — Preservar a semântica medida:** `vector`/`halfvec`/`tsvector` são TEXT com semântica de valor; o motor não oferece operações vetoriais e avisa explicitamente.

Opções registadas:
- **EV-B (operadores de distância L2/cosine)** — capacidade nova; sem necessidade demonstrada.
- **EV-C (índice HNSW/GIN)** — reescrita de âmbito largo, sem necessidade demonstrada.
- **EV-D (tipo vector nativo 16 bytes)** — sem necessidade demonstrada.

Contrapartida: o motor **não garante** que embedding seja válido semanticamente. Consumidor que dependa de operações vetoriais tem de verificar ou declarar EV-B/C/D.

---

## D. Medidores correspondentes

Testes da bateria `VALIDACAO_CAPACIDADES_VERTICAL.md` itens #1–#7 (vector/halfvec/tsvector). Nenhum medidor novo para EV-A; EV-B/C/D exigiriam medidores próprios.

---

## E. Registo

- Contrato **fechado** a 2026-09-26, Opção **EV-A**, sem alteração de código.
- Restavam **1** decisão de schema: forma do `UNIQUE` (agora fechada como contrato 7).
