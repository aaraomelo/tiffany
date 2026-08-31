/* tests/vite_u.js — esqueleto Vite do Golden Kingdom (id=gk, kind=pagina).
 *
 * Vite = hospedeiro, não kind novo. GLSL/LaTeX listam-se, não executam.
 *   node tests/vite_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import {
  ID_GK, MODULO_GK, CSS_GK, PUBLIC_DIR,
  parseEntradaHtml, parseImportsPrimeiroNivel, leEsqueleto, gkParaU, uParaGk, igualGk,
} from '../app/src/banco_vite_u.js'
import { igual } from '../app/src/banco_manifesto_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'vite.json')
const HTML = join(RAIZ, 'app', 'index.html')
const MAIN = join(RAIZ, 'app', 'src', 'main.js')
const VITE = join(RAIZ, 'app', 'vite.config.js')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')

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
const html = readFileSync(HTML, 'utf8')
const mainJs = readFileSync(MAIN, 'utf8')
const viteCfg = readFileSync(VITE, 'utf8')
const front = readFileSync(FRONT, 'utf8')
const G = leEsqueleto(html, mainJs)
const U = gkParaU(G, man)
const ent = parseEntradaHtml(html)

ok('§V0 index.html e main.js no disco', existsSync(HTML) && existsSync(MAIN))
ok('§V0 instancia gk no schema U (nao segundo $id)',
  inst.kind === 'pagina' && inst.id === ID_GK && schema.$id === 'tiffany://u')
ok('§V0 ponte_vite no motor',
  man.corpos?.motor?.ponte_vite === 'app/src/banco_vite_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_vite)))
ok('§V0 schema_vite no disco',
  man.corpos?.motor?.schema_vite === 'conecthus/schema/vite.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_vite)))

ok('§V1 kind=pagina id=gk, nao U nem Parte',
  U.kind === 'pagina' && U.id === 'gk' && U.star !== 'D')
ok('§V1 Vite nao e kind novo',
  schema.properties.kind.enum.includes('pagina') &&
  !schema.properties.kind.enum.includes('vite'))
ok('§V1 #app + type=module /src/main.js',
  ent.temApp && ent.eModulo && ent.modulo === MODULO_GK)
ok('§V1 css de entrada e style.css', G.css === CSS_GK)

ok('§V2 P→U→P = P', igualGk(uParaGk(U), G))
ok('§V2 U→P→U = U', igual(gkParaU(uParaGk(U), man), U))

ok('§V3 1o nivel lista tex_tradutor e substratos (nao executa)',
  G.imports.some((s) => s.includes('tex_tradutor')) &&
  G.imports.some((s) => s.includes('substratos.js')))
ok('§V3 motor_wasm no 1o nivel',
  G.imports.some((s) => s.includes('motor_wasm')))
ok('§V3 js da triade gk e o path do modulo, nao o bundle',
  U.filhos.find((f) => f.id === 'js').texto === MODULO_GK &&
  !U.filhos.find((f) => f.id === 'js').texto.includes('initMotorCampo'))
ok('§V3 html do esqueleto e index.html, nao pagina.html do banco',
  G.html.includes('id="app"') && !G.html.includes('id="banco"'))

ok('§V4 front nao importa main/tex/glsl',
  !/from ['"]\.\/main\.js['"]/.test(front) &&
  !/from ['"]\.\/tex_tradutor\.js['"]/.test(front) &&
  !/from ['"]\.\/substratos\.js['"]/.test(front) &&
  !/from ['"]\.\/motor_campo\.js['"]/.test(front) &&
  !/from ['"]\.\/motor_wasm\.js['"]/.test(front))
ok('§V4 iframe gk e pos-paint, nao import estatico',
  front.includes('function hospedaGk') &&
  front.includes("get('gk')") &&
  front.includes('iframe') &&
  front.includes('urlHospedadoGk') &&
  front.indexOf('montaDom(') < front.indexOf('hospedaGk(') &&
  front.indexOf('aposQuadro') < front.indexOf('hospedaGk('))
ok('§V4 Vite tem entradas / e /banco/',
  viteCfg.includes("main:") &&
  viteCfg.includes('./index.html') &&
  viteCfg.includes("banco:") &&
  viteCfg.includes('./banco/index.html'))
ok('§V4 publicDir figuras', G.publicDir === PUBLIC_DIR)

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
  ok('§V5 U do esqueleto cabe no schema', extra.length === 0)
}

ok('§V6 nucleo distingue Vite de pagina-banco',
  /hospedeiro/.test(man.corpos?.motor?.nucleo?.vite_esqueleto || '') &&
  /tardias/.test(man.corpos?.motor?.nucleo?.vite_esqueleto || ''))

{
  const imports = parseImportsPrimeiroNivel(mainJs)
  ok('§V6 parse para no primeiro statement que nao e import',
    imports[0] === './style.css' && imports.includes('./terminal.js'))
}

console.log('')
if (!falhas) {
  console.log('  Vite = hospedeiro; esqueleto id=gk; GLSL/LaTeX tardias; != pagina-banco.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
