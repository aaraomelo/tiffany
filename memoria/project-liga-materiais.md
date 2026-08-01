---
name: project-liga-materiais
description: "A colheita de RF, a dualidade em QUATRO (Wiedemann-Franz), a rotação sp3→sp2, e a liga grafeno+estanho que percola — e os quatro requisitos que não fecham juntos"
metadata:
  type: project
---

# A liga e os materiais — `colheita`, `octeto`, `liga`

01/08/2026, `a4c6b8b`→`02b74cb`. **221 medidores, 219 verdes, 0 falhas.**

## `colheita.c` — o caminho de volta, e a dualidade é em QUATRO

**Ler e escrever são adjuntos**, e a frase do Aarão é um teorema: **reciprocidade de Lorentz**,
`⟨Sf,g⟩ = ⟨f,Sg⟩`, resíduo zero. *O espelho é a transposta* — o mesmo par do `icc.c` §I4.

**Só a mistura absorve:** o metal reflete (`Z≈0`), o isolante transmite, e o máximo é **21,7% em
σ=3,46 S/m** com os dois extremos em 0%. *Falham por lados opostos.*

**Ouro e plástico são duais? PARCIALMENTE NÃO** — a média geométrica das condutividades erra por
5000×. *Eu ia dizer que sim porque a frase era bonita.* **Mas são duais na IMPEDÂNCIA**: `0,02 Ω` e
`233,6 Ω` cercam o `Z₀=376,7` por baixo e por cima.

**E a dualidade é em QUATRO** (pedido do Aarão: *"falta um par, vê diamante e prata"*). A prova de
que há **dois eixos independentes** é **Wiedemann–Franz**: nos metais `κ/(σT)` dá o número de Lorenz
(prata 0,93 L₀), e **o diamante viola-a por 3×10²¹** porque ali o calor vai por *fonões*.

                    conduz calor              isola calor
    conduz E        PRATA / OURO              Bi₂Te₃ (termoelétrico)
    isola E         DIAMANTE (dissipador)     PMMA (isolante)

O headjack usa **três**: prata na antena, Bi₂Te₃ no Seebeck, diamante no dissipador (*e isola E,
logo não curto-circuita a antena*).

## `octeto.c` — a rotação sp³→sp² muda o canto

**Diamante e grafeno são o mesmo átomo**, e a passagem **é uma rotação** com ângulos que se
*derivam*: `sp³ = arccos(−1/3) = 109,4712°` e `sp² = 120°`. **10,53° de rotação**, e o material muda
de canto — `1e-13` para `1e8 S/m`, **21 ordens**, mesmo octeto. *A razão é o π: o sp² deixa um
eletrão deslocalizado por átomo.*

**O octeto fixa QUANTAS ligações, não a FORMA** — e é essa folga que deixa lugar à rotação.

**Correção:** ouro + prata dá **electrum**, não bronze (que é cobre + estanho). E a emenda do Aarão
a meio (*"ou grafeno + estanho"*) **é melhor**: o estanho fecha o octeto, é condutor sem ser nobre, e
é o **único metal comum com alotropia como a do carbono** (branco metálico / cinzento semicondutor).

## `liga.c` — percola, e os requisitos conflituam

**Não interpola: PERCOLA.** `σ ∝ (p−pc)²`, salto de `1e14` no limiar. *O que decide não é a
quantidade — é a geometria de quem toca quem.* A janela é **larga em relativo (11%) e minúscula em
absoluto (1,3e-4)** — *é o absoluto que o fabrico vê*.

**E os quatro requisitos não fecham juntos:** absorver pede `p=0,0012`, dissipar pede `p=0,20` —
**200× de diferença**. A saída são **quatro camadas da mesma química**, ordenadas sozinhas.

## As famílias de erro desta série

**1. O ABSURDO NO RESULTADO denuncia o que a asserção não apanha.** Uma reflexão de **159% num
material passivo** é impossível — e a asserção passava. *Um número impossível no relatório é um
defeito, mesmo com a bateria verde.* **Gatilho: ler os números do relatório como se fossem de outra
pessoa, e perguntar se algum é fisicamente absurdo.**

**2. `printf` partido em dois com os ARGUMENTOS DE FORA** — os 159,5% eram lixo da pilha. Compilar
com `-Wformat` apanha isto e eu não o tinha ligado.

**3. O SINAL, pela TERCEIRA vez no mesmo dia** (`cimag(k)` no `colheita`, `csqrt` no `liga`, e o
`/(-12h)` no `dominios`). Nas três fui olhar para a fórmula em vez do **ramo da raiz**.
Ver [[feedback-simulacao-nao-bate]].

**4. E escrevi outro `1 == 1 ? ...`** no mesmo dia em que registei a constante disfarçada como a
sexta forma. **Escrever a regra não me impede de a violar meia hora depois; o que impede é MEDIR.**
Ver [[feedback-assercoes-vazias]].
