/* canal_chunk.js — §C4 chunk S_CHUNK: par de bytes por trama, montagem no slot final.
 *
 *   node tests/canal_chunk.js
 */
'use strict'

const ISA_TECTO = 1 << 16
const S_CANAL = ISA_TECTO + (600 << 14)
const S_CHUNK = S_CANAL + 9102
const S_FRONT_RSP = S_CANAL + 9201

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function montaTexto (parts, len) {
  const buf = new Uint8Array(len)
  let bi = 0
  for (let pi = 0; bi < len && pi < parts.length; pi++) buf[bi++] = parts[pi] & 255
  return new TextDecoder().decode(buf)
}

function simulaEnvio (texto, slotFim) {
  const b = Buffer.from(texto, 'utf8')
  const tramas = []
  for (let i = 0; i < b.length; i += 2) {
    tramas.push({ slot: S_CHUNK, total: b[i], e: i + 1 < b.length ? b[i + 1] : 0 })
  }
  tramas.push({ slot: slotFim, total: b.length & 255, e: (b.length >> 8) & 255 })
  return tramas
}

function simulaRecebe (tramas, slotFim) {
  const parts = []
  let rsp = null
  for (const t of tramas) {
    if (t.slot === S_CHUNK) parts.push(t.total, t.e)
    if (t.slot === slotFim) rsp = t
  }
  const len = rsp.total + (rsp.e << 8)
  return montaTexto(parts, len)
}

{
  const msg = '<main class="bk">oi</main>'
  const tr = simulaEnvio(msg, S_FRONT_RSP)
  ok('§C4 slots chunk', tr.length >= 2 && tr[tr.length - 1].slot === S_FRONT_RSP)
  const volta = simulaRecebe(tr, S_FRONT_RSP)
  ok('§C4 monta corpo', volta === msg)
}

{
  const msg = 'a'.repeat(500)
  const tr = simulaEnvio(msg, S_FRONT_RSP)
  const volta = simulaRecebe(tr, S_FRONT_RSP)
  ok('§C4 corpo longo', volta === msg)
}

console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
