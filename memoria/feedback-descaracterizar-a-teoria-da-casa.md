---
name: feedback-descaracterizar-a-teoria-da-casa
description: Reescrevi o teorema da casa na forma que EU conheço — troquei uma cláusula e transformei um autómato num agente que delibera.
metadata:
  type: feedback
---

Ao escrever `papers/aranha.tex` fiz duas coisas ao `thm:multiplicidade` da casa:

1. **Troquei a cláusula 4.** A da base é «**sentir é ler G** — a leitura local
   `P_t(x)` sobre `V(x)` distingue `G=0` de `G>0`; a distinção é propriedade do
   **espaço**, não propriedade cognitiva». Pus lá a **conservação** `∑G=|I|`,
   que na casa é `§AG5` e não é cláusula nenhuma. E a seguir escrevi «só a
   cláusula 4 usa a dimensão» — frase que só é verdadeira com a cláusula CERTA.
2. **Transformei o ciclo em «algoritmo + opcional».** A base diz, à letra: «cada
   passo do autómato **é** uma cláusula do teorema, **não um extra**» — Escrita ·
   Leitura · Decisão · Fecho. Eu escrevi dois passos e um terceiro marcado
   *«(opcional, o agente) ler a vizinhança e decidir o passo seguinte»*. Isso é
   um agente deliberativo — exactamente o que a teoria NEGA. A decisão é o
   **gradiente**: o mínimo de G no que está escrito no chão. E o **fecho**
   (`R⁴=id`, o mesmo `J⁴=id` da bidualidade) desapareceu do meu texto.

Ele: «colocou VARREDURA num autómato estigmérgico», e depois «leia a base de
papers e faça as provas com base na teoria daqui, não descaracterize tudo pra
caber nas suas réguas».

**Why:** não foi ignorância da base — eu tinha-a lido. Foi reescrevê-la na forma
que me é familiar: um algoritmo com um laço e uma heurística opcional. A forma
canónica que eu trago apaga precisamente o que é a contribuição. É irmão de
[[feedback-a-base-ja-existe]], mas pior: ali eu acrescentava maquinaria a mais;
aqui **substituí** a estrutura por outra, e o resultado passa a dizer outra
coisa sem que nada falhe — o paper compila, o medidor fica verde.

**How to apply:** antes de escrever um enunciado que a casa já tem, **copiar as
cláusulas dela para o lado e conferir uma a uma**, pela ordem e pelo nome.
Se um passo estiver marcado «opcional» ou «extra» no meu texto, procurar na base
se ela diz o contrário — aqui dizia, com essas palavras. E a pergunta que
apanha isto sozinha: *o que é que o agente sabe?* Se a minha versão lhe dá mais
do que a marca no chão, descaracterizei.

E o encadeamento é o que faz o estrago: a cláusula trocada tornou plausível uma
segunda afirmação («só a 4 usa a dimensão») que eu não teria escrito com a
cláusula certa à frente. Ver [[feedback-verdadeiro-e-parcial]].
