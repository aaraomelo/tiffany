# Frente JEV — protocolo de leitura semântica

## 1. Objeto de estudo

`jev.tex` — "JEV e Campos de Contagem: Construção de uma Camada
de Representação para Decisão Tipada".

## 2. Objectos fundamentais

- **I** — conjunto finito (eventos / observações / índices de realização)
- **X** — conjunto discreto finito (células / estados observáveis)
- **π : I → X** — realização discreta
- **G(x) = |π⁻¹(x)|** — campo de multiplicidade (contagem)
- **p_G(x) = G(x)/|I|** — distribuição de frequência induzida
- **q : X → Y** — critério de classificação abstrato (≠ pergunta de API)
- **G_q(y) = Σ_{x∈q⁻¹(y)} G(x)** — campo agregado
- **p_q(y) = G_q(y)/|I|** — distribuição induzida

## 3. Significado dos símbolos

| Símbolo | Significado | Natureza |
|---------|-------------|----------|
| π | realização discreta | função de I em X |
| G | campo de contagem | G : X → ℕ₀ |
| q | critério de classificação | função de X em Y |
| G_q | campo agregado | G_q : Y → ℕ₀ |
| p_q | distribuição normalizada | p_q : Y → [0,1] |
| L | funcional de leitura | depende do tipo JEV |

## 4. Noul / Choice / Score

### Noul (binário)
- Y = {0,1}
- P : X → {0,1} proposição
- G_P(1) = Σ G(x)·P(x), G_P(0) = Σ G(x)·(1-P(x))
- p_P(yes) = G_P(1)/|I|
- Leitura: p_P(yes) ∈ [0,1]

### Choice (categorial)
- Y = C = {c₁,...,c_n}
- G_q(c_i) = Σ_{x:q(x)=c_i} G(x)
- Leitura: c* ∈ argmax p_q(c), κ(p_q) = max p_q(c) (auxiliar, ≠ confidence JEV)

### Score (ordenado)
- Y = S = {s₀,...,s_m} ⊂ ℝ ordenado
- G_q(s_i) = Σ_{x:q(x)=s_i} G(x)
- Leitura: 𝔼_G[S] = Σ s_i·p_q(s_i) ∈ ℚ (finito); limite pode ser irracional

## 5. Invariantes explícitos

1. Conservação: Σ_x G(x) = |I| (Prop. conservação)
2. Conservação sob projeção: Σ_y G_q(y) = |I| (Prop. conservação sob projeção)
3. Normalização: Σ_y p_q(y) = 1
4. Score finito é racional: 𝔼_G[S] = N/D ∈ ℚ (Prop. score-limite)
5. Marginalização: G_{q_i}(y_i) = Σ_{j≠i} G_q(y_1,...,y_n) (Teor. marginal)

## 6. Entradas e saídas de cada transformação

```
π : I → X          (realização)
G : X → ℕ₀        (campo de contagem)
q : X → Y         (classificação)
G_q : Y → ℕ₀      (campo agregado)
p_q : Y → [0,1]   (distribuição)
L(p_q)            (leitura tipada: Noul/Choice/Score)
```

Cada seta tem entrada e saída tipadas. O documento distingue
representação construída ≠ implementação interna do JEV.

## 7. Objectos matemáticos vs classificatórios/operacionais

**Matemáticos (construção):**
- π, G, q, G_q, p_q, L
- Proposições e teoremas demonstrados

**Classificatórios/operacionais (contrato JEV):**
- state (entrada da API)
- question (pergunta tipada com instructions + criteria)
- typed answer (choice/score/noul)
- confidence (campo documentado, fórmula não pública)
- application code / action (política externa)

**O documento NÃO identifica:**
- state ↔ G
- question ↔ q (só ⋛, não =)
- typed answer ↔ leitura L (correspondência estrutural, não identidade)
- confidence ↔ κ (κ é auxiliar, não é confidence JEV)

## 8. Dependências com campos.tex e fisica.tex

- `campos.tex`: definição de G, conservação, transformação de representação,
  DFT-8, mudança de suporte vs transformação
- `fisica.tex`: lemas de conservação, construção de base, Walsh,
  realização angular, estigmergia
- `jev.tex` reutiliza notação mas **não depende de resultados não
  demonstrados** — é instanciação tipada dos invariantes gerais

## 9. Instância vs definição (o que é instanciado vs o que é definido)

**Definido em jev.tex:**
- Realização, campo de contagem, projeção, normalização, G_q, p_q
- Noul, Choice, Score como casos de Y e L
- Projecção conjunta e marginalização (Teor. marginal)
- Corolário: cada contrato basta-se com projeção individual

**Instanciado (validação computacional):**
- Piloto E1-E6 (577/577 verificações, 0 falhas)
- WASM ≡ oráculo ≡ esperado
- Código: tests/jev_backends.js, tests/jev_front.js,
  conecthus/backends/wasm/jev/marginal.c, assets/figuras/wasm/jev/marginal.wasm,
  app/src/jev_contratos.js

## 10. Afirmações do documento sobre implementação

- "Não se afirma que o JEV implemente campos G internamente"
- "A implementação interna do JEV permanece indeterminada"
- "Nada no material público afirma que o JEV mantenha um campo de multiplicidade G"
- "Qualquer construção deste paper que use G é uma representação externa"
- "A implementação, o treino e a calibração do JEV ficam fora do escopo"

## 11. Limites do documento

- Não demonstra que JEV usa G internamente
- Não reconstrói mecanismo interno do JEV
- Não identifica implementação do JEV
- Não prova equivalência entre contratos JEV e campos de contagem
- Apenas: os contratos admitem realização por campos de contagem

