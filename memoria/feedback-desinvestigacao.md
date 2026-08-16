---
name: feedback-desinvestigacao
description: "O paper final não conta como a matemática foi descoberta. Se a frase responde «como descobrimos isto?», sai."
metadata:
  node_type: memory
  type: feedback
---

Um revisor externo leu o `geometrico.tex` **como texto matemático** e o diagnóstico foi:
«ainda há uma diferença importante entre **formalização consolidada** e **rastro da
investigação**». O paper estava cheio de frases como «uma versão anterior deste paper…»,
«a primeira redacção afirmou de mais», «e o erro estava no pai», «…que era invenção do
autor».

**Why:** isso é excelente para commit, log e relatório de auditoria — e é veneno no paper.
O leitor não precisa de saber que houve uma versão errada; e a retratação enfraquece o que
devia ser uma definição. Comparar:

- *rastro*: «E a forma ESCOLHE-SE — não é a única. Uma versão anterior dizia… é falso.»
- *paper*: «A forma bilinear faz parte dos dados estruturais e deve ser especificada. Não
  há unicidade sobre 𝔽₂ⁿ. Adopta-se a forma padrão, para a qual a base é ortonormal.»

A segunda é **mais forte**. Não parece retratação: parece definição.

**How to apply — a regra, que é do revisor e é a melhor que já vi para isto:**

> Se a frase responde **«como descobrimos isto?»** → SAI.
> Se responde **«por que isto é verdade?»** → FICA.
> Se responde **«como verificámos a implementação?»** → vai para a COLUNA DE VERIFICAÇÃO.

E os três verbos com sentido fixo, que resolvem quase tudo sozinhos:
**demonstra** (deriva das hipóteses) · **realiza** (instancia a construção) ·
**verifica** (execução finita). Nunca «mede completude», «o teste prova», «concorda,
portanto demonstra».

**E o corolário que eu tinha ao contrário:** achar um erro num paper-pai NÃO é
«o pai estava errado». O Aarão corrigiu-me: «não foi erro, foi a INTERPRETAÇÃO que
precisou aqui da Lei 0 com Möbius — **faltou lá**». O ajuste certo é nos DOIS: tirar a
retratação do filho e **acrescentar ao pai a interpretação que faltava**.

Ver [[project-a-reta-construida]], [[feedback-o-escopo-da-afirmacao]].
