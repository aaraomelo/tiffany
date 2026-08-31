// canal_browser.js — trama S_CANAL (6 bytes) com bump na banda própria.
// Mesmo contrato que banco/sql.c canal_grava / canal_le — não é TFAL, não traduz protocolo.
//
// Trama clara: slot(4, LE) + Word.total + Word.e
// No fio: trama ⊕ keystream(banda)

import { bandaDeTecido, bandaDeChavePublica, keystream, bump, hex16, selaBlob, abreBlob } from './banda.js'

export const TRAMA_LEN = 6
const ISA_TECTO = 1 << 16
const ZBITS = 14
const ZONA = (k) => k << ZBITS
const ZONA_CANAL = 600

/** Igual a #define S_CANAL em banco/sql.c */
export const S_CANAL = ISA_TECTO + ZONA(ZONA_CANAL)
export const S_BASH_IN = S_CANAL + 9100
export const S_BASH_OUT = S_CANAL + 9101
export const S_PWSH_IN = S_CANAL + 9110
export const S_PWSH_OUT = S_CANAL + 9111
export const S_NODE_IN = S_CANAL + 9120
export const S_NODE_OUT = S_CANAL + 9121
/** Par de bytes por trama bump (Word.total, Word.e). */
export const S_CHUNK = S_CANAL + 9102
/** Pede pagina.{html,css,js} ao banco remoto (total=kind). */
export const S_FRONT_REQ = S_CANAL + 9200
export const S_FRONT_RSP = S_CANAL + 9201
/** GKBANCO: total=0 puxa; nin>0 empurra JSON (corpo em S_CHUNK). */
export const S_ESTADO_REQ = S_CANAL + 9210
export const S_ESTADO_RSP = S_CANAL + 9211
/** Depósito opaco de utilizador. ≠ S_ESTADO. Pátria grava bytes; o browser sela. */
export const S_DEPOSITO_REQ = S_CANAL + 9220
export const S_DEPOSITO_RSP = S_CANAL + 9221
/** Contrato N-partes. Chunk próprio — o watcher não acumula no S_CHUNK do shell. */
export const S_COORD = S_CANAL + 9230
export const S_COORD_CHUNK = S_CANAL + 9232
/** Worker i → coord: par próprio, para as peças não se entrelaçarem no bus. */
export function slotPeca (i) { return S_CANAL + 9240 + ((i | 0) * 2) }
export function slotPecaChunk (i) { return S_CANAL + 9241 + ((i | 0) * 2) }

const FRONT_KIND = { html: 0, css: 1, js: 2 }

function montaTexto (parts, len) {
  const buf = new Uint8Array(len)
  let bi = 0
  for (let pi = 0; bi < len && pi < parts.length; pi++) {
    buf[bi++] = parts[pi] & 255
  }
  return new TextDecoder().decode(buf)
}

/** Envia bytes em pares (S_CHUNK) e sinaliza no slot final. */
export async function enviaBytes (canal, slotFim, bytes, chunkSlot = S_CHUNK) {
  const b = bytes instanceof Uint8Array ? bytes : new TextEncoder().encode(String(bytes))
  for (let i = 0; i < b.length; i += 2) {
    await canal.grava(chunkSlot, b[i], i + 1 < b.length ? b[i + 1] : 0)
  }
  await canal.grava(slotFim, b.length & 255, (b.length >> 8) & 255)
}

/** Envia texto em pares de bytes (S_CHUNK) e sinaliza no slot final. */
export async function enviaChunks (canal, slotFim, texto) {
  return enviaBytes(canal, slotFim, new TextEncoder().encode(texto))
}

/** Espera slotFim e monta bytes a partir dos S_CHUNK. */
export async function recebeBytes (canal, slotFim, timeoutMs = 20000, chunkSlot = S_CHUNK) {
  const parts = []
  const off = canal.on(chunkSlot, ({ total, e }) => {
    parts.push(total, e)
  })
  try {
    const rsp = await canal.le(slotFim, timeoutMs)
    const len = rsp.total + (rsp.e << 8)
    const buf = new Uint8Array(len)
    for (let i = 0; i < len && i < parts.length; i++) buf[i] = parts[i] & 255
    return buf
  } finally {
    off()
  }
}

/** Espera slotFim e monta corpo a partir dos S_CHUNK recebidos entretanto. */
export async function recebeCorpo (canal, slotFim, timeoutMs = 20000) {
  const parts = []
  const off = canal.on(S_CHUNK, ({ total, e }) => {
    parts.push(total, e)
  })
  try {
    const rsp = await canal.le(slotFim, timeoutMs)
    const len = rsp.total + (rsp.e << 8)
    return montaTexto(parts, len)
  } finally {
    off()
  }
}

