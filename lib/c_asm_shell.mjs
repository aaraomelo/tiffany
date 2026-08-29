/* c_asm_shell.mjs — cadeia C → assembly → shell (bash/node/powershell).
 *
 *   interpretar.c  ──traduz──►  *.wasm  ──wasm_para_erg──►  celula.erg
 *                                    └──── MOVE(±1) + canal ──► pleno
 */
import fs from 'fs'
import path from 'path'
import { fileURLToPath } from 'url'
import { execFileSync } from 'child_process'
import { wasmParaErg, embuteErgNoWasm, extraiErgDoWasm } from './wasm_para_erg.mjs'
import { monta } from './erg_monta.mjs'
import { appendCustom, SEC_FITA } from '../tools/asm_wasm.mjs'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const RAIZ = path.join(__dirname, '..')
const TRADUZ = path.join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')
const WASM_DIR = path.join(RAIZ, 'assets', 'figuras', 'wasm')

/** @type {Record<string, { fonte: string, wasm: string, erg: string, pleno: string, move: string, secErg: string }>} */
export const SHELLS = {
  node: {
    fonte: 'conecthus/backends/node/interpretar.c',
    wasm: 'node.wasm',
    erg: 'conecthus/backends/node/celula.erg',
    pleno: 'banco/node.c',
    move: 'node_move',
    secErg: 'node.erg',
  },
  bash: {
    fonte: 'conecthus/backends/bash/interpretar.c',
    wasm: 'interpretar.wasm',
    erg: 'conecthus/backends/bash/celula.erg',
    pleno: 'banco/bash.c',
    move: 'bash_move',
    secErg: 'bash.erg',
  },
  powershell: {
    fonte: 'conecthus/backends/powershell/interpretar.c',
    wasm: 'powershell.wasm',
    erg: 'conecthus/backends/powershell/celula.erg',
    pleno: 'banco/powershell.c',
    move: 'powershell_move',
    secErg: 'powershell.erg',
  },
}

export function shellConfig (nome) {
  const cfg = SHELLS[nome]
  if (!cfg) throw new Error('shell «' + nome + '»')
  return {
    ...cfg,
    fontePath: path.join(RAIZ, cfg.fonte),
    wasmPath: path.join(WASM_DIR, cfg.wasm),
    ergPath: path.join(RAIZ, cfg.erg),
    plenoPath: path.join(RAIZ, cfg.pleno),
  }
}

export function garanteTraduz () {
  if (!fs.existsSync(TRADUZ)) {
    execFileSync('cc', ['-O2', '-std=c99', '-w', path.join(RAIZ, 'tools', 'traduz.c'), '-o', TRADUZ])
  }
  return TRADUZ
}

export function sobeC (nome, dest) {
  const cfg = shellConfig(nome)
  const wasmPath = dest || cfg.wasmPath
  garanteTraduz()
  fs.mkdirSync(path.dirname(wasmPath), { recursive: true })
  execFileSync(TRADUZ, [cfg.fontePath, '-o', wasmPath])
  return fs.readFileSync(wasmPath)
}

export function wasmParaAsm (nome, wasmBuf) {
  const cfg = shellConfig(nome)
  return wasmParaErg(wasmBuf, { fonte: cfg.fonte })
}

export function embuteAsm (nome, wasmBuf, ergText) {
  const cfg = shellConfig(nome)
  let w = embuteErgNoWasm(wasmBuf, ergText, cfg.secErg)
  try {
    const fita = monta(ergText)
    w = appendCustom(w, SEC_FITA, fita)
  } catch (_) { /* disasm pode ter só comentários */ }
  return w
}

export function sobeCadeia (nome, opts = {}) {
  const cfg = shellConfig(nome)
  const wasmPath = opts.wasm || cfg.wasmPath
  const ergPath = opts.erg || cfg.ergPath
  const wasm = sobeC(nome, wasmPath)
  const erg = wasmParaAsm(nome, wasm)
  fs.mkdirSync(path.dirname(ergPath), { recursive: true })
  fs.writeFileSync(ergPath, erg, 'utf8')
  const wasmFull = embuteAsm(nome, wasm, erg)
  fs.writeFileSync(wasmPath, wasmFull)
  return { wasm: wasmFull, erg, wasmPath, ergPath, nome }
}

export function desceCadeia (nome, wasmBuf) {
  const cfg = shellConfig(nome)
  const erg = extraiErgDoWasm(wasmBuf, cfg.secErg)
  if (!erg) throw new Error('secção ' + cfg.secErg + ' em falta')
  return erg
}

export function capasCadeia (nome) {
  const cfg = shellConfig(nome)
  return {
    c: cfg.fonte,
    wasm: cfg.wasm,
    asm: cfg.erg,
    pleno: cfg.pleno,
    sequencia: ['c', 'wasm', 'asm', nome + '_pleno'],
  }
}

/** Sobe C→asm para todos os shells. */
export function sobeTodos () {
  return Object.keys(SHELLS).map((n) => sobeCadeia(n))
}
