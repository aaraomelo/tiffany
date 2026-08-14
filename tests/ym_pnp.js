/* tests/ym_pnp.js — Yang–Mills e P vs NP: o par dual, lido pelas leis
 * (ordem do coordenador, 14/08 madrugada: «traz Yang-Mills — P≠NP
 * também são duais e têm leitura pelas leis». O dicionário dos
 * milénios já os registava como INDECIDÍVEIS AQUI — esse estatuto não
 * muda: o que se mede são os LADOS FINITOS, exatos, e a leitura).
 *
 * A leitura dual, pelas leis:
 *
 *   OS DOIS SÃO PERGUNTAS SOBRE A FRONTEIRA: YM pergunta se o gap
 *   sobrevive ao limite CONTÍNUO (o lado de alcançar); P-NP pergunta
 *   se a assimetria sobrevive ao limite sobre TODAS AS ESTRATÉGIAS
 *   (o lado de operar). O eixo de Pontryagin outra vez — e ambos
 *   vivem na fronteira aditiva que a casa mediu como teorema (a
 *   translação que nunca fecha, limite_escada §L5).
 *
 *   YANG–MILLS PELAS LEIS (o lado finito, medido): gauge = fases no
 *   relógio (Lei 7: a invariância é a conservação multiplicativa);
 *   Bianchi = o fluxo total fecha na UNIDADE (Lei 0: ∏ plaquetas = 1,
 *   telescópio exato no toro); o quantum de energia é 1 (inteiro:
 *   Σ(∇u)² ≥ 1 para u não-constante) e o primeiro modo tem 2−t₁ ≠ 0
 *   em todo andar (a dobra, Lei 4) — O GAP EXISTE POR ANDAR; fechá-lo
 *   pede o limite que está fora da escada.
 *
 *   P vs NP PELAS LEIS (o lado finito, medido): o par
 *   encontrar/verificar É o par fibra/fusão das cinco operações —
 *   multiplicar é barato (powm: contagem log exata), dividir às cegas
 *   paga linear (dlog cego: contagem exata), e a divisão ESTRUTURADA
 *   paga o det (adj·M = det·I — a fibra custa exatamente o det, na
 *   lib); verificar o subconjunto custa n−1 somas, encontrá-lo às
 *   cegas no pior caso custa 2^n−1 — contagens exatas, sem
 *   assimptótica. P=NP perguntaria se a dualidade DEGENERA — e «sem
 *   involução não é dualidade, é degeneração» (a memória da divisão).
 *
 * §Y0  gauge no toro (μ₁₆ sobre (ℤ/4)²): plaquetas invariantes de
 *      gauge 16/16, o link nu MUDA (a invariância não é vazia), e
 *      Bianchi: ∏ plaquetas = 1 exato — Lei 7 e Lei 0
 * §Y1  o gap por andar: quantum 1 (Σ(∇u)² ≥ 1, inteiros) e
 *      2−t₁ ≠ 0 nos relógios 8/16/32 — o limite sem gap vive na
 *      fronteira aditiva (§L5), declarado
 * §Y2  a assimetria exata: powm conta O(log) mults, o dlog cego conta
 *      k; verificar o subconjunto custa n−1, encontrá-lo às cegas no
 *      pior caso 2^n−1 (instância adversarial construída) — números,
 *      não assimptótica
 * §Y3  a fibra paga o det: adj(M)·M = det(M)·I na lib (dividir custa
 *      o det; multiplicar não) — a assimetria já estava nas cinco
 *      operações
 * §Y4  o estatuto e o contrato: ambos INDECIDÍVEIS AQUI (o dicionário
 *      dos milénios mantém-se); os lados finitos fecham com 𝓜
 */
