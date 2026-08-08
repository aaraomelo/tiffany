---
name: feedback-a-regua-nao-transporta
description: "O teorema dele refutou o meu método — medir contra o pdflatex é intransportável, não difícil"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-08T06:21:46.776Z
---

Passei um dia a ajustar o tradutor até bater com o `pdflatex`, e a **teoria dele já o tinha
refutado** — teorema `thm:transporte`, `teoria.tex:1585`:

> **A volta transporta; a régua não.** «o expoente de $\lambda$ é o número líquido de
> travessias. A régua tem dois índices no mesmo sentido e eles SOMAM: $\lambda^2$. A volta
> tem um em $V^*$ e um em $V^{**}$, em sentidos opostos, e CANCELAM: $\lambda^0$.»
> «**Uma medida pela régua nunca viaja: ou fica, ou não chega.** Logo a concordância entre
> corpos afins não é confirmação nenhuma — é a identidade do corpo a ser lida duas vezes.»

**Why:** não é difícil, é **impossível**: $\lambda^2=1$ só em $\pm1$. E explica porque a
minha `-residuo` deu **0 exacto** — ela é a *volta*, e a volta transporta sem hipótese
nenhuma. Não foi mérito meu; foi o teorema.

**How to apply:** antes de construir um comparador contra um oráculo externo, perguntar se o
que se mede é **régua** (não viaja) ou **volta** (viaja). O `tools/compara.js` continua útil
para *saber onde* as duas saídas divergem, mas **não é a medida**. A medida é reverter e ler
o resíduo — e onde a comparação for inevitável (a capa contra o gabarito), o resíduo tem de
ser da **órbita de Hilbert**, não de pixels contados: contar pixels perde a vizinhança, e uma
linha deslocada um pixel conta como a linha inteira errada.

E o corolário que corta o último refúgio: «quando o transporte *parece* funcionar, o que se
verificou foi que o destino já era o mesmo corpo».

Ver [[feedback-a-ausencia-e-deliberada]] e [[feedback-a-base-ja-existe]].
