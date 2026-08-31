/* tests/estado_gk_u.js — F2: classifica o estado do Reino. Não é S_ESTADO.
 *
 * GK = descrição estática + sessão + capacidades. iframe = oráculo.
 *   node tests/estado_gk_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { MAGIA, CHAVE_ESTADO, estadoVazio, leEstado, gravaEstado } from '../app/src/banco_disco.js'
import { discoIsolado, chaveIsolada } from '../app/src/banco_tenant_u.js'
import { CHAVE_NAV, gravaAncora, leAncora } from '../app/src/banco_nav_u.js'
import {
  ID_ESTADO_GK, VEL_OMISSAO, FATIAS, FONTES_FATIA, CAPACIDADES_TARDAS,
  fatiaDeFonte, parseDescricaoGk, parseVelOmissao,
  estadoGkDe, igualEstadoGk, estadoPerp, estadoGkParaU, uParaEstadoGk,
} from '../app/src/banco_estado_gk_u.js'
import { ligaIdentidade, idEstavelDaChave } from '../app/src/banco_identidade_u.js'
import { CHAVE_SESSAO } from '../app/src/banco_identidade_u.js'
import { CHAVE_CADEIA } from '../app/src/banco_cristalchain_u.js'
import { igual } from '../app/src/banco_manifesto_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'estado_gk.json')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const MAIN = join(RAIZ, 'app', 'src', 'main.js')
const VEL = join(RAIZ, 'app', 'src', 'vel_estado.js')
const DISCO = join(RAIZ, 'app', 'src', 'banco_disco.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_estado_gk_u.js')
const MAN_GK = join(RAIZ, 'app', 'src', 'manifesto.json')

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
const front = readFileSync(FRONT, 'utf8')
const mainJs = readFileSync(MAIN, 'utf8')
const velSrc = readFileSync(VEL, 'utf8')
const discoSrc = readFileSync(DISCO, 'utf8')
const ponte = readFileSync(PONTE, 'utf8')
const gkMan = JSON.parse(readFileSync(MAN_GK, 'utf8'))
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'

ok('§E0 instancia id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_ESTADO_GK &&
  !schema.properties.kind.enum.includes('estado'))
ok('§E0 ponte_estado_gk no motor',
  man.corpos?.motor?.ponte_estado_gk === 'app/src/banco_estado_gk_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_estado_gk)))
ok('§E0 schema_estado_gk no disco',
  man.corpos?.motor?.schema_estado_gk === 'conecthus/schema/estado_gk.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_estado_gk)))
ok('§E0 nucleo classifica fatias; S_ESTADO intocado',
  /descricao/.test(man.corpos?.motor?.nucleo?.estado_gk || '') &&
  /S_ESTADO != sessao/.test(man.corpos?.motor?.nucleo?.estado_gk || ''))

ok('§E1 main.js nao persiste localStorage (estado observavel = URL + defaults)',
  !/localStorage/.test(mainJs) && !/sessionStorage/.test(mainJs))
ok('§E1 velEstado omissao casa com o original e nao e GKBANCO',
  igual(parseVelOmissao(velSrc), VEL_OMISSAO) &&
  estadoVazio().magia === MAGIA &&
  estadoVazio().v === VEL_OMISSAO.v &&
  !('magia' in VEL_OMISSAO) &&
  /GKBANCO/.test(discoSrc) &&
  !/gravaEstado/.test(ponte) &&
  !/CHAVE_ESTADO/.test(ponte))

const D = parseDescricaoGk(gkMan)
ok('§E2 descricao = manifesto (titulo, secoes, kernels); nao e sessao',
  D.titulo === gkMan.titulo &&
  D.secoes.length === gkMan.secoes.length &&
  D.secoes[0] === gkMan.secoes[0].id &&
  D.kernels.includes('aura') &&
  D.npecas > 0)
ok('§E2 fatias cobrem os imports de 1o nivel do main',
  FATIAS.join(' ') === 'descricao sessao capacidades')

{
  /* parseImportsPrimeiroNivel vive em vite; aqui classificamos o texto do main. */
  const imports = []
  for (const raw of mainJs.split('\n')) {
    const t = raw.trim()
    if (!t || t.startsWith('//')) continue
    if (!t.startsWith('import ')) break
    const m = t.match(/from\s+['"](\.[^'"]+)['"]/)
    if (m) imports.push(m[1])
  }
  const classificados = imports.filter((s) => fatiaDeFonte(s) !== 'hospedeiro' || FONTES_FATIA.descricao.includes(s))
  ok('§E2 manifesto.json e descricao; tex/glsl sao capacidades',
    fatiaDeFonte('./manifesto.json') === 'descricao' &&
    fatiaDeFonte('./vel_estado.js') === 'sessao' &&
    fatiaDeFonte('./tex_tradutor.js') === 'capacidades' &&
    fatiaDeFonte('./motor_campo.js') === 'capacidades' &&
    fatiaDeFonte('./cards_kernel.js') === 'capacidades' &&
    imports.includes('./manifesto.json') &&
    imports.includes('./tex_tradutor.js') &&
    classificados.length >= 4)
}

