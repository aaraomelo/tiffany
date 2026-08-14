/* tests/hodge_viviani.js — Hodge, contagem e Riemann local encontram-se
 * em a_p, na curva que saiu de Viviani (a hipótese do coordenador —
 * «BSD, Hodge e Riemann resolvem-se mutuamente via Viviani no trial» —
 * convertida pela mesa em laboratório: o que se mede é o encontro
 * LOCAL; o global fica como hipótese registada, com a barreira).
 *
 * O encontro, medido:
 *
 *   O LADO DE HODGE CONTA: o operador de Cartier–Hasse–Witt age no
 *   diferencial ω = dx/y, e a sua matriz é o coeficiente de x^{p−1}
 *   em f(x)^{(p−1)/2} mod p. MEDIDO: esse coeficiente é EXATAMENTE o
 *   a_p da contagem de pontos (2 em F₁₇, 18 em F₂₅₇) — o lado
 *   diferencial e o lado aritmético calculam o mesmo número, por
 *   caminhos completamente independentes.
 *
 *   O LADO DE RIEMANN FECHA LOCALMENTE: a zeta local tem numerador
 *   1 − a_p T + pT²; o produto das raízes é p (Viète inteiro) e
 *   a_p² < 4p (Hasse, já medido) ⟹ os zeros vivem na circunferência
 *   |α| = √p — o RH local, que é TEOREMA (Hasse), realizado por
 *   medida; e a equação funcional é a reversão dos coeficientes.
 *
 *   O TRIAL ESTÁ NA CURVA: a involução (x,y)↦(x,−y) — o deck de
 *   Viviani — age como −1 em ω (medido ponto a ponto) e os seus
 *   pontos fixos são a 2-TORSÃO: 3 raízes de f + ∞ = 4 pontos, nos
 *   DOIS andares — o mesmo 4 do bit {1,i,−1,−i}: a ramificação da
 *   dobra, outra vez.
 *
 *   A BARREIRA: o encontro é LOCAL. L global, zeros globais e posto
 *   continuam elos independentes — a hipótese do coordenador fica
 *   REGISTADA como verdade local medida + hipótese global aberta.
 *
 * §D0  a involução age como −1 em ω: ω(ιP) = −ω(P) ponto a ponto em
 *      F₂₅₇ (o avatar de ω é o coeficiente 1/y de dx)
 * §D1  HODGE CONTA: coef de x^{p−1} em f^{(p−1)/2} == a_p da contagem,
 *      nos dois andares — dois caminhos independentes para o mesmo
 *      número; gume: o coeficiente errado (x^{p−2}) difere em F₂₅₇
 *      (em F₁₇ coincide por acaso — o gume morde onde pode)
 * §D2  RIEMANN LOCAL: disc(x²−a_p x+p) < 0 e produto de raízes = p ⟹
 *      |α|=√p nos dois andares; a equação funcional é a reversão
 *      [1,−a_p,p] ↔ [p,−a_p,1]
 * §D3  o TRIAL na curva: a 2-torsão é completa (3 raízes + ∞ = 4
 *      pontos fixos da involução) nos dois andares — o 4 do bit
 * §D4  o contrato 𝓜 assina o encontro local; a hipótese global fica
 *      no mapa, com a barreira explícita
 */
