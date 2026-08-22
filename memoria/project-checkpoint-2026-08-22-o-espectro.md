---
name: project-checkpoint-2026-08-22-o-espectro
description: "22/08 — o corpo desta casa JÁ ERA uma matriz 2×2: o cifra.h dizia-o na porta. O linear.h e o forma.h inteiros no motor, e o tensor ganhou o terceiro uso."
metadata:
  type: project
---

# 22/08 — O ESPECTRO, e o terceiro uso do tensor

`tests/pgwire.c` de **60 → 69 asserções**, bateria **530 verdes, 0 falhas**.

## A unificação: o corpo é a matriz 2×2, e ninguém precisou de os identificar

O `lib/cifra.h` **não foi escrito para matrizes** — foi escrito para corpos, e diz
na porta que as suas duas grandezas são «a razão, quanto se estica por nível → o
**traço** B» e «o sinal, se as duas direções se cancelam → o **determinante** C».
E diz mais: *«o hipercorpo não tem (B,C): o seu operador não é uma matriz 2×2»*.

Logo a cifra de um corpo **é o espectro** da matriz: as raízes de `λ² − Bλ + C`
em fração contínua periódica, exatas, sem uma raiz calculada, com o período como
invariante completo (Lagrange). `cifra(*)` não acrescentou álgebra: pegou no
traço e no determinante que o motor já dava.

E daí, de graça:

- **semelhantes ⟹ mesma cifra** — testemunha: `(2,1;−1,−1)` não tem uma entrada
  igual à de Fibonacci e devolve a mesma;
- **a rotação** `(0,−1;1,0)` tem `B²−4C = −4`, e o primeiro termo da cifra — «qual
  metade carrega o real» — passa de 1 a **2** sozinho. É o `i` a aparecer como a
  matriz cujo espectro só existe do outro lado;
- **`tₙ = B·tₙ₋₁ − C·tₙ₋₂`**: o traço de `Aⁿ` obedece à recorrência do corpo. Oito
  potências encadeadas com `produto`+`traco` deram 3, 4, 7, 11, 18, 29, 47 — os
  números de **Lucas**, que ninguém escreveu.

**E o escopo, que é o que impede a insinuação:** `mesma cifra ⇏ semelhantes`.
Jordan `(2,1;0,2)` e a homotetia `(2,0;0,2)` têm a mesma cifra `1 2 4 1 4 1 3` e
não são semelhantes (a homotetia comuta com tudo). A cifra é completo para o
**corpo**, não para a **matriz** — e quem as separa são os autovetores, um contra
dois. Ver [[feedback-o-escopo-da-afirmacao]].

## O que entrou no motor

`linear.h` e `forma.h` inteiros (com o alívio `#define Mat MatQz`, e o `cifra.h`
antes do `forma.h` porque é dele o `raizi` — o que faz dos dois caminhos para o
espectro duas leituras do **mesmo** discriminante).

`dual` · `aniquilador` · `gram` · `autovalores` · `autovetores` · `cifra`.

- **a conservação pela quarta vez**: `dim W + dim W° = n`, ao lado de `∑G = |I|`,
  `|presentes|+|ausentes| = |I|` e `posto + dim(ker) = n`;
- **`det G = (det A)²`** — lei a mais do que a que eu ia medir, de duas peças que
  o motor já tinha e nunca tinham sido postas em contacto;
- **`autovalores` recusa e REMETE** para a cifra quando o discriminante não é
  quadrado: a recusa é a resposta, não um limite.

## O terceiro uso do tensor: decidir, produzir, **escrever**

`UPDATE t SET b = b + 1` era «não entendido». A falta era assimetria, não
conveniência. O que a impedia: `S_V` guarda UM valor. A saída não foi sair da
ISA — grava-se `S_V` com o valor daquela linha e emite-se um programa para ela.

**E produzir não é escrever**: `SELECT b/2` responde `11/2`; a coluna é de
inteiros e o `UPDATE` recusa. A distinção é o **corpo** — a figura dos alcances
diferentes (oposto sempre, inversa às vezes).

## Quatro defeitos reais

1. o tecto que se contradizia (`>=` a recusar a 6×6 que a mensagem dizia caber),
   e a **inversa a ter tecto próprio** (`n×2n`, logo metade);
2. **um zero podia ser duas coisas** — `qz_mult` devolve 0 no estouro, e 0 é um
   determinante legítimo. Ver [[feedback-saturacao-nao-e-resultado]] ponto 5;
3. a **declaração que se calava** e a **recusa que criava** —
   [[feedback-a-recusa-que-deixa-rasto]];
4. **`agr_op` era uma variável só** — `SELECT MIN(a), MAX(a)` devolvia o máximo,
   com `ok`. Agora array, e **uma varredura só**, que é o que garante que MIN e
   MAX vêem o mesmo conjunto.

E a correção do 4 teve o seu próprio defeito, gravado em
[[feedback-decidir-onde-nao-se-sabe]].
