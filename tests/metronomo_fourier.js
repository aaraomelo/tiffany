/* tests/metronomo_fourier.js — a representação espectral do Metrónomo,
 * e o Maestro como derivação (ordem da mesa, eval 14/08).
 *
 * A frase do coordenador, f(t)=Σ c_k e^{ikt}, vira no discreto HONESTO
 * (diretor: «sem hallucination de derivada contínua»):
 *     x_n = N⁻¹ Σ_k c_k ω^{kn},   c_k = Σ_n x_n ω^{−kn},   ω^N = 1
 * no anel — e os c_k não se postulam: MEDEM-SE da órbita da batuta que
 * o toro já auditou. O que a medida dá:
 *
 *   OS c_k APRESENTADOS: o espectro da órbita tem EXATAMENTE DOIS
 *   modos não-nulos — e eles são AS DUAS FOLHAS (ω^{k₁}=σ, ω^{k₂}=σ†,
 *   com σσ†=−1). O Metrónomo mede QUANDO (o tick é diagonal: cada modo
 *   roda pela própria frequência; a folha roda pela própria folha); o
 *   Maestro decide QUAIS (a retração π é a DOBRA dos modos
 *   c_k+c_{k+N/2}, e o projetor construído da própria batuta é o
 *   seletor a_k ∈ {0,1}).
 *
 * Fourier NÃO fundamenta o Metrónomo: representa o operador já medido.
 *
 * §MF0  os c_k, medidos e apresentados: dois modos não-nulos, nas
 *       folhas (ω^{k₁}=σ, ω^{k₂}=σ†, σσ†=−1) — q=257, m=2, N=128
 * §MF1  Parseval no anel: Σ_k c_k c_{−k} = N·Σ_n x_n² exato — R=0
 * §MF2  o Metrónomo é diagonal: tick (x_{n+1}) ⟺ c_k ↦ ω^k·c_k em
 *       TODOS os k; nas folhas, ω^{k}=σ — o tick roda a folha pela
 *       própria folha
 * §MF3  a derivada discreta honesta: Δx = x_{n+1}−x_n ⟺
 *       c_k ↦ (ω^k−1)·c_k — o «ik» do contínuo é ω^k−1 no anel
 * §MF4  o Maestro como DOBRA espectral: a retração d→d/2 (subamostrar
 *       a órbita) dá ĉ_k = 2⁻¹(c_k + c_{k+N/2}) exato — T+T* nos modos
 * §MF5  o Maestro como SELETOR: o projetor 2⁻¹(id + T^{N/2}) age como
 *       c_k ↦ a_k c_k com a_k ∈ {0,1} (pares); o complementar
 *       seleciona os ímpares; a_par + a_ímpar = 1 e os projetores são
 *       idempotentes — QUAIS modos participam, medido
 * §MF6  a volta: a inversa devolve x_n EXATO (128/128) com
 *       R_total = 0 — a representação não perde um bit
 * §MF7  o palco grande (65537, N=8192): as duas folhas continuam a ser
 *       todo o espectro — c ≠ 0 nas folhas, c = 0 em 8 modos de
 *       controlo tirados por LCG determinístico
 *
 *   node tests/metronomo_fourier.js
 */
'use strict'
const { Universal, mat2 } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')
const U = Universal(sigmaPeano)

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const { mul, Am } = mat2

/* fase 3: o anel é o da infraestrutura */
const { anel } = require('../lib/universal.js')

/* ── o palco pequeno: q=257, m=2, a órbita de período 128 do censo ───────── */
const q = 257
const { mod, powm, inv } = anel(q)
const m = 2
const A = Am(m).map(mod)
const aplica = v => [mod(A[0] * v[0] + A[1] * v[1]), mod(A[2] * v[0] + A[3] * v[1])]

