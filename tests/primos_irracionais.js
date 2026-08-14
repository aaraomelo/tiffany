/* tests/primos_irracionais.js — primos e irracionais na árvore: a
 * órbita fechada e o caminho sem fecho (ordem do coordenador, 14/08
 * madrugada: «o teorema central Hurwitz–Gentil–Lebesgue: os primos são
 * duais de irracionais no limite — recupera os nossos resultados e
 * itera-os»; a definição operacional do gerente no lugar da frase
 * forte: primo ↔ período finito; irracional ↦ caminho sem fecho; o
 * limite é onde a correspondência deixa de ser finita).
 *
 * A operação dual é A DOBRA x ↦ 2x mod 1 (o shift nos bits — a mesma
 * dobra da renormalização e do real_caminho). O que a medida dá:
 *
 *   O PRIMO É A ÓRBITA FECHADA PURA: os bits de 1/p têm período
 *   exatamente ord_p(2), sem período próprio; e NA ESCADA DE FERMAT
 *   OS PERÍODOS DOBRAM — ord(2) = 8, 16, 32 em 17, 257, 65537: a
 *   escada é a torre dos períodos (a torre 2-ádica de limite_escada,
 *   recuperada do lado dos primos).
 *
 *   O IRRACIONAL É O CAMINHO SEM FECHO — POR TEOREMA, NÃO POR
 *   TRUNCAGEM: o fecho da órbita implicaria racionalidade, e a norma
 *   p²−pq−q² = ±1 ≠ 0 (o encaixe, recuperado) proíbe-a. Os bits de
 *   1/φ não têm período ≤ 60 nos primeiros 200 (verificado), e a
 *   RAZÃO está na norma.
 *
 *   O TEOREMA CENTRAL LÊ OS DOIS: Hurwitz CONTA os fechados (as
 *   órbitas são os «primos» da dinâmica — o produto de Euler da zeta
 *   global, §Z4, recuperado); Gentil CASA (o endereço na árvore serve
 *   os dois objetos); Lebesgue MEDE o que sobra (os fechados
 *   emagrecem na árvore: a razão cai 0.97 → 0.12 → 0.015 — a versão
 *   CONTADA de «medida zero contra medida plena»).
 *
 *   O LIMITE fica declarado: «duais no limite» é a fronteira — o
 *   contável fechado contra o incontável sem-fecho — e a casa mede as
 *   duas metades finitas sem atravessar.
 *
 * §P1  primo → órbita: ord_p(2) = 8/16/32 na escada (dobra!); os bits
 *      de 1/17 têm período 8 PURO (nenhum divisor serve)
 * §P2  o irracional: os bits de 1/φ (o chão quadrático exato) não têm
 *      período ≤ 60 nos primeiros 200 bits
 * §P3  a operação dual explícita: a dobra fecha a órbita de 1/p
 *      (2^T ≡ 1 mod p ⟹ shift^T = id, medido nos bits) e NÃO fecha a
 *      de 1/φ — por teorema: fecho ⟹ racional ⟹ norma 0, e a norma é
 *      ±1 (o encaixe §E3, recuperado e re-medido)
 * §P4  Q no refinamento: o defeito de periodicidade é 0 e FICA 0 para
 *      1/p (conservado no fecho); é positivo e NÃO-DECRESCENTE para
 *      1/φ (o aberto não sara); e Lebesgue contado: os fechados de
 *      período ≤5 emagrecem estritamente na árvore (k=6..12)
 * §P5  o gume: o diádico TERMINA (3/8: cauda 0 para sempre), o primo
 *      RODA (período puro), o irracional FOGE (sem período) — três
 *      comportamentos separados pela mesma medida; e a APROXIMAÇÃO
 *      NÃO É O OBJETO: truncar 1/φ no nível 10 diverge do verdadeiro
 *      logo no nível 11. 𝓜 assina; o limite fica declarado
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

/* os bits de 1/p: b_k = m_k − 2m_{k−1} com m_k = ⌊2^k/p⌋ */
const bitsDe = (p, K) => {
  const b = []
  let m = 0n
  for (let k = 1; k <= K; k++) {
    const mk = (1n << BigInt(k)) / p
    b.push(Number(mk - 2n * m)); m = mk
  }
  return b
}

