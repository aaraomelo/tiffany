/* canal_watcher.js — watcher local + §N1 par negro/branco.

 *

 *   node tests/canal_watcher.js

 */

'use strict'

import { execMoveDisco } from '../tools/banco_shell_core.mjs'

import { createCanalWatcher, medeParN1, medeIdaCanal, framesDeCorpo } from '../tools/canal_watcher.mjs'

import { bandaDeTecido, tramaBump, tramaClara, bump, keystream } from '../tools/banco_banda.mjs'

import { S_NODE_IN, S_NODE_OUT, S_CHUNK, S_PWSH_IN, S_PWSH_OUT } from '../tools/canal_slots.mjs'

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

let nIn = 0

for (let i = 0; i < body.length; i += 2) {

  handle(tramaBump(S_CHUNK, body[i], i + 1 < body.length ? body[i + 1] : 0, banda))

  nIn++

}

handle(tramaBump(S_NODE_IN, body.length & 255, (body.length >> 8) & 255, banda))

nIn++



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

ok('§N1 produto r·r^{-1} = 1 (negro.c)', medeParN1(body.length))

ok('§N1 ida: negro emitiu = branco absorveu', medeIdaCanal(nIn, framesDeCorpo(body.length)))

ok('§N1 volta: branco emitiu = negro absorveu', medeIdaCanal(frames.length, framesDeCorpo(outLen)))



const bIn = tramaBump(S_NODE_IN, body.length & 255, (body.length >> 8) & 255, banda)

const ks = keystream(banda, 6)

const t1 = tramaClara(bIn, banda)

const bId = bump(bIn, ks)

ok('§N1 bump∘bump = id (Lei 1)', Buffer.compare(bIn, bump(bId, ks)) === 0)

ok('§N1 trama IN legível', t1.slot === S_NODE_IN && t1.total === (body.length & 255))



const r = execMoveDisco(SQL_BASE, 'node', 'console.log(1)')

ok('§N1 execMoveDisco meta slots', r.meta?.slotIn && r.meta?.slotOut)



const framesPs = []

const handlePs = createCanalWatcher({

  sqlBase: SQL_BASE,

  banda,

  broadcast: (b) => framesPs.push(Buffer.from(b)),

  bancoDir: BANCO,

})

const psScript = "Write-Output 'canal'"

const psBody = Buffer.from(psScript, 'utf8')

let nInPs = 0

for (let i = 0; i < psBody.length; i += 2) {

  handlePs(tramaBump(S_CHUNK, psBody[i], i + 1 < psBody.length ? psBody[i + 1] : 0, banda))

  nInPs++

}

handlePs(tramaBump(S_PWSH_IN, psBody.length & 255, (psBody.length >> 8) & 255, banda))

nInPs++

let outLenPs = 0

const partsPs = []

for (const f of framesPs) {

  const t = tramaClara(f, banda)

  if (t.slot === S_CHUNK) partsPs.push(t.total, t.e)

  if (t.slot === S_PWSH_OUT) outLenPs = t.total + (t.e << 8)

}

const outPs = Buffer.alloc(outLenPs)

let bj = 0

for (let pi = 0; bj < outLenPs && pi < partsPs.length; pi++) outPs[bj++] = partsPs[pi] & 255

ok('§N1 watcher powershell absorção responde', outPs.toString('utf8').toLowerCase().includes('canal'))

ok('§N1 powershell ida negro=branco', medeIdaCanal(nInPs, framesDeCorpo(psBody.length)))

ok('§N1 powershell volta negro=branco', medeIdaCanal(framesPs.length, framesDeCorpo(outLenPs)))



console.log(`#TOTAL ${feitas} ${falhas}`)

process.exit(falhas ? 1 : 0)

