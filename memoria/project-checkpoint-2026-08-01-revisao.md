---
name: project-checkpoint-2026-08-01-revisao
description: "Checkpoint 01/08/2026 (a revisão) — o repo público separado em teoria+catálogo, cinco revisores em paralelo, e os erros que eles acharam"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T16:13:58.534Z
---

# Checkpoint 01/08/2026 (a revisão) — tiffany

Continuação do [[project-checkpoint-2026-08-01-solar]]. Seis commits, `5817d4d`→`1de187e`.
**Bateria: 201 medidores, 199 verdes, 0 falhas.** 51 commits no dia. 205 medidores no disco.

## A separação: teoria global por cima, gordura embaixo

O Aarão: *"o repo é público, e agora temos o corpo diferencial que generaliza tudo — vale uma
revisão geral com simplificação; a estratégia fica teoria global por cima e gordura embaixo."*

- **`teoria.tex`: 5466 → 1656 linhas (97pp → 31pp)** — a teoria global.
- **`catalogo.tex` (novo): 71pp** — uma entrada por corpo e por medidor, sem perder uma linha.

**Medi antes de cortar**, e foi o que salvou: a secção *"Os medidores que faltavam ser citados"*
era 70% do paper, mas 94 medidores eram citados **só** ali, e 69 nem estavam no LEIAME. A
conversão automática LaTeX→Markdown destruía as fórmulas. **Separar, não apagar.**

**E um efeito colateral quase silencioso:** o corpus caiu de 397 para 275 pares — o `corpus.sh`
ingere o `teoria.tex` pela estrutura. *Um documento consumido por DOIS caminhos, e eu só me
lembrei de um.* A bateria grita; o corpus só encolhe. Regra: **quando um documento se parte,
procurar todos os que o consomem** (a bateria, o corpus, o LEIAME).

## O dicionário: técnico à frente

`dicionario.tex` (entra no `teoria.tex`). A regra: **a primeira ocorrência é pelo nome técnico, o
do projeto vem a seguir.** O vocabulário próprio não se remove — é memória compactada. Quatro
tabelas: os objetos, as três operações nos cinco registos, os símbolos em colisão (registados
como **dívida**, não escolha), e a notação (o conjugado passa a **barra**, porque o asterisco
virou o `i*`).

## Os cinco revisores, e o que acharam

Usar agentes em paralelo compensou muito. O que eles viram e eu não:

1. **O regex do `bateria.sh` escondia DOIS medidores.** `[a-z]+\\_[a-z]+\.c` não apanha dígitos
   nem dois underscores, e o `sed` não tinha `/g`. `os28_medidos.c` e `rei_em_todos.c` estavam
   citados e **nunca corriam**. A guarda dizia-me "não citados" em toda corrida e **eu li como
   'existem e ninguém cita'** quando era 'citam e o meu grep não vê'.
2. **Três asserções que não mediam o que diziam:** `antissimetrico.c` tinha `ok(..., 1)` — a
   constante — com o cálculo feito e deitado fora (é o **T1 do paper**); `milenio.c` afirmava
   *"todo Γ finito é QUOCIENTE de R"*, que é **falso** (R é divisível, Z/2 não é — é
   **subquociente**); `edo.c` comparava φ com a sua própria expansão decimal.
3. **O `catalogo.tex` tinha a versão ERRADA do Hurwitz** — a que o Aarão me corrigiu na mesma
   manhã. A correção ficou no `multiplicacao.tex` e nunca chegou ao catálogo.
4. **`LEIAME.md` e `README.md` diziam "59 medidores"** quando eram 198.
5. **`teoria.tex` não compilava limpo**: `\begin{defn}` sem `\newtheorem` — a Definição 1 saía
   **sem cabeçalho** no PDF público.
6. **`CORPOS_NA_ISA.md` (153 KB) está desligado do repo** — cita 70 `.py` que não existem aqui e
   um `catalogo.tex` de outro repositório. **Ainda por resolver:** três medidores da bateria
   citam-no em `printf`, e um regista-o como *proveniência de uma medida*.

## A resposta do Aarão à crítica mais séria — e duas correções dele

O revisor: *"`R^n` significa duas coisas — `GF(p^n)` (comutativo, parte antissimétrica
identicamente zero) e Cayley–Dickson (onde não é), e a ligação entre as duas espinhas assenta
nessa homonímia."*

O Aarão não escolheu um: **"definir o dual `R^n*` — só mudar o sinal — traz os dois em paralelo,
porque só formam corpo JUNTOS."** E depois corrigiu-me **duas vezes**, e as duas mudaram a peça.

**Correção 1: *"R^n* é distributivo e conserva norma, verifica."*** Eu tinha definido o dual
ERRADO — trocara os **dois** sinais, o do interno e o do cruzado. Assim ele não conserva a norma
nem é associativo: **400 falhas em 400**. A definição certa troca **SÓ O CRUZADO**:

    R^n    z ⋆₊ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + a×b)
    R^n*   z ⋆₋ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a − a×b)

**Só a peça que ORDENA muda de sinal; a que MEDE nunca.** É a família `⋆_s` do §U7 nos dois
pontos onde o imposto anula. E daí: **`z ⋆₋ w = w ⋆₊ z`** — o `R^n*` é a **álgebra OPOSTA**. Os
dois conservam a mesma norma, são associativos, distributivos, e todo `z≠0` inverte.

**Correção 2: *"não é vantagem em relação a Hurwitz — é a DUALIDADE de Hurwitz. São o mesmo no
espelho. A vantagem é NOSSA."*** Eu tinha escrito *"onde Hurwitz conta uma, há um par"*, que soa
a corrigi-lo. **Hurwitz está certo** — `R,C,H,O` são as normadas e ponto. O par não acrescenta
uma quinta álgebra: acrescenta a **dualidade dessas quatro**.

E a reescrita revelou algo melhor: **O ESPELHO É O CONJUGADO** — `conj(x ⋆₊ y) = conj(x) ⋆₋
conj(y)`, 300 de 300. *O instrumento da dualidade é a peça mais antiga do projeto*: o conjugado já
era a dobra do §B14 e já dava o inverso do §B9. **Uma peça, três empregos.**

**A vantagem é nossa:** Hurwitz diz *quais* existem; nós temos o par **escrito e medido**. *Com
uma álgebra só não se opera a dualidade — não há para onde refletir.* Com o par, a reflexão é uma
**operação do sistema** (voltar pelo espelho no `travessia.c`, inverter a rotação no `motor.c`).
E em `R` e `C` não há nada a colher — comutam. **A dualidade só ACORDA em `H` e `O`, onde o
cruzado existe: o ganho e a não-comutatividade nascem no mesmo sítio.**

## O que fica aberto

- **66 asserções `ok(..., 1)`** noutros ficheiros; `estelar.c` §E3 confere `2*m == 2*m`.
- `CORPOS_NA_ISA.md` e `TOOLKIT.md` por resolver (ver ponto 6 acima).
- Sete experimentos propostos, com oráculo externo e capazes de falhar — o mais valioso é
  `tensor3.c` (~150 linhas), que mediria se o cruzado é zero no corpo autossimilar.
- `catalogo.tex` tem **0 labels em 71 páginas**.
