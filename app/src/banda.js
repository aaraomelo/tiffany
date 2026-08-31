// banda.js — bump = msg ⊕ keystream(banda), banda = sha256(tecido). Igual a lib/banda.h.

export async function sha256 (bytes) {
  const dig = await crypto.subtle.digest('SHA-256', bytes)
  return new Uint8Array(dig)
}

/** Tecido → banda (32 bytes). Omissão = a do sql.c / canal.c. */
export async function bandaDeTecido (tecido = 'tecido por omissao') {
  return sha256(new TextEncoder().encode(tecido))
}

export function parseChavePublica (s) {
  const t = String(s || '').replace(/[\s:]+/g, '')
  if (!t) return null
  if (/^[0-9a-fA-F]+$/.test(t) && t.length % 2 === 0) {
    const out = new Uint8Array(t.length / 2)
    for (let i = 0; i < t.length; i += 2) out[i >> 1] = parseInt(t.slice(i, i + 2), 16)
    return out
  }
  return null
}

/** Chave pública (hex) → banda = sha256(bytes). URL ?pub= liga o utilizador da chave. */
export async function bandaDeChavePublica (pub) {
  const raw = typeof pub === 'string' ? parseChavePublica(pub) : pub
  if (!raw || !raw.length) throw new Error('chave publica vazia')
  return sha256(raw)
}

export async function bandaDeAssinatura (assinatura) {
  return sha256(new TextEncoder().encode(assinatura))
}

/** keystream = sha256(banda||contador) em blocos de 32 */
export async function keystream (banda, n) {
  const ks = new Uint8Array(n)
  const sem = new Uint8Array(36)
  sem.set(banda, 0)
  for (let o = 0; o < n; o += 32) {
    const ctr = (o / 32) >>> 0
    sem[32] = ctr & 255
    sem[33] = (ctr >>> 8) & 255
    sem[34] = (ctr >>> 16) & 255
    sem[35] = (ctr >>> 24) & 255
    const bloco = await sha256(sem)
    ks.set(bloco.subarray(0, Math.min(32, n - o)), o)
  }
  return ks
}

/** Bump: XOR. Involução — bump∘bump = id (Lei 1). */
export function bump (ent, ks) {
  const sai = new Uint8Array(ent.length)
  for (let i = 0; i < ent.length; i++) sai[i] = ent[i] ^ ks[i]
  return sai
}

/** Selo do corpo (mesmo keystream da trama, comprimento do blob). Involução. */
export async function selaBlob (bytes, banda) {
  const u8 = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes)
  return bump(u8, await keystream(banda, u8.length))
}

/** Abre o selo — bump∘bump = id. */
export async function abreBlob (blob, banda) {
  return selaBlob(blob, banda)
}

export function hex16 (banda) {
  return [...banda.slice(0, 16)].map(b => b.toString(16).padStart(2, '0')).join('')
}