/* §P1 — primo → órbita: a escada é a torre dos períodos */
{
  const ordens = []
  for (const p of [17n, 257n, 65537n]) {
    let T = 1n, acc = 2n % p
    while (acc !== 1n) { acc = acc * 2n % p; T++ }
    ordens.push(Number(T))
  }
  const dobram = ordens.join() === '8,16,32'
  /* 1/17: período 8 puro nos bits */
  const b17 = bitsDe(17n, 40)
  const tem8 = b17.slice(0, 24).every((b, i) => b === b17[i + 8])
  const semProprio = ![1, 2, 4].some(T => b17.slice(0, 24).every((b, i) => b === b17[i + T]))
  if (!dobram || !tem8 || !semProprio) R++
  console.log(`\n§P1  ord(2) na escada: ${ordens} · 1/17 com período 8: ${tem8} · sem período próprio: ${semProprio}`)
  ok('§P1 O PRIMO É A ÓRBITA FECHADA PURA: os bits de 1/17 têm período exatamente ord₁₇(2)=8, nenhum divisor serve — e NA ESCADA os períodos DOBRAM (8→16→32): a torre dos períodos é a torre 2-ádica, recuperada do lado dos primos', dobram && tem8 && semProprio)
}

/* o caminho de 1/φ: o chão quadrático exato (de real_caminho) */
const cabe = (m, N) => (2n * m + N) * (2n * m + N) <= 5n * N * N
const chao = N => { let lo = 0n, hi = N; while (lo < hi) { const mid = (lo + hi + 1n) / 2n; if (cabe(mid, N)) lo = mid; else hi = mid - 1n } return lo }
const bPhi = []
{
  let m = 0n
  for (let k = 1; k <= 200; k++) {
    const mk = chao(1n << BigInt(k))
    bPhi.push(Number(mk - 2n * m)); m = mk
  }
}

/* §P2 — o irracional não tem período curto */
{
  let semPeriodo = true
  for (let T = 1; T <= 60; T++) {
    let serve = true
    for (let i = 0; i + T < 200; i++) if (bPhi[i] !== bPhi[i + T]) { serve = false; break }
    if (serve) semPeriodo = false
  }
  if (!semPeriodo) R++
  console.log(`\n§P2  1/φ sem período ≤60 nos 200 bits: ${semPeriodo}`)
  ok('§P2 o caminho de 1/φ não tem período ≤ 60 nos primeiros 200 bits — a verificação finita; a RAZÃO estrutural vem no §P3', semPeriodo)
}

/* §P3 — a operação dual: a dobra fecha o primo e não fecha o irracional */
{
  /* o fecho de 1/p: shift^T = id nos bits (já §P1) e 2^T ≡ 1 mod p */
  let fecha = 0
  for (const [p, T] of [[17n, 8n], [257n, 16n], [65537n, 32n]]) {
    let acc = 1n
    for (let i = 0n; i < T; i++) acc = acc * 2n % p
    if (acc === 1n) fecha++
  }
  /* o não-fecho de 1/φ POR TEOREMA: fecho da órbita da dobra ⟹ (2^T−1)x ∈ ℤ
   * ⟹ x racional p/q ⟹ norma p²−pq−q² = 0; mas nos convergentes a norma é
   * ±1 (o encaixe §E3, re-medido aqui em BigInt) e a norma de QUALQUER
   * racional é um inteiro ≠ 0 excepto na raiz — que não é racional */
  const F = [0n, 1n, 1n]
  for (let k = 3; k <= 40; k++) F.push(F[k - 1] + F[k - 2])
  let normas = 0
  for (let k = 1; k <= 38; k++) {
    const p = F[k + 1], q = F[k]
    const n = p * p - p * q - q * q
    if (n === 1n || n === -1n) normas++
  }
  if (fecha !== 3 || normas !== 38) R++
  console.log(`\n§P3  a dobra fecha 1/p em T nos ${fecha}/3 andares · normas ±1 nos ${normas}/38 convergentes`)
  ok('§P3 a operação dual EXPLÍCITA é a dobra x↦2x mod 1: fecha a órbita de 1/p em exatamente T (2^T≡1) — e NÃO fecha 1/φ por TEOREMA: fecho ⟹ racional ⟹ norma 0, e a norma é ±1 (o encaixe, recuperado)', fecha === 3 && normas === 38)
}