/** Empurra blob selado (S_DEPOSITO). Sem a banda o disco remoto não é JSON. */
export async function depositaBlob (canal, bytes, banda) {
  const u8 = bytes instanceof Uint8Array ? bytes : new TextEncoder().encode(String(bytes))
  const selado = await selaBlob(u8, banda)
  const waiter = recebeBytes(canal, S_DEPOSITO_RSP)
  await enviaBytes(canal, S_DEPOSITO_REQ, selado)
  return waiter
}

/** Puxa o depósito e abre o selo. Vazio = 0 bytes (sem GKBANCO inventado). */
export async function puxaDeposito (canal, banda) {
  const waiter = recebeBytes(canal, S_DEPOSITO_RSP)
  await canal.grava(S_DEPOSITO_REQ, 0, 0)
  const blob = await waiter
  if (!blob || !blob.length) return new Uint8Array(0)
  return abreBlob(blob, banda)
}

/** GET CORPO remoto: kind = html | css | js. */
export async function frontPedeRemoto (canal, tipo) {
  const kind = FRONT_KIND[tipo]
  if (kind === undefined) throw new Error('tipo: html|css|js')
  const waiter = recebeCorpo(canal, S_FRONT_RSP)
  await canal.grava(S_FRONT_REQ, kind, 0)
  return waiter
}

/** Empacota e bump-ia (lado negro — emite). */
export async function tramaBump (slot, total, e, banda) {
  const m = new Uint8Array(TRAMA_LEN)
  new DataView(m.buffer).setUint32(0, slot >>> 0, true)
  m[4] = total & 255
  m[5] = e & 255
  const ks = await keystream(banda, TRAMA_LEN)
  return bump(m, ks)
}

/** Desempacota após bump (lado branco — recebe). */
export async function tramaClara (buf, banda) {
  if (buf.length < TRAMA_LEN) throw new Error('trama curta')
  const ks = await keystream(banda, TRAMA_LEN)
  const m = bump(buf.subarray(0, TRAMA_LEN), ks)
  const slot = new DataView(m.buffer, m.byteOffset).getUint32(0, true)
  return { slot, total: m[4], e: m[5] }
}

/**
 * Sessão no barramento: WS /canal transporta bytes bump-ados (6), sem alterar.
 * O Vite é só o fio até UDP multicast — o protocolo é bump+banda.
 */
export function abrirCanal (opts = {}) {
  const tecido = opts.tecido ?? 'tecido por omissao'
  const url = opts.url ?? ((location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/canal')
  let banda = opts.banda ?? null
  const pub = opts.pub || null
  let ws = null
  const ouvintes = new Map() // slot → Set<fn>

  function emit (slot, total, e) {
    const set = ouvintes.get(slot)
    if (!set) return
    for (const fn of set) fn({ slot, total, e })
  }

  async function ligaBanda () {
    if (banda) return
    if (pub) banda = await bandaDeChavePublica(pub)
    else banda = await bandaDeTecido(tecido)
  }

  async function connect () {
    if (ws && ws.readyState === 1) return
    await ligaBanda()
    ws = new WebSocket(url)
    ws.binaryType = 'arraybuffer'
    await new Promise((resolve, reject) => {
      ws.onopen = resolve
      ws.onerror = () => reject(new Error('canal: falha ao abrir ' + url))
    })
    ws.onmessage = async (ev) => {
      const buf = new Uint8Array(ev.data)
      if (buf.length !== TRAMA_LEN) return
      try {
        const t = await tramaClara(buf, banda)
        emit(t.slot, t.total, t.e)
      } catch (e) { /* banda errada = ruído */ }
    }
    ws.onclose = () => { ws = null }
  }

  return {
    async grava (slot, total, e) {
      await connect()
      const frame = await tramaBump(slot, total, e, banda)
      ws.send(frame)
    },

    /** Espera uma trama para o slot (timeout ms). */
    async le (slot, timeoutMs = 3000) {
      await connect()
      return new Promise((resolve, reject) => {
        const to = setTimeout(() => {
          ouvintes.get(slot)?.delete(on)
          reject(new Error('canal: timeout slot ' + slot))
        }, timeoutMs)
        function on (t) {
          clearTimeout(to)
          ouvintes.get(slot)?.delete(on)
          resolve(t)
        }
        if (!ouvintes.has(slot)) ouvintes.set(slot, new Set())
        ouvintes.get(slot).add(on)
      })
    },

    on (slot, fn) {
      if (!ouvintes.has(slot)) ouvintes.set(slot, new Set())
      ouvintes.get(slot).add(fn)
      return () => { ouvintes.get(slot)?.delete(fn) }
    },

    async banda () {
      await ligaBanda()
      return banda
    },
    bandaHex () { return banda ? hex16(banda) : null },
    fechar () { if (ws) ws.close(); ws = null }
  }
}
