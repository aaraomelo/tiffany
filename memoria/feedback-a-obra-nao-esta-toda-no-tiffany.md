---
name: feedback-a-obra-nao-esta-toda-no-tiffany
description: "Varri o tiffany, concluí «falta X», e X estava em broca-so/papers/ — 615 .tex fora do repo, e foi ele que mos apontou duas vezes seguidas."
metadata:
  node_type: memory
  type: feedback
---

Em duas rodadas seguidas concluí que uma peça faltava, e ela estava escrita:

- **`matrix.tex`** nomeia o **gato** `(1,1;1,0)` e o **gambá** `(0,1;−1,0)` — as
  duas matrizes que eu media há horas sem lhes chamar nada — e declara **três**
  regimes das potências. Eu tinha dois; faltava-me `(A+G)^k = A+G`, a projeção
  idempotente.
- **`algebra_estelar.tex`** define `E_s` com `e² = 1−s`, que é exatamente o
  parâmetro da tríade que eu tinha estado a reconstruir pelo discriminante.

Nos dois casos foi o Aarão que apontou o ficheiro. Eu tinha varrido — mas só o
`tiffany/`.

**A conta:** fora do `tiffany` há **615 `.tex`**, e os que fundamentam esta obra
estão em dois sítios concretos:

```
broca-so/papers/    47 .tex   matrix, omnitrix, corpo_estelar (765 linhas),
                              corpo_dual (1881), equacoes_diferenciais, joalheria…
estelar/             9 .tex   algebra_estelar, torre_hurwitz_estelar,
                              geometria_estelar, transformada_estelar…
```

E `corpo_estelar.tex` e `corpo_dual.tex` **não têm cópia no tiffany**. O
`corpo_topologico.tex` tem duas versões (4166 linhas aqui, 114 lá — evolução, não
divergência).

**Why:** eu leio «o repo» como `tiffany/` porque é onde corro a bateria. Mas a
bateria mede a REALIZAÇÃO; a teoria que ela realiza está espalhada, e a memória
[[feedback-inventei-em-vez-de-procurar]] já dizia que «a cadeia não vive só em
`papers/`» — só que eu li isso como «também na raiz do tiffany».

**How to apply:** antes de escrever «isto falta» ou «isto é novo», varrer
`~/Documentos/{broca-so/papers,estelar}` além do repo. Um `grep -rl` nesses dois
custa um comando e já me poupou duas invenções. E quando ele nomeia um paper
(«matrix», «omnitrix», «corpo estelar»), é um ENDEREÇO —
[[feedback-inventei-em-vez-de-procurar]] outra vez, agora com o alcance certo.
