/* mvp_ponta.js — e2e: dois bancos wasm, canal, DISCO — sem runtime externo.
 *
 *   node tools/sobe_wasm_shell.mjs    # recompila wasm com semântica
 *   node tests/mvp_ponta.js
 */
'use strict'

import { spawn } from 'node:child_process'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import net from 'node:net'
import { mkdirSync } from 'node:fs'
import { abrirCanalNode, shellRemotoCanal } from '../tools/canal_cliente.mjs'
import { absorveNodeCanal } from '../tools/mvp_absorve.mjs'
import { createCanalWatcher, medeParN1 } from '../tools/canal_watcher.mjs'
import { bandaDeTecido, tramaBump, tramaClara } from '../tools/banco_banda.mjs'
import { execMoveMetal } from '../tools/banco_metal.mjs'
import { S_CHUNK, S_NODE_IN, S_NODE_OUT } from '../tools/canal_slots.mjs'

const __dir = dirname(fileURLToPath(import.meta.url))
const RAIZ = join(__dir, '..')
const BANCO = join(RAIZ, 'app', 'banco')
const SQL_BASE = process.env.TIFFANY_SQL_BASE || join(RAIZ, '.torre', 'reino_mvp')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function esperaPorta (port, ms = 20000) {
  const t0 = Date.now()
  return new Promise((resolve, reject) => {
    function tenta () {
      const s = net.connect(port, '127.0.0.1', () => { s.end(); resolve() })
      s.on('error', () => {
        if (Date.now() - t0 > ms) reject(new Error('timeout porta ' + port))
        else setTimeout(tenta, 80)
      })
    }
    tenta()
  })
}

mkdirSync(SQL_BASE, { recursive: true })

const script = "console.log('mvp-ponta')"
const banda = bandaDeTecido()

// §M0 — banco wasm + DISCO (sem child_process)
try {
  const r = execMoveMetal(SQL_BASE, 'node', script)
  ok('§M0 node→erg→metal', String(r.stdout).trim() === 'mvp-ponta')
  ok('§M0 via c→wasm→erg→metal', r.meta?.via?.includes('c→wasm→erg→metal'))
  ok('§M0 passos fusão', r.meta?.passos > 0 && r.meta.passos <= 3000)
} catch (e) {
  ok('§M0 metal (cc/erg: ' + (e.message || e) + ')', false)
}

// §M1 — canal watcher → mesmo banco wasm DISCO
const frames = []
const handle = createCanalWatcher({ sqlBase: SQL_BASE, banda, broadcast: (b) => frames.push(Buffer.from(b)), bancoDir: BANCO })
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
const outBuf = Buffer.alloc(outLen)
let bi = 0
for (let pi = 0; bi < outLen && pi < parts.length; pi++) outBuf[bi++] = parts[pi] & 255
ok('§M1 canal→wasm DISCO', outBuf.toString('utf8').includes('mvp-ponta'))
ok('§M1 par N1', medeParN1(body.length))

// §M2 — servidor HTTP+WS + cliente wasm (reutiliza MVP_PORT se ligado)
const PORT = Number(process.env.MVP_PORT) || (5180 + Math.floor(Math.random() * 200))
let srv = null
if (!process.env.MVP_PORT) {
  srv = spawn(process.execPath, [join(RAIZ, 'tools', 'serve_banco.mjs')], {
    env: { ...process.env, PORT: String(PORT), TIFFANY_SQL_BASE: SQL_BASE },
    stdio: ['ignore', 'pipe', 'pipe'],
  })
}

try {
  await esperaPorta(PORT)
  const canal = abrirCanalNode(PORT)
  await canal.pronto()

  const viaCanal = await shellRemotoCanal(canal, script, 'node')
  ok('§M2 WS canal→banco wasm', viaCanal.includes('mvp-ponta'))

  const { out: viaWasm } = await absorveNodeCanal(script, canal)
  ok('§M2 cliente wasm MOVE+canal+MOVE', viaWasm.includes('mvp-ponta'))

  canal.fechar()
} catch (e) {
  ok('§M2 e2e', false)
  console.error(e)
} finally {
  if (srv) srv.kill()
}

console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
