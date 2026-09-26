# PROTOCOLO_AUDITORIA_PRE_MIGRACAO_BANCO.md

**Agente responsável:** Big Pickle
**Data da auditoria:** 2026-09-25
**Natureza:** FORMALIZAÇÃO do estado atual e definição do contrato de transição. Nenhuma migração, nenhum DDL, nenhuma alteração de schema ou código.

---

## 0. REGRA DE OURO

Separar rigorosamente quatro coisas:

1. **estado observado** — o que existe e foi lido (arquivo, linha, teste);
2. **capacidade comprovada do motor** — o que o código + testes demonstram;
3. **estado-alvo do PostgreSQL/Prisma** — o schema e o DDL efetivamente gerados;
4. **decisão ainda pendente** — o que precisa de autorização antes de migrar.

Regras de conduta:

- Não transformar proposta em fato.
- Não transformar documentação antiga em evidência atual.
- Não inferir capacidade do motor apenas porque o PostgreSQL possui determinada funcionalidade.
- Toda afirmação técnica aponta para arquivo + linha/seção, teste, migration, schema ou artefato verificável.

---

## 1. ESCOPO

A auditoria cobre inicialmente:

```
Tenant → Cash → Product → CashSession → Order → OrderItem → Payment
```

e suas dependências diretas (Unit, Brand, Division, Section, Group, Subgroup, CustomerSupplier, TenantUser, Warehouse, Address, ModulePack).

O objetivo **não** é migrar essa fatia. O objetivo é determinar:

```
ESTADO ATUAL  →  ESTADO-ALVO
```

e identificar tudo que precisa estar resolvido antes da primeira transição.

---

## 2. FONTES PRIMÁRIAS

### Alvo

- `plataforma/tiffany-erp/erp-api/prisma/schema.prisma` (77 modelos, 45 enums; Tenant:96, Product:418, Order:675, OrderItem:711, Payment:773, Cash:966, CashSession:987)
- migrations do Prisma: `plataforma/tiffany-erp/erp-api/prisma/migrations/` (20, de `20260525220226_init` a `20260609000000_tenant_company_info`)
- `20260601070000_native_rls/migration.sql` (39 linhas, `DO $rls$`)
- `20260601080000_native_rls_exclude_assistant/migration.sql` (18 linhas)
- `plataforma/tiffany-erp/deploy/rls-native-activate.sql` (ativação: role `erp_app`)
- `plataforma/tiffany-erp/erp-api/prisma/seed-*.ts` (seed-login, seed-modules, seed-access)

### Motor

- `banco/sql.c` (motor SQL; CLI `./sql <base> "..."` em `sql.c:66-67`; main em `sql.c:16727`)
- `banco/pgwire.c` (listener FEBE/Trio PG3; bind 5432→55432 em `pgwire.c:54,72-83`)
- `lib/pgmsg.h` (mensagens PG; OIDs anunciados em `pgmsg.h:16-60`)
- `tests/pgwire.c` (medidor do motor, ~32.000 linhas, §W0–§W188; FK §W35 `4871-4999`, §W36 `5014-5200`; RLS §W187 `27030-27168`)
- `tests/sql_bash.c` (harness subprocesso)
- `tools/bateria.sh` (bateria oficial; VERDE/NEGATIVO/FALHA em `bateria.sh:4-8`)

### Registro

- `tools/atestados.txt` (registros verdes: `sql` linha 487, `pgwire` linha 389, `banco` linha 35)
- `RESULTADO_VALIDACAO_VERTICAL.md` (2026-09-24)
- `CONTRATO_VERTICAL_Tenant_Product_Order_Payment_Cash_v2.md`
- `DIAGNOSTICO_BUILD_VALIDACAO.md`
- `VALIDACAO_CAPACIDADES_VERTICAL.md`
- checkpoints de `memoria/` relacionados

**Quando documentação e código divergem, vale o código/teste; a divergência é registrada (§7, §13).**

