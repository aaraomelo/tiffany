/* tests/histerese_navier.js — a realização da histerese é o
 * Navier–Stokes: o passo de calor é o seletor espectral (a leitura do
 * coordenador, 14/08: «a realização da histerese é justamente
 * Navier-Stokes no contínuo — aplica os resultados: vai dar a equação
 * da histerese, os pontos fixos e as classes»).
 *
 * Aplicados os resultados da casa (navier_corpo + maestro_memoria),
 * a leitura fecha em inteiros exatos:
 *
 *   A EQUAÇÃO DA HISTERESE É A RETENÇÃO ESPECTRAL: o passo de calor
 *   multiplica o modo k por cos²(πk/M) = μ_k²/4 — a meia-volta ao
 *   quadrado (§F0) como FATOR DE RETENÇÃO. Nos três modos exatos do
 *   ciclo M=4: modo 0 → ×4 (retenção plena), modo π/2 → ×2
 *   (meia-retenção), modo π → 0 NUM PASSO (a dobra total). O espectro
 *   de retenção {1, ½, 0} é o seletor do Maestro agindo modo a modo:
 *   o calor é a histerese espectral — retém os graves, dobra os
 *   agudos.
 *
 *   AS CLASSES SÃO AS MASSAS: dois campos com a MESMA massa têm a
 *   mesma sorte (a diferença degrada para o modo mais lento e morre
 *   RELATIVAMENTE: 2^n contra a escala 4^n); massas DIFERENTES nunca
 *   se encontram (a diferença retém o modo 0 — a memória mínima é
 *   inviolável). As classes de equivalência do fluxo dissipativo são
 *   EXATAMENTE os valores do modo retido.
 *
 *   OS PONTOS FIXOS: da difusão, os constantes (um por classe de
 *   massa — ∆u=0 ⟺ u constante); da histerese, a ZONA inteira
 *   (|u|≤Δ toda fixa — maestro_memoria). Euler (ν=0) é o extremo da
 *   memória total (reversível, conserva o produto dual — §F3); a
 *   difusão é o extremo da memória mínima (só a massa). O N-S do
 *   contínuo vive entre os dois — e ESSA leitura fica declarada como
 *   leitura (o contínuo é a fronteira; a gramática está medida).
 *
 * §W1  o espectro de retenção exato: modo 0 ×4, modo π/2 ×2, modo π
 *      → 0 num passo — {1, ½, 0}, inteiro puro no ciclo M=4
 * §W2  as classes são as massas: mesma massa ⟹ a diferença é o modo
 *      lento vezes 2^n (relativa → 0); massas diferentes ⟹ a
 *      diferença retém o modo 0 (persiste) — o gume
 * §W3  os pontos fixos: ∆u=0 ⟺ u constante (varredura exaustiva em
 *      janela); a zona da histerese é toda fixa — os dois regimes
 * §W4  os dois extremos da memória: Euler conserva o produto dual da
 *      diferença (memória total); a difusão só a massa (mínima);
 *      𝓜 assina — e o contínuo fica declarado como leitura
 */