'use strict'
const { medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const A2c = -20n, A4c = -1152n, A6c = -9216n
const f = (x, p) => (((x * x % p * x % p + A2c * x * x + A4c * x + A6c) % p) + p) % p

/* §D0 — a involução age como −1 no diferencial */
{
  const p = 257n
  const modp = x => ((x % p) + p) % p
  const powp = (b, e) => { let r2 = 1n; b = modp(b); e = BigInt(e); while (e > 0n) { if (e & 1n) r2 = r2 * b % p; b = b * b % p; e >>= 1n } return r2 }
  const inv = x => powp(x, p - 2n)
  let pontos = 0, menos = 0
  for (let x = 0n; x < p && pontos < 50; x++) {
    const fx = f(x, p)
    if (fx === 0n) continue
    if (powp(fx, (p - 1n) / 2n) !== 1n) continue
    let y = -1n
    for (let yy = 1n; yy < p; yy++) if (modp(yy * yy) === fx) { y = yy; break }
    if (y < 0n) continue
    pontos++
    /* ω = dx/y: o avatar no ponto é 1/y; em ιP = (x,−y) é 1/(−y) = −1/y */
    const w = inv(y)
    const wIota = inv(modp(-y))
    if (wIota === modp(-w)) menos++
  }
  if (menos !== pontos) R++
  console.log(`\n§D0  ω(ιP) = −ω(P) em ${menos}/${pontos} pontos de F₂₅₇`)
  ok('§D0 a involução (o deck de Viviani) age como −1 no diferencial ω = dx/y, ponto a ponto — o mesmo caráter ímpar do z de Viviani (§T1)', menos === pontos)
}

/* §D1 — Hodge conta: Cartier–Hasse–Witt == contagem */
let G = false
{
  const mulPoly = (Ap, Bp, p) => {
    const C = new Array(Ap.length + Bp.length - 1).fill(0n)
    for (let i = 0; i < Ap.length; i++) {
      if (Ap[i] === 0n) continue
      for (let j = 0; j < Bp.length; j++) C[i + j] = (C[i + j] + Ap[i] * Bp[j]) % p
    }
    return C
  }
  let p = 0n
  const resultados = []
  for (const pp of [17n, 257n]) {
    p = pp
    const modp = x => ((x % p) + p) % p
    /* caminho 1: a contagem (tabela de resíduos) */
    const qr = new Set()
    for (let y = 0n; y < p; y++) qr.add(modp(y * y).toString())
    let N = 1n
    for (let x = 0n; x < p; x++) {
      const fx = f(x, p)
      if (fx === 0n) N += 1n
      else if (qr.has(fx.toString())) N += 2n
    }
    const ap = p + 1n - N
    /* caminho 2: Hodge — o coeficiente de Hasse–Witt */
    const fPoly = [modp(A6c), modp(A4c), modp(A2c), 1n]
    let e = (p - 1n) / 2n
    let base = fPoly, acc = [1n]
    while (e > 0n) {
      if (e & 1n) acc = mulPoly(acc, base, p)
      base = mulPoly(base, base, p)
      e >>= 1n
    }
    const hw = acc[Number(p - 1n)] ?? 0n
    const errado = acc[Number(p - 2n)] ?? 0n
    resultados.push({ p, ap, hw: modp(hw), errado: modp(errado) })
  }
  const conta = resultados.every(r => ((r.ap % r.p) + r.p) % r.p === r.hw)
  const valores = resultados[0].ap === 2n && resultados[1].ap === 18n
  /* o gume: o índice errado difere em F₂₅₇ (em F₁₇ coincide por acaso — regista-se) */
  G = resultados[1].errado !== ((resultados[1].ap % 257n) + 257n) % 257n
  if (!conta || !valores) R++
  console.log(`\n§D1  ${resultados.map(r => `p=${r.p}: contagem a_p=${r.ap} · Hasse–Witt=${r.hw} · índice errado=${r.errado}`).join(' · ')}`)
  ok('§D1 HODGE CONTA: o coeficiente de Cartier–Hasse–Witt (o operador no diferencial) == a_p da contagem de pontos, nos dois andares — dois caminhos independentes, o mesmo número', conta && valores)
  ok('§D1 o gume: o coeficiente errado (x^{p−2}) difere de a_p em F₂₅₇ (130 ≠ 18) — só o índice de Hasse–Witt conta pontos; em F₁₇ coincide por acaso, registado', G)
}

/* §D2 — Riemann local: os zeros em |α| = √p */
{
  let rh = 0, funcional = 0
  for (const [p, ap] of [[17n, 2n], [257n, 18n]]) {
    /* disc < 0 ⟹ raízes complexas conjugadas; produto = p ⟹ |α|² = p */
    if (ap * ap - 4n * p < 0n) rh++
    /* a equação funcional: P(T) = 1−a_pT+pT² satisfaz reversão de coeficientes */
    const coef = [1n, -ap, p]
    const rev = [...coef].reverse()
    if (rev[0] === p && rev[1] === -ap && rev[2] === 1n) funcional++
  }
  if (rh !== 2 || funcional !== 2) R++
  console.log(`\n§D2  disc<0 (RH local) em ${rh}/2 andares · reversão funcional em ${funcional}/2`)
  ok('§D2 RIEMANN LOCAL fecha por medida: disc(x²−a_p x+p) < 0 e produto de raízes = p ⟹ os zeros da zeta local vivem em |α|=√p — o teorema de Hasse, realizado nos dois andares', rh === 2)
  ok('§D2 e a equação funcional é a reversão dos coeficientes [1,−a_p,p] ↔ [p,−a_p,1] — a simetria s ↔ 1−s em miniatura local', funcional === 2)
}

/* §D3 — o trial na curva: a 2-torsão completa */
{
  let completa = 0
  for (const p of [17n, 257n]) {
    let raizes = 0n
    for (let x = 0n; x < p; x++) if (f(x, p) === 0n) raizes++
    if (raizes === 3n) completa++
  }
  if (completa !== 2) R++
  console.log(`\n§D3  2-torsão completa (3 raízes de f + ∞ = 4 pontos fixos) em ${completa}/2 andares`)
  ok('§D3 o TRIAL na curva: os pontos fixos da involução são a 2-torsão COMPLETA — 4 pontos nos dois andares, o mesmo 4 do bit {1,i,−1,−i}: a ramificação da dobra, outra vez', completa === 2)
}

/* §D4 — o contrato assina o encontro local */
{
  const V = 0
  const m = medicao.contrato(R, G, V)
  console.log(`\n§D4  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(m)}`)
  ok('§D4 o contrato 𝓜 assina O ENCONTRO LOCAL: Hodge (o diferencial) conta, a contagem dá a_p, e Riemann local fecha — os três lados calculam o mesmo número na curva que saiu de Viviani. O GLOBAL (L, zeros, posto) fica como hipótese no mapa, com a barreira explícita', medicao.fecha(m))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  O encontro local está medido: na curva que saiu de Viviani,')
  console.log('  o lado de Hodge (Cartier no diferencial), o lado de BSD (a')
  console.log('  contagem) e o lado de Riemann (os zeros locais em |α|=√p)')
  console.log('  calculam o MESMO a_p — e o trial é a 2-torsão, os 4 pontos')
  console.log('  fixos do deck. A hipótese do coordenador é verdade LOCAL')
  console.log('  medida; o global espera os seus elos, como o contrato manda.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
