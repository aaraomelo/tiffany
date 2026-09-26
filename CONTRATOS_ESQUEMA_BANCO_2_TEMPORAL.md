# CONTRATOS DO BANCO GRANDE — 2. TEMPORAL (CONGELADO)

> **Estado: FECHADO — Opção T-A, 2026-09-26.** Segundos desde 1970, sem fuso, sem sub-segundo. Nenhuma alteração de código, nenhum reatestado.
>
> Precedência: `PROTOCOLO_MATRICIAL_BIG_PICKLE.md` §11–§12. Este documento **não migra nada** — regista o estado medido e a decisão.
>
> Relevância: é o único contrato pendente com **impacto directo já observado em `banco/sql`** (que está VERDE 98/98, exit 0, selo `f1320f71b848d4f8`). A Opção T-A não lhe toca, logo a assinatura mantém-se.
>
> **Este contrato não tem interface com a `Qz`/Mat2Q.** É um contrato de schema, como os restantes cinco.

---

## A. Estado actual (medido, não presumido)

### A.1 A data é uma CONTAGEM, não um calendário
`banco/sql.c:422-431`:
```
* ── A DATA: o instante é uma CONTAGEM, e por isso é um índice como os outros.
* Não se guarda um calendário: guarda-se quantos passos desde a origem … o passo é
* o segundo e a origem é 1970 … a contagem precisa de MAIS UM ANDAR: 2^16 segundos
* são dezoito horas, e 2^32 são cento e trinta anos. A dobra volta a duplicar a
* largura, e o plano S_ALTO2 é o σ do andar seguinte.
#define CORPO_DATA     7
```
- Unidade: **segundo**. Origem: **1970-01-01**. Sem hora local, sem fuso, sem calendário guardado.
- A célula guarda a contagem e mais nada; o calendário é **leitura de fronteira** (`:12969-12970`).

### A.2 Precisão: 32 bits, três planos
- O valor ocupa `S_LINHAS` (baixo) + `S_ALTO` (byte 2) + `S_ALTO2` (bytes 3–4) (`:12965-12968`).
- Envelope declarado no INSERT: `lo = 0; hi = 4294967295L` (`:3881`) — a contagem em 32 bits sem sinal.
- `tests/pgwire.c:26949-26981` mede precisamente isto: escolhe `1787443200` e `1787529600` (2026 em segundos, ≈1,8·10⁹) **de propósito**, porque um valor que coubesse em 16 bits não exercitaria o terceiro plano; e verifica que a data **não muda** quando um `ALTER TABLE ADD COLUMN` remapeia a matriz (instante estável antes/depois).

### A.3 O `(3)` de `TIMESTAMP(3)` é metadado, como a escala do `numeric`
`banco/sql.c:3269-3273`: o parser lê o primeiro número (precisão) e **descarta o segundo**; para `DATA`/`TIMESTAMP` o parâmetro nem chega a ser usado como escala de contagem — o valor é um inteiro de segundos. Não há subdivisão de segundo em nenhum ponto.

### A.4 Impressão / serialização
`banco/sql.c:12971-12984`: a leitura converte a contagem em calendário civil **na fronteira**, com aritmética de calendário correcta (anos bissextos, mês de Fevereiro). Saída no formato `AAAA-MM-DD HH:MM:SS` — **sem milissegundos, sem fuso, sem `T`, sem `Z`**.

### A.5 Comparação e ordenação
`CORPO_DATA` ocorre em apenas **dois** sítios em todo o ficheiro: o envelope (`:3881`) e a impressão (`:12961`). A ordenação (`ORDER BY`) e a comparação (`WHERE`) operam sobre o **número bruto** da contagem, pelo corpo genérico. Consequência: como a unidade é uniformemente o segundo, **comparar e ordenar é coerente por construção**, e o `WHERE t > '2026-01-01'` (se existir tradução de literal) é feito na mesma contagem.

### A.6 Ausências measures (importante para o contrato)
Não existe, no motor actual, nenhuma das seguintes construções: `EXTRACT`, `EPOCH`, `AT TIME ZONE`, `to_timestamp`, `date_trunc`, nem tratamento de fuso/horário de Verão. Isto é uma **ausência declarada**, não uma falha: o motor não promete aritmética de calendário.

---

