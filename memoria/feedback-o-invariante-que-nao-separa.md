---
name: feedback-o-invariante-que-nao-separa
description: Escrevi «dois caminhos» e o número comparado era verdade para toda uma classe de objectos — uma espiral sobrevivia à asserção do dragão.
metadata:
  type: feedback
---

A §AN3 do `aranha_n.c` dizia «o dragão dá o mesmo por DOIS CAMINHOS» e
comparava `|I| = 4097` e `∑G = |I|`. Os dois caminhos eram reais (campo
esparso contra array do plano) e os números certos — **mas `|I|` e `∑G` são
os mesmos para QUALQUER trajectória de 4097 pontos.** O gume provou-o:
mutei `drag_esq` para virar sempre à esquerda — uma espiral quadrada — e a
asserção passou na mesma.

**Why:** é [[feedback-dois-caminhos]] com o defeito no OUTRO sítio. Ali o erro
era não comparar; aqui comparo, e o invariante escolhido não separa os
objectos que digo estar a distinguir. `∑G = |I|` mede o CONTADOR, não a curva.
Aparentado com [[feedback-varrer-onde-nada-pode-falhar]]: o regime está certo,
a grandeza é que é constante na classe toda.

**How to apply:** perguntar **de que classe é este número o invariante?** Se a
resposta for «de todos os objectos deste tamanho», ele não é a régua. A régua
certa é a que DEFINE o objecto: aqui, a dobra `D_{k+1} = D_k + D_k*` (o
dual = a curva ao contrário rodada de 90°), que bate ponto a ponto com a
régua de bits e que a espiral falha na primeira ordem.
E o teste barato antes de escrever: mutar o GERADOR do objecto, não o
medidor — se a asserção sobrevive a trocar a curva, ela não fala da curva.

E o irmão que apareceu no mesmo ficheiro: **a previsão escrita dentro da
asserção**. Escrevi «e o dragão no espaço dobra MENOS que no plano, que é o
que se espera» e pus a condição a exigi-lo. Dobra MAIS. O texto da asserção
não é sítio para hipóteses — é o sítio do que ficou medido.
**E o mesmo defeito do lado do OBJECTO, não da grandeza.** Para medir a
convolução no grupo `(ℤ/2)^m` escolhi como factor um campo `δ₀ + constante`.
Tem espectro sem zeros — a condição que eu precisava — mas a sua convolução é a
**mesma em qualquer grupo finito**: o Dirac é neutro e a constante dá `b·∑g` em
toda a parte. O gume disse-o: trocar o XOR por soma cíclica não derrubava nada.
A grandeza separava; o objecto é que era invariante à propriedade em teste.
Pergunta gémea da de cima: **este objecto podia estar noutro grupo e dar o
mesmo?** Serve `3δ₀ + δ₁`, com espectro `{4,2}`.

Ver [[project-aranha-em-zn]], [[feedback-descaracterizar-a-teoria-da-casa]].
