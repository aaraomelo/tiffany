---
name: feedback-a-definicao-do-extractor
description: "Publiquei «15 de 22 teoremas citam medidor» — mas isso era a definição do meu extractor (procurava um nome de ficheiro), não um facto sobre o paper. Os 22 estavam todos medidos."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T19:51:36.547Z
---

Escrevi um extractor para varrer `corpo_universal.tex` e contar quais teoremas citam
medidor. Ele procurava `\code{tests/…}`. Resultado publicado, numa asserção verde e numa
fala da assistente: **«22 teoremas etiquetados, 15 citam medidor, 7 não»** — e as sete
falas correspondentes diziam «ESTE TEOREMA NÃO CITA MEDIDOR no paper».

**Falso nos sete.** Eles citam por **secção** (`§D2–D3`, `§S0–S2`, `§C0–C7`), por
**contagem** (`100/100 transições na diagonal, 100/100 fora`), ou trazem a medida **dentro
do enunciado** (`4096/4096 pares, varredura completa`). Os **vinte e dois** estavam medidos.

## E foi a SEGUNDA vez no mesmo dia

A primeira versão do extractor tinha a classe de caracteres sem o `.`, e disse
«SEM MEDIDOR» em **22 de 22**. Apanhei essa — um total de 22 em 22 é sinal de ferramenta
partida ([[feedback-dois-caminhos]], ler o TOTAL). Corrigi a regex, deu 15/22, e
**publiquei**. O segundo erro passou *porque o número deixou de ser absurdo*.

## O gatilho

**Uma medição sobre um corpus mede a minha consulta, não o corpus.** Antes de publicar um
número extraído por programa, escrever a frase que ele realmente suporta:

    o que o programa mediu:  «quantos blocos contêm a cadeia \code{tests/…}»
    o que eu escrevi:        «quantos teoremas citam medidor»

Não são a mesma frase. E o teste barato: **abrir dois ou três dos que caíram do lado
negativo e ler**. Eu tinha os sete listados por nome e não abri nenhum.

Corolário: quando o extractor divide um conjunto em dois, o lado **negativo** é o que
precisa de leitura manual — o positivo já se explica sozinho.

Ver [[feedback-assercoes-vazias]], [[feedback-o-medido-sem-medidor]],
[[feedback-o-medidor-que-nunca-mediu]].
