/* tests/pagina_u.js — schema da aplicação web (tríade html/css/js) no U canónico.
 *
 * Mesmo $id tiffany://u. Cliente/servidor = faces MOVE no canal, não órbita Hopfield.
 *   node tests/pagina_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { spawnSync } from 'node:child_process'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { paginaParaU, uParaPagina, igualPagina, TRIADE, SLOTS_FRONT } from '../app/src/banco_pagina_u.js'
import { igual } from '../app/src/banco_manifesto_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'pagina.json')
const DIR = join(RAIZ, 'app', 'banco')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const inst = JSON.parse(readFileSync(INST, 'utf8'))
const P = {
  html: readFileSync(join(DIR, 'pagina.html'), 'utf8'),
  css: readFileSync(join(DIR, 'pagina.css'), 'utf8'),
  js: readFileSync(join(DIR, 'pagina.js'), 'utf8'),
}
const U = paginaParaU(P, man)

ok('§W0 ficheiros da tríade no disco',
  existsSync(join(DIR, 'pagina.html')) &&
  existsSync(join(DIR, 'pagina.css')) &&
  existsSync(join(DIR, 'pagina.js')))
ok('§W0 instancia pagina no schema U (nao segundo $id)',
  inst.kind === 'pagina' && schema.$id === 'tiffany://u')
ok('§W0 ponte_pagina no motor',
  man.corpos?.motor?.ponte_pagina === 'app/src/banco_pagina_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_pagina)))

ok('§W1 kind=pagina, nao U nem Parte',
  U.kind === 'pagina' && U.id === 'pagina' && U.star !== 'D')
ok('§W1 faces cliente/servidor (MOVE ±1)',
  U.sentido === 0 && U.faces.menos.sentido === -1 && U.faces.mais.sentido === 1)
ok('§W1 tríade html,css,js',
  U.triade.join(',') === TRIADE.join(',') &&
  U.filhos.length === 3 &&
  U.filhos.every((f) => f.kind === 'lingua' && f.faces))

ok('§W2 P→U→P = P', igualPagina(uParaPagina(U), P))
ok('§W2 U→P→U = U', igual(paginaParaU(uParaPagina(U), man), U))

ok('§W3 js ≠ node',
  U.filhos.find((f) => f.id === 'js').faz !==
    (man.linguagens.find((l) => l.nome === 'node') || {}).faz)
ok('§W3 html/css/js sao linguas, nao Partes',
  TRIADE.every((n) => (man.linguagens || []).some((l) => l.nome === n)) &&
  !TRIADE.some((n) => (man.corpos?.lista || []).some((c) => c.parte === n)))
ok('§W3 fetch/http nao absorvidos como orbita',
  (man.fios || []).some((f) => f.nome === 'fetch' && f.absorvido === false) &&
  (man.fios || []).some((f) => f.nome === 'http' && f.absorvido === false))

ok('§W4 slots S_FRONT = S_CANAL+9200/9201',
  U.slots.in === SLOTS_FRONT.in && U.slots.out === SLOTS_FRONT.out &&
  U.slots.base === 'S_CANAL')
ok('§W4 kind pagina e formatos html/css/js no schema',
  schema.properties.kind.enum.includes('pagina') &&
  ['html', 'css', 'js'].every((f) => schema.properties.formato.enum.includes(f)))
ok('§W4 texto da pagina copiado 1:1 (nao adivinha DOM)',
  U.filhos.find((f) => f.id === 'html').texto === P.html &&
  U.filhos.find((f) => f.id === 'css').texto === P.css &&
  U.filhos.find((f) => f.id === 'js').texto === P.js)

ok('§W7 MVP: contrato do selo na pagina',
  P.html.includes('id="bk-contrato"') &&
  P.html.includes('id="bk-abre"') &&
  P.html.includes('id="bk-abre-fuse"') &&
  P.html.includes('id="bk-corre"') &&
  P.html.includes('id="bk-fecha"') &&
  P.html.includes('id="bk-selo"') &&
  P.html.includes('id="bk-canal"') &&
  P.html.includes('patria=1') &&
  P.html.includes('papel=coord') &&
  P.html.includes('papel=worker') &&
  P.html.includes('modo=fuse') &&
  P.html.includes('canal.patriatechnology.com'))
{
  const front = readFileSync(join(RAIZ, 'app', 'src', 'banco_front.js'), 'utf8')
  ok('§W7 front liga o motor (nao o js da triade)',
    front.includes('ligaContrato') &&
    front.includes('fechaContrato') &&
    front.includes('ligaContratoWss'))
  ok('§W7 contrato no pos-paint, nao no grafo do montaDom',
    !/from ['"]\.\/banco_coord_u\.js['"]/.test(front) &&
    front.includes("import('./banco_coord_u.js')") &&
    front.includes('capacidadesPosPaint'))
}

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
  ok('§W5 U da pagina cabe no schema', extra.length === 0)
}

{
  const r = spawnSync(process.execPath, [
    join(RAIZ, 'tools', 'pagina_u.mjs'), 'para-u', DIR,
  ], { encoding: 'utf8' })
  let doc = null
  try { doc = JSON.parse(r.stdout) } catch { doc = null }
  ok('§W6 CLI para-u', r.status === 0 && doc?.kind === 'pagina' && doc.filhos?.length === 3)
}

console.log('')
if (!falhas) {
  console.log('  Pagina web = nodo U; P→U→P = P; cliente/servidor = faces; fetch ≠ órbita.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
