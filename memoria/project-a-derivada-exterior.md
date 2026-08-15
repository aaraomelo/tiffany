---
name: project-a-derivada-exterior
description: O d é UMA operação — grad, rot e div são três nomes que ela tinha; e Green/Stokes/Gauss são UM teorema, provado por o programa ser um só.
metadata:
  type: project
---

O eval apontou o andar pelo nome: **falta o `d`**. Feito, e a afirmação central não é
sobre números — é sobre **código**.

## As três reduções

| Cálculo III | com o d |
|---|---|
| grad, rot, div (três fórmulas) | **UMA** operação; o que muda é o grau da forma |
| rot∘grad = 0 e div∘rot = 0 | **UMA** identidade: `d² = 0`, a mesma função duas vezes |
| Green, Stokes, Gauss (dois pares de funções) | **UM** teorema: `∫_∂R ω = ∫_R dω`, um par |

**A comparação não é vazia**: o `campo.h` tem grad/rot/div construídos à mão, cada um com
a sua fórmula, e o `d` foi escrito de novo **sem os olhar**. Derivá-lo *das* fórmulas
tornaria a medição tautológica. 125 formas de cada grau, 0 divergências.

E o Stokes unificado: `frm_stokes(ω,…)` com grau 1 dá Green/Stokes, com grau 2 dá Gauss.
**Nenhuma linha distingue os teoremas** — só o grau. 625 campos cada.

## Tudo na mesma linguagem (era o pedido)

- **cruzado** = α∧β (grau 1+1) · **directo** = α∧⋆β (grau 1+2) — o **mesmo** ∧
- e Lagrange fecha lá dentro: `directo² + cruzado² = N(α)N(β)`
- sete objectos de cinco andares — directo, cruzado, ∧, ⋆, ι_v, d, ∂ — com **uma
  gramática: a GRADUAÇÃO**. Cada operação sobe, desce ou preserva o grau, e o sinal é
  (−1)^grau.
- **∂ e d são ADJUNTOS**: ⟨ω,∂R⟩ = ⟨dω,R⟩, e há simetria — `d² = 0` nas formas e
  `∂² = 0` nas regiões (a borda de uma borda é vazia).

## O sinal testado ONDE MORDE

Leibniz graduado com |α| = 1: **0 falhas com o sinal, 100 de 125 sem ele**. Em grau 0 o
sinal vale +1 e o ramo nunca correria — [[feedback-o-ramo-que-nunca-corre]] evitado à
partida em vez de descoberto por mutação.

## O limite honesto

Em ℚ[x,y,z] toda fechada é exacta (Poincaré), construída e conferida. Mas o **buraco da
cohomologia** precisa de um domínio com buraco, e a testemunha clássica
`(−y dx + x dy)/(x²+y²)` **não é polinomial** — não vive no meu anel. **A razão de não
ver o buraco é a REPRESENTAÇÃO, não a matemática.**

Em `lib/dforma.h`, §C45 e «formas 1..21». Ver [[project-calculo-2-3]],
[[project-o-fecho-do-dual-lagrange]].
