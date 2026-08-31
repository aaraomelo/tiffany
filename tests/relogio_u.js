/* tests/relogio_u.js — F6: contrato do pulso. Não cena, não GLSL, não o ficheiro.
 *
 * relógio conhecido ≠ módulo ≠ consumidor ≠ tique.
 *   node tests/relogio_u.js
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
  ID_RELOGIO_GK, CHAVE_RELOGIO, FONTE_RELOGIO, FONTE_AVANCA, ESCALA_FASE, VEL_BASE, DT0,
  cadeiaProibida, resetRelogio, faseRelogio, moduloRelogioCarregado, tiquesRelogio,
  consumidoresRelogio, chamadasConsumidor, faseDoPulso, catalogoRelogio, igualCatalogoRelogio,
  faseAposTique, querRelogio, leRelogioSelecionado, igualResultadoRelogio,
  disparaRelogio, relogioParaU, uParaRelogio,
} from '../app/src/banco_relogio_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'relogio.json')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const MAIN = join(RAIZ, 'app', 'src', 'main.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_relogio_u.js')
const REL = join(RAIZ, 'app', 'src', 'relogio.js')
const WASM_JS = join(RAIZ, 'app', 'src', 'motor_wasm.js')
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

resetRelogio()
resetGlsl()
resetLatex()

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const inst = JSON.parse(readFileSync(INST, 'utf8'))
const front = readFileSync(FRONT, 'utf8')
const mainJs = readFileSync(MAIN, 'utf8')
const ponte = readFileSync(PONTE, 'utf8')
const rel = readFileSync(REL, 'utf8')
const wasmJs = readFileSync(WASM_JS, 'utf8')
const campo = readFileSync(CAMPO, 'utf8')
const cc = readFileSync(CC, 'utf8')
const tr = readFileSync(TR, 'utf8')
const discoSrc = readFileSync(DISCO, 'utf8')
const cardsSrc = readFileSync(CARDS, 'utf8')
const latexSrc = readFileSync(LATEX, 'utf8')
const glslSrc = readFileSync(GLSL, 'utf8')
const estadoSrc = readFileSync(ESTADO, 'utf8')
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'

const cat = catalogoRelogio()
const pulso = { id: 'pulso', relogio: true }
const rainha = { id: 'rainha_tiffany', nome: CARD_PRIMEIRO.nome, secao: CARD_PRIMEIRO.secao, kernel: 'aura' }
const docLatex = { id: 'enredo', arquivo: '/docs/enredo.pdf', fonte: 'enredo.tex', compoe: true }

ok('§R0 instancia id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_RELOGIO_GK && inst.camada === 'capacidade' &&
  !schema.properties.kind.enum.includes('relogio'))
ok('§R0 ponte_relogio no motor',
  man.corpos?.motor?.ponte_relogio === 'app/src/banco_relogio_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_relogio)))
ok('§R0 schema_relogio no disco',
  man.corpos?.motor?.schema_relogio === 'conecthus/schema/relogio.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_relogio)))
ok('§R0 nucleo: quatro fases; F6 nao executa cena',
  /relogio conhecido != modulo != consumidor != tique/.test(man.corpos?.motor?.nucleo?.relogio_gk || '') &&
  /F6 nao executa cena card ou GLSL/.test(man.corpos?.motor?.nucleo?.relogio_gk || ''))

ok('§R1 descoberta: relogio.js existe e e identificavel; F2 nao reaberto',
  fatiaDeFonte(FONTE_RELOGIO) === 'hospedeiro' &&
  /from ['"]\.\/relogio\.js['"]/.test(mainJs) &&
  /export function iniciaRelogio/.test(rel) &&
  /export function registra/.test(rel) &&
  CAPACIDADES_TARDAS.map((c) => c.id).join(' ') === 'cards latex glsl' &&
  !/ciclo: 'F6'/.test(estadoSrc))

ok('§R1 catalogo: um rAF, wrap 2^20, sem cena',
  cat.fonte === FONTE_RELOGIO && cat.avanca === FONTE_AVANCA &&
  cat.escala === ESCALA_FASE && cat.velBase === VEL_BASE && cat.dt0 === DT0 &&
  cat.fontesRaf === 1 && cat.cena === false && cat.glsl === false &&
  igualCatalogoRelogio(cat, catalogoRelogio()))

ok('§R1 original: uma fonte de avanco; campo/cards/trailer nao tem rAF',
  (rel.match(/requestAnimationFrame/g) || []).length === 2 &&
  rel.includes('avancaFase(dt)') &&
  rel.indexOf('avancaFase(dt)') < rel.indexOf('consumidores[i]') &&
  !/requestAnimationFrame/.test(campo) &&
  !/requestAnimationFrame/.test(cc) &&
  !/requestAnimationFrame/.test(tr) &&
  /from ['"]\.\/relogio\.js['"]/.test(cc) &&
  /from ['"]\.\/relogio\.js['"]/.test(tr))

{
  const em = wasmJs.match(/const ESCALA = 1 << ([0-9]+)/)
  const vb = wasmJs.match(/const VEL_BASE = ([0-9.]+)/)
  ok('§R1 avancaFase original ≡ motor (escala, velBase, wrap AND)',
    em && Number(em[1]) === 20 && ESCALA_FASE === (1 << 20) &&
    vb && Number(vb[1]) === VEL_BASE &&
    wasmJs.includes('& MASK') &&
    wasmJs.includes('export function avancaFase') &&
    wasmJs.includes('if (!velEstado.tocando) return'))
}

ok('§R2 P→U→P escala/fonte; kind=pagina camada=capacidade',
  relogioParaU(cat).kind === 'pagina' && relogioParaU(cat).id === 'gk' &&
  relogioParaU(cat).camada === 'capacidade' &&
  uParaRelogio(relogioParaU(cat)).escala === ESCALA_FASE &&
  uParaRelogio(relogioParaU(cat)).fonte === FONTE_RELOGIO)

ok('§R2 card e LaTeX nao disparam o pulso',
  querRelogio(pulso) && !querRelogio(rainha) && !querRelogio(docLatex))

{
  let nImp = 0
  let nCons = 0
  const opts = {
    importar: async () => { nImp++; return { registra () {}, iniciaRelogio () {} } },
    aoConsumir: () => { nCons++ },
  }
  resetRelogio()
  resetGlsl()
  resetLatex()

  const d0 = await disparaRelogio(null, rainha)
  ok('§R3 ?gk=1 / card sem pulso => 0 modulo, 0 tique, 0 GLSL, 0 LaTeX',
    d0.tiques === 0 && d0.wasm === 0 &&
    faseRelogio() === 'existencia' && nImp === 0 && nCons === 0 &&
    fetchesPainelWasm() === 0 && kernelsCompilados() === 0 && kernelsExecutados() === 0 &&
    fetchesTexWasm() === 0)

  const d1 = await disparaRelogio(null, pulso)
  ok('§R3 seleccionar pulso nao carrega modulo nem tique',
    d1.id === 'pulso' && d1.tiques === 0 && !d1.modulo &&
    faseRelogio() === 'existencia' && nImp === 0)

  const d2 = await disparaRelogio(null, pulso, { ...opts, carregar: true })
  ok('§R3 carga = import() relogio; sem rAF, sem tique, sem GLSL',
    d2.modulo === true && d2.tiques === 0 && nImp === 1 && nCons === 0 &&
    faseRelogio() === 'carga' && moduloRelogioCarregado() &&
    fetchesPainelWasm() === 0 && fetchesTexWasm() === 0 &&
    !/requestAnimationFrame/.test(ponte))

  const d3 = await disparaRelogio(null, pulso, { ...opts, registar: true })
  ok('§R3 registo: consumidor sem executar capacidade',
    d3.consumidores === 1 && d3.tiques === 0 && nCons === 0 && nImp === 1 &&
    faseRelogio() === 'registro' && consumidoresRelogio() === 1)

  const esperada = Number(faseAposTique(0n, DT0, { v: 2, tocando: true })) / ESCALA_FASE
  const d4 = await disparaRelogio(null, pulso, { ...opts, tique: true, dt: DT0, vel: { v: 2, tocando: true } })
  ok('§R3 tique: um avanco = um evento observavel',
    d4.tiques === 1 && nCons === 1 && chamadasConsumidor() === 1 &&
    faseRelogio() === 'tique' &&
    d4.resultado && d4.resultado.tiques === 1 && d4.resultado.wasm === 0 &&
    d4.resultado.cena === false && d4.resultado.glsl === false &&
    d4.resultado.fase === esperada &&
    igualResultadoRelogio(d4.resultado, {
      fase: esperada, tiques: 1, escala: ESCALA_FASE, wasm: 0, cena: false, glsl: false,
    }))

  const d5 = await disparaRelogio(null, pulso, { ...opts, tique: true, dt: DT0, vel: { v: 2, tocando: true } })
  ok('§R3 segundo tique: fase avanca como o original; consumidor +1',
    d5.tiques === 2 && nCons === 2 &&
    d5.resultado.fase === faseDoPulso() &&
    d5.resultado.fase === Number(faseAposTique(faseAposTique(0n, DT0, { v: 2, tocando: true }), DT0, { v: 2, tocando: true })) / ESCALA_FASE)

  ok('§R3 1 tique: nenhum GLSL, nenhum WASM de campo, nenhum LaTeX extra',
    fetchesPainelWasm() === 0 && kernelsCompilados() === 0 && kernelsExecutados() === 0 &&
    fetchesTexWasm() === 0 &&
    d4.resultado.wasm === 0)

  ok('§R3 ponte nao importa a cadeia de cena; import() relogio so na carga',
    !/from ['"]\.\/relogio\.js['"]/.test(ponte) &&
    ponte.includes("import('./relogio.js')") &&
    !/from ['"]\.\/motor_wasm\.js['"]/.test(ponte) &&
    !/from ['"]\.\/motor_campo\.js['"]/.test(ponte) &&
    !/from ['"]\.\/cards_campo\.js['"]/.test(ponte) &&
    !/from ['"]\.\/trailer_campo\.js['"]/.test(ponte) &&
    !/from ['"]\.\/cards_kernel\.js['"]/.test(ponte) &&
    !/precision highp float/.test(ponte) &&
    !/painel_motor\.wasm/.test(ponte) &&
    !/tex\.wasm/.test(ponte) &&
    cadeiaProibida().every((s) => s.startsWith('./')))
}

{
  resetRelogio()
  const parado = { v: 2, tocando: false }
  const r = await disparaRelogio(null, pulso, { tique: true, dt: DT0, vel: parado })
  ok('§R3 pause: avancaFase nao muda a fase; tique mesmo assim conta',
    r.resultado.fase === 0 && r.tiques === 1)
}

ok('§R3 carga real: relogio.js entra sem iniciar rAF',
  (await (async () => {
    resetRelogio()
    const d = await disparaRelogio(null, pulso, { carregar: true })
    return d.modulo && tiquesRelogio() === 0 && faseRelogio() === 'carga'
  })()))

{
  resetRelogio()
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  await disparaRelogio(foo, { id: 'pulso-foo', relogio: true }, { tique: true, dt: DT0 })
  await disparaRelogio(bar, { id: 'pulso-bar', relogio: true })
  const e = estadoVazio()
  gravaEstado(e, ls)
  ok('§R4 descricao relogio(foo) = descricao(bar)', igualCatalogoRelogio(catalogoRelogio(), catalogoRelogio()))
  ok('§R4 execucao(foo) perp (bar); S_ESTADO intocado',
    leRelogioSelecionado(foo) === 'pulso-foo' &&
    leRelogioSelecionado(bar) === 'pulso-bar' &&
    tiquesRelogio(foo) === 1 && tiquesRelogio(bar) === 0 &&
    faseDoPulso(foo) !== 0 && faseDoPulso(bar) === 0 &&
    ls.getItem(CHAVE_RELOGIO) === null &&
    chaveIsolada(CHAVE_RELOGIO) &&
    chaveIsolada(CHAVE_CADEIA + 'x') &&
    !chaveIsolada(CHAVE_ESTADO) &&
    JSON.parse(ls.getItem(CHAVE_ESTADO)).magia === MAGIA &&
    !JSON.stringify(leEstado(ls)).includes('pulso') &&
    fetchesPainelWasm() === 0)
}

const idFoo = await ligaIdentidade(discoIsolado(memoriaLS(), 'foo'), { chave: pub })
const idBar = await ligaIdentidade(discoIsolado(memoriaLS(), 'bar'), { chave: pub })
const idHex = await idEstavelDaChave(pub)
ok('§R4 mesma chave → mesmo id(K) nos dois tenants',
  idFoo.id === idHex && idBar.id === idHex && idFoo.id !== 'foo')

ok('§R5 /banco/ sem relogio do GK: front nao importa ponte nem relogio.js',
  !/banco_relogio_u/.test(front) &&
  !/from ['"]\.\/relogio\.js['"]/.test(front) &&
  !/iniciaRelogio/.test(front) &&
  !/avancaFase/.test(front) &&
  !/from ['"]\.\/banco_disco\.js['"]/.test(ponte) &&
  !/gravaEstado/.test(ponte) &&
  !/CHAVE_ESTADO/.test(ponte) &&
  /GKBANCO/.test(discoSrc) &&
  !/from ['"]\.\/banco_front\.js['"]/.test(ponte) &&
  !/banco_relogio/.test(cardsSrc) &&
  !/banco_relogio/.test(latexSrc) &&
  !/banco_relogio/.test(glslSrc) &&
  !/initMotorCampo/.test(ponte) &&
  !/initCardsCampo/.test(ponte) &&
  !/initTrailerCampo/.test(ponte))

{
  const extra = []
  function walk (n) {
    for (const k of Object.keys(n || {})) {
      if (!(k in schema.properties)) extra.push((n.kind || '?') + k)
    }
    if (n?.faces?.menos) walk(n.faces.menos)
    if (n?.faces?.mais) walk(n.faces.mais)
    for (const f of n?.filhos || []) walk(f)
  }
  walk(relogioParaU(cat))
  ok('§R6 U do relogio cabe no schema', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  F6: relogio conhecido != modulo != consumidor != tique; sem cena/GLSL.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
