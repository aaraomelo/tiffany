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

/** Export canónico da absorção — sem inventar opcode. */
export function moveNome (L) {
  return L?.absorcao?.move || L?.exports?.find((e) => e.endsWith('_move')) || (L?.nome + '_move')
}

/** MOVE na arena segundo absorcao.forma (bytes4/bytes3/slot2/stack1/ints1/id1/unit0). */
export function moveByForma (ex, L, text, sentido = -1, inOff = 1024, outOff = 4096) {
  const fnName = moveNome(L)
  const fn = ex[fnName]
  if (typeof fn !== 'function') throw new Error('export ' + fnName + ' em falta — recompile wasm')
  const forma = L?.absorcao?.forma || (fnName === 'MOVE' ? 'slot2' : (fnName.endsWith('_move') ? 'bytes4' : 'bytes3'))
  if (forma === 'bytes4') return moveWasm(ex, fnName, text, sentido, inOff, outOff)
  if (forma === 'bytes3') {
    const n = enc(ex, inOff, text)
    const outLen = fn(inOff, n, outOff)
    return dec(ex, outOff, outLen)
  }
  if (forma === 'slot2') return String(fn(inOff, sentido))
  if (forma === 'stack1') {
    if (sentido < 0) return String(fn((text || '').length))
    const pop = ex.empilhar_pop
    return String(typeof pop === 'function' ? pop() : 0)
  }
  if (forma === 'ints1' || forma === 'id1') {
    const n = Math.max(0, Math.min(8, (text || '').length))
    return String(fn(n))
  }
  if (forma === 'unit0') return String(fn())
  return moveWasm(ex, fnName, text, sentido, inOff, outOff)
}

export async function loadWasm (base, nome) {
  const r = await fetch(base + nome)
  if (!r.ok) throw new Error('wasm ' + nome + ': ' + r.status)
  const { instance } = await WebAssembly.instantiate(await r.arrayBuffer())
  return instance.exports
}
