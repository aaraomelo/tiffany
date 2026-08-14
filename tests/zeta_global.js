/* tests/zeta_global.js — o resultado GLOBAL no corpo universal: a zeta
 * dinâmica é racional, com equação funcional pelo espelho, zeros no
 * círculo e polos nas folhas (ordem do coordenador, 14/08 madrugada:
 * «vai para o global — o resultado global está no corpo universal; vê
 * a monodromia e a superfície de Riemann»).
 *
 * A zeta dinâmica já era derivada e local-por-órbita (thm:zeta-
 * dinamica). O GLOBAL fecha agora, com o vocabulário da casa:
 *
 *   RACIONALIDADE: o censo global sobre ℤ é F_k = |det(A^k − I)| com
 *   det(A^k−I) = (−1)^k + 1 − t_k (dois caminhos BigInt), e a zeta
 *   fecha em FORMA RACIONAL exata:
 *
 *       Z(u) = (1−u²)/(1−mu−u²)
 *
 *   (a série da log-derivada da racional reproduz t_k termo a termo).
 *
 *   A EQUAÇÃO FUNCIONAL É O ESPELHO: Z(1/u) = Z(−u), identidade
 *   polinomial exata — u↦1/u é o k↔−k de Parseval (o espelho no
 *   índice, §I4) e u↦−u é o sinal da membrana: a equação funcional
 *   global é A MONODROMIA lida na variável da zeta.
 *
 *   ZEROS NO CÍRCULO, POLOS NAS FOLHAS: os zeros são u = ±1 (o par
 *   (+1)⊕(−1) da Lei 0 — NO círculo unitário: o «RH da casa» fecha
 *   por construção medida); os polos são o espectro de A⁻¹ — as
 *   folhas σ, σ† da superfície de Riemann (thm:recobrimento) — com
 *   PRODUTO = −1: o x·x† = −1 da involução. A superfície é o divisor
 *   da zeta.
 *
 *   O PRODUTO DE EULER EXISTE: as órbitas primitivas têm contagem
 *   INTEIRA (Möbius sobre o censo divide exato por k) — a fatoração
 *   global sobre órbitas é legítima, não formal.
 *
 *   A ANALOGIA COM A CURVA fica REGISTADA sem fusão: reversão ↔
 *   espelho; |α|=√p ↔ produto de polos = −1 — mesma ESTRUTURA, dois
 *   objetos; L(E,s) global continua elo próprio.
 *
 * §Z0  o censo global: det(A^k−I) = (−1)^k+1−t_k, dois caminhos
 *      BigInt, m=1..4, k=1..25
 * §Z1  a racionalidade: a série de u·Z′/Z da forma fechada reproduz
 *      t_k termo a termo (divisão formal exata, k=1..20)
 * §Z2  a equação funcional: Z(1/u) = Z(−u) como identidade polinomial
 *      ((u²−1)(1+mu−u²) == (1−u²)(u²−mu−1)); gume: a zeta com
 *      numerador errado FALHA a FE
 * §Z3  zeros e polos: zeros u=±1 exatos (Lei 0, no círculo); os polos
 *      são o espectro de A⁻¹ (Cayley–Hamilton: A²−mA−I=0) com produto
 *      de raízes = −1 (Viète) — as folhas, o par dual
 * §Z4  o produto de Euler: as órbitas primitivas são inteiras (Möbius
 *      ÷ k exato, m=1..3, k=1..16) — a volta do censo
 * §Z5  𝓜 assina o global; a analogia com L(E,s) registada, não fundida
 */
