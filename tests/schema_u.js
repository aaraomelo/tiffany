/* tests/schema_u.js — schema canónico do Corpo Universal (autossimilar).
 *
 * JSON é canónico. Completa = cruz das duas metades (Alonzo). U ≠ Parte ≠ Alonzo.
 *
 *   node tests/schema_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import {
  detectaFormato, parseClaim, parseFicheiro, nodoU, nodoLingua, metade, emit,
} from '../app/src/banco_schema.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'u.json')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const CLAIM = join(RAIZ, 'conecthus', 'claims', 'pareto.claim')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const inst = JSON.parse(readFileSync(INST, 'utf8'))
const man = JSON.parse(readFileSync(MAN, 'utf8'))

ok('§S0 schema e instancia no disco', existsSync(SCHEMA) && existsSync(INST))
ok('§S0 $id tiffany://u', schema.$id === 'tiffany://u')
ok('§S0 autossimilar: faces $ref #',
  schema.properties.faces.properties.menos.$ref === '#' &&
  schema.properties.filhos.items.$ref === '#')
ok('§S0 JSON e formato canónico', schema.properties.formato.enum[0] === 'json')

ok('§S1 instancia kind=U star=D gramatica',
  inst.kind === 'U' && inst.star === 'D' && inst.estatuto === 'gramatica')
ok('§S1 U nao e Parte', inst.kind !== 'realizacao' && inst.id === 'U')
ok('§S1 completa tem duas metades',
  inst.sentido === 0 && inst.faces.menos.sentido === -1 && inst.faces.mais.sentido === 1)
ok('§S1 metades tem o mesmo id (cruz)',
  inst.faces.menos.id === 'U' && inst.faces.mais.kind === 'metade')

const u = nodoU()
ok('§S2 nodoU autossimilar', u.faces.menos.kind === 'metade' && u.star === 'D')
ok('§S2 X dim=8; 3 dobras geram 8 leis; coordenadas',
  u.X.dim === 8 && u.X.dobras === 3 && u.X.leis === 8 &&
  u.X.base === 'e_k=2^k' && u.X.gram === 'Id' &&
  inst.X.dim === 8 && inst.X.dobras === 3 && inst.X.coord.includes('e_k'))
ok('§S2 metade nao inventa N/A', metade(u, -1).estatuto === 'gramatica')

const src = readFileSync(CLAIM, 'utf8')
ok('§S3 detecta .claim', detectaFormato(CLAIM, src) === 'claim')
const cl = parseClaim(src)
ok('§S3 claim Pareto no schema', cl.kind === 'claim' && cl.id === 'ParetoClosure' && cl.law === 6)
const volta = emit(cl, 'claim')
ok('§S3 claim emit∘parse = id (nome/law/step)',
  parseClaim(volta).id === cl.id && parseClaim(volta).step === cl.step)
ok('§S3 claim AS json tem faces', JSON.parse(parseFicheiro(CLAIM, src)).faces.menos.sentido === -1)

const manTxt = readFileSync(MAN, 'utf8')
ok('§S4 manifesto detectado', detectaFormato(MAN, manTxt) === 'manifesto')
const doc = JSON.parse(parseFicheiro(MAN, manTxt))
ok('§S4 manifesto → U com 19 filhos lingua',
  doc.kind === 'U' && Array.isArray(doc.filhos) && doc.filhos.length === 19)
ok('§S4 cada lingua tem schema proprio (kind=lingua + faces)',
  doc.filhos.every((f) => f.kind === 'lingua' && f.faces && f.faces.menos && f.faces.mais))
ok('§S4 js ≠ node nos filhos',
  doc.filhos.find((f) => f.id === 'js').faz !== doc.filhos.find((f) => f.id === 'node').faz)
ok('§S4 asm nao e filho', !doc.filhos.some((f) => f.id === 'asm'))

const sql = man.linguagens.find((l) => l.nome === 'sql')
const nsql = nodoLingua(sql)
ok('§S5 realizacao sql move=sql_move', nsql.move === 'sql_move' && nsql.p === 1)
ok('§S5 metade sql SENTIDO -1', metade(nsql, -1).sentido === -1 && metade(nsql, -1).kind === 'metade')

ok('§S6 motor declara schema',
  man.corpos?.motor?.schema === 'conecthus/schema/u.schema.json')
ok('§S6 parser no motor',
  man.corpos?.motor?.parser === 'banco/parse_ficheiro.h')
ok('§S6 Alonzo e Fractal, nao U',
  man.corpos.lista.find((c) => c.parte === 'Fractal')?.canonico === 'Alonzo' &&
  inst.id !== 'Alonzo')

const tex = parseFicheiro('catalogo.tex', '\\fichaingestao{Fractal}{}', { as: 'json' })
ok('§S7 tex = ficha nao localizada (nao se rele I0)',
  JSON.parse(tex).kind === 'ficha' && JSON.parse(tex).estatuto === 'nao localizada')

const sqlOut = emit(cl, 'sql')
ok('§S8 emite sql', sqlOut.startsWith("INSERT TEXTO 'schema/ParetoClosure"))
ok('§S8 emite tex', emit(cl, 'tex').includes('ficharow'))

console.log('')
console.log(falhas ? '' : '  Schema U autossimilar; JSON canónico; parser de ficheiros no motor.')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
