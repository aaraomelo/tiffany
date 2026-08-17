---
name: project-descida-racionais-pontos-fixos
description: A DESCIDA — ℚ são as classes de equivalência dos pontos fixos, o teorema dos resíduos dá-os directo, e a aritmética aditiva é a Möbius com um S no meio.
metadata:
  type: project
---

# A descida: reais → racionais, e ela estava toda escrita

17/08/2026, commits `2f0b354` → `fa7fb5e`. O `geometrico.tex` já subia — a órbita corre em
ℙ¹(ℚ) e o ponto fixo **não está lá** (`thm:corte-fixo`). Faltava a outra metade do par, e ela
não pedia construção nova: pedia ler o que o `thm:fixo-dual` já dizia.

## A construção (thm:descida)

Cada σ_m vive no **seu próprio corpo** ℚ(√(m²+4)) — φ em ℚ(√5), 1+√2 em ℚ(√8), um corpo por
cada m. Não há sítio onde caibam todos, **excepto o que a simetrização deixa**:

    σ + σ† = m      ← o TRAÇO: projecta, e é SOBREJECTIVO sobre ℤ
    σ · σ† = −1     ← a NORMA: constante, e não separa NADA

Logo **ℤ ≅ {pontos fixos}/∼ pelo traço**, a fibra sobre m é o par dual {σ_m, σ_m†} — *a classe
de equivalência* — e ℚ é a razão: `a/b = tr(σ_a)/tr(σ_b)`, com `ad = bc` **recuperado e não
posto**. E o racional é o que a DOBRA fixa: `x ∈ ℚ ⟺ x = x†`, o auto-espaço +1 (Dir alcança,
Cruz opera — Pontryagin literal).

**O gume está dentro da construção**, e é o que a torna medida e não definição: se a coordenada
escolhida fosse a norma, a fibra era a família inteira. Medido lado a lado — o traço separa 60
de 60, a norma separa 1; em 20 736 quádruplos o traço acerta todos e a norma acerta 432. É
[[feedback-normalizar-nao-e-medir]] a aparecer como escolha de coordenada.

## O teorema dos resíduos JÁ os dá (thm:residuo-fixo)

O denominador da zeta **factoriza nos pontos fixos**, e sai do traço e do determinante sem
formar raiz: `1 − mx − x² = (1−σx)(1−σ†x)`. Logo os polos SÃO os pontos fixos (nos recíprocos),
a decomposição dá uma parcela POR polo, e extrair o coeficiente de x^k é somar sobre eles — o
que sai é σ^k + σ†^k, **o traço**. O círculo fecha:

    extrair coeficiente = somar sobre os pontos fixos = o traço = ℚ

E **um resíduo sozinho não é racional**: σ^k tem parte √D não nula em todo k ≥ 1, e são os DOIS
que dão o inteiro. [[feedback-dual-exige-dois]] como conta.

## A aritmética aditiva (thm:aditiva)

`A_a · A_0 = T_a` — o passo da FC composto com a INVERSÃO dá a TRANSLAÇÃO — logo

    A_a · A_0 · A_b = A_{a+b}

**a soma realiza-se dentro do grupo multiplicativo, com S = A_0 no meio**: o eixo 0 ↔ ∞ a fazer
de logaritmo. Na palavra é a mesma frase: **um quociente ZERO soma os vizinhos**,
`[… a, 0, b …] = [… a+b …]`.

E o «∞ + 1 = −1» do Aarão **fecha nos índices**, com x_k := A_m^k(∞):

    x_0 = ∞ (o NEUTRO) · x_-1 = 0 (o 0 = 1/∞) · x_-2 = −1/m = norma/traço

— *o elemento que fecha é a razão dos dois invariantes do par dual*, o que conserva a dividir o
que separa. Em m=1 vale −1: **a relação dele é o caso do ouro**. Na volta ao quadrado fica um
passo só, e o anel muda: det de −1 para +1 (a volta passa a PRÓPRIA) e disc de D para m²·D,
isto é ℤ[m√D].

## E a descida via Möbius É Euclides

A casa tinha as duas metades em colunas diferentes sem nunca dizer que eram a mesma:
`rt_cf_de` escrita como mdc, `rt_dobra` escrita como Möbius — e `p − a·q` **é** `p mod q`
quando `a = ⌊p/q⌋`. O algoritmo da fracção contínua é iterar g⁻¹, e por isso a palavra sai sem
uma divisão em vírgula. Ver [[project-a-reta-construida]] e [[feedback-a-base-ja-existe]].

**Medidores:** `tests/racionais_fixos.c` (8:0), `tests/descida_mobius.c` (10:0), e o
`tests/continua.c` §C2 com a identidade `−log(1−mx−x²) = Σ t_k x^k/k` provada **coeficiente a
coeficiente em ℚ** em vez de avaliada em 15 pontos com duas réguas.
