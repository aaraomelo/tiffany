#!/usr/bin/env node
/* sobe_wasm_shell.mjs — recompila shells wasm com semântica (sem runtime). */
import { execFileSync } from 'node:child_process'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { sobeCadeia } from '../lib/c_asm_shell.mjs'

const __dir = dirname(fileURLToPath(import.meta.url))
const RAIZ = join(__dir, '..')
const TRADUZ = join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')

const shells = process.argv[2] === 'all' || !process.argv[2]
  ? ['node', 'bash', 'powershell']
  : [process.argv[2]]

for (const nome of shells) {
  const r = sobeCadeia(nome)
  console.log(`sobe_wasm_shell: ${nome} → ${r.wasmPath} (${r.wasm.length}B)`)
}

console.log('sobe_wasm_shell: pronto — node_corre/bash_corre/powershell_corre na wasm')