/* a órbita real: parte de um representante e fecha em N=128 (medido) */
const N = 128
let v = [1, 0]
const orbita = []
for (let n = 0; n < N; n++) { orbita.push(v); v = aplica(v) }
ok('§MF0 a órbita fecha em N=128 (o período do censo da zeta), com A^N v = v exato',
  v[0] === orbita[0][0] && v[1] === orbita[0][1])
const x = orbita.map(w => w[0])                     /* o observável: 1.ª componente */

/* ω de ordem 128 no anel: g primitivo, ω = g^(256/128) */
const g = 3
ok('§MF0 3 é raiz primitiva mod 257 (ord = 256, e 3^128 = −1)',
  powm(g, 256) === 1 && powm(g, 128) === q - 1)
const w = powm(g, 2)                                /* ord(ω) = 128 */

/* fase 3: a DFT é a da lib (o w de cada tamanho deriva do de ordem 128) */
const { dft: dftLib } = require('../lib/universal.js')
const ANEL = anel(q)
function dft (xs) { return dftLib(xs, ANEL, powm(w, 128 / xs.length)) }
const c = dft(x)

/* §MF0 — os c_k apresentados: duas folhas */
{
  const naoNulos = []
  for (let k = 0; k < N; k++) if (c[k] !== 0) naoNulos.push(k)
  /* as folhas: raízes de t² − mt − 1 mod q */
  const raizes = []
  for (let r = 0; r < q; r++) if (mod(r * r - m * r - 1) === 0) raizes.push(r)
  const freqs = naoNulos.map(k => powm(w, k))
  console.log(`c_k não-nulos: k = {${naoNulos.join(', ')}} · c = {${naoNulos.map(k => c[k]).join(', ')}}`)
  console.log(`ω^k nas riscas: {${freqs.join(', ')}} · folhas σ,σ† = {${raizes.join(', ')}} · σ·σ† = ${mod(raizes[0] * raizes[1])} (= −1 ⟺ ${q - 1})`)
  ok('§MF0 OS c_k SÃO DUAS FOLHAS: exatamente 2 modos não-nulos, com ω^k = σ e ω^k = σ†',
    naoNulos.length === 2 && raizes.length === 2 &&
    freqs.slice().sort((a, b) => a - b).join() === raizes.slice().sort((a, b) => a - b).join())
  ok('§MF0 as folhas multiplicam para −1 (a estaca): σ·σ† ≡ −1 mod q',
    mod(raizes[0] * raizes[1]) === q - 1)
}

/* §MF1 — Parseval no anel (a forma da casa, cristal_energia §E1).
 * NOTA MEDIDA: na órbita da batuta os DOIS lados dão 0 — não é
 * tautologia minha, é estrutura: as folhas são isotrópicas na volta
 * (Σσ^{2n}=0 sobre o período; o termo cruzado leva Σ(−1)^n=0 porque
 * σσ†=−1). Por isso o gume vem do CONTROLO: uma sequência real de
 * norma NÃO-nula tem de fechar com valor ≠ 0. */
{
  let LHS = 0, RHS = 0
  for (let k = 0; k < N; k++) LHS = (LHS + c[k] * c[(N - k) % N]) % q
  for (let n = 0; n < N; n++) RHS = (RHS + x[n] * x[n]) % q
  RHS = RHS * N % q
  console.log(`Parseval (órbita): Σ c_k c_{−k} = ${LHS} · N·Σ x² = ${RHS} — o zero é das folhas (σσ†=−1)`)
  ok('§MF1 Parseval na órbita: Σ_k c_k c_{−k} = N·Σ_n x_n² (ambos 0 — a isotropia das folhas, medida)',
    LHS === RHS && LHS === 0)
  /* o controlo com gume: bytes reais do cristal, norma ≠ 0 */
  const fs2 = require('fs')
  const path2 = require('path')
  const bytes = Buffer.from(fs2.readFileSync(
    path2.join(__dirname, '..', 'cristal', 'cristal.jsonl'), 'utf8').slice(0, N), 'utf8')
  const y = []
  for (let n = 0; n < N; n++) y.push(mod(bytes[n]))
  const cy = dft(y)
  let L2 = 0, R2 = 0
  for (let k = 0; k < N; k++) L2 = (L2 + cy[k] * cy[(N - k) % N]) % q
  for (let n = 0; n < N; n++) R2 = (R2 + y[n] * y[n]) % q
  R2 = R2 * N % q
  console.log(`Parseval (cristal, controlo): Σ c c_− = ${L2} · N·Σ y² = ${R2}`)
  ok('§MF1 o controlo tem gume: nos bytes reais do cristal, Parseval fecha com valor ≠ 0',
    L2 === R2 && L2 !== 0)
}

