/* tests/zeta_universal.js — a zeta dinâmica do Corpo Universal, DERIVADA
 * (ordem da mesa, eval 14/08: gerente «a zeta passa a ser uma função
 * GERADA pelo operador, não um axioma»; diretor autoriza como primitiva
 * derivada — «não force a conexão com Riemann; deixe que a medida diga»).
 *
 * A cadeia: Leis → operador (a batuta A_m) → órbitas → períodos → zeta.
 * Nada se importa: ζ_T(u) = ∏_d (1−u^d)^{−N_d} constrói-se do CENSO das
 * órbitas primitivas, e valida-se por DOIS CAMINHOS inteiros contra a
 * contagem de pontos fixos F_k = #ker(A^k−I):
 *     Σ_{d|k} d·N_d = F_k   (o censo e o núcleo têm de concordar)
 * e, na série, n·z_n = Σ_j F_j·z_{n−j} (a derivada logarítmica) contra a
 * expansão do produto — BigInt, sem um único racional intermédio.
 *
 * O palco pequeno é o anel do primo de Fermat F_3 = 257 = 2^8+1 (o irmão
 * de 8 bits do 65537 = 2^16+1 da casa): (ℤ/257)² tem 66049 pontos — o
 * censo é COMPLETO, não amostrado. O palco grande (65537) entra pelo gap
 * espectral, ligando ao toro já auditado (toro_histerese.js).
 *
 * §Z0  o censo completo: TODAS as órbitas de A_m em (ℤ/257)², m=1..3;
 *      Σ_d d·N_d = 257² exato; N_1 = 1 — a massa fica no centro (a
 *      única órbita de período 1 é a origem)
 * §Z1  os períodos são primitivos: A^d x = x e A^k x ≠ x (0<k<d), num
 *      representante de CADA classe de período
 * §Z2  dois caminhos: F_k pelo núcleo (posto de A^k−I) == Σ_{d|k} d·N_d
 *      pelo censo, para k = 1..ord(A) — sem exceção
 * §Z3  a histerese filtra: a volta da órbita (T^d x = x) dá
 *      R_total = (0,0,0,0,0) → RETAIN em toda classe; a caminhada
 *      truncada dá REOPEN — Ó_admissível medido, não declarado
 * §Z4  a zeta reconstruída: os coeficientes de ∏_d (1−u^d)^{−N_d}
 *      (produto, censo) == os da recorrência n·z_n = Σ F_j z_{n−j}
 *      (núcleo) — inteiros exatos até n=12, com a divisibilidade por n
 *      a fechar (podia não fechar; fecha)
 * §Z5  o palco grande: mod 65537, m=2, o espectro tem GAP — F_k = 1
 *      para k=1..64 (o centro está só) e o primeiro período é
 *      ord(σ)=8192 com A^8192 = I exato
 *
 *   node tests/zeta_universal.js
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

const { mul, igual, I, Am } = mat2

function fabrica (q) {
  const mod = x => ((x % q) + q) % q
  const mmod = X => X.map(mod)
  const mulp = (X, Y) => mmod(mul(X, Y))
  const powp = (X, e) => {
    let R = [1, 0, 0, 1], B = X.slice()
    while (e > 0) {
      if (e & 1) R = mulp(R, B)
      B = mulp(B, B)
      e >>= 1
    }
    return R
  }
  const aplica = (M, v) => [mod(M[0] * v[0] + M[1] * v[1]), mod(M[2] * v[0] + M[3] * v[1])]
  return { mod, mmod, mulp, powp, aplica }
}

/* ── §Z0/§Z1/§Z2 — o censo completo em (ℤ/257)² ──────────────────────────── */
const q = 257
const { mod, mmod, mulp, powp, aplica } = fabrica(q)

