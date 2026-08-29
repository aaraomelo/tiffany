// wasm_sec_browser.js — extrai secções custom do wasm (browser, sem Node runtime).
// Mesmo layout que tools/wasm_sec.c: secção 0 = [namelen][name][payload]

export const SEC_FITA = 'erg.fita'

function lebRead (buf, pos) {
  let v = 0
  let s = 0
  let p = pos
  while (p < buf.length) {
    const c = buf[p++]
    v |= (c & 0x7f) << s
    if ((c & 0x80) === 0) return { v, next: p }
    s += 7
    if (s > 63) break
  }
  throw new Error('LEB128 inválido')
}

/** @param {Uint8Array} buf */
export function parseWasmSections (buf) {
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

/**
 * @param {Uint8Array} wasmBuf
 * @param {string} secName
 * @param {'text'|'bytes'} as
 */
export function extraiSecaoWasm (wasmBuf, secName, as = 'text') {
  const alvo = new TextEncoder().encode(secName)
  for (const s of parseWasmSections(wasmBuf)) {
    if (s.id !== 0) continue
    let p = 0
    const { v: nlen, next } = lebRead(s.body, p)
    p = next
    const nm = s.body.subarray(p, p + nlen)
    p += nlen
    if (nm.length !== alvo.length || !nm.every((b, i) => b === alvo[i])) continue
    const payload = s.body.subarray(p)
    if (as === 'bytes') return payload.slice()
    return new TextDecoder().decode(payload)
  }
  return null
}

/** node.erg | bash.erg | powershell.erg */
export function secErgNome (backend) {
  return backend + '.erg'
}

/** @param {Uint8Array} wasmBuf @param {string} backend */
export function celulaDeWasm (wasmBuf, backend) {
  const sec = secErgNome(backend)
  const erg = extraiSecaoWasm(wasmBuf, sec, 'text')
  const fita = extraiSecaoWasm(wasmBuf, SEC_FITA, 'bytes')
  return {
    secErg: sec,
    erg,
    fita,
    fitaLen: fita ? fita.length : 0,
    temErg: !!erg,
    temFita: !!(fita && fita.length > 0),
  }
}