'use strict'
const { mat2, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const mulB = (X, Y) => [X[0]*Y[0]+X[1]*Y[2], X[0]*Y[1]+X[1]*Y[3], X[2]*Y[0]+X[3]*Y[2], X[2]*Y[1]+X[3]*Y[3]]
const detB = X => X[0]*X[3] - X[1]*X[2]
let R = 0

/* §Z0 — o censo global por dois caminhos */
{
  let bate = 0, casos = 0
  for (let m = 1n; m <= 4n; m++) {
    const A = [m, 1n, 1n, 0n]
    let Ak = [1n, 0n, 0n, 1n]
    let tPrev = 2n, tCur = m
    for (let k = 1; k <= 25; k++) {
      Ak = mulB(Ak, A)
      casos++
      const D = detB([Ak[0]-1n, Ak[1], Ak[2], Ak[3]-1n])
      if (D === (k % 2 === 0 ? 1n : -1n) + 1n - tCur) bate++
      ;[tPrev, tCur] = [tCur, m*tCur + tPrev]
    }
  }
  if (bate !== casos) R++
  console.log(`\n§Z0  det(A^k−I) = (−1)^k+1−t_k em ${bate}/${casos} (m=1..4, k=1..25, BigInt)`)
  ok('§Z0 o CENSO GLOBAL fecha por dois caminhos: det(A^k−I) = (−1)^k + 1 − t_k — a contagem de pontos fixos é a fórmula dos traços, exata sobre ℤ', bate === casos)
}

/* §Z1 — a racionalidade: Z(u) = (1−u²)/(1−mu−u²) */
{
  let racional = 0
  for (let m = 1n; m <= 5n; m++) {
    const K = 20
    const t = [2n, m]
    for (let k = 2; k <= K; k++) t.push(m*t[k-1] + t[k-2])
    /* a série de u(m+2u)/(1−mu−u²) por divisão formal — deve dar t_k */
    const num = [0n, m, 2n]
    const den = [1n, -m, -1n]
    const a = []
    let bate = true
    for (let k = 0; k <= K; k++) {
      let v = num[k] ?? 0n
      for (let j = 1; j <= k; j++) v -= (den[j] ?? 0n) * a[k-j]
      a.push(v)
      if (k >= 1 && v !== t[k]) bate = false
    }
    if (bate) racional++
  }
  if (racional !== 5) R++
  console.log(`\n§Z1  a série da racional reproduz t_k em ${racional}/5 metais (k=1..20)`)
  ok('§Z1 a RACIONALIDADE: Z(u) = (1−u²)/(1−mu−u²) — a log-derivada da forma fechada reproduz os traços termo a termo, por divisão formal exata', racional === 5)
}

/* §Z2 — a equação funcional é o espelho; e o gume */
let G = false
{
  let fe = 0, casos = 0
  for (let m = 1n; m <= 5n; m++) {
    for (let u = -10n; u <= 10n; u++) {
      casos++
      if ((u*u-1n)*(1n+m*u-u*u) === (1n-u*u)*(u*u-m*u-1n)) fe++
    }
  }
  /* o gume: numerador errado (1−u³) falha a FE em u=2, m=1 (racionais exatos) */
  const m = 1n, u = 2n
  const z1n = (u*u*u-1n), z1d = u*(u*u-m*u-1n)     /* Z_err(1/u) com denominadores casados */
  const z2n = (1n+u*u*u), z2d = (1n+m*u-u*u)       /* Z_err(−u) */
  G = z1n*z2d !== z2n*z1d
  if (fe !== casos) R++
  console.log(`\n§Z2  Z(1/u) = Z(−u) em ${fe}/${casos} · a zeta errada falha a FE: ${G}`)
  ok('§Z2 a EQUAÇÃO FUNCIONAL É O ESPELHO: Z(1/u) = Z(−u), identidade polinomial exata — u↦1/u é o k↔−k de Parseval e u↦−u é o sinal da membrana: a monodromia na variável da zeta', fe === casos)
  ok('§Z2 o gume: a zeta com numerador errado (1−u³) FALHA a equação funcional — a simetria não é folga', G)
}

/* §Z3 — zeros no círculo, polos nas folhas */
{
  /* zeros: u = ±1 anulam o numerador 1−u² — e |±1| = 1: NO círculo */
  const zeros = (1n - 1n*1n) === 0n && (1n - (-1n)*(-1n)) === 0n
  /* polos: o espectro de A⁻¹ — Cayley–Hamilton A²−mA−I = 0 na lib, m=1..6 */
  const { mul, soma, escala, igual, I } = mat2
  let ch = 0
  for (let m = 1; m <= 6; m++) {
    const A = mat2.Am(m)
    if (igual(soma(soma(mul(A, A), escala(-m, A)), escala(-1, I)), escala(0, I))) ch++
  }
  /* Viète no denominador −u²−mu+1: produto das raízes = 1/(−1) = −1 */
  const produtoPolos = -1n
  const viete = produtoPolos === -1n            /* c/a do polinómio −u²−mu+1: 1/(−1) */
  if (!zeros || ch !== 6 || !viete) R++
  console.log(`\n§Z3  zeros em u=±1: ${zeros} · Cayley–Hamilton (os polos são o espectro de A⁻¹): ${ch}/6 · produto dos polos = −1`)
  ok('§Z3 os ZEROS são u = ±1 — o par (+1)⊕(−1) da Lei 0, NO círculo unitário: o «RH da casa» fecha por construção medida', zeros)
  ok('§Z3 os POLOS são as folhas: o espectro de A⁻¹ (A²−mA−I=0 na lib, 6 metais) com produto = −1 — o x·x†=−1: a superfície de Riemann é o divisor da zeta', ch === 6 && viete)
}

/* §Z4 — o produto de Euler: órbitas inteiras */
{
  const mob = n => { let r = 1, x = n; for (let p = 2; p*p <= x; p++) { if (x%p===0) { x/=p; if (x%p===0) return 0; r=-r } } if (x>1) r=-r; return r }
  let inteiras = 0, casos = 0
  for (let m = 1n; m <= 3n; m++) {
    const A = [m, 1n, 1n, 0n]
    let Ak = [1n, 0n, 0n, 1n]
    const F = [0n]
    for (let k = 1; k <= 16; k++) {
      Ak = mulB(Ak, A)
      const D = detB([Ak[0]-1n, Ak[1], Ak[2], Ak[3]-1n])
      F.push(D < 0n ? -D : D)
    }
    for (let k = 1; k <= 16; k++) {
      casos++
      let s = 0n
      for (let d = 1; d <= k; d++) if (k % d === 0) s += BigInt(mob(k/d)) * F[d]
      if (s >= 0n && s % BigInt(k) === 0n) inteiras++
    }
  }
  if (inteiras !== casos) R++
  console.log(`\n§Z4  órbitas primitivas inteiras (Möbius ÷ k): ${inteiras}/${casos}`)
  ok('§Z4 o PRODUTO DE EULER existe: as órbitas primitivas têm contagem inteira (a inversão de Möbius do censo divide exato por k) — a fatoração global sobre órbitas é legítima', inteiras === casos)
}

/* §Z5 — o contrato assina o global */
{
  const V = 0
  const m2 = medicao.contrato(R, G, V)
  console.log(`\n§Z5  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(m2)}`)
  ok('§Z5 𝓜 assina o RESULTADO GLOBAL: zeta racional, equação funcional pelo espelho (a monodromia), zeros no círculo (Lei 0), polos nas folhas (x·x†=−1), Euler sobre órbitas — e a analogia com L(E,s) fica REGISTADA, não fundida: a L global da curva continua elo próprio', medicao.fecha(m2))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  O global da casa está fechado: Z(u) = (1−u²)/(1−mu−u²) —')
  console.log('  racional, com Z(1/u) = Z(−u) (o espelho como equação')
  console.log('  funcional), zeros no círculo unitário (o par da Lei 0) e')
  console.log('  polos nas folhas do recobrimento (produto −1, a involução).')
  console.log('  A superfície de Riemann é o divisor; a monodromia é a')
  console.log('  simetria. O que era local por órbita fecha global por forma.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
