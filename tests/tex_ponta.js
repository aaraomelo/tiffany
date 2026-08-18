/* tex_ponta.js — PONTA A PONTA: LS → inflate → miss → PDF (contrato do browser).
 *
 * O Aarão: PDF no cliente via WASM, sem TeX Live. O Node finge o browser:
 * localStorage (GKCORPO), Map inflate, __fich_miss, volta_compila — a mesma
 * sequência de app/src/tex_tradutor.js (sem import JSON do Vite).
 *
 *   §P0  dualsort: %%EOF + Semente + Assinatura, miss>0, origem LS
 *   §P1  2ª visita: dualsort id, inflate do LS (sem re-gravar)
 *   §P2  1 bit: enredo → catálogo×2 id
 *
 *   node tests/tex_ponta.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const zlib = require('zlib')
const { pathToFileURL } = require('url')
const { instanciaTex } = require('./tex_env.js')

const RAIZ = path.resolve(__dirname, '..')
const WASM = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'tex.wasm')
const man = JSON.parse(fs.readFileSync(path.join(RAIZ, 'app', 'src', 'corpo.json'), 'utf8'))

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
  console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`)
}

function streamZlib (fn) {
  return class {
    constructor (fmt) {
      if (fmt !== 'deflate') throw new TypeError(fmt)
      const chunks = []
      const ts = new TransformStream({
        transform (c) { chunks.push(c instanceof Uint8Array ? c : new Uint8Array(c)) },
        flush (ctrl) {
          ctrl.enqueue(new Uint8Array(fn(Buffer.concat(chunks.map((c) => Buffer.from(c))))))
        },
      })
      this.readable = ts.readable
      this.writable = ts.writable
    }
  }
}
globalThis.CompressionStream = streamZlib((u) => zlib.deflateSync(u, { level: 9 }))
globalThis.DecompressionStream = streamZlib((u) => zlib.inflateSync(u))

const num = (x) => typeof x === 'bigint' ? Number(x) : x
const now = () => Number(process.hrtime.bigint()) / 1e6

;(async () => {
  if (!fs.existsSync(WASM)) {
    console.log('  tex.wasm em falta — tools/sobe_tex_wasm.sh')
    console.log('#TOTAL 0 1')
    process.exit(1)
  }

  const disco = await import(pathToFileURL(path.join(RAIZ, 'app', 'src', 'corpo_disco.js')).href)
  const ls = disco.memoriaLS()
  const pares = man.ficheiros.map((nome) => ({ nome, bytes: fs.readFileSync(path.join(RAIZ, nome)) }))

  console.log('=== PONTA A PONTA: LS → miss → PDF ===\n')

  /* 1ª visita: grava GKCORPO no LS (como fetch+gravaCorpo no browser) */
  const grava = await disco.gravaCorpo(ls, pares, man.soma)
  const mapa = disco.leMapa(ls)
  console.log(`   LS grava ${grava.n} slots z=${(grava.zTotal / 1048576).toFixed(2)} MiB` +
    ` utf16=${(disco.bytesLS(ls) / 1048576).toFixed(2)} MiB  ${grava.ms.toFixed(0)} ms` +
    ` bate=${disco.mapaBate(mapa, man.soma, man.ficheiros)}`)

  const bytes = fs.readFileSync(WASM)
  let motor = null

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
    const can = disco.resolveCorpoNome(s, motor.cache)
    if (!can) return 0
    if (motor.poe.has(can)) return 1
    poeBytes(can, motor.cache.get(can))
    motor.poe.add(can)
    motor.miss.n++
    motor.miss.bytes += motor.cache.get(can).length
    return 1
  }

  function carregaMotor () {
    if (motor) return motor
    const E = instanciaTex(bytes, fichMiss).exports
    E.inicia_wasm()
    motor = { exports: E, poe: new Set(), cache: new Map(), miss: { n: 0, bytes: 0 } }
    return motor
  }

  async function encheCache (fonte) {
    const nomes = disco.ficheirosPara(fonte, man.ficheiros)
    let n = 0, bytesN = 0
    const t0 = now()
    for (const nome of nomes) {
      if (motor.cache.has(nome)) continue
      const u8 = await disco.leFicheiro(ls, mapa, nome)
      if (!u8) throw new Error('LS miss ' + nome)
      motor.cache.set(nome, u8)
      n++
      bytesN += u8.length
    }
    return { n, bytes: bytesN, ms: Math.round(now() - t0), lista: nomes.length }
  }

  async function compor (fonte) {
    const { exports: E } = carregaMotor()
    if (typeof E.volta_compila === 'function') E.volta_compila()
    motor.poe.clear()
    motor.miss = { n: 0, bytes: 0 }
    const cache = await encheCache(fonte)
    if (typeof E.marca_vfs === 'function') E.marca_vfs()
    E.limpa_saida()
    const t0 = now()
    const rc = num(E.compila_ficheiro(poeStr(fonte), poeStr('saida.pdf')))
    const msCompila = Math.round(now() - t0)
    if (rc !== 0) throw new Error(`compila(${fonte}) → ${rc}`)
    const tam = num(E.tam_saida())
    const end = num(E.MOVE(14, 1))
    const out = mem().slice(end, end + tam)
    if (typeof E.volta_compila === 'function') E.volta_compila()
    motor.poe.clear()
    const latin = Buffer.from(out).toString('latin1')
    return {
      fonte, tam, out, msCompila,
      miss: motor.miss.n,
      missBytes: motor.miss.bytes,
      cacheMs: cache.ms,
      cacheN: cache.lista,
      pages: (latin.match(/\/Type\s*\/Page[^s]/g) || []).length,
      forms: (latin.match(/\/Subtype\/Form/g) || []).length,
      eof: latin.includes('%%EOF'),
      semente: latin.includes('/Type/SementeEstrela'),
      selo: latin.includes('/Type/AssinaturaOito'),
      pct: out[0] === 0x25 && out[1] === 0x50,
    }
  }

  /* ─── §P0 dualsort ─────────────────────────────────────────────────────────── */
  const d0 = await compor('corpus/docs/dualsort.tex')
  console.log(`   §P0 dualsort tam=${d0.tam} pág=${d0.pages} Forms=${d0.forms} miss=${d0.miss}` +
    ` cache=${d0.cacheN} ${d0.msCompila} ms`)
  ok('§P0 dualsort: %PDF + Semente + Assinatura + miss>0 (corpo do LS)',
    d0.pct && d0.eof && d0.semente && d0.selo && d0.miss > 5 && d0.tam > 1e5 && d0.pages > 5)

  /* ─── §P1 2ª dualsort: id, cache quente ────────────────────────────────────── */
  const d1 = await compor('corpus/docs/dualsort.tex')
  console.log(`   §P1 2.ª dualsort tam=${d1.tam} Forms=${d1.forms} miss=${d1.miss} cacheMs=${d1.cacheMs}`)
  ok('§P1 2ª visita: dualsort id (1 bit + Map quente)',
    d1.tam === d0.tam && d1.forms === d0.forms && d1.miss === d0.miss && d1.pages === d0.pages)

  /* ─── §P2 enredo → catálogo×2 ──────────────────────────────────────────────── */
  console.log('   §P2 enredo…')
  const en = await compor('enredo.tex')
  console.log(`      enredo tam=${en.tam} pág=${en.pages} Forms=${en.forms} miss=${en.miss} ${en.msCompila} ms`)
  console.log('   §P2 catalogo×2…')
  const cA = await compor('catalogo.tex')
  const cB = await compor('catalogo.tex')
  console.log(`      cat A tam=${cA.tam} pág=${cA.pages} Forms=${cA.forms} miss=${cA.miss}`)
  console.log(`      cat B tam=${cB.tam} pág=${cB.pages} Forms=${cB.forms} miss=${cB.miss}`)
  ok('§P2 1 bit ponta a ponta: enredo ~385 pág. e catálogo×2 id',
    en.eof && en.semente && en.selo && en.pages > 300 && en.pages < 500
    && cA.eof && cA.semente && cA.selo && cA.pages > 400
    && cA.tam === cB.tam && cA.forms === cB.forms && cA.miss === cB.miss)

  /* ─── §P3 dualsort após catálogo (sessão longa) ────────────────────────────── */
  const d2 = await compor('corpus/docs/dualsort.tex')
  console.log(`   §P3 dualsort após catálogo tam=${d2.tam} Forms=${d2.forms}`)
  ok('§P3 sessão longa: dualsort volta ao mesmo após catálogo (estrela sem estado)',
    d2.tam === d0.tam && d2.forms === d0.forms && d2.pages === d0.pages)

  console.log('\n==========================================================================')
  if (!falhas) {
    console.log('  Ponta a ponta fecha: LS (GKCORPO) → Map → __fich_miss → PDF.')
    console.log(`  dualsort ${d0.pages} pág · enredo ${en.pages} pág · catálogo ${cA.pages} pág.`)
  } else console.log(`  FALHOU: ${falhas}`)
  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => {
  console.error(e)
  process.exit(1)
})
