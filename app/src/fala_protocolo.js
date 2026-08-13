// fala_protocolo.js — Protocolo TFAL no browser (banda = sha256(Assinatura)).
// Cada cliente: Assinatura(corpo) → banda → bump. Sem API de grafo.

const MAGIC = new TextEncoder().encode('TFAL')
export const FALA_HELLO = 1
export const FALA_FALA = 2
export const FALA_RESPOSTA = 3
export const FALA_APRENDE = 4
export const FALA_NAO_SEI = 5
export const FALA_ERR = 255
const HDR = 14

function be32 (n) {
  return new Uint8Array([(n >>> 24) & 255, (n >>> 16) & 255, (n >>> 8) & 255, n & 255])
}
function rb32 (u8, o) {
  return ((u8[o] << 24) | (u8[o + 1] << 16) | (u8[o + 2] << 8) | u8[o + 3]) >>> 0
}

/** Assinatura do corpo do cliente → banda (32 bytes). O hash É a assinatura. */
export async function bandaDeAssinatura (assinatura) {
  const data = new TextEncoder().encode(assinatura)
  const dig = await crypto.subtle.digest('SHA-256', data)
  return new Uint8Array(dig)
}

export function hex16 (banda) {
  return [...banda.slice(0, 16)].map(b => b.toString(16).padStart(2, '0')).join('')
}

async function sha256 (bytes) {
  return new Uint8Array(await crypto.subtle.digest('SHA-256', bytes))
}

/** keystream = sha256(banda||contador) em blocos de 32 — igual a lib/banda.h */
async function keystream (banda, n) {
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

function bump (ent, ks) {
  const sai = new Uint8Array(ent.length)
  for (let i = 0; i < ent.length; i++) sai[i] = ent[i] ^ ks[i]
  return sai
}

export async function empacota (op, seq, banda, texto) {
  const plain = new TextEncoder().encode(texto || '')
  const payload = (banda && op !== FALA_HELLO)
    ? bump(plain, await keystream(banda, plain.length))
    : plain
  const out = new Uint8Array(HDR + payload.length)
  out.set(MAGIC, 0)
  out[4] = 1
  out[5] = op
  out.set(be32(seq), 6)
  out.set(be32(payload.length), 10)
  out.set(payload, HDR)
  return out
}

export async function desempacota (buf, banda) {
  if (buf.length < HDR) throw new Error('frame curto')
  if (buf[0] !== 84 || buf[1] !== 70 || buf[2] !== 65 || buf[3] !== 76)
    throw new Error('magic')
  if (buf[4] !== 1) throw new Error('ver')
  const op = buf[5]
  const seq = rb32(buf, 6)
  const len = rb32(buf, 10)
  if (HDR + len > buf.length) throw new Error('len')
  const raw = buf.subarray(HDR, HDR + len)
  const plain = (banda && op !== FALA_HELLO)
    ? bump(raw, await keystream(banda, len))
    : raw
  return { op, seq, texto: new TextDecoder().decode(plain) }
}

/**
 * Assinatura canónica do corpo no front: soma medida por tools/corpo.sh.
 * É a identidade do cliente — a banda sai dela.
 */
export function assinaturaDoCorpo (corpoJson) {
  const soma = (corpoJson && corpoJson.soma) || 'sem-soma'
  return `Tiffany-Assinatura/1\nsoma=${soma}\n`
}

/** Sessão sobre WebSocket binário (Vite /antena → TCP fala). */
export function abrirAntena (url = (location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/antena') {
  let seq = 0
  let banda = null
  let ws = null
  const pendente = []

  function waitFrame () {
    return new Promise((resolve, reject) => {
      pendente.push({ resolve, reject })
    })
  }

  async function connect () {
    if (ws && ws.readyState === 1) return
    ws = new WebSocket(url)
    ws.binaryType = 'arraybuffer'
    await new Promise((resolve, reject) => {
      ws.onopen = resolve
      ws.onerror = () => reject(new Error('antena: falha ao abrir'))
    })
    ws.onmessage = async (ev) => {
      const p = pendente.shift()
      if (!p) return
      try {
        p.resolve(await desempacota(new Uint8Array(ev.data), banda))
      } catch (e) { p.reject(e) }
    }
    ws.onclose = () => {
      while (pendente.length) pendente.shift().reject(new Error('antena fechada'))
      ws = null
    }
  }

  async function enviar (op, texto) {
    await connect()
    seq += 1
    const frame = await empacota(op, seq, banda, texto)
    ws.send(frame)
    return waitFrame()
  }

  return {
    async hello (assinatura) {
      banda = await bandaDeAssinatura(assinatura)
      return enviar(FALA_HELLO, assinatura)
    },
    async fala (texto) {
      if (!banda) throw new Error('hello primeiro')
      return enviar(FALA_FALA, texto)
    },
    async aprende (fala, resposta) {
      if (!banda) throw new Error('hello primeiro')
      return enviar(FALA_APRENDE, fala + '\t' + resposta)
    },
    bandaHex () { return banda ? hex16(banda) : null },
    fechar () { if (ws) ws.close(); ws = null }
  }
}
