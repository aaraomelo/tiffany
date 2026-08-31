/* tests/cena_u.js — F7: um host de cena no pulso F6. Não motor_campo inteiro.
 *
 * host conhecido ≠ carregado ≠ registado ≠ avançado pelo tique.
 *   node tests/cena_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { MAGIA, CHAVE_ESTADO, estadoVazio, leEstado, gravaEstado } from '../app/src/banco_disco.js'
import { discoIsolado, chaveIsolada } from '../app/src/banco_tenant_u.js'
import { ligaIdentidade, idEstavelDaChave } from '../app/src/banco_identidade_u.js'
import { CHAVE_CADEIA } from '../app/src/banco_cristalchain_u.js'
import { CAPACIDADES_TARDAS, fatiaDeFonte } from '../app/src/banco_estado_gk_u.js'
import { CARD_PRIMEIRO } from '../app/src/banco_cards_u.js'
import { resetGlsl, fetchesPainelWasm, kernelsCompilados, kernelsExecutados } from '../app/src/banco_glsl_u.js'
import { resetLatex, fetchesTexWasm } from '../app/src/banco_latex_u.js'
import {
  resetRelogio, disparaRelogio, faseRelogio, tiquesRelogio, catalogoRelogio,
} from '../app/src/banco_relogio_u.js'
import {
  ID_CENA_GK, CHAVE_CENA, HOST_CANONICO, SELETOR_HOST, PECA_HOST, FONTE_CENA,
  C200_HERO, ANTIFASE_HERO, DT0, ESCALA_FASE,
  cadeiaProibida, resetCena, faseCena, hostCarregado, hostRegistado, quadrosHost, uTimeHost,
  catalogoCena, igualCatalogoCena, faseAposTique, querCena, leCenaSelecionada,
  igualResultadoCena, disparaCena, cenaParaU, uParaCena,
} from '../app/src/banco_cena_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'cena.json')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const MAIN = join(RAIZ, 'app', 'src', 'main.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_cena_u.js')
const REL_PONTE = join(RAIZ, 'app', 'src', 'banco_relogio_u.js')
const REL = join(RAIZ, 'app', 'src', 'relogio.js')
const CAMPO = join(RAIZ, 'app', 'src', 'motor_campo.js')
const CC = join(RAIZ, 'app', 'src', 'cards_campo.js')
const TR = join(RAIZ, 'app', 'src', 'trailer_campo.js')
const DISCO = join(RAIZ, 'app', 'src', 'banco_disco.js')
const CARDS = join(RAIZ, 'app', 'src', 'banco_cards_u.js')
const LATEX = join(RAIZ, 'app', 'src', 'banco_latex_u.js')
const GLSL = join(RAIZ, 'app', 'src', 'banco_glsl_u.js')
const ESTADO = join(RAIZ, 'app', 'src', 'banco_estado_gk_u.js')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

resetCena()
resetRelogio()
resetGlsl()
resetLatex()

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const inst = JSON.parse(readFileSync(INST, 'utf8'))
const front = readFileSync(FRONT, 'utf8')
const mainJs = readFileSync(MAIN, 'utf8')
const ponte = readFileSync(PONTE, 'utf8')
const relPonte = readFileSync(REL_PONTE, 'utf8')
const rel = readFileSync(REL, 'utf8')
const campo = readFileSync(CAMPO, 'utf8')
const cc = readFileSync(CC, 'utf8')
const tr = readFileSync(TR, 'utf8')
const discoSrc = readFileSync(DISCO, 'utf8')
const cardsSrc = readFileSync(CARDS, 'utf8')
const latexSrc = readFileSync(LATEX, 'utf8')
const glslSrc = readFileSync(GLSL, 'utf8')
const estadoSrc = readFileSync(ESTADO, 'utf8')
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'

const cat = catalogoCena()
const hero = { id: HOST_CANONICO, cena: true }
const pulso = { id: 'pulso', relogio: true }
const rainha = { id: 'rainha_tiffany', nome: CARD_PRIMEIRO.nome, secao: CARD_PRIMEIRO.secao, kernel: 'aura' }
const vel = { v: 2, tocando: true }
const f1 = Number(faseAposTique(0n, DT0, vel)) / ESCALA_FASE
const f2 = Number(faseAposTique(faseAposTique(0n, DT0, vel), DT0, vel)) / ESCALA_FASE

ok('§H0 instancia id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_CENA_GK && inst.camada === 'capacidade' &&
  !schema.properties.kind.enum.includes('cena'))
ok('§H0 ponte_cena no motor',
  man.corpos?.motor?.ponte_cena === 'app/src/banco_cena_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_cena)))
ok('§H0 schema_cena no disco',
  man.corpos?.motor?.schema_cena === 'conecthus/schema/cena.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_cena)))
ok('§H0 nucleo: F7 = F6 + um host; nao motor_campo inteiro',
  /F7 = F6 \+ hero/.test(man.corpos?.motor?.nucleo?.cena_gk || '') &&
  /nao motor_campo inteiro/.test(man.corpos?.motor?.nucleo?.cena_gk || ''))

ok('§H1 descoberta: host hero identificavel; F2/F6 nao reabertos',
  fatiaDeFonte(FONTE_CENA) === 'capacidades' &&
  CAPACIDADES_TARDAS.map((c) => c.id).join(' ') === 'cards latex glsl' &&
  catalogoRelogio().cena === false &&
  !/ciclo: 'F7'/.test(estadoSrc) &&
  /export function iniciaRelogio/.test(rel) &&
  !/banco_cena/.test(rel) &&
  !/banco_cena/.test(relPonte))

ok('§H1 catalogo: um host, sem trailer, sem cards, sem GLSL',
  cat.id === HOST_CANONICO && cat.seletor === SELETOR_HOST && cat.peca === PECA_HOST &&
  cat.c200 === C200_HERO && cat.antifase === ANTIFASE_HERO && cat.n === 1 &&
  cat.trailer === false && cat.cards === false && cat.glsl === false && cat.wasm === false &&
  igualCatalogoCena(cat, catalogoCena()))

ok('§H1 original ≡ motor (seletor, peca, c200, antifase, u_time)',
  mainJs.includes('class="heroart motor"') &&
  mainJs.includes('data-peca="coracao_revela"') &&
  mainJs.includes("querySelector('.heroart')") &&
  mainJs.includes('initMotorCampo(heroart, _assCoracao, _ANTIFASE)') &&
  mainJs.includes('c200: 9 / 200') &&
  mainJs.includes('const _ANTIFASE = Math.PI') &&
  campo.includes('gl.uniform1f(uTime, faseDoMotor())') &&
  campo.includes('registra(render)') &&
  campo.startsWith('//') &&
  campo.includes('#version 300 es') &&
  C200_HERO === 9 / 200 && ANTIFASE_HERO === Math.PI)

ok('§H2 P→U→P id/peca; kind=pagina camada=capacidade',
  cenaParaU(cat).kind === 'pagina' && cenaParaU(cat).id === 'gk' &&
  cenaParaU(cat).camada === 'capacidade' &&
  uParaCena(cenaParaU(cat)).id === HOST_CANONICO &&
  uParaCena(cenaParaU(cat)).peca === PECA_HOST)

ok('§H2 pulso, card e LaTeX nao disparam a cena',
  querCena(hero) && !querCena(pulso) && !querCena(rainha) &&
  !querCena({ id: 'enredo', arquivo: '/docs/enredo.pdf' }))

{
  resetCena()
  resetRelogio()
  resetGlsl()
  resetLatex()
  let nImp = 0
  const opts = { importar: async () => { nImp++; return { initMotorCampo () { return false } } } }

  const d0 = await disparaCena(null, rainha)
  ok('§H3 boot / card => 0 host, 0 GLSL, 0 exec',
    d0.quadros === 0 && d0.wasm === 0 && faseCena() === 'existencia' &&
    !hostCarregado() && nImp === 0 &&
    fetchesPainelWasm() === 0 && kernelsCompilados() === 0 && kernelsExecutados() === 0 &&
    fetchesTexWasm() === 0)

  await disparaRelogio(null, pulso)
  ok('§H3 ?gk=1: relogio conhecido, cena NAO carregada',
    faseRelogio() === 'existencia' && faseCena() === 'existencia' &&
    !hostCarregado() && tiquesRelogio() === 0)

  const d1 = await disparaCena(null, hero)
  ok('§H3 seleccionar cena: host existe, ainda nao carrega',
    d1.id === HOST_CANONICO && !d1.carregado && d1.quadros === 0 &&
    faseCena() === 'existencia' && nImp === 0)

  const d2 = await disparaCena(null, hero, { ...opts, carregar: true })
  ok('§H3 carga depois do paint: host carregado, 0 GLSL, 0 tique',
    d2.carregado === true && !d2.registado && d2.quadros === 0 && nImp === 1 &&
    faseCena() === 'carga' &&
    fetchesPainelWasm() === 0 && kernelsCompilados() === 0)

  const d3 = await disparaCena(null, hero, { ...opts, registar: true })
  ok('§H3 registo no relogio F6 sem executar capacidade',
    d3.registado === true && d3.quadros === 0 && nImp === 1 &&
    faseCena() === 'registro' && hostRegistado() && uTimeHost() === 0)

  const d4 = await disparaCena(null, hero, { ...opts, tique: true, dt: DT0, vel })
  ok('§H3 tick × 1 → fase = f1; u_time ≡ pulso F6',
    d4.quadros === 1 && d4.resultado.uTime === f1 && d4.resultado.uTime === uTimeHost() &&
    d4.resultado.tiques === 1 && d4.resultado.wasm === 0 && d4.resultado.compilou === false &&
    faseCena() === 'avanco' &&
    igualResultadoCena(d4.resultado, {
      uTime: f1, quadros: 1, tiques: 1, wasm: 0, compilou: false, peca: PECA_HOST,
    }))

  const d5 = await disparaCena(null, hero, { ...opts, tique: true, dt: DT0, vel })
  ok('§H3 tick × 1 → fase = f2',
    d5.quadros === 2 && d5.resultado.uTime === f2 && d5.resultado.tiques === 2)

  const d6 = await disparaCena(null, hero, { ...opts, tique: true, dt: DT0, vel: { v: 2, tocando: false } })
  ok('§H3 pause: fase congelada; tique contabilizado (contrato F6)',
    d6.resultado.uTime === f2 && d6.resultado.tiques === 3 && d6.quadros === 3)

  ok('§H3 1 host: 0 GLSL novo, 0 WASM, 0 LaTeX extra',
    fetchesPainelWasm() === 0 && kernelsCompilados() === 0 && kernelsExecutados() === 0 &&
    fetchesTexWasm() === 0)

  ok('§H3 ponte nao importa motor_campo/trailer/cards; F6 e o pulso',
    /from ['"]\.\/banco_relogio_u\.js['"]/.test(ponte) &&
    !/from ['"]\.\/motor_campo\.js['"]/.test(ponte) &&
    !/import\(['"]\.\/motor_campo\.js['"]\)/.test(ponte) &&
    !/from ['"]\.\/cards_campo\.js['"]/.test(ponte) &&
    !/from ['"]\.\/trailer_campo\.js['"]/.test(ponte) &&
    !/from ['"]\.\/motor_wasm\.js['"]/.test(ponte) &&
    !/precision highp float/.test(ponte) &&
    !/painel_motor\.wasm/.test(ponte) &&
    !/initMotorCampo\(/.test(ponte) &&
    !/initTrailerCampo\(/.test(ponte) &&
    !/initCardsCampo\(/.test(ponte) &&
    cadeiaProibida().every((s) => s.startsWith('./')))
}

ok('§H3 original nao pinta #coracao nem trailer neste ciclo',
  cat.n === 1 &&
  !ponte.includes('#coracao') &&
  !ponte.includes('initTrailerCampo') &&
  mainJs.includes('initTrailerCampo(screen)') &&
  cc.includes('export function initCardsCampo'))

{
  resetCena()
  resetRelogio()
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  await disparaCena(foo, { id: 'hero-foo', cena: true }, { registar: true, tique: true, dt: DT0, vel })
  await disparaCena(bar, { id: 'hero-bar', cena: true }, { carregar: true })
  const e = estadoVazio()
  gravaEstado(e, ls)
  ok('§H4 descricao cena(foo) = descricao(bar)', igualCatalogoCena(catalogoCena(), catalogoCena()))
  ok('§H4 execucao(foo) perp (bar); S_ESTADO intocado',
    leCenaSelecionada(foo) === 'hero-foo' &&
    leCenaSelecionada(bar) === 'hero-bar' &&
    quadrosHost(foo) === 1 && quadrosHost(bar) === 0 &&
    uTimeHost(foo) === f1 && uTimeHost(bar) === 0 &&
    ls.getItem(CHAVE_CENA) === null &&
    chaveIsolada(CHAVE_CENA) &&
    chaveIsolada(CHAVE_CADEIA + 'x') &&
    !chaveIsolada(CHAVE_ESTADO) &&
    JSON.parse(ls.getItem(CHAVE_ESTADO)).magia === MAGIA &&
    !JSON.stringify(leEstado(ls)).includes('hero') &&
    fetchesPainelWasm() === 0)
}

const idFoo = await ligaIdentidade(discoIsolado(memoriaLS(), 'foo'), { chave: pub })
const idBar = await ligaIdentidade(discoIsolado(memoriaLS(), 'bar'), { chave: pub })
const idHex = await idEstavelDaChave(pub)
ok('§H4 mesma chave → mesmo id(K) nos dois tenants',
  idFoo.id === idHex && idBar.id === idHex && idFoo.id !== 'foo')

ok('§H5 /banco/ sem cena: front nao importa ponte nem motor_campo',
  !/banco_cena_u/.test(front) &&
  !/from ['"]\.\/motor_campo\.js['"]/.test(front) &&
  !/heroart/.test(front) &&
  !/from ['"]\.\/banco_disco\.js['"]/.test(ponte) &&
  !/gravaEstado/.test(ponte) &&
  !/CHAVE_ESTADO/.test(ponte) &&
  /GKBANCO/.test(discoSrc) &&
  !/from ['"]\.\/banco_front\.js['"]/.test(ponte) &&
  !/banco_cena/.test(cardsSrc) &&
  !/banco_cena/.test(latexSrc) &&
  !/banco_cena/.test(glslSrc) &&
  !/banco_cena/.test(relPonte))

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
  walk(cenaParaU(cat))
  ok('§H6 U da cena cabe no schema', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  F7: F6 + um host; tique → u_time; sem GLSL/WASM/trailer.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