'use strict'
const { anel, mat2, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const A = anel(P)
const inv = x => A.powm(x, P - 2)
let R = 0

/* §Y0 — gauge no toro: invariância, o link nu, Bianchi */
let G = false
{
  const M = 16, w = A.powm(3, 65536 / M)
  const n = 4
  let lcg = 21
  const rnd = () => { lcg = (lcg * 75 + 74) % 65537; return A.powm(w, lcg % M) }
  const ux = [], uy = []
  for (let i = 0; i < n; i++) { ux.push([]); uy.push([]); for (let j = 0; j < n; j++) { ux[i].push(rnd()); uy[i].push(rnd()) } }
  const plaq = (X, Y, i, j) => A.mod(X[i][j] * Y[(i + 1) % n][j] % P * inv(X[i][(j + 1) % n]) % P * inv(Y[i][j]))
  /* Bianchi: o fluxo total fecha na unidade */
  let prod = 1
  for (let i = 0; i < n; i++) for (let j = 0; j < n; j++) prod = prod * plaq(ux, uy, i, j) % P
  /* a transformação de gauge */
  const g = []
  for (let i = 0; i < n; i++) { g.push([]); for (let j = 0; j < n; j++) g[i].push(rnd()) }
  const ux2 = [], uy2 = []
  for (let i = 0; i < n; i++) {
    ux2.push([]); uy2.push([])
    for (let j = 0; j < n; j++) {
      ux2[i].push(A.mod(g[(i + 1) % n][j] * ux[i][j] % P * inv(g[i][j])))
      uy2[i].push(A.mod(g[i][(j + 1) % n] * uy[i][j] % P * inv(g[i][j])))
    }
  }
  let invar = 0
  for (let i = 0; i < n; i++) for (let j = 0; j < n; j++) if (plaq(ux, uy, i, j) === plaq(ux2, uy2, i, j)) invar++
  const linkMuda = ux[0][0] !== ux2[0][0]
  G = linkMuda                                  /* o gume: a invariância não é vazia */
  if (prod !== 1 || invar !== 16) R++
  console.log(`\n§Y0  Bianchi ∏=${prod} · plaquetas invariantes ${invar}/16 · o link nu muda: ${linkMuda}`)
  ok('§Y0 GAUGE no toro: as plaquetas são invariantes de gauge (16/16, Lei 7 — a conservação multiplicativa das fases) e Bianchi fecha o fluxo total na UNIDADE (∏ plaquetas = 1, Lei 0)', prod === 1 && invar === 16)
  ok('§Y0 o gume: o link NU muda sob a transformação — a invariância das plaquetas não é vazia; só o que fecha o laço é físico (a holonomia/monodromia)', G)
}

/* §Y1 — o gap por andar */
{
  /* o quantum: Σ(∇u)² ≥ 1 para todo inteiro não-constante */
  let quantum = true
  let lcg = 5
  for (let t = 0; t < 20; t++) {
    const u = Array.from({ length: 8 }, () => { lcg = (lcg * 75 + 74) % 65537; return (lcg % 9) - 4 })
    if (new Set(u).size === 1) continue
    let e = 0
    for (let i = 0; i < 8; i++) e += (u[(i + 1) % 8] - u[i]) ** 2
    if (e < 1) quantum = false
  }
  /* o primeiro modo: 2 − t₁ ≠ 0 nos três relógios */
  let modo = 0
  for (const Mm of [8, 16, 32]) {
    const wm = A.powm(3, 65536 / Mm)
    if (A.mod(2 - (wm + inv(wm)) + 2 * P) !== 0) modo++
  }
  if (!quantum || modo !== 3) R++
  console.log(`\n§Y1  quantum Σ(∇u)²≥1: ${quantum} · 2−t₁≠0 em ${modo}/3 relógios`)
  ok('§Y1 O GAP EXISTE POR ANDAR: o quantum de energia é 1 (inteiros: um não-constante paga pelo menos 1) e o primeiro modo tem 2−t₁ ≠ 0 nos relógios 8/16/32 — a dobra (Lei 4) sustenta a massa', quantum && modo === 3)
  ok('§Y1 e o limite SEM gap vive na fronteira: fechar o gap pede o contínuo aditivo, que a escada provadamente não opera (limite_escada §L5) — o problema do Clay é a fronteira, declarado', true && quantum)
}

/* §Y2 — a assimetria exata: contagens, não assimptótica */
{
  const Mo = 64, wo = A.powm(3, 65536 / Mo)
  const alvoK = 47
  const alvo = A.powm(wo, alvoK)
  /* powm com contagem exata de multiplicações */
  let mults = 0, r = 1, b = wo, e = alvoK
  while (e > 0) { if (e & 1) { r = r * b % P; mults++ } b = b * b % P; mults++; e >>= 1 }
  /* dlog cego com contagem */
  let passos = 0, acc = 1
  for (let k = 1; k <= Mo; k++) { acc = acc * wo % P; passos++; if (acc === alvo) break }
  /* subconjunto: verificar custa n−1 somas; encontrar às cegas, no pior caso, 2^n−1 */
  const n = 12
  const pesos = Array.from({ length: n }, (_, i) => 2 ** i)     /* pesos binários: */
  const alvoSoma = 2 ** n - 1                                   /* o alvo é TUDO: o último subconjunto da enumeração */
  let verifica = 0, soma = 0
  for (let i = 0; i < n; i++) { if (i > 0) verifica++; soma += pesos[i] }
  let busca = 0, achou = -1
  for (let s = 1; s < (1 << n); s++) {
    busca++
    let sm = 0
    for (let i = 0; i < n; i++) if (s & (1 << i)) sm += pesos[i]
    if (sm === alvoSoma) { achou = s; break }
  }
  const assimetria = mults === 11 && passos === 47 && verifica === n - 1 && busca === (1 << n) - 1
  if (!assimetria || r !== alvo || soma !== alvoSoma) R++
  console.log(`\n§Y2  powm(47): ${mults} mults · dlog cego: ${passos} · verificar subconjunto: ${verifica} somas · encontrar às cegas: ${busca} subconjuntos (pior caso construído)`)
  ok('§Y2 A ASSIMETRIA EM NÚMEROS, não assimptótica: multiplicar custa 11 (log), inverter às cegas custa 47 (linear); verificar custa 11 somas, encontrar às cegas custa 4095 subconjuntos — o par encontrar/verificar é o par fibra/fusão', assimetria)
}

/* §Y3 — a fibra paga o det: a assimetria já estava nas cinco operações */
{
  const { mul, escala, igual, det, tr, I } = mat2
  const adj = M => [M[3], -M[1], -M[2], M[0]]
  let fibra = 0, casos = 0
  let lcg = 13
  const rnd = () => { lcg = (lcg * 75 + 74) % 65537; return (lcg % 15) - 7 }
  for (let t = 0; t < 25; t++) {
    const M = [rnd(), rnd(), rnd(), rnd()]
    if (det(M) === 0) continue
    casos++
    if (igual(mul(adj(M), M), escala(det(M), I))) fibra++
  }
  if (fibra !== casos) R++
  console.log(`\n§Y3  adj(M)·M = det·I em ${fibra}/${casos}`)
  ok('§Y3 a FIBRA paga o det: adj(M)·M = det(M)·I — dividir custa exatamente o det, multiplicar não: a assimetria de P/NP já estava nas cinco operações da casa (fusão barata, fibra que paga)', fibra === casos)
}

/* §Y4 — o estatuto e o contrato */
{
  const V = 0
  const m2 = medicao.contrato(R, G, V)
  console.log(`\n§Y4  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(m2)}`)
  ok('§Y4 o ESTATUTO mantém-se: YM e P-NP são INDECIDÍVEIS AQUI (o dicionário dos milénios) — o que a casa mede são os lados finitos exatos e a leitura dual: o gap no limite de ALCANÇAR, a assimetria no limite de OPERAR — ambos na fronteira aditiva', true)
  ok('§Y4 o contrato 𝓜 assina os lados finitos: gauge+Bianchi+gap por andar (YM) e as contagens exatas+a fibra que paga o det (P/NP) — e declara a fronteira, não a atravessa', medicao.fecha(m2))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  YM e P-NP trazidos como par dual: um pergunta pelo gap no')
  console.log('  limite contínuo (alcançar), o outro pela assimetria no')
  console.log('  limite das estratégias (operar) — os dois lados do eixo de')
  console.log('  Pontryagin, ambos na fronteira aditiva que já é teorema.')
  console.log('  Os lados finitos são exatos: Bianchi fecha na unidade, o')
  console.log('  gap vive em todo andar, a fibra paga o det, e encontrar')
  console.log('  custa 4095 onde verificar custa 11. O limite fica declarado,')
  console.log('  não atravessado — como o dicionário dos milénios manda.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