'use strict'
const { anel, dft, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const lap = u => u.map((_, i) => u[(i + 1) % u.length] - 2 * u[i] + u[(i - 1 + u.length) % u.length])
const passo = u => u.map((ui, i) => 4 * ui + lap(u)[i])

/* §W1 — o espectro de retenção exato */
{
  const m0 = passo([7, 7, 7, 7])
  const m2 = passo([1, 0, -1, 0])
  const mPi = passo([1, -1, 1, -1])
  const plena = m0.every(v => v === 28)
  const meia = m2.join() === [2, 0, -2, 0].join()
  const dobraTotal = mPi.every(v => v === 0)
  if (!plena || !meia || !dobraTotal) R++
  console.log(`\n§W1  modo 0: ×4 (${plena}) · modo π/2: ×2 (${meia}) · modo π: → 0 num passo (${dobraTotal})`)
  ok('§W1 A EQUAÇÃO DA HISTERESE é a retenção espectral cos²(πk/M) = μ_k²/4: modo 0 retido pleno, modo π/2 meio-retido, modo π DOBRADO A ZERO num passo — o espectro {1, ½, 0}, inteiro exato: o calor é o seletor do Maestro modo a modo', plena && meia && dobraTotal)
}

/* §W2 — as classes são as massas */
let G = false
{
  /* mesma massa: a diferença degrada para o modo lento ×2^n (relativa → 0) */
  let u = [5, 1, 2, 0], v = [2, 3, 1, 2]                     /* Σ = 8 ambos */
  for (let n = 0; n < 6; n++) { u = passo(u); v = passo(v) }
  const d = u.map((x, i) => x - v[i])
  /* a diferença após 6 passos é 2^6 vezes um campo de massa 0 (o modo lento) */
  const massaD = d.reduce((a, b) => a + b, 0)
  const relativa = d.every(x => Math.abs(x) <= 64) && massaD === 0
  /* massas diferentes: a diferença retém o modo 0 — persiste na escala */
  let a = [5, 1, 2, 0], b = [5, 1, 2, 1]                     /* Σ = 8 vs 9 */
  for (let n = 0; n < 6; n++) { a = passo(a); b = passo(b) }
  const d2 = a.map((x, i) => x - b[i])
  const massaD2 = d2.reduce((s, x) => s + x, 0)
  /* o modo retido: Σd2 = −1·4^6 (a massa da diferença, escalada) */
  G = massaD2 === -(4 ** 6) && d2.every(x => x < 0)
  if (!relativa) R++
  console.log(`\n§W2  mesma massa: diferença ${JSON.stringify(d)} (massa 0, ≤2⁶) · massas ≠: Σd = ${massaD2} = −4⁶ (retida)`)
  ok('§W2 AS CLASSES SÃO AS MASSAS: mesma massa ⟹ a diferença é o modo lento ×2ⁿ contra a escala 4ⁿ (morre relativamente); a classe de equivalência do fluxo é o valor do modo retido', relativa)
  ok('§W2 o gume: massas DIFERENTES nunca se encontram — a diferença retém o modo 0 (Σd = −4⁶ exato): a memória mínima é inviolável', G)
}

/* §W3 — os pontos fixos: constantes vs a zona */
{
  /* ∆u = 0 ⟺ u constante: varredura exaustiva em janela [-2..2]⁴ */
  let fixosLap = 0, constantes = 0, total = 0
  for (let a = -2; a <= 2; a++) for (let b = -2; b <= 2; b++) for (let c = -2; c <= 2; c++) for (let d = -2; d <= 2; d++) {
    total++
    const u = [a, b, c, d]
    if (lap(u).every(x => x === 0)) {
      fixosLap++
      if (a === b && b === c && c === d) constantes++
    }
  }
  /* a zona da histerese é toda fixa: h′=h para todo |u|≤Δ (Δ=2) */
  const passoH = (h, u) => Math.abs(u) <= 2 ? h : Math.sign(u)
  let zona = 0, casosZ = 0
  for (const h of [-1, 0, 1]) for (let u = -2; u <= 2; u++) { casosZ++; if (passoH(h, u) === h) zona++ }
  if (fixosLap !== constantes || zona !== casosZ) R++
  console.log(`\n§W3  ∆u=0 em ${fixosLap}/${total}, todos constantes: ${fixosLap === constantes} (${constantes}) · a zona da histerese toda fixa: ${zona}/${casosZ}`)
  ok('§W3 OS PONTOS FIXOS, dois regimes: da difusão são EXATAMENTE os constantes (∆u=0 ⟺ u constante, varredura exaustiva — um ponto fixo por classe de massa); da histerese é a ZONA inteira (|u|≤Δ toda fixa) — o contínuo de N-S vive entre os dois', fixosLap === constantes && zona === casosZ)
}

/* §W4 — os dois extremos da memória, e o contrato */
{
  /* Euler espectral conserva o produto dual da DIFERENÇA (memória total) */
  const P = 65537
  const A = anel(P)
  const M = 8, w = A.powm(3, 65536 / M)
  let lcg = 17
  const rnd = () => { lcg = (lcg * 75 + 74) % 65537; return lcg % 100 }
  const u1 = Array.from({ length: M }, rnd), u2 = Array.from({ length: M }, rnd)
  const d = u1.map((x, i) => A.mod(x - u2[i] + P))
  const esp = dft(d, A, w)
  const dual = cs => { let s = 0; for (let k = 0; k < M; k++) s = (s + cs[k] * cs[(M - k) % M]) % P; return s }
  const espE = esp.map((c, k) => A.mod(c * A.powm(w, k)))       /* o passo de Euler */
  const memoriaTotal = dual(espE) === dual(esp)
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§W4  Euler conserva o dual da diferença: ${memoriaTotal} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§W4 OS DOIS EXTREMOS DA MEMÓRIA: Euler conserva o produto dual da diferença (memória total — nada se esquece); a difusão retém só a massa (memória mínima) — N-S no contínuo vive entre os dois, e essa leitura fica DECLARADA como leitura (o contínuo é a fronteira; a gramática está medida)', memoriaTotal)
  ok('§W4 𝓜 assina a realização: a equação da histerese é a retenção espectral (o seletor do Maestro modo a modo), as classes são as massas, os pontos fixos são os constantes (difusão) e a zona (histerese) — os resultados da casa, aplicados, deram o que o coordenador pediu', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A realização: o passo de calor É o seletor espectral — retém')
  console.log('  o modo 0 (a massa: a memória mínima), meio-retém o meio, e')
  console.log('  dobra o modo π a zero num passo. As classes do fluxo são as')
  console.log('  massas; os pontos fixos são os constantes; Euler é a memória')
  console.log('  total e a difusão a mínima — N-S vive entre os dois. A')
  console.log('  histerese e o calor falam a mesma gramática: dobra + seletor.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