const orig = estadoGkDe({ man: gkMan, ancora: 'coracao', tenant: 'foo' })
const mot = estadoGkDe({ man: gkMan, ancora: 'coracao', tenant: 'foo', vel: parseVelOmissao(velSrc) })
ok('§E3 estado original ≡ motor (sessao simples)', igualEstadoGk(orig, mot))
ok('§E3 ancora #coracao; vel default 2× tocando; capacidades tardias listadas',
  orig.sessao.ancora === 'coracao' &&
  orig.sessao.vel.v === 2 && orig.sessao.vel.tocando === true &&
  orig.capacidades.join(' ') === 'cards latex glsl')

const U = estadoGkParaU(orig)
ok('§E4 kind=pagina id=gk, nao U; camada reino',
  U.kind === 'pagina' && U.id === 'gk' && U.star !== 'D' && U.camada === 'reino')
ok('§E4 P→U→P = P', igualEstadoGk(uParaEstadoGk(U), orig))
ok('§E4 U→P→U = U', igual(estadoGkParaU(uParaEstadoGk(U)), U))
ok('§E4 S_ESTADO != sessao != livro no nodo',
  /S_ESTADO != sessao != livro/.test(U.proibicao) &&
  U.slots.descricao === 'realizado' &&
  U.slots.capacidades === 'tardias' &&
  !U.filhos.some((f) => f.id === 'shells'))

ok('§E5 estado(foo) perp estado(bar); descricao igual',
  estadoPerp(estadoGkDe({ man: gkMan, tenant: 'foo', ancora: 'coracao' }),
    estadoGkDe({ man: gkMan, tenant: 'bar', ancora: 'coracao' })) &&
  igualEstadoGk(
    { ...estadoGkDe({ man: gkMan, tenant: 'foo' }), tenant: '' },
    { ...estadoGkDe({ man: gkMan, tenant: 'bar' }), tenant: '' }))

{
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  gravaAncora(foo, 'coracao')
  gravaAncora(bar, 'trailer')
  const e = estadoVazio()
  gravaEstado(e, ls)
  ok('§E5 sessao isolada; S_ESTADO nao leva ancora nem se parte',
    leAncora(foo) === 'coracao' &&
    leAncora(bar) === 'trailer' &&
    !chaveIsolada(CHAVE_ESTADO) &&
    ls.getItem(CHAVE_ESTADO) &&
    JSON.parse(ls.getItem(CHAVE_ESTADO)).magia === MAGIA &&
    !JSON.stringify(leEstado(ls)).includes('coracao') &&
    !JSON.stringify(leEstado(foo)).includes('coracao') &&
    ls.getItem(CHAVE_NAV) === null)
  ok('§E5 chaveIsolada nao mistura livro/sessao-id com S_ESTADO',
    chaveIsolada(CHAVE_SESSAO) &&
    chaveIsolada(CHAVE_NAV) &&
    chaveIsolada(CHAVE_CADEIA + 'x') &&
    !chaveIsolada(CHAVE_ESTADO))
}

const idFoo = await ligaIdentidade(discoIsolado(memoriaLS(), 'foo'), { chave: pub })
const idBar = await ligaIdentidade(discoIsolado(memoriaLS(), 'bar'), { chave: pub })
const idHex = await idEstavelDaChave(pub)
ok('§E6 mesma chave → mesmo id nos dois estados de tenant',
  idFoo.id === idHex && idBar.id === idHex && idFoo.id !== 'foo')

ok('§E7 iframe continua oraculo; front nao funde GK em gravaEstado',
  /urlHospedadoGk/.test(front) &&
  /estadoGkParaU/.test(front) &&
  /estadoGkDe/.test(front) &&
  !/gravaEstado\([^)]*coracao/.test(front) &&
  !/from ['"]\.\/main\.js['"]/.test(front) &&
  !/from ['"]\.\/motor_campo\.js['"]/.test(front) &&
  !/from ['"]\.\/tex_tradutor\.js['"]/.test(front) &&
  !/from ['"]\.\/vel_estado\.js['"]/.test(front))
ok('§E7 ponte nao importa disco/coord/fuse',
  !/banco_disco/.test(ponte) &&
  !/banco_coord/.test(ponte) &&
  !/banco_fuse/.test(ponte) &&
  !/from ['"]\.\/main\.js['"]/.test(ponte))

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
  ok('§E8 U do estado GK cabe no schema', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  F2: GK = descricao + sessao + capacidades; S_ESTADO intocado; iframe oraculo.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
