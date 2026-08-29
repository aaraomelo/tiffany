/* tools/sobe_backends_wasm.mjs — sobe linguagens[] via traduz. Sem python hospedeiro. */
import { existsSync, mkdirSync, readFileSync, statSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { spawnSync } from 'node:child_process'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const OUT = join(RAIZ, 'assets', 'figuras', 'wasm')
const MAN = JSON.parse(readFileSync(join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))
const TRADUZ = existsSync(join(RAIZ, 'tools', 'bin', 'traduz.exe'))
  ? join(RAIZ, 'tools', 'bin', 'traduz.exe')
  : join(RAIZ, 'tools', 'bin', 'traduz')

mkdirSync(OUT, { recursive: true })
if (!existsSync(TRADUZ)) {
  console.error('traduz em falta:', TRADUZ)
  process.exit(1)
}

let falhas = 0
for (const L of MAN.linguagens) {
  const fonte = join(RAIZ, L.fonte)
  const wasm = join(OUT, L.wasm)
  if (!existsSync(fonte)) {
    console.error('  FALTA fonte ' + L.nome + ': ' + fonte)
    falhas++
    continue
  }
  console.log('  → ' + L.wasm + '  (' + L.fonte + ')')
  const r = spawnSync(TRADUZ, [fonte, '-o', wasm], { encoding: 'utf8' })
  if (r.stdout) process.stdout.write(r.stdout)
  if (r.status !== 0) {
    process.stderr.write(r.stderr || 'traduz falhou\n')
    falhas++
    continue
  }
  console.log('     ' + statSync(wasm).size + ' bytes · exports ' + JSON.stringify(L.exports || []))
}
process.exit(falhas)