function censo (m) {
  const A = mmod(Am(m))
  const visto = new Uint8Array(q * q)
  const N = new Map()                 /* período primitivo → nº de órbitas */
  const reps = new Map()              /* período → um representante */
  for (let x = 0; x < q; x++) {
    for (let y = 0; y < q; y++) {
      const i0 = x * q + y
      if (visto[i0]) continue
      let v = [x, y], d = 0
      do {
        visto[v[0] * q + v[1]] = 1
        v = aplica(A, v)
        d++
      } while (v[0] !== x || v[1] !== y)
      N.set(d, (N.get(d) || 0) + 1)
      if (!reps.has(d)) reps.set(d, [x, y])
    }
  }
  return { A, N, reps }
}

const censos = new Map()
for (let m = 1; m <= 3; m++) censos.set(m, censo(m))

{
  let soma = true, centro = true
  for (const [m, { N }] of censos) {
    let tot = 0
    for (const [d, n] of N) tot += d * n
    if (tot !== q * q) soma = false
    if (N.get(1) !== 1) centro = false
    const per = [...N.keys()].sort((a, b) => a - b)
    console.log(`m=${m}: períodos {${per.join(', ')}} · órbitas ${[...per.map(d => N.get(d))].join(', ')}`)
  }
  ok('§Z0 o censo é completo: Σ d·N_d = 257² exato, m=1..3 — toda a dinâmica contada', soma)
  ok('§Z0 a massa fica no centro: N_1 = 1 (a única órbita de período 1 é a origem), m=1..3',
    centro)
}

/* §Z1 — períodos primitivos, num representante de cada classe */
{
  let primitivos = true
  for (const [, { A, reps }] of censos) {
    for (const [d, rep] of reps) {
      let v = rep.slice()
      for (let k = 1; k < d; k++) {
        v = aplica(A, v)
        if (v[0] === rep[0] && v[1] === rep[1]) primitivos = false
      }
      v = aplica(A, v)
      if (v[0] !== rep[0] || v[1] !== rep[1]) primitivos = false
    }
  }
  ok('§Z1 os períodos são PRIMITIVOS: A^d x = x e A^k x ≠ x (0<k<d), em cada classe',
    primitivos)
}

/* §Z2 — dois caminhos: núcleo contra censo */
function Fk (Apow) {
  /* #ker(A^k − I) sobre F_q: posto da 2×2 */
  const M = [mod(Apow[0] - 1), mod(Apow[1]), mod(Apow[2]), mod(Apow[3] - 1)]
  const zero = M.every(v => v === 0)
  if (zero) return q * q
  const d = mod(M[0] * M[3] - M[1] * M[2])
  return d === 0 ? q : 1
}
{
  let concordam = true
  for (const [, { A, N }] of censos) {
    const T = Math.max(...N.keys())
    let Ak = [1, 0, 0, 1]
    for (let k = 1; k <= T; k++) {
      Ak = mulp(Ak, A)
      let doCenso = 0
      for (const [d, n] of N) if (k % d === 0) doCenso += d * n
      if (Fk(Ak) !== doCenso) concordam = false
    }
  }
  ok('§Z2 dois caminhos concordam: F_k (núcleo de A^k−I) == Σ_{d|k} d·N_d (censo), k=1..ord(A), m=1..3',
    concordam)
}

/* §Z3 — a histerese filtra: a volta é RETAIN, a truncada é REOPEN */
{
  let retain = true, reopen = true
  for (const [, { A, reps }] of censos) {
    for (const [d, rep] of reps) {
      let v = rep.slice()
      for (let k = 0; k < d; k++) v = aplica(A, v)
      const R = U.residuoTotal([['x', rep.join(',')]], [['x', v.join(',')]])
      if (!U.retain(R)) retain = false
      if (d > 1) {
        let w = rep.slice()
        for (let k = 0; k < d - 1; k++) w = aplica(A, w)
        const R2 = U.residuoTotal([['x', rep.join(',')]], [['x', w.join(',')]])
        if (U.retain(R2)) reopen = false
      }
    }
  }
  ok('§Z3 T^d x = x ⟹ R_total = (0,0,0,0,0) → RETAIN, em TODA classe de período — a órbita é admissível',
    retain)
  ok('§Z3 a caminhada truncada (d−1 passos) dá REOPEN — a histerese filtra as voltas ilegítimas',
    reopen)
}

