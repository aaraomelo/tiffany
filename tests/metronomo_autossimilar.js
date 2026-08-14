/* tests/metronomo_autossimilar.js — a renormalização do espectro do
 * Metrónomo (ordem da mesa, eval 14/08: a intuição do coordenador
 * «fractal / coração / multiespectral / autossimilar» submetida ao
 * teste de renormalização — «a prosa de escala só entra se a matriz
 * fechar»).
 *
 * A lei de transição R não se inventa: É A DOBRA (a mesma do Maestro,
 * metronomo_fourier §MF4; a dobra temporal de Dirac x↦x²). Subamostrar
 * a órbita por 2 eleva as folhas ao quadrado, e o corpo renormaliza
 * pela lei inteira dos traços
 *     t_{j+1} = t_j² − 2·d_j,   d_{j+1} = d_j²
 * (o mapa de duplicação t ↦ t²−2 depois do primeiro passo). O que a
 * medida dá, na órbita real já auditada (m=2, p=65537, N=8192=2¹³):
 *
 *   MULTIESPECTRAL: a cascata tem 13 níveis, cada um com o seu
 *   espectro; AUTOSSIMILAR: todo nível satisfaz a MESMA forma
 *   y_{n+1} = t_j·y_n − d_j·y_{n−1}, e o ângulo da risca segue o mapa
 *   de duplicação θ ↦ 2θ (razão de escala ρ=2 exata); PONTO FIXO:
 *   R(2)=2 e a cascata atinge-o em j=13 e FICA; e O FUNDO DA CASCATA
 *   É O CATÁLOGO: as três últimas dobras são o bit i (t=0, ordem 4,
 *   Lei 5), o espelho (t=−2, ordem 2) e a unidade (t=+2, Lei 0).
 *
 *   O núcleo espectral invariável (o candidato técnico ao nome do
 *   coordenador) é o que sobrevive a TODAS as dobras em TODOS os
 *   andares de Fermat: a cauda universal i → −1 → +1.
 *
 * §A0  a lei de renormalização, por DOIS caminhos: t/d de A^{2^j} por
 *      quadraturas de matriz == a recorrência t²−2d — global (BigInt,
 *      j=0..8) e no anel (j=0..15); e o global REDUZ exato ao anel
 * §A1  a semelhança: em cada nível j=0..13 a órbita subamostrada
 *      satisfaz a MESMA forma y_{n+1}=t_j·y_n−d_j·y_{n−1} (mod p)
 * §A2  a razão de escala é 2: a risca do nível j está em k₀ mod N_j
 *      (θ_j = frac(2^j·θ₀) — o mapa de duplicação do ângulo), com
 *      controlos nulos
 * §A3  o fundo da cascata é o catálogo: t₁₁=0 com σ^{2¹¹} de ordem 4
 *      (o rotor/bit i), t₁₂=−2 com ordem 2 (o espelho), t₁₃=+2 com
 *      σ^{2¹³}=1 (a unidade)
 * §A4  o ponto fixo de renormalização: R(2)=2, e a cascata FICA lá
 *      (j=13,14,15) — R(S*)=S*
 * §A5  a mesma cascata em toda a escada de Fermat: q=17 (profundidade
 *      4), q=257 (7), q=65537 (13) — a cauda (0, −2, +2) é UNIVERSAL;
 *      o que muda de andar para andar é só a profundidade
 *
 *   node tests/metronomo_autossimilar.js
 */
'use strict'
const { mat2 } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const { mul, Am } = mat2

