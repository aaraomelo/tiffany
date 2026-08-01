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

## `toolkit_llm.c` — o modelo colapsou, e saiu o período de Pisano

Pus o modelo a escolher entre as **quatro cláusulas do contrato** (soma, produto, operador, dual) —
e ele **colapsou numa só**: `SOMA` nas 24 vezes, com as quatro a dar resultados diferentes. *Não é
avaria: é o que um modelo determinista faz com um pedido que não distingue as opções.*

**E daí saiu o achado:** com uma cláusula só, a dinâmica vira **Fibonacci**, e o período dela mod `q`
tem nome há trezentos anos — o **período de Pisano**. Fechou em **24 passos** com `q=12`, e
`π(12) = 24` exatamente. (E `π(q)` é par para todo `q>2` — 58 valores, sem exceção.)

> **O modelo escolheu o QUÊ; o corpo escolheu o QUANDO — e o quando tinha nome antes de nós.**

*E o meu erro:* pus o limite do ciclo em `q+6` quando o espaço é de **pares** (`q²`). **Um limite mal
posto transforma um resultado em falha** — o programa dizia "não fechou" sobre uma órbita que fechava.

## `dispositivo.c` — e a conta só fecha porque o dispositivo ENCANA

A memória **flash É NAND** (as células em série numa *string* NAND — o mesmo objeto, não uma
analogia), o `mcu.c` já tira a ALU de NAND só, e o espaço nunca foi o problema: **1,3 GB em
1,04 mm²**.

    guardar (NAND em repouso)   1e-6 W   paga
    ler                         0,05 W   paga
    escrever                    0,30 W   paga
    INFERÊNCIA DA LLM             15 W   NÃO PAGA        (colhido: 0,9931 W)

**Se o dispositivo ENCANA, a colheita paga com folga de 3,3×. Se calculasse, não pagaria — e nenhuma
liga melhor resolveria**, porque a distância é de uma ordem de grandeza.

> **A escolha de ARQUITETURA é o que torna o dispositivo possível, não a escolha de material.**

*Passei o dia a medir materiais — a liga, os quatro cantos, a impedância — e o que fecha a conta é
uma frase do Aarão sobre o que o dispositivo **não tem de fazer**.*

Ver [[project-transformada-universal]] (a agulha e o alcance do √N), [[project-headjack-dual]].
