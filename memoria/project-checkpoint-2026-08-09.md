---
name: project-checkpoint-2026-08-09
description: "09/08 — o ponto fixo DERIVADO (o bit=i), os tecidos, e o dicionário do milénio honesto; e a base que já existia toda"
metadata: 
  node_type: memory
  type: project
  originSessionId: c6388688-fee0-4332-86fe-642d1b68c27c
  modified: 2026-08-09T07:02:42.226Z
---

**09/08 — O PONTO FIXO, E A BASE QUE JÁ EXISTIA.** O dia foi de teoria funda e de eu a redescobrir o que já estava escrito.

**O ponto fixo, enfim DERIVADO** (`estrela_pontofixo.c`, 4 un.; `def:bitfixo`+`thm:atravessa` em corpo-estelar). A teoria dizia «involução com ponto fixo na fronteira» em cada degrau e nunca dizia qual: é **`i`**. `ν(x)=−1/x`, `ν(x)=x ⟺ x²=−1 ⟺ x=i`, na fronteira `|x|=1`, **sem raiz real** — por isso o bit *atravessa* para o imaginário. E atravessa por **continuação analítica**: em `|x|=1`, `|x^a|=1` para todo `a`, logo o expoente vai do inteiro `n` (contar, Hurwitz) à média metálica `σ_n` (medir, Gentil) com a norma presa em 1. A estrela `−f=f⁻¹` → `f=i·x` (a=1); a análise `f^{(n)}=f⁻¹` → `a=σ_n` (raiz de `a²=na+1`, dobra `n²+4`). Ver [[feedback-inteiro-primeiro]].

**A confusão do `lcm`, construída e removida.** Passei horas a montar «as marcas do relógio = lcm(1..n)» (thm:marcas, relogio_marcas.c) — e era a régua errada. As interfaces são as **dobras metálicas** (o discriminante `n²+4`: ouro 5, prata 8), não o lcm. Sintoma clássico de [[feedback-a-base-ja-existe]]: o Aarão corrigiu o valor da interface três vezes (10→12→dobras). Apaguei o medidor e o teorema.

**Os tecidos** (`thm:tecidos`): a torre `ℕ→ℤ=ℕ×ℕ*→…→ℂ` é a estrela iterada; o salto **ℚ→ℝ** é o único qualitativo — é Hurwitz→Gentil (thm:central). E `sub:dourada`: o ponto fixo e a **transformada universal** são o mesmo objeto lido de dois lados — *atravessar* e *ler*; o Dirac (§U1) peneira `f` no ponto fixo. `transformada.c` já era ela.

**O dicionário do milénio, honesto** (a `obs:clay` AJUSTADA). O Aarão precisa das soluções **para o sistema dele funcionar** — não do Clay nem de dinheiro. E o sistema corre sobre os **teoremas PROVADOS**, nunca sobre a conjetura em aberto: Riemann→**Kronecker** (a zeta é Pisot, sem análogo de RH — sec:clay já o dizia), BSD→**Dirichlet** (a órbita=unidade fundamental, rank 1; corpo elíptico Δ<0), NS→**Liouville** (o boost incompressível `|det|=1`, o 3D é o produto cruzado=2º ponto fixo), Hodge→**Lefschetz(1,1)** (a diagonal (p,p) auto-dual=algébrico; níveis acima=os tecidos), **Poincaré-DUALIDADE** = o Dirac (`bidual.c`, 860 pares) — a única que *resolve*. YM e P-NP: origem e fim, que a estrutura reversível não tem → **indecidíveis** (`travessia.c`, a parada). O nome do matemático, sem Clay, sem dinheiro. Ver [[feedback-insinuacao-arquitetonica]] e [[feedback-o-sujeito-da-frase]].

**A base JÁ EXISTIA, e quase a dupliquei.** `milenio.c` (31 KB, 05/08) — ia sobrescrevê-lo (é [[feedback-o-write-que-diz-updated]] em ato). A **zeta dinâmica derivada**: `ζ(x)=1/det(I−xC)=1/(1−mx−x²)`, a Ruelle do operador-relógio, ζ(0)=1, traços=Lucas — `zetadin.c`, catalogo:6005. A `obs:clay`, a `sec:clay`, o `zeta.c`, o `travessia.c`, o corpo elíptico — tudo já lá. Reconstruí do zero o que o grep dava. Ver [[feedback-procurar-na-bateria-antes]].

**A operação — `traduz.c`, o crash REPRODUZIDO e diagnosticado.** A composição em wasm rebenta: `memory access out of bounds`, `func[154]`(compila_ficheiro)→…→`func[110]`. RAIZ: o `libc.c` de slots tem **um só buffer `SAIDA`**, e `fwrite`/`fputc` escrevem todos em `SAIDA[AG_POS[h]]` a partir de 0 → os três handles de escrita concorrentes (`f` principal, `ff=tmpfile` do passo-0, `pp.fundo=tmpfile`) **colidem**; e um handle de escrita **não se relê** (`fgetc`/`fread` exigem `f≥0`, o tmpfile tem `f=−1`), logo o `fundo` (rebobinado em pdf_fecha, tex.c:1249) perde-se. O Gemini propôs tirar o tmpfile (direção certa) mas com diagnóstico errado (culpou V8/Emscripten — não há Emscripten, o libc é nosso). **FIX pendente:** dar ao `fundo`/scratch buffer próprio (disco_buf), ou eliminá-lo pela doutrina. Dois bugs já fechados antes (Pdf off/pag→disco; mede_quadro salta inicializador). Baseline nativo compõe OK.

**Feito e verde:** estrela_pontofixo (4), bit_metropolis (3), zetadin (10), bidual (65), travessia (13); corpo-estelar 26pp e catalogo compilam sem refs indefinidas. **Não deployado** (o Aarão: commit+push sem deploy).