/* §A0 — a lei de renormalização, por dois caminhos */
{
  /* global, BigInt: matrizes 2×2 por quadraturas vs recorrência */
  const mulB = (X, Y) => [X[0] * Y[0] + X[1] * Y[2], X[0] * Y[1] + X[1] * Y[3],
    X[2] * Y[0] + X[3] * Y[2], X[2] * Y[1] + X[3] * Y[3]]
  let M = [2n, 1n, 1n, 0n]
  let t = 2n, d = -1n
  let doisCaminhos = true
  const globais = [t]
  for (let j = 1; j <= 8; j++) {
    M = mulB(M, M)                                   /* A^{2^j} */
    const nt = t * t - 2n * d
    d = d * d
    t = nt
    globais.push(t)
    if (M[0] + M[3] !== t) doisCaminhos = false      /* traço da matriz */
    if (M[0] * M[3] - M[1] * M[2] !== d) doisCaminhos = false
  }
  ok('§A0 a lei t_{j+1}=t_j²−2d_j, d_{j+1}=d_j² == traço/det de A^{2^j} por quadraturas — global exato (BigInt, j=0..8)',
    doisCaminhos)
  /* no anel, e a redução global→anel */
  const P = 65537n
  let tp = 2, dp = -1
  let reduz = true
  for (let j = 0; j <= 8; j++) {
    const g = Number(((globais[j] % P) + P) % P)
    if (g !== ((tp % 65537) + 65537) % 65537) reduz = false
    const nt = tp * tp - 2 * dp
    dp = (dp * dp) % 65537
    tp = ((nt % 65537) + 65537) % 65537
  }
  ok('§A0 o global reduz EXATO ao anel: t_j mod 65537 == a cascata mod p, j=0..8 (1331714 → 20974, …)',
    reduz)
}

/* ── a cascata no anel, completa ─────────────────────────────────────────── */
const P = 65537
const mod = x => ((x % P) + P) % P
const powm = (b, e) => {
  let r = 1; b = mod(b)
  while (e > 0) { if (e & 1) r = r * b % P; b = b * b % P; e >>= 1 }
  return r
}
const cascata = [{ t: 2, d: P - 1 }]
for (let j = 1; j <= 15; j++) {
  const { t, d } = cascata[j - 1]
  const dd = d === P - 1 ? -1 : d
  cascata.push({ t: mod(t * t - 2 * dd), d: mod(d * d) })
}
console.log('cascata mod p: t = ' + cascata.map(c => c.t === P - 2 ? '-2' : c.t === P - 1 ? '-1' : c.t).slice(0, 14).join(', '))

/* a órbita real (a mesma dos medidores anteriores) */
const A = [2, 1, 1, 0]
let v = [1, 0]
const N = 8192
const x = []
for (let n = 0; n < N; n++) { x.push(v[0]); v = [mod(2 * v[0] + v[1]), v[0]] }

/* §A1 — a semelhança: todo nível satisfaz a mesma forma */
{
  let mesma = true
  for (let j = 0; j <= 13; j++) {
    const passo = 1 << j
    const { t, d } = cascata[j]
    const dd = d === P - 1 ? P - 1 : d               /* −d_j mod p */
    for (let n = 1; n <= 40; n++) {
      const y0 = x[((n - 1) * passo) % N]
      const y1 = x[(n * passo) % N]
      const y2 = x[((n + 1) * passo) % N]
      /* y2 = t·y1 − d·y0 */
      if (y2 !== mod(t * y1 - (d === P - 1 ? -1 : d) * y0)) mesma = false
    }
  }
  ok('§A1 AUTOSSIMILAR: a órbita subamostrada satisfaz a MESMA forma y₂ = t_j·y₁ − d_j·y₀ em TODOS os 14 níveis',
    mesma)
}

/* §A2 — a razão de escala é 2: o ângulo da risca duplica */
{
  const sigma = 4081                                 /* a folha (toro auditado) */
  const w = powm(3, 8)                               /* ord 8192 */
  /* k₀: o log da folha */
  let k0 = -1
  {
    let f = 1
    for (let k = 0; k < N; k++) { if (f === sigma) { k0 = k; break } f = f * w % P }
  }
  let riscas = true, controlos = true
  let SEED = 20260814
  const lcg = () => { SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff; return SEED >>> 4 }
  for (const j of [1, 2, 3]) {
    const passo = 1 << j
    const Nj = N / passo
    const wj = powm(w, passo)                        /* ord N_j */
    const soma = k => {
      let s = 0
      const wk = powm(wj, Nj - (k % Nj))
      let f = 1
      for (let n = 0; n < Nj; n++) { s = (s + x[(n * passo) % N] * f) % P; f = f * wk % P }
      return s
    }
    const kj = k0 % Nj                               /* θ_j = frac(2^j θ₀) */
    const kDual = (Nj - kj) % Nj                     /* σ†^{2^j} = σ^{−2^j} (j≥1) */
    if (soma(kj) === 0) riscas = false
    for (let c = 0; c < 4; c++) {
      let k = lcg() % Nj
      while (k === kj || k === kDual) k = (k + 1) % Nj
      if (soma(k) !== 0) controlos = false
    }
  }
  ok('§A2 a razão de escala é 2: a risca do nível j está em k₀ mod N_j — o ângulo segue o mapa de duplicação θ↦2θ',
    k0 >= 0 && riscas)
  ok('§A2 os controlos fora das riscas são nulos nos níveis 1..3 — o espectro de cada nível é atómico',
    controlos)
}

