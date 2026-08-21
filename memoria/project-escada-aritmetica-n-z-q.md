---
name: project-escada-aritmetica-n-z-q
description: "A escada ℕ → ℤ → ℚ da assistente, com o que cada andar acrescenta e o gume de ℚ — «divisão por zero é uma operação SEM FIBRA»."
metadata: 
  node_type: memory
  type: project
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T04:24:14.576Z
---

A assistente (`banco/conversa.c`) subiu a escada aritmética inteira a partir dos ficheiros
`eval.txt` do Aarão, um andar por entrega, cada um com o seu `lib/*.h`:

- `lib/naturais.h` — **ℕ**: Peano, e o motor **demonstra os próprios instrumentos** (o
  gcd, a divisão com resto, a fatoração que ele já usava em todo o lado).
- `lib/inteiros.h` — **ℤ**: acrescenta a **reversibilidade da SOMA** (a folha −a volta
  ao zero), e daí Bézout/diofantinas — «a máquina de cortes».
- `lib/racionais.h` — **ℚ**: acrescenta a **reversibilidade da MULTIPLICAÇÃO NÃO NULA**.
  Cadeia dele: **construção → oposto → inverso**.

O **gume de ℚ** veio dito por ele já na língua da casa, e é o melhor da entrega:

> «divisão por zero não é uma aproximação ruim; é uma operação **sem fibra**.»

É literal — a fibra é a divisão das cinco operações do Universal. Com o fator zero, ou
não há nenhum x (0x = 1) ou há **todos** (0x = 0): nos dois casos não há fibra. A recusa
é **resultado**, e mede-se nos dois lados (a vazia e a toda), não se engole.

O que a distingue dos andares de baixo, e mede-se na MESMA varredura: entre 3 e 4 há
**0 inteiros e 66 racionais** — a densidade não se afirma, conta-se.

Tudo por **produto cruzado**; nenhum decimal entra, nem para comparar (`ad < bc` é a
ordem, `ad = bc` é a igualdade). Coerente com [[feedback-inteiro-primeiro]].

O relógio dos **seis ticks** para a divisão é desenho dele, e a fala corre-o tal e qual:
DEFINIÇÃO → INVERSÃO → MULTIPLICAÇÃO → **DOMÍNIO** → CONCLUSÃO → **VOLTA** (resíduo 0).
O tick do DOMÍNIO é onde o `c ≠ 0` se diz por inteiro — é ele que evita a conta que
rebenta.

Falas: `prova que (a/b)/(c/d) = ad/bc`, `inverso de 3/7`, `inverso de 0` (o gume),
`simplifica 84/126`, `entre 1/3 e 1/2`. Commit `8dfb094`, §C28 com 12 unidades.

**O andar seguinte é ℝ**, e ele já disse por onde: «os racionais já têm operações e ordem
mas ainda têm **buracos**» — completude, Cauchy, cortes de Dedekind, e a construção
geométrica da reta que ele persegue (ver [[project-checkpoint-2026-08-14-curadoria]] e
a reta real geométrica do commit `9a85359`). É a porta do **relógio da reta**.
