/* tests/tenant_u.js — F0.5 borda: host → tenant → rota. Nginx realiza; motor cataloga.
 *
 * tenant ≠ id ≠ K_i. Apex = ausência. Wildcard catalogado, não activado sem cert.
 *   node tests/tenant_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { CHAVE_ESTADO } from '../app/src/banco_disco.js'
import {
  ID_BORDA, APEX_HOST, SUFIXO_TENANT, CANAL_HOST, ROTAS_BORDA, CABECALHO_TENANT,
  tenantDeHost, rotaDaPath, prefixoDisco, discoIsolado, chaveIsolada,
  canalDoHost, bordaDe, tenantParaU, uParaTenant, igualBorda, slugOk,
  tenantDeHeaders, cabecalhosTenant,
} from '../app/src/banco_tenant_u.js'
import { paramsDaSessao, sessaoParaU, PATRIA_PUB } from '../app/src/banco_sessao_u.js'
import { CHAVE_SESSAO, ligaIdentidade, idEstavelDaChave } from '../app/src/banco_identidade_u.js'
import { CHAVE_CADEIA, chaveLivro, integraCadeia, leLivro } from '../app/src/banco_cristalchain_u.js'
import { igual } from '../app/src/banco_manifesto_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'tenant.json')
const CONF = join(RAIZ, 'app', 'nginx', 'goldenkingdom.conf')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_tenant_u.js')
const SQL = join(RAIZ, 'banco', 'sql.c')

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
const ponte = readFileSync(PONTE, 'utf8')
const sql = existsSync(SQL) ? readFileSync(SQL, 'utf8') : ''
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'
const HOST_FOO = 'foo' + SUFIXO_TENANT
const HOST_BAR = 'bar' + SUFIXO_TENANT

ok('§T0 instancia borda no schema U (nao kind novo)',
  inst.kind === 'realizacao' && inst.id === ID_BORDA && schema.$id === 'tiffany://u')
ok('§T0 ponte_tenant no motor',
  man.corpos?.motor?.ponte_tenant === 'app/src/banco_tenant_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_tenant)))
ok('§T0 schema_tenant no disco',
  man.corpos?.motor?.schema_tenant === 'conecthus/schema/tenant.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_tenant)))
ok('§T0 tenant e propriedade, nao kind',
  schema.properties.tenant &&
  schema.properties.kind.enum.includes('realizacao') &&
  !schema.properties.kind.enum.includes('tenant'))
ok('§T0 nucleo tenant_borda',
  /tenant != id != K_i/.test(man.corpos?.motor?.nucleo?.tenant_borda || ''))

ok('§T1 apex = ausencia, nao tenant=goldenkingdom',
  tenantDeHost(APEX_HOST).tenant === '' &&
  tenantDeHost(APEX_HOST).estatuto === 'ausencia')
ok('§T1 localhost = ausencia',
  tenantDeHost('localhost').estatuto === 'ausencia' &&
  tenantDeHost('127.0.0.1:5190').estatuto === 'ausencia')
ok('§T1 foo.goldenkingdom → tenant=foo',
  tenantDeHost(HOST_FOO).tenant === 'foo' &&
  tenantDeHost(HOST_FOO).estatuto === 'realizado')
ok('§T1 canal.patriatechnology.com N/A como tenant GK',
  tenantDeHost(CANAL_HOST).estatuto === 'N/A' &&
  tenantDeHost(CANAL_HOST).tenant === '')
ok('§T1 wildcard desconhecido',
  tenantDeHost('nope.example.com').estatuto === 'nao localizada' &&
  tenantDeHost('www' + SUFIXO_TENANT).estatuto === 'nao localizada' &&
  tenantDeHost('a.b' + SUFIXO_TENANT).estatuto === 'nao localizada' &&
  !slugOk('canal') && !slugOk('goldenkingdom'))

ok('§T2 rotas / /banco/ /canal',
  ROTAS_BORDA.join(' ') === '/ /banco/ /canal' &&
  rotaDaPath('/') === '/' &&
  rotaDaPath('/banco/?gk=1') === '/banco/' &&
  rotaDaPath('/canal') === '/canal')
ok('§T2 tenant → rota (foo → /banco/)',
  bordaDe(HOST_FOO, '/banco/').tenant === 'foo' &&
  bordaDe(HOST_FOO, '/banco/').rota === '/banco/')
ok('§T2 nginx location = /canal e location /; wildcard nao activado',
  /location\s+=\s+\/canal/.test(conf) &&
  /location \/ \{/.test(conf) &&
  /server_name goldenkingdom\.patriatechnology\.com/.test(conf) &&
  !/^\s*server_name\s+\*\.goldenkingdom/m.test(conf) &&
  /F0\.5/.test(conf) &&
  /proxy_set_header Host \$host/.test(conf))

const B = bordaDe(HOST_FOO, '/banco/')
const U = tenantParaU(B)
ok('§T3 kind=realizacao id=borda, nao U',
  U.kind === 'realizacao' && U.id === 'borda' && U.star !== 'D')
ok('§T3 P→U→P = P', igualBorda(uParaTenant(U), B))
ok('§T3 U→P→U = U', igual(tenantParaU(uParaTenant(U)), U))
ok('§T3 apex P→U→P ausencia',
  igualBorda(uParaTenant(tenantParaU(bordaDe(APEX_HOST, '/'))), bordaDe(APEX_HOST, '/')))
ok('§T3 tenant != id != K_i no nodo',
  U.id === 'borda' && U.tenant === 'foo' && U.chave == null)

ok('§T4 ?tenant= so em local; host ganha',
  paramsDaSessao('?tenant=foo').tenant === 'foo' &&
  paramsDaSessao('?tenant=foo', APEX_HOST).tenant === '' &&
  paramsDaSessao('?tenant=bar', HOST_FOO).tenant === 'foo')
ok('§T4 tenant + ?patria=1 coexistem',
  paramsDaSessao('?patria=1', HOST_FOO).patria === true &&
  paramsDaSessao('?patria=1', HOST_FOO).chave === PATRIA_PUB &&
  paramsDaSessao('?patria=1', HOST_FOO).tenant === 'foo')
ok('§T4 sessaoParaU nao absorve tenant (sessao != borda)',
  sessaoParaU(paramsDaSessao('?tenant=foo&pub=' + pub)).tenant == null)
ok('§T4 X-Tenant deriva do Host; spoof no apex ignorado',
  tenantDeHeaders({ [CABECALHO_TENANT]: 'foo' }, APEX_HOST).tenant === '' &&
  tenantDeHeaders({ [CABECALHO_TENANT]: 'foo' }, HOST_FOO).tenant === 'foo' &&
  tenantDeHeaders({ [CABECALHO_TENANT]: 'bar' }, HOST_FOO).tenant === 'foo' &&
  paramsDaSessao('', 'localhost', { [CABECALHO_TENANT]: 'foo' }).tenant === 'foo')
ok('§T4 cabecalho nao substitui chave',
  cabecalhosTenant('foo')[CABECALHO_TENANT] === 'foo' &&
  cabecalhosTenant('')[CABECALHO_TENANT] == null)

ok('§T5 WSS same-host persiste tenant',
  canalDoHost(HOST_FOO, true) === 'wss://' + HOST_FOO + '/canal' &&
  tenantDeHost(new URL(canalDoHost(HOST_FOO, true)).host).tenant === 'foo')
ok('§T5 alias canal N/A — WSS pelo alias nao e tenant GK',
  tenantDeHost(CANAL_HOST).estatuto === 'N/A' &&
  canalDoHost(CANAL_HOST, true).includes(CANAL_HOST))

ok('§T6 prefixo vazio no apex; isolado com slug',
  prefixoDisco('') === '' &&
  prefixoDisco('foo') === 'gk:t:foo:' &&
  chaveIsolada(CHAVE_SESSAO) &&
  chaveIsolada('gk:banco:nav') &&
  chaveIsolada(CHAVE_CADEIA + 'x') &&
  !chaveIsolada(CHAVE_ESTADO))

{
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  foo.setItem(CHAVE_SESSAO, 'A')
  bar.setItem(CHAVE_SESSAO, 'B')
  foo.setItem(CHAVE_ESTADO, 'E')
  ok('§T6 dois tenants nao partilham sessao',
    foo.getItem(CHAVE_SESSAO) === 'A' &&
    bar.getItem(CHAVE_SESSAO) === 'B' &&
    ls.getItem(CHAVE_SESSAO) === null &&
    ls.getItem(prefixoDisco('foo') + CHAVE_SESSAO) === 'A')
  ok('§T6 S_ESTADO nao se parte em F0.5 (F2)',
    foo.getItem(CHAVE_ESTADO) === 'E' &&
    bar.getItem(CHAVE_ESTADO) === 'E' &&
    ls.getItem(CHAVE_ESTADO) === 'E')
}

const lsAB = memoriaLS()
const foo = discoIsolado(lsAB, 'foo')
const bar = discoIsolado(lsAB, 'bar')
const identFoo = await ligaIdentidade(foo, { chave: pub })
const identBar = await ligaIdentidade(bar, { chave: pub })
const idHex = await idEstavelDaChave(pub)
await integraCadeia(identFoo, foo)
await integraCadeia(identBar, bar)
ok('§T7 mesma pub → mesmo id criptografico (tenant != id)',
  identFoo.id === idHex && identBar.id === idHex && identFoo.id !== 'foo')
foo.setItem(chaveLivro(identFoo.id), 'FOO-ONLY')
ok('§T7 livros isolados; chave crua vazia',
  lsAB.getItem(chaveLivro(identFoo.id)) === null &&
  lsAB.getItem(prefixoDisco('foo') + chaveLivro(identFoo.id)) === 'FOO-ONLY' &&
  leLivro(bar, identBar.id).registos.length === 5 &&
  bar.getItem(chaveLivro(identBar.id)) !== 'FOO-ONLY')
ok('§T7 tenant != K_i', identFoo.chave === pub && identFoo.chave !== 'foo')

ok('§T8 ERP tenantId != tenant DNS',
  /tenantId/.test(sql) &&
  /ERP tenantId != tenant DNS/.test(ponte) &&
  !/CREATE INDEX/.test(ponte))
ok('§T8 front usa discoIsolado; nao importa nginx nem main.js',
  /discoIsolado/.test(front) &&
  /banco_tenant_u\.js/.test(front) &&
  !/from ['"]\.\/main\.js['"]/.test(front) &&
  !/goldenkingdom\.conf/.test(front))
ok('§T8 Nginx != motor (ponte nao parseia conf)',
  !/goldenkingdom\.conf/.test(ponte) &&
  !/readFileSync/.test(ponte))

console.log('')
if (!falhas) {
  console.log('  tenant != id != K_i; Nginx realiza, motor cataloga; apex = ausencia.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
