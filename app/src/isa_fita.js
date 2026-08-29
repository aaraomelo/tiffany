// isa_fita.js — corre fita ERG via isa.wasm (MOVE), browser-safe.
// Mesma porta que tests/isa_dupla.js wasmCorre — sem disco nativo.

export const S_PC = 1027
export const S_N = 1029
export const S_PROG = 1030
export const S_SOMA = 1040
export const S_SUB = 1041
export const S_TROCA = 1048
export const S_CMP = 1049
export const S_PASSO = 1050

/** @param {(d: bigint, s: bigint) => bigint|number} MOVEraw */
export function createIsaRunner (MOVEraw) {
  const MOVE = (d, s) => BigInt(MOVEraw(BigInt(d), BigInt(s)))
  const abs = (d) => MOVE(d, 1)
  const emi = (d) => MOVE(d, -1)

  abs(990); abs(990); abs(S_CMP)
  abs(1028); abs(S_TROCA); abs(S_TROCA); emi(700)
  abs(700); abs(700); abs(S_CMP)
  abs(1028); abs(S_TROCA); abs(S_TROCA); emi(701)
  abs(701); abs(700); abs(S_SUB); emi(702)

  function poeT (slot, v) {
    let n = BigInt.asUintN(64, BigInt(v))
    abs(991); abs(S_TROCA); abs(S_TROCA); emi(703)
    let viu = false
    for (let i = 63n; i >= 0n; i--) {
      const bit = (n >> i) & 1n
      if (!viu && bit === 0n) continue
      viu = true
      abs(703); abs(703); abs(S_SOMA); emi(703)
      if (bit) { abs(702); abs(703); abs(S_SOMA); emi(703) }
    }
    abs(703); emi(slot)
  }

  function poe (slot, t, e) {
    poeT(704, t)
    poeT(705, e)
    abs(705); abs(S_TROCA); emi(705)
    abs(705); abs(704); abs(S_SOMA); emi(slot)
  }

  function gravaFita (fita) {
    const buf = fita instanceof Uint8Array ? fita : new Uint8Array(fita)
    for (let k = 0; k * 16 < buf.length; k++) {
      let lo = 0n; let hi = 0n
      for (let j = 0; j < 8; j++) {
        lo |= BigInt(buf[k * 16 + j] ?? 0) << BigInt(8 * j)
        hi |= BigInt(buf[k * 16 + 8 + j] ?? 0) << BigInt(8 * j)
      }
      poe(S_PROG + k, lo, hi)
    }
    poeT(S_N, buf.length)
    poeT(S_PC, 0)
  }

  let passos = 0
  function corre (max = 2_000_000) {
    passos = 0
    for (let i = 0; i < max; i++) {
      if (MOVE(S_PASSO, 1) === 0n) break
      passos++
    }
    return passos
  }

  function leSlot (slot) {
    const t = abs(slot)
    const e = abs(S_TROCA)
    return [t, e]
  }

  return { poe, poeT, gravaFita, corre, leSlot, get passos () { return passos } }
}

/** Corre fita com sementes [[slot, total, e], …] (semear antes do programa). */
export function correFita (MOVE, fita, seeds = [], maxSteps = 2_000_000) {
  const r = createIsaRunner(MOVE)
  for (const [s, t, e] of seeds) r.poe(s, t, e)
  r.gravaFita(fita)
  r.corre(maxSteps)
  return r
}
