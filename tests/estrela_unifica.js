/* estrela_unifica.js — PAINEL + TEX: o mesmo contrato (escreve → chama → lê).
 *
 *   §U0  painel_motor: MULT ⊗ nos slots i64 (chamaNoDisco)
 *   §U1  tex.wasm: MOVE(−1) emite, MOVE(+1) absorve, volta zera
 *   §U2  Lei 7 ≠ Lei 8 nomeadas no PDF (Semente + AssinaturaOito)
 *
 *   node tests/estrela_unifica.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { pathToFileURL } = require('url')
const { instanciaTex } = require('./tex_env.js')

const RAIZ = path.resolve(__dirname, '..')
const PAINEL = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'painel_motor.wasm')
const TEX = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'tex.wasm')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
  console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`)
}

;(async () => {
  const porta = await import(pathToFileURL(path.join(RAIZ, 'app', 'src', 'estrela_porta.js')).href)
  console.log('=== ESTRELA UNIFICA: painel ⊕ tex, uma porta ===\n')

  /* ─── §U0 painel ─────────────────────────────────────────────────────────── */
  if (!fs.existsSync(PAINEL)) {
    console.log('  painel_motor.wasm em falta — §U0 saltado')
    ok('§U0 painel presente', false)
  } else {
    const { instance } = await WebAssembly.instantiate(fs.readFileSync(PAINEL))
    const view = new BigInt64Array(instance.exports.mem.buffer)
    const of = 1n << 20n, df = 17476n
    const { view: v } = porta.chamaNoDisco({
      view,
      prog: instance.exports.prog,
      escreve (s) {
        s[0] = 0n; s[1] = 0n; s[2] = of; s[3] = df; s[4] = 0n
        s[5] = 0n; s[6] = 1n; s[7] = of; s[8] = 1n; s[9] = 64n; s[10] = 0n
      },
    })
    const prod = v[4]
    const esperado = of * df
    console.log(`   §U0 MULT ⊗ prod=${prod} esperado=${esperado}`)
    ok('§U0 painel: escreve→prog→lê (chamaNoDisco) — MULT exacta',
      prod === esperado && typeof instance.exports.prog === 'function')
  }

  /* ─── §U1 tex MOVE ───────────────────────────────────────────────────────── */
  if (!fs.existsSync(TEX)) {
    console.log('  tex.wasm em falta')
    console.log('#TOTAL 0 1')
    process.exit(1)
  }
  const E = instanciaTex(fs.readFileSync(TEX)).exports
  E.inicia_wasm()
  const emi = porta.emite(E, 6)
  const abs = porta.absorve(E, 6)
  const atr0 = porta.atravessa(E, 7)   /* 7 ainda não emitido */
  const emi7 = porta.emite(E, 7)
  const atr7 = porta.atravessa(E, 7)
  if (typeof E.volta_compila === 'function') E.volta_compila()
  const apos = porta.absorve(E, 6)
  console.log(`   §U1 emite6=${emi} abs=${abs} atr7antes=${atr0} emi7=${emi7} atr7=${atr7} após volta=${apos}`)
  ok('§U1 tex: −1 emite, +1 absorve, 0 não nasce, volta_compila zera',
    emi > 0 && emi === abs && atr0 === 0 && emi7 > 0 && atr7 === emi7 && apos === 0)

  /* ─── §U2 dualsort: Lei 7 viaja + Lei 8 assina ───────────────────────────── */
  const man = JSON.parse(fs.readFileSync(path.join(RAIZ, 'app', 'src', 'corpo.json'), 'utf8'))
  const num = (x) => typeof x === 'bigint' ? Number(x) : x
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
  }
  const subset = ['estilo.tex', 'papers/dualsort.tex', 'papers/gkcapa.tex']
  for (const f of man.ficheiros) {
    if (!subset.some((s) => f === s || f.endsWith('/' + path.basename(s)))) continue
    const u8 = fs.readFileSync(path.join(RAIZ, f))
    poeFich(f, u8)
  }
  /* poe o manifesto lazy via ficheirosPara se disponível */
  const disco = await import(pathToFileURL(path.join(RAIZ, 'app', 'src', 'corpo_disco.js')).href)
  for (const f of disco.ficheirosPara('papers/dualsort.tex', man.ficheiros)) {
    if (!fs.existsSync(path.join(RAIZ, f))) continue
    poeFich(f, fs.readFileSync(path.join(RAIZ, f)))
  }
  if (typeof E.marca_vfs === 'function') E.marca_vfs()
  E.limpa_saida()
  const rc = num(E.compila_ficheiro(poeStr('papers/dualsort.tex'), poeStr('saida.pdf')))
  const tam = num(E.tam_saida())
  const end = porta.absorve(E, 14)
  const pdf = Buffer.from(mem().slice(end, end + tam)).toString('latin1')
  const lei7 = pdf.includes('/Type/SementeEstrela')   /* composição viaja (circuito) */
  const lei8 = pdf.includes('/Type/AssinaturaOito')   /* Caelum assina */
  console.log(`   §U2 rc=${rc} tam=${tam} Semente=${lei7} AssinaturaOito=${lei8}`)
  ok('§U2 dualsort: Lei 7 (semente no circuito) e Lei 8 (selo Caelum) no mesmo PDF',
    rc === 0 && tam > 1e5 && lei7 && lei8 && pdf.includes('%%EOF'))

  console.log('\n==========================================================================')
  if (!falhas) {
    console.log('  Porta única: painel (i64) e tex (DISCO+MOVE) — escreve→chama→lê.')
    console.log('  Lei 7 = circuito; Lei 8 = selo. Estrela reverte; banco fica.')
  }
  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => { console.error(e); process.exit(1) })
