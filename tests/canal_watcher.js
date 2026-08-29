/* canal_watcher.js — watcher local + §N1 par negro/branco.

 *

 *   node tests/canal_watcher.js

 */

'use strict'

import { execMoveDisco } from '../tools/banco_shell_core.mjs'

import { createCanalWatcher, medeParN1 } from '../tools/canal_watcher.mjs'

import { bandaDeTecido, tramaBump, tramaClara, bump, keystream } from '../tools/banco_banda.mjs'

import { S_NODE_IN, S_NODE_OUT, S_CHUNK } from '../tools/canal_slots.mjs'

import { join, dirname } from 'node:path'

import { fileURLToPath } from 'node:url'

import { mkdirSync } from 'node:fs'



const __dir = dirname(fileURLToPath(import.meta.url))

const RAIZ = join(__dir, '..')

const SQL_BASE = join(RAIZ, '.torre', 'reino_canal_test')

const BANCO = join(RAIZ, 'app', 'banco')



let falhas = 0

let feitas = 0

function ok (q, cond) {

  feitas++

  if (!cond) falhas++

  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)

}



mkdirSync(SQL_BASE, { recursive: true })



const banda = bandaDeTecido()

const frames = []



const handle = createCanalWatcher({

  sqlBase: SQL_BASE,

  banda,

  broadcast: (b) => frames.push(Buffer.from(b)),

  bancoDir: BANCO,

})



const script = "console.log('canal')"

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



ok('§N1 watcher node absorção responde', out.toString('utf8').includes('canal'))

ok('§N1 par negro·branco = 1', medeParN1(body.length, outLen))



const bIn = tramaBump(S_NODE_IN, body.length & 255, (body.length >> 8) & 255, banda)

const ks = keystream(banda, 6)

const t1 = tramaClara(bIn, banda)

const bId = bump(bIn, ks)

ok('§N1 bump∘bump = id (Lei 1)', Buffer.compare(bIn, bump(bId, ks)) === 0)

ok('§N1 trama IN legível', t1.slot === S_NODE_IN && t1.total === (body.length & 255))



const r = execMoveDisco(SQL_BASE, 'node', 'console.log(1)')

ok('§N1 execMoveDisco meta slots', r.meta?.slotIn && r.meta?.slotOut)



console.log(`#TOTAL ${feitas} ${falhas}`)

process.exit(falhas ? 1 : 0)

