/* mvp_absorve.mjs — absorção wasm+canal: MOVE na arena, sem runtime. */
import { readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { shellRemotoCanal } from './canal_cliente.mjs'

const __dir = dirname(fileURLToPath(import.meta.url))
const RAIZ = join(__dir, '..')
const NULO = 8
const OFF_NOUT = 24578
const OFF_BUF_OUT = 16384

function u8 (ex) {
  return new Uint8Array(ex.DISCO.buffer)
}

function enc (ex, off, str) {
  const b = Buffer.from(str, 'utf8')
  u8(ex).set(b, NULO + off)
  return b.length
}

function dec (ex, off, len) {
  return Buffer.from(u8(ex).slice(NULO + off, NULO + off + len)).toString('utf8')
}

function moveWasm (ex, fnName, text, sentido, inOff = 1024, outOff = 8192) {
  const n = enc(ex, inOff, text)
  const fn = ex[fnName]
  if (typeof fn !== 'function') throw new Error('export ' + fnName + ' em falta')
  const outLen = fn(inOff, n, outOff, sentido)
  return dec(ex, outOff, outLen)
}

function injetaStdoutArena (ex, texto) {
  const mem = u8(ex)
  const b = Buffer.from(texto, 'utf8')
  const cap = 8192
  const n = Math.min(b.length, cap)
  mem.set(b.subarray(0, n), NULO + OFF_BUF_OUT)
  mem[NULO + OFF_NOUT] = n & 255
  mem[NULO + OFF_NOUT + 1] = (n >> 8) & 255
}

let cacheWasm = null

function wasmNode () {
  if (cacheWasm) return cacheWasm
  const buf = readFileSync(join(RAIZ, 'assets', 'figuras', 'wasm', 'node.wasm'))
  cacheWasm = new WebAssembly.Instance(new WebAssembly.Module(buf), {}).exports
  return cacheWasm
}

/** Cliente: node_move(-1) → canal (banco remoto DISCO) → node_move(+1). */
export async function absorveNodeCanal (script, canal) {
  const ex = wasmNode()
  moveWasm(ex, 'node_move', script, -1)
  const raw = await shellRemotoCanal(canal, script, 'node')
  injetaStdoutArena(ex, raw)
  const out = moveWasm(ex, 'node_move', '', +1, 8192, 8192)
  return { out, raw, via: 'wasm+canal+DISCO' }
}
