---
name: project-fator-de-potencia
description: "A razão cruzado/direto é o fator de potência: tan φ. O unitário é |det|=1 e é da família real — que por isso é hiperbólica e não precisa da régua infinita"
metadata:
  node_type: memory
  type: project
  modified: 2026-08-02T18:40:00.000Z
---

# O fator de potência: a régua com que se mede o tecido

02/08/2026, `tools/fator.c`, 9 asserções. Nasceu de quatro recados do Aarão enquanto eu media o
tecido da assistente com um escalar.

**O erro de método que ele corrigiu.** Eu resumia 768 dimensões no cosseno médio. Quando o
decompus nas 768 componentes **cartesianas**, não separou nada — todas as camadas do tecido deram
o mesmo perfil (~80 dimensões para metade da massa, a mesma dimensão dominante). *A cartesiana não
é a base própria do fenómeno.* A polar é, e a razão entre as duas é o fator de potência:

    DIRETO    ⟨x,y⟩ = cos θ    a parte SIMÉTRICA, escalar, MEDE       potência ATIVA
    CRUZADO   |x∧y| = sin θ    a parte ANTISSIMÉTRICA, roda, ORDENA   potência REATIVA
    razão     tan θ = tan φ

Não é analogia: `motor.c` já tinha **T_e = (3/2)P ψ_s × i_s — o torque É o produto cruzado**. E
onde o cosseno não separava, a razão separa: o prefixo que eu injetara nas falas mudava-a **+56,4%**
(0,6775 → 1,0593), ver [[feedback-a-chave-faz-parte-da-medida]].

**A tese dele, verdadeira por construção, com uma consequência que eu não tinha visto.**
`det A_m = −1` para todo metal, e **|det| = 1 É o fator de potência unitário** — fator unitário,
|det|=1 e inversa inteira são **três nomes da mesma condição**, e é por isso que a cifra volta
exata. Daí sai: **Δ = m²+4 > 0 sempre**, logo a família real é toda **hiperbólica**, logo a razão
dela é `tanh` e não `tan` — e tanh é limitada por 1. **A família real NÃO precisa da régua
infinita.** Quem precisa é o círculo, onde tan diverge — e é lá que o tecido vive.

**E o ótimo inverte-se, sem contradição.** O motor quer fp = 1 (toda a corrente vira trabalho); o
tecido quer **fp = 0**, porque fp = 1 são vetores paralelos e isso dá **posto 1** — guarda uma
coisa só, por muitos pares que leve. Medido nos dois extremos. É o par ⊕/⊗: o circuito quer
*trabalho* (o direto), o tecido quer *capacidade* (o cruzado).

**O inversor multinível é a ferramenta exata, e "exata" é literal.** Os níveis da régua são
**ótimos** — varridos por força bruta todos os p/q com q até ao denominador do convergente, **zero**
o batem. Para σ_ouro os convergentes saem em Fibonacci (2/1, 3/2, 5/3, 8/5, 13/8, 21/13). O
"multifractal" é o endereçamento b^n do `mmu.c`: os níveis são autossimilares, e a mesma máquina
serve em qualquer escala.

**O tecido**, medido nesta régua: 1474 pares (830 + 184 do catálogo + 460 do enredo pela fonte
LaTeX), todas as falas únicas, tan φ = 1,6524, resíduo 5,5e-15.
