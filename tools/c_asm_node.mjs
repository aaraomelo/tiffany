/* c_asm_node.mjs — compat: reexporta c_asm_shell (node). */
export {
  sobeC,
  wasmParaAsm,
  embuteAsm,
  sobeCadeia,
  desceCadeia,
  capasCadeia,
  garanteTraduz,
  shellConfig,
  SHELLS,
} from '../lib/c_asm_shell.mjs'

import { sobeCadeia, desceCadeia, shellConfig } from '../lib/c_asm_shell.mjs'
import fs from 'fs'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const RAIZ = path.join(__dirname, '..')
export const FONTE_C = path.join(RAIZ, 'conecthus', 'backends', 'node', 'interpretar.c')
export const WASM_OUT = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'node.wasm')
export const ERG_OUT = path.join(RAIZ, 'conecthus', 'backends', 'node', 'celula.erg')

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const cmd = process.argv[2]
  if (cmd === 'sobe') {
    const r = sobeCadeia('node')
    console.log(`c_asm_node: ${r.wasmPath} (${r.wasm.length}B) + ${r.ergPath}`)
  } else if (cmd === 'desce') {
    let entrada = null
    let saida = null
    for (let i = 3; i < process.argv.length; i++) {
      if (process.argv[i] === '-o') saida = process.argv[++i]
      else entrada = process.argv[i]
    }
    if (!entrada || !saida) {
      console.error('uso: c_asm_node.mjs desce <node.wasm> -o celula.erg')
      process.exit(2)
    }
    fs.writeFileSync(saida, desceCadeia('node', fs.readFileSync(entrada)), 'utf8')
    console.log(`c_asm_node: ${entrada} -> ${saida}`)
  } else if (cmd === 'corre') {
    const script = process.argv[3] || 'console.log(1)'
    import('./banco_shell_core.mjs').then(({ execMoveDisco }) => {
      const SQL_BASE = process.env.TIFFANY_SQL_BASE || path.join(RAIZ, '.torre', 'reino')
      const r = execMoveDisco(SQL_BASE, 'node', script)
      process.stdout.write(r.stdout || '')
    })
  } else {
    console.error('uso: c_asm_node.mjs sobe | desce <wasm> -o <erg> | corre <script>')
    process.exit(2)
  }
}
