/* wasm_erg.mjs — wasm (pilha) → ERG-64 (registadores), chessb.c §C4 / fisica.tex duomorf.
 * Sem runtime: traduz a figura; erg corre no metal. */

function lebRead (buf, pos) {
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

function parseWasmSections (buf) {
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

const LOCAL_BASE = 600
const TEMP_BASE = 900
const CONST_BASE = 1200
const ZERO = 0
const RODATA_RELOC_BASE = 65400

function relocAddr (k) {
  return k >= 65536 ? RODATA_RELOC_BASE + (k - 65536) : k
}

function parseModule (buf) {
  const b = buf instanceof Uint8Array ? buf : new Uint8Array(buf)
  const secs = parseWasmSections(b)
  let importFuncCount = 0
  let typeSec = null
  let funcTypes = []
  let codes = []
  const exports = []

  for (const s of secs) {
    if (s.id === 2) {
      let p = 0
      const { v: n, next } = lebRead(s.body, p)
      p = next
      for (let i = 0; i < n; i++) {
        const { v: modLen, next: p2 } = lebRead(s.body, p)
        p = p2 + modLen
        const { v: nameLen, next: p3 } = lebRead(s.body, p)
        p = p3 + nameLen
        const kind = s.body[p++]
        if (kind === 0) importFuncCount++
        else p += 2
      }
    }
    if (s.id === 1) typeSec = s.body
    if (s.id === 3 && typeSec) {
      let p = 0
      const { v: n, next } = lebRead(s.body, p)
      p = next
      for (let i = 0; i < n; i++) {
        const { v: ti, next: p2 } = lebRead(s.body, p)
        p = p2
        funcTypes.push(ti)
      }
    }
    if (s.id === 10) {
      let p = 0
      const { v: n, next } = lebRead(s.body, p)
      p = next
      for (let i = 0; i < n; i++) {
        const { v: fsize, next: p2 } = lebRead(s.body, p)
        codes.push(s.body.subarray(p2, p2 + fsize))
        p = p2 + fsize
      }
    }
    if (s.id === 7) {
      let p = 0
      const { v: n, next } = lebRead(s.body, p)
      p = next
      for (let i = 0; i < n; i++) {
        const { v: nlen, next: p2 } = lebRead(s.body, p)
        p = p2
        const name = new TextDecoder().decode(s.body.subarray(p, p + nlen))
        p += nlen
        const kind = s.body[p++]
        const { v: idx, next: p3 } = lebRead(s.body, p)
        p = p3
        exports.push({ name, kind, idx })
      }
    }
  }
  return { importFuncCount, funcTypes, codes, exports, buf: b }
}

function skipLocals (body, p0) {
  let p = p0
  const { v: ng, next } = lebRead(body, p)
  p = next
  for (let g = 0; g < ng; g++) {
    const { v: cnt, next: p2 } = lebRead(body, p)
    p = p2 + 1
    void cnt
  }
  return p
}

function detectCallTarget (body) {
  const p = skipLocals(body, 0)
  if (body[p] === 0x10) {
    const { v: idx, next } = lebRead(body, p + 1)
    if (body[next] === 0x0f || body[next] === 0x0b) return idx
  }
  return null
}

class Ctx {
  constructor (body, mod, inlineFns) {
    this.body = body
    this.mod = mod
    this.inlineFns = inlineFns
    this.p = 0
    this.lines = []
    this.stack = []
    this.consts = new Map()
    this.constN = 0
    this.tempN = 0
    this.labelN = 0
    this.control = []
    this.depth = 0
  }

  slotLocal (i) { return LOCAL_BASE + i }
  freshTemp () { return TEMP_BASE + this.tempN++ }
  newLabel (pfx = 'L') { return `${pfx}${this.labelN++}` }

  slotConst (k) {
    k = relocAddr(k)
    if (!this.consts.has(k)) {
      const slot = CONST_BASE + this.constN++
      this.consts.set(k, slot)
      if (!this.constBySlot) this.constBySlot = new Map()
      this.constBySlot.set(slot, k)
    }
    return this.consts.get(k)
  }

  constVal (slot) {
    return this.constBySlot?.get(slot) ?? -1
  }

  emitLoad8At (addrSlot) {
    const addr = this.constBySlot?.get(addrSlot)
    const t = this.freshTemp()
    if (addr !== undefined && addrSlot >= CONST_BASE) {
      this.emitRFrom(this.slotConst(addr))
      this.emitStoreR(t)
      this.push(t)
      return
    }
    this.emit(`LOADS ${addrSlot}`, 'LOAD 0', 'ADD', `STORE ${t}`)
    this.push(t)
  }

  emitStore8At (valSlot, addrSlot) {
    const addr = this.constBySlot?.get(addrSlot)
    this.emitRFrom(valSlot)
    if (addr !== undefined && addrSlot >= CONST_BASE) {
      this.emitStoreR(this.slotConst(addr))
    } else {
      this.emit(`STORE_IND ${addrSlot}`)
    }
  }

  emitDiv256 (a) {
    const t = this.freshTemp()
    this.emitRFrom(a)
    this.emit('TROCA', `STORE ${t}`)
    this.push(t)
  }

  emitMod256 (a) {
    const t = this.freshTemp()
    this.emitRFrom(a)
    this.emitStoreR(t)
    this.push(t)
  }

  push (slot) { this.stack.push(slot) }
  pop () {
    if (!this.stack.length) throw new Error('pilha wasm vazia')
    return this.stack.pop()
  }

  emit (...xs) { for (const x of xs) this.lines.push(x) }

  /** R ← valor do slot (STORE pattern: LOAD k; LOAD 0; ADD). */
  emitRFrom (slot) {
    this.emit(`LOAD ${slot}`, `LOAD ${ZERO}`, 'ADD')
  }

  emitStoreR (slot) {
    this.emit(`STORE ${slot}`)
  }

  /** Copia slot → temp via R. */
  emitCopy (from, to) {
    this.emitRFrom(from)
    this.emitStoreR(to)
  }

  emitBinop (op) {
    const b = this.pop()
    const a = this.pop()
    const t = this.freshTemp()
    this.emit(`LOAD ${a}`, `LOAD ${b}`, op, `STORE ${t}`)
    this.push(t)
  }

  emitI32Eq (a, b) {
    const diff = this.freshTemp()
    const res = this.freshTemp()
    const lEq = this.newLabel('EQ')
    const lEnd = this.newLabel('ED')
    this.emit(`LOAD ${a}`, `LOAD ${b}`, 'SUB', `STORE ${diff}`)
    this.emitCmpZero(diff, lEq)
    this.emitRFrom(this.slotConst(0))
    this.emitStoreR(res)
    this.emit(`JMP ${lEnd}`, `:${lEq}`)
    this.emitRFrom(this.slotConst(1))
    this.emitStoreR(res)
    this.emit(`:${lEnd}`)
    this.push(res)
  }

  emitI32Gt (a, b) {
    const diff = this.freshTemp()
    const td = this.freshTemp()
    const ts = this.freshTemp()
    const res = this.freshTemp()
    const lEq = this.newLabel('EQ')
    const lNotGt = this.newLabel('NG')
    const lEnd = this.newLabel('ED')
    const c255 = this.slotConst(255)
    this.emit(`LOAD ${a}`, `LOAD ${b}`, 'SUB16', `STORE ${diff}`)
    this.emitCmpZero(diff, lEq)
    this.emit(`LOAD ${diff}`, 'TROCA', `STORE ${td}`)
    this.emit(`LOAD ${td}`, `LOAD ${c255}`, 'SUB', `STORE ${ts}`)
    this.emitCmpZero(ts, lNotGt)
    this.emitRFrom(this.slotConst(1))
    this.emitStoreR(res)
    this.emit(`JMP ${lEnd}`, `:${lNotGt}`)
    this.emitRFrom(this.slotConst(0))
    this.emitStoreR(res)
    this.emit(`JMP ${lEnd}`, `:${lEq}`)
    this.emitRFrom(this.slotConst(0))
    this.emitStoreR(res)
    this.emit(`:${lEnd}`)
    this.push(res)
  }

  emitI32Not (v) {
    const t = this.freshTemp()
    const lZ = this.newLabel('NZ')
    const lE = this.newLabel('NE')
    this.emitCmpZero(v, lZ)
    this.emitRFrom(this.slotConst(0))
    this.emitStoreR(t)
    this.emit(`JMP ${lE}`, `:${lZ}`)
    this.emitRFrom(this.slotConst(1))
    this.emitStoreR(t)
    this.emit(`:${lE}`)
    this.push(t)
  }

  emitCmpZero (slot, jumpLabel) {
    this.emit(`LOAD ${slot}`, `LOAD ${ZERO}`, 'CMP', `JZ ${jumpLabel}`)
  }

  emitBrIf (depth) {
    const cond = this.pop()
    const ctrl = this.control[this.control.length - 1 - depth]
    if (!ctrl) throw new Error(`br depth ${depth} inválido`)
    const target = ctrl.kind === 'loop' ? ctrl.start : ctrl.end
    const skip = this.newLabel('NB')
    this.emitCmpZero(cond, skip)
    this.emit(`JMP ${target}`)
    this.emit(`:${skip}`)
  }

  emitBr (depth) {
    const ctrl = this.control[this.control.length - 1 - depth]
    if (!ctrl) throw new Error(`br depth ${depth} inválido`)
    const target = ctrl.kind === 'loop' ? ctrl.start : ctrl.end
    this.emit(`JMP ${target}`)
  }

  readBlockType () {
    const t = this.body[this.p++]
    if (t === 0x40) return
    if (t === 0x7f || t === 0x7e) return
    throw new Error(`blocktype 0x${t.toString(16)} não suportado`)
  }

  readInstrs () {
    while (this.p < this.body.length) {
      const op = this.body[this.p++]
      switch (op) {
        case 0x00: break
        case 0x01: break
        case 0x02: {
          this.readBlockType()
          const end = this.newLabel('BE')
          this.control.push({ kind: 'block', end })
          this.depth++
          this.readInstrs()
          this.depth--
          this.emit(`:${end}`)
          this.control.pop()
          break
        }
        case 0x03: {
          this.readBlockType()
          const start = this.newLabel('LS')
          const end = this.newLabel('LE')
          this.emit(`:${start}`)
          this.control.push({ kind: 'loop', start, end })
          this.depth++
          this.readInstrs()
          this.depth--
          this.emit(`JMP ${start}`, `:${end}`)
          this.control.pop()
          break
        }
        case 0x04: {
          this.readBlockType()
          const cond = this.pop()
          const elseLbl = this.newLabel('EL')
          const endLbl = this.newLabel('EI')
          this.control.push({ kind: 'block', end: endLbl })
          this.emitCmpZero(cond, elseLbl)
          this.depth++
          this.readInstrs()
          this.depth--
          if (this.p < this.body.length && this.body[this.p] === 0x05) {
            this.p++
            this.emit(`JMP ${endLbl}`, `:${elseLbl}`)
            this.depth++
            this.readInstrs()
            this.depth--
          } else {
            this.emit(`:${elseLbl}`)
          }
          if (this.p < this.body.length && this.body[this.p] === 0x0b) this.p++
          this.emit(`:${endLbl}`)
          this.control.pop()
          break
        }
        case 0x05:
          if (this.depth > 0) {
            this.p--
            return
          }
          throw new Error(`else inesperado @${this.p - 1}`)
        case 0x0b:
          if (this.depth > 0) {
            this.p--
            return
          }
          break
        case 0x0c: {
          const { v: d, next } = lebRead(this.body, this.p)
          this.p = next
          this.emitBr(d)
          break
        }
        case 0x0d: {
          const { v: d, next } = lebRead(this.body, this.p)
          this.p = next
          const cond = this.pop()
          const ctrl = this.control[this.control.length - 1 - d]
          if (!ctrl) throw new Error(`br_if depth ${d}`)
          const target = ctrl.kind === 'loop' ? ctrl.start : ctrl.end
          const skip = this.newLabel('BI')
          this.emitCmpZero(cond, skip)
          this.emit(`JMP ${target}`)
          this.emit(`:${skip}`)
          void d
          break
        }
        case 0x0f:
          if (this.stack.length) {
            const r = this.pop()
            this.emitRFrom(r)
          }
          return
        case 0x10: {
          const { v: fi, next } = lebRead(this.body, this.p)
          this.p = next
          const codeIdx = fi - this.mod.importFuncCount
          if (codeIdx < 0 || codeIdx >= this.mod.codes.length) {
            throw new Error(`call função importada ${fi}`)
          }
          if (this.inlineFns.has(codeIdx)) {
            const sub = new Ctx(this.mod.codes[codeIdx], this.mod, this.inlineFns)
            sub.p = skipLocals(sub.body, 0)
            sub.readInstrs()
            this.lines.push(...sub.lines)
            for (const [k, v] of sub.consts) this.consts.set(k, v)
            this.constN = Math.max(this.constN, sub.constN)
            this.tempN = Math.max(this.tempN, sub.tempN)
          } else {
            throw new Error(`call ${fi} — inline apenas para núcleo interno`)
          }
          break
        }
        case 0x1a:
          this.pop()
          break
        case 0x20: {
          const { v: i, next } = lebRead(this.body, this.p)
          this.p = next
          this.push(this.slotLocal(i))
          break
        }
        case 0x21: {
          const { v: i, next } = lebRead(this.body, this.p)
          this.p = next
          const v = this.pop()
          this.emitRFrom(v)
          this.emitStoreR(this.slotLocal(i))
          break
        }
        case 0x22: {
          const { v: i, next } = lebRead(this.body, this.p)
          this.p = next
          const v = this.pop()
          this.emitRFrom(v)
          this.emitStoreR(this.slotLocal(i))
          this.push(this.slotLocal(i))
          break
        }
        case 0x41: {
          const { v: k, next } = lebRead(this.body, this.p)
          this.p = next
          this.push(this.slotConst(k))
          break
        }
        case 0x28: {
          this.p++
          const { v: off, next } = lebRead(this.body, this.p)
          this.p = next
          let addr = this.pop()
          if (off !== 0) {
            const a = this.freshTemp()
            const o = this.slotConst(off)
            this.emit(`LOAD ${addr}`, `LOAD ${o}`, 'ADD16', `STORE ${a}`)
            addr = a
          }
          this.emitLoad8At(addr)
          break
        }
        case 0x2c: {
          this.p++
          const { v: off, next } = lebRead(this.body, this.p)
          this.p = next
          let addr = this.pop()
          if (off !== 0) {
            const a = this.freshTemp()
            const o = this.slotConst(off)
            this.emit(`LOAD ${addr}`, `LOAD ${o}`, 'ADD16', `STORE ${a}`)
            addr = a
          }
          this.emitLoad8At(addr)
          break
        }
        case 0x2d: {
          this.p++
          const { v: off, next } = lebRead(this.body, this.p)
          this.p = next
          let addr = this.pop()
          if (off !== 0) {
            const a = this.freshTemp()
            const o = this.slotConst(off)
            this.emit(`LOAD ${addr}`, `LOAD ${o}`, 'ADD16', `STORE ${a}`)
            addr = a
          }
          this.emitLoad8At(addr)
          break
        }
        case 0x36: {
          this.p++
          const { v: off, next } = lebRead(this.body, this.p)
          this.p = next
          const val = this.pop()
          let addr = this.pop()
          if (off !== 0) {
            const a = this.freshTemp()
            const o = this.slotConst(off)
            this.emit(`LOAD ${addr}`, `LOAD ${o}`, 'ADD16', `STORE ${a}`)
            addr = a
          }
          this.emitRFrom(val)
          const imm = this.constBySlot?.get(addr)
          if (imm !== undefined && addr >= CONST_BASE) {
            this.emitStoreR(this.slotConst(imm))
          } else {
            this.emit(`STORE_IND ${addr}`)
          }
          break
        }
        case 0x3a: {
          this.p++
          const { v: off, next } = lebRead(this.body, this.p)
          this.p = next
          const val = this.pop()
          let addr = this.pop()
          if (off !== 0) {
            const a = this.freshTemp()
            const o = this.slotConst(off)
            this.emit(`LOAD ${addr}`, `LOAD ${o}`, 'ADD16', `STORE ${a}`)
            addr = a
          }
          this.emitStore8At(val, addr)
          break
        }
        case 0x46: {
          const b = this.pop()
          const a = this.pop()
          this.emitI32Eq(a, b)
          break
        }
        case 0x47: {
          const b = this.pop()
          const a = this.pop()
          this.emitI32Eq(a, b)
          this.emitI32Not(this.pop())
          break
        }
        case 0x48: case 0x49: {
          const b = this.pop()
          const a = this.pop()
          this.emitI32Gt(b, a)
          break
        }
        case 0x4a: case 0x4b: {
          const b = this.pop()
          const a = this.pop()
          this.emitI32Gt(a, b)
          break
        }
        case 0x4c: case 0x4d: {
          const b = this.pop()
          const a = this.pop()
          this.emitI32Gt(a, b)
          this.emitI32Not(this.pop())
          break
        }
        case 0x4e: case 0x4f: {
          const b = this.pop()
          const a = this.pop()
          this.emitI32Gt(b, a)
          this.emitI32Not(this.pop())
          break
        }
        case 0x6a: this.emitBinop('ADD16'); break
        case 0x6b: this.emitBinop('SUB16'); break
        case 0x6c: this.emitBinop('MUL16'); break
        case 0x6d: {
          const b = this.pop()
          const a = this.pop()
          if (this.constVal(b) === 256) this.emitDiv256(a)
          else throw new Error(`div só por 256 @${this.p - 1}`)
          break
        }
        case 0x6e: case 0x6f: {
          const b = this.pop()
          const a = this.pop()
          if (this.constVal(b) === 256) this.emitMod256(a)
          else throw new Error(`rem só por 256 @${this.p - 1}`)
          break
        }
        case 0x71: this.emitBinop('AND'); break
        case 0x72: this.emitBinop('OR'); break
        case 0x73: this.emitBinop('XOR'); break
        default:
          throw new Error(`opcode wasm 0x${op.toString(16)} @${this.p - 1} não traduzido`)
      }
    }
  }
}

/** Export wasm → { erg, consts, nome }. */
export function exportWasmParaErg (wasmBuf, exportName, opts = {}) {
  const mod = parseModule(wasmBuf)
  const exp = mod.exports.find((e) => e.name === exportName && e.kind === 0)
  if (!exp) throw new Error(`export «${exportName}» em falta no wasm`)

  let funcIdx = exp.idx
  let codeIdx = funcIdx - mod.importFuncCount
  if (codeIdx < 0 || codeIdx >= mod.codes.length) {
    throw new Error(`export «${exportName}» sem corpo`)
  }

  let body = mod.codes[codeIdx]
  const callee = detectCallTarget(body)
  if (callee !== null) {
    const ci = callee - mod.importFuncCount
    if (ci >= 0 && ci < mod.codes.length) {
      body = mod.codes[ci]
      codeIdx = ci
    }
  }

  const ctx = new Ctx(body, mod, new Set([codeIdx]))
  ctx.p = skipLocals(body, 0)
  ctx.readInstrs()

  if (!ctx.lines.some((l) => l === 'HALT')) ctx.emit('HALT')

  const header = [
    `; ${exportName} — wasm→ERG (chessb §C4, fisica.tex §fis:def:duomorf)`,
    opts.fonte ? `; C: ${opts.fonte}` : '',
    '',
  ].filter(Boolean).join('\n')

  return {
    erg: header + ctx.lines.join('\n') + '\n',
    consts: ctx.consts,
    nome: exportName,
  }
}

/** Módulo inteiro: uma secção por export de função. */
export function wasmParaErgCompleto (wasmBuf, opts = {}) {
  const mod = parseModule(wasmBuf)
  const parts = [
    '; wasm → assembly ERG-64 — cadeia física (sem runtime)',
    opts.fonte ? `; C: ${opts.fonte}` : '',
    '',
  ].filter(Boolean)
  for (const e of mod.exports) {
    if (e.kind !== 0) continue
    const { erg } = exportWasmParaErg(wasmBuf, e.name, opts)
    parts.push(erg)
  }
  return parts.join('\n')
}

export { parseModule }
