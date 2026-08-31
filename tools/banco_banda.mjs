/* banco_banda.mjs — bump na banda (sync, Node metal). Igual a app/src/banda.js */
import { createHash } from 'node:crypto'

export function bandaDeTecido (tecido = 'tecido por omissao') {
  return createHash('sha256').update(tecido, 'utf8').digest()
}

/** Chave pública hex → banda = sha256(bytes). Mesmo bump que o tecido. */
export function bandaDeChave (hex) {
  const t = String(hex || '').replace(/[\s:]+/g, '')
  if (!t || t.length % 2) throw new Error('chave publica: hex impar')
  const raw = Buffer.from(t, 'hex')
  if (!raw.length) throw new Error('chave publica vazia')
  return createHash('sha256').update(raw).digest()
}

export function keystream (banda, n) {
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

export function bump (ent, ks) {
  const sai = Buffer.alloc(ent.length)
  for (let i = 0; i < ent.length; i++) sai[i] = ent[i] ^ ks[i]
  return sai
}

/** Selo do corpo (mesmo keystream da trama). Involução. ≠ domain separator do id. */
export function selaBlob (bytes, banda) {
  const b = Buffer.isBuffer(bytes) ? bytes : Buffer.from(bytes)
  return bump(b, keystream(banda, b.length))
}

export function abreBlob (blob, banda) {
  return selaBlob(blob, banda)
}

export function tramaBump (slot, total, e, banda) {
  const m = Buffer.alloc(6)
  m.writeUInt32LE(slot >>> 0, 0)
  m[4] = total & 255
  m[5] = e & 255
  return bump(m, keystream(banda, 6))
}

export function tramaClara (buf, banda) {
  const m = bump(Buffer.from(buf), keystream(banda, 6))
  return { slot: m.readUInt32LE(0), total: m[4], e: m[5] }
}

export function publicaMeta (broadcast, meta, banda) {
  if (!meta || !broadcast) return
  if (meta.slotIn && meta.bytesIn) {
    broadcast(tramaBump(meta.slotIn, meta.bytesIn & 255, (meta.bytesIn >> 8) & 255, banda))
  }
  if (meta.slotOut && meta.bytesOut) {
    broadcast(tramaBump(meta.slotOut, meta.bytesOut & 255, (meta.bytesOut >> 8) & 255, banda))
  }
}
