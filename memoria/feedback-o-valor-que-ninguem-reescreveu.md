---
name: feedback-o-valor-que-ninguem-reescreveu
description: "`COUNT(*)` numa tabela vazia devolvia a contagem da consulta ANTERIOR: a saída antecipada saltava a linha que escreve o contador, e o leitor leu o valor velho."
metadata:
  node_type: memory
  type: feedback
---

O `count(*)` desta casa **não reconta**: corre a mesma varredura do `WHERE` e
lê depois o `ultima_conta`, que é o ∑ sobre o campo — «não é uma segunda
contagem, é a leitura da que já ficou escrita». Bom desenho.

Só que a escrita `ultima_conta = achou` fica **no fim** da varredura, e há uma
saída antecipada para a tabela sem linhas que nunca lá chega. Resultado:

```
SELECT COUNT(*) FROM tabela_com_3_linhas   →  3
SELECT COUNT(*) FROM tabela_vazia          →  3     ← o valor de antes
```

Medido: devolveu **2**, que era a contagem de uma consulta feita antes noutra
tabela.

**Why:** eu verifico que o caminho normal escreve o valor. Não verifico que
**todos** os caminhos o escrevem — e um `return` antecipado é, por definição,
o que salta o resto. Onde há um leitor de estado global, cada saída é uma
oportunidade de o deixar velho.

**How to apply:**
1. Quando um valor é escrito num sítio e lido noutro, listar **as saídas** da
   função que escreve. Cada `return` antes da escrita é um caso.
2. **O gume tem de ser deliberado.** Contar a tabela vazia *à cabeça* dá zero
   por acaso — o contador ainda está a zero — e o defeito passa verde. Põe-se
   primeiro uma contagem NÃO-NULA e só depois a vazia. Aqui ele só apareceu
   porque a ordem dos blocos deixou um `2` lá dentro; a asserção acusou e eu
   quase a dei por errada.
3. O mesmo padrão vale para qualquer «último resultado» guardado: `ultima_*`,
   `sql_ultimo_prog`, caches de resposta.

Da família de [[feedback-o-medidor-que-nunca-mediu]] — o valor sai e não foi
medido — e de [[feedback-a-recusa-que-deixa-rasto]]: as duas vezes o caminho
curto deixou o estado a meio.

**E o mesmo padrão do lado do CONSUMIDOR** (22/08): numa sonda li
`o.cell[0][0]` sem verificar `o.ok`. O `INSERT` de 243 tinha sido recusado
(não cabe no `Word_8`) e o `traco` recusou com «empty table» — as duas portas
a funcionar —, mas a célula de um resultado recusado está VAZIA e `atol("")`
dá **0**. Quase reportei «o motor devolveu 0» sobre uma recusa correcta.
Ler o valor sem verificar que houve valor é a mesma doença: um estado que
ninguém escreveu, lido como se tivesse sido.
