/* traduz_asm_wasm.js — ponte ASM ↔ WASM: tradução completa entre as duas réguas da ISA.
 *   node tests/traduz_asm_wasm.js
 */
'use strict'

const fs = require('fs')
const path = require('path')
const os = require('os')
const { execFileSync } = require('child_process')
const { pathToFileURL } = require('url')

const RAIZ = path.resolve(__dirname, '..')
const TMP = process.env.TEMP || process.env.TMPDIR || os.tmpdir()
const TRADUZ = path.join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')
const ERG = path.join(TMP, 'erg_aw')
const SOMA = path.join(RAIZ, 'banco', 'apps', 'soma.erg')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function difBytes (a, b) {
  const A = a instanceof Uint8Array ? a : new Uint8Array(a)
  const B = b instanceof Uint8Array ? b : new Uint8Array(b)
  let d = 0
  for (let i = 0; i < Math.max(A.length, B.length); i++) if (A[i] !== B[i]) d++
  return d
}

async function main () {
  const { monta, desmonta } = await import(pathToFileURL(path.join(RAIZ, 'lib', 'erg_monta.mjs')).href)
  const { sobeErg, desceWasm, fitaDeWasm, extractCustom, garanteIsaWasm, SEC_FITA } =
    await import(pathToFileURL(path.join(RAIZ, 'tools', 'asm_wasm.mjs')).href)

  const prog = fs.readFileSync(SOMA, 'utf8')
  const fita0 = monta(prog)

  ok('§AW0 monta(soma.erg) > 0', fita0.length > 0)

  const txt1 = desmonta(fita0)
  const fita1 = monta(txt1)
  ok('§AW1 desmonta(monta) remonta byte a byte', difBytes(fita0, fita1) === 0)

  try {
    if (!fs.existsSync(TRADUZ)) {
      execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', TRADUZ])
    }
    garanteIsaWasm()
  } catch (e) {
    ok('§AW2 isa.wasm disponível', false)
    console.log('       ', String(e.message || e).split('\n')[0])
    finish()
    return
  }

  const wasm = sobeErg(prog)
  ok('§AW2 sobe(erg) gera wasm válido', wasm[0] === 0 && wasm[1] === 0x61)
  ok('§AW2 secção erg.fita presente', extractCustom(wasm, SEC_FITA) !== null)

  const fita2 = fitaDeWasm(wasm)
  ok('§AW2 fita extraída = monta(original)', difBytes(fita0, fita2) === 0)

  const ergVolta = desceWasm(wasm)
  const fita3 = monta(ergVolta)
  ok('§AW3 desce(sobe(erg)) remonta fita', difBytes(fita0, fita3) === 0)

  const ex = WebAssembly.Module.exports(new WebAssembly.Module(wasm))
  ok('§AW4 módulo exporta MOVE', ex.some((e) => e.name === 'MOVE' && e.kind === 'function'))

  let temCc = false
  try { execFileSync('cc', ['--version'], { stdio: 'ignore' }); temCc = true } catch (_) {}
  if (temCc) {
    try {
      execFileSync('cc', ['-O2', '-std=c99', '-w', '-I' + path.join(RAIZ, 'lib'),
        path.join(RAIZ, 'banco', 'erg.c'), '-o', ERG])
      const src = path.join(TMP, 'aw.erg')
      const bin = path.join(TMP, 'aw.bin')
      fs.writeFileSync(src, prog)
      execFileSync(ERG, ['monta', src, bin])
      ok('§AW5 monta JS = monta erg', difBytes(fita0, fs.readFileSync(bin)) === 0)
    } catch (e) {
      ok('§AW5 monta JS = monta erg', false)
      console.log('       ', String(e.message || e).split('\n')[0])
    }
  } else {
    console.log('   (§AW5 erg nativo omitido — sem cc)')
  }

  const wasm2 = sobeErg(desceWasm(wasm))
  ok('§AW6 sobe∘desce preserva erg.fita', difBytes(fitaDeWasm(wasm), fitaDeWasm(wasm2)) === 0)

  finish()
}

function finish () {
  console.log(`\n=== traduz_asm_wasm: ${feitas - falhas}/${feitas} OK ===`)
  process.exit(falhas ? 1 : 0)
}

main().catch((e) => {
  console.error(e)
  process.exit(1)
})
