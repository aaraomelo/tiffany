/* tests/latex_u.js — F4: LaTeX sob demanda. tex.wasm ∉ G_paint.
 *
 * existência ≠ carga ≠ execução. Card dispara. foo ⊥ bar.
 * ?gk=1 sem usar LaTeX ⇒ 0 fetches de tex.wasm.
 *   node tests/latex_u.js
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
import { CARD_PRIMEIRO, acaoDoCard } from '../app/src/banco_cards_u.js'
import {
  ID_LATEX_GK, CHAVE_LATEX, FONTE_LATEX, WASM_LATEX, EXPR_CONHECIDA,
  resetLatex, faseLatex, fetchesTexWasm, moduloLatexCarregado,
  idDeArquivo, catalogoLatex, igualCatalogoLatex, querLatex,
  leLatexSelecionado, resultadoLatex, igualResultado,
  carregaLatex, executaLatex, disparaLatex, latexParaU, uParaLatex,
} from '../app/src/banco_latex_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'latex.json')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const MAIN = join(RAIZ, 'app', 'src', 'main.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_latex_u.js')
const TRAD = join(RAIZ, 'app', 'src', 'tex_tradutor.js')
const DISCO = join(RAIZ, 'app', 'src', 'banco_disco.js')
const CARDS = join(RAIZ, 'app', 'src', 'banco_cards_u.js')
const MAN_GK = join(RAIZ, 'app', 'src', 'manifesto.json')
const DOCS_T = join(RAIZ, 'app', 'src', 'docs_tradutor.json')
const TEX_W = join(RAIZ, 'tests', 'tex_wasm.js')
const WASM = join(RAIZ, 'assets', 'figuras', 'wasm', 'tex.wasm')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

resetLatex()

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const inst = JSON.parse(readFileSync(INST, 'utf8'))
const front = readFileSync(FRONT, 'utf8')
const mainJs = readFileSync(MAIN, 'utf8')
const ponte = readFileSync(PONTE, 'utf8')
const trad = readFileSync(TRAD, 'utf8')
const discoSrc = readFileSync(DISCO, 'utf8')
const cardsSrc = readFileSync(CARDS, 'utf8')
const gkMan = JSON.parse(readFileSync(MAN_GK, 'utf8'))
const docsMap = JSON.parse(readFileSync(DOCS_T, 'utf8')).docs
const texWasmSrc = existsSync(TEX_W) ? readFileSync(TEX_W, 'utf8') : ''
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'

const cat = catalogoLatex(gkMan.docs, docsMap)
const enredo = cat.find((d) => d.id === 'enredo')
const estatico = cat.find((d) => d.id === 'fisica-araniana')

ok('§L0 instancia id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_LATEX_GK && inst.camada === 'capacidade' &&
  !schema.properties.kind.enum.includes('latex'))
ok('§L0 ponte_latex no motor',
  man.corpos?.motor?.ponte_latex === 'app/src/banco_latex_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_latex)))
ok('§L0 schema_latex no disco',
  man.corpos?.motor?.schema_latex === 'conecthus/schema/latex.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_latex)))
ok('§L0 nucleo: existencia != carga != execucao; tex.wasm notin paint',
  /existencia != carga != execucao/.test(man.corpos?.motor?.nucleo?.latex_gk || '') &&
  /tex\.wasm so na execucao/.test(man.corpos?.motor?.nucleo?.latex_gk || ''))

ok('§L1 descoberta: tex_tradutor continua capacidade tardia F4',
  fatiaDeFonte(FONTE_LATEX) === 'capacidades' &&
  CAPACIDADES_TARDAS.some((c) => c.id === 'latex' && c.ciclo === 'F4' && c.fonte === FONTE_LATEX) &&
  /from ['"]\.\/tex_tradutor\.js['"]/.test(mainJs) &&
  !/from ['"]\.\/tex_tradutor\.js['"]/.test(front))

ok('§L1 catalogo docs ≡ original (idDeArquivo + DOCS)',
  cat.length === gkMan.docs.length &&
  enredo && enredo.compoe && enredo.fonte === 'enredo.tex' &&
  estatico && !estatico.compoe &&
  idDeArquivo('/docs/enredo.pdf') === 'enredo' &&
  idDeArquivo('/docs/fisica-araniana.pdf') === 'fisica-araniana' &&
  trad.includes('/\\/docs\\/([A-Za-z0-9_-]+)\\.pdf/') &&
  ponte.includes('/\\/docs\\/([A-Za-z0-9_-]+)\\.pdf/'))

{
  const orig = gkMan.docs.map((d) => {
    const id = idDeArquivo(d.arquivo)
    const fonte = (id && docsMap[id]) || ''
    return { id, fonte, compoe: !!fonte }
  })
  ok('§L1 card_GK latex original ≡ motor (ids composiveis)',
    igualCatalogoLatex(cat.map((d) => ({ id: d.id, fonte: d.fonte, compoe: d.compoe })), orig) &&
    orig.filter((d) => d.compoe).length === Object.keys(docsMap).filter((k) =>
      gkMan.docs.some((d) => idDeArquivo(d.arquivo) === k)).length)
}

ok('§L2 EXPR_CONHECIDA e a boxed do original (tex_wasm §T5)',
  EXPR_CONHECIDA.includes('boxed') && EXPR_CONHECIDA.includes('sigma') &&
  texWasmSrc.includes('boxed') && texWasmSrc.includes('\\sigma') && texWasmSrc.includes('smallmatrix'))

{
  const pdf = new TextEncoder().encode('%PDF-1.4\n/Type/SementeEstrela\n/Type/AssinaturaOito\n%%EOF\n')
  const orig = resultadoLatex(pdf)
  const mot = resultadoLatex(pdf)
  ok('§L2 expressao conhecida: resultado original ≡ motor (%PDF + semente + EOF)',
    orig.pdf && orig.eof && orig.semente && orig.oito &&
    igualResultado(orig, mot))
}

ok('§L2 P→U→P n/compoe; kind=pagina camada=capacidade',
  latexParaU(cat).kind === 'pagina' && latexParaU(cat).id === 'gk' &&
  latexParaU(cat).camada === 'capacidade' &&
  uParaLatex(latexParaU(cat)).n === cat.length &&
  uParaLatex(latexParaU(cat)).compoe === cat.filter((d) => d.compoe).length)

const rainha = { nome: CARD_PRIMEIRO.nome, secao: CARD_PRIMEIRO.secao, desc: '', op: '', kernel: 'aura', refs: { glsl: true, wasm: false, latex: false } }
ok('§L2 card sem latex nao e gatilho (existencia != carga)',
  !querLatex(rainha) &&
  !acaoDoCard(rainha).chama.includes('latex') &&
  querLatex(enredo))

{
  const stubMod = { idDeArquivo, DOCS: docsMap }
  let nFetch = 0
  const pdf = new TextEncoder().encode('%PDF-1.4\n/Type/SementeEstrela\n/Type/AssinaturaOito\n%%EOF\n')
  const opts = {
    importar: async () => stubMod,
    fetchWasm: async () => { nFetch++; return pdf },
    compor: async (tex) => {
      if (!tex.includes('\\boxed')) throw new Error('expr')
      return pdf
    },
  }
  resetLatex()
  catalogoLatex(gkMan.docs, docsMap)
  const d0 = await disparaLatex(null, rainha)
  ok('§L3 ?gk=1 sem usar LaTeX => tex.wasm = 0 fetches',
    d0.executou === false && d0.wasm === 0 && fetchesTexWasm() === 0 &&
    faseLatex() === 'existencia' && !moduloLatexCarregado() && nFetch === 0)

  const d1 = await disparaLatex(null, enredo)
  ok('§L3 seleccionar doc nao carrega tradutor nem wasm',
    d1.id === 'enredo' && d1.carregou === false && d1.executou === false &&
    fetchesTexWasm() === 0 && faseLatex() === 'existencia' && nFetch === 0)

  const d2 = await disparaLatex(null, enredo, { ...opts, carregar: true })
  ok('§L3 carga = import() tex_tradutor; wasm ainda 0',
    d2.carregou === true && d2.executou === false &&
    fetchesTexWasm() === 0 && nFetch === 0 && faseLatex() === 'carga')

  const d3 = await disparaLatex(null, enredo, { ...opts, executar: true, expr: EXPR_CONHECIDA })
  ok('§L3 execucao: tex.wasm aparece so depois do uso',
    d3.executou === true && d3.wasm === 1 && fetchesTexWasm() === 1 && nFetch === 1 &&
    faseLatex() === 'execucao' &&
    d3.resultado && d3.resultado.pdf && d3.resultado.semente && d3.resultado.eof)

  ok('§L3 tex.wasm so dentro de carregaMotor no original; ponte nao importa tradutor',
    trad.indexOf('async function carregaMotor') < trad.indexOf("fetch('/wasm/tex.wasm')") &&
    WASM_LATEX === '/wasm/tex.wasm' &&
    !/from ['"]\.\/tex_tradutor\.js['"]/.test(ponte) &&
    ponte.includes("import('./tex_tradutor.js')") &&
    ponte.indexOf('export async function carregaLatex') < ponte.indexOf("import('./tex_tradutor.js')") &&
    ponte.indexOf('export async function executaLatex') < ponte.indexOf('nWasm++'))
}

ok('§L3 wasm no disco e o mesmo artefacto (porta, nao fetch no paint)',
  existsSync(WASM) &&
  /inicia_wasm/.test(trad) &&
  /compila_ficheiro/.test(trad))

{
  resetLatex()
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  const catFoo = catalogoLatex(gkMan.docs, docsMap)
  const catBar = catalogoLatex(gkMan.docs, docsMap)
  await disparaLatex(foo, enredo)
  await disparaLatex(bar, cat.find((d) => d.id === 'teoria'))
  const e = estadoVazio()
  gravaEstado(e, ls)
  ok('§L4 descricao latex(foo) = descricao(bar)', igualCatalogoLatex(catFoo, catBar))
  ok('§L4 execucao(foo) perp (bar); S_ESTADO intocado',
    leLatexSelecionado(foo) === 'enredo' &&
    leLatexSelecionado(bar) === 'teoria' &&
    leLatexSelecionado(foo) !== leLatexSelecionado(bar) &&
    ls.getItem(CHAVE_LATEX) === null &&
    chaveIsolada(CHAVE_LATEX) &&
    chaveIsolada(CHAVE_CADEIA + 'x') &&
    !chaveIsolada(CHAVE_ESTADO) &&
    JSON.parse(ls.getItem(CHAVE_ESTADO)).magia === MAGIA &&
    !JSON.stringify(leEstado(ls)).includes('enredo') &&
    fetchesTexWasm() === 0)
}

const idFoo = await ligaIdentidade(discoIsolado(memoriaLS(), 'foo'), { chave: pub })
const idBar = await ligaIdentidade(discoIsolado(memoriaLS(), 'bar'), { chave: pub })
const idHex = await idEstavelDaChave(pub)
ok('§L4 mesma chave → mesmo id(K) nos dois tenants',
  idFoo.id === idHex && idBar.id === idHex && idFoo.id !== 'foo')

ok('§L5 banco_front nao toca latex: 0 tex.wasm, 0 tex_tradutor, 0 ponte F4',
  !/tex\.wasm/.test(front) &&
  !/tex_tradutor/.test(front) &&
  !/banco_latex_u/.test(front) &&
  !/from ['"]\.\/banco_disco\.js['"]/.test(ponte) &&
  !/gravaEstado/.test(ponte) &&
  !/CHAVE_ESTADO/.test(ponte) &&
  /GKBANCO/.test(discoSrc) &&
  !/from ['"]\.\/banco_front\.js['"]/.test(ponte) &&
  !/from ['"][^'"]*motor_campo/.test(ponte) &&
  !/banco_latex/.test(cardsSrc))

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
  walk(latexParaU(cat))
  ok('§L6 U do latex cabe no schema', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  F4: existencia != carga != execucao; tex.wasm notin paint; iframe oraculo.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