---

## 3. MATRIZ DE COMPATIBILIDADE

Estados controlados: `COMPATÍVEL` · `PARCIAL` · `AUSENTE` · `BLOQUEADOR` · `PENDENTE` · `DRIFT` · `DESATUALIZADO`.

| Feature | PostgreSQL/Prisma | Tiffany atual | Evidência | Estado | Decisão necessária |
|---|---|---|---|---|---|
| UUID PK/FK | `uuid`, `String @db.Uuid`, geração client-side | Sem tipo UUID; serial→INTEIRO sem auto; ids podem viver como TEXTO | zero `uuid` em `sql.c`; `sql.c:3251-3254` (SERIAL/BIGSERIAL→INTEIRO) | `AUSENTE` | UUID→TEXT com geração client-side (§4.1) |
| numeric(18,4) | `numeric(18,4)` (OID 1700) | DECIMAL/NUMERIC→RACIONAL; `(p,s)` declarado não muda o corpo | `sql.c:3249-3250`, `3230-3232`; OID 1700 em `pgmsg.h:46-60` | `PARCIAL` | confirmar escala monetária/quantização (§4.2) |
| Enum nativo | `CREATE TYPE ... AS ENUM` (41 no init) | Coluna fica TEXTO; `CREATE TYPE AS ENUM` = coro de membership; `ALTER TYPE ADD VALUE` append | `sql.c:15505-15513`, `15374-15396` | `PARCIAL` | decidir contrato igualdade vs tipo (§4.3) |
| JSONB | `jsonb` nativo | Coluna→TEXTO; sem operadores JSONB | `sql.c:3227`, `13611-13625` | `PARCIAL` (storage sim, operadores não) | aceitar TEXT na fatia (§4.4) |
| timestamp(3) | `timestamp(3) without time zone`, ms | `CORPO_DATA` = epoch em segundos | `sql.c:422-431` | `PENDENTE` | definir contrato temporal (§4.5) |
| RLS tenant isolation | `tenant_isolation` USING+WITH CHECK (`native_rls:30-36`); `tenant_isolation_tenant` para Tenant | filtro **pós-WHERE** em SELECT; INSERT/UPDATE/DELETE sem RLS; WITH CHECK ausente; `S_RLS/S_RLSCOL` usados, `S_TENANT/S_BYPASS` inativos | slots `sql.c:927-930`; filtro `sql.c:8942-8953`; leitura `pgcat_valor` `sql.c:8956-8961`; só `mem_le(S_RLS)` em `sql.c:8953` | `PARCIAL` | decidir SELECT vs SELECT+IUD (§5) |
| FK ON DELETE CASCADE | universal nas FKs de tenant (`init` 1219–1468) | implementado (RESTRICT/CASCADE/SET NULL) | parser `sql.c:3360-3390`; `fk_propaga` `sql.c:6205-6214`; dois passos `sql.c:6221-6223`; teste §W35/§W36 | `COMPATÍVEL` | — |
| FK ON DELETE SET NULL | `init:1300,1303,1306,1369...` | implementado | `fk_propaga` `sql.c:6205-6214`; §W36 | `COMPATÍVEL` | — |
| FK ON DELETE RESTRICT | `init:1252,1312,1363,1366...` | implementado | `fk_propaga` `sql.c:6205-6214`; §W35/§W36 | `COMPATÍVEL` | — |
| FK ON UPDATE CASCADE | universal em `init` | implementado, modo independente de DELETE | `sql.c:3360-3390`; §W36 | `COMPATÍVEL` | separado do contrato ON DELETE |
| UNIQUE como constraint | `PRIMARY KEY`/`UNIQUE` em CREATE TABLE; `ADD CONSTRAINT ... UNIQUE` | enforced pela rede/árvore | `sql.c:3154-3155,3286-3289,3386`; `3951-3960`; `6003-6033`; ADD CONSTRAINT `15401-15445` | `COMPATÍVEL` | usar esta forma na migração (§7) |
| `CREATE UNIQUE INDEX` (forma do Prisma) | `@@unique(x)` → `CREATE UNIQUE INDEX` | flag UNIQUE **descartada** (`(void)unico`) | `sql.c:15495` | `AUSENTE` | converter para UNIQUE CONSTRAINT (§7) |
| Índice composto | justo e composto no PG | desce só pela 1ª coluna | `sql.c:15484-15496` | `PARCIAL` | medir desempenho/plano (§7) |
| vector embedding | `vector(1024)` declarado; `vector(768)` real no DDL | aceito como TEXTO com warning | `sql.c:3243-3248` | `BLOQUEADOR` (semântica vetorial) | excluir do contrato operacional (§8) |
| Delta vector 1024↔768 | schema 1024 vs DDL 768 | n/a | schema `438-440`; init `321` vs nenhum ALTER de resize | `DRIFT` | formalizar, não corrigir agora (§9) |
| Bateria/registro | n/a | atestados verdes vs resultado com falhas | `atestados.txt:487,389,35` vs `RESULTADO_VALIDACAO_VERTICAL.md:55-76` | `BLOQUEADOR` | explicar divergência antes de migrar (§12) |

