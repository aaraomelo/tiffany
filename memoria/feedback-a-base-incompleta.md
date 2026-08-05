---
name: feedback-a-base-incompleta
description: "Quando um ponto cai fora, procurei a régua que o incluía — e o que faltava não era a régua, era METADE DO CORPO"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-05T14:44:45.821Z
---

**Um ponto fora do campo não pede uma régua nova. Pode estar a dizer que falta metade do
corpo.**

O `protocolo.c` tinha um membro (`Fator primo`) fora de 2σ, e eu tentei quatro réguas
seguidas para o medir. Cada uma foi uma tentativa de encontrar o critério que o incluía —
**que é procurar o número que faz a asserção passar**, o mesmo vício do limiar escrito à
mão que eu tinha acabado de corrigir nesse mesmo ficheiro.

| tentativa | Fator primo |
|---|---|
| `0,05` escrito à mão | 6 de 7 — **régua importada de outro corpo** |
| razão + defeito (2 colunas do ficheiro) | z = 2,396 FORA |
| `x† = +1/x` (produto **+1**) | z = 2,521 FORA, **e pior** |
| `x† = −1/x` (produto **−1**) | z = 2,135 FORA, a convergir |
| **a base COMPLETA, 2n entradas** | **z = 0,924 — o mais DENTRO** |

**Why:** é o `thm:torrecruz`, que eu tinha lido no mesmo dia: `A_{n+1} = Aₙ ⊕ Aₙ†`, com
`dim A_{n+1} = 2·dim Aₙ`. Para cada entrada entra **o seu dual**, e a estaca **leva
sinal** — `x† = −1/x`, com `x·x† = −1` exacto, que é o `σσ' = −1` da família metálica.
Ele passou de o mais fora ao mais dentro **não por o limiar ter alargado, mas por o corpo
ter passado a ter os dois lados**.

**How to apply:** quando um ponto cai fora e as réguas não o salvam, a pergunta deixa de
ser *qual é o limiar* e passa a ser **o corpo está inteiro?** Sinais de que não está:

1. **Nenhuma régua o inclui**, e cada uma que se tenta ou o piora ou o aproxima sem
   chegar. Isso não é ruído: é uma direcção a apontar.
2. **A base tem `n` entradas e o objecto é dual.** Se `dim` devia duplicar e não duplicou,
   está a medir-se uma projecção como se fosse o todo — *«o corpo é projeção de cima»*.
3. **O produto com o dual não dá `−1`.** Se der `+1`, a estaca está sem sinal e a reversão
   não fecha.

E **medir que a asserção pode falhar**, senão é a base completa a virar passe livre:
intruso a 1,5 e 2,0 **passa**; a 3,0 e 5,0 **apanha**. O limiar sai do campo em vez de ser
escrito, e o teste da mutação fica *dentro* do medidor.

**E o que o controlo deu de graça, e fica dito:** com um intruso de razão **−0,5** ela
**não** apanha. A estaca leva positivos em negativos, e quem já vem do outro lado cai no
meio da metade duplicada. A base completa cobre a reversão dos positivos e não distingue
quem já era negativo.

E antes disto tudo: chamar **«estocástico»** ao doador era eu a dizer *não sei medir isto*
— *«tudo é estocástico se for medido na régua errada»*. Um LLM é determinista dado o mesmo
input; o que varia é a amostragem, e a amostragem é **escolha minha**.

Ligado a [[feedback-a-base-ja-existe]], [[feedback-assercoes-vazias]] e
[[feedback-a-referencia-escrita-a-mao]].
