/* erg_monta.mjs — monta/desmonta ERG-64 (mesma tabela que banco/erg.c §E2).
 * A fita é a representação comum entre assembly (.erg) e webassembly (isa.wasm + erg.fita). */

/** @type {{ nome: string, op: number, operando: 0|1|2 }[]} */
export const ISA = [
  { nome: 'HALT', op: 0, operando: 0 },
  { nome: 'LOAD', op: 1, operando: 2 },
  { nome: 'STORE', op: 2, operando: 2 },
  { nome: 'ADD', op: 3, operando: 0 },
  { nome: 'SUB', op: 4, operando: 0 },
  { nome: 'AND', op: 5, operando: 0 },
  { nome: 'OR', op: 6, operando: 0 },
  { nome: 'XOR', op: 7, operando: 0 },
  { nome: 'GOLD', op: 8, operando: 0 },
  { nome: 'CMP', op: 9, operando: 0 },
  { nome: 'JMP', op: 10, operando: 1 },
  { nome: 'JZ', op: 11, operando: 1 },
  { nome: 'JNZ', op: 12, operando: 1 },
  { nome: 'LOADS', op: 14, operando: 2 },
  { nome: 'NEGRO_OURO', op: 15, operando: 0 },
  { nome: 'ESQUILO', op: 16, operando: 0 },
  { nome: 'TROCA', op: 17, operando: 0 },
  { nome: 'ADD16', op: 19, operando: 0 },
  { nome: 'SUB16', op: 20, operando: 0 },
  { nome: 'CMP16', op: 21, operando: 0 },
  { nome: 'MUL16', op: 22, operando: 0 },
  { nome: 'ESPALHA', op: 23, operando: 0 },
  { nome: 'STORE_IND', op: 24, operando: 2 },
  { nome: 'VINCO', op: 25, operando: 0 },
  { nome: 'INC', op: 26, operando: 2 },
]

const POR_NOME = Object.fromEntries(ISA.map((i) => [i.nome, i]))
const POR_OP = Object.fromEntries(ISA.map((i) => [i.op, i]))

/** @param {string} texto */
export function monta (texto) {
  /** @type {{ nome: string, end: number }[]} */
  const rots = []
  const saida = []

  for (let passagem = 0; passagem < 2; passagem++) {
    let pos = 0
    const linhas = texto.split(/\r?\n/)
    for (let linha = 0; linha < linhas.length; linha++) {
      let s = linhas[linha].replace(/;.*/, '').trim()
      if (!s) continue
      if (s.startsWith(':')) {
        if (passagem === 0) rots.push({ nome: s.slice(1).trim(), end: pos })
        continue
      }
      const sp = s.split(/\s+/)
      const mnem = (sp[0] || '').toUpperCase()
      const arg = sp[1] || ''
      const in_ = POR_NOME[mnem]
      if (!in_) throw new Error(`linha ${linha + 1}: opcode desconhecido '${mnem}'`)
      if (in_.operando && !arg) throw new Error(`linha ${linha + 1}: ${mnem} precisa de operando`)

      const base = pos
      if (passagem === 1) saida[pos] = in_.op
      pos += 1

      if (in_.operando === 2) {
        const slot = Number.parseInt(arg, 0)
        if (!Number.isFinite(slot) || slot < 0 || slot > 65535) {
          throw new Error(`linha ${linha + 1}: slot ${arg} fora de 0..65535`)
        }
        if (passagem === 1) {
          saida[base + 1] = slot & 0xff
          saida[base + 2] = (slot >> 8) & 0xff
        }
        pos += 2
      } else if (in_.operando === 1) {
        let destino
        if (/^-?\d/.test(arg)) destino = Number.parseInt(arg, 0) + base
        else {
          const r = rots.find((x) => x.nome === arg)
          if (!r) throw new Error(`linha ${linha + 1}: rótulo '${arg}' não existe`)
          destino = r.end
        }
        const rel = destino - (base + 2)
        if (rel < -128 || rel > 127) {
          throw new Error(`linha ${linha + 1}: salto de ${rel} bytes não cabe em s8`)
        }
        if (passagem === 1) saida[base + 1] = rel & 0xff
        pos += 1
      }
    }
    if (passagem === 1) return Uint8Array.from(saida)
  }
  return new Uint8Array(0)
}

/** @param {Uint8Array|Buffer} b */
export function desmonta (b) {
  const out = []
  let pos = 0
  while (pos < b.length) {
    const in_ = POR_OP[b[pos]]
    if (!in_) {
      out.push(`; byte ${pos} desconhecido: ${b[pos]}`)
      pos++
      continue
    }
    if (in_.operando === 2 && pos + 2 < b.length) {
      const slot = b[pos + 1] | (b[pos + 2] << 8)
      out.push(`${in_.nome} ${slot}`)
      pos += 3
    } else if (in_.operando === 1 && pos + 1 < b.length) {
      const rel = (b[pos + 1] << 24) >> 24
      out.push(`${in_.nome} ${pos + 2 + rel}`)
      pos += 2
    } else {
      out.push(in_.nome)
      pos += 1
    }
  }
  return out.join('\n') + (out.length ? '\n' : '')
}