---

## 4. LACUNAS CONFIRMADAS

### 4.1 UUID

- Motor não possui UUID nativo (zero ocorrências de `uuid` em `sql.c`; `SERIAL/BIGSERIAL`→INTEIRO, `sql.c:3251-3254`).
- Decisão operacional atual (pendente de ratificação):

```
UUID do domínio  →  TEXT no Tiffany
```

com geração client-side (o próprio alvo já gera os UUIDs no cliente Prisma — o DDL do PG **não** tem `gen_random_uuid()`; zero ocorrências em todas as migrations).
- **Registro:** a premissa anterior — *"motor usa UUID ou ISA auto-increment"* — está **incorreta** e deve ser removida do contrato antigo. (O contrato v2, §11 gap "UUID PK/FK — PROPOSTA", não demonstra a capacidade; esta auditoria confirma que ela é `AUSENTE`.)
- **Proibido nesta etapa:** implementar UUID.

### 4.2 NUMERIC(18,4)

- Representação atual: racional exato (`DECIMAL/NUMERIC`→`RACIONAL`, `sql.c:3249-3250`).
- `(p,s)` é declarado, mas **não muda o corpo** (`sql.c:3230-3232`): não foi verificado arredondamento/quantização na escala 4.
- PG anuncia OID NUMERIC 1700 no fio (`pgmsg.h:46-60`) — a leitura via wire precisa de teste.
- **Decisão pendente:** confirmar escala monetária efetivamente aplicada antes de declarar compatibilidade monetária.

### 4.3 ENUM

- `CREATE TYPE ... AS ENUM` no Tiffany declara um coro de elementos; **a coluna permanece TEXTO** e a validade é membership (`sql.c:15505-15513`). `ALTER TYPE ... ADD VALUE` acrescenta elemento (`sql.c:15374-15396`).
- Distinção formal:

```
PostgreSQL ENUM (tipo real)  ≠  TEXT + restrição/membership do Tiffany
```

- Registra-se o comportamento efetivamente implementado; não se chama de "ENUM nativo".

### 4.4 JSONB

- `JSON/JSONB`→TEXTO (`sql.c:3227`); o token `json` só aparece entre operadores no parser (`sql.c:13611-13625`); não há operadores de extração.
- Não é equivalência completa de semântica PostgreSQL.

### 4.5 TIMESTAMP(3)

- PostgreSQL/Prisma: precisão de milissegundos. `CORPO_DATA`: epoch em **segundos** (`sql.c:422-431`).
- Existe perda potencial de precisão na borda.
- **Classificação:** `PENDENTE` até definir o contrato temporal.

---

## 5. RLS — ISOLAMENTO DE LEITURA POR TENANT

