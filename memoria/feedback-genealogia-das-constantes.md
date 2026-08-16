---
name: feedback-genealogia-das-constantes
description: Cada constante apresenta a sua genealogia ou sai fora — e π está na casa, exacto por andar.
metadata:
  type: feedback
---

**«Cada constante tem de apresentar a sua genealogia no Universal ou sair fora.»** Um
literal decimal na fonte é uma afirmação **sem proveniência**. As quatro genealogias que a
casa reconhece — e as três primeiras obrigam o valor a **derivar-se**:

- **LIMITE** — π é o membro **parabólico** da família metálica (`thm:pi-familia`):
  fronteira, não constante operacional.
- **RECORRÊNCIA** — φ, σ_m, √2, √3: têm passo inteiro por trás (`p_{k+1} = p_k + p_{k−1}`),
  e o valor sai dele. Escrevê-lo é copiar o que se pode gerar.
- **DEFINIÇÃO** — `c = 299792458` é exacto no SI: um inteiro disfarçado de decimal.
- **EXPERIMENTO** — uma medida do mundo. Honesto, mas tem de ser **declarado**: um número
  medido não é um número derivado.

**E π NÃO É O CASO HONESTO — ele está na casa, exacto por andar.** Duas vezes escrevi que
π justificava um double, e duas vezes estava errado. O `thm:pi-familia`: os polígonos são
os membros **elípticos** e fecham em ordem `2n` **exacta** no primo da ordem (√2 é 11 em
𝔽₁₇, √3 é 9 em 𝔽₁₃); os metais são os hiperbólicos; o círculo é o **parabólico** `t = 2`,
que **nunca fecha em ordem da escada**. `π_n` é exacto; `π_∞` não fecha **por teorema**.
«Pedir o valor exacto é pedir o membro-limite» — a idealização do redondo **sem dinâmica**.

**O caso sem desculpa** é o literal que é o *resultado* de uma conta que o programa podia
fazer. Encontrado em `tests/escada.c`: comparava com `0.742742944625` e um limiar `1e-11`
a segurá-lo — e esse número é `φ^(1−φ)`, dito noutro ficheiro da própria casa. **A
referência escrita à mão reintroduz o defeito dentro da asserção que devia medi-lo.**

`tools/genealogia.sh` (191 literais auditados) · `tools/triagem_limiares.sh` (918) ·
`tools/audita_tipos.sh` (3616 doubles). Ver [[feedback-a-referencia-escrita-a-mao]],
[[feedback-o-limiar-tem-tres-causas]], [[project-escada-paga-uma-fibra]].