/* §MF2 — o Metrónomo é diagonal no espectro */
{
  const y = x.map((_, n) => x[(n + 1) % N])          /* o tick */
  const cy = dft(y)
  let diagonal = true
  for (let k = 0; k < N; k++) {
    if (cy[k] !== c[k] * powm(w, k) % q) diagonal = false
  }
  ok('§MF2 o tick é DIAGONAL: c_k ↦ ω^k·c_k em todos os 128 modos — o Metrónomo mede QUANDO',
    diagonal)
  /* nas folhas: a rotação do modo É a folha */
  const naoNulos = []
  for (let k = 0; k < N; k++) if (c[k] !== 0) naoNulos.push(k)
  const raizes = []
  for (let r = 0; r < q; r++) if (mod(r * r - m * r - 1) === 0) raizes.push(r)
  ok('§MF2 a folha roda pela própria folha: nos modos das riscas, ω^k ∈ {σ, σ†}',
    naoNulos.every(k => raizes.includes(powm(w, k))))
}

/* §MF3 — a derivada discreta honesta: Δ ⟺ ω^k − 1 */
{
  const dx = x.map((_, n) => mod(x[(n + 1) % N] - x[n]))
  const cd = dft(dx)
  let honesta = true
  for (let k = 0; k < N; k++) {
    if (cd[k] !== c[k] * mod(powm(w, k) - 1) % q) honesta = false
  }
  ok('§MF3 a derivada discreta: Δx ⟺ c_k ↦ (ω^k−1)·c_k exato — o «ik» do contínuo é ω^k−1 no anel',
    honesta)
}

/* §MF4 — o Maestro como dobra espectral (a retração d→d/2) */
{
  const y = []
  for (let n = 0; n < N / 2; n++) y.push(x[2 * n])   /* π: subamostra — a dobra da torre */
  const cy = dft(y)
  const i2 = inv(2)
  let dobra = true
  for (let k = 0; k < N / 2; k++) {
    if (cy[k] !== i2 * mod(c[k] + c[k + N / 2]) % q) dobra = false
  }
  ok('§MF4 o Maestro é a DOBRA dos modos: retração d→d/2 dá ĉ_k = 2⁻¹(c_k + c_{k+N/2}) exato — T+T*',
    dobra)
}

/* §MF5 — o Maestro como seletor a_k ∈ {0,1} */
{
  const i2 = inv(2)
  const par = x.map((_, n) => i2 * mod(x[n] + x[(n + N / 2) % N]) % q)
  const imp = x.map((_, n) => i2 * mod(x[n] - x[(n + N / 2) % N] + q) % q)
  const cp = dft(par), ci = dft(imp)
  let seleciona = true, soma = true, idem = true
  for (let k = 0; k < N; k++) {
    const aPar = k % 2 === 0 ? 1 : 0
    if (cp[k] !== aPar * c[k] % q) seleciona = false
    if (ci[k] !== (1 - aPar) * c[k] % q) seleciona = false
    if (mod(cp[k] + ci[k]) !== c[k]) soma = false
  }
  const parPar = par.map((_, n) => i2 * mod(par[n] + par[(n + N / 2) % N]) % q)
  for (let n = 0; n < N; n++) if (parPar[n] !== par[n]) idem = false
  ok('§MF5 o Maestro SELECIONA: o projetor 2⁻¹(id+T^{N/2}) age como c_k ↦ a_k c_k, a_k ∈ {0,1} (pares)',
    seleciona)
  ok('§MF5 a partição fecha: a_par + a_ímpar = 1 em todo modo, e o projetor é idempotente',
    soma && idem)
}

