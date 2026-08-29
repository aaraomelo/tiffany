/* canal_browser.js — trama S_CANAL bump-ada no browser (§C0–§C3).
 *
 *   node tests/canal_browser.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { createHash } = require('crypto')

const RAIZ = path.join(__dirname, '..')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function sha256 (buf) {
  return createHash('sha256').update(buf).digest()
}

function bandaDeTecido (tecido) {
  return sha256(Buffer.from(tecido, 'utf8'))
}

function keystream (banda, n) {
  const ks = Buffer.alloc(n)
  const sem = Buffer.alloc(36)
  banda.copy(sem, 0)
  for (let o = 0; o < n; o += 32) {
    const ctr = Math.floor(o / 32)
    sem.writeUInt32LE(ctr, 32)
    const bloco = sha256(sem)
    bloco.copy(ks, o, 0, Math.min(32, n - o))
  }
  return ks
}

function bump (ent, ks) {
  const sai = Buffer.alloc(ent.length)
  for (let i = 0; i < ent.length; i++) sai[i] = ent[i] ^ ks[i]
  return sai
}

const ISA_TECTO = 1 << 16
const S_CANAL = ISA_TECTO + (600 << 14)
const S_BASH_IN = S_CANAL + 9100
const S_CHUNK = S_CANAL + 9102
const S_FRONT_REQ = S_CANAL + 9200
const S_FRONT_RSP = S_CANAL + 9201

function tramaBump (slot, total, e, banda) {
  const m = Buffer.alloc(6)
  m.writeUInt32LE(slot >>> 0, 0)
  m[4] = total & 255
  m[5] = e & 255
  return bump(m, keystream(banda, 6))
}

function tramaClara (buf, banda) {
  const m = bump(buf, keystream(banda, 6))
  return { slot: m.readUInt32LE(0), total: m[4], e: m[5] }
}

(async () => {
  const banda = bandaDeTecido('tecido por omissao')

  ok('§C0 S_CANAL bate sql.c', S_CANAL === 9895936)
  ok('§C0 S_BASH_IN', S_BASH_IN === S_CANAL + 9100)
  ok('§C4 S_CHUNK', S_CHUNK === S_CANAL + 9102)
  ok('§C4 S_FRONT_REQ/RSP', S_FRONT_REQ === S_CANAL + 9200 && S_FRONT_RSP === S_CANAL + 9201)

  const b1 = tramaBump(S_BASH_IN, 9, 1, banda)
  ok('§C1 trama tem 6 bytes', b1.length === 6)

  const t1 = tramaClara(b1, banda)
  ok('§C1 volta slot', t1.slot === S_BASH_IN)
  ok('§C1 volta Word', t1.total === 9 && t1.e === 1)

  const b2 = bump(b1, keystream(banda, 6))
  const t2 = tramaClara(b2, banda)
  ok('§C2 bump∘bump = id (Lei 1)', t2.total === 9 && t2.e === 1)

  const bandaErr = bandaDeTecido('outro tecido')
  const t3 = tramaClara(b1, bandaErr)
  ok('§C3 banda errada ≠ claro', !(t3.total === 9 && t3.e === 1 && t3.slot === S_BASH_IN))

  const wasm = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'interpretar.wasm')
  ok('§C3 interpretar.wasm existe', fs.existsSync(wasm))

  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})()
