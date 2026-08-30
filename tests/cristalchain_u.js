/* tests/cristalchain_u.js — Cadeia de Cristais no motor: identidade + liquidação.
 *   node tests/cristalchain_u.js
 * I0: nao e blockchain; sha256 nao e a cifra; det=+1 != Pisano do ouro.
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { ligaIdentidade } from '../app/src/banco_identidade_u.js'
import {
  B_OURO_BRANCO, CONTRATOS, AGENTES, CHAVE_CADEIA,
  det2, detFamilia, detPotenciaOuroBranco,
  reguaDe, liquida, passoCifra, pisano, orbitaOuro,
  liquidaNaCadeia, liquidaTodos, integraCadeia, donoDaCadeia,
  leLivro, cadeiaParaU,
} from '../app/src/banco_cristalchain_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const MOTOR = join(RAIZ, 'app', 'src', 'banco_cristalchain_u.js')
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

ok('§C0 ponte no disco',
  existsSync(MOTOR) &&
  man.corpos?.motor?.ponte_cristalchain === 'app/src/banco_cristalchain_u.js')
ok('§C0 sem kind cristalchain no schema',
  !schema.properties.kind.enum.includes('cristalchain') &&
  schema.properties.kind.enum.includes('orbita'))

ok('§C1 det familia metalica = -1; ouro branco = +1',
  detFamilia(1) === -1 && detFamilia(2) === -1 && detFamilia(3) === -1 &&
  det2(B_OURO_BRANCO) === 1)
ok('§C1 det(B^k)=+1 para k=1..6',
  [1, 2, 3, 4, 5, 6].every((k) => detPotenciaOuroBranco(k) === 1))
{
  const p = passoCifra(1, 0)
  ok('§C1 passo cifra B(1,0)=(4,1)', p.a === 4 && p.b === 1)
}

ok('§C2 cinco contratos no medidor', CONTRATOS.length === 5)
{
  let n = 0
  for (const c of CONTRATOS) {
    const L = liquida(c.termos, 12)
    if (L.liquidado) n++
  }
  ok('§C2 os cinco liquidam-se sozinhos', n === 5)
}
ok('§C2 sem termos nada liquida', !liquida([], 12).liquidado)
ok('§C2 agentes gira/estica/limite',
  AGENTES[0] === 'gira' && AGENTES[1] === 'estica' && AGENTES[2] === 'limite')

{
  const qs = [3, 4, 5, 7, 11, 12]
  const esperados = [8, 6, 20, 16, 10, 24]
  let discorda = 0
  let teto = 0
  for (let i = 0; i < qs.length; i++) {
    const o = orbitaOuro(qs[i])
    const pi = pisano(qs[i])
    if (pi !== esperados[i] || o.passos !== pi) discorda++
    if (o.passouTeto || !o.fechou) teto++
  }
  ok('§C3 Pisano 8,6,20,16,10,24 por Fibonacci, sem tocar na orbita da cifra',
    discorda === 0 && teto === 0)
}

ok('§C4 sem identidade nao ha livro',
  liquidaNaCadeia(CONTRATOS[0].termos, { nome: 'ouro' }).motivo.includes('identidade'))

{
  const disco = memoriaLS()
  const id = await ligaIdentidade(disco, { chave: pub })
  const { dono, camada } = await donoDaCadeia(id)
  ok('§C5 dono = id da chave, nao sha256-como-cifra',
    camada === 'chave' && dono === id.id && dono.length === 64)
  const a = liquidaNaCadeia(CONTRATOS[0].termos, {
    dono, camada, storage: disco, nome: 'ouro',
  })
  const b = liquidaNaCadeia(CONTRATOS[0].termos, {
    dono, camada, storage: disco, nome: 'ouro',
  })
  ok('§C5 primeira acrescenta; segunda idempotente',
    a.acrescentou === 1 && b.acrescentou === 0 &&
    leLivro(disco, dono).registos.length === 1)
  ok('§C5 chave LS prefixo cristalchain + id',
    disco.getItem(CHAVE_CADEIA + dono) &&
    JSON.parse(disco.getItem(CHAVE_CADEIA + dono)).dono === dono)
}

{
  const disco = memoriaLS()
  const id = await ligaIdentidade(disco, {})
  const pac = await integraCadeia(id, disco)
  ok('§C6 sessao tambem e dono',
    id.camada === 'sessao' && pac.dono === id.id && pac.camada === 'sessao')
  ok('§C6 integra coloca os cinco no livro',
    pac.livro.registos.length === 5 &&
    pac.resultados.every((r) => r.liquidado && r.acrescentou))
  const deNovo = await integraCadeia(id, disco)
  ok('§C6 reintegra nao duplica',
    deNovo.livro.registos.length === 5 &&
    deNovo.resultados.every((r) => r.acrescentou === 0))
  const u = cadeiaParaU(pac)
  ok('§C6 nodo U kind=orbita id=cristalchain',
    u.kind === 'orbita' && u.id === 'cristalchain' &&
    u.estatuto === 'realizado' && u.faces &&
    /blockchain/.test(u.proibicao) && u.slots.n === '5')
}

{
  const disco = memoriaLS()
  const a = await ligaIdentidade(disco, { chave: pub })
  const b = await ligaIdentidade(memoriaLS(), {})
  await integraCadeia(a, disco)
  const discoB = memoriaLS()
  await ligaIdentidade(discoB, {})
  const pacB = await integraCadeia(b, discoB)
  ok('§C7 livros de donos distintos nao se misturam',
    a.id !== b.id &&
    leLivro(disco, a.id).registos.length === 5 &&
    pacB.livro.dono === b.id)
}

ok('§C8 liquidaTodos devolve 5', liquidaTodos({ dono: 'x' }).length === 5)

console.log('')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
