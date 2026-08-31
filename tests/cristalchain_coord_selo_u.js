/* tests/cristalchain_coord_selo_u.js — 1 chave do canal → N complementares.
 *   node tests/cristalchain_coord_selo_u.js
 * Algebra de tests/selo.c no motor: Σ partes + canal = 0; todos carregam o canal;
 * o contrato só fecha com as N; uma forja derruba o conjunto.
 * Livros por id intactos. S_ESTADO continua cego. cripto:obs:coord-nqubits
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import {
  CHAVE_ESTADO, estadoVazio, leEstado, gravaEstado, mergeEstado,
} from '../app/src/banco_disco.js'
import { ligaIdentidade } from '../app/src/banco_identidade_u.js'
import { donoDaCadeia, leLivro } from '../app/src/banco_cristalchain_u.js'
import { bandaDeTecido } from '../app/src/banda.js'
import {
  MOD_BANDA, P_SOMA, P_VARREDURA,
  separaChave, fechaSelo, recuperaParte, contaFalta, validaConjunto,
  zParaHex, rngLcg,
} from '../app/src/banco_selo_u.js'
import {
  parteFaixas, somaMonolito, correFaixa, juntaJob, leJob,
  entregaPeca, entregaCanal, abreJob, fechaContrato, gravaParte, leParte,
  estadoCegoAoRecibo, CHAVE_COORD,
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
ok('§S0 ponte_selo + ponte_coord no disco',
  man.corpos?.motor?.ponte_selo === 'app/src/banco_selo_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_selo)) &&
  man.corpos?.motor?.ponte_coord === 'app/src/banco_coord_u.js' &&
  /selo/.test(man.corpos?.motor?.nucleo?.coord_distribuida || ''))
ok('§S0 MVP pagina /banco/ leva o contrato',
  /realizado/.test(man.mvp?.selo || '') &&
  man.mvp.selo.includes('bk-contrato') &&
  readFileSync(join(RAIZ, 'app', 'banco', 'pagina.html'), 'utf8').includes('id="bk-contrato"'))

{
  const rng = rngLcg(20260830)
  const K = 424242n % P_SOMA
  const partes = separaChave(K, 6, { mod: P_SOMA, rng })
  ok('§S3 seis partes + canal somam zero (selo.c)',
    partes.length === 6 && fechaSelo(K, partes, P_SOMA))
  const rng2 = rngLcg(7)
  const p4 = separaChave(K, 4, { mod: P_SOMA, rng: rng2 })
  ok('§S3 N=4 tambem fecha', fechaSelo(K, p4, P_SOMA))
}

{
  const rng = rngLcg(11)
  const K = 314159n % P_SOMA
  const partes = separaChave(K, 6, { mod: P_SOMA, rng })
  let mau = 0
  for (let j = 0; j < 6; j++) {
    const r = recuperaParte(K, partes, j, P_SOMA)
    if (r !== partes[j]) mau++
  }
  ok('§S4 qualquer parte recupera-se das outras + canal', mau === 0)
  ok('§S4 N-1 nao fecha o contrato (nao se usa o atalho)',
    !fechaSelo(K, partes.slice(0, 5), P_SOMA))
}

{
  const rng = rngLcg(97)
  const partes = separaChave(11n, 4, { mod: P_VARREDURA, rng })
  const K = 11n
  const c = contaFalta(partes, 3, K, P_VARREDURA)
  ok('§S5 sem o canal a falta e o espaco inteiro', c.sem === Number(P_VARREDURA))
  ok('§S5 com o canal a falta e unica', c.com === 1)
}

const canal = await bandaDeTecido('tecido por omissao')
ok('§C0 banda do canal tem 32 bytes', canal.length === 32)

{
  const rng = rngLcg(256)
  const aberto = await abreJob({
    storage: memoriaLS(),
    jobId: 'probe',
    n: 8,
    workers: 4,
    canal,
    rng,
  })
  const hexes = aberto.partes
  const vazio = await validaConjunto(aberto.job.canal, hexes.slice(0, 3), aberto.job.compromissos)
  const forjado = hexes.slice()
  forjado[1] = zParaHex((BigInt('0x' + forjado[1]) + 1n) % MOD_BANDA)
  const vforja = await validaConjunto(aberto.job.canal, forjado, aberto.job.compromissos)
  const vok = await validaConjunto(aberto.job.canal, hexes, aberto.job.compromissos)
  ok('§N uma falta nao fecha', vazio.fecha === 0 && vazio.motivo === 'falta')
  ok('§N uma forja derruba o conjunto', vforja.fecha === 0 && vforja.motivo === 'forja')
  ok('§N as N + canal fecham', vok.fecha === 1)
}

const pubs = ['00'.repeat(32), '11'.repeat(32), '22'.repeat(32), '33'.repeat(32)]
const workers = []
for (let i = 0; i < 4; i++) workers.push(memoriaLS())
const coord = memoriaLS()
const donos = []
for (let i = 0; i < 4; i++) {
  const id = await ligaIdentidade(workers[i], { chave: pubs[i] })
  donos.push(await donoDaCadeia(id))
}

const JOB = 'jselo'
const faixas = parteFaixas(10000, 4)
const aberto = await abreJob({
  storage: coord,
  jobId: JOB,
  n: 10000,
  workers: 4,
  canal,
  rng: rngLcg(42),
})

ok('§J0 coord abre com canal e sem pecas',
  aberto.partes.length === 4 &&
  leJob(coord, JOB).canal.length === 64 &&
  leJob(coord, JOB).compromissos.length === 4 &&
  Object.keys(leJob(coord, JOB).pecas).length === 0 &&
  !leParte(coord, JOB, 0))

for (let i = 0; i < 4; i++) {
  gravaParte(workers[i], JOB, i, aberto.partes[i])
  entregaCanal(coord, workers[i], JOB)
}

ok('§J1 todos carregam a chave do canal',
  workers.every((w) => leJob(w, JOB).canal === leJob(coord, JOB).canal) &&
  workers.every((w, i) => leParte(w, JOB, i) === aberto.partes[i]) &&
  workers.every((w, i) => !leParte(w, JOB, (i + 1) % 4)))

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

ok('§J2 cada worker liquida no seu id e leva a parte',
  corridas.every((c) => c.liq.liquidado === 1 && c.peca && c.peca.parte) &&
  donos.every((d, i) => leLivro(workers[i], d.dono).registos.length === 1) &&
  new Set(donos.map((d) => d.dono)).size === 4)

{
  for (let i = 0; i < 4; i++) {
    gravaEstado(estadoVazio(), workers[i])
    gravaEstado(mergeEstado(leEstado(coord), leEstado(workers[i])), coord)
  }
  const f = await fechaContrato(leJob(coord, JOB))
  ok('§J3 S_ESTADO nao fecha o contrato',
    estadoCegoAoRecibo(leEstado(coord)) &&
    !CHAVE_ESTADO.includes('coord') &&
    f.fecha === 0)
}

{
  entregaPeca(workers[0], coord, JOB, 0)
  entregaPeca(workers[1], coord, JOB, 1)
  entregaPeca(workers[2], coord, JOB, 2)
  const f3 = await fechaContrato(leJob(coord, JOB))
  const rec = recuperaParte(
    leJob(coord, JOB).canal,
    [0, 1, 2, 3].map((i) => (leJob(coord, JOB).pecas[i] || {}).parte || aberto.partes[i]),
    3,
  )
  ok('§J4 tres pecas nao fecham (mesmo podendo recuperar a quarta)',
    f3.fecha === 0 && f3.motivo === 'falta' &&
    rec === BigInt('0x' + aberto.partes[3]))
}

{
  entregaPeca(workers[3], coord, JOB, 3)
  const f = await fechaContrato(leJob(coord, JOB))
  ok('§J5 as N + canal fecham; junta = monolito',
    f.fecha === 1 &&
    f.junta === somaMonolito(10000) &&
    juntaJob(leJob(coord, JOB)) === somaMonolito(10000))
}

{
  const job = leJob(coord, JOB)
  const orig = job.pecas[2].parte
  job.pecas[2] = { ...job.pecas[2], parte: zParaHex((BigInt('0x' + orig) + 1n) % MOD_BANDA) }
  const f = await fechaContrato(job)
  ok('§J6 uma forja derruba o conjunto, nao so a peca',
    f.fecha === 0 && f.motivo === 'forja' && f.junta === null)
}

ok('§J7 prefixo coord intacto; livros nao colapsaram num so id',
  CHAVE_COORD === 'gk:banco:coord:' &&
  donos.every((d, i) => leLivro(workers[i], d.dono).dono === d.dono) &&
  new Set(donos.map((d) => d.dono)).size === 4)

console.log('')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
