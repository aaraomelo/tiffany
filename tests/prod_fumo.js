/* prod_fumo.js — SMOKE da produção: wasm + corpo + painel no ar.
 *   node tests/prod_fumo.js
 */
'use strict'
const BASE = process.env.GK_URL || 'https://goldenkingdom.patriatechnology.com'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

;(async () => {
  console.log(`=== FUMO PRODUÇÃO: ${BASE} ===\n`)
  async function get (path) {
    const r = await fetch(BASE + path)
    const buf = Buffer.from(await r.arrayBuffer())
    return { status: r.status, type: r.headers.get('content-type') || '', buf, n: buf.length }
  }

  const home = await get('/')
  ok('§F0 home 200', home.status === 200 && home.n > 200)

  const wasm = await get('/wasm/tex.wasm')
  ok('§F1 tex.wasm 200 e módulo válido', wasm.status === 200 && wasm.n > 100000)
  if (wasm.status === 200) {
    const mod = await WebAssembly.compile(wasm.buf)
    const needs = WebAssembly.Module.imports(mod).some((i) => i.module === 'env' && i.name === '__fich_miss')
    const inst = needs
      ? await WebAssembly.instantiate(mod, { env: { __fich_miss: () => 0 } })
      : await WebAssembly.instantiate(mod)
    const E = inst.exports
    ok('§F1b porta: MOVE + volta_compila + compila',
      typeof E.MOVE === 'function' && typeof E.volta_compila === 'function'
      && typeof E.compila_ficheiro === 'function' && E.DISCO)
    E.inicia_wasm()
    const a = Number(E.MOVE(1, -1))
    const b = Number(E.MOVE(1, 1))
    ok('§F1c MOVE ±1 no banco (produção)', a > 0 && a === b)
  }

  const estilo = await get('/corpo/estilo.tex')
  const dual = await get('/corpo/papers/arquitetura.tex')
  ok('§F2 corpo estilo+computacional 200', estilo.status === 200 && dual.status === 200 && estilo.n > 100 && dual.n > 1000)

  const painel = await get('/wasm/painel_motor.wasm')
  ok('§F3 painel_motor.wasm 200', painel.status === 200 && painel.n > 100)
  if (painel.status === 200) {
    const { instance } = await WebAssembly.instantiate(painel.buf)
    const v = new BigInt64Array(instance.exports.mem.buffer)
    v[2] = 1n << 20n; v[3] = 1000n; v[6] = 1n; v[7] = v[2]; v[8] = 1n; v[9] = 64n
    instance.exports.prog()
    ok('§F3b painel MULT corre em produção', v[4] === (1n << 20n) * 1000n)
  }

  const cv = await get('/docs/cv.pdf')
  ok('§F4 cv.pdf estático', cv.status === 200 && cv.type.includes('pdf') && cv.buf[0] === 0x25)

  console.log(`\n#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => { console.error(e); process.exit(1) })