Seção de maior atenção. Estado observado:

```
RLS Tiffany = SELECT-only
```

- Aplicação: filtro de coluna aplicado **após** o WHERE do cliente, limpando `S_MATCH` (`sql.c:8942-8953`).
- Origem do tenant: `pgcat_valor("app.tenant_id")`; bypass: `pgcat_valor("app.bypass_rls")=='on'` (`sql.c:8956-8961`).
- Slots: `S_RLS`, `S_RLSCOL` (usados); `S_TENANT`, `S_BYPASS` (definidos em `sql.c:927-930`, **nunca lidos/escritos** — só `mem_le(S_RLS)` em `sql.c:8953`).
- Próprias das escritas:
  - **INSERT:** sem política WITH CHECK no motor (o PG alvo tem WITH CHECK idêntico ao USING, `native_rls:30-36`).
  - **UPDATE / DELETE:** sem aplicação equivalente verificada.
- Não se chama isso de "RLS implementado"; usa-se **"isolamento de leitura por tenant"**, que é o alcance comprovado.
- Teste existente §W187 já demonstra o isolamento de leitura (acme vê 2, globex vê 1, `assistant_log` vê 3 — `tests/pgwire.c:27163`).
- **Decisão de contrato pendente (não é decisão do código):**

```
SELECT
```
ou
```
SELECT + INSERT + UPDATE + DELETE
```

---

## 6. FOREIGN KEYS

- `ON DELETE CASCADE` e `ON DELETE SET NULL`: **implementados e testados** (parser em `sql.c:3360-3390`; `fk_propaga` RESTRICT/CASCADE/SET NULL em `sql.c:6205-6214`; dois passos "pergunta tudo, age depois" em `sql.c:6221-6223`; §W35 `tests/pgwire.c:4871-4999`, §W36 `5014-5200`).
- `ON UPDATE CASCADE`: tratado **separadamente** (modo independente de DELETE no mesmo octeto, `sql.c:3360-3390`; §W36) — também testado.
- **Correção de documentação anterior:** `CONTRATO v2 §10/§11` e `VALIDACAO_CAPACIDADES_VERTICAL.md:130-131` declaravam SET NULL / ON UPDATE CASCADE como AUSENTES. O código e os testes os demonstram. A documentação fica marcada como `DESATUALIZADO` (§0: prevalece código/teste).
- **Nota:** pela régua da casa, `COMPATÍVEL` aqui é por código + teste presente; a execução da bateria completa continua dependente do §12.
- **Característica do alvo:** `Order.warehouseId` existe como coluna **sem FK** (schema.prisma:682; `init:428`, sem `Order_warehouseId_fkey`). Trata-se de característica do schema-alvo, não de bug do motor.

---

## 7. UNIQUE E ÍNDICES

Separar dois contratos:

### UNIQUE como constraint
`PRIMARY KEY`/`UNIQUE` na CREATE TABLE e `ADD CONSTRAINT ... UNIQUE/PRIMARY KEY/FOREIGN KEY` são suportados e enforced (`sql.c:3154-3155,3286-3289,3386`; `3951-3960`; `6003-6033`; `15401-15445`). Estado `COMPATÍVEL`.

### CREATE UNIQUE INDEX
Não assumir equivalência. O parser **descarta a semântica UNIQUE** de `CREATE UNIQUE INDEX` (`(void)unico;` em `sql.c:15495`). Estado `AUSENTE`.

Consequência para a migração: o Prisma gera `@@unique([tenantId, sku])` como `CREATE UNIQUE INDEX "Product_tenantId_sku_key" ...`. Para a fatia Tiffany, a forma compatível atualmente conhecida e semanticamente equivalente é **UNIQUE CONSTRAINT** — a conversão deve ser explícita na migração mínima.

