/* canal_patria.js — contrato do lado branco UDP (banco/canal_patria.c).
 *
 *   node tests/canal_patria.js
 *
 * Não sobe o binário POSIX. Mede o fio: slots, grupo/porta, bump 6B no datagrama.
 */
import { readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import dgram from 'node:dgram'
import { bandaDeTecido, tramaBump, tramaClara } from '../tools/banco_banda.mjs'
import {
  S_CANAL, S_BASH_IN, S_BASH_OUT, S_CHUNK, S_PWSH_IN, S_PWSH_OUT,
  S_NODE_IN, S_NODE_OUT, S_FRONT_REQ, S_FRONT_RSP, S_ESTADO_REQ, S_ESTADO_RSP,
  S_DEPOSITO_REQ, S_DEPOSITO_RSP,
} from '../tools/canal_slots.mjs'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const SRC = readFileSync(join(RAIZ, 'banco', 'canal_patria.c'), 'utf8')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function offsetC (nome) {
  const m = SRC.match(new RegExp('#define ' + nome + '\\s+\\(S_CANAL \\+ (\\d+)u\\)'))
  return m ? Number(m[1]) : NaN
}

ok('§P0 S_CANAL JS = 9895936', S_CANAL === 9895936)
ok('§P0 S_BASH_IN C', S_BASH_IN === S_CANAL + offsetC('S_BASH_IN'))
ok('§P0 S_BASH_OUT C', S_BASH_OUT === S_CANAL + offsetC('S_BASH_OUT'))
ok('§P0 S_CHUNK C', S_CHUNK === S_CANAL + offsetC('S_CHUNK'))
ok('§P0 S_PWSH_IN C', S_PWSH_IN === S_CANAL + offsetC('S_PWSH_IN'))
ok('§P0 S_PWSH_OUT C', S_PWSH_OUT === S_CANAL + offsetC('S_PWSH_OUT'))
ok('§P0 S_NODE_IN C', S_NODE_IN === S_CANAL + offsetC('S_NODE_IN'))
ok('§P0 S_NODE_OUT C', S_NODE_OUT === S_CANAL + offsetC('S_NODE_OUT'))
ok('§P0 S_FRONT_REQ C', S_FRONT_REQ === S_CANAL + offsetC('S_FRONT_REQ'))
ok('§P0 S_FRONT_RSP C', S_FRONT_RSP === S_CANAL + offsetC('S_FRONT_RSP'))
ok('§P0 S_ESTADO_REQ C', S_ESTADO_REQ === S_CANAL + offsetC('S_ESTADO_REQ'))
ok('§P0 S_ESTADO_RSP C', S_ESTADO_RSP === S_CANAL + offsetC('S_ESTADO_RSP'))
ok('§P0 S_DEPOSITO_REQ C', S_DEPOSITO_REQ === S_CANAL + offsetC('S_DEPOSITO_REQ'))
ok('§P0 S_DEPOSITO_RSP C', S_DEPOSITO_RSP === S_CANAL + offsetC('S_DEPOSITO_RSP'))

ok('§P0 grupo omissão', SRC.includes('"239.7.31.27"'))
ok('§P0 porta omissão', /47313u/.test(SRC))
ok('§P0 atende bash', SRC.includes('slot == S_BASH_IN'))
ok('§P0 atende node', SRC.includes('slot == S_NODE_IN'))
ok('§P0 atende powershell', SRC.includes('slot == S_PWSH_IN'))
ok('§P0 atende front', SRC.includes('slot == S_FRONT_REQ'))
ok('§P0 atende estado', SRC.includes('slot == S_ESTADO_REQ') && SRC.includes('gk/banco/estado.json'))
ok('§P0 atende deposito opaco',
  SRC.includes('slot == S_DEPOSITO_REQ') && SRC.includes('gk/banco/deposito.bin') &&
  !/GKBANCO/.test((SRC.match(/serve_deposito[\s\S]*?serve_estado/m) || [''])[0]))

const GRUPO = '239.7.31.27'
const PORTA = Number(process.env.TIFFANY_CANAL_TEST_PORTA) || 47913
const banda = bandaDeTecido('tecido por omissao')

function udpIdaVolta (slot, total, e) {
  return new Promise((resolve, reject) => {
    const sock = dgram.createSocket({ type: 'udp4', reuseAddr: true })
    const to = setTimeout(() => {
      sock.close()
      reject(new Error('timeout UDP'))
    }, 3000)
    sock.on('error', (err) => {
      clearTimeout(to)
      sock.close()
      reject(err)
    })
    sock.on('message', (msg) => {
      if (msg.length !== 6) return
      clearTimeout(to)
      const t = tramaClara(msg, banda)
      sock.close()
      resolve(t)
    })
    sock.bind(PORTA, () => {
      try {
        sock.setMulticastLoopback(true)
        sock.setMulticastTTL(1)
        sock.addMembership(GRUPO)
      } catch (err) {
        clearTimeout(to)
        sock.close()
        reject(err)
        return
      }
      const frame = tramaBump(slot, total, e, banda)
      sock.send(frame, PORTA, GRUPO, (err) => {
        if (err) {
          clearTimeout(to)
          sock.close()
          reject(err)
        }
      })
    })
  })
}

try {
  const t = await udpIdaVolta(S_NODE_IN, 9, 1)
  ok('§P1 UDP trama 6B volta slot', t.slot === S_NODE_IN)
  ok('§P1 UDP volta Word', t.total === 9 && t.e === 1)
  const tPs = await udpIdaVolta(S_PWSH_IN, 4, 2)
  ok('§P1 UDP powershell slot', tPs.slot === S_PWSH_IN && tPs.total === 4 && tPs.e === 2)
} catch (err) {
  ok('§P1 UDP multicast loopback', false)
  console.log('       ', String(err.message || err).split('\n')[0])
}

console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
