/* banco_metal.js — núcleo no metal: interpretar.c → wasm → ERG → erg corre.
 *   node tests/banco_metal.js
 */
'use strict'
import { join, dirname } from 'node:path'
import { existsSync } from 'node:fs'
import { fileURLToPath } from 'node:url'
import { execMoveMetal, garanteErg, garanteNucleo } from '../tools/banco_metal.mjs'

const __dir = dirname(fileURLToPath(import.meta.url))
const RAIZ = join(__dir, '..')
const BASE = join(RAIZ, '.torre', 'reino_test_metal')

let falhas = 0, feitas = 0
function ok (q, c) { feitas++; if (!c) falhas++; console.log(`#UNIT ${c ? 'ok' : 'falha'} ${q}`) }

ok('§MT0 garanteErg', existsSync(garanteErg()))
ok('§MT0 garanteNucleo node', garanteNucleo('node').fitaPath.endsWith('node_corre.fita.bin'))

try {
  const r = execMoveMetal(BASE, 'node', "console.log('metal')")
  ok('§MT1 node_corre stdout', r.stdout.trim() === 'metal')
  ok('§MT1 via cadeia física', r.meta.via.includes('c→wasm→erg→metal'))
  ok('§MT1 node passos fusão', r.meta.passos > 0 && r.meta.passos <= 3000)
  const b = execMoveMetal(BASE, 'bash', 'echo bench\n')
  ok('§MT1 bash_corre', b.stdout.includes('bench'))
  ok('§MT1 bash passos fusão', b.meta.passos > 0 && b.meta.passos <= 5000)
} catch (e) {
  ok('§MT1 metal: ' + (e.message || e), false)
}

console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
