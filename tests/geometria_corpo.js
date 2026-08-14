/* tests/geometria_corpo.js — a geometria trazida: o Klein da casa
 * (ordem do coordenador, 14/08 madrugada: «trazer geometria» — e a
 * pergunta aberta da mesa: «que estrutura as operações determinam?»).
 *
 * A resposta é geométrica e mede-se: A GEOMETRIA É O PAR (FORMA, GRUPO).
 *
 *   O GRUPO DO NÚCLEO É D₄: {±I, ±J, ±S, ±X} fecha por produto
 *   (64/64), J tem ordem 4, S ordem 2, SJS = J⁻¹ — o diedral do
 *   quadrado, graduado pelo det (4 rotações +1, 4 reflexões −1).
 *
 *   E É A INTERSEÇÃO DAS DUAS GEOMETRIAS: os 8 preservam o círculo
 *   x²+y² EXATAMENTE e a hipérbole diagonal x²−y² A MENOS DE SINAL —
 *   e o sinal é um SEGUNDO caráter do grupo. Os dois caracteres
 *   (det, sinal-D) separam os 8 em 4 classes: a abelianização
 *   ℤ/2 × ℤ/2, medida elemento a elemento.
 *
 *   A CARTA W DIAGONALIZA A FORMA METÁLICA:
 *   4N(a,b) = (2a+mb)² − (m²+4)b² — a forma de traço do produto dual
 *   (§I3) lida como mudança de carta; e NELA A ESTACA É O ESPELHO:
 *   a conjugação (a,b) ↦ (a+mb, −b) vira (u,v) ↦ (u, −v). A álgebra
 *   e a geometria são a MESMA involução em cartas diferentes.
 *
 *   CADA MEMBRANA ESTENDE O NÚCLEO COM O SEU FLUXO: o relógio
 *   (rotações do círculo, compacto, transitivo) e o Pell (σ^k, com
 *   N a alternar (−1)^k — o mesmo sinal do encaixe). E os fluxos NÃO
 *   se trocam: σ quebra o círculo, a rotação genérica quebra a
 *   hipérbole — as geometrias não se confundem.
 *
 * §G0  o grupo: fecho 64/64, ordens, relação diedral, graduação det
 * §G1  a interseção: os 8 preservam x²+y² exato e x²−y² a menos de
 *      sinal; os dois caracteres separam em 4 classes de 2
 * §G2  a carta W: 4N = (2a+mb)² − (m²+4)b² (identidade em m=1..6,
 *      amostra), e a estaca vira o espelho na carta
 * §G3  os fluxos: o relógio preserva o círculo e é transitivo (2N
 *      rotações); o Pell alterna N com (−1)^k (BigInt, k=1..20);
 *      gume: σ quebra o círculo e a rotação genérica quebra a
 *      hipérbole
 * §G4  Klein da casa: o contrato 𝓜 assina — a estrutura que as
 *      operações determinam é o par de geometrias com núcleo comum
 */
