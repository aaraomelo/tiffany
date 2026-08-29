/* wasm_para_erg.mjs — wasm (pilha) → assembly ERG-64 (registadores).
 * Mesma correspondência que chessb.c §C4 / tests/chessb.c TRADUZ[].
 */
import { wasmParaErgCompleto } from './wasm_erg.mjs'

export function lebRead (buf, pos) {
  let v = 0n
  let s = 0n
  let p = pos
  while (p < buf.length) {
    const c = BigInt(buf[p++])
    v |= (c & 0x7fn) << s
    if ((c & 0x80n) === 0n) return { v: Number(v), next: p }
    s += 7n
    if (s > 63n) break
  }
  throw new Error('LEB inválido')
}

function lebWrite (v) {
  const out = []
  let n = BigInt(v)
  do {
    let c = Number(n & 0x7fn)
    n >>= 7n
    if (n) c |= 0x80
    out.push(c)
  } while (n)
  return Uint8Array.from(out)
}

/** @param {Uint8Array} buf */
export function parseWasm (buf) {
  if (buf.length < 8 || buf[1] !== 0x61 || buf[2] !== 0x73 || buf[3] !== 0x6d) {
    throw new Error('não é wasm')
  }
  const sections = []
  let p = 8
  while (p < buf.length) {
    const id = buf[p++]
    const { v: size, next } = lebRead(buf, p)
    p = next
    sections.push({ id, body: buf.subarray(p, p + size) })
    p += size
  }
  return sections
}

const MAP = {
  0x20: { erg: 'LOAD', args: 1, note: 'local.get' },
  0x21: { erg: 'STORE', args: 1, note: 'local.set' },
  0x41: { erg: null, args: 1, note: 'i32.const' },
  0x6a: { erg: 'ADD', args: 0 },
  0x6b: { erg: 'SUB', args: 0 },
  0x71: { erg: 'AND', args: 0 },
  0x72: { erg: 'OR', args: 0 },
  0x73: { erg: 'XOR', args: 0 },
  0x46: { erg: 'CMP', args: 0, note: 'i32.eq' },
  0x0c: { erg: 'JMP', args: 1, note: 'br' },
  0x0d: { erg: 'JNZ', args: 1, note: 'br_if' },
  0x0b: { erg: 'HALT', args: 0, note: 'end' },
  0x2d: { erg: null, args: 2, note: 'i32.load8_u' },
  0x3a: { erg: null, args: 2, note: 'i32.store8' },
  0x28: { erg: null, args: 2, note: 'i32.load' },
  0x36: { erg: null, args: 2, note: 'i32.store' },
  0x41: { erg: null, args: 1, note: 'i32.const' },
  0x10: { erg: null, args: 1, note: 'call' },
  0x1a: { erg: null, args: 0, note: 'drop' },
}

function parseExports (body) {
  const { v: n, next } = lebRead(body, 0)
  let p = next
  const out = []
  for (let i = 0; i < n; i++) {
    const { v: nlen, next: p2 } = lebRead(body, p)
    p = p2
    const name = new TextDecoder().decode(body.subarray(p, p + nlen))
    p += nlen
    const kind = body[p++]
    const { v: idx, next: p3 } = lebRead(body, p)
    p = p3
    if (kind === 0) out.push({ name, funcIdx: idx })
  }
  return out
}

function disasmFunc (body, label) {
  const lines = [`; === ${label} ===`]
  let p = 0
  const { v: nloc, next } = lebRead(body, p)
  p = next
  for (let i = 0; i < nloc; i++) {
    const { v: cnt, next: p2 } = lebRead(body, p)
    p = p2
    const { next: p3 } = lebRead(body, p)
    p = p3
    void cnt
  }
  const { v: nb, next: p4 } = lebRead(body, p)
  p = p4
  void nb
  const start = p
  while (p < body.length) {
    const op = body[p++]
    const m = MAP[op]
    if (!m) {
      lines.push(`; wasm 0x${op.toString(16)} @${p - 1 - start}`)
      continue
    }
    const args = []
    for (let k = 0; k < m.args; k++) {
      const { v, next: np } = lebRead(body, p)
      p = np
      args.push(v)
    }
    if (m.erg === 'LOAD' || m.erg === 'STORE') {
      lines.push(`${m.erg} ${args[0]}`)
    } else if (m.erg === 'JMP' || m.erg === 'JNZ') {
      lines.push(`; ${m.note || m.erg} rel ${args[0]}`)
      lines.push(m.erg === 'JMP' ? `JMP ${args[0]}` : `JNZ ${args[0]}`)
    } else if (m.erg) {
      lines.push(m.erg)
    } else {
      lines.push(`; ${m.note || 'wasm'} ${args.join(' ')}`)
    }
    if (m.erg === 'HALT') break
  }
  lines.push('HALT')
  return lines.join('\n') + '\n'
}

/** @param {Uint8Array|Buffer} wasmBuf @param {{ fonte?: string }} opts */
export function wasmParaErg (wasmBuf, opts = {}) {
  return wasmParaErgCompleto(wasmBuf, opts)
}

/** Embute assembly no módulo wasm (secção custom, ex. bash.erg). */
export function embuteErgNoWasm (wasmBuf, ergText, secName = 'node.erg') {
  const nome = new TextEncoder().encode(secName)
  const texto = new TextEncoder().encode(ergText)
  const corpo = new Uint8Array(lebWrite(nome.length).length + nome.length + texto.length)
  let o = 0
  const nl = lebWrite(nome.length)
  corpo.set(nl, o); o += nl.length
  corpo.set(nome, o); o += nome.length
  corpo.set(texto, o)
  const sec = new Uint8Array(1 + lebWrite(corpo.length).length + corpo.length)
  let q = 0
  sec[q++] = 0
  const sl = lebWrite(corpo.length)
  sec.set(sl, q); q += sl.length
  sec.set(corpo, q)
  const base = wasmBuf instanceof Uint8Array ? wasmBuf : new Uint8Array(wasmBuf)
  const out = new Uint8Array(base.length + sec.length)
  out.set(base, 0)
  out.set(sec, base.length)
  return out
}

export function extraiErgDoWasm (wasmBuf, secName = 'node.erg') {
  const buf = wasmBuf instanceof Uint8Array ? wasmBuf : new Uint8Array(wasmBuf)
  const alvo = new TextEncoder().encode(secName)
  for (const s of parseWasm(buf)) {
    if (s.id !== 0) continue
    let p = 0
    const { v: nlen, next } = lebRead(s.body, p)
    p = next
    const nm = s.body.subarray(p, p + nlen)
    p += nlen
    if (nm.length === alvo.length && nm.every((b, i) => b === alvo[i])) {
      return new TextDecoder().decode(s.body.subarray(p))
    }
  }
  return null
}
