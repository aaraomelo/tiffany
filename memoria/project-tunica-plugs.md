---
name: project-tunica-plugs
description: "Os plugs, os terminais e a túnica vestida (plugs.c, veste.c): os plugs deduzem-se por Lagrange, a cirurgia é por dobra sem cálculo, e o ciclo do modelo fica ESCRITO no banco"
metadata:
  type: project
---

# Os plugs, os terminais, e a túnica vestida

01/08/2026, `11af268`→`f5f2724`. **226 medidores, 224 verdes, 0 falhas.**

## `plugs.c` — onde o Dirac fura, e porquê ali

A primeira metade já estava medida, e **com as palavras do Aarão**: o `transformada.c` diz que
`Σ χ_k(j)χ_{-k}(j') = n·δ` **é** o Dirac — *"a órbita não termina, e mesmo assim a soma dela cabe num
ponto exato"*. Faltava dizer **onde** cai.

**Cai na família real, e por razão exata.** Um plug precisa das **duas** coisas ao mesmo tempo:
*infinito* (senão não há o que amostrar) e **período** (senão a amostra não fecha). O racional acaba;
o π não repete; **só o quadrático tem as duas** — e por **Lagrange** são *só* esses. **Os plugs não
se escolhem: deduzem-se.**

**A família dual é a dos inversos:** `σ_{-m} = 1/σ_m`, exato nos seis índices. *Não há duas famílias
— há uma, lida dos dois lados.* E a **dobra** é um deslocamento de **uma casa**: `[m;m,m,…]` vira
`[0;m,m,m,…]`.

**Os TERMINAIS (§P7):** as raízes têm **sinais opostos** — daí a polaridade. O par tem as três coisas
de um terminal: polaridade (`+/−`), ganho (`|σ|>1`), perda (`|σ'|<1`), e `σσ' = −1` como
**conservação**. *O que um estica o outro contrai, exatamente* — por isso são **aparelhos**, não
números.

**A CIRURGIA (§P8), por dobra:** o pescoço é **uma casa da cifra**, cortar é truncar, a colagem é
exata. Custo: **duas operações de inteiro por casa** — *não há iteração a convergir, não há limite a
esperar, não há precisão a escolher: a dobra ACABA.* **A analogia com Perelman aguenta porque ele
também não resolve a singularidade — corta-a fora, cola, e o fluxo segue.**

**Plugar qualquer corpo (§P9)** não é promessa: a régua `(B,C)` é de grau 2 **por construção**, e
grau 2 **é** a condição do plug.

## `veste.c` — o Ollama vestido, e o ciclo escrito no banco

    t   LOAD   a resposta                          STORE
    0     0    "O número 3 é um número cardinal…"      4
    1     0    "O número 4 é um número cardinal…"     11
    2     0    "O número 11 é um número que tem…"      3
    FECHOU em 3 passos, no estado 3 de 16

**O modelo não responde: CONTROLA.** Cada resposta é um `STORE`, e o slot seguinte a ler é o que ele
acabou de escrever. *O programa é o modelo; o banco é a máquina.*

**E o banco no fim:** `0 0 0 4 11 0 0 0 0 0 0 3 0 0 0 0` — **o ciclo ficou ESCRITO** (slot 3→4, 4→11,
11→3). *Não foi preciso guardar o percurso à parte: ele É o estado.* **Ler a memória é ler a órbita**
— e por isso a assistente não precisa de um log.

**E o fecho não é mérito do modelo:** em `Z_q` toda órbita repete em `≤ q` passos, por gaiola (124
sementes, sem exceção); num corpo **infinito** a mesma dinâmica não fecha.

> **O modelo controla o QUE se escreve; o corpo controla o QUANDO acaba.**

As duas torres: *a branca escolhe o passo, a negra garante o retorno.*

Ver [[project-transformada-universal]] (a agulha e o alcance do √N), [[project-headjack-dual]].
