/* traduz_c_wasm_shell.js — C↔wasm volta para bash/node/powershell (duomorfismo).
 *   node tests/traduz_c_wasm_shell.js
 */
'use strict'

const fs = require('fs')
const os = require('os')
const path = require('path')
const { execFileSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TMP = process.env.TEMP || process.env.TMPDIR || os.tmpdir()
const TRADUZ = path.join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')

const SHELLS = [
  { nome: 'node', fonte: 'conecthus/backends/node/interpretar.c', wasm: 'node.wasm', move: 'node_move' },
  { nome: 'bash', fonte: 'conecthus/backends/bash/interpretar.c', wasm: 'interpretar.wasm', move: 'bash_move' },
  { nome: 'powershell', fonte: 'conecthus/backends/powershell/interpretar.c', wasm: 'powershell.wasm', move: 'powershell_move' },
]

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function difBytes (a, b) {
  let d = 0
  for (let i = 0; i < Math.max(a.length, b.length); i++) if (a[i] !== b[i]) d++
  return d
}

for (const sh of SHELLS) {
  const fonte = path.join(RAIZ, sh.fonte)
  const wasmTmp = path.join(TMP, `tws_${sh.nome}.wasm`)
  const voltaC = path.join(TMP, `tws_${sh.nome}_volta.c`)
  const wasm2 = path.join(TMP, `tws_${sh.nome}_volta.wasm`)
  try {
    execFileSync(TRADUZ, [fonte, '-o', wasmTmp])
    const a = fs.readFileSync(wasmTmp)
    execFileSync(TRADUZ, [wasmTmp, '-o', voltaC])
    execFileSync(TRADUZ, [voltaC, '-o', wasm2])
    const b = fs.readFileSync(wasm2)
    ok(`§TS ${sh.nome} sobe(desce(M))=M`, difBytes(a, b) === 0)
    const ex = WebAssembly.Module.exports(new WebAssembly.Module(a))
    ok(`§TS ${sh.nome} export ${sh.move}`, ex.some((e) => e.name === sh.move))
    const outWasm = path.join(RAIZ, 'assets', 'figuras', 'wasm', sh.wasm)
    fs.mkdirSync(path.dirname(outWasm), { recursive: true })
    fs.copyFileSync(wasmTmp, outWasm)
  } catch (e) {
    ok(`§TS ${sh.nome} volta traduz`, false)
    console.log('       ', String(e.message || e).split('\n')[0])
  }
}

console.log(`\n=== traduz_c_wasm_shell: ${feitas - falhas}/${feitas} OK ===`)
process.exit(falhas ? 1 : 0)
