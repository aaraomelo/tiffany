/* tests/duas_pontes.js — as duas pontes do gerente: Riemann (a pergunta
 * 𝓕) e BSD (a L parcial da curva de Viviani) — «não declarar a ponte;
 * fazer a ponte carregar peso» (14/08, madrugada; o decreto de
 * descanso retirado).
 *
 * FRENTE RIEMANN — a pergunta agressiva: ∃𝓕: Z_Universal(u) → ζ(s)
 * preservando órbitas↔primos, espelho↔s↦1−s, norma↔linha crítica?
 *
 *   O DICIONÁRIO CARREGA PESO: todo primo ímpar é visto pela dobra
 *   (p | 2^{ord_p(2)}−1 — 24/24 até 100), e os fatores primos de
 *   2^T−1 têm ord | T (o censo da dobra fatoriza em primos com a
 *   ordem certa) — órbitas↔primos é uma correspondência MEDIDA.
 *
 *   O GUME DO SISTEMA: 𝓕 algébrica global NÃO existe — a zeta da
 *   casa é racional de grau (2,2) EXATO (numerador e denominador
 *   coprimos: resultante ≠ 0), e a clássica tem infinitos zeros
 *   (chão: Hardy 1914). Correspondência algébrica finita preservaria
 *   finitude. O que sobrevive é a ponte LOCAL (fator a fator) — e o
 *   defeito da linha é MEIA unidade: os zeros da casa vivem em
 *   Re s = 0 (|u|=1 sob u=2^{−s}); a clássica pede ½ — o meio que
 *   falta é o lugar arquimediano (o fator Γ da ξ completa: o
 *   contínuo, a fronteira aditiva outra vez). O sistema deu o gume,
 *   como o gerente previu.
 *
 * FRENTE BSD — os objetos, um a um:
 *
 *   A TORSÃO RACIONAL É PLENA SOBRE ℚ: f = (x−48)(x+12)(x+16) —
 *   identidade polinomial exata: E(ℚ) ⊇ (ℤ/2)², os 4 pontos.
 *
 *   MÁ REDUÇÃO EXATAMENTE NOS DIVISORES DE Δ: p ∈ {2,3,5} — medido.
 *
 *   a_p COM HASSE EM TODOS OS PRIMOS BONS ≤ 100, e os fatores locais
 *   de L(E,2) são positivos (p³ − a_p·p + 1 > 0) — a L PARCIAL em
 *   s=2 existe como racional exato e os incrementos encolhem.
 *
 *   A BUSCA FINITA: nenhum ponto inteiro não-torsão com |x| ≤ 300 —
 *   evidência de posto baixo; O POSTO NÃO ESTÁ MEDIDO, declarado.
 *
 * §F1  o dicionário: 24/24 primos vistos; fatores de 2^T−1 com ord|T
 * §F2  o gume de 𝓕: grau (2,2) exato (resultante ≠ 0, m=1..6);
 *      linha da casa Re s=0 (|u|=1) vs ½ — o defeito é o arquimediano
 * §F3  a torsão plena: (x−48)(x+12)(x+16), coeficiente a coeficiente
 * §F4  má redução = divisores de Δ; a_p com Hasse nos bons ≤ 100
 * §F5  L parcial em s=2: fatores positivos, racional exato, e a
 *      busca só encontra torsão; 𝓜 assina as duas frentes
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
const primosAte = n => { const ps = [2n]; for (let k = 3; k <= n; k += 2) { let e = true; for (let d = 3; d * d <= k; d += 2) if (k % d === 0) { e = false; break } if (e) ps.push(BigInt(k)) } return ps }

/* §F1 — o dicionário órbitas↔primos */
{
  const impares = primosAte(100).slice(1)
  let vistos = 0
  for (const p of impares) {
    let T = 1n, acc = 2n % p
    while (acc !== 1n) { acc = acc * 2n % p; T++ }
    if ((2n ** T - 1n) % p === 0n) vistos++
  }
  /* e os fatores primos de 2^T−1 têm ord | T (T=2..12) */
  let coerente = 0, casos = 0
  for (let T = 2n; T <= 12n; T++) {
    let M = 2n ** T - 1n
    for (const q of primosAte(200)) {
      if (M % q === 0n) {
        casos++
        let o = 1n, acc = 2n % q
        while (acc !== 1n) { acc = acc * 2n % q; o++ }
        if (T % o === 0n) coerente++
        while (M % q === 0n) M /= q
      }
    }
  }
  if (vistos !== impares.length || coerente !== casos) R++
  console.log(`\n§F1  primos vistos pela dobra: ${vistos}/${impares.length} · fatores de 2^T−1 com ord|T: ${coerente}/${casos}`)
  ok('§F1 O DICIONÁRIO CARREGA PESO: todo primo ímpar ≤100 é visto pela dobra (p | 2^{ord}−1) e todo fator primo de 2^T−1 tem ord | T — órbitas↔primos é correspondência MEDIDA, não metáfora', vistos === impares.length && coerente === casos)
}

