---
name: feedback-inventei-em-vez-de-procurar
description: "Ele disse «Julia» e «métricas»; eu escrevi uma derivação CONTÍNUA de Euler (fluxos, cosh, e^{∂θ}) na Def. 1 do aranha. A discreta já estava em teoria.tex §sec:euler."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: cee0686a-c852-4d69-8fca-ca206e1fba24
  modified: 2026-08-22T00:58:17.774Z
---

Pediu a métrica circular de **Julia**. Eu inventei um fluxo `γ' = ∂γ`, as
séries de `cos`/`cosh`, e enfiei tudo na Definição 1 do `aranha.tex` — o
documento travado, na porta de entrada.

**Já estava derivado, em três sítios, e nenhum era o meu:**

- `teoria.tex` §`sec:euler`, `thm:euler`: `J²=−1`, `exp(tJ)=c·1+s·J`,
  `c²+s²≡1`, e «as potências ciclam: `J⁰,J,−1,−J`, e depois repetem» — o
  período 4, **discreto**.
- `corpo_algebrico.tex` `thm:cantor-julia`: Cantor é o shift `θ↦2θ`
  (aditivo), Julia é `z↦z²` (multiplicativo), conjugados por `e^{2πiθ}`;
  `φψ=−1`; e o que os separa é a **conservação** — «ciclar não é uma
  propriedade do operador, é o que sobra quando o orçamento deixa de poder
  ser gasto».
- `teoria.tex` (Pascal): `C(n,k)=C(n−1,k−1)+C(n−1,k)` **é o passo da torre**
  `A_{n+1}=A_n⊕A_n†` lido como contagem; a simetria é a involução; e em
  por-unidade o máximo cai em `k=n/2`, **o ponto fixo**, onde a régua custa
  menos. Medido em `tests/pascal.c`.

**Porquê:** ele diz um nome próprio — «Julia», «Pascal», «corpo estelar» —
e eu leio-o como *tema* em vez de como *endereço*. É uma citação, não uma
sugestão: aponta um teorema que existe.

**Como aplicar:** quando ele nomear alguém, `grep -rn` pelo nome ANTES de
escrever uma linha. E lembrar que a cadeia não vive só em `papers/`:
`teoria.tex` e `catalogo.tex` estão na RAIZ, e o *Corpo estelar* está em
`app/dist/corpo/papers/corpo-estelar.tex`.

É a quarta vez do mesmo padrão de [[feedback-a-base-ja-existe]], agora com
a agravante de escrever no paper travado. Ver também
[[feedback-procurar-na-bateria-antes]] e [[feedback-o-aranha-tex-esta-travado]].
