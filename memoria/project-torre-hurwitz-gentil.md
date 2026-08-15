---
name: project-torre-hurwitz-gentil
description: "O eval dos hipercomplexos é METADE — é Hurwitz, o discreto. A outra metade é Gentil (o contínuo, sem grau) e Lebesgue (a soma reversível que os casa), e o tecto de 8 é da NORMA."
metadata: 
  node_type: memory
  type: project
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T19:18:46.302Z
---

O `eval.txt` trouxe ℂ, ℍ, 𝕆, Cayley–Dickson, a tabela das perdas e dez exercícios. O
Aarão: **«aquilo é metade viu, e a outra metade da torre é gentil, e lebesgue
discreto-contínuo»**.

## O teorema central (`corpo-estelar.tex` thm:central)

- **HURWITZ** — o lado DISCRETO: a norma N(x) = Σxᵢ² lida pela cruz, multiplicativa para
  o produto **bilinear** exactamente nos graus 1, 2, 4, 8. **Conta o domínio.**
- **GENTIL** — o lado CONTÍNUO: a norma e a fusão homogénea, **sem limite de grau**.
- **LEBESGUE** — a soma reversível que os casa: ∫f + ∫f⁻¹ = b·f(b) − a·f(a), e na forma
  discreta **Σxₙ + Σ#{xₙ<v} = N·q**, exacta em inteiros. Já medido em
  `tests/lebesgue_toro.js` (7:0) — **não se refaz**.
- E a bijeção dual é **A ESTRELA**, ν∘ν = id resíduo 0. «Nunca se avalia uma raiz.»

**A frase que decide:** «o limite no grau oito é do lado discreto/bilinear — Hurwitz
classifica o bilinear —, **NÃO DO OBJECTO**».

## O que isso muda

A tabela do eval tem uma coluna só de ✗ a descer (comuta, associa, divide). **Todas essas
perdas são do lado da NORMA.** Pelo `def:octoniao-dual`: 𝕆 = ℍ × ℍ*, dois tecidos ligados
pela estrela — pela norma perde a associatividade; **pela dualidade não perde nada**.

E isso mede-se **onde a norma já morreu**. O `tests/hurwitz.c` §H5 **parava em 8** — media
a involução só onde a norma também valia, que é onde a pergunta não tem gume. Agora: em
16 a multiplicatividade está partida e ν∘ν = id continua com resíduo ZERO, na mesma linha.

## A torre da casa já era escrita com o dual (`corpo_peano.tex` thm:rn)

    A_{n+1} = A_n + A_n*,     dim A_n = 2ⁿ

«a ordem sobe por indução; **NÃO é herdada da reta — é PRODUZIDA pela dualidade**». O eval
escreve A_n ⊕ A_n e; a casa diz de onde vem o `e`: a segunda cópia entra **conjugada**.

## E a tabela reproduz-se por BUSCA, não por citação

O comutador aparece em dim 4 (e₁,e₂), o associador em 8 (e₁,e₂,e₄), o divisor de zero em
16 ((e₁+e₁₀)(e₅+e₁₄) = 0) — e os buscadores voltam vazios em baixo, que é o controlo.

Realizado em `lib/torre.h`, `tests/hurwitz.c` §H5–H7 e `banco/conversa.c` §C41 +
«torre 1..16». Ver [[feedback-o-tecto-do-array]], [[project-o-fecho-do-dual-lagrange]].
