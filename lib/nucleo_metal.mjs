/* nucleo_metal.mjs — núcleo semântico no metal: C → traduz → wasm → ERG → erg corre.
 * fisica.tex §fis:def:duomorf · duomorfismo-pipe.md — sem runtime, só DISCO. */
import { existsSync, readFileSync, writeFileSync, mkdirSync, statSync } from 'node:fs'
import { join } from 'node:path'
import { execFileSync } from 'node:child_process'
import { fileURLToPath } from 'node:url'
import { monta } from './erg_monta.mjs'
import { exportWasmParaErg } from './wasm_erg.mjs'
import { shellConfig, garanteTraduz, sobeC } from './c_asm_shell.mjs'

const __dir = fileURLToPath(new URL('.', import.meta.url))
const RAIZ = join(__dir, '..')

const BACKENDS = {
  node: { corre: 'node_corre' },
  bash: { corre: 'bash_corre' },
  powershell: { corre: 'powershell_corre' },
}

function mtime (p) {
  try { return statSync(p).mtimeMs } catch { return 0 }
}

function wasmErgBin () {
  return join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'wasm_erg.exe' : 'wasm_erg')
}

function parseConstsErg (text) {
  const consts = new Map()
  for (const line of String(text || '').split('\n')) {
    const m = line.match(/^; CONST (\d+) (\d+)/)
    if (m) consts.set(Number(m[1]), Number(m[2]))
  }
  return consts
}

/** Semeia ; CONST slot val como Word (total,e) no mem.dat. */
export function semeiaConsts (erg, memPath, consts) {
  execFileSync(erg, ['poe', memPath, '0', '0', '0'])
  for (const [slot, val] of consts) {
    const lo = val & 0xff
    const hi = (val >> 8) & 0xff
    execFileSync(erg, ['poe', memPath, String(slot), String(lo), String(hi)])
  }
}

/** Garante fita.bin do núcleo *_corre (interpretar.c → wasm → ERG → monta). */
export function garanteFitaCorre (backend, ergBin, opts = {}) {
  const cfg = BACKENDS[backend]
  if (!cfg) throw new Error('backend «' + backend + '» sem núcleo')
  const shell = shellConfig(backend)
  const dir = join(RAIZ, 'conecthus', 'backends', backend)
  const fitaPath = join(dir, `${cfg.corre}.fita.bin`)
  const ergPath = join(dir, `${cfg.corre}.erg`)
  const wasmPath = shell.wasmPath
  const fonte = shell.fontePath

  const srcM = mtime(fonte)
  const wasmM = mtime(wasmPath)
  const fitaM = mtime(fitaPath)
  const ergM = mtime(ergPath)
  const wasmErgM = mtime(wasmErgBin())

  if (fitaM > 0 && fitaM >= srcM && fitaM >= wasmM && fitaM >= ergM &&
      (!existsSync(wasmErgBin()) || fitaM >= wasmErgM) && !opts.force) {
    const ergText = existsSync(ergPath) ? readFileSync(ergPath, 'utf8') : ''
    return { fitaPath, ergPath, ergText, consts: parseConstsErg(ergText) }
  }

  garanteTraduz()
  sobeC(backend, wasmPath)
  mkdirSync(dir, { recursive: true })

  let erg
  let consts
  const exe = wasmErgBin()
  if (existsSync(exe)) {
    execFileSync(exe, [wasmPath, cfg.corre, ergPath])
    erg = readFileSync(ergPath, 'utf8')
    consts = parseConstsErg(erg)
  } else {
    const wasm = readFileSync(wasmPath)
    const r = exportWasmParaErg(wasm, cfg.corre, { fonte: shell.fonte })
    erg = r.erg
    consts = r.consts
    writeFileSync(ergPath, erg, 'utf8')
  }
  const fita = monta(erg)
  writeFileSync(fitaPath, fita)
  return { fitaPath, ergPath, ergText: erg, consts }
}

/** Corre *_corre no metal: mem.dat já tem arena (MOVE −1). */
export function correNucleo (erg, backend, memPath, opts = {}) {
  const { fitaPath, consts } = garanteFitaCorre(backend, erg, opts)
  semeiaConsts(erg, memPath, consts)
  const teto = opts.teto ?? 2_000_000
  const out = execFileSync(erg, ['corre', fitaPath, memPath, String(teto)], { encoding: 'utf8' })
  const m = out.match(/(\d+) passos/)
  const passos = m ? Number(m[1]) : 0
  return { fitaPath, via: 'c→wasm→erg→metal', passos }
}

export function nomeCorre (backend) {
  return BACKENDS[backend]?.corre
}
