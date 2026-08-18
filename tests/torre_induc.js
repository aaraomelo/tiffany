/* torre_induc.js — indução dos tecidos: dim 64, NTT 2048, Gentil.
 *
 *   node tests/torre_induc.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { pathToFileURL } = require('url')
const { instanciaTex } = require('./tex_env.js')

const RAIZ = path.resolve(__dirname, '..')
const WASM = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'tex.wasm')
const FONTE = 'corpus/docs/torre_induc.tex'
const TORRE_NTT_MAX = 4096

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
  console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`)
}
function torreNesp (dim) {
  return dim >= 16 ? Math.min(TORRE_NTT_MAX, dim * 32) : 0
}
function limiteEsqueleto (latin) {
  let cut = latin.length
  for (const m of ['/Type/AssinaturaOito', '/Type/SementeEstrela', '/Type/FonteTeX']) {
    const i = latin.indexOf(m)
    if (i >= 0 && i < cut) cut = i
  }
  return latin.slice(0, cut)
}
function espectroTorre (latin, n) {
  const z = Buffer.from(limiteEsqueleto(latin), 'latin1')
  const A = new Array(n).fill(0)
  let q = 0
  const len = z.length
  const mk = n - 1
  while (q + 26 < len) {
    if (z.slice(q, q + 26).toString('latin1') !== '/Type/XObject/Subtype/Form') { q++; continue }
    let a2 = q
    while (a2 + 7 < len && z.slice(a2, a2 + 7).toString('latin1') !== 'stream\n') a2++
    a2 += 7
    let b2 = a2, k2 = 0
    while (b2 + 9 < len && z.slice(b2, b2 + 9).toString('latin1') !== 'endstream') {
      A[k2 & mk] = (A[k2 & mk] + z[b2]) % 65537
      b2++; k2++
    }
    q = b2 + 9
  }
  let raiz = 1, b3 = 3, e3 = 65536 / n
  while (e3 > 0) { if (e3 & 1) raiz = raiz * b3 % 65537; b3 = b3 * b3 % 65537; e3 >>= 1 }
  const S = new Array(n)
  for (let j = 0; j < n; j++) {
    let acc = 0, w = 1, passo = 1, e4 = j, b4 = raiz
    while (e4 > 0) { if (e4 & 1) passo = passo * b4 % 65537; b4 = b4 * b4 % 65537; e4 >>= 1 }
    for (let t = 0; t < n; t++) { acc = (acc + A[t] * w) % 65537; w = w * passo % 65537 }
    S[j] = acc
  }
  return S
}
function parseSemente (latin) {
  const i = latin.indexOf('/Type/SementeEstrela')
  if (i < 0) return null
  const chunk = latin.slice(i, i + 450)
  const num = (k) => {
    const m = chunk.match(new RegExp('/' + k + '\\s+(\\d+)'))
    return m ? Number(m[1]) : null
  }
  return {
    alcance: num('Alcance'), dim: num('Dim'), induc: num('Induc'),
    lado: num('Lado'), norma: num('Norma'), iface: num('Interface'), piN: num('PiN'),
  }
}
function parseAssinaturaTorre (latin) {
  const i = latin.indexOf('/Type/AssinaturaTorre')
  if (i < 0) return null
  const chunk = latin.slice(i, i + 120)
  const num = (k) => {
    const m = chunk.match(new RegExp('/' + k + '\\s+(\\d+)'))
    return m ? Number(m[1]) : null
  }
  const a = latin.indexOf('[', i)
  const sel = []
  let w = a + 1
  while (w < latin.length && latin[w] !== ']' && sel.length < TORRE_NTT_MAX) {
    while (w < latin.length && latin[w] === ' ') w++
    if (latin[w] === ']') break
    let u = 0
    while (w < latin.length && latin.charCodeAt(w) >= 48 && latin.charCodeAt(w) <= 57) {
      u = u * 10 + (latin.charCodeAt(w) - 48)
      w++
    }
    sel.push(u)
  }
  return { n: num('N'), torreDim: num('TorreDim'), sel }
}

;(async () => {
  if (!fs.existsSync(WASM) || !fs.existsSync(path.join(RAIZ, FONTE))) {
    console.log('wasm ou torre_induc.tex em falta')
    process.exit(1)
  }
  const man = JSON.parse(fs.readFileSync(path.join(RAIZ, 'app', 'src', 'corpo.json'), 'utf8'))
  const disco = await import(pathToFileURL(path.join(RAIZ, 'app', 'src', 'corpo_disco.js')).href)
  const cache = new Map()
  for (const f of man.ficheiros) {
    const p = path.join(RAIZ, f)
    if (fs.existsSync(p)) cache.set(f, fs.readFileSync(p))
  }
  const num = (x) => typeof x === 'bigint' ? Number(x) : x
  const poeSet = new Set()
  let E = null
  const mem = () => new Uint8Array(E.DISCO.buffer)
  function reserva (n) {
    const p = num(E.vfs_reserva(n))
    if (!p) throw new Error('vfs')
    return p
  }
  function poeStr (s) {
    const nb = Buffer.from(s, 'latin1')
    const p = reserva(nb.length + 1)
    mem().set(nb, p); mem()[p + nb.length] = 0
    return p
  }
  function poeFich (nome, bytes) {
    const pN = poeStr(nome)
    const pD = reserva(Math.max(bytes.length, 0) + 1)
    if (bytes.length) mem().set(bytes, pD)
    mem()[pD + bytes.length] = 0
    if (!E.poe_ficheiro(pN, pD, bytes.length)) throw new Error('poe ' + nome)
    poeSet.add(nome)
  }
  const fichMiss = (ptr) => {
    const v = mem()
    let s = ''
    for (let i = ptr; i < v.length && v[i]; i++) s += String.fromCharCode(v[i])
    const can = disco.resolveCorpoNome(s, cache)
    if (!can || !cache.has(can)) return 0
    if (poeSet.has(can)) return 1
    poeFich(can, cache.get(can))
    return 1
  }
  E = instanciaTex(fs.readFileSync(WASM), fichMiss).exports
  E.inicia_wasm()
  for (const f of disco.ficheirosPara(FONTE, man.ficheiros)) {
    if (cache.has(f)) poeFich(f, cache.get(f))
  }
  poeFich(FONTE, fs.readFileSync(path.join(RAIZ, FONTE)))
  if (typeof E.marca_vfs === 'function') E.marca_vfs()
  E.limpa_saida()
  const rc = num(E.compila_ficheiro(poeStr(FONTE), poeStr('torre.pdf')))
  const tam = num(E.tam_saida())
  const end = num(E.MOVE(14, 1))
  const latin = Buffer.from(mem().slice(end, end + tam)).toString('latin1')
  const sem = parseSemente(latin)
  const torre = parseAssinaturaTorre(latin)
  const dim = sem && sem.dim
  const nesp = torreNesp(dim)
  let bate = false
  if (torre && torre.sel && torre.n === nesp) {
    const S = espectroTorre(latin, torre.n)
    bate = S.length === torre.sel.length && S.every((v, i) => v === torre.sel[i])
  }
  console.log('=== TORRE INDUÇÃO (tecidos T+T*) ===\n')
  console.log(`   rc=${rc} tam=${tam} alc=${sem && sem.alcance} dim=${dim} if=${sem && sem.iface}` +
    ` L=${sem && sem.lado} ind=${sem && sem.induc} NTT=${torre && torre.n} bate=${bate}`)
  ok('§I1 compila corpus/docs/torre_induc.tex', rc === 0 && tam > 1000 && latin.includes('%%EOF'))
  ok('§I2 indução: alcance 6, dim 128, /Induc viaja', sem && sem.alcance === 6 && sem.dim === 128 && sem.induc === 6)
  ok('§I3 Gentil: Lado 1, Norma 1, interface 24 (2.ª estrela)', sem && sem.lado === 1 && sem.norma === 1 && sem.iface === 24)
  ok('§I4 selo complementar N=min(4096,dim·32)=4096', torre && torre.n === 4096 && torre.torreDim === 128)
  ok('§I5 NTT recomputado bate o escrito (dois caminhos)', bate)
  console.log(`\n#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => { console.error(e); process.exit(1) })
