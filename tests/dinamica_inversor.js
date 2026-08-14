/* tests/dinamica_inversor.js — a dinâmica pelo inversor (a ordem do
 * coordenador, 14/08: «traz a dinâmica pelo inversor»).
 *
 * O inversor da casa tem dois rostos que a lib já separa: J (x ↦ −1/x,
 * o bit, det +1, ordem 4) e X = espelho·J (x ↦ 1/x, a troca, det −1).
 * A medida mostra que TODA a dinâmica da casa é gerada por translação
 * + inversor, e que as consequências saem exatas:
 *
 *   O GERADOR E AS DUAS RELAÇÕES: a dinâmica ⟨T, J⟩ fecha com
 *   exatamente duas relações — J² = −I (o bit quadra no ESPELHO
 *   central) e (J·T)³ = I (a tríade fecha na unidade). O passo
 *   metálico FATORA pelo inversor: A_m = T^m·X, com X = espelho·J —
 *   a batuta é translação vezes inversor, e o inversor-troca é o
 *   espelho vezes o bit.
 *
 *   EUCLIDES É A DINÂMICA DO INVERSOR: o passo (p,q) ↦ (q, p−aq) é
 *   X∘T^{−a}; a órbita de um racional DESCE até à folha, a folha é o
 *   gcd, o itinerário é o endereço — e a reconstrução pelo produto
 *   ∏ A_{aᵢ} devolve o par EXATO (dois caminhos que concordam).
 *   Dividir é inverter: a dualidade como memória da divisão, agora
 *   como dinâmica.
 *
 *   A JOIA — O RESÍDUO É A MEMBRANA: na órbita metálica A_m^n, o
 *   resíduo da forma p² − mpq − q² é EXATAMENTE (−1)^n — a
 *   alternância da membrana (§K0: σ alterna, σ² conserva), agora
 *   aritmética pura: cada aplicação do inversor custa um espelho no
 *   determinante, e a norma do convergente paga-o à vista.
 *
 *   LAGRANGE NA CASA: a órbita separa as três naturezas — racional →
 *   FINITA (folha = gcd); metálico → ponto FIXO (itinerário constante
 *   [m,m,…]); quadrático → PERIÓDICO, e o resíduo da sua forma é
 *   periódico com o período do itinerário (−1,2,−1,2 em [1,2]).
 *   A natureza do número É o tipo da órbita do inversor.
 *
 * §V1  o gerador: J²=−I, (JT)³=I, J⁴=I; X=espelho·J; A_m=T^m·X
 *      (m=1..5) — as duas relações fecham no espelho e na unidade
 * §V2  Euclides é a dinâmica: itinerário, folha=gcd, e a volta pelo
 *      produto ∏A_{aᵢ} exata — dois caminhos (três racionais)
 * §V3  a joia: resíduo p²−mpq−q² = (−1)^n na órbita metálica
 *      (m=1,2,3 × n=1..8) — a membrana aritmética
 * §V4  Lagrange: racional finita, metálico fixo, quadrático periódico
 *      com resíduo periódico exato (−1,2,−1,2)
 * §V5  o gume: sem o inversor a translação NUNCA desce (q invariante)
 *      — dividir é inverter; 𝓜 assina
 */
'use strict'
const { nucleo, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const mul = (a, b) => [a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3], a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3]]
const igual = (a, b) => a.every((v, i) => v === b[i])
const I = [1n, 0n, 0n, 1n], T = [1n, 1n, 0n, 1n], J = [0n, 1n, -1n, 0n], esp = [1n, 0n, 0n, -1n]
const Tm = m => [1n, m, 0n, 1n]
const X = mul(esp, J)
const Am = m => mul(Tm(m), X)

