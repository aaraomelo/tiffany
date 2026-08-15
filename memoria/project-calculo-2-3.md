---
name: project-calculo-2-3
description: Cálculo II e III exactos — a série é o objecto (o valor é que precisa do limite), local→global é o eixo, e Green/Stokes/Gauss são uma frase só medida pelos dois lados.
metadata:
  type: project
---

## Cálculo II — local → global

O eixo que o Aarão acrescentou à espinha, e aparece em toda a parte: **o termo geral é
local e a convergência é global** — e a harmónica prova que o salto **não é automático**.

- **A série é o OBJECTO**; o *valor* é que precisa do limite. `(1−x)Σxⁿ = 1` e
  `sin² + cos² = 1` provam-se nos **coeficientes**, sem um ângulo e sem um decimal. É o
  que o `dirichlet.h` já fazia: manipular coeficientes e nunca avaliar o s.
- **A Hessiana tem QUATRO estados** (o quarto pedido por nome no eval): definida +,
  definida −, indefinida, **semidefinida**. Em x² o ponto é mínimo e ela não o sabe.
- `Df(a)` deixa de ser número e passa a **operador**: compor vira multiplicar, e o det
  multiplica junto (Λⁿ do andar exterior).

## Cálculo III — número → vetor → operador → campo → BORDA

**Green, Stokes e Gauss são UMA frase**: `∫_{∂R} ω = ∫_R Dω`, com D a mudar de nome. Não
se medem como três teoremas — mede-se a estrutura, e **os dois lados não partilham
código**: a circulação percorre 4 segmentos, o rotacional integra uma área; o fluxo
percorre 6 faces, a divergência integra um volume. **625 campos cada, 0 falhas.**

- **div/rot é o directo/cruzado**, literalmente: div é ⟨∇,F⟩ e rot é ∇×F. E a cadeia
  ∇ → rot → div é um **complexo** (rot∘grad = 0, div∘rot = 0), varrido.
- **Conservativo é uma FIBRA**: constrói-se φ e depois **confere-se** que ∇φ devolve F.
  Sem essa volta, «achei um potencial» não vale nada. Gume: rot ≠ 0 ⟹ fibra vazia.
- **Os dois caminhos**: conservativo dá 36 e 36; não-conservativo dá **6 e −6** —
  simétricos, e a diferença é a circulação no laço.
- **O que não se mede, dito à frente**: polares e esféricas escrevem-se e não se calculam
  (seno e cosseno não vivem em ℚ). O que interessa é que o factor É determinante de
  jacobiana.

## O que falta, e está dito

As **formas diferenciais**, que fariam dos três teoremas uma lei só. A álgebra já cá está
do andar exterior — Λᵏ, a cunha, o ⋆, a contração ι_v. **Falta o `d`**, a derivada
exterior. Fingir que já lá cheguei seria [[feedback-insinuacao-arquitetonica]].

Em `lib/calculo2.h`, `lib/campo.h`, §C43 e §C44, «calculo2 1..21» e «calculo3 1..21».
Ver [[project-calculo-exacto]], [[feedback-o-objecto-que-nao-cabe]].
