/* traduz_c_asm_shell.js — cadeia C → assembly → shell (bash + framework).
 *   node tests/traduz_c_asm_shell.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { tmpdir } from 'node:os'
import { fileURLToPath } from 'node:url'
import {
  sobeC, wasmParaAsm, sobeCadeia, desceCadeia, shellConfig, SHELLS,
} from '../lib/c_asm_shell.mjs'
import { execMoveDisco, S_CANAL_BASE, SLOTS } from '../tools/banco_shell_core.mjs'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const TMP = process.env.TEMP || process.env.TMPDIR || tmpdir()

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

async function testShell (nome, cas, script, expectOut) {
  const cfg = shellConfig(nome)
  ok(`§${cas}0 ${nome} interpretar.c`, existsSync(cfg.fontePath))

  let wasm0
  try {
    wasm0 = sobeC(nome, join(TMP, `${nome}_limpo.wasm`))
    ok(`§${cas}1 sobe(${nome}) → wasm`, wasm0.length > 100)
    const ex = WebAssembly.Module.exports(new WebAssembly.Module(wasm0))
    ok(`§${cas}1 export ${SHELLS[nome].move}`, ex.some((e) => e.name === SHELLS[nome].move))
  } catch (e) {
    ok(`§${cas}1 sobe C→wasm`, false)
    console.log('       ', String(e.message || e).split('\n')[0])
    return
  }

  let erg
  try {
    erg = wasmParaAsm(nome, wasm0)
    ok(`§${cas}2 wasm→assembly`, erg.length > 50 && erg.includes(SHELLS[nome].move.replace('_move', '')))
  } catch (e) {
    ok(`§${cas}2 wasm→assembly`, false)
    console.log('       ', String(e.message || e).split('\n')[0])
  }

  let wasmFull
  try {
    const r = sobeCadeia(nome, {
      wasm: join(TMP, `${nome}_full.wasm`),
      erg: join(TMP, `${nome}_celula.erg`),
    })
    wasmFull = r.wasm
    ok(`§${cas}3 ${SHELLS[nome].secErg} embutido`, desceCadeia(nome, wasmFull).includes('_move'))
    ok(`§${cas}3 wasm cresceu`, wasmFull.length > wasm0.length)
  } catch (e) {
    ok(`§${cas}3 embute assembly`, false)
    console.log('       ', String(e.message || e).split('\n')[0])
  }

  if (wasmFull && erg) {
    const volta = desceCadeia(nome, wasmFull)
    ok(`§${cas}4 desce ≈ wasmParaAsm`, volta.trim() === erg.trim())
  }

  try {
    const SQL_BASE = join(RAIZ, `.torre/reino_bench_${nome}`)
    const r = execMoveDisco(SQL_BASE, nome, script)
    ok(`§${cas}5 ${nome} MOVE stdout`, (r.stdout || '').includes(expectOut))
    ok(`§${cas}5 slots canal`, r.meta?.slotIn === S_CANAL_BASE + SLOTS[nome].in)
  } catch (e) {
    ok(`§${cas}5 ${nome} via MOVE`, false)
    console.log('       ', String(e.message || e).split('\n')[0])
  }
}

ok('§CAS0 shells registrados', Object.keys(SHELLS).join(',') === 'node,bash,powershell')

await testShell('bash', 'CB', 'echo 42', '42')

const man = JSON.parse(readFileSync(join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))
const bash = man.linguagens.find((l) => l.nome === 'bash')
ok('§CAS6 bash cadeia asm', !!bash?.cadeia?.asm)
ok('§CAS6 ponte wasm_sec.c', bash?.cadeia?.ponte_asm === 'tools/wasm_sec.c')
ok('§CAS6 sequencia bash', bash?.cadeia?.sequencia?.includes('asm'))
ok('§CAS7 medidor shell', man.operacoes.medidor_cadeia.includes('traduz_c_asm_shell'))

console.log(`\n=== traduz_c_asm_shell: ${feitas - falhas}/${feitas} OK ===`)
process.exit(falhas ? 1 : 0)
