---
name: feedback-o-if-que-engoliu-o-return
description: "Um `if(0)` órfão colou-se ao `return 0` e engoliu-o; a função caiu no fim SEM RETORNAR e devolveu lixo. O `-w` calou o aviso que o apanhava."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: cee0686a-c852-4d69-8fca-ca206e1fba24
  modified: 2026-08-21T22:09:14.110Z
---

Em `passo_do_slot` ficou isto:

```c
switch(s >> ZBITS){ case 1: case 2: case 3: return rel_ncols; default: break; }
if(0)
/* comentário */
return 0;      /* ← corpo do if(0): NUNCA executa */
```

O `if(0)` não tinha chavetas, logo adoptou o `return 0` como corpo. Para
todo o slot de prefixo 0 — as constantes, o rascunho, o acumulador, os
dois bitmaps — a função **caía no fim sem retornar** e devolvia o que
estivesse no registo. Tudo passou a deslizar com a linha: a linha 0
acertava e as outras liam o vizinho.

**Porquê:** o defeito sobreviveu a várias sessões de bissecção porque o
sintoma parecia lógico (`WHERE a = 5` devolvia 7 de 8) e eu procurei-o
na lógica. O compilador tinha a resposta pronta — «control reaches end
of non-void function» — e eu compilava com `-w`.

**Como aplicar:** antes de bissectar um sintoma que parece lógico,
compilar UMA vez com `-Wall` e ler. Custa um comando. E um `if` sem
chavetas junto de um `return` de saída é para desconfiar sempre: o
comentário entre os dois esconde a adopção à vista.

O corolário é o par de [[feedback-o-ramo-que-nunca-corre]]: uma mutação
que sobrevive tem três causas, e esta é a quarta — o ramo que engole o
que vinha a seguir. Da mesma família de
[[feedback-a-mensagem-que-nao-pode-falhar]]: o programa estava
sintaticamente bem e semanticamente ausente.

E a sonda mediu-se a si própria antes de medir o motor: escrevi Words em
slots consecutivos e a Word É o par `[e+o, e]`, pelo que os ímpares foram
comidos pelos pares. [[feedback-a-chave-faz-parte-da-medida]].
