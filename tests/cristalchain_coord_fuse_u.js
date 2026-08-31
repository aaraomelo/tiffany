/* tests/cristalchain_coord_fuse_u.js — N chaves independentes → σ colectivo.
 *   node tests/cristalchain_coord_fuse_u.js
 * Fuse ≠ separaChave (1→N). Fuse ≠ soma em Z_{2^256}. Fuse ≠ Ed25519.
 * Cada K_i → id_i; remover ou forjar impede o fecho; os id_i não se apagam.
 * cripto:obs:coord-fuse
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { ligaIdentidade } from '../app/src/banco_identidade_u.js'
import { donoDaCadeia } from '../app/src/banco_cristalchain_u.js'
import { somaZ, hexParaZ, zParaHex, MOD_BANDA } from '../app/src/banco_selo_u.js'
import { fuse, validaFuse, compromissoDeChave } from '../app/src/banco_fuse_u.js'
import {
  abreJobFuse, correFaixa, fechaContrato, leJob, parteFaixas, somaMonolito,
  estadoCegoAoRecibo,
} from '../app/src/banco_coord_u.js'
import { leEstado } from '../app/src/banco_disco.js'

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
ok('§F0 ponte_fuse no disco',
  man.corpos?.motor?.ponte_fuse === 'app/src/banco_fuse_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_fuse)))
ok('§F0 nucleo distingue fuse de particao',
  /fuse/.test(man.corpos?.motor?.nucleo?.coord_distribuida || '') &&
  /particao|pedaços|separa/.test(man.corpos?.motor?.nucleo?.coord_distribuida || ''))

const Ks = ['00'.repeat(32), '11'.repeat(32), '22'.repeat(32), '33'.repeat(32)]

{
  const r = await fuse(Ks)
  ok('§F1 quatro chaves independentes fecham fuse', r.ok === 1 && r.ids.length === 4)
  ok('§F1 ids distintos', new Set(r.ids).size === 4)
  const soma = somaZ(Ks.map((k) => hexParaZ(k, MOD_BANDA)), MOD_BANDA)
  ok('§F1 σ ≠ soma em Z_2^256 (nao trata chave como inteiro)',
    r.sigma !== zParaHex(soma, MOD_BANDA))
  const ordem = await fuse([Ks[3], Ks[1], Ks[0], Ks[2]])
  ok('§F1 ordem irrelevante (canónico por id)', ordem.sigma === r.sigma)
}

{
  const r = await fuse(Ks)
  const vFalta = await validaFuse(['', Ks[1], Ks[2], Ks[3]])
  ok('§F2 remover K_0 impede o fecho', vFalta.fecha === 0)
  const mau = Ks.slice()
  mau[0] = 'aa'.repeat(32)
  const vForja = await validaFuse(mau, r.sigma)
  ok('§F2 alterar K_0 invalida σ', vForja.fecha === 0 && vForja.motivo === 'forja')
  const vClone = await fuse([Ks[0], Ks[0], Ks[2], Ks[3]])
  ok('§F2 a mesma chave duas vezes é clone, nao N identidades',
    vClone.ok === 0 && vClone.motivo === 'clone')
  const vOk = await validaFuse(Ks, r.sigma)
  ok('§F2 as N + σ fecham', vOk.fecha === 1)
}

{
  const ids = []
  for (const k of Ks) {
    const id = await ligaIdentidade(memoriaLS(), { chave: k })
    ids.push(id.id)
  }
  const r = await fuse(Ks)
  ok('§F3 id_i = identidade da chave (nao se apaga)',
    r.ids.slice().sort().join(',') === ids.slice().sort().join(','))
  const c0 = await compromissoDeChave(Ks[0])
  ok('§F3 compromisso da chave ≠ hex da chave', c0 !== Ks[0] && c0.length === 64)
}

{
  const ls = memoriaLS()
  const N = 10000
  const W = 4
  await abreJobFuse({ storage: ls, jobId: 'fuse', n: N, workers: W })
  const faixas = parteFaixas(N, W)
  for (let i = 0; i < W; i++) {
    const id = await ligaIdentidade(ls, { chave: Ks[i] })
    const { dono, camada } = await donoDaCadeia(id)
    correFaixa({
      storage: ls, dono, camada, jobId: 'fuse', i,
      a: faixas[i].a, b: faixas[i].b, n: N, workers: W,
      chave: Ks[i], idFuse: id.id,
    })
  }
  const f = await fechaContrato(leJob(ls, 'fuse'))
  ok('§F4 coord fuse fecha; junta = monolito',
    f.fecha === 1 && f.junta === somaMonolito(N) && f.sigma && f.ids.length === 4)
  ok('§F4 S_ESTADO cego ao fuse; job vive no coord',
    estadoCegoAoRecibo(leEstado(ls)) &&
    leJob(ls, 'fuse') && leJob(ls, 'fuse').modo === 'fuse')
}

console.log('')
if (!falhas) {
  console.log('  Fuse: N chaves → σ; ≠ partição; ≠ soma Z; ids intactos.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
