---
name: feedback-o-gume-por-lei
description: "Uma condição `A && B && C` precisa de TRÊS mutações, uma por termo — mutar uma só mede que a conjunção tem pelo menos um termo vivo."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-17T05:34:55.942Z
---

Corrigi uma tautologia minha no `§F4` do `dif.c`, mutei uma linha, vi a asserção cair, e
commitei a dizer que estava medido. Fui continuar no mesmo ficheiro e encontrei **outra
tautologia minha, no mesmo bloco, já commitada**:

```c
long tD[7]; for(k) tD[k] = k * ts[k];
for(k) if(tD[k] != sgrau * ts[k]) bate = 0;
```

Como `ts` só vale 1 em `k = sgrau` e 0 no resto, os dois lados são a **mesma expressão**
para todo `k`. O `euler_ok` valia 7 fizesse-se o que se fizesse.

**Why:** o bloco tinha TRÊS leis na mesma condição —
`maus_leib == 0 && maus_lin == 0 && maus_euler == 0 && ... && euler_ok == 7`. Eu mutei a
linha do `maus_euler`, que é real, e a asserção caiu. Mas uma mutação que derruba diz
apenas isto: **que UM termo morde**. Os outros continuam por testar, e um deles era meu e
estava vazio.

É a forma mais perigosa da asserção vazia, porque o gume dá verde: a conjunção *parece*
medida.

**How to apply:**
1. **Uma condição com `N` termos precisa de `N` mutações, uma por termo.** Não uma por
   bloco. Se não vale a pena mutar cada termo, é porque o termo não devia estar lá.
2. **O sinal:** se uma mutação num termo não muda o número impresso desse termo, o termo é
   decorativo. Imprimir cada quantidade à parte torna isto visível — `bate em 2 de 7` a
   mudar para `1 de 7` é a testemunha de que a mutação chegou ao sítio certo.
3. E a fonte destas duas: **escrever a lei e o teste na mesma linha**. `tD[k] = k*ts[k]`
   seguido de `if(tD[k] != sgrau*ts[k])` é a definição a olhar-se ao espelho. A conta tem
   de vir por um caminho e a verificação por outro — aqui, construir `tD` pelas duas
   OPERAÇÕES (derivar, depois deslocar) e comparar com o valor próprio.

Duas vezes no mesmo dia, e as duas dentro de uma correcção de tautologias.

Ver [[feedback-assercoes-vazias]], [[feedback-dois-caminhos]], [[feedback-gume-automatico]].
