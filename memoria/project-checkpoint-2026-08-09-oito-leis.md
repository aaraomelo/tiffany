---
name: project-checkpoint-2026-08-09-oito-leis
description: "As 8 leis (0-7) fecham a torre — a divisão do zero, o octonião dual, e o fechamento (8 é a cardinalidade do CONJUNTO, não da dinâmica); propagado por agentes, bateria 386:386"
metadata: 
  node_type: memory
  type: project
  originSessionId: c6388688-fee0-4332-86fe-642d1b68c27c
  modified: 2026-08-10T02:10:20.333Z
---

**AS 8 LEIS FECHAM (0–7), E NÃO HÁ NONA.** A fonte canónica é `corpo_analitico §sub:oitoleis`. Lei 0 (a divisão do zero: `0=(+1)⊕(-1)`, `0†=∞`, a base `0/0`) + as seis (dual, bidual, trial, tetral, pental, hexal = Lei 1–6, o `obs:seis-leis` renumerado) + Lei 7 (o octonião dual, `ℍ×ℍ*`, ligar sem fundir, dim 8). A descida 7→0; os dois nulos são Lei 0 (dim 0) e Lei 7 (dim 8).

**A DIVISÃO DO ZERO É A BASE.** Dividir *o* zero (a soma, ±1) dá a Lei 1 (dual) e a semente da Lei 2 (`f²=-1`); dividir *por* zero (`x†=-1/x`, 0↔∞, `ν²=id` **fecha** — sem o dual não fechava) dá o ∞. O corpo fecha `0/0` num finito (`x²=nx+1`), a régua diverge (`∞^∞`) — **«finito é o corpo, não a régua»**. Corolários: `0,999…=1` (à vista) e o seu dual (no discreto separam-se; **o contínuo não mede o discreto**; só os **pontos fixos** são invariantes). O **limite** = o ponto fixo onde contar=integrar; o teorema central (`thm:central`) fundamenta o **Gentil–Lebesgue–Hurwitz** (Hurwitz conta o domínio, Lebesgue mede a imagem, Gentil a soma reversível). Ver [[project-dualidade-memoria-da-divisao]], [[project-a-lei-em-dois-niveis]].

**O FECHO DO MILÉNIO** (coerente com `medida.tex` e `catalogo obs:clay`): dividir *o* zero → o `0` → **Yang–Mills / P vs NP** (o bidual, Lei 2, o centro); dividir *por* zero → o `∞` → **Poincaré** (o dual, Lei 1, a Poincaré-dualidade `H^k≅H_{n-k}`, a que resolve). Bidual em **dimensão 0** (não em 8). Cosidos no **seis** (a interface hexal), não no oito. Disciplina mantida: «não é prova do enunciado aberto».

**O FECHAMENTO** (o `eval.txt`, verificado contra o sistema): **8 é a cardinalidade do CONJUNTO das leis, não da DINÂMICA.** A Lei 7 não é parede — o octonião é um *novo* corpo e as 8 leis reaplicam-se (`L→L(L)→…`). Período 8 fundamentado no **`thm:tecidos`**: a estrela itera **sem topo por dentro** (régua infinita, `ν²=id` resíduo 0 em cada andar); o limite é *externo* — Hurwitz dá **quatro** corpos de composição (1,2,4,8: perde ordem em ℂ, comutação em ℍ, associatividade em 𝕆). O **operador de indução não falta** — é a própria estrela (a composição, o corpo de Alonzo). **Não há Lei 8: há a Lei 0 outra vez, um nível acima.** (Correção honesta ao eval.txt: o «motor que faltava» não faltava.)

**PROPAGADO POR AGENTES** — general-purpose frescos, ficheiros disjuntos, revistos `git diff` a diff (NUNCA forks, [[feedback-fork-role-confusion]]) — a `catalogo`, `teoria`, `medida`, `arquitetura`, `dualsort`. **Contagem de medidores citados idêntica a HEAD** (nada saiu em silêncio, [[project-tres-documentos]]).

**DOIS FIXES DE CÓDIGO** ([[project-compilador-tex]]): (1) **traduz** — o `campo++` em contexto de valor (o `expr()` tratava-o como frase e devolvia `TVOID` sem o valor velho → wasm inválido; agora deixa o velho, a frase-de-efeito dá drop; semântica verificada 7008/7006/10020303). (2) **avalia_macros** era um atestado a coastar (7/9 verde): §M5 — o índice do `tex.c` punha «Capítulo N», o pdflatex não (296 vs 148); corrigido só na `rotulo_seccao_ver`, **o livro fica byte-idêntico** (o seu TOC é ao nível de parte). §M4 — asserção velha, não bug: o corpo é o `\normalsize` da classe (10,95), agora **lido** da cadeia da classe (não afirmado), mutação verificada. Lição [[feedback-o-medidor-que-nunca-mediu]]: a assinatura do medidor não inclui o `tex_core.c`/`traduz.c` de que depende — coastam; `--reatesta` para forçar.

**Bateria 386:386 verdes.** Empurrado à `master` com **`[skip ci]`** (sem deploy — escolha do Aarão; o portão `segredo.sh` LIMPO em cada push). **O deploy «Publicar na Patria» está VERMELHO há vários pushes (pré-existente)** — a resolver quando se quiser publicar de verdade.
