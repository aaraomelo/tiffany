/* banco_wasm.mjs — banco wasm na arena DISCO. Sem child_process, sem runtime externo. */
import { mkdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs'
import { join } from 'node:path'
import { fileURLToPath } from 'node:url'
import { parseMove, SLOTS, S_CANAL_BASE } from './banco_shell_parse.mjs'

const __dir = fileURLToPath(new URL('.', import.meta.url))
const RAIZ = join(__dir, '..')
const WASM_DIR = join(RAIZ, 'assets', 'figuras', 'wasm')
const NULO = 8

const BACKENDS = {
  node: { wasm: 'node.wasm', move: 'node_move', corre: 'node_corre' },
  bash: { wasm: 'interpretar.wasm', move: 'bash_move', corre: 'bash_corre' },
  powershell: { wasm: 'powershell.wasm', move: 'powershell_move', corre: 'powershell_corre' },
}

const OFF_NOUT = 24578
const OFF_BUF_OUT = 16384

const bancos = new Map()

function discoPath (base, nome) {
  const dir = `${base}_${nome}`
  mkdirSync(dir, { recursive: true })
  return join(dir, 'disco.bin')
}

function carregaWasm (nome) {
  const cfg = BACKENDS[nome]
  if (!cfg) throw new Error('backend «' + nome + '»')
  const buf = readFileSync(join(WASM_DIR, cfg.wasm))
  const { instance } = new WebAssembly.Instance(new WebAssembly.Module(buf))
  const ex = instance.exports
  if (typeof ex[cfg.move] !== 'function') throw new Error(cfg.move + ' em falta em ' + cfg.wasm)
  if (typeof ex[cfg.corre] !== 'function') {
    throw new Error(
      cfg.corre + ' em falta — recompile wasm com semântica: node tools/c_asm_shell.mjs sobe ' + nome
    )
  }
  return { ex, cfg }
}

function banco (base, nome) {
  const k = base + '\0' + nome
  if (!bancos.has(k)) bancos.set(k, { ...carregaWasm(nome), path: discoPath(base, nome) })
  return bancos.get(k)
}

function u8 (ex) {
  return new Uint8Array(ex.DISCO.buffer)
}

function discoCarrega (ex, path) {
  if (!existsSync(path)) return
  const mem = Buffer.from(ex.DISCO.buffer)
  const d = readFileSync(path)
  d.copy(mem, NULO, 0, Math.min(d.length, mem.length - NULO))
}

function discoGrava (ex, path) {
  const mem = Buffer.from(ex.DISCO.buffer)
  writeFileSync(path, mem.subarray(NULO, NULO + 65536))
}

function enc (ex, off, str) {
  const b = Buffer.from(str, 'utf8')
  u8(ex).set(b, NULO + off)
  return b.length
}

function dec (ex, off, len) {
  return Buffer.from(u8(ex).slice(NULO + off, NULO + off + len)).toString('utf8')
}

function leStdout (ex) {
  const mem = u8(ex)
  const n = mem[NULO + OFF_NOUT] + mem[NULO + OFF_NOUT + 1] * 256
  return Buffer.from(mem.slice(NULO + OFF_BUF_OUT, NULO + OFF_BUF_OUT + Math.min(n, 8192))).toString('utf8')
}

/** MOVE(−1) → semântica (corre) → MOVE(+1) — só wasm + DISCO. */
export function execMoveWasm (base, backend, script) {
  const nome = String(backend || '').toLowerCase()
  if (!BACKENDS[nome]) throw new Error('backend «' + backend + '»')
  const { label } = SLOTS[nome]
  const { ex, cfg, path } = banco(base, nome)

  discoCarrega(ex, path)

  const body = nome === 'bash' && script && !script.endsWith('\n') ? script + '\n' : script
  const n = enc(ex, 1024, body)
  const nw = ex[cfg.move](1024, n, 0, -1)
  if (nw !== n) throw new Error(cfg.move + '(-1) divergiu')

  const got = ex[cfg.corre]()
  if (got < 0) throw new Error(cfg.corre + ' falhou — semântica não reconhece o script')

  const outLen = ex[cfg.move](8192, 8192, 8192, +1)
  const stdout = dec(ex, 8192, outLen) || leStdout(ex)

  discoGrava(ex, path)

  const meta = {
    backend: nome,
    slotIn: S_CANAL_BASE + SLOTS[nome].in,
    slotOut: S_CANAL_BASE + SLOTS[nome].out,
    bytesIn: Buffer.byteLength(body, 'utf8'),
    bytesOut: Buffer.byteLength(stdout, 'utf8'),
    via: 'wasm+DISCO',
  }
  const logs = [
    `${label} MOVE -1: ${meta.bytesIn} bytes [wasm+DISCO]`,
    `${label} ${cfg.corre}: ${got} bytes [semântica]`,
    `${label} MOVE +1: ok [wasm+DISCO]`,
  ]
  return { stdout, logs, meta }
}

export function createShell (base) {
  function shellMove (label, nome, sentido, script) {
    if (sentido === +1) {
      const { ex, path } = banco(base, nome)
      discoCarrega(ex, path)
      const body = leStdout(ex)
      return { stdout: body, logs: [`${label} MOVE +1 [DISCO]`], meta: { backend: nome } }
    }
    if (sentido === -1 && script) {
      const { ex, cfg, path } = banco(base, nome)
      discoCarrega(ex, path)
      const n = enc(ex, 1024, script)
      ex[cfg.move](1024, n, 0, -1)
      discoGrava(ex, path)
      return { stdout: '', logs: [`${label} MOVE -1 [DISCO]`], meta: { backend: nome, bytesIn: script.length } }
    }
    return execMoveWasm(base, nome, script)
  }

  function execQuery (q) {
    const query = String(q || '').trim()
    for (const nome of Object.keys(SLOTS)) {
      const { label } = SLOTS[nome]
      const mv = parseMove(query, label)
      if (!mv) continue
      if (mv.err) throw new Error(`${label} MOVE literal invalido`)
      return shellMove(label, nome, mv.sentido, mv.script)
    }
    throw new Error(`query desconhecida — ${query.slice(0, 80)}`)
  }

  return {
    execQuery,
    execMove: (nome, script) => execMoveWasm(base, nome, script),
  }
}

/** Alias — o disco é o estado wasm persistido. */
export const execMoveDisco = execMoveWasm
