/* tests/cristalchain_coord_u.js — job artificial em 4 ids; livro = atribuição.
 *   node tests/cristalchain_coord_u.js
 * 1 banco 4 id (partilhado). NÃO são 4 workers remotos.
 * Distribuição ≠ consenso. Job ≠ Liquidacao. B não distribui.
 * cripto:obs:coord
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { ligaIdentidade } from '../app/src/banco_identidade_u.js'
import { donoDaCadeia, leLivro, liquida, CONTRATOS } from '../app/src/banco_cristalchain_u.js'
import {
  CHAVE_COORD, TERMOS_TESTEMUNHA,
  parteFaixas, somaFaixa, somaMonolito,
  correFaixa, juntaJob, leJob, clonaStorage, nomeFaixa,
} from '../app/src/banco_coord_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const MOTOR = join(RAIZ, 'app', 'src', 'banco_coord_u.js')
const FONTE = readFileSync(MOTOR, 'utf8')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
ok('§J0 ponte_coord no disco',
  existsSync(MOTOR) &&
  man.corpos?.motor?.ponte_coord === 'app/src/banco_coord_u.js')

ok('§J0 testemunha = ouro; faixa nao e termos',
  TERMOS_TESTEMUNHA === CONTRATOS[0].termos &&
  TERMOS_TESTEMUNHA.length === 6)

ok('§J0 coord nao chama B nem passa faixa a liquida',
  !FONTE.includes('B_OURO_BRANCO') &&
  !FONTE.includes('passoCifra') &&
  !FONTE.includes('liquida(') &&
  FONTE.includes('liquidaNaCadeia'))

{
  const faixas = parteFaixas(10000, 4)
  ok('§J1 quatro faixas cobrem 0..9999',
    faixas.length === 4 &&
    faixas[0].a === 0 && faixas[0].b === 2500 &&
    faixas[3].a === 7500 && faixas[3].b === 10000)
  ok('§J1 soma das faixas = monolito',
    faixas.reduce((s, f) => s + somaFaixa(f.a, f.b), 0) === somaMonolito(10000) &&
    somaMonolito(10000) === 49995000)
}

ok('§J1 faixa numerica nao e o objecto Liquidacao',
  JSON.stringify([0, 1, 2, 3, 4]) !== JSON.stringify(TERMOS_TESTEMUNHA))

const pubs = ['00'.repeat(32), '11'.repeat(32), '22'.repeat(32), '33'.repeat(32)]
const disco = memoriaLS()
const ids = []
for (const pub of pubs) {
  ids.push(await ligaIdentidade(disco, { chave: pub }))
}
const donos = []
for (const id of ids) {
  const { dono, camada } = await donoDaCadeia(id)
  donos.push({ dono, camada })
}

ok('§J2 quatro ids distintos no mesmo banco',
  new Set(donos.map((d) => d.dono)).size === 4 &&
  donos.every((d) => d.camada === 'chave' && d.dono.length === 64))

const JOB = 'j0'
const faixas = parteFaixas(10000, 4)
const corridas = []
for (let i = 0; i < 4; i++) {
  corridas.push(correFaixa({
    storage: disco,
    dono: donos[i].dono,
    camada: donos[i].camada,
    jobId: JOB,
    i,
    a: faixas[i].a,
    b: faixas[i].b,
    n: 10000,
    workers: 4,
  }))
}

ok('§J3 os quatro workers executam e liquidam',
  corridas.every((c) => c.liq.liquidado === 1 && c.liq.acrescentou === 1 && c.peca))

ok('§J3 cada resultado volta ao banco',
  corridas.every((c) => typeof c.peca.soma === 'number') &&
  leJob(disco, JOB) &&
  Object.keys(leJob(disco, JOB).pecas).length === 4 &&
  disco.getItem(CHAVE_COORD + JOB))

ok('§J3 cada R_i no livro do seu id; termos = testemunha',
  donos.every((d, i) => {
    const livro = leLivro(disco, d.dono)
    const rec = livro.registos[0]
    return livro.registos.length === 1 &&
      rec.nome === nomeFaixa(JOB, i) &&
      rec.termos.length === 6 &&
      rec.termos[0] === 0 && rec.termos[5] === 5 &&
      rec.soma === undefined
  }))

{
  const deNovo = correFaixa({
    storage: disco,
    dono: donos[1].dono,
    camada: donos[1].camada,
    jobId: JOB,
    i: 1,
    a: faixas[1].a,
    b: faixas[1].b,
    n: 10000,
    workers: 4,
  })
  ok('§J4 repetir um worker nao duplica a liquidacao',
    deNovo.liq.acrescentou === 0 &&
    leLivro(disco, donos[1].dono).registos.length === 1 &&
    Object.keys(leJob(disco, JOB).pecas).length === 4)
}

ok('§J5 juntar = processamento monolitico',
  juntaJob(leJob(disco, JOB)) === somaMonolito(10000))

{
  const clone = clonaStorage(disco)
  const id2 = await ligaIdentidade(clone, { chave: pubs[2] })
  const { dono, camada } = await donoDaCadeia(id2)
  const outra = correFaixa({
    storage: clone,
    dono,
    camada,
    jobId: JOB,
    i: 2,
    a: faixas[2].a,
    b: faixas[2].b,
    n: 10000,
    workers: 4,
  })
  ok('§J6 desligar/reconectar nao corrompe',
    dono === donos[2].dono &&
    outra.liq.acrescentou === 0 &&
    leLivro(clone, dono).registos.length === 1 &&
    juntaJob(leJob(clone, JOB)) === somaMonolito(10000) &&
    leJob(clone, JOB).pecas[2].soma === corridas[2].peca.soma)
}

ok('§J7 Liquidacao original intacta',
  liquida(CONTRATOS[0].termos, 12).liquidado === 1 &&
  liquida(CONTRATOS[4].termos, 12).liquidado === 1)

ok('§J7 prefixo coord != cristalchain',
  CHAVE_COORD === 'gk:banco:coord:' &&
  !CHAVE_COORD.includes('cristalchain'))

ok('§J8 um banco quatro id; nao quatro processos remotos',
  donos.every((d) => leLivro(disco, d.dono).dono === d.dono) &&
  leJob(disco, JOB).workers === 4 &&
  !FONTE.includes('browser') &&
  !FONTE.includes('WebSocket'))

console.log('')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
