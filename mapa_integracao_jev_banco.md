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

**HIPÓTESE (não demonstrada):**
- G poderia ser persistido como tabela de ocupação sobre realizações
- Uma query poderia computar G_q como derivado de G
- q poderia ser metadata de pergunta no schema
- lib/racionais.h e lib/linear.h são partilhados mas não pontes

**SEM PONTE:**
- Nenhuma integração JEV → banco demonstrada
- sql.c não refere JEV (0 hits)
- can/ não refere JEV (0 hits)
- lib/*.h não refere JEV (0 hits)
- q-JEV ≠ q-Mat2Q (critério de classificação ≠ parâmetro de polinómio)
- state JEV ≠ G (não identificados)
- question ≠ q (só ⋛)
- confidence ≠ κ

## 18. Nota de controlo

Este mapa regista o que existe e o que não existe.
Não implementa. Não assume equivalências.
A frente JEV → banco só avança quando houver uma pergunta mínima
mensurável — e essa pergunta ainda não foi formulada.