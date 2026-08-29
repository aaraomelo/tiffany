/* asm_wasm.mjs — ponte ASM ↔ WASM: a ISA ERG-64 nas duas réguas.
 *
 *   texto .erg  ──monta──►  fita  ──sobe──►  isa.wasm + secção custom erg.fita
 *   texto .erg  ◄─desmonta─  fita  ◄─desce──  (extrai erg.fita do módulo)
 *
 * A máquina é isa.wasm (MOVE); a fita é o programa montado — a mesma que o erg corre.
 *
 *   node tools/asm_wasm.mjs sobe programa.erg -o programa.wasm
 *   node tools/asm_wasm.mjs desce programa.wasm -o programa.erg
 */
import fs from 'fs'
import path from 'path'
import os from 'os'
import { execFileSync } from 'child_process'
import { fileURLToPath } from 'url'
import { monta, desmonta } from '../lib/erg_monta.mjs'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const RAIZ = path.join(__dirname, '..')
const ISA_WASM = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'isa.wasm')
const TRADUZ = path.join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')
const FONTE_ISA = path.join(RAIZ, 'tools', 'isa.c')
export const SEC_FITA = 'erg.fita'

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
  throw new Error('LEB128 inválido')
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
  if (buf.length < 8 || buf[0] !== 0 || buf[1] !== 0x61 || buf[2] !== 0x73 || buf[3] !== 0x6d) {
    throw new Error('não é wasm')
  }
  const sections = []
  let p = 8
  while (p < buf.length) {
    const id = buf[p++]
    const { v: size, next } = lebRead(buf, p)
    p = next
    const body = buf.subarray(p, p + size)
    p += size
    sections.push({ id, size, body })
  }
  return { header: buf.subarray(0, 8), sections }
}

/** @param {Uint8Array} wasm @param {string} nome @param {Uint8Array} payload */
export function appendCustom (wasm, nome, payload) {
  const mod = parseWasm(wasm)
  const nomeBytes = new TextEncoder().encode(nome)
  const corpo = new Uint8Array(lebWrite(nomeBytes.length).length + nomeBytes.length + payload.length)
  let o = 0
  const nl = lebWrite(nomeBytes.length)
  corpo.set(nl, o); o += nl.length
  corpo.set(nomeBytes, o); o += nomeBytes.length
  corpo.set(payload, o)
  const sec = new Uint8Array(1 + lebWrite(corpo.length).length + corpo.length)
  let q = 0
  sec[q++] = 0
  const sl = lebWrite(corpo.length)
  sec.set(sl, q); q += sl.length
  sec.set(corpo, q)
  const out = new Uint8Array(wasm.length + sec.length)
  out.set(wasm, 0)
  out.set(sec, wasm.length)
  return out
}

/** @param {Uint8Array} wasm @param {string} nome */
export function extractCustom (wasm, nome) {
  const mod = parseWasm(wasm)
  const alvo = new TextEncoder().encode(nome)
  for (const s of mod.sections) {
    if (s.id !== 0) continue
    let p = 0
    const { v: nlen, next } = lebRead(s.body, p)
    p = next
    const nm = s.body.subarray(p, p + nlen)
    p += nlen
    if (nm.length === alvo.length && nm.every((b, i) => b === alvo[i])) {
      return s.body.subarray(p)
    }
  }
  return null
}

export function garanteIsaWasm () {
  if (fs.existsSync(ISA_WASM)) return ISA_WASM
  fs.mkdirSync(path.dirname(ISA_WASM), { recursive: true })
  if (!fs.existsSync(TRADUZ)) {
    try {
      execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', TRADUZ])
    } catch (_) {
      throw new Error('isa.wasm em falta e traduz não disponível')
    }
  }
  execFileSync(TRADUZ, [FONTE_ISA, '-o', ISA_WASM])
  return ISA_WASM
}

/** @param {string} textoErg */
export function sobeErg (textoErg) {
  const fita = monta(textoErg)
  const base = fs.readFileSync(garanteIsaWasm())
  return appendCustom(base, SEC_FITA, fita)
}

/** @param {Uint8Array|Buffer} wasm */
export function desceWasm (wasm) {
  const fita = extractCustom(wasm, SEC_FITA)
  if (!fita) throw new Error(`secção custom '${SEC_FITA}' em falta`)
  return desmonta(fita)
}

/** @param {Uint8Array|Buffer} wasm */
export function fitaDeWasm (wasm) {
  const fita = extractCustom(wasm, SEC_FITA)
  if (!fita) throw new Error(`secção custom '${SEC_FITA}' em falta`)
  return fita
}

function uso () {
  console.error('uso: asm_wasm.mjs sobe <fonte.erg> -o <saida.wasm>')
  console.error('      asm_wasm.mjs desce <entrada.wasm> -o <saida.erg>')
  process.exit(2)
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const cmd = process.argv[2]
  let saida = null
  let entrada = null
  for (let i = 3; i < process.argv.length; i++) {
    if (process.argv[i] === '-o' && process.argv[i + 1]) saida = process.argv[++i]
    else if (!entrada) entrada = process.argv[i]
  }
  if (!cmd || !entrada || !saida) uso()
  if (cmd === 'sobe') {
    const txt = fs.readFileSync(entrada, 'utf8')
    fs.writeFileSync(saida, sobeErg(txt))
    console.log(`asm_wasm: ${entrada} -> ${saida}  (${monta(txt).length}B fita)`)
  } else if (cmd === 'desce') {
    const wasm = fs.readFileSync(entrada)
    fs.writeFileSync(saida, desceWasm(wasm), 'utf8')
    console.log(`asm_wasm: ${entrada} -> ${saida}`)
  } else uso()
}
