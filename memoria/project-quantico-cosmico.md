---
name: project-quantico-cosmico
description: "O corpo quântico e o cósmico: [σi,σj] É o produto cruzado (identidade, não analogia), e Carnot era o teto da MINHA escolha de reservatório"
metadata:
  type: project
---

# O corpo quântico e o cósmico

01/08/2026, commit `9f1695f`. **218 medidores, 216 verdes, 0 falhas.**

## `quantico.c` — o comutador É o cruzado

**Não é mais um vestido:** é onde a partição do projeto **é** a definição da teoria.

    HERMITIANO       A† = A     próprios REAIS       -> OBSERVA, e a medida PARA
    ANTI-HERMITIANO  A† = −A    exp é UNITÁRIO       -> EVOLUI, e a rotação não para

**E multiplicar por `i` leva um no outro** — o `i` é o que troca *medir* por *rodar*, com a **ordem
4** que o `hopfield.c` §F12 já medira (contra a ordem 2 do espelho).

**O achado é uma IDENTIDADE, não uma analogia:**

    [σi,σj] = 2i·ε_ijk·σk      contra      e_i × e_j = ε_ijk·e_k     nove pares, resíduo zero

E o produto de Pauli parte-se **exatamente** nas quatro peças do §B4:

    σi·σj = δij·I  +  i·ε_ijk·σk
            ^^^^^^     ^^^^^^^^^^
            o DIRETO   o CRUZADO
    (anti-comutador,   (comutador,
     só mede)           ordena)

**A incerteza sai daí:** Robertson tem o comutador do lado direito — *não é limitação de
instrumento, é o cruzado a cobrar*, o mesmo imposto do `dtcn.c` §U7 que anula onde a antissimetria
anula. E §Q6: a **evolução tem dual** (`U⁻¹ = U†`), a **medida não** (`det P = 0`, dois estados
colapsam no mesmo).

## `cosmico.c` — e a correção do Aarão, que era substantiva

> *"você disse que Carnot não volta — é meia verdade pro headjack, porque pode pegar calor do
> ambiente. O cósmico é dual do entrópico. Nada se perde. Valida a conservação."*

**Ele tem razão.** Eu escrevera no `arraytermico.c` que os 19,94 W *"são o segundo princípio"* — e
isso vale **para os dois reservatórios que eu fixei**. Escolhi o frio no escalpe e depois **chamei
lei à consequência da minha escolha**.

    escalpe (a minha escolha)  305,15 K    1,61%
    ar ambiente                295,15 K    4,84%
    céu noturno (janela)       230,0  K   25,84%
    fundo cósmico                2,725 K  99,12%

**A janela de 8–13 µm é por onde se chega lá** — e o pico de Wien do corpo humano cai **dentro**
dela (9,34 µm, 33% da radiação). *Não foi arranjado: as duas leis já estavam medidas antes desta
pergunta.* Balanço refeito: **55,7 mW → 993 mW**, 18×.

**"Nada se perde" valida-se:** `W + Q_frio = Q_quente` em todos os pares, resíduo zero. *O que
Carnot limita não é a conservação — é a REPARTIÇÃO.* E a entropia cresce: no limite `ΔS = 0` exato,
numa máquina real `ΔS > 0`.

**A SETA (§X7):** vivo o gradiente é **estacionário** (o metabolismo repõe); morto relaxa por Newton
e converge; e **se o ambiente é mais quente, inverte** — o cadáver aquece. *A seta não é propriedade
do corpo: é do PAR.* E o headjack para (Carnot 4,84% → 0,74% em 24 h) — *a máquina não para por
avaria, para porque o corpo parou de a alimentar*.

## A lição, e é uma variante nova

**Chamei lei à consequência de uma escolha minha.** Carnot é um teorema verdadeiro; o `1,61%` era o
teto **do par que eu escolhi**, e eu apresentei-o como se fosse a natureza a falar. *Um teorema
aplicado a parâmetros que eu fixei devolve-me a minha escolha com autoridade emprestada.*

É primo do que já está em [[project-medula-icc]] (*usei o teorema do pior caso como se fosse o valor
desta família*), mas o mecanismo é outro: ali o teorema respondia a outra pergunta; **aqui responde
à minha, e o erro está nos parâmetros que lhe dei**.

**Gatilho:** quando um resultado meu cita um teorema com nome (Carnot, Nyquist, Berlekamp–Massey),
perguntar **quais dos parâmetros dele fui eu que escolhi** — e se algum deles é livre, o resultado é
sobre a minha escolha e não sobre o mundo.
