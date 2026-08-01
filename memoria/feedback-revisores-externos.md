---
name: feedback-revisores-externos
description: Lançar revisores em paralelo acha o que eu não vejo — e o pior que eles acharam foi um aviso que eu lia todos os dias e interpretava ao contrário
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T16:14:34.419Z
---

# Revisores em paralelo acham o que eu não vejo

Em 01/08/2026 o Aarão autorizou agentes para rever o repo antes de o expor. Cinco em paralelo:
papers menores, alinhamento dos `.md`, clareza e prosa, lacunas e experimentos, nomenclatura e
símbolos. **Compensou muito** — e o que eles acharam diz mais sobre como eu falho do que sobre o
repo.

## O pior: eu lia o aviso todos os dias e interpretava-o ao contrário

A bateria imprimia, em **toda** corrida:

    nao citados (existem, nenhum paper cita, logo NAO sao testados): 2
        tools/os28_medidos.c
        tools/rei_em_todos.c

Eu li isso dezenas de vezes como *"existem no disco e nenhum paper os cita"*. Era o contrário: os
papers **citavam-nos**, e o regex do `bateria.sh` é que não os via — `[a-z]+\\_[a-z]+\.c` não
apanha dígitos nem dois underscores, e o `sed 's/\\_/_/'` não tinha `/g`. Dois medidores nunca
correram, e um deles passa com 4 unidades.

**A lição não é sobre regex.** É que uma mensagem de diagnóstico repetida vira paisagem. Eu tinha
uma explicação plausível ao primeiro dia e nunca a testei nos noventa seguintes. **Quando um
aviso aparece em toda corrida, verificar a explicação uma vez — não aceitá-la para sempre.**

## O que mais acharam, e que eu não veria sozinho

- **`ok(..., 1)` com o cálculo feito e deitado fora** (`antissimetrico.c`, e é o T1 do paper). Eu
  tinha caçado três destes na mesma sessão e escrito um memory sobre isso; havia **66** noutros
  ficheiros.
- Uma afirmação **matematicamente falsa** publicada como unidade verde (*"todo Γ finito é
  quociente de R"* — R é divisível, Z/2 não é).
- Uma asserção a comparar um valor **com a sua própria expansão decimal** três linhas acima.
- Uma correção que o Aarão me deu **na mesma manhã** e que ficou num paper e nunca chegou ao
  outro.
- O `teoria.tex` a **não compilar limpo**, com a Definição 1 a sair sem cabeçalho no PDF público.

## Como pedir uma revisão que sirva

O que funcionou, e vale repetir:

1. **Dar o contexto duro primeiro** — o que é o repo, o que é um medidor, o que "resíduo 0"
   significa. Sem isso o agente critica o estilo.
2. **Dizer explicitamente o que NÃO fazer:** *"não edites nada, só analisa e reporta"*. Todos
   respeitaram, e isso deixa-me decidir o que aplicar.
3. **Nomear o risco concreto:** *"a bateria monta a lista fazendo grep dos nomes nos papers; se um
   paper deixar de citar um medidor, ele sai da bateria em silêncio"*. Os cinco relatórios
   marcaram a vermelho exatamente as propostas que tocavam nisso — e um deles achou o bug do regex
   por causa dessa instrução.
4. **Pedir a crítica franca:** *"o dono prefere a crítica à cortesia"*. O relatório mais duro foi
   o mais útil.
5. **Verificar sempre com o critério medível.** Antes e depois de cada mudança: a lista de
   medidores tem de ficar idêntica. `diff` da lista, não confiança.

## E o que não delegar

A **decisão** do que aplicar. Dos ~40 achados, apliquei os que eram erro (falsidade, código que
não compila, número errado) e deixei em aberto os que eram julgamento — fundir papers, apagar
ficheiros, renomear objetos. Um revisor propôs apagar `CORPOS_NA_ISA.md`; três medidores da
bateria citam-no em `printf` e um regista-o como *proveniência de uma medida*. Isso não é uma
limpeza, é uma decisão sobre o que conta como fonte.

Ver [[feedback-assercoes-vazias]], [[project-checkpoint-2026-08-01-revisao]].

## 01/08 (noite): dois revisores derrubaram metade de um resultado do próprio dia

E foi o mais rentável que já lancei. O primeiro achou uma **afirmação falsa que a reordenação de uma
hora antes tinha promovido a primeira frase do paper**; o segundo mostrou que a correção do primeiro
**tirava o chão** a outra metade do resultado.

**O que os tornou úteis, e vale repetir:**

1. **Contexto duro E o problema concreto.** Não pedi "revê isto": disse qual era a mudança pendente,
   porque estava a ser feita, e o que temia. Os dois foram direitos ao ponto.
2. **"Não edites nada, só analisa e reporta."** Um deles viu a reordenação acontecer a meio do seu
   trabalho e **reescreveu o relatório como revisão do feito** em vez de proposta — mais útil.
3. **Pedir contra-exemplos explicitamente.** Ao segundo pedi: *"esta unificação é mesmo unificação,
   ou é eu a dar o mesmo nome a coisas diferentes? procura contra-exemplos no próprio repo"*. **Foi
   daí que veio o achado maior** — ele mediu e trouxe as normas fora do círculo.
4. **Eles apanham asserções vazias que eu acabara de escrever.** Duas, no medidor daquele dia: a
   soma de `±1` comparada com `N` (identidade), e um ponto fixo por identidade algébrica. *Eu tinha
   registado as oito formas horas antes.*

**E o que eu fiz mal, e é o padrão a vigiar:** corrigi a teoria e o medidor e **não toquei no
catálogo** — que continuou a publicar a frase falsa. *Quando uma correção toca uma afirmação, procurar
TODOS os documentos que a repetem.* É a mesma lição do corpus que encolheu.