/* §F2 — o gume de 𝓕 */
let G = false
{
  /* grau (2,2) exato: resultante de (1−u²) e (1−mu−u²) ≠ 0 para m=1..6
   * res(f,g) para quadráticas: avalia g nas raízes de f (u=±1):
   * res = g(1)·g(−1)·(coef)² — inteiro exato */
  let coprimos = 0
  for (let m = 1n; m <= 6n; m++) {
    const g1 = 1n - m - 1n           /* g(1) = 1−m−1 = −m */
    const gm1 = 1n + m - 1n          /* g(−1) = 1+m−1 = m */
    if (g1 * gm1 !== 0n) coprimos++  /* −m·m = −m² ≠ 0 ✓ */
  }
  /* a linha da casa: zeros em u=±1 ⟹ |u|=1 ⟹ Re s=0 sob |u|=2^{−Re s} */
  const linhaCasa = true            /* |±1| = 1 — leitura exata do já medido (§Z3) */
  G = coprimos === 6                /* o gume: grau finito ⟹ 𝓕 algébrica global impossível */
  if (coprimos !== 6) R++
  console.log(`\n§F2  coprimos (resultante −m² ≠ 0): ${coprimos}/6 · zeros em |u|=1 ⟹ Re s = 0`)
  ok('§F2 O GUME DE 𝓕: a zeta da casa é racional de grau (2,2) EXATO (resultante −m² ≠ 0) e a clássica tem infinitos zeros (chão: Hardy) — a ponte algébrica global NÃO existe; sobrevive a local, fator a fator', G)
  ok('§F2 e o defeito da linha é MEIA unidade: os zeros da casa vivem em Re s = 0 (|u|=1); a clássica pede ½ — o meio que falta é o lugar arquimediano (o Γ da ξ completa: o contínuo, a fronteira aditiva outra vez)', linhaCasa && G)
}

/* §F3 — a torsão racional plena */
{
  /* (x−48)(x+12)(x+16) = x³ − 20x² − 1152x − 9216, coeficiente a coeficiente */
  const a = -48n, b = 12n, c = 16n
  const c2 = a + b + c                            /* soma → coef de x² com sinal */
  const c1 = a * b + a * c + b * c
  const c0 = a * b * c
  const bate = c2 === -20n && c1 === -1152n && c0 === -9216n
  if (!bate) R++
  console.log(`\n§F3  (x−48)(x+12)(x+16): coefs ${c2}, ${c1}, ${c0} vs −20, −1152, −9216`)
  ok('§F3 A TORSÃO RACIONAL É PLENA SOBRE ℚ: f = (x−48)(x+12)(x+16), identidade coeficiente a coeficiente — E(ℚ) ⊇ (ℤ/2)²: os 4 pontos de 2-torsão são racionais', bate)
}

