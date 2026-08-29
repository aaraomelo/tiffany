/* banco_metal.mjs — MOVE no metal: semântica (interpretar.c) → ERG → erg corre no DISCO.
 * fisica.tex: linguagens são realizações; ⊕/⊗ = MOVE(+1/−1); sem runtime externo. */
import { mkdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs'
import { join } from 'node:path'
import { execFileSync } from 'node:child_process'
import { fileURLToPath } from 'node:url'
import {
  arenaParaMem, memParaArena, seedScriptArena, readStdout,
  OFF_NOUT, OFF_OUT, CAP, recordCorreVisit, getG, G_CELL,
} from '../lib/arena_disco.mjs'
import { correNucleo, garanteFitaCorre } from '../lib/nucleo_metal.mjs'
import { parseMove, SLOTS, S_CANAL_BASE } from './banco_shell_parse.mjs'

const __dir = fileURLToPath(new URL('.', import.meta.url))
const RAIZ = join(__dir, '..')

const ERG_NEW = join(__dir, 'bin', process.platform === 'win32' ? 'erg_new.exe' : 'erg_new')
const ERG_BIN = process.env.TIFFANY_ERG || (existsSync(ERG_NEW) ? ERG_NEW : join(__dir, 'bin', process.platform === 'win32' ? 'erg.exe' : 'erg'))
const ERG_SRC = join(RAIZ, 'banco', 'erg.c')

function shellDir (base, nome) {
  const d = `${base}_${nome}`
  mkdirSync(d, { recursive: true })
  return d
}

export function garanteErg () {
  if (existsSync(ERG_BIN)) return ERG_BIN
  mkdirSync(join(__dir, 'bin'), { recursive: true })
  const cc = process.env.CC || (process.platform === 'win32' ? 'gcc' : 'cc')
  const objs = [
    '-O2', '-std=c99', '-w',
    `-I${join(RAIZ, 'lib')}`,
    ERG_SRC,
  ]
  if (process.platform === 'win32') {
    objs.push(join(RAIZ, 'lib', 'pread_posix.c'))
  }
  objs.push('-o', ERG_BIN)
  execFileSync(cc, objs)
  return ERG_BIN
}

/** Pré-monta fita do núcleo (cadeia C→wasm→ERG). */
export function garanteNucleo (backend) {
  const erg = garanteErg()
  return garanteFitaCorre(backend, erg)
}

/** MOVE no metal: arena DISCO → node_corre/bash_corre na fita → lê OFF_OUT.
 *  journal opcional: regista π(i) por visita (protocolo G_visit=G_real). */
export function execMoveMetal (base, backend, script, journal = null) {
  const nome = String(backend || '').toLowerCase()
  if (!SLOTS[nome]) throw new Error('backend «' + backend + '»')
  const { label } = SLOTS[nome]
  const erg = garanteErg()
  const dir = shellDir(base, nome)
  const arenaPath = join(dir, 'arena.bin')
  const memPath = join(dir, 'mem.dat')

  const wasmPath = join(RAIZ, 'assets', 'figuras', 'wasm', `${nome}.wasm`)
  const wasmBuf = existsSync(wasmPath) ? readFileSync(wasmPath) : null

  const arena = seedScriptArena(script, nome)
  writeFileSync(arenaPath, arena)
  writeFileSync(memPath, arenaParaMem(arena, wasmBuf))

  const { fitaPath, via, passos } = correNucleo(erg, nome, memPath)

  const outArena = memParaArena(readFileSync(memPath))
  recordCorreVisit(outArena, nome, journal)
  writeFileSync(arenaPath, outArena)
  writeFileSync(memPath, arenaParaMem(outArena, wasmBuf))
  const stdout = readStdout(outArena)

  const meta = {
    backend: nome,
    via: via + '+DISCO',
    fita: fitaPath,
    slotIn: S_CANAL_BASE + SLOTS[nome].in,
    slotOut: S_CANAL_BASE + SLOTS[nome].out,
    bytesIn: Buffer.byteLength(script, 'utf8'),
    bytesOut: Buffer.byteLength(stdout, 'utf8'),
    passos,
    gCell: G_CELL[nome],
    gVisit: getG(outArena, G_CELL[nome]),
  }
  const logs = [
    `${label} interpretar.c → wasm → ERG (${join(nome, '*.fita.bin')})`,
    `${label} erg corre no metal [DISCO]`,
    `${label} MOVE +1: ${meta.bytesOut} bytes`,
  ]
  return { stdout, logs, meta }
}

export function createShell (base) {
  function execQuery (q) {
    const query = String(q || '').trim()
    for (const nome of Object.keys(SLOTS)) {
      const { label } = SLOTS[nome]
      const mv = parseMove(query, label)
      if (!mv) continue
      if (mv.err) throw new Error(`${label} MOVE literal invalido`)
      if (mv.sentido === +1) {
        const arena = readFileSync(join(shellDir(base, nome), 'arena.bin'))
        const n = arena[OFF_NOUT] + arena[OFF_NOUT + 1] * 256
        return { stdout: arena.toString('utf8', OFF_OUT, OFF_OUT + n), logs: [], meta: { backend: nome } }
      }
      if (mv.sentido === -1 && mv.script) {
        const dir = shellDir(base, nome)
        const arena = seedScriptArena(mv.script)
        writeFileSync(join(dir, 'arena.bin'), arena)
        writeFileSync(join(dir, 'mem.dat'), arenaParaMem(arena, existsSync(join(RAIZ, 'assets', 'figuras', 'wasm', `${nome}.wasm`)) ? readFileSync(join(RAIZ, 'assets', 'figuras', 'wasm', `${nome}.wasm`)) : null))
        return { stdout: '', logs: [`${label} MOVE -1 [DISCO]`], meta: { backend: nome } }
      }
      return execMoveMetal(base, nome, mv.script)
    }
    throw new Error(`query desconhecida — ${query.slice(0, 80)}`)
  }
  return {
    execQuery,
    execMove: (nome, script) => execMoveMetal(base, nome, script),
  }
}

export const execMoveDisco = execMoveMetal
