# CONTRATOS DO BANCO GRANDE — 1. MONETÁRIO (CONGELADO)

> **Estado: FECHADO — Opção A, 2026-09-26.** `(p,s)` permanece metadado, sem arredondamento nem quantização. Nenhuma alteração de código.
>
> Precedência: `PROTOCOLO_MATRICIAL_BIG_PICKLE.md` §11 (a casa decide) e §12 (nada se migra sem contrato escrito). Este documento **não migra nada** — fixa o que a medição demonstrou e regista a decisão do utilizador.
>
> Estado do atestado no momento do fecho: `banco/sql` VERDE 98/98, exit 0 (selo `f1320f71b848d4f8`), inalterado por este contrato.

---

## A. Natureza do contrato (leitura obrigatória antes de qualquer decisão)

O monetário é o **único** dos contratos pendentes com interface matemática real com a `Qz`. Os outros seis (temporal, RLS, `UUID→TEXT`, isolamento transitivo, embedding, forma do `UNIQUE`) são **contratos de schema** e ficam deliberadamente **fora** desta camada — não têm semanticamente nada a ver com `Mat2Q`.

Não se escreve, para este contrato, uma correspondência `schema → Mat2Q`. Escreve-se a cadeia real, que passa pelo corpo `RACIONAL` do motor e pela `Qz` que a casa já mediu.

---

## B. Estado actual (medido, não presumido)

### B.1 O tipo já é o racional
`banco/sql.c:3249-3250`:
```c
else if(!strcasecmp(tipo,"DECIMAL") || !strcasecmp(tipo,"NUMERIC")
     || !strcasecmp(tipo,"MONEY"))    corpo_j = CORPO_RACIONAL;
```
`DECIMAL`, `NUMERIC` e `MONEY` recebem o **mesmo corpo** de `RACIONAL` (`:3212`). A equivalência de leitura já é exacta por construção.

### B.2 A precisão é declarada e lida; a escala é lida e descartada
`banco/sql.c:3268-3274`:
```c
if(*p == '('){ p++; long q; if(numero(&p, &q)) parm_j = q; pula(&p);
               while(*p == ','){ p++; pula(&p); long q2;
                                 if(!numero(&p, &q2)) break;
                                 pula(&p); }   /* q2 = ESCALA: lida, não guardada */
               if(*p == ')') p++; }
```
- `DECIMAL(18,4)`: guarda-se `p = 18` em `parm_j`; o `4` (escala) é consumido e **descartado**.
- A escala **não muda o corpo** — comentário explícito em `:3269-3270` e `:3229-3232`: *"o racional guarda a CLASSE, não uma escala"*.
- `parm_j` é persistido em `wc.e` da coluna (`:3396`).

### B.3 O valor é exacto e simétrico na leitura/escrita
- **Escrita (INSERT)**: o valor entra como fracção reduzida pela classe única `ra_classe` (`:3745-3746`), o denominador reduzido vai para o plano `S_DEN` (`:4021-4023`) e o numerador para `S_LINHAS`/`S_ALTO`/`S_ALTO2` (`:4005-4016`).
- **Leitura (`celula_qz`, `:7301-7304`)**: onde o corpo é `RACIONAL`, o valor é rederivado pela mesma `ra_classe` (`:7302`).

Consequência medida: a interface `numeric(18,4) → Qz` é **exacta e reversível** (ida e volta pelo mesmo representante canónico `n/d`, `d>0`, reduzido por `mdc`), e não há arredondamento em nenhum ponto do caminho. Isto é o que §2 do protocolo exige: a casa não trunca nem arredonda em silêncio.

### B.4 Escala não é arredondamento
Como a escala é descartada e o valor é uma `Qz`, **não existe arredondamento de escala** neste motor. Um `numeric(18,4)` guarda a fracção exactamente como escrita; não há `ROUND(…, 4)`. Isto é uma **propriedade medida** (ver §C), não uma decisão pendente.

### B.5 `parm_j.e` é reserva, não denominador fixo (parte do contrato, não detalhe de implementação)

Este ponto explica **porque a leitura continua correcta** apesar de `p` não funcionar como denominador fixo, e por isso é parte do contrato medido:

- A palavra da célula tem duas componentes: `total` e `e` (`.e`). O `.e` da **célula** é o denominador do valor **quando o INSERT o escreveu** (`:4005` grava `w.e = den[j]`; o denominador reduced vai também para o plano `S_DEN`, `:4021-4023`).
- O `.e` da **coluna** (`parm_j` = precisão `p`, `:3396`) é a **reserva** para o caso sem denominador próprio: em `celula_qz`, `c.e ? c.e : 1` (`:7302`) usa o `.e` da célula quando o corpo é `RACIONAL`; para os corpos quadráticos o `.e` é o coeficiente de `σ`, não um denominador (`:7285-7299`).
- Em quaquer valor escrito por INSERT, o denominador effective vem **da própria célula**; a precisão `p` só actua como denominador por omissão quando não há denominação própria. É esta reserva — e não um `p` fixo — que torna a leitura exacta e simétrica.

Consequência de contrato: introduzir uma semântica de "casas decimais" **não pode** ser feito só tocando em `parm_j.e`; essa palavra é estrutura de armazenamento partilhada com `σ` e com o plano `S_DEN`.

---

## C. O que precisa de ser preservado (invariantes, já verificadas)

1. **Exactidão**: `DECIMAL/NUMERIC/MONEY` não suffer arredondamento, truncagem nem passagem por `double`. O caminho é `int`/`Par`, nunca vírgula-flutuante.
2. **Classe única**: cada racional tem um representante único `n/d` (reduzido por `mdc`, denominador positivo), o mesmo que `racional_pg.c §Q1` mede.
3. **Escala preservada como metadado, não como truncagem**: o `(p,s)` declarado não reescreve o valor; o que se perde é a **restrição de magnitude** que o Postgres aplicaria, não o número.
4. **Idempotência de ida e volta**: `INSERT 3/2` → `SELECT` devolve a mesma classe.

---

## D. DECISÃO DO UTILIZADOR — CONGELADA (Opção A)

> **Opção A escolhida: `(p,s)` permanece metadado, sem introdução de arredondamento nem quantização.**
>
> Data: 2026-09-26. Estado: **fechado**. Não há alteração de código — a Opção A descreve o comportamento já medido, pelo que a assinatura de `banco/sql` e os atestados ficam **inalterados**.

Ressalva explícita, do próprio utilizador, que fica no contrato:

> **Isto não significa que `(18,4)` seja irrelevante para todo o sistema; significa apenas que o motor actualmente não o interpreta como uma restrição de escala sobre o valor racional armazenado.**

Opções descartadas e porquê (registadas para que ninguém as reintroduza sem revisão):

- **Opção B — `(p,s)` como restrição efectiva:** rejeitada. Introduziria uma regra nova (recusa de valores fora de faixa) e é uma **mudança semântica**, não uma equivalência; exigiria contrato e testes próprios, e colidiria com o `parm_j.e` ser estrutura partilhada (ver B.5).
- **Opção C — arredondar para a escala:** rejeitada. **Contraria directamente** a propriedade já medida de armazenamento/reconstituição racional exacta (B.3) e a exactidão que §W43 mede na média racional. Não é admissível sem revisão do protocolo.

Contrapartida que o contrato assume: o que fica por modelar é a **restrição de magnitude** que o Postgres aplicaria ao `numeric(18,4)` — não o número. Um sistema que precise dessa restrição terá de a introduzir como contrato próprio e explícito, com medidor; não a herda deste motor.

---

## E. Medidor correspondente

`tests/pgwire.c` (o caminho que exercita `DECIMAL`/`NUMERIC` contra o driver e a migração) e `banco/sql.c` §W43 (a média como racional exacto) são os medidores que hoje **demonstram** as invariantes de C.1-C.4. Como a Opção A **não altera o motor**, nenhum medidor novo é necessário: as assinaturas de `banco/sql` e `pgwire` ficam como estão e os atestados não mudam.

A Opção B (rejeitada) exigiria um medidor de overrange; a Opção C (rejeitada) exigiria rever o medidor §W43. Nenhuma das duas está em vigor.

## F. Registo final

- Contrato **fechado** a 2026-09-26, Opção A.
- **Nenhuma alteração de código** foi feita para este contrato: a Opção A é uma descrição do estado medido, não uma alteração.
- Consequência para o checkpoint: das **7** decisões de contrato do banco grande, o **monetário está resolvido**; restam **6** (temporal, RLS além de SELECT, `UUID→TEXT`, isolamento transitivo, embedding, forma do `UNIQUE`).
- O **temporal** é o próximo, por ser o único com impacto directo já observado em `banco/sql`.
