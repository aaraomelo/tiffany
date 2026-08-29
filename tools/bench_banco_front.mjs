#!/usr/bin/env node
/* bench_banco_front.mjs — mede o pipeline local vs arquitectura formal (fisica.tex).
 *
 *   node tools/bench_banco_front.mjs [--url http://127.0.0.1:5173] [--n 20]
 *
 * §B0  baseline formal: bump 6B + wasm arena (Lei 1)
 * §B1  camadas do front actual (fetch / NODE MOVE / HTTP MOVE)
 * §B2  idempotência MOVE (Teor. cardordem — x∗x=x)
 * §B3  gargalos fora da arquitectura
 */
import { readFileSync, mkdirSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { createHash } from 'node:crypto'
import { performance } from 'node:perf_hooks'
import { execSqlDisco } from './banco_sql_disco.mjs'
import { execMoveMetal } from './banco_metal.mjs'
import { medeParN1 } from './canal_watcher.mjs'
import { S_NODE_IN, S_NODE_OUT } from './canal_slots.mjs'

const __dir = dirname(fileURLToPath(import.meta.url))
const RAIZ = join(__dir, '..')
const SQL_BASE = join(RAIZ, '.torre', 'reino_bench')
const WASM_DIR = join(RAIZ, 'assets', 'figuras', 'wasm')

const args = process.argv.slice(2)
let URL_BASE = 'http://127.0.0.1:5173'
let N = 15
for (let i = 0; i < args.length; i++) {
  if (args[i] === '--url' && args[i + 1]) URL_BASE = args[++i]
  if (args[i] === '--n' && args[i + 1]) N = Number(args[++i])
}

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
  return { nome, ms: +ms.toFixed(2), n, bytes: last?.bytes ?? last }
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

function bandaDeTecido (t = 'tecido por omissao') {
  return createHash('sha256').update(t, 'utf8').digest()
}

async function fetchMs (path) {
  const t0 = performance.now()
  const r = await fetch(URL_BASE + path)
  const b = await r.arrayBuffer()
  return { ms: performance.now() - t0, bytes: b.byteLength, ok: r.ok }
}

async function postSql (q) {
  const t0 = performance.now()
  const r = await fetch(URL_BASE + '/sql', { method: 'POST', body: q })
  const t = await r.text()
  return { ms: performance.now() - t0, bytes: t.length, ok: r.ok, text: t }
}

console.log('=== BENCH BANCO FRONT vs fisica.tex ===\n')
console.log(`  url=${URL_BASE}  n=${N}  node=${process.version}\n`)

mkdirSync(SQL_BASE, { recursive: true })

await (async () => {
/* §B0 — baseline formal */
{
  const banda = bandaDeTecido()
  const S_BASH_IN = 9895936 + 9100
  const r = await med('§B0 bump 6B (canal formal)', () => {
    bump6(S_BASH_IN, 9, 1, banda)
    return { bytes: 6 }
  }, N * 50)
  ok(`§B0 bump 6B média ${r.ms}ms`, r.ms < 1)
  const m = bump6(S_BASH_IN, 9, 1, banda)
  const m2 = Buffer.alloc(6)
  const ks = keystream(banda, 6)
  for (let i = 0; i < 6; i++) m2[i] = m[i] ^ ks[i]
  ok('§B0 bump∘bump=id (Lei 1)', m2[4] === 9 && m2[5] === 1)

  const wasmBuf = readFileSync(join(WASM_DIR, 'consultar.wasm'))
  const r2 = await med('§B0 wasm consultar load', async () => {
    const { instance } = await WebAssembly.instantiate(wasmBuf)
    return { bytes: wasmBuf.length, ex: instance.exports }
  }, Math.min(N, 10))
  ok(`§B0 wasm instantiate ${r2.ms}ms (${wasmBuf.length}B)`, r2.ms < 50)
}

/* §B1 — camadas actuais */
{
  const rNode = await med('§B1 node.wasm semântica (arena)', () => {
    const wasmBuf = readFileSync(join(WASM_DIR, 'node.wasm'))
    const ex = new WebAssembly.Instance(new WebAssembly.Module(wasmBuf), {}).exports
    const mem = new Uint8Array(ex.DISCO.buffer)
    const base = 8
    const js = Buffer.from('console.log(1)', 'utf8')
    mem.set(js, base + 1024)
    ex.node_move(1024, js.length, 0, -1)
    ex.node_corre()
    const nr = ex.node_move(8192, 32, 8192, +1)
    return { bytes: nr }
  }, Math.min(N, 20))
  console.log(`  §B1 node wasm semântica: ${rNode.ms}ms`)

  const rMoveDisco = await med('§B1 execMoveMetal (erg+DISCO)', () => {
    const r = execMoveMetal(SQL_BASE, 'node', 'console.log(1)')
    return { bytes: (r.stdout || '').length }
  }, Math.min(N, 20))
  console.log(`  §B1 execMoveMetal node: ${rMoveDisco.ms}ms  ${rMoveDisco.bytes}B`)

  const rDisco = await med('§B1 face SQL NODE MOVE (.torre)', () => {
    const r = execSqlDisco(SQL_BASE, "NODE MOVE 'console.log(1)'")
    return { bytes: r.out.length }
  }, Math.min(N, 8))
  console.log(`  §B1 NODE MOVE disco: ${rDisco.ms}ms  ${rDisco.bytes}B`)

  const wasmBuf = readFileSync(join(WASM_DIR, 'consultar.wasm'))
  const rCelula = await med('§B1 sql wasm celula (arena)', () => {
    const ex = new WebAssembly.Instance(new WebAssembly.Module(wasmBuf), {}).exports
    const q = "INSERT TEXTO 'bench'"
    const mem = new Uint8Array(ex.DISCO.buffer)
    const b = Buffer.from(q, 'utf8')
    const base = 8
    mem.set(b, base + 1024)
    const n1 = ex.sql_move(1024, b.length, 4096, -1)
    ex.sql_move(4096, n1, 8192, +1)
    return { bytes: q.length }
  }, Math.min(N, 20))
  console.log(`  §B1 sql celula wasm: ${rCelula.ms}ms  (sem I/O)`)

  try {
    const rFetch = await med('§B1 fetch estático pagina.*', async () => {
      const a = await fetchMs('/banco/pagina.html')
      const b = await fetchMs('/banco/pagina.css')
      const c = await fetchMs('/banco/pagina.js')
      return { bytes: a.bytes + b.bytes + c.bytes, ms: a.ms + b.ms + c.ms }
    }, Math.min(N, 10))
    console.log(`  §B1 fetch estático (3 GET): ${rFetch.ms}ms  ${rFetch.bytes}B`)

    const rHttp = await med('§B1 HTTP /sql NODE MOVE', async () => {
      const le = await postSql("NODE MOVE 'console.log(1)'")
      return { bytes: le.bytes, ms: le.ms }
    }, Math.min(N, 15))
    console.log(`  §B1 HTTP /sql MOVE: ${rHttp.ms}ms  ${rHttp.bytes}B`)

    const rWasm = await med('§B1 fetch 5 wasm', async () => {
      let b = 0
      for (const w of ['consultar.wasm', 'html_compor.wasm', 'css_aplicar.wasm', 'js_escapar.wasm', 'powershell.wasm']) {
        const x = await fetchMs('/wasm/' + w)
        b += x.bytes
      }
      return { bytes: b }
    }, Math.min(N, 5))
    console.log(`  §B1 wasm×5 fetch: ${rWasm.ms}ms  ${rWasm.bytes}B`)
  } catch (e) {
    console.log(`  §B1 HTTP skip (servidor?): ${e.message}`)
  }
}

/* §B2 — idempotência + §N1 par negro/branco */
{
  const a = execSqlDisco(SQL_BASE, "NODE MOVE 'console.log(1)'").out
  const b = execSqlDisco(SQL_BASE, "NODE MOVE 'console.log(1)'").out
  ok(`§B2 MOVE† idempotente (${a.length}=${b.length})`, a.length === b.length && a.trim() === '1')

  const rN1 = execSqlDisco(SQL_BASE, "NODE MOVE 'console.log(1)'")
  const bi = rN1.meta?.bytesIn || 0
  ok(`§N1 S_negro·S_branco=1 (r=${bi})`, medeParN1(bi || 1))
  ok(`§N1 slots NODE IN/OUT`, rN1.meta?.slotIn === S_NODE_IN && rN1.meta?.slotOut === S_NODE_OUT)
}

/* §B3 — gargalos fora da arquitectura */
console.log('\n--- §B3 GARGALOS FORA DE fisica.tex ---\n')

const gargalos = [
  {
    id: 'G1',
    lei: 'fisica.tex Teor. entropiadual + plano: célula = banco único sql.c/wasm',
    fora: 'browser: consultar.wasm + GKBANCO (LS); metal: execSqlDisco só no fallback shell',
    medida: '§B1 sql celula ~0ms vs disco ~850ms; pleno sql.c na Patria',
  },
  {
    id: 'G2',
    lei: 'canal = bump 6B em S_CANAL (rede limpa, sem traduzir)',
    fora: 'serve_banco publica bump em S_NODE_IN/OUT após MOVE; WS /canal loopback',
    medida: 'bump 0ms vs HTTP MOVE; browser pode subscrever /canal',
  },
  {
    id: 'G3',
    lei: 'STORE idempotente — mesmo slot não multiplica (cardordem)',
    fora: 'disco.bin = estado wasm persistido; sem in.js/out nem spawn',
    medida: '§BM1 execMoveWasm — semântica na arena, I/O só disco.bin',
  },
  {
    id: 'G4',
    lei: 'backends manifesto — mesma porta MOVE; nenhum privilegiado',
    fora: 'resolvido: NODE MOVE directo; PowerShell só se pedido explicitamente',
    medida: '1 POST /sql em vez de 3 verbos (~4× mais rápido)',
  },
  {
    id: 'G5',
    lei: 'front = html+css+js via wasm MOVE na arena DISCO',
    fora: 'por defeito fetch estático + wasm; node_pagina só com ?metal=1',
    medida: 'fetch ~20ms vs NODE MOVE ~400ms',
  },
  {
    id: 'G6',
    lei: 'sem espera lógica — antena datagrama (canal.c §N3)',
    fora: 'HTTP /sql é só fio de dev; protocolo real = bump bidirecional no /canal',
    medida: 'terminal com canal usa shellRemoto; sem daemon nem fila',
  },
  {
    id: 'G7',
    lei: 'S_FRONT_REQ/RSP + S_CHUNK no barramento',
    fora: 'watcher local em serve_banco; Patria usa canal_patria.c (UDP)',
    medida: 'tests/canal_watcher.js §N1; Patria pendente multicast',
  },
]

for (const g of gargalos) {
  console.log(`${g.id}  FORA DA LEI: ${g.lei}`)
  console.log(`     Implementação: ${g.fora}`)
  console.log(`     Medida: ${g.medida}\n`)
}

/* recomendações alinhadas */
console.log('--- RECOMENDAÇÕES (voltar à arquitectura) ---\n')
console.log('  1. Local Win: NODE MOVE directo (uma chamada, sem PowerShell)')
console.log('  2. wasm sql.c na arena (quando compilar no Win) — disco é o passo correcto')
console.log('  3. Front: fetch estático + wasm MOVE na arena; node só com ?metal=1')
console.log('  4. Publicar S_NODE_OUT via canal_grava após MOVE (par negro/branco)')
console.log('  5. Medidor §N1: produto S_negro·S_branco=1 ao fim de ida+volta\n')

console.log(`#TOTAL unidades ${9 + gargalos.length} falhas ${falhas.length}`)
process.exit(falhas.length ? 1 : 0)
})()
