---
name: project-a-casa-ja-corria-cayley
description: A `estaca` da casa É Cayley–Hamilton, e as duas cartas são as duas formas quadráticas — o andar novo não trouxe motor, trouxe o NOME.
metadata:
  type: project
---

Ele pediu «busca referências sobre matrizes no repo, em teoria.tex ou corpo estelar». A
busca deu mais do que uma lista: deu que **o andar já corria, sem o nome** (commit
`74f4139`, `lib/forma.h` + §C38).

## O que já cá estava

| a casa chamava | e é |
|---|---|
| `mat2.estaca(m)` = mI − A_m, com **A·(mI−A) = −I** | **Cayley–Hamilton**: A² − mA − I = 0 é p_A(A)=0 |
| `mat2.W(m)` = 2A − mI, com **W² = (m²+4)I** | a mesma relação, a realizar o discriminante em matriz |
| **T = C + C⁻¹ com T² = T + I** (o pentágono) | a terceira aparição |
| as duas **cartas**: a²+b² e a²+mab−b² | as duas **formas quadráticas** do §3, definida e indefinida |
| M_ij = Σ(xᵢ−cᵢ)(xⱼ−cⱼ) no `corpo-estelar.tex` | a matriz **simétrica** cuja massa escalar é o **traço** |

**A lei que o explica, medida**: «a quadrática do ponto fixo deriva-se da matriz»
(`cx² + (d−a)x − b`) coincide com o **característico** (`λ² − tr·λ + det`) exatamente nas
**companheiras** — 25 das 625 varridas, e nas companheiras sempre. É essa coincidência que
põe **o inversor e o espectro no mesmo sítio** nesta casa.

**E fica uma ponte por fazer**: «a massa escalar é apenas o seu traço, logo a massa tem
DIREÇÃO» — essa direção É a decomposição espectral de M_ij. O teorema espectral aplicado
ao corpo estelar.

## A régua que tornou o andar possível: a raiz nunca se tira

Cauchy–Schwarz mede-se **ao quadrado** (`⟨u,v⟩² ≤ ⟨u,u⟩⟨v,v⟩`) e fica exato em ℚ; os
valores singulares por **σ²**; e Gram–Schmidt dá a base **ortogonal** sem normalizar,
porque normalizar era dividir por `√⟨u,u⟩`. A ortonormal existe — vive um andar acima.
É [[feedback-inteiro-primeiro]] a chegar ao andar analítico.

## E a forma nova da asserção vazia: a TESE SEMPRE VERDADEIRA

Duas asserções não mordiam, pela mesma razão: **sobre o produto euclidiano,
Cauchy–Schwarz nunca PODE ser falsa**, logo uma implementação que devolvesse sempre «sim»
era indistinguível da correta. A cura foi **generalizar a função à forma** — `fb_cauchy(A,…)`
alimentada com a indefinida da casa tem de devolver «não», e aí morde (cai em 6016 pares).

**Regra**: quando a tese é sempre verdadeira no domínio testado, a implementação que a
decide não está a ser testada. Alargar o domínio até onde ela possa ser falsa — e é o mesmo
que [[feedback-varrer-onde-nada-pode-falhar]] visto do lado da função em vez dos dados.


## E o andar seguinte (Gram → exterior, `18aa6c7`)

Três exigências sobre COMO medir, e nenhuma de conteúdo:

- **«Não basta olhar para a matriz: é preciso PROCURAR TESTEMUNHAS»** — a assinatura passou
  a ser caçada, com o vetor de cada sinal exibido.
- **O contra-exemplo em característica 2**: a polarização precisa do 1/2, e em 𝔽₂ ele é uma
  **fibra vazia** — a exceção de sempre a derrubar uma identidade. E lá −1 = 1, logo
  simétrica e antissimétrica **colapsam** (2 formas medidas que são as duas coisas).
- **A correção ao meu método**: [[feedback-medir-os-extremos]].

E o determinante deixou de ser receita: `Tu ∧ Tv = det(T)·(u∧v)` — «a ação do operador no
VOLUME ORIENTADO». O tensorial fecha com a universalidade `B̃(u⊗v) = B(u,v)`, que é «a
versão abstrata de transformar duas entradas numa operação única».
