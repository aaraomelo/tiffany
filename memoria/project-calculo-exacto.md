---
name: project-calculo-exacto
description: "Cálculo I sem um double — porque (f(a+h)−f(a))/h É um polinómio em h, a derivada é uma AVALIAÇÃO e não um limite; e a casa já derivava exacto pela parte ε do dual."
metadata: 
  node_type: memory
  type: project
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T20:11:49.061Z
---

O `eval.txt` trouxe o Cálculo I inteiro, e a aparente impossibilidade: é a matéria dos
limites, e aqui não entra double. **O problema desaparece com um facto sobre polinómios:**

    (f(a+h) − f(a)) / h   É ELE PRÓPRIO UM POLINÓMIO EM h

porque o dividendo não tem termo constante. Logo **f'(a) = q(0) é uma AVALIAÇÃO**, não um
limite. E a divisão por h é a **FIBRA** (`thm:divisao-fibra`): «dado f(a+h)−f(a) e o factor
h, achar o outro». Sem processo infinito.

## E a casa já derivava exacto — por outro caminho

`resolve_calculo` («deriva …») já dizia: «é a parte **ε do dual**, f(a+bε) = f(a) + f'(a)bε
com ε² = 0 — sem passo h e sem limite». Não refiz: medi a **concordância**.
**Três caminhos independentes** — regra formal, quociente de diferenças, parte ε do dual —
concordam em 52 pontos, com a parte REAL do dual a devolver f(a) como controlo.
([[feedback-a-base-ja-existe]], [[feedback-procurar-na-bateria-antes]])

## Os dois achados que o andar deu

1. **dir − esq = (f(b) − f(a))·h, EXACTO**, por telescopagem. Quanto o cerco de Riemann
   ainda está aberto diz-se **sem um ε** — é igualdade, não estimativa.
2. **O ponto do Valor Médio para x³−3x em [0,2] é 2/√3 — IRRACIONAL.** A malha racional
   não o pode achar, e isso **não é falha do programa**: é o teorema a dizer que o ponto
   vive no corte. A bisseção devolve o encaixe, que é como esta casa devolve um real.

## Os gumes acham-se sozinhos

- ε-δ: o δ é procurado e exibido; e o **mesmo** buscador com L errado não acha nenhum.
- TVI: sem troca de sinal, a bisseção **RECUSA** — não devolve intervalo errado.
- Fermat: x² em [0,2] tem o máximo na **borda**, f'(2) = 4 ≠ 0 — sem «interior» é falso.
- Riemann: com f monótona o cerco vale nas 64 malhas; sem monotonia falha numa.

## A tradução para o universal (a ordem: «tudo tem tradução»)

| Cálculo I | Corpo Universal |
|---|---|
| limite / cerco | o CORTE — `thm:real-caminho`, «a folha NÃO é nó» |
| ε-δ | a ESCADA de observadores — `thm:escada`, cada ε é um degrau |
| continuidade | a MEMBRANA — `def:membrana`, o corpo em trânsito |
| derivada | a FIBRA — `thm:divisao-fibra`; e Δ ⟺ c_k ↦ (ω^k−1)c_k, `thm:metronomo-fourier` |
| integral | a SOMA REVERSÍVEL de Gentil — `obs:triade-central` |
| **Teorema Fundamental** | a VOLTA: ∫ e d/dx são operações inversas |

⚠ **E NÃO é o `thm:central`.** Escrevi que «É» e o Aarão corrigiu: aquele é
∫f + ∫f⁻¹ = bf(b) − af(a), a integral da **função** inversa — teorema clássico distinto
que *deriva* do TFC. Um inverte **operações**, o outro inverte **funções**. A
correspondência entre os dois é **proposta por esta casa**, uma camada acima; a
matemática clássica do TFC sustenta-se sozinha. Ver [[feedback-insinuacao-arquitetonica]].

Realizado em `lib/calculo.h`, `banco/conversa.c` §C42 e «calculo 1..21».
Ver [[project-torre-hurwitz-gentil]], [[project-o-real-e-o-corte]].
