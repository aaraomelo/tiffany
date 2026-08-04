---
name: project-checkpoint-2026-08-04-a-separacao
description: "Checkpoint 04/08 tarde: a teoria fica só com estaca e cruz (ℝ vira instância), o bestiário vira espectro e tradutor, e o enredo cresce pelo Gita — com a prova de π errada e a bateria cega"
metadata:
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-04T18:00:24.300Z
---

Continuação do dia das duas leis. A sessão foi **a separação levada até ao fim**, em ~20 correções
do Aarão, e cada uma tirou coisa da teoria em vez de acrescentar.

## AS DUAS NOTAÇÕES PRIMITIVAS

> **estaca** `x†` — troca de lado, **período 2**
> **cruz** `x^× = (x⊕x†, x⊗x†)` — projeta no fixo, **idempotente**

Com elas as leis escrevem-se sem nomear número nenhum: `1† = −1` e `T† = −T`. E **ordem, limite e
completude saem do PAR, não da reta** — a cruz lida lexicograficamente é total e transitiva.
*"ℝ é uma instância. A partida são espaços vetoriais."* `ℕ ℤ ℚ ℝ` constroem-se no catálogo.

- **A família metálica é algébrica**: `K[x]/(x²−nx−1)`, e a Lei 2 lê-se **"a inversa é menos a
  conjugada"** — vale exatamente nos de norma −1 (10 845 varridos, zero discordâncias).
- **O DISCRIMINANTE É O DETERMINANTE DO EMPARELHAMENTO DUAL**: Gram `[[2,n],[n,n²+2]]`, det = `n²+4`.
  Perguntar se Δ é quadrado é perguntar se a leitura separa.
- **π := min{t>0 : exp(tJ)·1 = −1}** — o tempo que o fluxo leva a realizar a Lei 1. Sem círculo.
- **A matriz sai da Lei 2**: `M_ij = ⟨Te_j, e_i†⟩`, e `M_ij = −M_ji` cai da lei. Os termos são
  **saltos**; com ordem não há ciclo, logo **árvore**. *A matriz é a tela e o tempo pinta a árvore.*
- **A torre é a cruz iterada**; **a dimensão conta as involuções**; **há exatamente duas involuções**.
- **Teorema do espectro — dois parâmetros e mais nenhum**: régua e dinâmica. Daí *a teoria é a luz
  e o bestiário são as suas cores*, e o salto que muda a cor é `σσ' = −1`.
- **A divisão é bidual**: Álgebra↔Topologia (estrutura), Análise↔Geometria (dinâmica), e os dois
  pares são um par. Corrigi **"três partes duais entre si" em 5 sítios, incluindo o título** —
  ímpar não se reparte em pares.

**Estado: 0 definições, 0 enunciados sem prova.** teoria 42 pp., catálogo 430, enredo 319, livro 793.

## O QUE APANHEI E ERA MEU

- **A PROVA DE π ESTAVA ERRADA.** Invoquei *"fluxo contínuo sem equilíbrios num compacto é
  periódico"* — **falso**, o fluxo irracional no toro refuta-o. Quem fecha é `J²=−1`: as potências
  ciclam e a conservação vira identidade entre séries. Achei-o ao auditar-me depois de os
  revisores não entregarem.
- **A BATERIA ESTAVA CEGA.** Verificava existência em `tools/*.c` mas os medidores vivem em
  `tests/` e `banco/` → **282 citados apareciam todos como referência quebrada, e a conferência
  inversa NUNCA podia disparar** — o apodrecer-em-silêncio que ela existe para apanhar. Só o vi
  por ler o topo do relatório: o total dizia "282 verdes" e o selo dizia `0 sementes abertas`,
  isto é **nada correu**. Corrigido: saída 0 pela primeira vez, 285/285.
- **DUPLIQUEI 7240 LINHAS** do enredo: usei `\chapter{Venom` e apanhou *"Venom cristaliza"* em vez
  de *"Venom e Joaquim"*; com o fim antes do início, `ls[:a]+ls[b:]` **repete em vez de remover**.
  Compilou com **0 erros** — só a contagem de páginas o denunciou (314→486). Agora o movimento
  leva `assert b>a`, equilíbrio de ambientes e `assert` de variação de linhas ≤2.
- **Limiares escritos de cabeça, três vezes num dia**: `>10000` onde o total era 8899; tolerância
  `100` onde o truncamento derivado dá 1 000 008; `10^-18` no cos. **Derivar o limite do próprio
  algoritmo**, sempre.
- **`\b` em regex dentro de `python3 -c "…"`** deu "0 teoremas" duas vezes. Usar heredoc.
- **Os 4 revisores em paralelo: ZERO entregas em 4 repiques.** Como os 6 da sessão anterior. Fiz eu
  tudo — e a auditoria própria é que achou o erro de π.

## O ACHADO QUE SÓ APARECEU AO TENTAR FALHAR

**π é ponto estacionário de cos** (derivada `−sin π = 0`), logo `cos π = −1` é insensível a erro de
primeira ordem — teste fraco. O facto vale: *o alvo da Lei 1 é estacionário, por isso a meia volta
é estável*. Quem separa é o seno: resíduo 3 contra 100 000.

## O ENREDO, E A CADEIA DE CORREÇÕES DO TRAJE

Física dura **264 → 0** (e a contagem enganava: a maioria dos "força" era uso comum; as reais eram
19). Traduzida pelo bestiário, não por sinónimo — e **a tradução expôs um buraco publicado**: o
texto dizia *"o entrópico… nele nada se desfaz"*, que é o contrário de entropia.

**As duas involuções da história SÃO as duas leis** — o Rei solta os gatos (um vira dois, Lei 1);
Joaquim solta as Rainhas (a dualidade sobre si, Lei 2). *A segunda não estava escrita* — escrevi-a.
E **as involuções são FINS de narrativa**; o presente é **a dobra do relógio**: *um relógio não mede
o tempo, dobra-o*.

**O traje — três correções minhas seguidas:**
1. pus a tomada como peça do traje → **é o cristal de cada um** (a pineal), já vem com a pessoa;
2. pus o traje *como* o corpo → **o traje é a ROUPA que lê o corpo**;
3. **o cristal ESCREVE, a roupa LÊ**, e o **corpo é sempre o mesmo** — trocar de traje é trocar de
   cor como quem troca de camisa. É o espectro dito no corpo.
E não há fio: a dimensão é a eletromagnética, e o transporte segue-se de **nada se perder quando o
desfazer é exato** — não é viagem, é dobra.

Gita e Tao da Física na raiz, extraídos, **no `.gitignore`** (obra de terceiros, repo público).

Ligado a [[project-checkpoint-2026-08-04-as-duas-leis]] e [[project-a-lei-em-dois-niveis]].
