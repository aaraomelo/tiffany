---
name: project-checkpoint-2026-08-13-pipeline
description: "13/08 — IR Claim no ar (v0.5–v1.6): Claim≠Result, Controlo acima, 9 LMS, fronteira Lei 7, estação, banco, portão Pátria. DeployPatria fecha (GET 200). Aarão adianta corpus; estrutura pára aqui."
metadata:
  node_type: memory
  type: project
  originSessionId: 24684222-009a-48be-807f-96d4008d229a
  modified: 2026-08-13T22:54:00.000Z
---

**13/08 — O PIPELINE FECHA NO AR.** Spec `conecthus/pipeline.tex` v1.6. Commit `1d8af30` em `master`. Workflow Publicar na Patria verde. Live: `https://goldenkingdom.patriatechnology.com/corpo/conecthus/pipeline.tex` e `/wasm/claim.wasm` — HTTP 200. `tools/patria.sh --live` OK.

**O que a IR é.** Fonte = `.claim` (lei, objecto, passo, volta, medidor, invariante, mutação, classificar). **Claim ≠ Result.** `residual=` no ficheiro é recusado. `STEP→BACK→MEASURE→R`; `CLOSE := R=0`. Paper / teste / código / wasm são projeções. LLM **não** é o Maestro (extrai Claims; não projecta π_k nem atesta λ⁺+λ⁻=0).

**Camadas (não colar).** IR (parser `conecthus/lang/`) → executor `core/execute.c` → Controlo **acima** (`control.c`: RETAIN/MOVE/RETRACT; 8 eixos 𝒱; sem keyword `control` no `.claim`). Fronteira = resíduo **entre** Claims (Lei 7), não fusão. Rede dual P/ℋ/H ≠ Controlo C ≠ Fronteira F ≠ Estação E ≠ Banco B ≠ Pátria D.

**Versões fechadas nesta cadeia («segue» = a entrega seguinte da spec):**

| v | fecha |
|---|--------|
| 0.5 | Controlo wasm, 𝒱 |
| 0.6 | chip C |
| 0.7 | eixos texto reais (`eixos_texto.c`) — 8 eixos |
| 0.8–1.0 | 9 ferramentas LMS + PipelineClosure como `.claim` |
| 1.1 | Fronteira: GUT↔5W2H, Why↔Ishikawa, PDCA↔VSM |
| 1.2 | `tools/pipeline.sh` + acaso §K8 |
| 1.3 | chip F ao vivo |
| 1.4 | Estação R=\|P−M\|; fecha só com MES; réu não fecha (`estacao.claim`) |
| 1.5 | Banco: documento `implante P= M= R= fonte=MES\|reu`; `emit∘parse=id`; **não pesos** (`ob:banco`) |
| 1.6 | Portão Pátria: local (corpo tem pipeline.tex) contra live (GET). **0∧0 não fecha.** Réu (sem fetch) não fecha. |

**Chips na assistente:** C Controlo · F Fronteira · E Estação · B Banco · D Pátria. Falas: `gut:60,48 ordem:0,1` · `lead:65 estacao:65` · «mostra o banco» · «mostra o deploy».

**Wasm `claim_run` ids 0–14** (`backends/claim/runtime.c` → `claim.wasm`; DISCO, NULO=8). Wasm claim **não** liga `fronteira.c` (snprintf): roundtrip wasm de Banco/Deploy é ints na arena; prova de emit/parse é C. `tests/backends_wasm.js` #TOTAL 26 0 (na hora do fecho).

**Corpus.** `tools/pipeline.sh` (aprende inventário + fronteira + controlo + banco + Pátria). `papers/conversa_pipeline.tex`. O Aarão **adianta o corpus com o Claude**; volta quando for avançar **estrutura**. Não inventar a próxima entrega — a spec v1.6 não aponta nenhuma depois do push.

**Contratos que não ceder.** Interface sem estado. Banco = RAG (par fala/resposta), não fine-tune. Estação: referência do réu (`external=0`) nunca fecha. Deploy: afirmar no ar sem GET nunca fecha. Nenhuma linguagem-backend privilegiada: C subset → `traduz` → wasm. Nenhum binário no git (excepto wasm medido em `assets/figuras/wasm/`). Não commit/push sem pedido — desta vez o Aarão disse «segue» depois de o portão pedir o push.

**Onde está.** `conecthus/{pipeline.tex,claims/,lang/,core/,backends/}` · `app/src/{controlo,fronteira,estacao,patria,assistente}.js` · `tests/claim_{ir,control,fronteira}.c` · `tests/{controlo,fronteira,estacao,patria}_front.js` · `tools/{pipeline,patria,sobe_backends_wasm}.sh` · health `publica.yml`: `/corpo/conecthus/pipeline.tex` e `/wasm/claim.wasm`.

Ver [[project-publicacao-patria]], [[project-deploy-sem-github]], `conecthus/fundamento.tex` `ob:banco` `ob:residuo` `ob:controlo`.
