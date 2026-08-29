#!/usr/bin/env node
/* bench_mvp.mjs — benchmark MVP: wasm + DISCO + canal (sem runtime externo).
 *
 *   node tools/bench_mvp.mjs [--url http://127.0.0.1:5173] [--n 50]
 */
import { readFileSync, mkdirSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { performance } from 'node:perf_hooks'
import { createHash } from 'node:crypto'
import { execMoveMetal, garanteErg } from './banco_metal.mjs'
import { execSqlDisco } from './banco_sql_disco.mjs'
import { createCanalWatcher, medeParN1 } from './canal_watcher.mjs'
import { bandaDeTecido, tramaBump, tramaClara } from './banco_banda.mjs'
import { abrirCanalNode, shellRemotoCanal } from './canal_cliente.mjs'
import { absorveNodeCanal } from './mvp_absorve.mjs'
import { S_CHUNK, S_NODE_IN, S_NODE_OUT } from './canal_slots.mjs'

const __dir = dirname(fileURLToPath(import.meta.url))
const RAIZ = join(__dir, '..')
const BANCO = join(RAIZ, 'app', 'banco')
const SQL_BASE = process.env.TIFFANY_SQL_BASE || join(RAIZ, '.torre', 'reino_bench_mvp')
const WASM_DIR = join(RAIZ, 'assets', 'figuras', 'wasm')

const args = process.argv.slice(2)
let URL_BASE = 'http://127.0.0.1:5173'
let N = 50
for (let i = 0; i < args.length; i++) {
  if (args[i] === '--url' && args[i + 1]) URL_BASE = args[++i]
  if (args[i] === '--n' && args[i + 1]) N = Number(args[++i])
}

const PORT = Number(new URL(URL_BASE).port || 5173)
const SCRIPT = "console.log('bench')"
const SCRIPT_BASH = 'echo bench\n'

const falhas = []
function ok (q, cond) {
  if (!cond) falhas.push(q)
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

async function med (nome, fn, n = N) {
  const t0 = performance.now()
  let last
  for (let i = 0; i < n; i++) last = await fn()
  const ms = (performance.now() - t0) / n
  return {
    nome,
    ms: +ms.toFixed(3),
    n,
    bytes: last?.bytes,
    bytesIn: last?.bytesIn,
    bytesOut: last?.bytesOut,
  }
}

function bump6 (slot, total, e, banda) {
  const m = Buffer.alloc(6)
  m.writeUInt32LE(slot >>> 0, 0)
  m[4] = total & 255
  m[5] = e & 255
  const ks = keystream(banda, 6)
  for (let i = 0; i < 6; i++) m[i] ^= ks[i]
  return m
}

function keystream (banda, n) {
  const ks = Buffer.alloc(n)
  const sem = Buffer.alloc(36)
  banda.copy(sem, 0)
  for (let o = 0; o < n; o += 32) {
    sem.writeUInt32LE(Math.floor(o / 32), 32)
    const bloco = createHash('sha256').update(sem).digest()
    bloco.copy(ks, o, 0, Math.min(32, n - o))
  }
  return ks
}

function canalRoundTrip (script) {
  const banda = bandaDeTecido()
  const frames = []
  const handle = createCanalWatcher({
    sqlBase: SQL_BASE,
    banda,
    broadcast: (b) => frames.push(Buffer.from(b)),
    bancoDir: BANCO,
  })
  const body = Buffer.from(script, 'utf8')
  for (let i = 0; i < body.length; i += 2) {
    handle(tramaBump(S_CHUNK, body[i], i + 1 < body.length ? body[i + 1] : 0, banda))
  }
  handle(tramaBump(S_NODE_IN, body.length & 255, (body.length >> 8) & 255, banda))
  let outLen = 0
  const parts = []
  for (const f of frames) {
    const t = tramaClara(f, banda)
    if (t.slot === S_CHUNK) parts.push(t.total, t.e)
    if (t.slot === S_NODE_OUT) outLen = t.total + (t.e << 8)
  }
  const out = Buffer.alloc(outLen)
  let bi = 0
  for (let pi = 0; bi < outLen && pi < parts.length; pi++) out[bi++] = parts[pi] & 255
  return { out: out.toString('utf8'), bytesIn: body.length, bytesOut: outLen }
}

console.log('=== BENCH MVP — wasm + DISCO + canal (sem runtime) ===\n')
console.log(`  url=${URL_BASE}  n=${N}  base=${SQL_BASE}\n`)

mkdirSync(SQL_BASE, { recursive: true })
try { garanteErg() } catch (e) { console.warn('erg:', e.message) }

const linhas = []

/* §BM0 formal */
{
  const banda = bandaDeTecido()
  const r = await med('§BM0 bump 6B', () => {
    bump6(S_NODE_IN, 9, 1, banda)
    return { bytes: 6 }
  }, N * 20)
  linhas.push(r)
  ok(`§BM0 bump < 0.1ms (${r.ms}ms)`, r.ms < 0.1)

  const wasmBuf = readFileSync(join(WASM_DIR, 'node.wasm'))
  const r2 = await med('§BM0 node.wasm load', async () => {
    await WebAssembly.instantiate(wasmBuf)
    return { bytes: wasmBuf.length }
  }, Math.min(N, 15))
  linhas.push(r2)
}

/* §BM1 wasm MOVE + semântica + DISCO */
{
  const rNode = await med('§BM1 execMoveMetal node (erg)', () => {
    const r = execMoveMetal(SQL_BASE, 'node', SCRIPT)
    return { bytes: (r.stdout || '').length, via: r.meta?.via }
  }, N)
  linhas.push(rNode)
  ok(`§BM1 node→assembly→metal (${rNode.bytes}B)`, rNode.bytes > 0)

  const rBash = await med('§BM1 execMoveMetal bash', () => {
    const r = execMoveMetal(SQL_BASE, 'bash', SCRIPT_BASH)
    return { bytes: (r.stdout || '').length }
  }, Math.min(N, 20))
  linhas.push(rBash)

  const rSql = await med('§BM1 NODE MOVE SQL face', () => {
    const r = execSqlDisco(SQL_BASE, "NODE MOVE 'console.log(1)'")
    return { bytes: r.out.length }
  }, Math.min(N, 20))
  linhas.push(rSql)
}

/* §BM2 canal in-process */
{
  const r = await med('§BM2 canal watcher round-trip', () => {
    const { out, bytesIn, bytesOut } = canalRoundTrip(SCRIPT)
    return { bytes: out.length, bytesIn, bytesOut }
  }, N)
  linhas.push(r)
  ok('§BM2 par N1', medeParN1(r.bytesIn || 1))
}

/* §BM3 HTTP + WS (servidor ligado) */
try {
  const rFetch = await med('§BM3 GET /banco/', async () => {
    const t0 = performance.now()
    const r = await fetch(URL_BASE + '/banco/')
    const b = await r.arrayBuffer()
    return { ms: performance.now() - t0, bytes: b.byteLength }
  }, Math.min(N, 10))
  linhas.push({ nome: rFetch.nome, ms: rFetch.ms, n: rFetch.n, bytes: rFetch.bytes })

  const rPost = await med('§BM3 POST /sql NODE MOVE', async () => {
    const t0 = performance.now()
    const r = await fetch(URL_BASE + '/sql', { method: 'POST', body: "NODE MOVE 'console.log(1)'" })
    const t = await r.text()
    return { ms: performance.now() - t0, bytes: t.length }
  }, Math.min(N, 15))
  linhas.push({ nome: rPost.nome, ms: rPost.ms, n: rPost.n, bytes: rPost.bytes })

  const canal = abrirCanalNode(PORT)
  await canal.pronto()
  const rWs = await med('§BM3 WS shellRemoto node', async () => {
    const t0 = performance.now()
    const out = await shellRemotoCanal(canal, SCRIPT, 'node')
    return { ms: performance.now() - t0, bytes: out.length }
  }, Math.min(N, 15))
  linhas.push({ nome: rWs.nome, ms: rWs.ms, n: rWs.n, bytes: rWs.bytes })

  const rAbs = await med('§BM3 wasm absorve+canal', async () => {
    const t0 = performance.now()
    const { out } = await absorveNodeCanal(SCRIPT, canal)
    return { ms: performance.now() - t0, bytes: out.length }
  }, Math.min(N, 10))
  linhas.push({ nome: rAbs.nome, ms: rAbs.ms, n: rAbs.n, bytes: rAbs.bytes })
  canal.fechar()
} catch (e) {
  console.log(`  §BM3 skip (servidor?): ${e.message}`)
}

console.log('\n--- TABELA (ms médio por operação) ---\n')
console.log('  operação'.padEnd(42) + 'ms'.padStart(10) + '  n'.padStart(6) + '  bytes')
console.log('  ' + '-'.repeat(62))
for (const r of linhas) {
  console.log(`  ${r.nome.padEnd(40)} ${String(r.ms).padStart(8)}  ${String(r.n).padStart(4)}  ${r.bytes ?? '-'}`)
}

console.log(`\n#TOTAL falhas ${falhas.length}`)
process.exit(falhas.length ? 1 : 0)
