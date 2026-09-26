# CONTRATOS DO BANCO GRANDE — 4. `UUID → TEXT` (CONGELADO)

> **Estado: FECHADO — Opção U-A, 2026-09-26.** `UUID` é `TEXT` (corpo `CORPO_TEXTO`, endereço no pool). Unicidade é **por valor**, não por representação. Sem alteração de código, sem reatestado.
>
> Precedência: `PROTOCOLO_MATRICIAL_BIG_PICKLE.md` §11–§12. **Sem interface com a `Qz`/Mat2Q**: é um contrato de schema.
>
> Nenhuma alteração de código: U-A descreve o comportamento já medido; as assinaturas de `banco/sql` e `pgwire` mantêm-se.

---

## A. O que o motor faz hoje (medido)

### A.1 `UUID` é `TEXTO` — um único ramo
`banco/sql.c:3226` — `UUID` entra na mesma lista de `TEXTO`/`VARCHAR`/`CHAR`/`STRING`/`JSON`/`JSONB` e recebe `CORPO_TEXTO`. A célula guarda o **endereço no pool** (`:418-421`, *«índice como estante»*); a cadeia vive na gaveta.

### A.2 Não há validação de formato UUID
Não existe, em todo o motor, qualquer verificação de que a cadeia tem a forma `8-4-4-4-12`, nem de que é um UUID canónico. Um `UUID` aceita qualquer cadeia. Isto é coerente com o princípio que a casa já aplica a `vector`/`TEXT[]`/`tsvector` (`:3233-3248`): **guardar não é saber operar** — o motor diz o que não faz, em vez de fingir.

### A.3 A unicidade é por VALOR (a garantia que fecha o contrato)
O `UNIQUE` constrói uma árvore de índices (`:3471`) que, para um corpo de texto, indexa o **endereço** da célula (`celula_valor`, `:2014`; comparação em `:2038-2042`). Isto só é correcto porque `tx` garante, por desenho, que **a mesma cadeia recebe sempre o mesmo endereço**:
> *«A MESMA CADEIA TEM DE DAR O MESMO ENDEREÇO, e isto não é economia de espaço: é o CRITÉRIO DA LEITURA … sem isto duas cadeias iguais recebiam endereços diferentes, pelo que comparar endereços deixava de comparar textos.»* (`:1666-1670`)

E o registo em `:1672-1675` mostra que esta condição **já foi necessária para a segurança funcionar**: a política de isolamento falhava quando cadeias iguais davam endereços distintos — *"o comando era aceite e não fazia nada --- que é o pior desfecho possível numa peça de segurança"*. Conclusão medida: **`UNIQUE` sobre `UUID` é uma restrição de valor**, não de representação, e é segura.

> **Memória de contrato (preservada deliberadamente).** Este não é um detalhe de implementação descartável: é um invariante que a casa **já precisou** para que o isolamento por inquilino (Contrato 3, RLS) não falhasse em silêncio. A cadeia de dependências é
> `mesma cadeia → mesmo endereço (tx) → comparar endereços compara textos → RLS e UNIQUE são restrições de VALOR`.
> Qualquer alteração futura ao pool de cadeias, à deduplicação, ou ao `UNIQUE` tem de preservar as três setas, ou volta a abrir exactamente esta falha.

---

## B. Invariantes que permanecem verdadeiros

1. `UUID` e `TEXT` são o **mesmo tipo efectivo** (mesmo corpo, mesma armazenamento, mesma impressão). `WHERE uuid_col = 'texto'` compara por valor, com a certeza garantida por A.3.
2. `UNIQUE` sobre uma coluna `UUID` recusa duplicados **por igualdade de cadeia**, de forma fiável.
3. **Não há** validação de formato: a forma `8-4-4-4-12` é responsabilidade do cliente, e o motor não promete o que não faz.

---

## C. A decisão conceptual (isolada) — CONGELADA (Opção U-A)

> **U-A — Preservar a semântica medida:** `UUID` é `TEXT` com semântica de valor; o motor não valida o formato e a unicidade é por valor. Validar a forma canónica, gerar UUIDs, ou introduzir um tipo UUID distinto (16 bytes, operadores próprios) são **capacidades novas**, com contrato e medidor próprios.

Ressalva (mesma forma dos anteriores): isto fixa **o que o motor mede e garante**, não afirma que `UUID`-como-`TEXT` seja a melhor modelação de produto. Um sistema que exija formato canónico, ordenação por versão ou UUID nativo declara-o como capacidade separada.

Opções registadas:
- **U-B (validar a forma `8-4-4-4-12` no INSERT)** — capacidade nova; contradiz a invariante B.3 (guardar não é validar) e introduz rejeição que hoje não existe.
- **U-C (UUID nativo de 16 bytes, com operadores de versão)** — reescrita de âmbito largo, sem necessidade demonstrada.

Contrapartida que o contrato assume: o motor **não garante** que um `UUID` seja um UUID. Qualquer consumidor que dependa da forma tem de a verificar, ou declarar U-B.

## D. Medidores correspondentes

A unicidade por valor é exercida pelos testes de `UNIQUE`/`PRIMARY KEY` em `banco/sql` (VERDE 98/98) e pelo caminho de texto em `pgwire` (VERDE). Nenhum medidor novo é necessário para U-A; U-B/U-C exigiriam medidores próprios.

## E. Registo

- Contrato **fechado** a 2026-09-26, Opção **U-A**, sem alteração de código.
- Restam **3** decisões de schema: isolamento transitivo, embedding, forma do `UNIQUE`.