/* §Z4 — a zeta reconstruída por dois caminhos inteiros (BigInt) */
{
  const K = 12
  let iguais = true, divisivel = true
  for (const [m, { N }] of censos) {
    /* caminho do CENSO: ∏_d (1−u^d)^{−N_d} até u^K */
    let prod = new Array(K + 1).fill(0n)
    prod[0] = 1n
    for (const [d, n] of N) {
      if (d > K) continue
      for (let vez = 0; vez < n; vez++) {
        /* multiplicar por (1−u^d)^{−1} = Σ u^{jd} */
        const novo = new Array(K + 1).fill(0n)
        for (let i = 0; i <= K; i++) {
          if (prod[i] === 0n) continue
          for (let j = 0; i + j * d <= K; j++) novo[i + j * d] += prod[i]
        }
        prod = novo
      }
    }
    /* caminho do NÚCLEO: n·z_n = Σ_{j=1..n} F_j z_{n−j} */
    const Amat = mmod(Am(m))
    const F = [0n]
    let Ak = [1, 0, 0, 1]
    for (let k = 1; k <= K; k++) {
      Ak = mulp(Ak, Amat)
      F.push(BigInt(Fk(Ak)))
    }
    const z = [1n]
    for (let n = 1; n <= K; n++) {
      let s = 0n
      for (let j = 1; j <= n; j++) s += F[j] * z[n - j]
      if (s % BigInt(n) !== 0n) divisivel = false
      z.push(s / BigInt(n))
    }
    for (let n = 0; n <= K; n++) if (z[n] !== prod[n]) iguais = false
  }
  ok('§Z4 a divisibilidade fecha: Σ F_j z_{n−j} ≡ 0 (mod n) em todo n ≤ 12 — a série é inteira (podia falhar)',
    divisivel)
  ok('§Z4 a zeta é UMA: os coeficientes do produto (censo) == os da recorrência (núcleo), inteiros exatos',
    iguais)
}

/* §Z5 — o palco grande: o gap espectral mod 65537 (liga ao toro auditado) */
{
  const P = 65537
  const g = fabrica(P)
  const A = g.mmod(Am(2))
  let gap = true
  let Ak = [1, 0, 0, 1]
  for (let k = 1; k <= 64; k++) {
    Ak = g.mulp(Ak, A)
    const M = [g.mod(Ak[0] - 1), Ak[1], Ak[2], g.mod(Ak[3] - 1)]
    if (g.mod(M[0] * M[3] - M[1] * M[2]) === 0) gap = false
  }
  ok('§Z5 o gap espectral no anel grande: F_k = 1 para k=1..64 (mod 65537, m=2) — o centro está só',
    gap)
  ok('§Z5 o primeiro período é o da folha: A_2^8192 = I exato mod 65537 (a órbita fecha onde o toro disse)',
    igual(g.powp(A, 8192), I))
}

console.log('')
if (!falhas) {
  console.log('  A ZETA DINÂMICA É DERIVADA, NÃO IMPORTADA: Leis → operador →')
  console.log('  órbitas → períodos → ζ_T(u) = ∏(1−u^d)^{−N_d}. O censo é completo,')
  console.log('  o núcleo concorda com o censo, a histerese filtra as voltas')
  console.log('  (RETAIN/REOPEN), e a série fecha inteira pelos dois caminhos.')
  console.log('  A pergunta sobre a transformação espectral u = F(s) fica CONCRETA')
  console.log('  e em aberto — o nome clássico só entra se a medida o trouxer.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