## B. Invariantes que precisam de permanecer verdadeiros

1. **Unidade única e uniforme**: tudo em segundos desde 1970. Nenhuma coluna `DATA` pode estar em ms — não há como declará-lo (o `(3)` é descartado).
2. **Estabilidade do instante** sob remapeamento de matriz: o instante é um valor de célula, não um índice; `ALTER`/`UPDATE` não o podem deslocar. Já medido por `pgwire` (os três planos viajam juntos).
3. **Leitura de fronteira correcta**: a conversão contagem→calendário civil tem de respeitar bissextos (a casa já mede isso).
4. **Comparação/ordenação = ordenação cronológica**: só é verdade porque a unidade é única (A.1). Se alguma coluna passasse a ms, esta equivalência **quebraria silenciosamente** (uma data de 2026 em ms é ~1,8·10¹², muito maior que outra em s).

---

## C. A decisão conceptual (isolada) — CONGELADA (Opção T-A)

> **T-A — Temporalidade em segundos desde 1970, sem fuso.**
> `DATA` representa um instante inteiro em segundos desde a época Unix. `TIMESTAMP(3)` mantém a precisão declarada como metadado, sem introduzir milissegundos na representação. Necessidades de milissegundos, fuso ou outra granularidade exigem uma coluna/capacidade com contrato próprio.

**Ressalva exigida pelo utilizador, e que fica no contrato:**

> Isto é **preservação da semântica medida**, e **não** uma afirmação de que segundos são universalmente suficientes. O que se fixa é o que o motor garante hoje, com a evidência de A.1–A.6; não é um juízo sobre o domínio de aplicação.

Razão pela evidência é forte e não é apenas documental — cinco medidas independentes:

1. `CORPO_DATA` ocorre em **exactamente dois** sítios (`:3881` envelope, `:12961` impressão);
2. não existe caminho alternativo de `EPOCH`, timezone ou truncamento (A.6);
3. o `1787443200` de `pgwire:26964` **força** o terceiro plano por construção (`:26978-26981`);
4. o `ALTER` não modifica o instante (mesmo teste);
5. e, sobretudo, **não existe uma segunda unidade temporal disponível na sintaxe actual** (A.3) — logo a coerência de ordenação (B.4) é forçada, não acidental.

Opções registadas para que ninguém as reintroduza sem revisão:

- **T-B (fuso / `ms` como capacidade do motor) — não é uma correção de algo quebrado, é uma EXTENSÃO DE CAPACIDADE.** Seria funcionalidade nova (calendário, fuso, `EXTRACT`/`EPOCH`/`date_trunc`), não equivalência; exigiria contrato e medidores próprios, e hoje nada disto existe.
- **T-C (mudar a unidade actual para ms) — rejeitada de antemão.** Destruiria a invariante B.4 (comparação/ordenação) e as tabelas existentes, sem qualquer evidência de que o produto precise de ms.

Contrapartida que o contrato assume: as necessidades de milissegundos, fuso ou outra granularidade **não são absorvidas** por `DATA`; exigem coluna/capacidade com contrato e medidor próprios.

---

## D. Medidores correspondentes (já existem, demonstram o estado)

- `tests/pgwire.c` §(3) — os três planos viajam juntos; o instante não muda sob `ALTER`; controlo de que 2026 em segundos excede 16 bits. **VERDE.**
- `banco/sql.c` §W43 (medida racional) e os testes de `DATA` no `pgwire` (`:27434-27443`, UPDATE numa DATA). **VERDE.**
- Um medidor novo só é necessário se T-B for escolhida (fuso/`EXTRACT`) ou se alguém introduzir uma coluna em ms (para fixer a invariante B.4 por medição).

## E. Registo

- Contrato **fechado** a 2026-09-26, Opção **T-A**.
- **Nenhuma alteração de código** e **nenhum reatestado**: T-A descreve o comportamento já medido, logo `banco/sql` e `pgwire` mantêm assinatura e atestado (selo `f1320f71b848d4f8`).
- Consequência para o checkpoint: das **6** decisões de contrato do banco grande, o **temporal está resolvido**; restam **5** — RLS além de `SELECT`, `UUID→TEXT`, isolamento transitivo, embedding, forma do `UNIQUE`. Todos são contratos de **schema**, fora da camada Mat2Q.
