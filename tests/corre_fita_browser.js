/* corre_fita_browser.js — isa.wasm + erg.fita no browser (WebAssembly API).
 *   node tests/corre_fita_browser.js
 *   (correr após tools/gera_nucleo.bat — wasm com erg.fita embutida)
 */
import fs from 'fs'
import path from 'path'
import { fileURLToPath } from 'url'
import { execFileSync } from 'node:child_process'
import { extraiSecaoWasm, SEC_FITA } from '../app/src/wasm_sec_browser.js'
import { parseConstsErg, arenaSeeds, leStdoutArena } from '../app/src/corre_metal_browser.js'
import { correFita } from '../app/src/isa_fita.js'

const RAIZ = path.join(path.dirname(fileURLToPath(import.meta.url)), '..')
const TRADUZ = path.join(RAIZ, 'tools/bin/traduz.exe')
const ISA_WASM = path.join(RAIZ, 'assets/figuras/wasm/isa.wasm')

let nFalhas = 0
let nFeitas = 0
function ok (q, cond) {
  nFeitas++
  if (!cond) nFalhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function garanteIsa () {
  if (fs.existsSync(ISA_WASM)) return
  if (!fs.existsSync(TRADUZ)) {
    execFileSync('gcc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools/traduz.c'), '-o', TRADUZ])
  }
  execFileSync(TRADUZ, [path.join(RAIZ, 'tools/isa.c'), '-o', ISA_WASM])
}

function loadIsa () {
  garanteIsa()
  return new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(ISA_WASM))).exports
}

function fitaDoWasm (backend) {
  const p = path.join(RAIZ, 'assets/figuras/wasm', backend + '.wasm')
  const buf = new Uint8Array(fs.readFileSync(p))
  return extraiSecaoWasm(buf, SEC_FITA, 'bytes')
}

function correErg (backend, script) {
  const fita = fitaDoWasm(backend)
  const correErgText = fs.readFileSync(
    path.join(RAIZ, 'conecthus/backends', backend, backend + '_corre.erg'), 'utf8')
  const seeds = [...parseConstsErg(correErgText), ...arenaSeeds(script, backend)]
  const isa = loadIsa()
  return correFita(isa.MOVE, fita, seeds, 2_000_000)
}

ok('IF0 isa.wasm existe', fs.existsSync(ISA_WASM) || (garanteIsa(), true))
{
  const ex = loadIsa()
  ok('IF0 export MOVE', typeof ex.MOVE === 'function')
}

{
  const fita = fitaDoWasm('node')
  ok('IF1 node erg.fita no wasm', fita && fita.length > 0)
}

{
  const r = correErg('node', "console.log('42')")
  const out = leStdoutArena(r)
  ok('IF2 node isa+fita stdout 42', out.includes('42'))
  ok('IF2 node passos > 0', r.passos > 0)
}

{
  const r = correErg('bash', 'echo 42\n')
  const out = leStdoutArena(r)
  ok('IF3 bash isa+fita stdout 42', out.includes('42'))
}

console.log(`#TOTAL ${nFeitas} ${nFalhas}`)
process.exit(nFalhas ? 1 : 0)
