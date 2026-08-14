/* tests/eliptica_viviani.js — a curva elíptica sai de Viviani, pela
 * desafinação do nó (ordem do coordenador, 14/08 madrugada: «a curva
 * elíptica sai de viviani»; o elo autorizado pela mesa: família → nó →
 * desafinação → Weierstrass → pontos/involução → a_p — e NADA além:
 * a_p medido ≠ L(E,s) construída ≠ BSD demonstrado).
 *
 * O achado central, preservado como a mesa pediu:
 *
 *     VIVIANI NÃO É ELÍPTICA — E É POR ISSO QUE A ELÍPTICA SAI DELA.
 *
 *   A FAMÍLIA: eliminar x de {esfera R², cilindro (x−c)²+y²=r²} dá
 *   Y² = −z⁴ + 2Kz² + C, com K = R²−c²−r² e C = 4c²r²−K² (identidade
 *   polinomial exata). Viviani é (R,c,r) = (2a,a,a): C = 0 — a fibra
 *   NODAL (o nó em z=0 é a auto-interseção/ramificação já medida em
 *   viviani_universal §V3); o género cai, a curva é racional.
 *
 *   A DESAFINAÇÃO: (5,2,2) dá q(z) = (z²−9)(25−z²), quatro raízes
 *   distintas — fibra suave, género 1. A transformação z = 3+1/t,
 *   Y = s/(z−3)² leva-a REVERSIVELMENTE na cúbica
 *   s² = 96t³−20t²−12t−1, e daí em Weierstrass
 *   y² = x³−20x²−1152x−9216 (x=96t, y=96s), com Δ ≠ 0.
 *
 *   O DECK VIRA A INVOLUÇÃO DE WEIERSTRASS: Y↦−Y (a troca de folha de
 *   Viviani) corresponde a s↦−s, i.e. (x,y)↦(x,−y) — a geometria
 *   passa à aritmética sem resto.
 *
 *   E O PRIMEIRO COEFICIENTE DA L NASCE MEDIDO: contagens EXAUSTIVAS
 *   #E(F₁₇) e #E(F₂₅₇) por DOIS caminhos (tabela de resíduos vs
 *   critério de Euler), a_p = p+1−N, e Hasse a_p² ≤ 4p inteiro.
 *   BSD continua no mapa: L global e posto são elos independentes.
 *
 * §V0  a família: Y² = −z⁴+2Kz²+C, identidade verificada em varredura
 *      de (R,c,r) e z — a eliminação é exata
 * §V1  o nó: Viviani (2a,a,a) tem C = 0 para todo a (a fibra nodal),
 *      q(0)=q'(0)=0; a desafinada (5,2,2) tem 4 raízes distintas e
 *      gcd(q,q') constante — suave
 * §V2  a transformação reversível (R_curva = 0): amostra de pontos de
 *      Y²=q(z) em F₂₅₇ → cúbica → VOLTA byte a byte; e o deck Y↦−Y
 *      vira s↦−s
 * §V3  Weierstrass: y² = x³−20x²−1152x−9216 com Δ ≠ 0 (BigInt), e os
 *      pontos da cúbica passam ao modelo por (x,y)=(96t,96s), exato
 * §V4  os pontos contam-se: #E(F₁₇) e #E(F₂₅₇) exaustivos por dois
 *      caminhos; a_p = p+1−N; Hasse a_p² ≤ 4p — os objetos de BSD
 *      nascem; a conjectura NÃO se toca (declarado)
 * §V5  o gume e o contrato: a fibra NODAL falha o teste elíptico
 *      (C=0 ⟹ disc=0 — Viviani mesma não é elíptica, como deve);
 *      𝓜 assina o elo geometria → aritmética
 */