'use strict'
const { anel, mat2, nucleo, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const { mul, igual, det, I, J, espelho } = mat2
const X = nucleo.X
const neg = M => M.map(v => -v)
const G8 = [I, J, neg(I), neg(J), espelho, X, neg(espelho), neg(X)]
const indice = M => { for (let i = 0; i < 8; i++) if (igual(M, G8[i])) return i; return -1 }
let R = 0

/* §G0 — o grupo do núcleo é D₄ */
{
  let fecho = 0
  for (const A2 of G8) for (const B2 of G8) if (indice(mul(A2, B2)) >= 0) fecho++
  const ordens = igual(mul(mul(J, J), mul(J, J)), I) && igual(mul(espelho, espelho), I)
  const diedral = igual(mul(mul(espelho, J), espelho), neg(J))
  const dets = G8.map(det)
  const graduado = dets.slice(0, 4).every(d => d === 1) && dets.slice(4).every(d => d === -1)
  if (fecho !== 64 || !ordens || !diedral || !graduado) R++
  console.log(`\n§G0  fecho ${fecho}/64 · J⁴=S²=I: ${ordens} · SJS=J⁻¹: ${diedral} · dets: ${dets.join()}`)
  ok('§G0 o grupo do núcleo é D₄: {±I,±J,±S,±X} fecha (64/64), J ordem 4, S ordem 2, SJS=J⁻¹ — graduado pelo det (4 rotações, 4 reflexões)', fecho === 64 && ordens && diedral && graduado)
}

/* §G1 — a interseção das duas geometrias, e os dois caracteres */
{
  const age = (M, [a, b]) => [M[0] * a + M[1] * b, M[2] * a + M[3] * b]
  const C = ([a, b]) => a * a + b * b
  const D = ([a, b]) => a * a - b * b
  const amostra = [[3, 5], [-2, 7], [1, 0], [4, -9], [6, 6]]
  let circulo = 0, hiperbole = 0, casos = 0
  const sinais = []
  for (const g of G8) {
    let sg = null, coerente = true
    for (const v of amostra) {
      casos++
      const w = age(g, v)
      if (C(w) === C(v)) circulo++
      const s = D(v) !== 0 ? D(w) / D(v) : null
      if (s !== null) {
        if (sg === null) sg = s
        else if (sg !== s) coerente = false
      }
      if (s === 1 || s === -1 || D(v) === 0) hiperbole++
    }
    sinais.push(coerente ? sg : '?')
  }
  /* os dois caracteres (det, sinal-D) separam os 8 em 4 classes de 2 */
  const classes = new Set(G8.map((g, i) => det(g) + '/' + sinais[i]))
  if (circulo !== casos || hiperbole !== casos || classes.size !== 4) R++
  console.log(`\n§G1  círculo exato: ${circulo}/${casos} · hipérbole ±: ${hiperbole}/${casos} · sinais-D: ${sinais.join()} · classes (det,sinal): ${classes.size}`)
  ok('§G1 o núcleo é a INTERSEÇÃO: os 8 preservam x²+y² exatamente e x²−y² a menos de sinal — e o sinal é coerente por elemento (um caráter)', circulo === casos && hiperbole === casos && !sinais.includes('?'))
  ok('§G1 os dois caracteres (det, sinal-D) separam D₄ em 4 classes de 2 — a abelianização ℤ/2×ℤ/2, medida', classes.size === 4)
}

/* §G2 — a carta W diagonaliza, e a estaca é o espelho */
{
  let diagonaliza = 0, casos = 0, estacaEspelho = 0
  for (let m = 1; m <= 6; m++) {
    for (const [a, b] of [[3, 5], [-2, 7], [1, 0], [4, -9], [0, 1], [11, -3]]) {
      casos++
      const N = a * a + m * a * b - b * b
      const u = 2 * a + m * b, v = b
      if (4 * N === u * u - (m * m + 4) * v * v) diagonaliza++
      /* a estaca: (a,b) ↦ (a+mb, −b); na carta W: u' = 2(a+mb)−mb = u, v' = −b */
      const a2 = a + m * b, b2 = -b
      const u2 = 2 * a2 + m * b2, v2 = b2
      if (u2 === u && v2 === -v) estacaEspelho++
    }
  }
  if (diagonaliza !== casos || estacaEspelho !== casos) R++
  console.log(`\n§G2  4N = u²−(m²+4)v²: ${diagonaliza}/${casos} · estaca → (u,−v): ${estacaEspelho}/${casos}`)
  ok('§G2 a carta W diagonaliza a forma metálica: 4N(a,b) = (2a+mb)² − (m²+4)b² — a forma de traço do produto dual como mudança de carta', diagonaliza === casos)
  ok('§G2 e NELA a estaca É o espelho: a conjugação (a,b)↦(a+mb,−b) vira (u,v)↦(u,−v) — álgebra e geometria, a mesma involução em cartas diferentes', estacaEspelho === casos)
}

/* §G3 — os fluxos próprios, e o gume da confusão */
let G = false
{
  const P = 65537
  const A = anel(P)
  const M = 32
  const h = A.powm(3, 65536 / M), i4 = A.powm(3, 16384), i2 = A.inv(2)
  /* o relógio: rotações carta(c_k,s_k) preservam o círculo e são transitivas */
  let preserva = 0, transitivo = true
  const pontos = []
  for (let k = 0; k < M; k++) {
    const hk = A.powm(h, k), hki = A.inv(hk)
    const c = A.mod((hk + hki) * i2)
    const s = A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4))
    pontos.push([c, s])
  }
  for (let k = 0; k < M; k++) {
    const [c, s] = pontos[k]
    const [x, y] = pontos[3]                       /* um ponto do relógio */
    const rx = A.mod(c * x + s * y), ry = A.mod(-s * x + c * y + P * P)
    if (A.mod(rx * rx + ry * ry) === 1) preserva++
  }
  /* transitividade: a órbita do ponto (1,0) sob as M rotações é o relógio inteiro */
  const orbita = new Set()
  for (let k = 0; k < M; k++) {
    const [c, s] = pontos[k]
    orbita.add(A.mod(c * 1 + s * 0) + ',' + A.mod(-s * 1 + c * 0 + P))
  }
  transitivo = orbita.size === M
  /* o Pell: N alterna (−1)^k, BigInt */
  const m = 2n
  const Nb = (a, b) => a * a + m * a * b - b * b
  const sigma = ([a, b]) => [b, a + m * b]
  let alterna = 0
  let w = [3n, 5n]
  const N0 = Nb(3n, 5n)
  for (let k = 1; k <= 20; k++) {
    w = sigma(w)
    if (Nb(w[0], w[1]) === (k % 2 === 0 ? N0 : -N0)) alterna++
  }
  /* o gume: σ quebra o círculo; a rotação genérica quebra a hipérbole diagonal */
  const Cn = ([a, b]) => a * a + b * b
  const vs = [5n, 3n + m * 5n]                     /* σ·(3,5) */
  const quebraCirculo = Cn(vs) !== Cn([3n, 5n])
  const [c5, s5] = pontos[5]
  const Dv = (a, b) => A.mod(a * a - b * b + P * P)
  const gx = A.mod(c5 * 3 + s5 * 5), gy = A.mod(-s5 * 3 + c5 * 5 + P * P)
  const quebraHiperbole = Dv(gx, gy) !== Dv(3, 5) && Dv(gx, gy) !== A.mod(-(3 * 3 - 5 * 5) + P)
  G = quebraCirculo && quebraHiperbole
  if (preserva !== M || !transitivo || alterna !== 20) R++
  console.log(`\n§G3  rotações preservam o círculo: ${preserva}/${M} · transitivo: ${transitivo} · Pell alterna (−1)^k: ${alterna}/20 · σ quebra círculo: ${quebraCirculo} · rotação quebra hipérbole: ${quebraHiperbole}`)
  ok('§G3 cada membrana estende o núcleo com o SEU fluxo: o relógio (compacto, transitivo, preserva o círculo) e o Pell (N alterna (−1)^k — o sinal do encaixe)', preserva === M && transitivo && alterna === 20)
  ok('§G3 o gume: σ quebra o círculo e a rotação genérica quebra a hipérbole — os fluxos não se trocam, as geometrias não se confundem', G)
}