/* §MF6 — a volta exata, com R_total = 0 */
{
  const { idft } = require('../lib/universal.js')
  const volta = idft(c, ANEL, w)
  const R = U.residuoTotal([['orbita', x.join(',')]], [['orbita', volta.join(',')]])
  console.log(`volta: R=(${R.Rend},${R.RE},${R.RF1},${R.RF2},${R.RD})`)
  ok('§MF6 a inversa devolve a órbita EXATA (128/128) com R_total = 0 — nada se perde na representação',
    volta.every((s, n) => s === x[n]) && U.retain(R))
}

/* §MF7 — o palco grande: 65537, N=8192 — as folhas continuam a ser o espectro */
{
  const P = 65537
  const G = anel(P)
  const A2 = Am(2).map(G.mod)
  const passo = u => [G.mod(A2[0] * u[0] + A2[1] * u[1]), G.mod(A2[2] * u[0] + A2[3] * u[1])]
  const NN = 8192
  const W = G.powm(3, 8)                             /* ord = 65536/8 = 8192 */
  let u = [1, 0]
  const X = []
  for (let n = 0; n < NN; n++) { X.push(u[0]); u = passo(u) }
  ok('§MF7 a órbita grande fecha: A₂^8192 v = v exato mod 65537', u[0] === X[0] && u[1] === 0)
  /* as folhas mod P (σ=4081 do toro auditado) e os seus logs em ω */
  const raizes = []
  for (let r = 0; r < P; r++) if (G.mod(r * r - 2 * r - 1) === 0) raizes.push(r)
  const risca = k => {
    let s = 0
    const wk = G.powm(W, NN - (k % NN))
    let f = 1
    for (let n = 0; n < NN; n++) { s = (s + X[n] * f) % P; f = f * wk % P }
    return s
  }
  const logs = raizes.map(r => {
    let f = 1
    for (let k = 0; k < NN; k++) { if (f === r) return k; f = f * W % P }
    return -1
  })
  let SEED = 20260814
  const lcg = () => { SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff; return SEED >>> 4 }
  let controlos = true
  for (let t = 0; t < 8; t++) {
    let k = lcg() % NN
    while (logs.includes(k)) k = (k + 1) % NN
    if (risca(k) !== 0) controlos = false
  }
  console.log(`folhas mod 65537: {${raizes.join(', ')}} · logs em ω: {${logs.join(', ')}} · c nas folhas: {${logs.map(risca).join(', ')}}`)
  ok('§MF7 no anel grande as folhas SÃO o espectro: c ≠ 0 nas duas riscas das folhas, c = 0 nos 8 controlos',
    raizes.length === 2 && logs.every(k => k >= 0) &&
    logs.every(k => risca(k) !== 0) && controlos)
}

console.log('')
if (!falhas) {
  console.log('  FOURIER NÃO FUNDAMENTA O METRÓNOMO: REPRESENTA-O. Os c_k medidos')
  console.log('  da órbita são DUAS FOLHAS (ω^k = σ, σ†; σσ† = −1); o Metrónomo é')
  console.log('  diagonal (mede QUANDO: cada modo roda pela própria frequência, a')
  console.log('  folha pela própria folha; Δ ⟺ ω^k−1); o Maestro é derivação')
  console.log('  espectral (decide QUAIS: a dobra c_k+c_{k+N/2} e o seletor')
  console.log('  a_k ∈ {0,1}); Parseval fecha e a volta é exata com R_total = 0.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
