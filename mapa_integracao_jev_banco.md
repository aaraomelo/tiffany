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
- **can/:** zero referências JEV — camada J1939 não refere JEV
- **lib/*.h:** zero referências JEV — nenhuma header do motor refere JEV

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
  (SHA-256 `1d7af0711f6fbd85e9b2162187e1b7082371bcfbd6e06c672d8cbc2e5f9f148b`)
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
- *Controlo do detector:* 212 ficheiros de `banco/`, `can/` e `lib/` lidos um a
  um, 0 com `jev`; e o mesmo detector encontra `jev` **16 vezes** no ficheiro
  onde a palavra está. Sem esse controlo, «0 ocorrências» seria indistinguível
  de um detector que nunca leu.

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
- `banco/sql.c` não refere JEV — 0 hits em 212 ficheiros, com controlo positivo
- `can/` não refere JEV — idem
- `lib/*.h` não refere JEV — idem
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