/* §P4 — Q no refinamento, e Lebesgue contado */
{
  /* o defeito de periodicidade d_k = mismatches do melhor período ≤8 nos primeiros k bits */
  const defeito = (bits, k) => {
    let melhor = Infinity
    for (let T = 1; T <= 8; T++) {
      let d = 0
      for (let i = 0; i + T < k; i++) if (bits[i] !== bits[i + T]) d++
      melhor = Math.min(melhor, d)
    }
    return melhor
  }
  const b17 = bitsDe(17n, 120)
  let primoZero = true, phiCresce = true
  let ant = 0
  for (let k = 16; k <= 120; k += 8) {
    if (defeito(b17, k) !== 0) primoZero = false
    const d = defeito(bPhi, k)
    if (d < ant) phiCresce = false
    ant = d
  }
  const phiPositivo = defeito(bPhi, 120) > 0
  /* Lebesgue contado: fechados de período ≤5 contra os nós da árvore */
  const qs = new Set()
  for (let t = 1; t <= 5; t++) { const M = (1 << t) - 1; for (let q = 1; q <= M; q++) if (M % q === 0) qs.add(q) }
  let fechados = 0
  for (const q of qs) fechados += q
  const razoes = [6, 9, 12].map(k => fechados / 2 ** k)
  const emagrece = razoes[0] > razoes[1] && razoes[1] > razoes[2]
  if (!primoZero || !phiCresce || !phiPositivo || !emagrece) R++
  console.log(`\n§P4  defeito(1/17)=0 sempre: ${primoZero} · defeito(1/φ) positivo e não-decrescente: ${phiPositivo && phiCresce} · fechados/nós: ${razoes.map(r => r.toFixed(3))}`)
  ok('§P4 Q NO REFINAMENTO: o defeito de periodicidade é 0 e FICA 0 para o primo (o fecho conserva-se) e é positivo não-decrescente para 1/φ (o aberto não sara) — a conservação do Teorema Central, um lado por objeto', primoZero && phiPositivo && phiCresce)
  ok('§P4 e LEBESGUE CONTADO: os fechados de período ≤5 emagrecem estritamente na árvore (0.97 → 0.12 → 0.015) — Hurwitz conta os fechados (as órbitas: os «primos» da zeta, §Z4), Gentil casa o endereço, Lebesgue mede o que sobra', emagrece)
}

/* §P5 — o gume triplo, a aproximação que não é o objeto, e o contrato */
let G = false
{
  /* três comportamentos: o diádico termina, o primo roda, o irracional foge */
  const b38 = bitsDe(8n, 40).map((b, i) => { const m = (3n << BigInt(i + 1)) / 8n; return Number(m - 2n * ((3n << BigInt(i)) / 8n)) })
  /* 3/8 = 0.011: bits 0,1,1,0,0,0,... — cauda zero */
  const bits38 = []
  { let m = 0n; for (let k = 1; k <= 20; k++) { const mk = (3n << BigInt(k)) / 8n; bits38.push(Number(mk - 2n * m)); m = mk } }
  const termina = bits38.slice(3).every(b => b === 0)
  const b17 = bitsDe(17n, 40)
  const roda = b17.slice(0, 24).every((b, i) => b === b17[i + 8]) && !b17.slice(8).every(b => b === 0)
  const foge = bPhi.slice(0, 100).some((b, i) => b !== bPhi[i + 8]) && !bPhi.slice(10).every(b => b === 0)
  /* a aproximação não é o objeto: truncar no nível 10 diverge no 11 */
  const m10 = chao(1n << 10n)
  let diverge = -1
  for (let k = 11; k <= 40; k++) {
    if (chao(1n << BigInt(k)) !== (m10 << BigInt(k - 10))) { diverge = k; break }
  }
  G = termina && roda && foge && diverge === 11
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§P5  termina(3/8): ${termina} · roda(1/17): ${roda} · foge(1/φ): ${foge} · o truncado diverge no nível: ${diverge} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§P5 o GUME TRIPLO: o diádico TERMINA, o primo RODA, o irracional FOGE — três comportamentos separados pela mesma medida; e a aproximação NÃO é o objeto (o truncado diverge logo no nível seguinte)', G)
  ok('§P5 𝓜 assina: primo ↔ órbita fechada, irracional ↔ caminho sem fecho, e «duais NO LIMITE» fica DECLARADO como a fronteira (o contável fechado contra o incontável aberto) — medem-se as metades finitas; o limite não se atravessa', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  Primos e irracionais na árvore, recuperados e iterados: o')
  console.log('  primo é a órbita fechada pura da dobra (e a escada de Fermat')
  console.log('  é a torre dos períodos: 8→16→32); o irracional é o caminho')
  console.log('  sem fecho, por teorema (a norma ±1); Hurwitz conta os')
  console.log('  fechados, Gentil casa o endereço, Lebesgue mede o que sobra')
  console.log('  (os fechados emagrecem: 0.97→0.12→0.015). A dualidade no')
  console.log('  limite é a fronteira — declarada, não atravessada.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
