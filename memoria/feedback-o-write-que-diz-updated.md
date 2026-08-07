---
name: feedback-o-write-que-diz-updated
description: "Criei tests/roupa.c com Write sem o ler — o ficheiro já existia e media outra coisa. Nenhuma asserção o apanhou: foi a CONTAGEM da bateria subir uma e não duas"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-07T05:17:03.744Z
---

07/08/2026. Escrevi `tests/roupa.c` do zero, com `Write`, para medir o vestir das escalas físicas.
**O ficheiro já existia** — media se um corpo vestir outra roupa o torna diferente dos outros (não
torna; o Δ é invariante em 9 261 transportes), tinha 114 linhas, 3 unidades, e estava **citado no
catálogo** desde uma sessão anterior. Escrevi por cima dele.

## Como foi apanhado, e como NÃO foi

- **Não** foi por asserção: os meus 6 medidores passaram todos, verdes.
- **Não** foi pela bateria acusar referência quebrada: a referência continuava a existir.
- **Não** foi pela leitura: eu nunca abri o ficheiro.
- **Foi pela CONTAGEM.** Acrescentei dois medidores a 314 e o total deu **315**, não 316. Um só
  número fora do lugar.

## O sinal que eu tinha à frente e ignorei

O resultado da ferramenta dizia **`has been updated successfully`**, não *created*. Está lá, em
texto, em cada `Write` sobre ficheiro existente. Eu li aquilo como confirmação de sucesso.

**Regra:** ao escrever um ficheiro novo, se o resultado disser *updated*, parar e ler o que lá
estava. E antes de `Write` num caminho que eu não abri nesta sessão: `git cat-file -e HEAD:<path>`
ou `ls` — custa uma linha.

## E aconteceu mais DUAS vezes no mesmo dia, uma delas sem aviso nenhum

Horas depois, na mesma sessão e já com esta memória escrita: `tests/protocolo.c` (as seis fases da
túnica contra o painel) e `tests/metades.c` (*«não há "não é corpo": há metade de corpo»*). Ambos
existiam, ambos citados, ambos sobrescritos.

O primeiro deu `updated` e eu **vi** — a memória tinha funcionado. **O segundo não deu aviso
nenhum**, porque o escrevi com `open(...,"w")` dentro de um bloco Python, e por aí não há harness a
avisar. Foi apanhado só pela contagem: acrescentei um medidor a 320 e a bateria continuou a dizer
320.

**A regra corrigida, e é esta que vale:** verificar *antes* de escrever, sempre, e não esperar pelo
aviso — porque nem todos os caminhos o dão. `Write` avisa; `open(...,"w")` em Python, `>` em bash e
`cp` não avisam nada.

E o padrão dos nomes é o mesmo das três vezes: `roupa`, `protocolo`, `metades` são **vocabulário
dele**, com significado já fixado. Um nome que descreve bem o que estou a fazer descreve
provavelmente bem algo que ele já fez.

## E o nome não era coincidência

`roupa` é vocabulário **dele**, já com significado fixado no catálogo: *roupa é tudo o que muda com
a base; corpo é o que não muda*. Escolher um nome do vocabulário do projeto e assumir que está livre
é a mesma falha de fundo que trazer régua de fora — [[feedback-a-base-ja-existe]]. O meu ficou
`tests/vestir.c`, que é o outro lado da mesma distinção: dada a separação, QUAL roupa se veste.

Recuperado com `git checkout HEAD -- tests/roupa.c`, porque estava commitado. Se não estivesse,
tinha-se perdido em silêncio — como em [[feedback-destruir-antes-do-inventario]], e desta vez sem
agente nenhum a avisar-me.

**O teste que funciona:** a contagem total da bateria, antes e depois. Ver [[feedback-dois-caminhos]]
— quem sai, sai em silêncio, e só o total o diz.
