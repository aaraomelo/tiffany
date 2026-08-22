---
name: feedback-o-numero-no-veredicto
description: "Escrevo o texto do ok() com os números ANTES de correr — três vezes num dia, e a bateria passa verde na mesma porque o veredicto é uma string."
metadata:
  type: feedback
---

Escrevo o bloco, escrevo o `ok("… 15/15 … 30 coeficientes …", mal == 0)`, corro, e
os números reais são **10/10 e 20**. A bateria dá **verde**: a condição é
`mal == 0`, e o texto é uma *string* que nenhuma asserção lê.

Três vezes no mesmo dia: §W94 («15/15» → 10/10, «30» → 20), §W98 («100 de 108» →
34 de 192), §W103 («143/143» → 77/77, «3220 … 876» → 2401 … 1695).

**Why:** o `\medido` e o veredicto são o que o paper cita e o que fica no commit.
Um número errado ali é uma **afirmação falsa publicada**, e o gume não a alcança
por construção — o medidor mede o objecto, não o que eu disse sobre ele. É a
[[feedback-a-referencia-escrita-a-mao]] a reaparecer no sítio onde menos se vê.

**How to apply:**
1. **O veredicto escreve-se DEPOIS de correr.** Deixar o `ok()` com o texto
   qualitativo, correr, ler os contadores, e só então pôr os números.
2. **Nunca antecipar uma contagem** «óbvia»: `n=2..6 × m=1..3` parecem 15 e são 10
   quando o regime muda; 4096 triplos parecem repartir-se de uma maneira e
   repartem-se de outra.
3. **Ler a linha do medidor e a linha do veredicto lado a lado** antes de commitar
   — é uma comparação de dois segundos e apanha tudo.
4. E o mesmo vale para o `\medido` do `.tex`: ele copia os números do veredicto, e
   o erro propaga-se do medidor para o paper sem ninguém o ler.

Ver [[feedback-o-medido-sem-medidor]], [[feedback-assercoes-vazias]] e
[[feedback-o-grep-que-nao-conta]] — os três são a mesma família: o instrumento
certo a atestar a coisa errada.
