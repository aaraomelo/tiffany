---
name: project-corpos-a-escada-fecha
description: O andar dos corpos na assistente — onde «toda operação com fibra tem volta» vira estrutura formal, e a exceção continua a ser a mesma.
metadata:
  type: project
---

Commit `71a0b73`, `lib/corpo.h` + §C36, 25 exercícios em quatro níveis (`corpos` dá o
índice). Zero doubles.

## A frase que fecha a escada

> «corpo é praticamente o ponto em que "toda operação que tem fibra tem volta" vira uma
> estrutura algébrica formal. A **exceção** continua sendo exatamente a que vocês já
> descobriram: **0⁻¹ não existe**.»

E com a reversibilidade nova em cada salto: ℕ (+ ×) → ℤ (a↦−a) → ℚ (a↦a⁻¹) → ℝ
(completude) → K (inversão fechada). Ver [[project-escada-aritmetica-n-z-q]] e
[[project-o-real-e-o-corte]].

**O andar quase não trouxe motor**: o `nm_inv_mod` já era o inverso por Euclides, o
`an_corpo` já decidia, o `nm_ordem` já dava o primitivo. Faltava só a EXTENSÃO — e o 𝔽₄
é medido pelo **mesmo `an_corpo`** que mediu o ℤ₅, não por uma segunda régua
([[feedback-duas-reguas]] a favor, desta vez).

**Onde o gume estava**: se eu tivesse medido a característica só em ℤₘ, char = m e o
teorema parecia dizer «char = |K|» — que é falso. São as EXTENSÕES que o decidem: 𝔽₄ tem
4 elementos e característica 2. É [[feedback-varrer-onde-nada-pode-falhar]] outra vez, na
forma «o exemplo onde as duas quantidades coincidem».

**A fibra em ℚ(√2) é a NORMA**: a² − 2b² só zera no zero, porque √2 ∉ ℚ. E o conjugado é
o **dual** (x†† = x, xx† = N(x)) — o mesmo par que a casa tem em todo o lado.

**A redundância que é a tese**: o guarda `if(a == 0) return -1` em `corpo_inv` é redundante
(medido) — a busca já recusa sozinha, porque 0·b = 0 ≠ 1 para todo b. **O zero não fica sem
inverso por decreto nosso: fica porque a fibra é literalmente vazia, e a máquina
descobre-o a procurar.**

## Os defeitos, e o que ensinam

1. **Estouro de buffer** que travou a máquina — [[feedback-o-teto-nao-verificado]].
2. **Quatro `%ld` dentro de `tique()`**, que não formata: dois estavam lá desde sessões
   anteriores a imprimir o literal. Quinta forma de [[feedback-o-destino-rotativo]].
3. **O `\pmod` outra vez** — o comando que deu ORIGEM ao bench da membrana, reintroduzido
   por mim. O medidor apanhou-o na hora; sem ele teria ido para o corpus.
4. **O nome «corpo de 9» comido pelo prefixo «corpo»** — a fala morria calada, e foi a
   varredura do índice que a apanhou, não a leitura. Regra: **nome exato antes do prefixo**.
5. **Um limiar meu escrito de cabeça** (`> 300` quando o verdadeiro é 225) — agora o total
   confere com Σ(p−1) contado por um segundo caminho.