/* §A3 — o fundo da cascata é o catálogo */
{
  const sigma = 4081
  const s11 = powm(sigma, 1 << 11)
  const s12 = powm(sigma, 1 << 12)
  const s13 = powm(sigma, 1 << 13)
  ok('§A3 t₁₁ = 0 e σ^{2¹¹} tem ORDEM 4 — o rotor, o bit i (Lei 5) aparece na 11.ª dobra',
    cascata[11].t === 0 && powm(s11, 2) === P - 1 && powm(s11, 4) === 1 && s11 !== 1 && s11 !== P - 1)
  ok('§A3 t₁₂ = −2 e σ^{2¹²} = −1 — o espelho (ordem 2) na 12.ª dobra',
    cascata[12].t === P - 2 && s12 === P - 1)
  ok('§A3 t₁₃ = +2 e σ^{2¹³} = 1 — a unidade: o fundo é o par da Lei 0',
    cascata[13].t === 2 && s13 === 1)
}

/* §A4 — o ponto fixo de renormalização */
{
  const R = t => mod(t * t - 2)
  ok('§A4 R(S*) = S*: R(2)=2 é ponto fixo, a cascata atinge-o em j=13 e FICA (j=14,15)',
    R(2) === 2 && cascata[13].t === 2 && cascata[14].t === 2 && cascata[15].t === 2)
  ok('§A4 o gume: R(0)=−2 e R(−2)=2 — o rotor NÃO é fixo (cai no espelho) e o espelho cai na unidade: a cauda anda',
    R(0) === P - 2 && R(P - 2) === 2)
}

/* §A5 — a mesma cascata em toda a escada de Fermat */
{
  let cauda = true
  const profundidades = []
  for (const q of [17, 257, 65537]) {
    const modq = xx => ((xx % q) + q) % q
    /* j=1: t²−2d com d=−1 → t²+2; daí em diante d=1 e t ↦ t²−2 */
    const ts = [2, modq(2 * 2 + 2)]
    while (ts[ts.length - 1] !== 2 && ts.length <= 20) {
      const t = ts[ts.length - 1]
      ts.push(modq(t * t - 2))
    }
    const prof = ts.length - 1
    profundidades.push(prof)
    /* a cauda universal: (…, 0, −2, +2) */
    if (!(ts[prof] === 2 && ts[prof - 1] === q - 2 && ts[prof - 2] === 0)) cauda = false
  }
  console.log(`profundidades da cascata na escada de Fermat: ${profundidades.join(', ')}`)
  ok('§A5 a cauda (0, −2, +2) é UNIVERSAL nos três andares de Fermat — a cascata é a mesma, só muda a profundidade',
    cauda)
  ok('§A5 MULTIESPECTRAL: as profundidades medidas são 4, 7, 13 — a escada aprofunda a cascata (2²+…: q−1 = 2^{2^j})',
    profundidades.join() === '4,7,13')
}

console.log('')
if (!falhas) {
  console.log('  A RENORMALIZAÇÃO FECHOU EM INTEIROS: a lei de transição é a DOBRA')
  console.log('  (t ↦ t²−2d, as folhas ao quadrado), todo nível tem a MESMA forma')
  console.log('  (autossimilar), o ângulo da risca segue o mapa de duplicação (ρ=2),')
  console.log('  e há ponto fixo R(S*)=S* atingido e mantido. O espectro é')
  console.log('  MULTIESPECTRAL (13 níveis em 65537; 7 em 257; 4 em 17) com a cauda')
  console.log('  universal i → espelho → unidade: o núcleo que sobrevive a todas as')
  console.log('  dobras em todos os andares é o catálogo mínimo (Lei 5 → Lei 0).')
  console.log('  Fractal clássico não se afirma (sem métrica de Hausdorff);')
  console.log('  a fractalidade OPERACIONAL — ponto fixo de renormalização — está')
  console.log('  medida. O nome do coordenador tem agora candidato técnico: o')
  console.log('  núcleo espectral invariável da torre.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
