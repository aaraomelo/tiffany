/* arena_disco.mjs — arena DISCO ↔ mem.dat (fisica.tex: realização no suporte).
 * Cada offset da arena é slot ISA; mem.dat guarda Word_8 em .total (Lei 7 / slot_mem.h). */
import { readFileSync, writeFileSync } from 'node:fs'

export const ARENA_SIZE = 65536
/** Base do DISCO no wasm (traduz NULO=8): arena[i] ↔ slot ISA i+NULO.
 * fis:thm:zeroinf — andar onde 0 (Word nulo) e ∞ (rodata remapeada) são o mesmo bloco. */
export const NULO_DISCO = 8
/** Cobre arena + rodata remapeada (wasm ≥65536 → 65400+). */
export const MEM_SLOTS = 65500
export const RODATA_RELOC_BASE = 65400
export const RODATA_TAG = RODATA_RELOC_BASE + 8

const RODATA_PREFIX = {
  node: 'console.log(',
  bash: 'echo ',
  powershell: 'Write-Output ',
}

export function relocAddr (k) {
  return k >= 65536 ? RODATA_RELOC_BASE + (k - 65536) : k
}
export const OFF_NIN = 24576
export const OFF_NOUT = 24578
export const OFF_SEQ = 24580
export const OFF_GSUM = 24582
export const OFF_G = 24608
export const G_CELLS = 256

/** Células G_visit do pipe (host): um backend = um ramo experimental. */
export const G_CELL = { node: 1, bash: 2, powershell: 3 }
export const OFF_IN = 256
export const OFF_OUT = 16384
export const CAP = 8192

function lebRead (buf, pos) {
  let v = 0n
  let s = 0n
  let p = pos
  while (p < buf.length) {
    const c = BigInt(buf[p++])
    v |= (c & 0x7fn) << s
    if ((c & 0x80n) === 0n) return { v: Number(v), next: p }
    s += 7n
  }
  return { v: 0, next: p }
}

/** Segmentos data do wasm → mesmos slots que a figura lê (interface estrela). */
export function semeiaWasmData (mem, wasmBuf) {
  if (!wasmBuf || wasmBuf.length < 8) return
  const buf = Buffer.isBuffer(wasmBuf) ? wasmBuf : Buffer.from(wasmBuf)
  let p = 8
  while (p < buf.length) {
    const id = buf[p++]
    const { v: size, next } = lebRead(buf, p)
    p = next
    const body = buf.subarray(p, p + size)
    if (id === 11) {
      let q = 0
      const { v: n, next: q1 } = lebRead(body, 0)
      q = q1
      for (let i = 0; i < n; i++) {
        q++ // memory index
        if (body[q] === 0x41) {
          const { v: off, next: q2 } = lebRead(body, q + 1)
          q = q2
          const { v: dlen, next: q3 } = lebRead(body, q)
          q = q3
          const data = body.subarray(q, q + dlen)
          for (let j = 0; j < data.length; j++) {
            const slot = relocAddr(off + j)
            if (slot * 2 < mem.length) mem[slot * 2] = data[j]
          }
          q += dlen
        }
      }
    }
    p += size
  }
}

/** Arena semântica → mem.dat (slot ISA = offset arena + NULO_DISCO). */
export function arenaParaMem (arena, wasmBuf = null) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  const mem = Buffer.alloc(MEM_SLOTS * 2, 0)
  for (let s = 0; s < ARENA_SIZE; s++) {
    const slot = NULO_DISCO + s
    mem[slot * 2] = s < a.length ? a[s] : 0
  }
  if (wasmBuf) semeiaWasmData(mem, wasmBuf)
  return mem
}

/** mem.dat → arena (lê .total do slot NULO_DISCO+s). */
export function memParaArena (mem) {
  const m = Buffer.isBuffer(mem) ? mem : Buffer.from(mem)
  const arena = Buffer.alloc(ARENA_SIZE, 0)
  for (let s = 0; s < ARENA_SIZE; s++) {
    const slot = NULO_DISCO + s
    arena[s] = m[slot * 2] ?? 0
  }
  return arena
}

