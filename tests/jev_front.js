/* jev_front.js — a cola do E6 em node (fase → bins → campo → selo).
 * E6 é a demonstração no front (jev_contratos.js), não um medidor da construção —
 * o bloco E1–E5 (tests/jev_backends.js, 509/509) já vale a construção. Este script
 * mede a COLA que o navegador segura: a fase em ponto fixo (a MESMA recursão de
 * avancaFase/motor_wasm.js, wrap bit-a-bit) vira a realização por bins, e o MESMO
 * marginal.wasm (marginal/escore) re-conta o campo contra o oráculo JS — tudo sem
 * DOM, com reads determinísticas. Se a cola diverge aqui, diverge na página.
 *
 *   node tests/jev_front.js
 */
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const BASE = 8
const X = 16                                    // as células da realização (o mesmo do front)
const S = [0, 1, 2, 3, 4]
const m = S.length
const LEITURAS = [
  { nome: 'Noul', c: 2, fronteiras: [0, Math.floor(X / 2), X] },
  { nome: 'Choice', c: 4, fronteiras: [0, 4, 8, 12, 16] },
  { nome: 'Score', c: 5, fronteiras: [0, 3, 6, 9, 12, 16] },
]
const cla = (b, c, x) => { for (let i = 0; i < c; i++) { if (x >= b[i] && x < b[i + 1]) return i } return c - 1 }
const eq = (a, b, n) => { for (let i = 0; i < n; i++) if (a[i] !== b[i]) return false; return true }

let falhas = 0, feitas = 0
function ok (q, cond) { feitas++; if (!cond) falhas++; console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`) }

// a fase em ponto fixo 2^20 — a MESMA recursão do avancaFase (branco sem painel):
// fase = (fase + (ω·dt·2^40 + 2^19)>>20) & MASK — resíduo 0, wrap por AND.
function lecturas (omega, n) {
  const ESCALA = 1 << 20, MASK = ESCALA - 1
  const dtF = ESCALA                        // 1 leitura = 1 unidade de tempo
  const omegaF = Math.round(omega * ESCALA)
  const out = []
  let fase = 0n
  for (let t = 0; t < n; t++) {
    const inc = ((BigInt(omegaF) * BigInt(dtF) + (1n << 19n)) >> 20n)
    fase = (fase + inc) & BigInt(MASK)
    out.push(Number(fase) / ESCALA)
  }
  return out
}

function oraculo (G) {
  const out = LEITURAS.map((l) => {
    const gq = new Array(l.c).fill(0)
    for (let x = 0; x < X; x++) gq[cla(l.fronteiras, l.c, x)] += G[x]
    return gq
  })
  let N = 0
  for (let k = 0; k < m; k++) N += S[k] * out[2][k]
  return { out, N }
}

;(async () => {
  const { instance } = await WebAssembly.instantiate(fs.readFileSync(path.join(RAIZ, 'assets', 'figuras', 'wasm', 'jev', 'marginal.wasm')))
  const ex = instance.exports
  const a = new Int32Array(ex.DISCO.buffer, BASE)

  const c0 = 2, c1 = 4, c2 = 5
  const B = X + 5, So = c0 + c1 + c2 + 3, D1 = B + So, PROD = c0 * c1 * c2, J = D1 + 48, D2 = J + PROD, OUT = D2 + 48

  const casos = [
    { omega: 0.05,  n: 300 },    // varredura lenta: as células são alcançadas aos poucos
    { omega: 0.37,  n: 500 },    // pulso fracionário: leituras espalhadas pela volta
    { omega: 0.9999, n: 2000 },  // perto do wrap: uma volta inteira por leitura
    { omega: 1.0,   n: 1000 },   // degene: fase constante 0 — tudo na célula 0
  ]

  for (const caso of casos) {
    const { omega, n } = caso
    const G = new Array(X).fill(0)
    for (const f of lecturas(omega, n)) {
      let x = Math.floor(f * X)
      if (x > X - 1) x = X - 1
      if (x >= 0) G[x]++
    }
    const o = oraculo(G)

    // arena do marginal (a mesma do front e do medidor)
    for (let x = 0; x < X; x++) a[x] = G[x]
    a[X] = n; a[X + 1] = 3
    a[X + 2] = c0; a[X + 3] = c1; a[X + 4] = c2
    let off = X + 5
    for (let i = 0; i < 3; i++) for (let j = 0; j <= LEITURAS[i].c; j++) a[off++] = LEITURAS[i].fronteiras[j]
    ex.marginal(X)

    const id = `omega${omega} n${n}`

    ok(`E6-COL a${id}: total recontado == |I|=${n}`, a[OUT] === n && a[OUT + 1] === n)
    let somaGs = 0
    for (let x = 0; x < X; x++) somaGs += G[x]
    ok(`E6-COL a${id}: ΣG == |I|`, somaGs === n)

    for (let i = 0; i < 3; i++) {
      const c = LEITURAS[i].c
      const d1 = [], d2 = []
      for (let k = 0; k < c; k++) { d1.push(a[D1 + 16 * i + k]); d2.push(a[D2 + 16 * i + k]) }
      ok(`E6-COL a${id} ${LEITURAS[i].nome}: D1 == oráculo`, eq(d1, o.out[i], c))
      ok(`E6-COL a${id} ${LEITURAS[i].nome}: D2 (marginal da conjunta) == oráculo`, eq(d2, o.out[i], c))
      ok(`E6-COL a${id} ${LEITURAS[i].nome}: D1 == D2 (o teorema no front)`, eq(d1, d2, c))
      let s = 0
      for (let k = 0; k < c; k++) s += d1[k]
      ok(`E6-COL a${id} ${LEITURAS[i].nome}: Σ G_q = |I|`, s === n)
    }

    // arena do escore (a mesma do front)
    a[X] = n; a[X + 1] = m
    let o2 = X + 2
    for (let j = 0; j <= m; j++) a[o2++] = LEITURAS[2].fronteiras[j]
    for (let j = 0; j < m; j++) a[o2++] = S[j]
    ex.escore(X)
    const OUTE = o2, N = a[OUTE], D = a[OUTE + 1]

    ok(`E6-COL a${id}: (N,D) == (${o.N}, ${n}) — par exato`, N === o.N && D === n && N * n === o.N * D)
    ok(`E6-COL a${id}: escore emite (N, |I|, m) — os níveis vêm da coordenada Score da marginal (D1, já selada)`, a[OUTE + 2] === m)

    // campo intacto: marginal + escore não recriam G
    let gix = true
    for (let x = 0; x < X; x++) if (a[x] !== G[x]) gix = false
    ok(`E6-COL a${id}: o campo G não é recriado`, gix)
  }

  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => { console.error(e); console.log('#TOTAL', feitas, falhas + 1); process.exit(1) })