'use strict'
const { anel, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0

/* §V0 — a família: a eliminação é exata */
{
  let identidade = 0, casos = 0
  for (const [Rr, c, r] of [[5n, 2n, 2n], [7n, 3n, 2n], [10n, 4n, 3n], [6n, 2n, 3n]]) {
    const K = Rr * Rr - c * c - r * r
    const C = 4n * c * c * r * r - K * K
    for (let z = -6n; z <= 6n; z++) {
      casos++
      /* o ponto da curva: x = (R²+c²−r²−z²)/(2c); Y = 2c·y com
       * y² = r²−(x−c)²  ⟹  Y² = 4c²r² − (R²−c²−r²−z²)²  = −z⁴+2Kz²+C */
      const z2 = z * z
      const lhs = 4n * c * c * r * r - (Rr * Rr - c * c - r * r - z2) * (Rr * Rr - c * c - r * r - z2)
      const rhs = -(z2 * z2) + 2n * K * z2 + C
      if (lhs === rhs) identidade++
    }
  }
  if (identidade !== casos) R++
  console.log(`\n§V0  Y² = −z⁴+2Kz²+C em ${identidade}/${casos} (4 famílias × 13 valores de z)`)
  ok('§V0 a FAMÍLIA: eliminar x de esfera∩cilindro dá Y² = −z⁴+2Kz²+C com K=R²−c²−r², C=4c²r²−K² — identidade polinomial exata', identidade === casos)
}

/* §V1 — o nó de Viviani, e a fibra suave */
{
  let nodal = 0
  for (let a = 1n; a <= 8n; a++) {
    const K = 4n * a * a - a * a - a * a
    const C = 4n * a * a * a * a - K * K
    /* C = 4a⁴ − (2a²)² = 0: a fibra nodal; e q(0)=C=0, q'(0)=0 (só termos pares) */
    if (C === 0n && K === 2n * a * a) nodal++
  }
  /* a desafinada: q = (z²−9)(25−z²): raízes ±3, ±5 distintas; gcd(q,q') sem raiz comum */
  const q = z => (z * z - 9n) * (25n - z * z)
  const raizes = [3n, -3n, 5n, -5n]
  const distintas = new Set(raizes.map(String)).size === 4 && raizes.every(z => q(z) === 0n)
  /* suavidade: q'(z) = −4z³+68z ≠ 0 nas raízes */
  const qd = z => -4n * z * z * z + 68n * z
  const suave = raizes.every(z => qd(z) !== 0n)
  if (nodal !== 8 || !distintas || !suave) R++
  console.log(`\n§V1  Viviani C=0 em ${nodal}/8 valores de a · desafinada: 4 raízes distintas ${distintas} · q'≠0 nas raízes: ${suave}`)
  ok('§V1 VIVIANI É A FIBRA NODAL: C = 0 exato para todo a — o nó em z=0 é a auto-interseção já medida; o género cai, a curva é racional', nodal === 8)
  ok('§V1 a DESAFINADA (5,2,2) é suave: q = (z²−9)(25−z²) com 4 raízes distintas e q′ ≠ 0 em todas — género 1', distintas && suave)
}

/* a cúbica e o modelo de Weierstrass */
const P = 257n
const A2 = anel(257)
const mod = x => ((x % P) + P) % P
const powm = (b, e) => { let r2 = 1n; b = mod(b); e = BigInt(e); while (e > 0n) { if (e & 1n) r2 = r2 * b % P; b = b * b % P; e >>= 1n } return r2 }
const inv = x => powm(x, P - 2n)

/* §V2 — a transformação reversível, e o deck */
let V = 0
{
  let amostra = 0, cubica = 0, volta = 0, deck = 0
  for (let z = 0n; z < P && amostra < 60; z++) {
    if (z === 3n) continue
    const z2 = z * z
    const qz = mod(-(z2 * z2) + 34n * z2 - 225n)
    if (qz === 0n) continue
    if (powm(qz, (P - 1n) / 2n) !== 1n) continue
    let Y = -1n
    for (let y = 1n; y < P; y++) if (mod(y * y) === qz) { Y = y; break }
    if (Y < 0n) continue
    amostra++
    const t = inv(mod(z - 3n))
    const s = mod(Y * inv(mod((z - 3n) * (z - 3n))))
    const t3 = t * t % P * t % P
    if (mod(s * s) === mod(96n * t3 - 20n * t * t - 12n * t - 1n)) cubica++
    const zv = mod(3n + inv(t)), Yv = mod(s * inv(mod(t * t)))
    if (zv === z && Yv === Y) volta++
    /* o deck: (z, −Y) ↦ (t, −s) */
    const sNeg = mod((P - Y) * inv(mod((z - 3n) * (z - 3n))))
    if (sNeg === mod(-s)) deck++
  }
  if (cubica !== amostra || volta !== amostra || deck !== amostra) V++
  console.log(`\n§V2  amostra ${amostra} · cúbica ${cubica} · volta byte a byte ${volta} · deck Y↦−Y vira s↦−s: ${deck}`)
  ok('§V2 a transformação é REVERSÍVEL no conjunto declarado (R_curva = 0): ida à cúbica, volta byte a byte, nos 60 pontos de F₂₅₇', cubica === amostra && volta === amostra)
  ok('§V2 e o DECK de Viviani (Y↦−Y) corresponde a s↦−s — a involução (x,y)↦(x,−y) de Weierstrass: a geometria passa à aritmética sem resto', deck === amostra)
}

/* §V3 — Weierstrass com Δ ≠ 0, e a passagem (x,y) = (96t, 96s) */
{
  /* y² = x³ + a2x² + a4x + a6, com a2=−20, a4=−1152, a6=−9216
   * Δ do cúbico x³+a2x²+a4x+a6: disc = 18abc... usa a fórmula do disc de
   * f = x³+px+q após depressão? Mede-se sem depressão: disc(f) via
   * resultante f, f' em BigInt (Sylvester 5×5 é pesado; usa a forma
   * fechada do disc para cúbica geral a=1):
   * disc = 18·a2·a4·a6 − 4·a2³·a6 + a2²·a4² − 4·a4³ − 27·a6² */
  const a2 = -20n, a4 = -1152n, a6 = -9216n
  const disc = 18n * a2 * a4 * a6 - 4n * a2 * a2 * a2 * a6 + a2 * a2 * a4 * a4 - 4n * a4 * a4 * a4 - 27n * a6 * a6
  /* a passagem: (t,s) da cúbica → (x,y)=(96t,96s) satisfaz o modelo */
  let passa = 0, amostra = 0
  for (let t = 1n; t < P && amostra < 40; t++) {
    const t3 = t * t % P * t % P
    const s2 = mod(96n * t3 - 20n * t * t - 12n * t - 1n)
    if (s2 === 0n || powm(s2, (P - 1n) / 2n) !== 1n) continue
    let s = -1n
    for (let y = 1n; y < P; y++) if (mod(y * y) === s2) { s = y; break }
    if (s < 0n) continue
    amostra++
    const x = mod(96n * t), y = mod(96n * s)
    const x3 = x * x % P * x % P
    if (mod(y * y) === mod(x3 + a2 * x * x + a4 * x + a6)) passa++
  }
  if (disc === 0n || passa !== amostra) R++
  console.log(`\n§V3  Δ = ${disc} (≠0) · (96t,96s) satisfaz o modelo em ${passa}/${amostra}`)
  ok('§V3 o modelo de WEIERSTRASS y² = x³−20x²−1152x−9216 tem Δ ≠ 0 (BigInt) e recebe a cúbica por (x,y) = (96t,96s), exato', disc !== 0n && passa === amostra)
}

/* §V4 — os pontos contam-se: a_p com Hasse, por dois caminhos */
{
  const resultados = []
  for (const p of [17n, 257n]) {
    const modp = x => ((x % p) + p) % p
    const powp = (b, e) => { let r2 = 1n; b = modp(b); e = BigInt(e); while (e > 0n) { if (e & 1n) r2 = r2 * b % p; b = b * b % p; e >>= 1n } return r2 }
    /* caminho 1: tabela de resíduos */
    const qr = new Set()
    for (let y = 0n; y < p; y++) qr.add(modp(y * y).toString())
    let N1 = 1n
    /* caminho 2: critério de Euler */
    let N2 = 1n
    for (let x = 0n; x < p; x++) {
      const f = modp(x * x % p * x % p - 20n * x * x - 1152n * x - 9216n)
      if (f === 0n) { N1 += 1n; N2 += 1n; continue }
      if (qr.has(f.toString())) N1 += 2n
      if (powp(f, (p - 1n) / 2n) === 1n) N2 += 2n
    }
    const ap = p + 1n - N1
    resultados.push({ p, N: N1, N2, ap, hasse: ap * ap <= 4n * p })
  }
  const doisCaminhos = resultados.every(r => r.N === r.N2)
  const hasse = resultados.every(r => r.hasse)
  const valores = resultados[0].N === 16n && resultados[0].ap === 2n &&
    resultados[1].N === 240n && resultados[1].ap === 18n
  if (!doisCaminhos || !hasse || !valores) R++
  console.log(`\n§V4  ${resultados.map(r => `F_${r.p}: N=${r.N} (dois caminhos: ${r.N === r.N2}) a_p=${r.ap} Hasse:${r.hasse}`).join(' · ')}`)
  ok('§V4 os pontos CONTAM-SE: #E(F₁₇)=16 e #E(F₂₅₇)=240, exaustivos e por DOIS caminhos (tabela vs Euler) — a_p = 2 e 18', doisCaminhos && valores)
  ok('§V4 HASSE fecha nos dois andares (a_p² ≤ 4p, inteiro) — o primeiro coeficiente da L-função nasce medido; a conjectura BSD NÃO se toca: L global e posto são elos independentes, declarado', hasse)
}

/* §V5 — o gume e o contrato */
{
  /* a fibra NODAL falha o teste elíptico: com C=0, q(z)=z²(2K−z²) tem
   * raiz dupla em 0 ⟹ q e q' partilham a raiz ⟹ disc = 0 */
  const K = 98n                                 /* a=7: Viviani */
  const qN = z => z * z * (2n * K - z * z)
  const qNd = z => 4n * K * z - 4n * z * z * z
  const G = qN(0n) === 0n && qNd(0n) === 0n     /* raiz dupla: o nó — não é elíptica */
  const m2 = medicao.contrato(R, G, V)
  console.log(`\n§V5  a nodal tem raiz dupla em 0 (q=q'=0): ${G} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(m2)}`)
  ok('§V5 o GUME: a fibra nodal FALHA o teste elíptico (raiz dupla no nó, disc=0) — Viviani mesma não é elíptica, exatamente como deve ser', G)
  ok('§V5 o contrato 𝓜 assina o elo geometria → aritmética: a elíptica sai de Viviani PELA DESAFINAÇÃO DO NÓ, com transformação reversível e a_p com Hasse', medicao.fecha(m2))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A curva elíptica sai de Viviani — desafinando o nó: Viviani')
  console.log('  é a fibra nodal (C=0, racional); a fibra genérica é suave,')
  console.log('  vai reversivelmente a Weierstrass, o deck vira (x,−y), e os')
  console.log('  pontos contam-se com Hasse nos dois andares. A geometria')
  console.log('  passou à aritmética sem resto. BSD continua no mapa: a_p')
  console.log('  medido não é L construída, e L construída não é BSD.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
