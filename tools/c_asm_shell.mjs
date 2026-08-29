/* c_asm_shell.mjs — cadeia C → assembly → shell (bash/node/powershell).
 *
 *   node tools/c_asm_shell.mjs sobe bash
 *   node tools/c_asm_shell.mjs sobe all
 *   node tools/c_asm_shell.mjs desce bash assets/figuras/wasm/interpretar.wasm -o celula.erg
 *   node tools/c_asm_shell.mjs corre bash "echo 42"
 */
import fs from 'fs'
import path from 'path'
import { fileURLToPath } from 'url'
import {
  SHELLS,
  sobeCadeia,
  desceCadeia,
  sobeTodos,
} from '../lib/c_asm_shell.mjs'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const RAIZ = path.join(__dirname, '..')

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const cmd = process.argv[2]
  const alvo = process.argv[3]

  if (cmd === 'sobe') {
    if (!alvo || alvo === 'all') {
      for (const r of sobeTodos()) {
        console.log(`c_asm_shell: ${r.nome} → ${r.wasmPath} (${r.wasm.length}B)`)
      }
    } else if (SHELLS[alvo]) {
      const r = sobeCadeia(alvo)
      console.log(`c_asm_shell: ${r.wasmPath} (${r.wasm.length}B) + ${r.ergPath}`)
    } else {
      console.error('shell desconhecido:', alvo, '— use', Object.keys(SHELLS).join('|'), '| all')
      process.exit(2)
    }
  } else if (cmd === 'desce') {
    const nome = alvo
    let entrada = null
    let saida = null
    for (let i = 4; i < process.argv.length; i++) {
      if (process.argv[i] === '-o') saida = process.argv[++i]
      else if (!entrada) entrada = process.argv[i]
    }
    if (!SHELLS[nome] || !entrada || !saida) {
      console.error('uso: c_asm_shell.mjs desce <bash|node|powershell> <wasm> -o <erg>')
      process.exit(2)
    }
    fs.writeFileSync(saida, desceCadeia(nome, fs.readFileSync(entrada)), 'utf8')
    console.log(`c_asm_shell: ${entrada} -> ${saida}`)
  } else if (cmd === 'corre') {
    const nome = alvo
    const script = process.argv[4]
    if (!SHELLS[nome] || !script) {
      console.error('uso: c_asm_shell.mjs corre <bash|node|powershell> "<script>"')
      process.exit(2)
    }
    import('./banco_shell_core.mjs').then(({ execMoveDisco }) => {
      const SQL_BASE = process.env.TIFFANY_SQL_BASE || path.join(RAIZ, '.torre', 'reino')
      const r = execMoveDisco(SQL_BASE, nome, script)
      process.stdout.write(r.stdout || '')
    })
  } else {
    console.error('uso: c_asm_shell.mjs sobe <shell|all> | desce <shell> <wasm> -o <erg> | corre <shell> "<script>"')
    process.exit(2)
  }
}
