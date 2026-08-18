---
name: project-checkpoint-2026-08-17-o-espaco
description: 17/08 — O ESPAÇO BASE Q(m√D) e o seu DUAL com as quatro propriedades; a transformada universal é a passagem à base do dual; a dourada discreta é a dourada na borda; migração 102→67 em nove ficheiros. 508:508
metadata:
  type: project
---

**508:508, 0 falhas. `geometrico.tex` 55 páginas.** Dois medidores novos: `transformada_universal.c` (24:0) e `cuspide.c` (11:0).

## O que ficou estabelecido

**O ESPAÇO É `Q(m√D)`**, anunciado no abstract e definido na fundação (`def:espaco`): espaço VECTORIAL sobre ℚ de dim 2, base {1, m√D} — a primeira dobra da torre `dim A_n = 2ⁿ dim A_0` do [[project-torre-hurwitz-gentil]] (`thm:rn` do corpo_topologico). **O DUAL não é a conjugação**: é o outro lado com a SUA RÉGUA, `m† = −1/(mD)`, que sai de ν(x) = −1/x aplicado à folha. `(m†)† = m`, `s·s† = −1`, e `V† = V` — o mesmo corpo, régua invertida. **Uma é o CONE, a outra a ESPIRAL** (a `obs:area` já o dizia das direcções próprias; eu reescrevi em vez de remeter).

**AS QUATRO PROPRIEDADES**, duas de cada lado: (P1) √D∈ℚ ⟺ D quadrado — o espaço EXISTE; (P2) m≠0 ⟹ m√D∉ℚ — tem RÉGUA; (D1) α=α* ⟺ α∈ℚ — o TRAÇO; (D2) α·α*∈ℚ — a NORMA. Uma sai, a outra volta. Oito teoremas centrais passaram a nomear qual delas usam.

**A TRANSFORMADA UNIVERSAL É A PASSAGEM À BASE DO DUAL** — {eval₊,eval₋} é base de V*, e o que a torna invertível (det = −2s) É a (P2). Não acrescenta objecto nenhum. Só existe UMA: «discreta» e «rápida» são redundância, e saíram do corpus.

**A DOURADA DISCRETA é a dourada NA BORDA**: σ²=mσ+1 faz de σ unidade de ordem N finita, os caracteres são σ^k, o Dirac dá N·δ, e a inversa corre por σ⁻¹ = −σ†. **O factor é N e não √N** — o √N é do lado ADITIVO (raízes no círculo, m=0), e as nossas folhas são RECÍPROCAS. Zero «DFT» no corpus.

**NENHUM PASSO NORMALIZA**: o mmc é a leitura do texto para ℤ; quem normaliza é a NORMA (o produto das folhas, que é o primeiro passo); o gcd saiu do `rt_ciclo` — medido nos 71 medidores que usam `reta.h`. mmc/gcd correspondem às duas metades NA FORMA (produto conservado) mas não no valor: mmc·gcd varia, s·s† é constante. **O gcd converte-se**: invariante da descida ↔ a norma, invariante do produto.

**PRIMO → IRRACIONAL é DISCRETO → CONTÍNUO** (`thm:duas-propriedades`), pelo lema de Euclides. Primo é SUFICIENTE, não necessário — a condição exacta é (P1). [[feedback-verdadeiro-e-parcial]]

**A CÚSPIDE é o TRIAL** — ver [[project-a-cuspide-e-o-trial]].

## A migração — 102 → 67 em nove ficheiros

`cosmico` 16→9 · `liga` 15→11 · `colheita` 14→11 · `encanamento` 12→9 · `dif` 10→8 · `simula` 10→5 · `dominios` 9→1 · `llm` 8→6 · `nne` 8→7.

O método está no `cursor.txt` §2. O que se repetiu: **quase nenhum limiar era da física** — um era do passo da grelha, outro era margem de quinze ordens, outro era `cabs` sobre um terno pitagórico. Peças que valem: o pico de Wien sem formar a raiz; Friis exacto em ℚ; `(a/b)¹⁰>10` por enquadrar 10^(1/10) entre racionais; a norma multiplicativa medida onde os ternos são pitagóricos; o RMSNorm onde a raiz CANCELA.

**A regra que o Aarão deu e que destravou o resto**: *«quem precisa de testemunha? o corpo auto-atesta»*. Eu mantinha o double na condição como segundo caminho, e enquanto o mantive não vi que a inversão dos NV era uma igualdade de inteiros (os √3 cancelam) nem que o zero da Lorentziana era estrutural. Ver [[feedback-o-double-que-so-transportava]].

## As duas regras para o próximo (do `eval.txt`)

- **Não preservar aproximação quando existe fechamento exacto no corpo.**
- **Não transformar descoberta de implementação em camada teórica nova se já é consequência da Transformada Universal.**
