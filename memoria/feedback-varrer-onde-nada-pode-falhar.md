---
name: feedback-varrer-onde-nada-pode-falhar
description: A varredura corria só no regime de esforço alto, onde já não há indecisos — o caminho onde o defeito vive nunca era exercitado, e a mutação sobrevivia.
metadata:
  type: feedback
---

Medi «os quatro métodos concordam» varrendo 22960 racionais com **esforço 14**. Zero
choques, verde. Depois mutei a bisseção para **fingir que decidiu** quando ainda está
indecisa — e a asserção **sobreviveu**.

Porquê: com esforço 14 a caixa é tão apertada que nenhum racional da varredura lhe cai
dentro. O ramo do INDECISO — o único onde a mentira aparece — **nunca corria**. Eu tinha
varrido 22960 casos e nenhum deles podia falhar.

**Why:** é a [[feedback-assercoes-vazias]] com muitos zeros à frente. O tamanho da
varredura enganou-me: 22960 × 6 pares parece exaustivo, mas exaustivo **dentro de um
regime onde o defeito não existe** é o mesmo que não medir. E o volume torna-o pior, não
melhor, porque dá confiança.

**How to apply:** antes de dar por medido, perguntar **em que regime é que o defeito
viveria**, e varrer LÁ — normalmente é o regime *pobre* (esforço baixo, dados curtos,
precisão pequena), não o rico. A cura foi varrer quatro esforços (1, 2, 5, 14): os
baixos trazem 11952 indecisos e a mutação morre na hora.

**A forma mais traiçoeira: o objeto SIMÉTRICO.** Medi «normal ⟺ gH = Hg» em ℤ₁₂, que é
ABELIANO — lá todo subgrupo é normal e os dois lados coincidem sempre. A varredura era
completa e não distinguia nada; apagar a conjugação sobrevivia. O mesmo com «corpo ⟹
domínio tem um sentido só» varrido em ℤₘ, onde vale a equivalência. **Quando o teorema
afirma uma ASSIMETRIA, o exemplo tem de ser assimétrico** — S₃ em vez de ℤ₁₂, ℤ em vez de
ℤₘ. Ver [[project-algebra-moderna-sete-ticks]].

**E a terceira forma: a TESE SEMPRE VERDADEIRA.** Sobre o produto euclidiano,
Cauchy–Schwarz nunca *pode* ser falsa — logo uma implementação que devolvesse sempre «sim»
era indistinguível da correta, e a mutação sobrevivia. A cura foi **generalizar a função
à forma**: alimentada com a indefinida, ela tem de devolver «não». Quando a tese é sempre
verdadeira no domínio testado, **a função que a decide não está a ser testada**.

E o segundo defeito da mesma família, no mesmo sítio: **«0 choques» podia ser um detetor
avariado**, e um detetor avariado dá sempre 0. Um contador de desacordos precisa de um
**caso forjado** que o obrigue a disparar — sem isso, o zero não distingue «concordam» de
«não estou a olhar». Ver [[feedback-o-medidor-que-nunca-mediu]].