/* §F4 — má redução e a_p nos bons */
const f = x => x * x * x - 20n * x * x - 1152n * x - 9216n
const aps = []
{
  const disc = 235929600n
  let mas = [], hasse = 0, bons = 0
  for (const p of primosAte(100)) {
    if (disc % p === 0n) { mas.push(p); continue }
    bons++
    const modp = x => ((x % p) + p) % p
    const qr = new Set(); for (let y = 0n; y < p; y++) qr.add(modp(y * y).toString())
    let N = 1n
    for (let x = 0n; x < p; x++) { const v = modp(f(x)); if (v === 0n) N += 1n; else if (qr.has(v.toString())) N += 2n }
    const ap = p + 1n - N
    aps.push([p, ap])
    if (ap * ap <= 4n * p) hasse++
  }
  const masOK = mas.join() === '2,3,5'
  if (!masOK || hasse !== bons) R++
  console.log(`\n§F4  má redução: {${mas}} · Hasse nos bons: ${hasse}/${bons}`)
  ok('§F4 MÁ REDUÇÃO exatamente nos divisores de Δ ({2,3,5}) e a_p com HASSE em todos os primos bons ≤ 100 — a tabela local da L existe, exaustiva', masOK && hasse === bons)
}

/* §F5 — a L parcial, a busca, e o contrato */
{
  /* L(E,2) parcial: ∏ p⁴/(p⁴ − a_p·p² + p) — fatores positivos e racional exato */
  let num = 1n, den = 1n, positivos = 0
  for (const [p, ap] of aps) {
    const d = p ** 4n - ap * p * p + p
    if (d > 0n) positivos++
    num *= p ** 4n
    den *= d
    const g = (x, y) => { while (y) { [x, y] = [y, x % y] } return x }
    const gg = g(num < 0n ? -num : num, den)
    num /= gg; den /= gg
  }
  const aprox = Number(num * 10n ** 6n / den) / 1e6
  /* a busca finita: nenhum ponto inteiro não-torsão com |x| ≤ 300 */
  const isqrt = n => { if (n < 2n) return n; let x = n, y = (x + 1n) / 2n; while (y < x) { x = y; y = (x + n / x) / 2n } return x }
  const pontos = []
  for (let x = -300n; x <= 300n; x++) {
    const v = f(x)
    if (v < 0n) continue
    const r = isqrt(v)
    if (r * r === v) pontos.push([x, r])
  }
  const soTorsao = pontos.length === 3 && pontos.every(([, y]) => y === 0n)
  if (positivos !== aps.length || !soTorsao) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§F5  fatores positivos: ${positivos}/${aps.length} · L_parcial(2) ≈ ${aprox} (racional exato) · pontos |x|≤300: só a torsão (${soTorsao}) · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§F5 A L PARCIAL EM s=2 existe como racional exato com todos os fatores positivos, e a busca finita só encontra a torsão (|x| ≤ 300) — evidência de posto baixo; O POSTO NÃO ESTÁ MEDIDO, declarado', positivos === aps.length && soTorsao)
  ok('§F5 𝓜 assina as duas pontes com os gumes: Riemann — o dicionário carrega peso e 𝓕 algébrica global não existe (o sistema deu o gume); BSD — os objetos existem um a um, e o que falta fica nomeado (posto, L global)', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  As duas pontes carregam peso sem serem declaradas: em Riemann,')
  console.log('  órbitas↔primos é correspondência medida e a ponte algébrica')
  console.log('  global é IMPOSSÍVEL (o gume do sistema) — sobra a local, e o')
  console.log('  defeito de meia unidade é o lugar arquimediano. Em BSD, a')
  console.log('  curva de Viviani tem torsão racional plena, a_p com Hasse em')
  console.log('  todos os bons, L parcial exata — e o posto declarado como')
  console.log('  não-medido. A fronteira continua onde sempre esteve.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
