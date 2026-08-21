---
name: feedback-o-tipo-nao-e-a-aritmetica
description: "«0 doubles» é verdade sobre o TIPO e falso sobre a conta: 81 literais de vírgula fazem aritmética flutuante sem escrever a palavra."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: cee0686a-c852-4d69-8fca-ca206e1fba24
  modified: 2026-08-20T22:45:28.795Z
---

`tools/doubles_todos.py` conta o tipo `double|float` e está em **0**. É verdade.
Mas em 20/08 encontrei **81 literais de vírgula flutuante em código, 28
ficheiros** — `2.0`, `100.0`, `0.5`, `1e3`. Cada um faz a conta em vírgula
flutuante sem a palavra lá estar.

E não era teórico. `tests/sshb.c` tinha `long rtt = 0.5`: a Fase A trocou o tipo
e deixou a constante em vírgula. **O 0,5 ms virou 0**, a tabela de latências
inteira saía de uma conta em double impressa com `%ld`, e nenhuma asserção o via
porque nenhuma delas lia o que estava escrito.

**Why:** é [[feedback-a-definicao-do-extractor]] outra vez, e a terceira. O número
que publiquei era a definição da consulta — `grep -w double` —, não um facto sobre
o repo. E o par que faltava é o de sempre: o tipo é METADE (a declaração); a
aritmética é a outra ([[feedback-medir-so-metade-do-par]]).

**How to apply:** um alvo tem sempre duas caras — a DECLARAÇÃO e o USO. Ao fechar
uma migração de tipo, medir também as constantes, os literais e os formatos. Aqui
os formatos deram o outro achado: **216 avisos `-Wformat`** no repo, todos da
mesma família (o tipo mudou, o `%d`/`%ld`/`%lld` ficou). Em x86-64 o valor sai
certo por acidente de ABI — logo nenhuma asserção cai — mas trunca acima de 2³¹,
que são exactamente os valores que a promoção existe para guardar.

A régua barata que faltava, e que agora corre em segundos:

    cc -fsyntax-only -std=c99 -Wformat …    # tem de dar ZERO

Um alvo que a régua não vê não é um alvo a menos: é um alvo que ninguém sabe que
existe — e era isso que o cabeçalho do próprio `doubles_todos.py` já dizia.
