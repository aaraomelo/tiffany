/* tests/absorve_node.js — node absorvido como latex/bash (§W10), não adaptador SQL.
 *   node tests/absorve_node.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const man = JSON.parse(readFileSync(
  join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const BASE = man.nulo_disco || 8
const node = man.linguagens.find((l) => l.nome === 'node')

ok('§A0 node tem absorcao no manifesto', node?.absorcao?.move === 'node_move')
ok('§A0 node slots canal', node?.absorcao?.slots?.in === 9120)
ok('§A0 node nao em fios', !(man.fios || []).find((f) => f.nome === 'node'))
ok('§A0 canal→node', !!man.arestas.find((e) => e.de === 'canal' && e.para === 'node'))

const wasmPath = join(RAIZ, 'assets', 'figuras', 'wasm', 'node.wasm')
if (!existsSync(wasmPath)) {
  ok('§A1 node.wasm', false)
} else {
  const N = new WebAssembly.Instance(new WebAssembly.Module(readFileSync(wasmPath)), {}).exports
  const m = new Uint8Array(N.DISCO.buffer)
  const js = Buffer.from('console.log(1)', 'utf8')
  m.set(js, BASE + 1024)
  ok('§A1 node_move -1', N.node_move(1024, js.length, 0, -1) === js.length)
  ok('§A1 node_pendente', N.node_pendente() === js.length)
  const stub = Buffer.from('1\n', 'utf8')
  for (let i = 0; i < stub.length; i++) m[BASE + 16384 + i] = stub[i]
  m[BASE + 24578] = stub.length & 255
  m[BASE + 24579] = (stub.length >> 8) & 255
  const nr = N.node_move(8192, 32, 8192, +1)
  const got = Buffer.from(m.slice(BASE + 8192, BASE + 8192 + nr)).toString('utf8')
  ok('§A1 node_move +1 roundtrip arena', got === '1\n')
}

ok('§A2 banco_absorve.js', existsSync(join(RAIZ, 'app', 'src', 'banco_absorve.js')))
ok('§A2 sem banco_semantica', !existsSync(join(RAIZ, 'app', 'src', 'banco_semantica.js')))

console.log(`\n=== absorve_node: ${feitas - falhas}/${feitas} OK ===`)
process.exit(falhas ? 1 : 0)