### Índice composto
`CREATE INDEX ON t (a,b)` desce pela árvore **somente da primeira coluna**; as demais filtram depois (`sql.c:15484-15496`). Estado `PARCIAL` — a equivalência de plano/performance precisa de medição, não de afirmação.

---

## 8. VECTOR

```
vector(1024) → TEXT (com warning)
```
(`sql.c:3243-3248`)

**Classificação:** `BLOQUEADOR` para qualquer migração que exija semântica vetorial real. O contrato operacional da fatia pode prosseguir **sem** o campo embedding. Proibido implementar busca vetorial nesta etapa.

---

## 9. DRIFT 1024 ↔ 768

| Item | Valor |
|---|---|
| Schema Prisma declara | `embedding Unsupported("vector(1024)")` (6 campos: CustomerSupplier:301, Product:438, ProductVariant:499, SupplierProduct:1428, AssistantMemory:1686, AssistantMessage:1725) |
| DDL executado (init) | `vector(768)` em CustomerSupplier (`init:222`) e Product (`init:321`) |
| Migrations posteriores | Nenhuma redimensiona (`grep ALTER COLUMN ... embedding` → sem correspondências) |
| Tabelas nascidas depois | `vector(1024)` (assistant_messages:78, assistant_memories:46, supplier_integration:49, product_variants:22) |

Formalizado como `DRIFT` independente, na origem e sem migration corretiva. **Não corrigir agora** — apenas registrar o conflito.

---

## 10. TENANT TRANSITIVO

Entidades **sem `tenantId` próprio** (21 no schema): OrderItem:711, CashOperation:1016, CashPhysicalClose:1031, ServiceOrderPart:1188, ServiceOrderLabor:1203, BudgetItem:1262, WalletTransaction:855, AccountsReceivableInstallment:1080, InvoiceEvent:1539, WebhookDelivery:1586, Module/ModulePack/ModulePackItem, City, Address, Installation, PasswordReset, AccessRule, AssistantMessage.

No PG alvo, o `native_rls` só cobre tabelas com coluna `tenantId` (`native_rls:17-25`) — portanto essas 21 não recebem política.

Não se conclui automaticamente que estão inseguras. Registra-se a questão formal:

```
isolamento transitivo  versus  isolamento explícito por tabela
```

A decisão será tomada posteriormente a partir do modelo de autorização.

---

## 11. INVARIANTES PRÉ-MIGRAÇÃO

### Persistência

```
R_PG = R_T
```
continua `PENDENTE` (sem comparação real; registros em `CONTRATO_v2:560-577,627-646`, `RESULTADO_VALIDACAO_VERTICAL.md:7`, `DIAGNOSTICO:165`). Não se declara equivalência sem comparação real.

### RLS
O alcance efetivo (isolamento de leitura por tenant; escopo SELECT) permanece documentado — §5.

### Integridade referencial
As regras FK observadas (CASCADE/SET NULL/RESTRICT, ON UPDATE CASCADE) devem ser preservadas na transição quando fizerem parte do contrato.

### JEV
A migração não deve quebrar o contrato JEV já fechado (JEV vive fora de `banco/` — `conecthus/backends/wasm/jev`; fechado em `c1ec973c`). Não se reabre a implementação; declara-se a preservação como requisito.

---

## 12. BLOQUEADOR — ESTADO DE BATERIA INCONSISTENTE

| Origem | Afirmação |
|---|---|
| `tools/atestados.txt:487,389,35` | `sql` → `0` (verde), `pgwire` → `0` (verde), `banco` → `0` (verde). Registro histórico; reexecução recente não comprovada nesta auditoria. |
| `RESULTADO_VALIDACAO_VERTICAL.md:55-76` (2026-09-24) | 451/568 VERDE, **117 FALHAS**; `banco/sql.c` com FALHA `exit 134` (SIGABRT) no runner GitHub (ubuntu). |

Difere em: quantidade de testes, veredicto, ambiente (runner CI vs registro local), janela de tempo, estado do código. Não há commit assinalado em `atestados.txt` que prove a mesma base.

