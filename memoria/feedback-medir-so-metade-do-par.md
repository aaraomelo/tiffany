---
name: feedback-medir-so-metade-do-par
description: "O bench da membrana media só a ENTRADA; aberto o outro sentido, caíram dois comandos inventados na SAÍDA que passavam há sessões."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T04:23:42.839Z
---

O `bench_membrana.sh` existia para apanhar dialecto inventado — nasceu do `\ast` e do
`\pmod`. E media **só metade**: varria a tabela `LX` (o que a assistente **LÊ**) contra
o dialecto do tradutor, e nunca o que a assistente **ESCREVE**.

Assim que abri o segundo sentido — `grep` dos `\\comando` nos `printf` — caíram na hora
`\land` e `\overline`, emitidos há sessões, desconhecidos do tradutor. O bench estava
verde o tempo todo. 16:0 virou 29:2 e depois 29:0.

**Why:** a membrana é o Dual **com sinal** (+1 desdobra a entrada, −1 veste a saída) — o
código já dizia isso em comentário. Um medidor que só corre num sinal não mede o objeto:
mede uma projeção dele, e chama-lhe o nome do par. É o [[feedback-dual-exige-dois]]
dentro de um **medidor**, e por isso é pior: passa por medição.

**How to apply:** quando um medidor compara duas fontes através de uma operação que tem
sentido (ler/escrever, codificar/descodificar, entrar/sair), perguntar qual dos dois
sentidos ele varre. Se a resposta for «um», o outro está sem medida nenhuma. O sintoma é
o mesmo de sempre: o total é bonito e estável há muito tempo.

E o teste barato: **o objeto medido é uma tabela ou é o comportamento?** Aqui a varredura
era da TABELA `LX` — um dado estático. O que a assistente escreve está espalhado pelos
`printf`, e por isso ninguém foi lá. O que não está numa tabela não é varrido por
acidente: tem de ser procurado de propósito. Ver [[feedback-verdadeiro-e-parcial]] e
[[feedback-dois-caminhos]].
