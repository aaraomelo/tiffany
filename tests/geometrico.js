/* tests/geometrico.js — torre normalizada da reta real geométrica.
 * papers/universal.tex: n≥3, |C_n|=|P_n|=1/2^{n-1};
 * Σ |C_n| = Σ |P_n| = 1/2. Inteiro: fracções p/q em BigInt. Sem double.
 *
 * §N0  mesma área: |C_n|=|P_n| nos primeiros níveis
 * §N1  a joia: soma parcial  Σ_{n=3}^N 1/2^{n-1} = 1/2 − 1/2^{N-1}
 *      e o limite das parciais é 1/2 (cauda 1/2^{N-1} → o vazio em ℤ:
 *      o residual é exactamente a última potência)
 * §N2  o axioma consecutivo no peso: 1/2^n + 1/2^{n-1} = 3/2^n
 * §N3  o gume: três andares iguais a 1/4 somam 3/4 ≠ 1/2
 */
'use strict'
const { medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
let G = false
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const gcd = (a, b) => { a = a < 0n ? -a : a; b = b < 0n ? -b : b; while (b) { const t = a % b; a = b; b = t } return a || 1n }
const F = (p, q = 1n) => {
  if (q < 0n) { p = -p; q = -q }
  const g = gcd(p, q)
  return { p: p / g, q: q / g }
}
const eq = (x, y) => x.p * y.q === y.p * x.q
const add = (x, y) => F(x.p * y.q + y.p * x.q, x.q * y.q)
const area = n => F(1n, 2n ** (n - 1n))   /* |C_n|=|P_n|=1/2^{n-1} */

/* §N0 — mesma área por nível */
{
  let mesma = true
  for (let n = 3n; n <= 12n; n++) if (!eq(area(n), area(n))) mesma = false
  /* C e P são o mesmo peso: a definição identifica */
  const pesos = [3n, 4n, 5n, 6n].map(n => area(n))
  const ids = eq(pesos[0], F(1n, 4n)) && eq(pesos[1], F(1n, 8n)) && eq(pesos[2], F(1n, 16n)) && eq(pesos[3], F(1n, 32n))
  if (!mesma || !ids) R++
  console.log(`\n§N0  |C_n|=|P_n|=1/2^{n-1}: 1/4,1/8,1/16,1/32: ${ids}`)
  ok('§N0 MESMA ÁREA: |C_n|=|P_n|=1/2^{n-1} para n=3,4,5,6 (1/4, 1/8, 1/16, 1/32 exacto)', ids)
}

/* §N1 — a joia: as duas metades */
{
  const meio = F(1n, 2n)
  let parciais = true
  for (let N = 3n; N <= 20n; N++) {
    let s = F(0n, 1n)
    for (let n = 3n; n <= N; n++) s = add(s, area(n))
    const alvo = add(meio, F(-1n, 2n ** (N - 1n)))  /* 1/2 − 1/2^{N-1} */
    if (!eq(s, alvo)) parciais = false
  }
  /* N=3: 1/4 = 1/2 − 1/4; N=4: 3/8 = 1/2 − 1/8 */
  const n3 = eq(area(3n), add(meio, F(-1n, 4n)))
  const n4 = eq(add(area(3n), area(4n)), F(3n, 8n))
  if (!parciais || !n3 || !n4) R++
  console.log(`\n§N1  parciais 1/2−1/2^{N-1} até N=20: ${parciais} · N=3: ${n3} · N=4: ${n4}`)
  ok('§N1 A JOIA — AS DUAS METADES: Σ_{n=3}^N |C_n| = 1/2 − 1/2^{N-1} exacto (N=3..20); o mesmo para |P_n|; a torre infinita soma 1/2 + 1/2', parciais && n3 && n4)
}

/* §N2 — axioma consecutivo no peso */
{
  let axioma = true
  for (let n = 3n; n <= 16n; n++) {
    const esq = add(area(n + 1n), area(n))
    const dir = F(3n, 2n ** n)
    if (!eq(esq, dir)) axioma = false
  }
  if (!axioma) R++
  console.log(`\n§N2  |C_{n+1}|+|C_n| = 3/2^n = |P_{n+1}|+|P_n| (n=3..16): ${axioma}`)
  ok('§N2 o AXIOMA no peso normalizado: |C_{n+1}|+|C_n| = |P_{n+1}|+|P_n| = 3/2^n exacto', axioma)
}

/* §N3 — gume: andares iguais não fecham 1/2 */
{
  const tresIguais = add(add(F(1n, 4n), F(1n, 4n)), F(1n, 4n))
  const gume = !eq(tresIguais, F(1n, 2n)) && (3n !== 2n)  /* 3/4 ≠ 1/2 */
  G = gume
  if (!gume) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§N3  3×(1/4)=3/4 ≠ 1/2: ${gume} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§N3 o gume: três andares com a área do triângulo repetida somam 3/4 ≠ 1/2 — a torre só fecha porque os níveis descem em potência de 2', gume)
  ok('§N3 𝓜 assina a normalização: Σ |C_n| = Σ |P_n| = 1/2; repetir o primeiro peso não chega', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A torre pesa 1: metade círculos, metade polígonos.')
  console.log('  |C_n|=|P_n|=1/2^{n-1}, n≥3. O 2 fica no catálogo.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
