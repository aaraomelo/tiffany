---
name: feedback-o-teto-nao-verificado
description: `an_zn(&R, 40)` escreveu 1600 inteiros numa tábua de 24×24 e travou a máquina — um teto que não é verificado não é teto, é uma suposição sobre quem chama.
metadata:
  type: feedback
---

`Anel` tem `soma[ES_MAX][ES_MAX]` com ES_MAX = 24. Escrevi varreduras com `m <= 40` e
`an_zn(&R, m)` escreveu 1600 inteiros nesse array — **por cima da pilha**. O programa
deixou de terminar, e não deu erro nenhum: ficou em ciclo, e eu perdi tempo a procurar o
laço infinito na lógica antes de olhar para o tamanho.

**Why:** o teto estava DECLARADO (`#define ES_MAX 24`) e não VERIFICADO. Um `#define` que
ninguém testa é documentação, não é limite — e a distância entre as duas coisas é
silenciosa até deixar de ser. Pior: o sintoma (ciclo infinito) não aponta para a causa
(escrita fora do array), portanto a busca começa no sítio errado.

**How to apply:** todo construtor que preenche um array de tamanho fixo **recusa** acima
do teto e devolve 0 — nunca escreve e reza. E quem varre um tipo desses varre até
`ES_MAX`, não até um número que eu escolhi.

E o gatilho de diagnóstico: **quando um programa que era rápido deixa de terminar depois
de eu acrescentar uma varredura, a primeira suspeita é o TAMANHO, não a lógica.** É o
irmão do [[feedback-o-numero-que-nao-cabe]] — ali era o número que não cabia no tipo,
aqui é o objeto que não cabe no array; e nos dois casos a pergunta barata é a mesma:
*cabe?*
