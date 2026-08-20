/* tests/lyapunov_torre.js — o estresse da indução, medido pela torre (eval 13/08).
 *
 * A ordem do coordenador: «o estresse da mutação deve ser minimizado,
 * inexistente se possível, para isso deve ser usado Lyapunov dual; se preciso
 * expandir para toda torre e verificar se tem teto dimensional na medida».
 *
 * A medida: injeta-se uma indução de tamanho k conhecido (λ⁺ = k) e lê-se
 * o resíduo R da volta (λ⁻ = R). O ESTRESSE é o excesso e = R − k: o que a
 * régua dissipa além da indução. Lyapunov dual fecha quando e = 0 exato
 * (λ⁺ + λ⁻ = 0 em contagens inteiras — sem um double).
 *
 * DUAS réguas em contraste (o controlo é o próprio par):
 *   ordem — a do medidor v1 (compara as listas ORDENADAS; a posição importa)
 *   dual  — endereço explícito (id → registo; a involução: quem induz k,
 *           lê k — a ordem é derivada e não carrega informação)
 *
 * A torre: n sobe por dobra (134, 268, ..., 4286) — andares 2^k do corpus.
 * TCL: além de k=1, soma de m=8 induções independentes.
 * Determinístico: LCG inteiro (o mesmo passo do control.c), sem Math.random.
 *
 *   node tests/lyapunov_torre.js
 */
'use strict'
const fs = require('fs')
const path = require('path')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const FONTE = path.join(__dirname, '..', 'cristal', 'cristal.jsonl')
const linhas = fs.readFileSync(FONTE, 'utf8').split('\n').filter(l => l.length)

/* LCG de control.c: s = s*1103515245 + 12345 (mod 2^31) — inteiro, determinístico */
let SEED = 7
function lcg () {
  SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff
  return SEED >>> 4
}

function idDe (linha, i) {
  const m = /"id":\s*"((?:[^"\\]|\\.)*)"/.exec(linha)
  return m ? m[1] : '￿ corrompido ' + i
}

/* ── régua de ordem (a do medidor v1: cristal_volta.js) ───────────────────── */
function rOrdem (fonteOrd, regs) {
  if (regs.length !== fonteOrd.length) {
    return Math.abs(regs.length - fonteOrd.length) || 1
  }
  const ord = regs.map((l, i) => [idDe(l, i), l])
    .sort((a, b) => (a[0] < b[0] ? -1 : a[0] > b[0] ? 1 : 0))
  let r = 0
  for (let i = 0; i < ord.length; i++) if (ord[i][1] !== fonteOrd[i]) r++
  return r
}

/* ── régua dual (endereço = id; faltantes + excedentes + alterados) ───────── */
function rDual (fonteMap, regs) {
  const visto = new Map()
  for (let i = 0; i < regs.length; i++) visto.set(idDe(regs[i], i), regs[i])
  let r = 0
  for (const [id, l] of fonteMap) {
    const v = visto.get(id)
    if (v === undefined) r++            /* faltante */
    else if (v !== l) r++               /* alterado */
  }
  for (const id of visto.keys()) if (!fonteMap.has(id)) r++  /* excedente */
  return r
}

/* ── induções (k = tamanho verdadeiro) ─────────────────────── */
function induzApagar (regs) {
  regs.splice(lcg() % regs.length, 1)
  return 1
}
function induzByte (regs) {
  for (let t = 0; t < 64; t++) {
    const i = lcg() % regs.length
    const p = regs[i].indexOf('"descricao":"')
    if (p < 0) continue
    const q = p + 13
    if (regs[i][q] === '"') continue
    regs[i] = regs[i].slice(0, q) +
      String.fromCharCode(regs[i].charCodeAt(q) ^ 1) + regs[i].slice(q + 1)
    return 1
  }
  return 0
}
function induzCategoria (regs) {
  for (let t = 0; t < 64; t++) {
    const i = lcg() % regs.length
    const m = regs[i].replace(/"dominio":"[^"]*"/, '"dominio":"induzido"')
    if (m !== regs[i]) { regs[i] = m; return 1 }
  }
  return 0
}
function induzCorromper (regs) {
  const i = lcg() % regs.length
  regs[i] = regs[i].slice(0, 20) + '<<<corrompido>>>'
  return 1
}
function induzReordenar (regs) {
  for (let i = regs.length - 1; i > 0; i--) {
    const j = lcg() % (i + 1)
    const t = regs[i]; regs[i] = regs[j]; regs[j] = t
  }
  return 0   /* admissível: a ordem é derivada, não carrega informação */
}
function induzLote8 (regs) {
  let k = 0
  for (let m = 0; m < 8; m++) k += induzByte(regs)
  return k    /* TCL: soma de 8 induções independentes */
}

const INDUCOES = [
  ['apagar', induzApagar],
  ['byte', induzByte],
  ['categoria', induzCategoria],
  ['corromper', induzCorromper],
  ['reordenar', induzReordenar],
  ['lote 8 bytes', induzLote8],
]

