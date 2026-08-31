/* tests/cristalchain_coord_wss_u.js — N clientes no mesmo WSS /canal.
 *   node tests/cristalchain_coord_wss_u.js
 * Fan-out do loopback: coord abre, 4 workers (id_i, s_i, R_i) enviam peca,
 * fecha Σ s_i + K = 0 e junta = monolito. Não reabre a Cadeia.
 * cripto:obs:coord-wss
 */
import http from 'node:http'
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { attachCanalLoopback } from '../tools/canal_loopback.mjs'
import { abrirCanalNode } from '../tools/canal_cliente.mjs'
import { bandaDeTecido } from '../tools/banco_banda.mjs'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { ligaIdentidade } from '../app/src/banco_identidade_u.js'
import { donoDaCadeia } from '../app/src/banco_cristalchain_u.js'
import {
  abreJob, correFaixa, fechaContrato, leJob, gravaJob, gravaParte,
  parteFaixas, somaMonolito,
} from '../app/src/banco_coord_u.js'
import { enviaAbre, enviaPeca, ouveCoord, ouvePeca } from '../app/src/banco_coord_canal.js'

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
ok('§Y0 ponte_coord_canal no disco',
  man.corpos?.motor?.ponte_coord_canal === 'app/src/banco_coord_canal.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_coord_canal)))
ok('§Y0 nucleo cita WSS realizado',
  /WSS/.test(man.corpos?.motor?.nucleo?.coord_distribuida || '') &&
  /realizado/.test(man.corpos?.motor?.nucleo?.coord_distribuida || ''))

const JOB = 'wss'
const N = 10000
const W = 4
const pubs = ['00'.repeat(32), '11'.repeat(32), '22'.repeat(32), '33'.repeat(32)]
const faixas = parteFaixas(N, W)
const alvo = somaMonolito(N)

const server = http.createServer()
attachCanalLoopback(server)
await new Promise((resolve) => server.listen(0, '127.0.0.1', resolve))
const port = server.address().port

const coordLs = memoriaLS()
const workerLs = [0, 1, 2, 3].map(() => memoriaLS())
const coord = abrirCanalNode(port)
const workers = [0, 1, 2, 3].map(() => abrirCanalNode(port))

try {
  const pecas = {}
  let nAbre = 0
  let resolvePecas
  const donePecas = new Promise((resolve, reject) => {
    resolvePecas = resolve
    setTimeout(() => reject(new Error('timeout pecas')), 20000)
  })

  for (let i = 0; i < W; i++) {
    ouvePeca(coord, i, (msg) => {
      if (msg.tipo !== 'peca') return
      pecas[msg.i | 0] = msg
      if (Object.keys(pecas).length >= W) resolvePecas()
    })
  }

  for (let i = 0; i < W; i++) {
    const idx = i
    ouveCoord(workers[idx], (msg) => {
      if (msg.tipo !== 'abre') return
      nAbre++
      Promise.resolve().then(async () => {
        const storage = workerLs[idx]
        gravaJob(storage, {
          id: msg.jobId,
          n: msg.n | 0,
          workers: msg.workers | 0,
          pecas: {},
          canal: String(msg.canal || ''),
          compromissos: msg.compromissos || [],
        })
        const parte = msg.partes[idx]
        gravaParte(storage, JOB, idx, parte)
        const id = await ligaIdentidade(storage, { chave: pubs[idx] })
        const { dono, camada } = await donoDaCadeia(id)
        const fx = msg.faixas[idx]
        const r = correFaixa({
          storage, dono, camada, jobId: JOB, i: idx,
          a: fx.a, b: fx.b, n: msg.n | 0, workers: msg.workers | 0, parte,
        })
        await enviaPeca(workers[idx], idx, {
          jobId: JOB, dono, a: fx.a, b: fx.b, soma: r.peca.soma, parte,
        })
      })
    })
  }

  await coord.pronto()
  await Promise.all(workers.map((w) => w.pronto()))

  const canalBytes = bandaDeTecido()
  const aberto = await abreJob({
    storage: coordLs, jobId: JOB, n: N, workers: W, canal: canalBytes,
  })
  await enviaAbre(coord, {
    jobId: JOB,
    n: N,
    workers: W,
    canal: aberto.job.canal,
    compromissos: aberto.job.compromissos,
    faixas,
    partes: aberto.partes,
  })
  await donePecas

  ok('§Y1 fan-out: os 4 workers viram abre no WSS', nAbre === W)

  const job = leJob(coordLs, JOB)
  for (let i = 0; i < W; i++) {
    const p = pecas[i]
    job.pecas[i] = {
      dono: String(p.dono || ''),
      i,
      a: p.a | 0,
      b: p.b | 0,
      soma: Number(p.soma),
      parte: String(p.parte || ''),
    }
  }
  gravaJob(coordLs, job)
  const f = await fechaContrato(leJob(coordLs, JOB))
  ok('§Y2 quatro pecas no coord', Object.keys(pecas).length === W)
  ok('§Y2 junta = monolito', f.junta === alvo)
  ok('§Y2 selo fecha (Σ s_i + K = 0)', f.fecha === 1)
  ok('§Y2 livros por id (donos distintos)',
    pecas[0].dono !== pecas[1].dono && pecas[2].dono !== pecas[3].dono)
} catch (e) {
  ok('§Y2 WSS contrato (' + (e.message || e) + ')', false)
} finally {
  coord.fechar()
  for (const w of workers) w.fechar()
  server.close()
}

console.log('')
if (!falhas) {
  console.log('  WSS: 4 workers + coord no mesmo /canal; selo fecha; cadeia intacta.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
