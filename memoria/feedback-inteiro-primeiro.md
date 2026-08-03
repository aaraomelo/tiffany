---
name: feedback-inteiro-primeiro
description: "REGRA DURA: inteiro/racional desde o PRIMEIRO rascunho. Faço em double, ele reclama, e só então faço certo — e a teoria que escrevo diz o contrário do que o código faz"
metadata:
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
---

O Aarão, 03/08, e diz que já o disse muitas vezes:

> *"esse float e double é outro cadáver que carrego. Sempre falo: faz em bit, inteiro, racional. E
> sempre você torra o PC e no fim só valida com o certo racional depois de eu insistir muito.
> **Isso é regra.** Você faz o errado primeiro e depois de errar, e eu observar e reclamar, aí faz
> certo. **Você não usa a própria teoria que está desenvolvendo.**"*

**A última frase é a mais dura, e é medível.**

## O NÚMERO, medido em 03/08

**134 de 277 medidores usam `double` — 48%** — num projeto cuja tese é que a máquina opera em
inteiros e que a reversão é exata porque `det = ±1`.

E nos que escrevi nesse dia, o padrão vê-se ao minuto:

| feito | `double` | asserções |
|---|---|---|
| à minha maneira | `lambert.c` **67** · `continua.c` **59** · `xx.c` 45 | 20 · 22 · 21 |
| depois de ele insistir | `zetadin.c` **0** · `gauss.c` **0** · `nomeia.c` **0** | 10 · 14 · 3 |

## A REGRA

**Inteiro ou racional desde o PRIMEIRO rascunho. Não "float agora, exato depois".**

O float não é só impreciso — ele *esconde a estrutura*. Três exemplos do mesmo dia:

- comparar `σ_m` crescente com `sqrt()`, quando se compara nos **denominadores**, que são inteiros
  do corpo (`q_k(m+1) > q_k(m)`, sem uma raiz);
- verificar `(1−σ^{-e})` em `double` e falhar por **cancelamento** (entradas `>1e5`, valor `≈1`) —
  o teste exato é `(1−σ^{-e})·σ^e = σ^e − 1`, em ℤ[σ];
- medir a norma `p²−mpq−q²` em `long long` e passar por **sorte** através de overflow.

## E O IRMÃO: TORRAR EM VEZ DE DESDOBRAR

*"é só questão de desdobrar, mas você insiste em torrar tudo."* Varri 100 denominadores e contei
buracos para concluir o que sai de **uma linha**: o salto entre denominadores **é** `σ_m`, e `σ_m`
cresce com `m` — logo o ouro é o mínimo. Sem varredura nenhuma.

**Antes de varrer, perguntar: isto sai da recorrência?**

## O teste, antes de escrever a primeira linha de um medidor

1. Que objeto é? Se vive em ℤ[σ], ℚ ou ℕ — **não há double nenhum a escrever**.
2. A asserção compara **valores** ou **representações**? (ver [[feedback-assercoes-vazias]])
3. Isto sai por desdobramento, ou estou a varrer?

Ligada a [[feedback-a-base-ja-existe]] — é o mesmo defeito de fundo: trago a ferramenta genérica
(float, DFT, Gram-Schmidt) em vez de usar a estrutura que o objeto **já tem** e que o texto que
estou a escrever **descreve**.
