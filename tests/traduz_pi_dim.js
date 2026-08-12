/* traduz_pi_dim.js — DIMENSÃO e π de CADA tradução .tex→PDF.
 *
 * Teorema do Metrónomo (Corpo de Peano): maestro projecta; metrónomo lê;
 * Lyapunov dualizado atesta. Por documento: o inversor sobe (espiral); a Dim =
 * 2^(alcance+1) viaja na /SementeEstrela; π_n sai da dobra (Hurwitz=dobra
 * discreta, Gentil=dual); Lyapunov dualizado = FonteTeX de volta, bits da diferença.
 *
 *   node tests/traduz_pi_dim.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { instanciaTex } = require('./tex_env.js')
const { pathToFileURL } = require('url')

const RAIZ = path.resolve(__dirname, '..')
const WASM = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'tex.wasm')
const docsJson = JSON.parse(fs.readFileSync(path.join(RAIZ, 'app/src/docs_tradutor.json'), 'utf8'))
const DOCS = { ...docsJson.docs, ...(docsJson.so_teste || {}) }

const PI = Math.PI

function isqrt (x) {
  x = BigInt(x)
  if (x < 2n) return x
  let g = x, h = (g + 1n) / 2n
  while (h < g) { g = h; h = (g + x / g) / 2n }
  return g
}
function piFixo (q) {
  const S = 1 << 30
  q = BigInt(q)
  let c = -BigInt(S), m2 = 2n
  while (m2 < q) {
    c = isqrt(((BigInt(S) + c) / 2n) * BigInt(S))
    m2 *= 2n
  }
  const s = isqrt(((BigInt(S) - c) / 2n) * BigInt(S))
  return Number(q * s) / S
}

function log2dim (n) {
  let e = 0
  while (n > 1) { e++; n >>= 1 }
  return e
}
function parseRegua (v) {
  if (v == null) return { reguaL: null, reguaC: null }
  return { reguaL: Math.floor(v / 100), reguaC: v % 100 }
}
function reguaCap (alc) {
  return 12 + 6 * Math.floor((alc || 0) / 3)
}
function reguaL (alc) {
  return 30
}
function piGentilFixo (dim, alc) {
  let iters = dim <= 1 ? 1 : log2dim(dim) + dim - 2
  const cap = reguaCap(alc)
  if (iters > cap) iters = cap
  const S = 1 << 30
  let sq = S
  for (let s = 0; s < iters; s++) {
    const sq2 = Math.floor((sq * sq) / S)
    let inner = S - sq2
    inner = Number(isqrt(BigInt(inner) * BigInt(S)))
    const ux = (2 * S + 2 * inner) * S
    const g = Number(isqrt(BigInt(ux)))
    sq = Math.floor((sq * S) / g)
  }
  let num = BigInt(sq) * 1000000000n
  for (let i = 0; i <= iters; i++) num <<= 1n
  return Number(num / BigInt(S)) / 1e9
}
function qDim (n) { return n * (2 ** (n - 1)) }
function medidaPi (dim, alc) {
  const q = qDim(dim)
  const c = q / 2
  let pi, hurwitz = true
  if (dim >= 16 || q > (1 << 20)) {
    hurwitz = false
    pi = piGentilFixo(dim, alc)
  } else {
    pi = piFixo(q)
  }
  const err = Math.abs(pi - PI)
  const bits = err > 0 ? -Math.log2(err) : Infinity
  const fecha = err * c < 0.5
  return { q, c, pi, err, bits, fecha, hurwitz }
}

function parseSemente (latin) {
  const i = latin.indexOf('/Type/SementeEstrela')
  if (i < 0) return null
  const chunk = latin.slice(i, i + 400)
  const num = (k) => {
    const m = chunk.match(new RegExp('/' + k + '\\s+(\\d+)'))
    return m ? Number(m[1]) : null
  }
  const regua = num('Regua')
  const { reguaL, reguaC } = parseRegua(regua)
  return {
    resp: num('Resp'), ascN: num('AscN'), ascD: num('AscD'), desc: num('Desc'),
    alcance: num('Alcance'), dim: num('Dim'), induc: num('Induc'),
    lado: num('Lado'), norma: num('Norma'), iface: num('Interface'),
    regua, reguaL, reguaC, piN: num('PiN'),
  }
}
function parseAssinatura (latin) {
  const i = latin.indexOf('/Type/AssinaturaOito')
  if (i < 0) return null
  const chunk = latin.slice(i, i + 120)
  const num = (k) => {
    const m = chunk.match(new RegExp('/' + k + '\\s+(\\d+)'))
    return m ? Number(m[1]) : null
  }
  return { n: num('N'), torreDim: num('TorreDim'), torreN: num('TorreN') }
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
  if (a < 0) return { n: num('N'), torreDim: num('TorreDim'), sel: null }
  const sel = []
  let w = a + 1
  const dig = (w) => latin.charCodeAt(w) - 48
  while (w < latin.length && latin[w] !== ']' && sel.length < TORRE_NTT_MAX) {
    while (w < latin.length && latin[w] === ' ') w++
    if (latin[w] === ']') break
    let u = 0
    while (w < latin.length && latin.charCodeAt(w) >= 48 && latin.charCodeAt(w) <= 57) {
      u = u * 10 + dig(w)
      w++
    }
    sel.push(u)
  }
  return { n: num('N'), torreDim: num('TorreDim'), sel }
}
const TORRE_NTT_MAX = 4096
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
  const latin0 = limiteEsqueleto(latin)
  const A = new Array(n).fill(0)
  let q = 0
  const z = Buffer.from(latin0, 'latin1')
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
function extraiFonteTeX (buf) {
  const latin = buf.toString('latin1')
  const m = /\/Type\/FonteTeX\/Length\s+(\d+)>>stream\n/.exec(latin)
  if (!m) return null
  const len = Number(m[1])
  const start = m.index + m[0].length
  return Buffer.from(latin.slice(start, start + len), 'latin1')
}
function bitsDif (a, b) {
  const na = a.length, nb = b.length
  let dif = Math.abs(na - nb)
  const n = Math.min(na, nb)
  for (let i = 0; i < n; i++) if (a[i] !== b[i]) dif++
  let e = 0, d = dif
  while (d > 0) { e++; d = Math.floor(d / 2) }
  return { dif, bits: e }
}

;(async () => {
  if (!fs.existsSync(WASM)) {
    console.log('tex.wasm em falta')
    process.exit(1)
  }
  const man = JSON.parse(fs.readFileSync(path.join(RAIZ, 'app', 'src', 'corpo.json'), 'utf8'))
  const disco = await import(pathToFileURL(path.join(RAIZ, 'app', 'src', 'corpo_disco.js')).href)
  const bytes = fs.readFileSync(WASM)
  const num = (x) => typeof x === 'bigint' ? Number(x) : x

  let motor = null
  const cache = new Map()
  for (const f of man.ficheiros) {
    cache.set(f, fs.readFileSync(path.join(RAIZ, f)))
  }
  for (const fonte of Object.values(DOCS)) {
    if (!cache.has(fonte) && fs.existsSync(path.join(RAIZ, fonte))) {
      cache.set(fonte, fs.readFileSync(path.join(RAIZ, fonte)))
    }
  }

  function mem () { return new Uint8Array(motor.exports.DISCO.buffer) }
  function reserva (n) {
    const p = num(motor.exports.vfs_reserva(n))
    if (!p) throw new Error('vfs')
    return p
  }
  function poeStr (s) {
    const nb = Buffer.from(s, 'latin1')
    const p = reserva(nb.length + 1)
    mem().set(nb, p); mem()[p + nb.length] = 0
    return p
  }
  function poeBytes (nome, u8) {
    const pN = poeStr(nome)
    const pD = reserva(Math.max(u8.length, 0) + 1)
    if (u8.length) mem().set(u8, pD)
    mem()[pD + u8.length] = 0
    if (!motor.exports.poe_ficheiro(pN, pD, u8.length)) throw new Error('poe ' + nome)
  }
  function fichMiss (ptr) {
    const v = mem()
    let s = ''
    for (let i = ptr; i < v.length && v[i]; i++) s += String.fromCharCode(v[i])
    const can = disco.resolveCorpoNome(s, cache)
    if (!can || !cache.has(can)) return 0
    if (motor.poe.has(can)) return 1
    poeBytes(can, cache.get(can))
    motor.poe.add(can)
    return 1
  }

  const E0 = instanciaTex(bytes, fichMiss).exports
  E0.inicia_wasm()
  motor = { exports: E0, poe: new Set() }

  console.log('=== DIMENSÃO e π por tradução (.tex→PDF) ===\n')
  console.log('doc'.padEnd(18) + 'alc' + '  dim' + ' if' + ' L' + '     π_n' + '       bitsπ' +
    '  fecha' + '   λbits' + '  Forms' + '   ms')
  console.log('-'.repeat(86))

  const rows = []
  for (const [id, fonte] of Object.entries(DOCS)) {
    const E = motor.exports
    if (E.volta_compila) E.volta_compila()
    motor.poe.clear()
    for (const nome of disco.ficheirosPara(fonte, man.ficheiros)) {
      if (!cache.has(nome)) continue
      /* só Map; miss põe no fopen */
    }
    /* enche cache já está; miss resolve */
    motor.cache = cache
    if (E.marca_vfs) E.marca_vfs()
    E.limpa_saida()
    const t0 = Date.now()
    const rc = num(E.compila_ficheiro(poeStr(fonte), poeStr('saida.pdf')))
    const ms = Date.now() - t0
    if (rc !== 0) {
      console.log(`${id.padEnd(18)} FALHA rc=${rc}`)
      rows.push({ id, ok: false })
      continue
    }
    const tam = num(E.tam_saida())
    const end = num(E.MOVE(14, 1))
    const out = Buffer.from(mem().slice(end, end + tam))
    const latin = out.toString('latin1')
    const sem = parseSemente(latin)
    const selo = parseAssinatura(latin)
    const torre = parseAssinaturaTorre(latin)
    const alcance = sem && sem.alcance != null ? sem.alcance : -1
    const dim = sem && sem.dim != null ? sem.dim : -1
    const lado = sem && sem.lado != null ? sem.lado : null
    const iface = sem && sem.iface != null ? sem.iface : null
    const piMotor = sem && sem.piN != null ? sem.piN / 1e9 : null
    const piM = dim > 0 ? medidaPi(dim, alcance) : null
    const pi = piMotor != null ? piMotor : (piM ? piM.pi : null)
    const bitsPi = pi != null && piM ? piM.bits : null
    const fecha = piM ? piM.fecha : false
    const gentil = lado === 1 || (piM && !piM.hurwitz)
    const forms = (latin.match(/\/Subtype\/Form/g) || []).length
    const torreNespV = torreNesp(dim)
    let torreBate = dim < 16 ? !torre : false
    if (dim >= 16 && torre && torre.sel && torre.n === torreNespV) {
      const S = espectroTorre(latin, torre.n)
      torreBate = S.length === torre.sel.length && S.every((v, i) => v === torre.sel[i])
    }

    const emb = extraiFonteTeX(out)
    const src = cache.get(fonte)
    let lyap = { dif: -1, bits: -1 }
    if (emb && src) lyap = bitsDif(emb, src)

    if (E.volta_compila) E.volta_compila()
    motor.poe.clear()

    const linha = {
      id, alcance, dim, lado, iface,
      induc: sem && sem.induc != null ? sem.induc : null,
      norma: sem && sem.norma != null ? sem.norma : null,
      reguaL: sem && sem.reguaL != null ? sem.reguaL : null,
      reguaC: sem && sem.reguaC != null ? sem.reguaC : null,
      torreDim: selo && selo.torreDim != null ? selo.torreDim : null,
      torreN: torre && torre.n != null ? torre.n : null,
      torreBate,
      pi, bitsPi, fecha, gentil,
      piMotor, piHarness: piM ? piM.pi : null,
      lyapBits: lyap.bits,
      lyapDif: lyap.dif,
      forms, ms, tam, ok: true,
    }
    rows.push(linha)
    console.log(
      id.padEnd(18) +
      String(alcance).padStart(3) +
      String(dim).padStart(5) +
      String(iface != null ? iface : '?').padStart(3) +
      String(lado != null ? lado : '?').padStart(2) +
      (pi != null ? pi.toFixed(8) : '        ?').padStart(12) +
      (bitsPi != null ? bitsPi.toFixed(1) : '?').padStart(8) +
      (fecha ? '   sim' : '   nao').padStart(7) +
      (gentil ? '*' : ' ') +
      String(lyap.bits).padStart(6) +
      String(forms).padStart(7) +
      String(ms).padStart(6),
    )
  }

  console.log('\n==========================================================================')
  console.log('  Dim = 2^(alcance+1). if = interface (6,12,24,…). L: 0=Hurwitz 1=Gentil.')
  console.log('  π_n = /PiN; /Regua = 30·100+C; C=12+6·⌊alc/3⌋ é a ordem (régua dinâmica).')
  console.log('  * = lado Gentil/Lebesgue — a régua continua, Hurwitz só dobrou.')
  const ok = rows.filter((r) => r.ok)
  const comPi = ok.filter((r) => r.pi != null)
  const comContrato = ok.filter((r) => {
    const torreOk = r.dim < 16 ? !r.torreN : (r.torreN === torreNesp(r.dim) && r.torreBate)
    return r.iface != null && r.lado != null && r.piMotor != null &&
      r.norma === r.lado && r.induc === r.alcance &&
      r.reguaC === reguaCap(r.alcance) &&
      r.reguaL === reguaL(r.alcance) &&
      (r.regua == null || r.regua === r.reguaL * 100 + r.reguaC) &&
      (r.torreDim == null || r.torreDim === r.dim) && torreOk
  })
  if (comPi.length) {
    const maxD = Math.max(...comPi.map((r) => r.dim))
    const minB = Math.min(...comPi.filter((r) => r.bitsPi != null).map((r) => r.bitsPi))
    const maxB = Math.max(...comPi.filter((r) => r.bitsPi != null).map((r) => r.bitsPi))
    const ly0 = ok.filter((r) => r.lyapBits === 0).length
    console.log(`  dim ∈ {${[...new Set(comPi.map((r) => r.dim))].sort((a, b) => a - b).join(',')}}` +
      `  max=${maxD}  bitsπ ∈ [${minB.toFixed(1)}, ${maxB.toFixed(1)}]  λ=0: ${ly0}/${ok.length}` +
      `  contrato: ${comContrato.length}/${ok.length}`)
  }

  fs.writeFileSync('/tmp/traduz_pi_dim.json', JSON.stringify({ quando: new Date().toISOString(), rows }, null, 2))
  console.log('  → /tmp/traduz_pi_dim.json')

  const falhas = ok.filter((r) => {
    const torreOk = r.dim < 16 ? !r.torreN : (r.torreN === torreNesp(r.dim) && r.torreBate)
    return r.lyapBits !== 0 || r.dim < 2 || r.piMotor == null ||
      r.norma !== r.lado ||
      r.induc !== r.alcance ||
      r.reguaC !== reguaCap(r.alcance) ||
      r.reguaL !== reguaL(r.alcance) ||
      (r.torreDim != null && r.torreDim !== r.dim) ||
      !torreOk ||
      (r.piHarness != null && Math.abs(r.piMotor - r.piHarness) > 0.001)
  }).length
  console.log(`#TOTAL ${ok.length} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => { console.error(e); process.exit(1) })