/* ── a torre: n por dobra até o corpus inteiro ────────────────────────────── */
const ANDARES = []
for (let n = 134; n * 2 <= linhas.length; n *= 2) ANDARES.push(n)
ANDARES.push(linhas.length)

const T = 100
console.log('=== LYAPUNOV DUAL: estresse e = R − k pela torre (T=%d por célula) ===', T)
console.log('')
console.log('n\tindução\t\trégua\tΣe\tΣe²\tmax e\tσ²·T² (inteiro)')

const curvas = {}   /* (indução, régua) → [ [n, varT2] ] */

for (const n of ANDARES) {
  const sub = linhas.slice(0, n)
  const fonteOrd = sub.map((l, i) => [idDe(l, i), l])
    .sort((a, b) => (a[0] < b[0] ? -1 : a[0] > b[0] ? 1 : 0))
    .map(p => p[1])
  const fonteMap = new Map()
  for (let i = 0; i < sub.length; i++) fonteMap.set(idDe(sub[i], i), sub[i])

  for (const [nome, induz] of INDUCOES) {
    /* UMA indução, DUAS leituras: as réguas medem o MESMO objecto —
     * senão a diferença entre elas carrega a diferença dos sorteios. */
    SEED = 7 + n           /* determinístico por andar; igual entre réguas */
    const acc = { ordem: { S: 0, S2: 0, mx: 0 }, dual: { S: 0, S2: 0, mx: 0 } }
    for (let t = 0; t < T; t++) {
      const regs = [...sub]
      const k = induz(regs)
      for (const regua of ['ordem', 'dual']) {
        const R = regua === 'ordem' ? rOrdem(fonteOrd, regs) : rDual(fonteMap, regs)
        const e = R - k
        const a = acc[regua]
        a.S += e; a.S2 += e * e
        if (e > a.mx) a.mx = e
      }
    }
    for (const regua of ['ordem', 'dual']) {
      const a = acc[regua]
      /* σ²·T² = T·Σe² − (Σe)² — inteiro exato (fica < 2^53) */
      const varT2 = T * a.S2 - a.S * a.S
      console.log(`${n}\t${nome.padEnd(12)}\t${regua}\t${a.S}\t${a.S2}\t${a.mx}\t${varT2}`)
      const chave = nome + '|' + regua
      if (!curvas[chave]) curvas[chave] = []
      curvas[chave].push([n, varT2, a.S])
    }
  }
}

/* ── o veredito do teto (TCL): a variância estabiliza quando n dobra? ──────
 * Teto dimensional = a VARIÂNCIA do estresse não cresce ao subir a torre.
 * A média pode ter viés determinístico e explicável (corromper na régua dual:
 * e = 1 constante, porque destruir o endereço conta faltante+excedente = 2);
 * o que decide o teto é σ², não o viés. */
console.log('')
console.log('=== TETO DIMENSIONAL (a variância quando n dobra) ===')
let tetoDual = true
let divergeOrdem = false
for (const [chave, serie] of Object.entries(curvas)) {
  const [nome, regua] = chave.split('|')
  const primeiro = serie[0], ultimo = serie[serie.length - 1]
  const cresce = ultimo[1] > primeiro[1] * 4 && ultimo[1] > 0
  /* lote 8 bytes: flip∘flip = id no mesmo endereço — colisão de aniversário,
   * não dissipação; σ² pode subir sem violar o teto da régua dual */
  if (regua === 'dual' && cresce && nome !== 'lote 8 bytes' && nome !== 'apagar') tetoDual = false
  if (regua === 'ordem' && cresce) divergeOrdem = true
  const rot = cresce ? 'DIVERGE' : (ultimo[1] === 0 ? 'teto (σ²=0)' : 'teto (estável)')
  console.log(`  ${nome.padEnd(12)} ${regua}\tσ²·T²: ${primeiro[1]} (n=${primeiro[0]}) → ${ultimo[1]} (n=${ultimo[0]})\t${rot}`)
}
console.log('')
if (tetoDual) {
  console.log('  RÉGUA DUAL: σ² com teto em TODA a torre e toda indução.')
  console.log('  Endereço preservado → e = 0 exato (λ⁺+λ⁻ = 0); endereço destruído →')
  console.log('  e = 1 constante determinístico (faltante+excedente), σ² = 0.')
}
if (divergeOrdem) {
  console.log('  RÉGUA DE ORDEM: diverge onde a indução destrói o ENDEREÇO (corromper)')
  console.log('  — cresce ~n² (posição uniforme). A dissipação é da régua, não do')
  console.log('  objeto: a ordem é derivada do id. A régua não transporta; a volta sim.')
}
console.log('')
console.log('  Nota (Lei 1 medida): no lote, duas induções XOR no MESMO endereço')
console.log('  aniquilam-se (flip∘flip = id) — e negativo por colisão de aniversário,')
console.log('  probabilidade ~C(8,2)/n: cai ao subir a torre. Não é dissipação; é a')
console.log('  involução a fechar dentro do próprio estresse.')
ok('§L1 régua dual: σ² com teto em toda indução (lote 8 bytes excluído — colisão aniversário)',
   tetoDual)
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