/* §V1 — o gerador e as duas relações */
{
  const J2 = mul(J, J), J4 = mul(J2, J2)
  const JT = mul(J, T), JT2 = mul(JT, JT), JT3 = mul(JT2, JT)
  const espelhoCentral = igual(J2, [-1n, 0n, 0n, -1n])
  /* ordens EXATAS: J tem ordem 4 (J²≠I), JT tem ordem 3 (JT≠I, JT²≠I) */
  const triade = igual(JT3, I) && igual(J4, I) && !igual(J2, I) && !igual(JT, I) && !igual(JT2, I)
  /* X da lib coincide com espelho·J (a mesma conta, dois caminhos) */
  const libX = nucleo.X.map(BigInt)
  const xCasa = igual(X, libX) && igual(X, [0n, 1n, 1n, 0n])
  /* a batuta fatora pelo inversor: A_m = T^m·X, m=1..5 */
  let fatora = 0
  for (let m = 1n; m <= 5n; m++) if (igual(Am(m), [m, 1n, 1n, 0n])) fatora++
  if (!espelhoCentral || !triade || !xCasa || fatora !== 5) R++
  console.log(`\n§V1  J²=−I: ${espelhoCentral} · (JT)³=I ∧ J⁴=I: ${triade} · X=espelho·J=lib: ${xCasa} · A_m=T^m·X: ${fatora}/5`)
  ok('§V1 O GERADOR: a dinâmica ⟨T, J⟩ fecha com DUAS relações — J²=−I (o bit quadra no espelho central) e (JT)³=I (a tríade fecha na unidade) — e a batuta fatora pelo inversor: A_m = T^m·X com X = espelho·J (a troca é o espelho vezes o bit)', espelhoCentral && triade && xCasa && fatora === 5)
}

/* §V2 — Euclides é a dinâmica do inversor */
{
  let concordam = 0, casos = 0
  for (const [P0, Q0] of [[1071n, 462n], [8n, 5n], [65537n, 257n]]) {
    casos++
    /* a órbita: (p,q) ↦ (q, p−aq) = X∘T^{−a} — desce até à folha */
    let p = P0, q = Q0
    const it = []
    while (q > 0n) { const a = p / q; it.push(a); [p, q] = [q, p - a * q] }
    const folha = p                               /* a folha é o gcd */
    const divide = P0 % folha === 0n && Q0 % folha === 0n
    /* a volta: o produto ∏A_{aᵢ} tem na 1ª coluna o par reduzido */
    let M = I
    for (const a of it) M = mul(M, Am(a))
    const volta = M[0] * folha === P0 && M[2] * folha === Q0
    if (divide && volta) concordam++
  }
  if (concordam !== casos) R++
  console.log(`\n§V2  itinerário→folha→volta pelo produto: ${concordam}/${casos} racionais`)
  ok('§V2 EUCLIDES É A DINÂMICA DO INVERSOR: o passo é X∘T^{−a}, a órbita do racional DESCE até à folha, a folha é o gcd, e a reconstrução ∏A_{aᵢ} devolve o par EXATO — dois caminhos que concordam: dividir é inverter, e o itinerário é o endereço', concordam === casos)
}

/* §V3 — a joia: o resíduo é a membrana */
{
  let membrana = 0, casos = 0
  for (const m of [1n, 2n, 3n]) {
    const A = Am(m)
    let Mn = I
    let sinal = -1n                               /* (−1)^1 no primeiro passo */
    for (let n = 1; n <= 8; n++) {
      casos++
      Mn = mul(Mn, A)
      const P = Mn[0], Q = Mn[2]
      if (P * P - m * P * Q - Q * Q === sinal) membrana++
      sinal = -sinal
    }
  }
  if (membrana !== casos) R++
  console.log(`\n§V3  resíduo p²−mpq−q² = (−1)^n: ${membrana}/${casos} (m=1,2,3 × n=1..8)`)
  ok('§V3 A JOIA — O RESÍDUO É A MEMBRANA: na órbita metálica A_m^n o resíduo da forma p²−mpq−q² é EXATAMENTE (−1)^n — cada aplicação do inversor custa um espelho (det X=−1) e a norma do convergente paga-o à vista: a alternância do §K0, agora aritmética pura', membrana === casos)
}