Regras:

- Não se escolhe arbitrariamente um dos dois.
- Não se declara a suíte verde.
- Não se inicia migração enquanto a discrepância não estiver explicada.

```
estado de validação global = NÃO CONSOLIDADO
```

---

## 13. ESTADO-ALVO

Estado-alvo **não significa estado implementado**.

| Feature | Estado atual (motor) | Estado-alvo (PG/Prisma) | Gap | Bloqueador? | Ação futura |
|---|---|---|---|---|---|
| UUID | AUSENTE | `uuid`, ids client-side | TEXTO vs uuid | não | decidir UUID→TEXT |
| numeric(18,4) | PARCIAL (RACIONAL) | `numeric(18,4)` | escala/quantização | não (verificar) | teste OID 1700 + escala |
| Enum | PARCIAL (TEXT+coro) | enum real | igualdade/ordenação | não | definir contrato de uso |
| JSONB | PARCIAL (TEXT) | `jsonb` | operadores | não (só storage na fatia) | documentar alcance |
| timestamp(3) | PENDENTE (segundos) | ms | precisão | não | contrato temporal |
| RLS | PARCIAL (SELECT) | USING+WITH CHECK | IUD ausentes | **sim** (se contrato SELECT+IUD) | decisão de contrato |
| FK Delete/Update | COMPATÍVEL | CASCADE/SET NULL/RESTRICT | — | não | preservar |
| UNIQUE constraint | COMPATÍVEL | — | — | não | usar nas tabelas |
| UNIQUE INDEX | AUSENTE | `@@unique` | forma | **sim** (na fatia) | converter em CONSTRAINT |
| Índice composto | PARCIAL | pleno | 1ª coluna | não | medir |
| vector | BLOQUEADOR | `vector(...)` | semântica vetorial | sim (se exigido) | excluir da fatia |
| drift 1024↔768 | n/a | DDL 768 vs schema 1024 | pendente de decisão | **sim** (no alvo) | formalizar; decidir depois |
| Bateria/registro | NÃO CONSOLIDADO | — | §12 | **sim** | explicar antes de migrar |

---

## 14. ORDEM DA FUTURA MIGRAÇÃO (proposta, NÃO executada)

```
resolver validação (§12)
→ resolver drift (§9)
→ fechar contrato (§5 RLS, §4 decisões)
→ migration mínima (fatia 7 modelos + dependências diretas)
→ comparação PG/Tiffany (R_PG = R_T)
→ RLS no alcance decidido
→ JEV/regressão
```

A ordem definitiva só será aprovada depois que os bloqueadores forem resolvidos.

---

## 15. PROIBIÇÕES (ESTA ETAPA)

- não executar DDL;
- não criar migration;
- não alterar Prisma;
- não alterar `banco/sql.c`;
- não alterar `banco/pgwire.c`;
- não alterar RLS;
- não corrigir drift;
- não criar UUID;
- não implementar vector;
- não alterar testes existentes;
- não apagar falhas;
- não declarar a bateria global verde;
- não transformar proposta em contrato.

---

## 16. ENTREGÁVEL FINAL

### A. Estado atual consolidado

Motor Tiffany com 8 corpos (INTEIRO, RACIONAL–m=6 famílias, BOOLEANO, TEXTO, DATA), UUID ausente, JSON→TEXT, enum=TEXT+coro, timestamp em segundos, RLS = isolamento de leitura SELECT-only, FK completos (CASCADE/SET NULL/RESTRICT + ON UPDATE), UNIQUE só via constraint, `CREATE UNIQUE INDEX` descartado, índice composto pela 1ª coluna, vector→TEXT, transações com diário de undo idempotente, storage por arquivo-tabela `<base>__<nome>.mem`, FEBE PG3 Simple+Extended. Nenhuma tabela de commerce carregada. Bateria: `NÃO CONSOLIDADO` (§12).

