/* tests/manifesto_u.js — ponte Manifesto ↔ Schema U (M↔U reversível).
 *
 * M = projecção corpos×órbitas. U = nodo canónico. Não adivinha Hebb.
 *   node tests/manifesto_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { spawnSync } from 'node:child_process'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import {
  matrizDoManifesto, manifestoParaU, uDeMatriz, uParaMatriz, uParaManifesto,
  igual, diffMatriz, chaveCorpo,
} from '../app/src/banco_manifesto_u.js'

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
const M = matrizDoManifesto(man)
const U = manifestoParaU(man)

ok('§U0 manifesto e schema no disco', existsSync(MAN) && existsSync(SCHEMA))
ok('§U0 ponte no motor',
  man.corpos?.motor?.ponte_u === 'app/src/banco_manifesto_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_u)))

const palcos = (man.corpos?.lista || []).filter((c) => c.camada === 'Fisica')
ok('§U1 14 palcos nas linhas',
  M.corpos.palcos === 14 && palcos.length === 14 &&
  M.linhas.filter((id) => palcos.some((c) => chaveCorpo(c) === id)).length === 14)
ok('§U1 orbitas = sql, latex, node',
  M.colunas.length === 3 &&
  M.colunas[0] === 'sql' && M.colunas[1] === 'latex' && M.colunas[2] === 'node')
ok('§U1 celulas = lista × orbitas',
  M.celulas.length === M.linhas.length * M.colunas.length &&
  M.linhas.length === (man.corpos?.lista || []).length)

ok('§U2 M→U→M = M', igual(uParaMatriz(manifestoParaU(man)), M))
ok('§U2 U→M→U = U', igual(uDeMatriz(uParaMatriz(U)), U))
ok('§U2 diff vazio no id',
  (() => {
    const d = diffMatriz(M, uParaMatriz(U))
    return d.corpos.length === 0 && d.orbitas.length === 0 && d.celulas.length === 0
  })())

ok('§U3 filhos: realizacao + orbita + celula',
  U.filhos.filter((n) => n.kind === 'realizacao').length === M.linhas.length &&
  U.filhos.filter((n) => n.kind === 'orbita').length === 3 &&
  U.filhos.filter((n) => n.kind === 'celula').length === M.celulas.length)
ok('§U3 cada nodo tem faces (autossimilar)',
  U.faces?.menos?.sentido === -1 && U.faces?.mais?.sentido === 1 &&
  U.filhos.every((n) => n.faces && n.faces.menos && n.faces.mais))
ok('§U3 U e gramatica, nao Parte',
  U.kind === 'U' && U.star === 'D' && U.estatuto === 'gramatica' && U.id === 'U')
ok('§U3 Alonzo e realizacao Fractal, nao U',
  U.filhos.some((n) => n.kind === 'realizacao' && n.id === 'Fractal' && n.canonico === 'Alonzo') &&
  U.id !== 'Alonzo')

ok('§U4 hopfield copiado, nao recomputado',
  U.hopfield?.W_s === man.hopfield?.W_s &&
  Array.isArray(U.hopfield?.orbitas) &&
  U.hopfield.orbitas.join(',') === 'sql,latex,node' &&
  typeof U.hopfield.W_s === 'string' && !Array.isArray(U.hopfield.W_s))
ok('§U4 celula nao adivinha ocupacao (orbita null)',
  M.celulas.every((c) => c.ocupada === false))
ok('§U4 relacoes Hopfield so em Redes × {sql,latex,node}',
  M.celulas.filter((c) => c.relacoes).length === 3 &&
  M.celulas.filter((c) => c.relacoes).every((c) => c.corpo === 'Redes'))

ok('§U5 matriz textual copiada (nao reinterpretada)',
  U.filhos.find((n) => n.id === 'Algebra')?.matriz ===
    palcos.find((c) => c.parte === 'Algebra')?.matriz &&
  U.filhos.find((n) => n.kind === 'celula' && n.corpo === 'Algebra' && n.orbita === 'sql')?.matriz ===
    palcos.find((c) => c.parte === 'Algebra')?.matriz)

ok('§U6 kinds da ponte no schema',
  ['orbita', 'celula'].every((k) => schema.properties.kind.enum.includes(k)))
ok('§U6 indice e hopfield no schema',
  schema.properties.indice && schema.properties.hopfield && schema.properties.corpos)

const chaves = new Set(M.linhas)
ok('§U7 chaves de corpo conservadas',
  (man.corpos?.lista || []).every((c, i) => chaves.has(chaveCorpo(c, i))))
ok('§U7 js ≠ node (linguas fora da matriz)',
  (man.linguagens || []).some((l) => l.nome === 'js') &&
  (man.linguagens || []).some((l) => l.nome === 'node') &&
  !M.colunas.includes('js') && M.colunas.includes('node'))
ok('§U7 asm nao e orbita nem linha',
  !M.colunas.includes('asm') && !M.linhas.includes('asm'))

const fatia = uParaManifesto(U)
ok('§U8 fatia manifesto: 3 orbitas + lista',
  fatia.orbitas.length === 3 &&
  fatia.corpos.lista.length === (man.corpos?.lista || []).length &&
  fatia.hopfield.canonico === 'Corpo Neural')

{
  const M2 = copiaMut(M)
  M2.corpos.lista = M2.corpos.lista.map((c) =>
    c.parte === 'Redes' ? { ...c, estatuto: 'realizado' } : c)
  const d = diffMatriz(M, M2)
  ok('§U9 ΔM aponta Redes quando o estado muda',
    d.corpos.includes('Redes') && d.corpos.length === 1)
}

function extraDoSchema (nodo, props, acc = []) {
  for (const k of Object.keys(nodo || {})) {
    if (!(k in props)) acc.push((nodo.kind || '?') + '.' + k)
  }
  if (nodo?.faces?.menos) extraDoSchema(nodo.faces.menos, props, acc)
  if (nodo?.faces?.mais) extraDoSchema(nodo.faces.mais, props, acc)
  for (const f of nodo?.filhos || []) extraDoSchema(f, props, acc)
  return acc
}
{
  const extra = extraDoSchema(U, schema.properties)
  ok('§U10 U da ponte cabe no schema (sem chaves extra)', extra.length === 0)
}

{
  const r = spawnSync(process.execPath, [
    join(RAIZ, 'tools', 'manifesto_u.mjs'), 'para-u', MAN,
  ], { encoding: 'utf8' })
  let doc = null
  try { doc = JSON.parse(r.stdout) } catch { doc = null }
  ok('§U11 CLI para-u', r.status === 0 && doc?.kind === 'U' && doc.filhos?.length === U.filhos.length)
}

function copiaMut (x) { return JSON.parse(JSON.stringify(x)) }

console.log('')
if (!falhas) {
  console.log('  Ponte M↔U: M→U→M = M; U→M→U = U; orbitas sql/latex/node; Hopfield copiado.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
