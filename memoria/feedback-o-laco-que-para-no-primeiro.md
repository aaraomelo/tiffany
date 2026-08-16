---
name: feedback-o-laco-que-para-no-primeiro
description: A busca por EXISTÊNCIA decide e não mede — para no primeiro caso, e é sempre o mesmo
metadata:
  node_type: memory
  type: feedback
---

16/08/2026. Escrevi um cálculo de posto por menores, com `break` no primeiro menor não nulo,
e reportei que um gume «não mordia, e a razão é legítima». **O Aarão apanhou-o na pergunta
certa:** *«como assim o gume não mordeu? estás à espera que o laço pare em algum lado ou
estás a varrer o infinito?»*

```c
for(int i = 0; i < nv && !menor_vivo; i++)
    for(int j = i+1; j < nv; j++)
        if(V[i][0]*V[j][1] - V[i][1]*V[j][0] != 0){ menor_vivo = 1; break; }
```

**De 91 pares, examinava UM — e sempre o mesmo.** O primeiro par é `(0,1)`, onde
`p₁q₂ − q₁p₂` e `p₁q₂ + q₁p₂` são ambos não nulos (os convergentes são todos positivos).
Por isso o gume do sinal não podia morder: **nunca chegava a ser testado**.

## O padrão, e ele é novo

A busca por **existência** — «algum menor é não nulo?» — está CERTA para *decidir* o posto.
E não mede coisa nenhuma, porque termina no primeiro sucesso. O código está correcto e a
medida é vazia; e a minha explicação — «a cláusula só decide não-nulidade» — era verdadeira
e desviava do defeito.

**Gatilho:** quando um gume não morde, a primeira pergunta não é *porque é que a cláusula é
robusta* — é **quantos casos é que ela chegou a ver**. Um `break`, um `&& !achou`, um
`return` cedo: a varredura tem o tamanho do primeiro sucesso, não o do laço.

## E a saída é quase sempre a mesma: a forma fechada

O menor tinha lei, e a lei mede em todos os pares:

```
p_i q_j − p_j q_i  =  (−1)^(i+1) · F_(j−i)          91 de 91, por rota independente
```

— não «é não nulo», mas **é este Fibonacci com este sinal**. Passou de 1 par varrido para
455, e os três gumes passaram a morder, incluindo o que não mordia. Ver
[[feedback-representacao-inteligente]], [[feedback-varrer-onde-nada-pode-falhar]] e
[[feedback-assercoes-vazias]].
