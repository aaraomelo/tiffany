// banco_move.js — MOVE(±1) na arena wasm DISCO

export const NULO = 8

export function backendsDom (man) {
  return man.linguagens
    .filter((l) => ['html', 'css', 'js', 'sql'].includes(l.nome))
    .map((l) => ({
      nome: l.nome,
      wasm: l.wasm,
      move: l.exports?.find((e) => e.endsWith('_move')) || `${l.nome}_move`,
      faz: l.faz,
    }))
}

export function u8 (ex) {
  return new Uint8Array(ex.DISCO.buffer)
}

export function dec (ex, off, len) {
  return new TextDecoder().decode(u8(ex).slice(NULO + off, NULO + off + len))
}

export function enc (ex, off, str) {
  const b = new TextEncoder().encode(str)
  u8(ex).set(b, NULO + off)
  return b.length
}

export function moveWasm (ex, fnName, text, sentido = -1, inOff = 1024, outOff = 4096) {
  const n = enc(ex, inOff, text)
  const fn = ex[fnName]
  if (typeof fn !== 'function') throw new Error('export ' + fnName + ' em falta — recompile wasm')
  const outLen = fn(inOff, n, outOff, sentido)
  return dec(ex, outOff, outLen)
}

export async function loadWasm (base, nome) {
  const r = await fetch(base + nome)
  if (!r.ok) throw new Error('wasm ' + nome + ': ' + r.status)
  const { instance } = await WebAssembly.instantiate(await r.arrayBuffer())
  return instance.exports
}