/** Tag de prefixo no DISCO (interpretar.c compara OFF_IN+k com RODATA_TAG+k). */
export function seedRodataArena (arena, backend) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  const tag = RODATA_PREFIX[String(backend || '').toLowerCase()]
  if (tag) {
    for (let i = 0; i < tag.length; i++) a[RODATA_TAG + i] = tag.charCodeAt(i)
  }
  return a
}

/** MOVE(−1): script cru em OFF_IN + OFF_NIN (semântica em interpretar.c). */
export function seedScriptArena (script, backend = null) {
  const arena = Buffer.alloc(ARENA_SIZE, 0)
  const body = Buffer.from(String(script ?? ''), 'utf8')
  const n = Math.min(body.length, CAP)
  body.copy(arena, OFF_IN, 0, n)
  arena[OFF_NIN] = n & 255
  arena[OFF_NIN + 1] = (n >> 8) & 255
  if (backend) seedRodataArena(arena, backend)
  return arena
}

export function readStdout (arena) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  const n = a[OFF_NOUT] + a[OFF_NOUT + 1] * 256
  return a.toString('utf8', OFF_OUT, OFF_OUT + Math.min(n, CAP))
}

/** G_visit(x) na arena — contador host de visitas por célula byte.
 * NÃO confundir com G_real(x)=|π^{-1}(x)| (fis:def:objeto); ver gRealFromJournal. */
export function getG (arena, cell) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  const c = Number(cell) & 255
  if (OFF_G + c >= a.length) return 0
  return a[OFF_G + c]
}

/** alias explícito para G_visit */
export function getGVisit (arena, cell) {
  return getG(arena, cell)
}

export function incG (arena, cell) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  const c = Number(cell) & 255
  const i = OFF_G + c
  if (i < a.length && a[i] < 255) a[i] = a[i] + 1
  let sum = a[OFF_GSUM] + a[OFF_GSUM + 1] * 256
  if (sum < 65535) sum = sum + 1
  a[OFF_GSUM] = sum & 255
  a[OFF_GSUM + 1] = (sum >> 8) & 255
  return a
}

export function sumG (arena) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  let s = 0
  for (let c = 0; c < G_CELLS && OFF_G + c < a.length; c++) s += a[OFF_G + c]
  return s
}

export function gsumTotal (arena) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  return a[OFF_GSUM] + a[OFF_GSUM + 1] * 256
}

/** G_real(x)=|π^{-1}(x)| a partir de diário π: lista ordenada π(0),π(1),… */
export function gRealFromJournal (journal, cell) {
  const x = Number(cell) & 255
  let n = 0
  for (let i = 0; i < journal.length; i++) {
    if ((journal[i] & 255) === x) n++
  }
  return n
}

/** Verifica G_visit(x)=G_real(x) para todo x activo (protocolo de realização).
 *  §RG2: true sob protocolo; §RG3: false em violações controladas. */
export function verifyGVisitEqualsReal (arena, journal) {
  const seen = new Set()
  for (let i = 0; i < journal.length; i++) seen.add(journal[i] & 255)
  for (const c of seen) {
    if (getG(arena, c) !== gRealFromJournal(journal, c)) return false
  }
  for (let c = 0; c < G_CELLS; c++) {
    const v = getG(arena, c)
    if (v > 0 && !seen.has(c)) return false
    if (v === 0 && seen.has(c)) return false
  }
  return journal.length === gsumTotal(arena)
}

/** Incrementa G_visit; se journal[] for passado, regista π(i)=cell (uma entrada por visita). */
export function recordCorreVisit (arena, backend, journal = null) {
  const cell = G_CELL[String(backend || '').toLowerCase()]
  if (!cell) return arena
  incG(arena, cell)
  if (journal != null && Array.isArray(journal)) journal.push(cell)
  return arena
}