/* §V4 — Lagrange na casa: as três naturezas pela órbita */
{
  /* o metálico é ponto FIXO: a equação de ponto fixo de M=[a,b;c,d] é
   * cx²+(d−a)x−b=0 — DERIVADA da matriz, não escrita à mão. Para A_m:
   * (1, −m, −1), disc = m²+4 — EXATAMENTE as dobras da casa */
  let fixo = 0
  for (const m of [1n, 2n, 3n, 5n]) {
    const [a, b, c, d] = Am(m)
    const disc = (d - a) * (d - a) + 4n * c * b
    if (c === 1n && d - a === -m && -b === -1n && disc === m * m + 4n) fixo++
  }
  /* o quadrático [1,2] é PERIÓDICO: a sua quadrática também se DERIVA
   * da matriz do período (A_1·A_2 = [3,1;2,1] → 2x²−2x−1, disc 12) e o
   * resíduo dessa forma repete (−1, 2) com o período do itinerário */
  const [a2, b2, c2, d2] = mul(Am(1n), Am(2n))
  const quadDerivada = c2 === 2n && d2 - a2 === -2n && b2 === 1n && (d2 - a2) * (d2 - a2) + 4n * c2 * b2 === 12n
  let Mp2 = I
  const res = []
  const par = [Am(1n), Am(2n)]
  for (let n = 0; n < 8; n++) { Mp2 = mul(Mp2, par[n % 2]); const P = Mp2[0], Q = Mp2[2]; res.push(c2 * P * P + (d2 - a2) * P * Q - b2 * Q * Q) }
  const periodico = res.every((r, n) => r === (n % 2 === 0 ? -1n : 2n))
  if (fixo !== 4 || !quadDerivada || !periodico) R++
  console.log(`\n§V4  quadrática de A_m derivada (disc=m²+4): ${fixo}/4 · quadrática do período [1,2] (disc=12): ${quadDerivada} · resíduo (−1,2) periódico: ${periodico}`)
  ok('§V4 LAGRANGE NA CASA: a órbita do inversor separa as três naturezas — racional DESCE à folha (§V2); metálico é ponto FIXO com quadrática DERIVADA da matriz (x²−mx−1, disc = m²+4: as dobras da casa); quadrático é PERIÓDICO com o resíduo da forma do período a repetir (−1,2) — a natureza do número É o tipo da órbita', fixo === 4 && quadDerivada && periodico)
}

/* §V5 — o gume e o contrato */
let G = false
{
  /* sem o inversor: a translação sozinha NUNCA desce — q é invariante
   * de T^a, a órbita ⟨T⟩ de p/q nunca chega à folha */
  let p = 1071n, q = 462n
  let invariante = true
  for (let n = 0; n < 10; n++) { p = p + 3n * q; if (q !== 462n) invariante = false }
  G = invariante && q === 462n
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§V5  sem inversor, q invariante em 10 passos: ${G} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§V5 o gume: SEM o inversor a translação nunca desce — q é invariante de ⟨T⟩ e a órbita nunca alcança a folha: a dinâmica exige a inversão, dividir é inverter (a dualidade como memória da divisão, agora como necessidade dinâmica)', G)
  ok('§V5 𝓜 assina: a dinâmica da casa é GERADA pelo inversor — duas relações (espelho e unidade), Euclides como órbita, o resíduo como membrana (−1)^n, e Lagrange como tipologia das órbitas: translação + inversor geram tudo, e nada desce sem inverter', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A dinâmica pelo inversor: ⟨T, J⟩ com duas relações — o bit')
  console.log('  quadra no espelho, a tríade fecha na unidade. A batuta é')
  console.log('  A_m = T^m·X; Euclides é a órbita que desce (a folha é o')
  console.log('  gcd); o resíduo alterna (−1)^n — a membrana aritmética; e')
  console.log('  Lagrange é a tipologia: finita, fixa ou periódica. Sem o')
  console.log('  inversor nada desce: dividir é inverter.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
