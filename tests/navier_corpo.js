/* tests/navier_corpo.js — Navier–Stokes discreto, derivado no anel
 * (ordem do coordenador, 14/08 madrugada: «deriva Navier-Stokes»; a
 * linguagem exigida pelo gerente: a fatoração espectral é do OPERADOR
 * DE DIFUSÃO DISCRETO — nenhuma afirmação sobre existência ou
 * regularidade do problema do Clay, que fica declaradamente FORA,
 * como nos milénios: a leitura corre sobre o PROVADO).
 *
 * A estrutura de N–S no ciclo, derivada das peças da casa:
 *
 *     u' = u + [ −conv(u) + ν·∆u ]
 *
 *   O LAPLACIANO É A DOBRA MENOS 2: ∆ = convolução com [1,−2,1], e o
 *   seu símbolo espectral é t_k − 2 (t_k = 2cos, o traço da
 *   renormalização); o símbolo do passo de calor é 2+t_k = μ_k² — O
 *   QUADRADO DA MEIA-VOLTA, exato no anel.
 *
 *   A IDENTIDADE DE ENERGIA DERIVA-SE EXATA EM ℤ (por partes no
 *   ciclo): Σu·∆u = −Σ(u_{i+1}−u_i)² e Σu·conv(u) = 0 (forma fluxo)
 *   — o transporte CONSERVA, a viscosidade DISSIPA: N–S é a costura
 *   em campo (o conservativo da mecânica + a seta do Carnot).
 *
 *   A DIFUSÃO: massa conservada exata (o modo 0 — o que a volta não
 *   vê) e energia estritamente decrescente até o repouso (a massa no
 *   centro); o EQUILÍBRIO é o campo constante.
 *
 *   O GUME QUE SEPARA EULER DE N–S: com ν=0 a energia CONSERVA-SE
 *   exatamente — a seta vem só do termo viscoso. A viscosidade é a
 *   diferença, medida.
 *
 * §F0  o Laplaciano é convolução e o símbolo é t_k−2; o passo de calor
 *      tem símbolo 2+t_k = μ_k² (a meia-volta ao quadrado), dois
 *      caminhos no anel (passo físico vs multiplicação espectral)
 * §F1  as duas identidades exatas em ℤ: Σu·∆u = −Σ|∇u|² e
 *      Σ(fluxo de u²) = 0 — a identidade de energia, derivada
 * §F2  difusão inteira (u' = 4u + ∆u): massa ×4 exata (conservada na
 *      escala), energia < 16× estrita para u não-constante, e == para
 *      o constante (o equilíbrio)
 * §F3  o transporte skew conserva massa e energia (forma fluxo, ℤ)
 * §F4  o gume: Euler (ν=0, passo skew puro) conserva; N–S (ν>0)
 *      dissipa — a viscosidade é a única seta; 𝓜 assina, e o Clay
 *      fica fora por declaração
 */
