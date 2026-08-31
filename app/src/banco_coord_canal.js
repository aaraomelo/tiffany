// banco_coord_canal.js — contrato N-partes no WSS /canal.
// Não é a Cadeia. Não é S_ESTADO. Chunks fora do S_CHUNK do shell.

import {
  enviaBytes, S_COORD, S_COORD_CHUNK, slotPeca, slotPecaChunk,
} from './canal_browser.js'

export { S_COORD, S_COORD_CHUNK, slotPeca, slotPecaChunk }

function parseMsg (parts, len) {
  const buf = new Uint8Array(len)
  for (let i = 0; i < len && i < parts.length; i++) buf[i] = parts[i] & 255
  return JSON.parse(new TextDecoder().decode(buf))
}

export async function enviaMsg (canal, slotFim, chunkSlot, obj) {
  const bytes = new TextEncoder().encode(JSON.stringify(obj))
  await enviaBytes(canal, slotFim, bytes, chunkSlot)
}

export async function enviaAbre (canal, obj) {
  await enviaMsg(canal, S_COORD, S_COORD_CHUNK, { tipo: 'abre', ...obj })
}

export async function enviaFecho (canal, obj) {
  await enviaMsg(canal, S_COORD, S_COORD_CHUNK, { tipo: 'fecho', ...obj })
}

export async function enviaPeca (canal, i, obj) {
  await enviaMsg(canal, slotPeca(i), slotPecaChunk(i), { tipo: 'peca', i: i | 0, ...obj })
}

export function ouveSlot (canal, slotFim, chunkSlot, onMsg) {
  const parts = []
  const offC = canal.on(chunkSlot, ({ total, e }) => { parts.push(total, e) })
  const offF = canal.on(slotFim, ({ total, e }) => {
    const len = total + (e << 8)
    const copy = parts.slice()
    parts.length = 0
    try { onMsg(parseMsg(copy, len)) } catch { /* ruído / banda */ }
  })
  return () => { offC(); offF() }
}

export function ouveCoord (canal, onMsg) {
  return ouveSlot(canal, S_COORD, S_COORD_CHUNK, onMsg)
}

export function ouvePeca (canal, i, onMsg) {
  return ouveSlot(canal, slotPeca(i), slotPecaChunk(i), onMsg)
}