## 12. Estado actual da integração JEV ↔ banco

- **Nenhuma ponte demonstrada** (grep: 0 hits JEV/no/code em banco/, can/)
- **Código JEV existente:** apenas `conecthus/backends/wasm/jev/marginal.c`
  (piloto separado, não sql.c)
- **Objectos partilhados (não integração):** lib/racionais.h, lib/linear.h
  (usados por sql.c, mas não para JEV)
- **q-JEV ≠ q-Mat2Q** (critério de classificação vs parâmetro de polinómio)
- **sql.c:** zero referências JEV — banco não sabe que JEV existe
- **can/:** zero referências JEV — camada J1939 não refere JEV. *(Confirmado à
  parte em §19.2: 51 ficheiros, 0 hits. `can/` não estava no workspace remoto na
  execução do `§G8`, logo a prova original desta linha não existia.)*
- **lib/*.h:** zero referências JEV — nenhuma header do motor refere JEV
- **`§19` — onde vive `q` decide o comportamento do motor.** Tabela separada:
  **não consumido**. Mesma linha de `G`: **consumido** (mas é outra
  representação). Texto da consulta: **não está no banco** (`DROP` mantém o
  resultado). Detalhe e prova em §19.1. **Nenhuma destas foi escolhida como
  arquitectura.**

## 13. O que existe no repo (sem tocar no banco)

| Peça | Onde | Papel |
|------|------|-------|
| Paper matemático | `redes/jev.tex` | Definição + proposições + validação E1-E6 |
| Motor wasm JEV | `conecthus/backends/wasm/jev/marginal.c` | Piloto E1-E5, único módulo JEV |
| WASM compilado | `assets/figuras/wasm/jev/marginal.wasm` | Artefacto do piloto |
| Medidor backends | `tests/jev_backends.js` | E1-E5, 509/509 |
| Medidor front | `tests/jev_front.js` | E6 smoke, 68/68 |
| Selo front | `app/src/jev_contratos.js` | Demonstração ao vivo E6 |
| Integração docs | `docs/INTEGRACAO_JEV_WASM.md` | Registo do piloto fechado |
| Medidor §17 (G) | `tests/campo_g_no_banco.c` | 15/15, resíduo 0 — checkpoint `77d021eb` |
| **Medidor §19 (q armazenado)** | **`tests/q_armazenado_no_banco.c`** | **23/23, resíduo 0, 3 execuções byte-idênticas** — SHA-256 `201799acb2cef69e99432d400416d42d63664c33bb0c3babc443fe6595ddd7d9`. Ver §19 e §19.1 |

## 14. Ponte real entre JEV e banco

**Não existe ponte JEV → banco.**

O que existe é uma relação indirecta por objectos partilhados:
- `lib/racionais.h` — usado por sql.c E por marginal.c (ambos usam racionais, mas para fins diferentes)
- `lib/linear.h` — usado por sql.c (mat_esc) e potencialmente por construções JEV
- `lib/campo.h` — usado por banco/conversa.c (campos físicos) — ≠ campo G de JEV

Estes são **objectos partilhados do repo**, não pontes JEV → banco.

## 15. Camada 2 — o que pode ser persistido no banco

Sem tocar em sql.c. Sem implementar. Sem assumir equivalência.

**Candidatos a persistência (representação externa):**
- G(x) — campo de multiplicidade: tabela `ocupacao(célula, contagem)` sobre realização I
- q — critério de classificação: metadata da pergunta, não dado do banco
- G_q(y) — campo agregado: **derivado** de G, não persistir (computável por query)
- p_q(y) — distribuição: **derivado** de G_q, não persistir (derivável)
- L(p_q) — leitura tipada: saída da API, não persistir no schema do banco
- π — realização: eventos da aplicação, já existem no DISCO/arena

**O que NÃO é candidato:**
- state do JEV — não identificado com G (Obs. ref{obs:correspondencia})
- confidence — campo documentado JEV, fórmula não pública
- question — objecto de API com texto, não é q matemático
- implementation interna do JEV — caixa preta

## 16. Invariantes a preservar (se houver integração futura)

1. Conservação: Σ G(x) = |I|
2. Conservação sob projeção: Σ G_q(y) = |I|
3. Normalização: Σ p_q(y) = 1
4. Score racional em realização finita
5. Separação representação ≠ implementação JEV
6. question ⟷ q é ⋛ (analogia estrutural), nunca =

## 17. Classificação FECHADO / HIPÓTESE / SEM PONTE

**FECHADO:**
- Leitura semântica de jev.tex completa
- Identificação de objectos fundamentais e invariantes
- Mapeamento de dependências com campos.tex / fisica.tex
- Instanciação vs definição separadas
- Piloto E1-E6 documentado (577/577 verificações)

**HIPÓTESE (com o veredicto medido, uma por uma):**
- ~~G poderia ser persistido como tabela de ocupação sobre realizações~~
  → **MEASURED, confirmado.** `ocupacao(celula TEXTO, contagem INTEIRO)`
  guarda e devolve G linha a linha, célula a célula, `G=0` incluída.
- ~~Uma query poderia computar G_q como derivado de G~~
  → **MEASURED, confirmado numa forma e recusado noutra.** Numa tabela só,
  `GROUP BY classe` devolve `G_q` certo (7, 5) e Σ=12. Com `G` e `q` em duas
  tabelas, o `JOIN` não acede à coluna da tabela da direita e a consulta é
  recusada. Ver limite 2.
- ~~q poderia ser metadata de pergunta no schema~~
  → **MEASURED, confirmado como metadata, e só isso.** `q` entrou no banco
  como coluna `TEXTO` e voltou. O banco não sabe que é uma pergunta; e o que
  o impede de servir a derivação é o `JOIN`, não o `q`.
- lib/racionais.h e lib/linear.h são partilhados mas não pontes
  → **inalterado** por este experimento, que não os tocou.

**EXPERIMENTO EXECUTADO — resultado medido (2026-09-27):**

Pergunta (§18): *"O banco consegue representar um campo externo de contagem G e
preservar seus invariantes?"*

- **Classificação: FECHADO** — para os três invariantes, medidos e preservados.
  Não é FECHADO para a integração: continua a não haver ponte (ver abaixo).
- Medidor: `tests/campo_g_no_banco.c`
  - SHA-256 do **blob committado** (forma **LF**, que é a que um clone dá):
    `6ad5a0d1d93e0d7dbd0f51a64a35ab6f054580293dfd5a3eb58774be17696a3a`
  - ~~SHA-256 `1d7af0711f6fbd85e9b2162187e1b7082371bcfbd6e06c672d8cbc2e5f9f148b`~~
    **CORRIGIDO — ver §19.2.** Era o hash da variante **CRLF** do *worktree*
    (29496 bytes), não a do ficheiro committado (28999 bytes): o repositório tem
    `core.autocrlf=true` e não tem `.gitattributes`, por isso o blob é LF e um
    checkout em Windows dá CRLF. **O número registado não era reproduzível a
    partir de um clone.**
- Resultado: **15 unidades, 0 falhas — resíduo 0**
- **Três execuções consecutivas, três vezes igual** — a base de teste foi apagada
  entre cada uma (`rm -f /tmp/campo_g_base*`) e as três devolveram
  `15 unidade(s), 0 falha(s) — RESIDUO 0`. O resultado não é de uma corrida
  favorável.
- Motor **inalterado**: `banco/sql.c` SHA-256
  `54718e6767fc6a5c025400d1c0ba22650c2938bcf416eddde7aac3c2b8dd08af`
  antes e depois. Zero alterações ao motor.
- Ambiente: Linux. O motor não compila nativamente no Windows/MinGW
  (`conflicting types for 'mkdir'`, `lib/disco.h`), por isso mediu-se em
  `/root/jev_g_campo` com `cc -O2 -std=c99 -Ilib -Ibanco -DSQL_NO_MAIN`.

**Instância medida**

| | |
|---|---|
| I | `{0..11}`, \|I\| = 12 |
| X | `{a,b,c,d}` |
| π | `a,a,a,b,b,b,b,c,c,c,c,c` |
| G | `a=3  b=4  c=5  d=0` (fibra vazia) |
| q | `a,b,d → y1`; `c → y2` |
| G_q | `y1=7  y2=5` |
| p_q | `7/12, 5/12` |

**As três métricas, medidas dentro do banco**

1. Conservação: `SELECT sum(contagem) FROM ocupacao` → **12** = \|I\|
2. Conservação sob projeção: `Σ_y G_q(y)` = **7+5 = 12** = \|I\|
3. Normalização: `Σ_y p_q(y)` = 1 em aritmética exacta
   (mediu-se em inteiros: em `double` a mesma soma daria 1 por arredondamento)

**Representação que passou, e a que não passou — sem escolher entre elas**

| forma | consulta | resultado |
|---|---|---|
| (i) duas tabelas, `G` em `ocupacao`, `q` em `criterio` | `SELECT c.classe, sum(o.contagem) FROM ocupacao o JOIN criterio c ON o.celula = c.celula GROUP BY c.classe` | **RECUSADA** |
| (ii) uma tabela, `registo(celula, contagem, classe)` | `SELECT classe, sum(contagem) FROM registo GROUP BY classe` | **ACEITE** — devolveu `y1=7`, `y2=5`, Σ=12 |

O experimento **não diz qual das duas é a certa**. Escolher a que passa sem
dizer qual se escolheu seria fraude; ficam as duas registadas com o resultado
de cada uma. A (ii) duplica o campo `G` dentro do mesmo registo — é uma
representação diferente, não uma melhoria.

**Limites medidos (não são invariantes, são factos sobre o motor)**

1. **Coluna sem tipo declarado nasce `INTEIRO`.** Uma chave textual precisa de
   `TEXTO` explícito. A primeira execução gravou zero linhas e a medição leu a
   tabela vazia como «o banco não guarda o campo» — um falso limite que foi a
   primeira descoberta.
2. **O JOIN não lê colunas da tabela da direita.** Qualificada:
   `«c.classe» não é coluna desta tabela nem expressão que eu saiba ler —
   RECUSADA`. Sem qualificar: `column "classe" does not exist`. O `GROUP BY`
   sozinho funciona (2 grupos, 2 colunas). É o JOIN que não acede a `q`.
3. **A porta C entrega uma coluna a mais do que a projecção pediu.** Para
   `classe, sum(contagem)` — dois itens — `SqlOut` devolve `ncols = 3`:
   `[chave, tamanho-da-fibra, soma]`. O agregado é a **última** coluna.
   A primeira versão do medidor somou a coluna 1 e concluiu que o banco não
   sabia agregar; a soma estava à espera na coluna 2. Só a impressão de `ncols`
   apanhou o erro de leitura — e um erro de leitura que se apresenta como
   defeito do motor é o pior dos dois desfechos.
4. **Coluna de contagem a zero persiste e relê-se** (`d | 0` volta), e o motor
   tem o agregado `sum(contagem)`.

**Controlos (para que «verde» não seja indistinguível de «não mediu»)**

- *Controlo negativo:* `G'(d) = 2` foi gravado no mesmo banco, relido pelo
  mesmo caminho, e a **mesma lei** viu a violação: `Σ G' = 14 ≠ 12` e
  `Σ G_q' = 14 ≠ 12`. Uma versão anterior deste controlo passava pelo motivo
  errado (a tabela ficava vazia, e tabela vazia também «viola» a soma), por
  isso o campo errado tem de existir no banco antes de se afirmar que a lei o
  viu.
- *Controlo do detector:* ~~212 ficheiros~~ **CORRIGIDO — ver §19.2. A contagem
  «212» é de ficheiros lidos com **triplicação** (três prefixos sem
  deduplicar), não de ficheiros distintos.** 0 com `jev`; e o mesmo detector
  encontra `jev` **16 vezes** no ficheiro onde a palavra está. Sem esse controlo,
  «0 ocorrências» seria indistinguível de um detector que nunca leu. **A conclusão
  mantém-se**: ler um ficheiro duas vezes não muda se contém `jev`.

**O que o experimento NÃO mediu**

- `q` em forma de predicado de linguagem natural (a §17 só mediu `q` como
  metadata tabular)
- ordenação temporal, multi-salto, `|X|` grande, `|I|` em escala real
- desempenho, atomicidade, concorrência, persistência entre reinícios
- `p_q` do lado do banco, e a leitura tipada `L(p_q)`
- qualquer coisa sobre o motor JEV: `banco/sql.c` **já** usa `G(x)` no seu
  próprio vocabulário para descrever `GROUP BY` (`banco/sql.c:6826-6830`).
  É um paralelo estrutural preexistente, e registá-lo **não** é identificar
  `G` do banco com `G` do JEV.

**SEM PONTE (reconfirmado por medição, não por grep solto):**
- Nenhuma integração JEV → banco demonstrada
- `banco/sql.c` não refere JEV — 0 hits nos ficheiros de `banco/` e `lib/`
  lidos. ~~0 hits em 212 ficheiros~~ → **contagem corrigida, §19.2**: 0 é o
  número certo; **212 não era o número de ficheiros**
- `can/` não refere JEV — **CONFIRMADO, mas por via diferente das duas
  restantes:** `can/` **não existia** no workspace remoto quando o `§G8` correu
  (§19.2), logo esta linha **não tinha prova** naquele momento. Verificado à parte
  depois: **51 ficheiros `.c`/`.h`, 0 ocorrências**
- `lib/*.h` não refere JEV — 0 hits
- `G_q` e `p_q` não foram persistidos: `criterio` guarda só `q` como metadata,
  e `G_q` foi sempre derivado
- q-JEV ≠ q-Mat2Q (critério de classificação ≠ parâmetro de polinómio)
- state JEV ≠ G (não identificados)
- question ≠ q (só ⋛)
- confidence ≠ κ

## 18. Nota de controlo

Este mapa regista o que existe e o que não existe.
Não implementa. Não assume equivalências.
A frente JEV → banco só avança quando houver uma pergunta mínima
mensurável — e **essa pergunta foi formulada e executada**:**
"O banco consegue representar um campo externo de contagem G e preservar
seus invariantes?"

Resposta medida, com `tests/campo_g_no_banco.c`, 15/15 e resíduo 0:
**sim, representa, e os três invariantes medem-se conservados dentro do banco**
(Σ G = 12, Σ G_q = 12, Σ p_q = 1, com \|I\| = 12).

O que isso **não** diz: que exista ponte, que `q` possa ser uma pergunta, nem
que `G` do banco seja `G` do JEV. `G_q` é derivado — e o experimento não escolheu
entre as duas representações testadas, uma das quais o motor recusa.

## 19. G + q → G_q — PERFILHA EXECUTADA

**Estado: EXECUTADO. 23/23, resíduo 0, três execuções byte-idênticas.** Medidor:
`tests/q_armazenado_no_banco.c`, SHA-256
`201799acb2cef69e99432d400416d42d63664c33bb0c3babc443fe6595ddd7d9` — **do blob
committado, forma LF**, verificado byte-a-byte contra o que foi medido (34870
bytes nos dois). Um checkout em Windows dá CRLF e um SHA diferente; ver §19.2.
`banco/sql.c` inalterado: SHA-256
`54718e6767fc6a5c025400d1c0ba22650c2938bcf416eddde7ac3c2b8dd08af`, blob
`aa2c4960debacf04f7f734ba625d96b4ae0ee2e7` no HEAD, no índice e no worktree.

**Consequência de conceito, e é a frase principal desta secção:** «G + q no
banco» **não é uma pergunta só**. A *localização* de `q` muda o comportamento
observável do motor, e as três localizações dão três motores diferentes. Este
resultado apareceu porque se deixou o motor responder, não porque se discutiu
arquitectura.

### O que `77d021eb` já fechou, e portanto não se repete

A instância proposta para a próxima frente é **a mesma, célula a célula**, que a
que já foi medida:

| | |
|---|---|
| I | `{0..11}`, \|I\| = 12 |
| X | `{a,b,c,d}` |
| G | `(3,4,5,0)` |
| q | `a→y1, b→y1, c→y2, d→y1` |
| G_q | `y1=7, y2=5` |
| p_q | `7/12, 5/12` |

`tests/campo_g_no_banco.c:69` tem `Q[4] = { "y1", "y1", "y2", "y1" }` — a mesma
função. E `77d021eb` mediu, sobre essa instância:

- `Σ G = 12` (INVARIANTE 1, §G3)
- `Σ G_q = 7 + 5 = 12` (INVARIANTE 2, §G5)
- `Σ p_q = 1` (INVARIANTE 3, §G5)
- e na §G6 **as duas representações de `q`**: duas tabelas + `JOIN` → recusada;
  uma tabela (`celula, contagem, classe`) + `GROUP BY` → aceite, `G_q` correcto

Repetir esta instância daria 15/15 outra vez e não acrescentaria uma medição.
Por isso a pergunta abaixo não é a repetição.

### ACHADO 1 — o controlo negativo pedido não pode existir na forma pedida

A especificação pedia «alterar `q` … e produzir uma conservação diferente». Para
qualquer função `q : X → Y`:

```
Σ_y G_q(y) = Σ_y Σ_{x: q(x)=y} G(x) = Σ_x G(x) = |I|
```

`X` fica partido pelas fibras de `q`, e a conservação é uma **tautologia** dessa
partição. Nenhuma função `q` a pode violar: mudar `q` muda a *distribuição*
entre classes, nunca o *total*. Um controlo que «muda `q` e vê a conservação
cair» é vazio — passaria igualmente com `q` completamente ignorado, que é
precisamente o defeito que um controlo negativo tem de apanhar.

### ACHADO 2 — o único mecanismo entre tabelas do motor é o `JOIN`

Lido em `banco/sql.c`, sem o alterar: existem `WHERE`, `IN`, `JOIN`, `GROUP BY`
e `ORDER BY`; **não existe `CASE`**. O `IN` é a disjuncão de igualdades
(`sql.c:4703`). A «subquery» de `sql.c:8879` é o **lado direito do `JOIN`** com
guarda de lotação — o comentário em `sql.c:8852` diz-o — e não uma subconsulta
SQL. E a §G6 mediu que o `JOIN` não lê colunas da tabela da direita.

Consequência a medir: **não há caminho no motor pelo qual um valor guardado numa
tabela filtre, associe ou classifique linhas de outra tabela.** Os caminhos que
restam são (a) fundir `q` no mesmo registo de `G`, ou (b) escrever `q` no texto
da consulta — e em (b) o `q` deixa de estar no banco.

~~Por reconciliar: uma sondagem por CLI chegou a aceitar um `JOIN` que o medidor,
pela porta `SqlOut`, recusa.~~ **RECONCILIADO pela medição — o medidor é que
usava a forma errada.** A sondagem por CLI e a porta `SqlOut` concordavam; a
discrepância era do teste, não do motor. Duas formas distintas:

| forma | resultado medido |
|---|---|
| `FROM ocupacao o JOIN criterio c ON o.celula = c.celula` (**com alias**) | recusada |
| `FROM ocupacao JOIN criterio ON ocupacao.celula = criterio.celula` (**sem alias**) | **aceite** |

O motor não percebe o alias; qualifica pelo **nome da tabela**. Isto importa para
o registo: uma recusa que se atribuiu ao `JOIN` era, na verdade, uma recusa de
sintaxe, e tê-la escrito como limitação do `JOIN` teria sido um falso negativo
sobre o motor. A §Q1b do `§19` mede a forma sem alias de propósito.

### A pergunta mínima, falsificável

> **P1.** Consegue um critério externo `q : X → Y`, **guardado no banco**, ser
> usado pelo motor para agregar o campo externo `G`? Ou seja: o motor deriva
> `G_q` a partir de `q` **armazenado**, e não de `q` escrito no texto da
> consulta?

Falsificável nos dois sentidos: um «sim» mostra `G_q = (7, 5)` e `Σ = 12` com `q`
apenas em linhas de tabela; um «não» é um caminho medido que só funciona com `q`
no texto ou fundido no registo.

> **P2.** Em cada caminho que funciona, onde vive `q`: na base de dados, ou no
> programa?

> **P3.** A medição da conservação é informativa? Construir o detector que
> acumula `Σ G_q = |I|` **sem usar `q`**, mostrar que passa, e depois mostrar que
> uma medição distinta **distingue** um `q` certo de um `q` errado.

`P3` é a que dá conteúdo novo. Sem ela, o INVARIANTE 2 é uma tautologia
disfarçada de verificação.

### Controlos propostos

| | o que muda | o que tem de acontecer |
|---|---|---|
| **C1** — `q` outra função | `a→y1, b→y2, c→y2, d→y1` ⇒ `G_q = (3, 9)` | a **distribuição** muda; a conservação **mantém-se** em 12. Se caísse, a medição estaria errada |
| **C2** — `q` não é função | `b` recebe `y1` **e** `y2` | `Σ G_q = 12 + G(b) = 16`; o detector acusa um excesso de magnitude prevível |
| **C3** — `q` parcial | uma célula de `X` fica sem classe | `Σ G_q = \|I\| − G(x)`; o detector acusa a falta, de magnitude prevível |
| **C4** — detector cego | `G_q` acumulado sem usar `q` | `Σ = 12` **e passa** — é isto que mostra que C1–C3 dão conteúdo à lei |
| **C5** — `G` errado | `G'(b) = 5` | `Σ G' = 13 ≠ 12`; o detector de `G` continua vivo |

Nota sobre C2: a célula `d` tem `G(d) = 0`, logo duplicá-la **não** produziria
excesso nenhum — `Σ` ficaria 12 e o detector não veria nada. O controlo tem de
usar uma célula com fibra não vazia, `b`, senão C2 é vacuo por construção.

### O que esta frente continua a NÃO ser

Não é integração JEV → banco. Não mede nem postula `question = q`, `state = G`,
`confidence = κ`. Não escolhe arquitectura. Não persiste `G_q` nem `p_q` sem
razão experimental explícita. Não altera `banco/sql.c`.

---

## 19.1 O QUE A MEDIÇÃO RESPONDEU

### P1 — RESPOSTA: **NÃO.** O motor não consome um `q` armazenado numa tabela separada

O que torna isto uma medição e não uma recusa de API: **o mesmo motor consome um
`q` armazenado quando ele está na mesma linha que `G`** (veredito A abaixo), e
**não** o consome quando está noutra tabela. O que falta não é somar — é **ler a
coluna da tabela da direita**.

| via | tentativa | resultado medido |
|---|---|---|
| 1 | TEXTO + `JOIN` + `GROUP BY` | **recusada** |
| 1b | TEXTO + `JOIN`, sem a coluna da direita | **ACEITE — e ERRADA**: 1 linha de 4, e `1\|3\|1\|3` são **códigos**, não cadeias |
| 1c | TEXTO, **zero chaves a bater** | **ACEITE — 1 linha inventada** (a junção verdadeira é 0) |
| 2 | **INTEIRO** + `JOIN` | **correcto**: 4 de 4 pares, chaves iguais linha a linha |
| 3 | INTEIRO, ler `ci.y` na projecção | **recusada** |
| 4 | INTEIRO, `WHERE ci.y = 1` | **recusada** — *«o WHERE não foi entendido»* |

Duas precisões que a formulação tinha por-meio e a medição corrigiu:

- **O `JOIN` não está partido.** Com chave `INTEIRO` junta certo, linha a linha. O
  que não existe é a capacidade de **usar** a coluna da direita.
- **`q` fundido no registo ⇒ veredito A, e isso NÃO é uma ponte JEV → banco.**
  `q` deixa de ser campo externo separado e passa a ser uma coluna da linha de
  `G`. Registar A aqui como se resolvesse P1 seria falsear a pergunta: A é uma
  *representação diferente*, não uma resposta à pergunta original.
- **`q` no texto da consulta ⇒ veredito C**, e em C o `q` **não está no banco**.

**Nenhuma das três foi escolhida como arquitectura definitiva.** A frente
registra que as três existem e o que cada uma mede.

### P2 — ONDE VIVE `q`, EM CADA CAMINHO

| caminho | onde vive `q` | **prova** (não inferência) |
|---|---|---|
| classes no **texto** da consulta (`WHERE celula IN ('a','b')` → 7) | **programa** | **`DROP TABLE criterio`, e a mesma consulta devolve 7.** O banco nunca entrou na conta |
| `q` **fundido** no registo (`registo(celula,contagem,classe)`) | **mesma linha de `G`** | **`UPDATE` de três células muda `(7,5)` → `(5,7)`, e Σ fica 12** |
| `q` em **tabela separada** | **inacessível** | a coluna da direita é ilegível na projecção e inutilizável no `WHERE` |

A via fundida é a única com veredito A, **e é a que muda a pergunta**. Sem o
teste do `DROP`, a via do texto pareceria uma resposta a P1 — e não é: o `DROP`
é o que a desmente.

### P3 — A CONSERVAÇÃO, ISOLADA, **NÃO** TESTA SE `q` FOI USADO

Detector cego: soma `G` e reparte por `|Y|`, **sem ler `q`** → dá
`G_q = (6,6)`, `Σ = 12` → **PASSA**.

Logo: **qualquer verificação de `G_q` que só conserve é decorativa.** O INVARIANTE
2 isolado é uma tautologia disfarçada de verificação — confirmado
experimentalmente, não só por argumento. O sinal informativo é a **distribuição**,
e é isso que o C1 mede.

### C1–C5 — RESULTADOS CONCRETOS

| | o que mudou | medido |
|---|---|---|
| **C1** | `q' = (y1, y2, y2, y1)` | **`G_q' = (3,9)`. Distribuição MUDA, Σ fica 12.** Confirmado por dois caminhos: `INSERT` numa tabela nova **e** `UPDATE` na coluna existente (deslocamento de 2 em ambos) |
| **C2** | `b` recebe `y1` **e** `y2` (`q` não-função) | **`y1=7, y2=9, Σ=16 = \|I\| + G(b)`.** Quebra de magnitude **previsível**. O motor não faz nada: o limite de P1 é de **leitura**, não do dado |
| **C3** | `c` (`G=5`) fica sem classe (`q` parcial) | **`Σ = 7 = \|I\| − G(c)`.** A falta é medida, e é da magnitude certa |
| **C4** | detector cego, sem ler `q` | **`G_q = (6,6)`, `Σ = 12`, PASSA** — é isto que dá conteúdo a C1, C2 e C3 |
| **C5** | `G'(b) = 5` | **C dá 13, o banco diz 13, `≠ 12`.** A lei ainda vê um `G` errado |

C2 teve de usar `b` e não `d`, como a formulação já previa: `G(d) = 0`, logo
duplicar `d` não daria excesso nenhum e o controlo seria **vazio por
construção**.

### LIMITES DO MOTOR, MEDIDOS

1. Coluna da tabela da direita: **ilegível** na projecção, **inutilizável** no `WHERE`.
2. Sem `CASE`, sem subconsulta. A «subquery» de `sql.c:8879` é o lado direito do `JOIN`.
3. `JOIN` **com alias**: recusado. Sem alias: aceite.
4. `JOIN` com chave **TEXTO**: aceite e **produz linhas erradas**. Com chave **INTEIRO**: correcto. `celula_valor` (`sql.c:7259`) devolve o par numérico da célula, e a chave de texto casa por **código**, não por cadeia.
5. `GROUP BY … sum` devolve `ncols=3` para uma projecção de 2 itens (`[chave, tamanho, soma]`).

### FALSOS POSITIVOS E FALSOS NEGATIVOS ENCONTRADOS

- **FALSO POSITIVO DO MOTOR — o mais grave desta frente.** `VIA 1c`: chave
  `TEXTO`, **zero** chaves a bater, o `JOIN` é **aceite** e devolve 1 linha. *«Aceita»
  não significa «correcto».* Numa integração, alguém leria `1\|3\|1\|3` como
  dados válidos, e nenhuma excepção seria levantada.
- **FALSO NEGATIVO MEU, na primeira sondagem.** Reli a coluna errada do
  `GROUP BY` e quase registei «o banco não sabe somar». O `ncols=3` apanhou-o.
  Registado porque o erro foi meu e quase passou por facto do motor.
- **ERRO DE EVIDÊNCIA NO CHECKPOINT `77d021eb`** — ver a correcção na §19.2.

### 19.2 CORRECÇÃO DE EVIDÊNCIA — checkpoint `77d021eb`, §G8

**O histórico não é reescrito.** A medição anterior aconteceu como aconteceu; o
que se corrige é a descrição da evidência, não o resultado.

| | o que `77d021eb` afirmava | o que é verdade |
|---|---|---|
| contagem | «**212 ficheiro(s)**» | **contagem incorrecta por duplicação.** O `§G8` percorre três prefixos (`""`, `"../"`, `"./"`) sem deduplicar, e conta o mesmo ficheiro mais do que uma vez. **Não é uma contagem de ficheiros distintos** e não deve voltar a ser apresentada como prova de minuciosidade |
| cobertura | «nenhuma referência a JEV em `banco/sql.c`, `can/` ou `lib/`» | **`can/` NÃO estava presente no workspace remoto** `/root/jev_g_campo` durante aquela execução. A varredura **nunca o cobriu**, apesar do texto o nomear |
| conclusão | «zero ocorrências de `jev`» | **mantém-se**, e por um motivo independente do número: ler um ficheiro duas vezes não muda se ele contém `jev`. A conclusão não dependia da contagem |
| hash do medidor | `1d7af071…` para `tests/campo_g_no_banco.c` | **o hash registado não era o do ficheiro committado.** Era a variante **CRLF** do *worktree* (29496 bytes); o blob é **LF** (28999 bytes), `6ad5a0d1…`. Causa: `core.autocrlf=true` sem `.gitattributes`. **Um clone não reproduzia o número registado** — e a medição corre em Linux, onde o clone dá LF |

Consequência prática, e é a mesma em ambos os medidores: **os SHA-256 que este
mapa registam são sempre da forma LF (a do blob)**, que é a que um clone produz e
a sobre a qual a medição corre. Um checkout em Windows dá CRLF e um SHA
diferente, por bytes idênticos em termos de código — as quebras de linha ficam
entre aspas de strings adjacentes, nunca dentro de uma string, por isso a saída
do medidor é a mesma. Para tornar isto inequívco e parar de se repetir a
confusão, a correcção é uma linha de `.gitattributes`; **fica registada e não
feita**, porque mexer em `.gitattributes` altera o comportamento de checkout de
todo o repositório e é decisão que não é desta frente.

Verificação posterior, à parte, para fechar a lacuna de cobertura:

| directório | ficheiros | ocorrências de `jev` |
|---|---|---|
| `can/` (51 ficheiros `.c`/`.h`) | 51 | **0** |
| `banco/` + `lib/` | — | **0** |

**A conclusão é verdadeira, mas foi apresentada como se a prova a tivesse
cobberto e a prova não a tinha.** A distinção é o que fica registado.

O `§Q8` novo de `tests/q_armazenado_no_banco.c` **conta cada ficheiro uma única
vez** (uma passagem, cada directório uma vez) e **declara explicitamente a
cobertura efectiva**: imprime `AVISO: o directório can/ NAO EXISTE aqui — a
varredura NAO o cobriu`, distingue `lidos` de `ilegiveis`, e diz que *o que não
foi coberto aqui tem de ser coberto noutro sitio, e o resultado registado não
pode dizer que o cobriu*. Um número errado apresentado como minuciosidade é
exactamente o tipo de coisa que esta frente existe para apanhar.

### 19.3 O QUE ESTA FRENTE FECHOU, E O QUE NÃO FECHOU

**Fechou:** que «G + q no banco» são **três perguntas distintas** (texto/programa,
mesma linha de `G`, tabela separada inacessível); que o motor é sensível à
localização de `q`; que a conservação isolada não é teste; e que o motor aceita
uma junção semanticamente errada sem o dizer.

**Não fechou, e não é para fechar aqui:** qual destas representações é a
arquitectura. **Nenhuma foi escolhida.** `q` fundido dá A e é a única que o motor
consome, mas muda a pergunta; `q` separado é o que a pergunta original pedia e é
exactamente o que o motor não faz. Escolher entre os dois é decisão de
arquitectura, e esta frente **não decide arquitectura**.

### 19.4 Relação com `campos.tex` e `fisica.tex` — DOCUMENTADO, não medido

Secção de registo, escrita **depois** da medição e **sem** a alterar. O mapa
declarava `campos.tex` e `fisica.tex` como dependências (§8) sem nunca as ter
usado; aqui ficam três leituras. Nenhuma muda um resultado da §19.1, e uma delas
é uma pista que **não** foi testada.

A escala de estatuto é explícita em cada item: **[LIDO]** é o que o documento
diz, com citação; **[MEDIDO]** é o que o motor respondeu na §19.1; **[NÃO
MEDIDO]** é o que se lê no código e fica por testar.

#### (a) `G_q` já tem nome e lugar formal — [LIDO]

A operação medida na §19.1,

```
G_q(y) = Σ_{x : q(x)=y} G(x)
```

é **exactamente** a *mudança de suporte* de `campos.tex` §6:

| | |
|---|---|
| `campos.tex:346-352` | Definição: `G_T(t) = Σ_{s ∈ f⁻¹(t)} G_S(s)` |
| `campos.tex:354-364` | Proposição *Pushforward composto*: se `π_T = f ∘ π_S` então `G_T = f_# G_S` |
| `campos.tex:366-372` | Observação: mudança de suporte **e** transformação de representação `T` **não se identificam** |

Tomando `f = q`, `S = X` e `T = Y`, tem-se `G_q = q_# G`. **Isto é o que
importa para a §19:** o experimento não inventou uma operação para testar o
banco. O que a §19 mediu foi se o motor **realiza uma mudança de suporte de um
campo de contagem por uma classificação externa** — e a resposta medida é
específica:

| representação de `q` | [MEDIDO] §19.1 |
|---|---|
| `q` separado de `G` (outra tabela) | **não** |
| `q` fundido na mesma linha de `G` | **sim** |
| `q` no texto da consulta | funciona, mas `q` **não está no banco** (`DROP` mantém o resultado) |

**[MEDIDO]** E o mais consequente: o motor realiza `f_# G` **mal declarado**
(fundido na linha) e **não** o realiza **bem declarado** (tabela separada). É a
mesma operação, com duas representações, e a representação decide se funciona.

Isto **não** demonstra ponte JEV → banco: fundir `q` na linha de `G` é mudar a
representação do problema, não ligar o banco ao JEV.

#### (b) A conservação é o extremo; a distribuição é a trajectória — [LIDO], e explica o [MEDIDO] P3

`campos.tex:309-318` (Definição *Levantamento*) define a coordenada de ocorrência

```
k(i) = |{ j ∈ I : j ≤ i, π(j) = π(i) }|
```

`k(i)` é uma **soma-prefixo por fibra**: conta as visitas à célula de `i` até `i`.
Duas propriedades suas:

- `k` é injectiva **dentro de cada fibra** — `campos.tex:326`: os índices da fibra
  ordenados `i₁ < … < i_g` satisfazem `k(i_r) = r` ([LIDO], `campos.tex:320-327`)
- o **valor final** de `k` na célula `s` é `g = G_S(s) = |π⁻¹(s)|` — **consequência
  imediata** de `k(i_g) = g` com `G_S(s) = |π⁻¹(s)|` (`campos.tex:104-108`). O
  `campos.tex` não afirma isto por extenso; decorre das duas definições.

Daí a leitura, e ela explica o P3 medido sem ser pós-hoc:

| | | |
|---|---|---|
| **conservação** `Σ_y G_q(y) = |I|` | o contador de prefixo **acaba** no tamanho da fibra — é uma propriedade de **extremo** (`campos.tex:145-154`) |
| **distribuição** `G_q` | o que o contador faz **no caminho** dentro da fibra | |

**[MEDIDO]** §19.1 / P3: o detector cego, que não lê `q`, dá `G_q = (6,6)` com
`Σ = 12` e **passa**. A razão está nesta distinção: ele verificava **só o
extremo**, e o extremo é compatível com um detector que nunca leu `q`. O C1 é o
que exige a trajectória: `q → (7,5)` e `q' → (3,9)`, com `7+5 = 3+9 = 12`.

Ou seja: **a conservação sobrevive à mudança de `q`; a distribuição responde a
`q`.** É a formulação mais curta do motivo de o INVARIANTE 2, isolado, ser
decorativo.

#### (c) O motor tem uma soma-prefixo — **[NÃO MEDIDO]**

`banco/sql.c` contém `ACUMULA` e `DELTA`, que o proprio motor descreve como a
convolução com `ζ` e `μ` ao longo da ordem:

| | |
|---|---|
| `sql.c:91-93` | *«O `ACUMULA`/`DELTA` do WHERE são ζ e μ ao longo da ordem»* |
| `sql.c:15118-15123` | *«`SELECT ACUMULA(col) … [ORDER BY …]` é a convolução com ζ ao longo da ordem — a soma-prefixo»*; `DELTA(ACUMULA(x)) = x` |
| `fisica.tex:3894` | *«a soma-prefixo, `μ` a diferença finita — ζ∘μ = μ∘ζ = id»* |

**[NÃO MEDIDO]** Se as classes de `q` fossem contíguas na ordem, cada `G_q(y)`
seria um incremento da soma-prefixo, e a conservação seria o valor final dela. É
uma leitura plausível, e **nada disto foi testado nesta frente**: `ACUMULA` não
foi executado, e o §19 não afirma que o motor consiga por esta via o que não
consegue por tabela separada.

Duas condições que a leitura impõe, e que valem como objecção antes de ser
experimento:

1. `ACUMULA` actua sobre **uma coluna de uma tabela**, ao longo de uma ordem. Para
   usar `q` seria preciso ordenar pelas classes — isto é, precisar da coluna da
   classe, que é precisamente o que a **VIA 4 da §19.1 mediu como recusado** no
   `WHERE`. A soma-prefixo parece ser caminho para a representação **fundida**,
   não para ler a tabela separada.
2. O argumento das classes contíguas exige que `q` seja uma **partição em
   intervalos** da ordem. Um `q` arbitrário é uma partição qualquer. Isto é uma
   **restrição ao alcance da hipótese**, não um resultado.

**[NÃO MEDIDO]** Fica registado como **questão futura**, e deliberadamente não
tratado aqui: introduzir `ACUMULA` como variável nova contaminaria a §19, que está
consolidada e é sobre *onde vive `q`*. A fronteira actual, em uma frase: **o
motor tem uma ferramenta de soma-prefixo, e ainda não foi demonstrado que ela
forneça a ponte externa `q ↦ G_q`.**