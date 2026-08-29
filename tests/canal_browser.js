/* canal_browser.js — trama S_CANAL bump-ada (§C0–§C4). Mesma banda que o metal.
 *
 *   node tests/canal_browser.js
 */
import { existsSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { bandaDeTecido, tramaBump, tramaClara, bump, keystream } from '../tools/banco_banda.mjs'
import {
  S_CANAL, S_BASH_IN, S_PWSH_IN, S_PWSH_OUT, S_CHUNK,
  S_FRONT_REQ, S_FRONT_RSP, S_NODE_IN,
} from '../tools/canal_slots.mjs'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const banda = bandaDeTecido('tecido por omissao')

ok('§C0 S_CANAL bate sql.c', S_CANAL === 9895936)
ok('§C0 S_BASH_IN', S_BASH_IN === S_CANAL + 9100)
ok('§C0 S_PWSH_IN/OUT', S_PWSH_IN === S_CANAL + 9110 && S_PWSH_OUT === S_CANAL + 9111)
ok('§C0 S_NODE_IN', S_NODE_IN === S_CANAL + 9120)
ok('§C4 S_CHUNK', S_CHUNK === S_CANAL + 9102)
ok('§C4 S_FRONT_REQ/RSP', S_FRONT_REQ === S_CANAL + 9200 && S_FRONT_RSP === S_CANAL + 9201)

const b1 = tramaBump(S_BASH_IN, 9, 1, banda)
ok('§C1 trama tem 6 bytes', b1.length === 6)

const t1 = tramaClara(b1, banda)
ok('§C1 volta slot', t1.slot === S_BASH_IN)
ok('§C1 volta Word', t1.total === 9 && t1.e === 1)

const ks = keystream(banda, 6)
const b2 = bump(b1, ks)
ok('§C2 bump∘bump = id (Lei 1)', Buffer.compare(b1, bump(b2, ks)) === 0)
ok('§C2 claro após involução', b2.readUInt32LE(0) === S_BASH_IN && b2[4] === 9 && b2[5] === 1)

const bandaErr = bandaDeTecido('outro tecido')
const t3 = tramaClara(b1, bandaErr)
ok('§C3 banda errada ≠ claro', !(t3.total === 9 && t3.e === 1 && t3.slot === S_BASH_IN))

const bPs = tramaBump(S_PWSH_IN, 4, 2, banda)
const tPs = tramaClara(bPs, banda)
ok('§C1 powershell volta slot', tPs.slot === S_PWSH_IN && tPs.total === 4 && tPs.e === 2)

ok('§C3 interpretar.wasm existe', existsSync(join(RAIZ, 'assets', 'figuras', 'wasm', 'interpretar.wasm')))
ok('§C3 powershell.wasm existe', existsSync(join(RAIZ, 'assets', 'figuras', 'wasm', 'powershell.wasm')))

console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
