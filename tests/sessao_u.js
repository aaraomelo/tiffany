/* tests/sessao_u.js — sessão remota no U: endereço + chave pública → bash/powershell.
 *
 *   node tests/sessao_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { createHash } from 'node:crypto'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { sessaoParaU, uParaSessao, paramsDaSessao, urlCanal, modoDaSessao, SHELLS_SESSAO, PATRIA_PUB, CANAL_ALIAS } from '../app/src/banco_sessao_u.js'
import { igual } from '../app/src/banco_manifesto_u.js'
import { bandaDeChave } from '../tools/banco_banda.mjs'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'
const S = { endereco: 'ws://192.0.2.10/canal', chave: pub }
const U = sessaoParaU(S, man)

ok('§R0 kind sessao no schema', schema.properties.kind.enum.includes('sessao'))
ok('§R0 formatos sh/ps1',
  schema.properties.formato.enum.includes('sh') &&
  schema.properties.formato.enum.includes('ps1'))
ok('§R0 ponte_sessao no motor',
  man.corpos?.motor?.ponte_sessao === 'app/src/banco_sessao_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_sessao)))

ok('§R1 kind=sessao, nao U', U.kind === 'sessao' && U.star !== 'D')
ok('§R1 faces MOVE ±1', U.faces.menos.sentido === -1 && U.faces.mais.sentido === 1)
ok('§R1 filhos bash e powershell',
  U.filhos.map((f) => f.id).join(',') === SHELLS_SESSAO.join(',') &&
  U.filhos.every((f) => f.kind === 'lingua'))

ok('§R2 S→U→S = S', igual(uParaSessao(U), S))
ok('§R2 U→S→U = U', igual(sessaoParaU(uParaSessao(U), man), U))

ok('§R3 params ?addr=&pub=',
  paramsDaSessao('?addr=' + S.endereco + '&pub=' + pub).chave === pub &&
  paramsDaSessao('?addr=' + S.endereco + '&pub=' + pub).endereco === S.endereco)
ok('§R3 host vira ws …/canal',
  urlCanal(null, 'ws://x/canal') === 'ws://x/canal' &&
  urlCanal('192.0.2.10').endsWith('192.0.2.10/canal'))
ok('§R3 sem addr+pub = solo',
  modoDaSessao(paramsDaSessao('')) === 'solo' &&
  modoDaSessao({ endereco: S.endereco, chave: '' }) === 'solo')
ok('§R3 addr+pub = remoto', modoDaSessao(S) === 'remoto' && U.modo === 'remoto')
ok('§R3 U solo sem chave', sessaoParaU({}, man).modo === 'solo')
{
  const j = JSON.parse(readFileSync(join(RAIZ, 'app', 'banco', 'patria.json'), 'utf8'))
  const b64 = j.ssh.trim().split(/\s+/)[1]
  const b = Buffer.from(b64, 'base64')
  let o = 0
  const n1 = b.readUInt32BE(o); o += 4; o += n1
  const n2 = b.readUInt32BE(o); o += 4
  const hex = b.subarray(o, o + n2).toString('hex')
  ok('§R3 patria.json pub = ed25519 raw, nao OpenSSH',
    hex === PATRIA_PUB && j.pub === PATRIA_PUB && PATRIA_PUB.length === 64)
  ok('§R3 ?patria=1 preenche pub',
    paramsDaSessao('?patria=1').chave === PATRIA_PUB &&
    paramsDaSessao('?patria=1').patria === true &&
    modoDaSessao(paramsDaSessao('?patria=1')) === 'solo')
  ok('§R3 patria+addr = remoto; papel/i no query',
    modoDaSessao(paramsDaSessao('?patria=1&addr=ws://127.0.0.1/canal')) === 'remoto' &&
    paramsDaSessao('?papel=worker&i=2').papel === 'worker' &&
    paramsDaSessao('?papel=worker&i=2').indice === 2)
  ok('§R3 alias genérico canal.patriatechnology.com',
    CANAL_ALIAS === 'canal.patriatechnology.com' &&
    urlCanal(CANAL_ALIAS).includes(CANAL_ALIAS + '/canal'))
  ok('§R3 ?modo=fuse e omissão parte',
    paramsDaSessao('?modo=fuse').modo === 'fuse' &&
    paramsDaSessao('?patria=1').modo === 'parte')
}

ok('§R4 banda = sha256(bytes da chave), nao utf8 do hex',
  Buffer.compare(
    bandaDeChave(pub),
    createHash('sha256').update(Buffer.from(pub, 'hex')).digest()
  ) === 0)
ok('§R4 mesma chave → mesma banda (utilizador da chave)',
  Buffer.compare(bandaDeChave(pub), bandaDeChave(pub.toUpperCase())) === 0)

ok('§R5 bash/powershell nao sao orbitas Hopfield',
  (man.hopfield?.orbitas || []).every((o) => !SHELLS_SESSAO.includes(o)))
ok('§R5 js ≠ node continua',
  (man.linguagens || []).find((l) => l.nome === 'js')?.faz !==
    (man.linguagens || []).find((l) => l.nome === 'node')?.faz)

{
  const extra = []
  function walk (n) {
    for (const k of Object.keys(n || {})) {
      if (!(k in schema.properties)) extra.push((n.kind || '?') + '.' + k)
    }
    if (n?.faces?.menos) walk(n.faces.menos)
    if (n?.faces?.mais) walk(n.faces.mais)
    for (const f of n?.filhos || []) walk(f)
  }
  walk(U)
  ok('§R6 sessao cabe no schema U', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  Sessao: addr+pub → U; banda=sha256(chave); bash/pwsh no canal, nao Hopfield.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
