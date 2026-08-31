/* tests/nav_u.js — F1: Host → tenant → gk → navegação original (#id).
 *
 * iframe = oráculo. Não reinterpreta main.js. nav(foo) ⊥ nav(bar); id(K) pode coincidir.
 *   node tests/nav_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { CHAVE_ESTADO } from '../app/src/banco_disco.js'
import {
  SUFIXO_TENANT, CABECALHO_TENANT, discoIsolado, tenantDeHeaders, prefixoDisco,
} from '../app/src/banco_tenant_u.js'
import {
  ID_NAV_GK, CHAVE_NAV, ANCORAS_FIXAS,
  ancoraDeHash, ancorasDoManifesto, hrefsDoMain,
  urlHospedadoGk, urlBancoGk, navPerp, leAncora, gravaAncora,
  navParaU, uParaNav, igualNav,
} from '../app/src/banco_nav_u.js'
import { ligaIdentidade, idEstavelDaChave } from '../app/src/banco_identidade_u.js'
import { igual } from '../app/src/banco_manifesto_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'nav.json')
const CONF = join(RAIZ, 'app', 'nginx', 'goldenkingdom.conf')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const MAIN = join(RAIZ, 'app', 'src', 'main.js')
const MAN_GK = join(RAIZ, 'app', 'src', 'manifesto.json')
const PONTE = join(RAIZ, 'app', 'src', 'banco_nav_u.js')

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
const conf = readFileSync(CONF, 'utf8')
const front = readFileSync(FRONT, 'utf8')
const mainJs = readFileSync(MAIN, 'utf8')
const ponte = readFileSync(PONTE, 'utf8')
const gkMan = JSON.parse(readFileSync(MAN_GK, 'utf8'))
const ancoras = ancorasDoManifesto(gkMan)
const hrefs = hrefsDoMain(mainJs)
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'
const HOST_FOO = 'foo' + SUFIXO_TENANT
const HOST_BAR = 'bar' + SUFIXO_TENANT

ok('§N0 instancia nav id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_NAV_GK &&
  !schema.properties.kind.enum.includes('nav'))
ok('§N0 ponte_nav no motor',
  man.corpos?.motor?.ponte_nav === 'app/src/banco_nav_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_nav)))
ok('§N0 schema_nav no disco',
  man.corpos?.motor?.schema_nav === 'conecthus/schema/nav.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_nav)))
ok('§N0 nucleo nav_gk',
  /iframe oraculo/.test(man.corpos?.motor?.nucleo?.nav_gk || ''))

ok('§N1 ancoras incluem secoes + fixas do original',
  gkMan.secoes.every((s) => ancoras.includes(s.id)) &&
  ANCORAS_FIXAS.every((id) => ancoras.includes(id)))
ok('§N1 hrefs literais de main.js estao nas ancoras (oraculo)',
  hrefs.length > 0 && hrefs.every((id) => ancoras.includes(id)))
ok('§N1 ponte nao executa main/tex/glsl',
  !/from ['"]\.\/main\.js['"]/.test(ponte) &&
  !/tex_tradutor/.test(ponte) &&
  !/motor_campo/.test(ponte))

ok('§N2 url hospedado e o original; nunca ?tenant=',
  urlHospedadoGk({ ancora: 'coracao', tenant: 'foo' }) === '/#coracao' &&
  urlHospedadoGk({}) === '/' &&
  !urlHospedadoGk({ ancora: 'coracao', tenant: 'foo' }).includes('tenant') &&
  !urlBancoGk({ ancora: 'coracao', tenant: 'foo' }).includes('tenant='))
ok('§N2 /banco/?gk=1#ancora = entrada hospedada',
  urlBancoGk({ ancora: 'coracao' }) === '/banco/?gk=1#coracao' &&
  urlBancoGk({}) === '/banco/?gk=1')
ok('§N2 Host → tenant → gk (X-Tenant casa com Host)',
  tenantDeHeaders({ [CABECALHO_TENANT]: 'foo' }, HOST_FOO).tenant === 'foo' &&
  urlHospedadoGk({ ancora: 'inicio' }) === '/#inicio')

const N = { tenant: 'foo', ancora: 'coracao' }
const U = navParaU(N)
ok('§N3 kind=pagina id=gk, nao U',
  U.kind === 'pagina' && U.id === 'gk' && U.star !== 'D' && U.tenant === 'foo')
ok('§N3 P→U→P = P', igualNav(uParaNav(U), N))
ok('§N3 U→P→U = U', igual(navParaU(uParaNav(U)), U))

ok('§N4 nav(foo) perp nav(bar) mesmo ancora',
  navPerp({ tenant: 'foo', ancora: 'coracao' }, { tenant: 'bar', ancora: 'coracao' }) &&
  !navPerp({ tenant: 'foo', ancora: 'coracao' }, { tenant: 'foo', ancora: 'trailer' }))

{
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  gravaAncora(foo, 'coracao')
  gravaAncora(bar, 'trailer')
  foo.setItem(CHAVE_ESTADO, 'E')
  ok('§N4 ancoras isoladas; S_ESTADO nao se parte',
    leAncora(foo) === 'coracao' &&
    leAncora(bar) === 'trailer' &&
    ls.getItem(CHAVE_NAV) === null &&
    ls.getItem(prefixoDisco('foo') + CHAVE_NAV) != null &&
    foo.getItem(CHAVE_ESTADO) === 'E' &&
    bar.getItem(CHAVE_ESTADO) === 'E')
}

const identFoo = await ligaIdentidade(discoIsolado(memoriaLS(), 'foo'), { chave: pub })
const identBar = await ligaIdentidade(discoIsolado(memoriaLS(), 'bar'), { chave: pub })
const idHex = await idEstavelDaChave(pub)
ok('§N5 mesma chave → mesmo id nos dois tenants',
  identFoo.id === idHex && identBar.id === idHex && identFoo.id !== 'foo')

ok('§N6 nginx map Host → X-Tenant; nao e location ~',
  /map \$host \$gk_tenant/.test(conf) &&
  /proxy_set_header X-Tenant \$gk_tenant/.test(conf) &&
  /add_header X-Tenant \$gk_tenant/.test(conf) &&
  !/^\s*location\s+~/m.test(conf) &&
  !/^\s*server_name\s+\*\.goldenkingdom/m.test(conf))
ok('§N6 front hospeda iframe oraculo; nao importa main.js',
  /urlHospedadoGk/.test(front) &&
  /ligaNavGk/.test(front) &&
  /dataset\.tenant/.test(front) &&
  !/from ['"]\.\/main\.js['"]/.test(front) &&
  !/from ['"]\.\/banco_vite_u\.js['"]/.test(front))
ok('§N6 ancoraDeHash ignora lixo',
  ancoraDeHash('#coracao') === 'coracao' &&
  ancoraDeHash('#inicio') === 'inicio' &&
  ancoraDeHash('?tenant=foo') === '' &&
  ancoraDeHash('#../x') === '')

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
  ok('§N7 U da nav cabe no schema', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  F1: hash original no hospedeiro; nav(foo) ⊥ nav(bar); iframe oráculo.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
