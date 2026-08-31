/* tests/glsl_u.js — F5: GLSL/WASM sob demanda. Não puxa a cadeia para o boot.
 *
 * GLSL catalogado ≠ WASM carregado ≠ kernel compilado ≠ kernel executado.
 * Card dispara. foo ⊥ bar. ?gk=1 sem uso ⇒ 0 fetches, 0 compiles.
 *   node tests/glsl_u.js
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
import { CARD_PRIMEIRO, acaoDoCard, indicePecasArt } from '../app/src/banco_cards_u.js'
import {
  ID_GLSL_GK, CHAVE_GLSL, FONTE_GLSL, FONTE_WASM, WASM_PAINEL, KERNEL_PRIMEIRO,
  VS_CANONICO, ESCALA_FASE, cadeiaProibida,
  resetGlsl, faseGlsl, fetchesPainelWasm, kernelsCompilados, kernelsExecutados,
  catalogoGlsl, kernelsEmitidos, igualCatalogoGlsl, querGlsl,
  leGlslSelecionado, resultadoGlsl, igualResultadoGlsl,
  disparaGlsl, glslParaU, uParaGlsl,
} from '../app/src/banco_glsl_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'glsl.json')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const MAIN = join(RAIZ, 'app', 'src', 'main.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_glsl_u.js')
const WASM_JS = join(RAIZ, 'app', 'src', 'motor_wasm.js')
const CAMPO = join(RAIZ, 'app', 'src', 'motor_campo.js')
const CK = join(RAIZ, 'app', 'src', 'cards_kernel.js')
const CC = join(RAIZ, 'app', 'src', 'cards_campo.js')
const DISCO = join(RAIZ, 'app', 'src', 'banco_disco.js')
const CARDS = join(RAIZ, 'app', 'src', 'banco_cards_u.js')
const LATEX = join(RAIZ, 'app', 'src', 'banco_latex_u.js')
const MAN_GK = join(RAIZ, 'app', 'src', 'manifesto.json')
const ART = join(RAIZ, 'app', 'src', 'kernels_campo.json')
const WASM = join(RAIZ, 'assets', 'figuras', 'wasm', 'painel_motor.wasm')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function indiceDoArtefato (src) {
  const i = String(src).lastIndexOf('"pecas":')
  if (i < 0) return {}
  return indicePecasArt(JSON.parse('{' + src.slice(i)))
}

resetGlsl()

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const inst = JSON.parse(readFileSync(INST, 'utf8'))
const front = readFileSync(FRONT, 'utf8')
const mainJs = readFileSync(MAIN, 'utf8')
const ponte = readFileSync(PONTE, 'utf8')
const wasmJs = readFileSync(WASM_JS, 'utf8')
const campo = readFileSync(CAMPO, 'utf8')
const ck = readFileSync(CK, 'utf8')
const cc = readFileSync(CC, 'utf8')
const discoSrc = readFileSync(DISCO, 'utf8')
const cardsSrc = readFileSync(CARDS, 'utf8')
const latexSrc = readFileSync(LATEX, 'utf8')
const gkMan = JSON.parse(readFileSync(MAN_GK, 'utf8'))
const indice = indiceDoArtefato(readFileSync(ART, 'utf8'))
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'

const cat = catalogoGlsl(gkMan.kernels, indice)
const aura = cat.find((k) => k.nome === KERNEL_PRIMEIRO)
const rainha = {
  id: 'rainha_tiffany',
  nome: CARD_PRIMEIRO.nome,
  secao: CARD_PRIMEIRO.secao,
  kernel: KERNEL_PRIMEIRO,
  refs: { glsl: true, wasm: false, latex: false },
}
const docLatex = { id: 'enredo', arquivo: '/docs/enredo.pdf', fonte: 'enredo.tex', compoe: true }

ok('§G0 instancia id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_GLSL_GK && inst.camada === 'capacidade' &&
  !schema.properties.kind.enum.includes('glsl'))
ok('§G0 ponte_glsl no motor',
  man.corpos?.motor?.ponte_glsl === 'app/src/banco_glsl_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_glsl)))
ok('§G0 schema_glsl no disco',
  man.corpos?.motor?.schema_glsl === 'conecthus/schema/glsl.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_glsl)))
ok('§G0 nucleo: quatro fases; cadeia fora do boot',
  /GLSL catalogado != WASM carregado != kernel compilado != kernel executado/.test(man.corpos?.motor?.nucleo?.glsl_gk || '') &&
  /substratos\/motor_campo\/kernels fora/.test(man.corpos?.motor?.nucleo?.glsl_gk || ''))

ok('§G1 descoberta: motor_campo/motor_wasm continuam capacidade tardia F5',
  fatiaDeFonte(FONTE_GLSL) === 'capacidades' &&
  fatiaDeFonte(FONTE_WASM) === 'capacidades' &&
  CAPACIDADES_TARDAS.some((c) => c.id === 'glsl' && c.ciclo === 'F5' && c.fonte === FONTE_GLSL) &&
  /from ['"]\.\/motor_campo\.js['"]/.test(mainJs) &&
  /from ['"]\.\/motor_wasm\.js['"]/.test(mainJs) &&
  !/from ['"]\.\/motor_campo\.js['"]/.test(front) &&
  !/from ['"]\.\/motor_wasm\.js['"]/.test(front))

ok('§G1 catalogo kernels ≡ original (chips do hero, sem GLSL)',
  cat.length === gkMan.kernels.length &&
  aura && aura.emitido && aura.nome === 'aura' &&
  !JSON.stringify(cat).includes('#version') &&
  !JSON.stringify(indice).includes('#version') &&
  kernelsEmitidos(indice).includes('aura') &&
  kernelsEmitidos(indice).includes('venom_rev'))

{
  const orig = gkMan.kernels.map((k) => ({
    nome: k.nome,
    emitido: kernelsEmitidos(indice).includes(k.nome),
  }))
  ok('§G1 GLSL original ≡ motor (nome + emitido)',
    igualCatalogoGlsl(cat.map((k) => ({ nome: k.nome, emitido: k.emitido })), orig) &&
    orig.every((k) => k.emitido))
}

ok('§G2 VS canónico e escala do relógio ≡ original',
  VS_CANONICO === '#version 300 es\nin vec2 pos; void main(){ gl_Position = vec4(pos, 0.0, 1.0); }' &&
  campo.includes(VS_CANONICO.split('\n')[0]) &&
  ck.includes('in vec2 pos') && cc.includes('in vec2 pos') &&
  wasmJs.includes('1 << 20') &&
  ESCALA_FASE === (1 << 20))

{
  const a = resultadoGlsl({ kernel: 'aura', compilou: true, executou: false })
  const b = resultadoGlsl({ kernel: 'aura', compilou: true, executou: false })
  ok('§G2 resultado original ≡ motor (aura, VS 300 es, escala 2^20)',
    a.vs && a.escala === 1048576 && igualResultadoGlsl(a, b) && a.executou === false)
}

ok('§G2 P→U→P n/emitido; kind=pagina camada=capacidade',
  glslParaU(cat).kind === 'pagina' && glslParaU(cat).id === 'gk' &&
  glslParaU(cat).camada === 'capacidade' &&
  uParaGlsl(glslParaU(cat)).n === cat.length &&
  uParaGlsl(glslParaU(cat)).emitido === cat.filter((k) => k.emitido).length)

ok('§G2 card rainha dispara GLSL; doc LaTeX nao',
  querGlsl(rainha) &&
  acaoDoCard(rainha).chama.includes('glsl') &&
  !querGlsl(docLatex) &&
  rainha.kernel === KERNEL_PRIMEIRO)

{
  let nFetch = 0
  let nComp = 0
  let nRun = 0
  const opts = {
    importar: async () => ({ initMotorWasm: async () => true }),
    fetchWasm: async () => { nFetch++; return new Uint8Array([0, 0x61, 0x73, 0x6d]) },
    aoCompilar: async (n) => { nComp++; return { kernel: n, ok: true } },
    aoExecutar: async () => { nRun++ },
  }
  resetGlsl()
  catalogoGlsl(gkMan.kernels, indice)
  const d0 = await disparaGlsl(null, docLatex)
  ok('§G3 ?gk=1 sem usar GLSL => 0 fetches, 0 compiles, 0 exec',
    d0.wasm === 0 && d0.compilou === 0 && d0.executou === 0 &&
    fetchesPainelWasm() === 0 && kernelsCompilados() === 0 && kernelsExecutados() === 0 &&
    faseGlsl() === 'existencia' && nFetch === 0 && nComp === 0 && nRun === 0)

  const d1 = await disparaGlsl(null, rainha)
  ok('§G3 seleccionar card nao carrega WASM nem compila',
    d1.id === 'rainha_tiffany' && d1.wasm === 0 && d1.compilou === 0 && d1.executou === 0 &&
    faseGlsl() === 'existencia' && nFetch === 0)

  const d2 = await disparaGlsl(null, rainha, { ...opts, carregar: true })
  ok('§G3 WASM carregado != kernel compilado',
    d2.wasm === 1 && d2.compilou === 0 && d2.executou === 0 &&
    nFetch === 1 && nComp === 0 && nRun === 0 && faseGlsl() === 'carga')

  const d3 = await disparaGlsl(null, rainha, { ...opts, compilar: true })
  ok('§G3 kernel compilado != kernel executado',
    d3.wasm === 1 && d3.compilou === 1 && d3.executou === 0 &&
    nFetch === 1 && nComp === 1 && nRun === 0 && faseGlsl() === 'compilacao')

  const d4 = await disparaGlsl(null, rainha, { ...opts, executar: true })
  ok('§G3 execucao: kernel corre so depois de compilado',
    d4.executou === 1 && nRun === 1 && nComp === 1 && nFetch === 1 &&
    faseGlsl() === 'execucao' &&
    d4.resultado && d4.resultado.kernel === 'aura' && d4.resultado.compilou && d4.resultado.executou)

  ok('§G3 painel_motor.wasm so dentro de initMotorWasm; ponte nao importa a cadeia',
    wasmJs.indexOf('export async function initMotorWasm') < wasmJs.indexOf("fetch('/wasm/painel_motor.wasm')") &&
    WASM_PAINEL === '/wasm/painel_motor.wasm' &&
    !/from ['"]\.\/motor_wasm\.js['"]/.test(ponte) &&
    !/from ['"]\.\/motor_campo\.js['"]/.test(ponte) &&
    !/from ['"]\.\/cards_kernel\.js['"]/.test(ponte) &&
    !/from ['"]\.\/substratos\.js['"]/.test(ponte) &&
    !/from ['"]\.\/kernels_campo/.test(ponte) &&
    !/precision highp float/.test(ponte) &&
    ponte.includes("import('./motor_wasm.js')") &&
    cadeiaProibida().every((s) => s.startsWith('./')))
}

ok('§G3 wasm no disco e o mesmo artefacto (painel, nao fetch no paint)',
  existsSync(WASM) &&
  /initMotorWasm/.test(wasmJs) &&
  ck.includes("art.kernels[kernel]") &&
  ponte.includes('compile e pelo nome'))

{
  resetGlsl()
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  const catFoo = catalogoGlsl(gkMan.kernels, indice)
  const catBar = catalogoGlsl(gkMan.kernels, indice)
  await disparaGlsl(foo, rainha)
  await disparaGlsl(bar, { id: 'rei_cifra', kernel: 'aura', refs: { glsl: true } })
  const e = estadoVazio()
  gravaEstado(e, ls)
  ok('§G4 descricao glsl(foo) = descricao(bar)', igualCatalogoGlsl(catFoo, catBar))
  ok('§G4 execucao(foo) perp (bar); S_ESTADO intocado',
    leGlslSelecionado(foo) === 'rainha_tiffany' &&
    leGlslSelecionado(bar) === 'rei_cifra' &&
    leGlslSelecionado(foo) !== leGlslSelecionado(bar) &&
    ls.getItem(CHAVE_GLSL) === null &&
    chaveIsolada(CHAVE_GLSL) &&
    chaveIsolada(CHAVE_CADEIA + 'x') &&
    !chaveIsolada(CHAVE_ESTADO) &&
    JSON.parse(ls.getItem(CHAVE_ESTADO)).magia === MAGIA &&
    !JSON.stringify(leEstado(ls)).includes('rainha') &&
    fetchesPainelWasm() === 0 && kernelsCompilados() === 0)
}

const idFoo = await ligaIdentidade(discoIsolado(memoriaLS(), 'foo'), { chave: pub })
const idBar = await ligaIdentidade(discoIsolado(memoriaLS(), 'bar'), { chave: pub })
const idHex = await idEstavelDaChave(pub)
ok('§G4 mesma chave → mesmo id(K) nos dois tenants',
  idFoo.id === idHex && idBar.id === idHex && idFoo.id !== 'foo')

ok('§G5 banco_front nao toca GLSL: 0 cadeia, 0 ponte F5',
  !/painel_motor\.wasm/.test(front) &&
  !/motor_campo/.test(front) &&
  !/motor_wasm/.test(front) &&
  !/cards_kernel/.test(front) &&
  !/substratos/.test(front) &&
  !/kernels_campo/.test(front) &&
  !/banco_glsl_u/.test(front) &&
  !/from ['"]\.\/banco_disco\.js['"]/.test(ponte) &&
  !/gravaEstado/.test(ponte) &&
  !/CHAVE_ESTADO/.test(ponte) &&
  /GKBANCO/.test(discoSrc) &&
  !/from ['"]\.\/banco_front\.js['"]/.test(ponte) &&
  !/banco_glsl/.test(cardsSrc) &&
  !/banco_glsl/.test(latexSrc) &&
  !/initMotorCampo/.test(ponte) &&
  !/initCardsKernel/.test(ponte))

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
  walk(glslParaU(cat))
  ok('§G6 U do glsl cabe no schema', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  F5: GLSL catalogado != WASM != compile != exec; cadeia fora do boot.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
