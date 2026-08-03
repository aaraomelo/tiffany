---
name: feedback-procurar-na-bateria-antes
description: Escrevi uma peça nova sobre uma conta que a minha própria bateria já media — e a versão dela era mais completa
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T06:22:07.446Z
---

03/08/2026. Escrevi uma secção a justificar a involução pela conservação, com o argumento de que
uma onda na fronteira tem *"três saídas e só três"*: atravessar, voltar, desaparecer. Faltava
**dissipar**, e sem ela o argumento é falso (`|Γ|²+T<1` com perdas).

Descobri a falta por me atacar a mim próprio. Mas depois, a rever o catálogo, encontrei isto:

> `tools/colheita.c` — *"a onda do ambiente vira calor, e a dualidade é em **QUATRO** […] fechar em
> 100% não é retórica: é **R+T+A=1**, verificado em todos os materiais e frequências"*

**O balanço completo, com a absorção, já estava medido** — antes de eu escrever a peça. Escrevi uma
versão pior de uma coisa que a minha própria bateria já sabia, e demorei uma hora a descobrir a
falta que o medidor não tinha.

## A regra

**Antes de escrever uma peça sobre um assunto, procurar na bateria se já há medidor dele.** São 277
e eu não sei o que todos medem. A nota antiga diz *"o que não é citado não é testado"*
([[feedback-destruir-antes-do-inventario]]); esta é a gémea: **o que já é medido e eu não sei,
escrevo pior**.

O `grep` é barato:

```
grep -l "<o conceito>" tools/*.c
grep -rn "<a fórmula>" catalogo.tex
```

## E o efeito colateral bom

Ancorar a peça no medidor que já existe é melhor que a peça sozinha: ela deixa de **afirmar por si**
e passa a apoiar-se em algo que **corre**. Foi o que se fez.

## O que a mesma revisão mostrou de bom

O catálogo, varrido à procura de *"provado/confirmado"* sem medição, saiu **limpo**: as dez
ocorrências têm prova por argumento (uma por contradição), ou dizem-se explicitamente não
demonstradas — *"o enunciado é um resultado conhecido, usado aqui e não demonstrado. Vai dito onde
aparece, para que o leitor saiba de onde vem o peso."* É o padrão certo, e já lá estava.

## Nota sobre os agentes, nesta sessão

Doze lançados, **um entregou**. O que funcionou nesse: uma pergunta **única e específica**, enviada
depois de ele já estar idle. Os briefings longos não voltaram nenhum. Se voltar a precisar, começar
pequeno — e não contar com eles para o caminho crítico. Ver [[feedback-revisores-externos]], que
continua verdadeira quando eles respondem: o único que respondeu apanhou **duas frases falsas
publicadas**.
