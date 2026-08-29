// banda.js — bump = msg ⊕ keystream(banda), banda = sha256(tecido). Igual a lib/banda.h.

export async function sha256 (bytes) {
  const dig = await crypto.subtle.digest('SHA-256', bytes)
  return new Uint8Array(dig)
}

/** Tecido → banda (32 bytes). Omissão = a do sql.c / canal.c. */
export async function bandaDeTecido (tecido = 'tecido por omissao') {
  return sha256(new TextEncoder().encode(tecido))
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

export function hex16 (banda) {
  return [...banda.slice(0, 16)].map(b => b.toString(16).padStart(2, '0')).join('')
}
