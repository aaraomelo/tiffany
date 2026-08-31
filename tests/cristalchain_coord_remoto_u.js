/* tests/cristalchain_coord_remoto_u.js — recibo local ?→ recibo remoto.
 *   node tests/cristalchain_coord_remoto_u.js
 * 4 discos isolados. S_ESTADO não leva o recibo. Entrega KV sim.
 * Não é WSS/Patria ao vivo. Não é consenso. cripto:obs:coord-remoto
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import {
  CHAVE_ESTADO, estadoVazio, leEstado, gravaEstado, mergeEstado,
} from '../app/src/banco_disco.js'
import { ligaIdentidade } from '../app/src/banco_identidade_u.js'
import { donoDaCadeia, leLivro, CHAVE_CADEIA } from '../app/src/banco_cristalchain_u.js'
import {
  parteFaixas, somaMonolito, correFaixa, juntaJob, leJob,
  entregaPeca, entregaLivro, estadoCegoAoRecibo, CHAVE_COORD,
} from '../app/src/banco_coord_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
ok('§R0 ponte_coord + ponte_sync',
  man.corpos?.motor?.ponte_coord === 'app/src/banco_coord_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_coord)) &&
  man.corpos?.motor?.ponte_sync === 'app/src/banco_sync.js')

ok('§R0 S_ESTADO e o job sao chaves distintas',
  CHAVE_ESTADO === 'gk:banco:estado' &&
  CHAVE_COORD === 'gk:banco:coord:' &&
  CHAVE_CADEIA === 'gk:banco:cristalchain:')

ok('§R0 documento de estado e cego ao recibo',
  estadoCegoAoRecibo(estadoVazio()) &&
  estadoCegoAoRecibo(mergeEstado(estadoVazio(), estadoVazio())))

const pubs = ['00'.repeat(32), '11'.repeat(32), '22'.repeat(32), '33'.repeat(32)]
const workers = []
for (let i = 0; i < 4; i++) workers.push(memoriaLS())
const coord = memoriaLS()

const donos = []
for (let i = 0; i < 4; i++) {
  const id = await ligaIdentidade(workers[i], { chave: pubs[i] })
  const { dono, camada } = await donoDaCadeia(id)
  donos.push({ dono, camada })
}

ok('§R1 quatro discos, quatro ids; o coord comeca vazio',
  new Set(donos.map((d) => d.dono)).size === 4 &&
  workers.every((w, i) => w !== coord && w !== workers[(i + 1) % 4]) &&
  !leJob(coord, 'j1'))

const JOB = 'j1'
const faixas = parteFaixas(10000, 4)
const corridas = []
for (let i = 0; i < 4; i++) {
  corridas.push(correFaixa({
    storage: workers[i],
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

ok('§R2 cada worker liquida no seu disco',
  corridas.every((c) => c.liq.liquidado === 1 && c.liq.acrescentou === 1 && c.peca) &&
  workers.every((w, i) => leLivro(w, donos[i].dono).registos.length === 1) &&
  workers.every((w, i) => !leLivro(w, donos[(i + 1) % 4].dono).registos.length))

ok('§R2 o coordenador ainda nao tem o job',
  !leJob(coord, JOB) &&
  donos.every((d) => leLivro(coord, d.dono).registos.length === 0))

{
  for (let i = 0; i < 4; i++) {
    gravaEstado(estadoVazio(), workers[i])
    const merged = mergeEstado(leEstado(coord), leEstado(workers[i]))
    gravaEstado(merged, coord)
  }
  ok('§R3 S_ESTADO nao transporta o recibo',
    estadoCegoAoRecibo(leEstado(coord)) &&
    !leJob(coord, JOB) &&
    juntaJob(leJob(coord, JOB)) === null)
}

{
  const entregas = []
  for (let i = 0; i < 4; i++) entregas.push(entregaPeca(workers[i], coord, JOB, i))
  ok('§R4 entrega KV das pecas: junta = monolito',
    entregas.every((e) => e.entregou === 1) &&
    juntaJob(leJob(coord, JOB)) === somaMonolito(10000))
}

{
  const deNovo = correFaixa({
    storage: workers[1],
    dono: donos[1].dono,
    camada: donos[1].camada,
    jobId: JOB,
    i: 1,
    a: faixas[1].a,
    b: faixas[1].b,
    n: 10000,
    workers: 4,
  })
  const outra = entregaPeca(workers[1], coord, JOB, 1)
  ok('§R5 repetir worker + reentrega nao duplica',
    deNovo.liq.acrescentou === 0 &&
    outra.entregou === 0 &&
    Object.keys(leJob(coord, JOB).pecas).length === 4 &&
    juntaJob(leJob(coord, JOB)) === somaMonolito(10000))
}

{
  const livrosAntes = donos.map((d) => leLivro(coord, d.dono).registos.length)
  const liv = entregaLivro(workers[0], coord, donos[0].dono)
  ok('§R6 livro tambem e KV, nao S_ESTADO',
    livrosAntes.every((n) => n === 0) &&
    liv.entregou === 1 &&
    leLivro(coord, donos[0].dono).registos.length === 1 &&
    leLivro(coord, donos[1].dono).registos.length === 0 &&
    coord.getItem(CHAVE_CADEIA + donos[0].dono) &&
    !JSON.stringify(leEstado(coord)).includes(donos[0].dono.slice(0, 16)))
}

ok('§R7 cadeias dos workers nao se misturam',
  workers.every((w, i) => leLivro(w, donos[i].dono).registos[0].nome.endsWith(':faixa:' + i)))

ok('§R8 ainda nao e WSS ao vivo',
  !readFileSync(join(RAIZ, 'app', 'src', 'banco_coord_u.js'), 'utf8').includes('WebSocket'))

console.log('')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