/* §G4 — Klein da casa: o contrato assina */
{
  const V = 0                                     /* a volta: G8 é grupo — cada elemento tem inverso no conjunto */
  let inversos = 0
  for (const g of G8) {
    for (const h2 of G8) if (igual(mul(g, h2), I)) { inversos++; break }
  }
  const m = medicao.contrato(R + (inversos === 8 ? 0 : 1), G, V)
  console.log(`\n§G4  inversos no conjunto: ${inversos}/8 · 𝓜 = (R=${R}, G=${m.G}, V=${V}) → fecha: ${medicao.fecha(m)}`)
  ok('§G4 a volta: cada elemento de D₄ tem o inverso DENTRO do grupo — a geometria fecha sobre si', inversos === 8)
  ok('§G4 o contrato 𝓜 assina o Klein da casa: a estrutura que as operações determinam é o PAR de geometrias (círculo compacto, hipérbole de Pell) com o núcleo D₄ por interseção', medicao.fecha(m))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A geometria trazida: as operações do núcleo determinam D₄ —')
  console.log('  a interseção exata das duas geometrias. O círculo estende-o')
  console.log('  com o relógio (compacto); a hipérbole com o Pell (o sinal do')
  console.log('  encaixe); a carta W diagonaliza e nela a estaca É o espelho.')
  console.log('  A geometria é o par (forma, grupo) — e o par tem duas formas')
  console.log('  e um só núcleo: a resposta à pergunta da mesa.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