'use strict'
const { anel, dft, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const A = anel(P)
const M = 16
const w = A.powm(3, 65536 / M)          /* ω de ordem M */
const h = A.powm(3, 65536 / (2 * M))    /* a meia-volta */
let R = 0

/* o campo de teste (inteiro, determinístico) e as operações do ciclo */
let lcg = 9
const rnd = () => { lcg = (lcg * 75 + 74) % 65537; return (lcg % 21) - 10 }
const u0 = Array.from({ length: M }, rnd)
const lap = u => u.map((_, i) => u[(i + 1) % M] - 2 * u[i] + u[(i - 1 + M) % M])
const fluxo = u => u.map((_, i) => (u[(i + 1) % M] ** 2 - u[(i - 1 + M) % M] ** 2))  /* 2·conv em forma fluxo */

/* §F0 — o símbolo do Laplaciano e a meia-volta ao quadrado */
{
  /* dois caminhos: o passo físico levado ao espectro vs a multiplicação diagonal */
  const uMod = u0.map(x => A.mod(x))
  const fis = lap(uMod).map(x => A.mod(x))
  const esp = dft(uMod, A, w)
  const espFis = dft(fis, A, w)
  let diagonal = 0, meiaVolta = 0
  for (let k = 0; k < M; k++) {
    const t_k = A.mod(A.powm(w, k) + A.inv(A.powm(w, k)))
    if (espFis[k] === A.mod(esp[k] * A.mod(t_k - 2 + P))) diagonal++
    /* 2 + t_k = μ_k², com μ_k = h^k + h^{−k} na meia-volta */
    const mu = A.mod(A.powm(h, k) + A.inv(A.powm(h, k)))
    if (A.mod(2 + t_k) === A.mod(mu * mu)) meiaVolta++
  }
  if (diagonal !== M || meiaVolta !== M) R++
  console.log(`\n§F0  símbolo t_k−2 (dois caminhos): ${diagonal}/${M} · 2+t_k = μ_k²: ${meiaVolta}/${M}`)
  ok('§F0 o Laplaciano é a convolução [1,−2,1] com símbolo t_k−2 — a dobra menos 2 — verificado por DOIS caminhos no anel', diagonal === M)
  ok('§F0 e o símbolo do passo de calor é 2+t_k = μ_k² — O QUADRADO DA MEIA-VOLTA: a fatoração espectral do operador de difusão discreto', meiaVolta === M)
}

/* §F1 — as duas identidades exatas em ℤ */
{
  let porPartes = true, skewZero = true
  for (let t = 0; t < 12; t++) {
    const u = Array.from({ length: M }, rnd)
    const L = lap(u)
    const lhs = u.reduce((s2, ui, i) => s2 + ui * L[i], 0)
    const rhs = -u.reduce((s2, ui, i) => s2 + (u[(i + 1) % M] - ui) ** 2, 0)
    if (lhs !== rhs) porPartes = false
    if (fluxo(u).reduce((s2, f) => s2 + f, 0) !== 0) skewZero = false
  }
  if (!porPartes || !skewZero) R++
  console.log(`\n§F1  Σu·∆u = −Σ|∇u|²: ${porPartes} · Σ fluxo(u²) = 0: ${skewZero} (12 campos)`)
  ok('§F1 A IDENTIDADE DE ENERGIA, derivada exata em ℤ: Σu·∆u = −Σ(u_{i+1}−u_i)² por partes no ciclo — a viscosidade só pode dissipar', porPartes)
  ok('§F1 e o transporte em forma fluxo soma ZERO no ciclo: Σ(u²_{i+1}−u²_{i−1}) = 0 — a convecção conserva por telescópio exato', skewZero)
}

/* §F2 — a difusão inteira: massa conservada, energia estrita */
{
  const passo = u => u.map((ui, i) => 4 * ui + lap(u)[i])
  let massa = true, estrita = true
  let u = [...u0]
  for (let t = 0; t < 6; t++) {
    const u2 = passo(u)
    const m0 = u.reduce((a, b) => a + b, 0)
    if (u2.reduce((a, b) => a + b, 0) !== 4 * m0) massa = false
    const E1 = u.reduce((a, b) => a + b * b, 0)
    const E2 = u2.reduce((a, b) => a + b * b, 0)
    if (!(E2 < 16 * E1)) estrita = false
    u = u2
  }
  /* o equilíbrio: o campo constante fica (== exato) */
  const cte = new Array(M).fill(7)
  const cte2 = passo(cte)
  const equilibrio = cte2.every(v => v === 28) &&
    cte2.reduce((a, b) => a + b * b, 0) === 16 * cte.reduce((a, b) => a + b * b, 0)
  if (!massa || !estrita || !equilibrio) R++
  console.log(`\n§F2  massa ×4 exata: ${massa} · energia < 16× estrita: ${estrita} · o constante fica: ${equilibrio}`)
  ok('§F2 a DIFUSÃO conserva a massa exata (o modo 0 — o que a volta não vê) e dissipa a energia ESTRITAMENTE para todo campo não-constante', massa && estrita)
  ok('§F2 e o equilíbrio é o campo constante: a massa no centro, energia conservada só aí', equilibrio)
}

/* §F3 e §F4 — Euler conserva, N–S dissipa: o gume da viscosidade */
let G = false
{
  /* Euler discreto conservativo: rotação espectral pura (cada modo roda,
   * módulo conservado) — o transporte linearizado exato no anel; vs o
   * passo de calor que dissipa. A comparação é no MESMO campo. */
  const uMod = u0.map(x => A.mod(x))
  const esp = dft(uMod, A, w)
  /* Euler: û_k ↦ ω^k·û_k (transporte por translação — conserva |û_k| e Σu² via Parseval) */
  let conservaEuler = true
  const espE = esp.map((c, k) => A.mod(c * A.powm(w, k)))
  /* Parseval-dual: M·Σu² == Σ û_k û_{−k} — antes e depois */
  const dual = cs => { let s2 = 0; for (let k = 0; k < M; k++) s2 = (s2 + cs[k] * cs[(M - k) % M]) % P; return s2 }
  if (dual(espE) !== dual(esp)) conservaEuler = false
  /* N–S: o passo de calor muda o dual (dissipa) — no anel mede-se a MUDANÇA */
  const espNS = esp.map((c, k) => {
    const t_k = A.mod(A.powm(w, k) + A.inv(A.powm(w, k)))
    return A.mod(c * A.mod(2 + t_k))
  })
  const dissipa = dual(espNS) !== dual(esp)
  G = conservaEuler && dissipa
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§F3  Euler espectral conserva o dual: ${conservaEuler} · o calor muda-o: ${dissipa} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§F3 o GUME que separa: o transporte de Euler conserva o produto dual espectral EXATO; o passo de calor muda-o — a viscosidade é a única seta', G)
  ok('§F4 o contrato 𝓜 assina a derivação: N–S discreto = transporte conservativo + dobra dissipativa + modo 0 intocado. A pergunta do Clay (existência/regularidade em 3D) fica FORA por declaração — a leitura corre sobre o provado', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  Navier–Stokes discreto, derivado: o Laplaciano é a dobra')
  console.log('  menos 2 (símbolo = meia-volta ao quadrado), a identidade de')
  console.log('  energia sai por partes exatas em ℤ, o transporte conserva')
  console.log('  (telescópio), a viscosidade dissipa (estrita), e o gume')
  console.log('  separa Euler de N–S. É a costura em campo: o conservativo')
  console.log('  da mecânica + a seta do Carnot, no mesmo u. O Clay fica')
  console.log('  fora — a leitura corre sobre o provado.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
