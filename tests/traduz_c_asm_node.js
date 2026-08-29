/* traduz_c_asm_node.js — cadeia C → assembly → Node.
 *   interpretar.c → node.wasm → celula.erg → Node (MOVE + canal, sem SQL adaptador)
 *
 *   node tests/traduz_c_asm_node.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { tmpdir } from 'node:os'
import { fileURLToPath } from 'node:url'
import {
  sobeC, wasmParaAsm, sobeCadeia, desceCadeia, shellConfig,
} from '../lib/c_asm_shell.mjs'
import { execMoveDisco, S_CANAL_BASE, SLOTS } from '../tools/banco_shell_core.mjs'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const TMP = process.env.TEMP || process.env.TMPDIR || tmpdir()

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

function finish () {
  console.log(`\n=== traduz_c_asm_node: ${feitas - falhas}/${feitas} OK ===`)
  process.exit(falhas ? 1 : 0)
}

const cfg = shellConfig('node')
const WASM_OUT = cfg.wasmPath
ok('§CAN0 interpretar.c existe', existsSync(cfg.fontePath))
ok('§CAN0 traduz disponível', existsSync(join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')))
let wasm0
try {
  wasm0 = sobeC('node', WASM_OUT)
  ok('§CAN1 sobe(interpretar.c) → node.wasm', wasm0.length > 100)
  const ex = WebAssembly.Module.exports(new WebAssembly.Module(wasm0))
  ok('§CAN1 export node_move', ex.some((e) => e.name === 'node_move'))
} catch (e) {
  ok('§CAN1 sobe C→wasm', false)
  console.log('       ', String(e.message || e).split('\n')[0])
  finish()
}

/* §CAN2 — wasm → assembly */
let erg
try {
  erg = wasmParaAsm('node', wasm0)
  ok('§CAN2 wasm→assembly gera texto', erg.length > 50)
  ok('§CAN2 assembly menciona node_move', erg.includes('node_move'))
  ok('§CAN2 assembly tem opcodes ERG', /\b(LOAD|ADD|STORE|HALT)\b/.test(erg))
} catch (e) {
  ok('§CAN2 wasm→assembly', false)
  console.log('       ', String(e.message || e).split('\n')[0])
}

/* §CAN3 — embute node.erg no módulo */
let wasmFull
try {
  const r = sobeCadeia('node', { wasm: WASM_OUT, erg: join(TMP, 'celula.erg') })
  wasmFull = r.wasm
  erg = r.erg
  ok('§CAN3 node.erg embutido', desceCadeia('node', wasmFull).includes('node_move'))
  ok('§CAN3 wasm cresceu com secções', wasmFull.length > wasm0.length)
} catch (e) {
  ok('§CAN3 embute assembly', false)
  console.log('       ', String(e.message || e).split('\n')[0])
}

/* §CAN4 — desce recupera assembly */
if (wasmFull && erg) {
  const volta = desceCadeia('node', wasmFull)
  ok('§CAN4 desce(sobe) preserva node_move', volta.includes('node_move'))
  ok('§CAN4 desce ≈ wasmParaAsm', volta.trim() === erg.trim())
}

/* §CAN5 — Node via MOVE (cadeia completa, sem SQL) */
try {
  const SQL_BASE = join(RAIZ, '.torre', 'reino_bench_node')
  const r = execMoveDisco(SQL_BASE, 'node', 'console.log(42)')
  ok('§CAN5 assembly→Node MOVE stdout', (r.stdout || '').includes('42'))
  ok('§CAN5 meta slots canal', r.meta?.slotIn === S_CANAL_BASE + SLOTS.node.in &&
    r.meta?.slotOut === S_CANAL_BASE + SLOTS.node.out)
} catch (e) {
  ok('§CAN5 Node via MOVE', false)
  console.log('       ', String(e.message || e).split('\n')[0])
}

/* §CAN6 — manifesto: ponte C que o metal compila; JS gémeo em lib/ */
const man = JSON.parse(readFileSync(join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))
const node = man.linguagens.find((l) => l.nome === 'node')
ok('§CAN6 cadeia tem asm', !!node?.cadeia?.asm)
ok('§CAN6 ponte wasm_sec.c', node?.cadeia?.ponte_asm === 'tools/wasm_sec.c')
ok('§CAN6 sequencia c→wasm→asm→node', Array.isArray(node?.cadeia?.sequencia))

/* §CAN7 — C↔wasm byte (sem secções custom da cadeia asm) */
try {
  const wasmLimpo = sobeC('node', join(TMP, 'node_limpo.wasm'))
  const wasm2 = sobeC('node', join(TMP, 'node_limpo2.wasm'))
  ok('§CAN7 sobe(C) determinístico', difBytes(wasmLimpo, wasm2) === 0)
} catch (_) {
  ok('§CAN7 sobe(C) determinístico', false)
}

finish()
