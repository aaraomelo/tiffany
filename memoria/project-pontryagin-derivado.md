---
name: project-pontryagin-derivado
description: Pontryagin derivado do zero (21/08) — |Ĝ|=|G| NÃO é o teorema; o teorema é a naturalidade de ev, e ψ falha exactamente onde existe u com u²≠1.
metadata:
  type: project
---

`tests/pontryagin.c` (21/08/2026), 9 asserções, gume 12/12. Tudo em ℤ/e — os
valores dos caracteres vivem no **expoente**, não em ℂ, e a «troca ⊕ → ⊗» é
literalmente isso. Zero vírgulas.

**A distinção que faltava na casa.** O `bidual.c` §B5 media `|Ĝ| = |G|` e tinha
`long bidual = distintos;` — uma atribuição. Mas **|Ĝ| = |G| não é o teorema de
Pontryagin**: é o acidente de tamanho. O teorema é que
`ev_x(χ) = χ(x)` é iso **natural** — comuta com todo morfismo porque não escolhe
nada. Um iso `X → X̂` também existe, e são MUITOS (1, 2, 4, 6, 8, 12, 48, 168
nos doze grupos medidos): escolher um é escolher geradores.

**A lei do §PG7, n a n e sem excepção:**

    ψ : X → X̂ falha a naturalidade  ⟺  existe u com u² ≢ 1 (mod n)

Nos n em que toda unidade tem u² = 1 — **2, 3, 4, 6, 8, 12** — ψ passa, e passa
por acidente do grupo. Varrer só esses daria «ψ é natural», que é falso:
[[feedback-varrer-onde-nada-pode-falhar]].

**A ponte com a aranha:** o caractere é uma **realização** no sentido do
`aranha.tex`, e a ortogonalidade é a sua fibra ser **uniforme** —
`|χ⁻¹(c)| = |X|/|im χ|`. Dito assim não é preciso somar uma raiz da unidade, e é
essa constância que inverte a transformada. Ver [[project-transformada-universal]],
[[project-dois-papers-algebra-topologia]].

**A fronteira:** a finitude entra em dois sítios — os morfismos enumeram-se, e
injectiva passa a bijectiva pela contagem. A testemunha de que a contagem é
essencial: `2x : ℤ/4 → ℤ/8` é injectiva e **não** sobrejectiva.