### B. Estado-alvo

Fatia `Tenant→Cash→Product→CashSession→Order→OrderItem→Payment` + dependências diretas, sobre o schema Prisma de 77 modelos/45 enums, RLS `tenant_isolation` (USING+WITH CHECK) nas tabelas com `tenantId`, UUID client-side, `numeric(18,4)`, `timestamp(3)`, JSONB, FK CASCADE/SET NULL/RESTRICT com ON UPDATE CASCADE, índices e uniques `tenantId`-prefixados — tudo **como contrato a comprovar**, não como estado implementado.

### C. Gaps e bloqueadores

- Bloqueadores: estado de bateria inconsistente (§12); RLS além de SELECT (§5); semântica vetorial (§8); drift 1024↔768 no alvo (§9).
- Gaps abertos: UUID→TEXT (§4.1); escala numeric (§4.2); enum (§4.3); JSONB operadores (§4.4); timestamp ms (§4.5); `CREATE UNIQUE INDEX` (§7); índice composto (§7).

### D. Decisões necessárias

1. Ratificar UUID→TEXT com geração client-side.
2. Contrato temporal (epoch segundos vs timestamp(3) ms).
3. Contrato monetário (escala/quantização numeric(18,4)).
4. Alcance RLS: `SELECT` ou `SELECT+INSERT+UPDATE+DELETE`.
5. Isolamento transitivo vs explícito para entidades sem `tenantId`.
6. Destino do campo `embedding` (excluir ou tratar como TEXT).
7. Forma do UNIQUE na migração (CONSTRAINT, não INDEX).
8. Explicação e resolução da discrepância de bateria (§12).

---

## 18. CHECKPOINT FINAL — CONTRATOS DE SCHEMA 2026-09-26

**Situação:** auditoria de 7 potenciais incompatibilidades de schema concluída. Todos os 7 contratos fechados sem alterar o núcleo do motor.

### Tabela de contratos

| # | Contrato | Resultado | Código alterado |
|---|----------|-----------|-----------------|
| 1 | Monetário | DECIMAL/NUMERIC/MONEY → CORPO_RACIONAL, (p,s) como metadado | 0 |
| 2 | Temporal | T-A congelado, DATA=epoch seg, TIMESTAMP(3) sem ms | 0 |
| 3 | RLS | SELECT/UPDATE/DELETE medidos; INSERT documentado como lacuna | 0 |
| 4 | UUID→TEXT | CORPO_TEXTO, unicidade por valor | 0 |
| 5 | Isolamento transitivo | Fronteira documentada: depende do schema do cliente | 0 |
| 6 | Embedding | vector/halfvec/tsvector → TEXT, sem operações vetoriais | 0 |
| 7 | UNIQUE | Unicidade por coluna, NULL ilimitado, sem composto | 0 |

### Fronteiras de capacidade registradas

- RLS INSERT — fora do mecanismo atual, documentado como lacuna medida
- UNIQUE composto — capacidade ausente, futura extensão
- Operações vetoriais (embedding) — sem implementação, sem bug
- Isolamento transitivo via JOIN — depende da chave ON, propriedade do schema

### Versionamento e sincronização

- Commit: `2af50a48` (`docs: close schema semantic contracts audit`)
- 6 arquivos novos, 443 linhas de documentação
- Patria: 6/6 contratos presentes, hash MATCH (md5sum verificado)
- `git diff origin/master` vazio no momento do commit
- Nenhum arquivo de motor (`banco/sql.c`, `banco/banco.c`, `lib/`) entrou no commit

### Registro

A auditoria demonstrou que a resposta a "O que precisamos mudar?" foi:

**"O que o sistema já garante, onde estão suas fronteiras e como sabemos que continuará garantindo isso?"**

Isso constitui o produto principal desta etapa: conhecimento verificável sobre a semântica do sistema, com contratos explícitos e medidores. A investigação Mat2Q permanece como etapa separada e posterior.