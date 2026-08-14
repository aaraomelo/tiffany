/* app_arranca.js — O APP ARRANCA, E O RELÓGIO CHEGA A ARRANCAR.
 *
 * Este medidor nasceu de um defeito meu que NENHUMA das medidas que eu tinha apanhava.
 *
 * Ao tirar os GIFs do `velocidade.js` levei à frente a função `controle()` — que não era do
 * GIF, era o SLIDER. Ficou a chamada sem a definição. E então:
 *
 *   · o `npm run build` PASSOU        (o bundler não resolve nomes livres; não é o trabalho dele)
 *   · a página devolveu 200           (o HTML serve-se na mesma)
 *   · a bateria ficou 330 : 330       (nenhum medidor toca no app)
 *
 * e mesmo assim o app estava morto: `initVelocidade()` atirava ReferenceError, e a linha
 * SEGUINTE do main — `iniciaRelogio(avancaFase)` — nunca corria. O relógio único nunca
 * arrancava. O que se via era o último quadro pintado, parado. O Aarão viu-o antes de mim:
 * «aparece só imagem da corte sem animação, o resto apagado».
 *
 * A lição é sobre a MEDIDA, não sobre o bug: eu tinha três sinais verdes e os três mediam
 * outra coisa. Compilar não é arrancar, e servir não é correr. Só EXECUTAR mede executar.
 *
 * Aqui o bundle corre mesmo, num DOM mínimo, e mede-se em DUAS METADES:
 *
 *   §A1  o módulo carrega sem lançar          — o resíduo que tem de ser ZERO
 *   §A2  e o relógio chega a ser armado       — o que NÃO pode ser zero: se ninguém se
 *                                                regista, não há nada a animar
 *   §A3  e a mutação acusa                    — apaga-se uma definição e o §A1 TEM de falhar,
 *                                                senão este medidor não está a medir nada
 *
 *   node tests/app_arranca.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const raiz = path.join(__dirname, '..', 'app')
let falhas = 0
const ok = (msg, cond) => {
  console.log('      ' + (cond ? '✓' : '✗') + '  ' + msg)
  if (!cond) falhas++
}

/* O DOM mínimo: só o que o arranque toca. Não é para simular o navegador — é para deixar o
 * código CORRER até onde um nome livre rebentaria. */
function domMinimo () {
  const registados = []
  const elem = () => {
    const e = {
      style: {}, dataset: {}, classList: { add () {}, remove () {}, toggle () {}, contains: () => false },
      children: [], attributes: {},
      appendChild (c) { this.children.push(c); return c },
      removeChild () {}, remove () {}, setAttribute (k, v) { this.attributes[k] = v },
      getAttribute (k) { return this.attributes[k] }, removeAttribute () {},
      addEventListener () {}, removeEventListener () {},
      querySelector: () => elem(), querySelectorAll: () => [],
      getBoundingClientRect: () => ({ width: 100, height: 100, top: 0, bottom: 100, left: 0, right: 100 }),
      getContext: () => null,             // sem campo: é o caminho que o app promete suportar
      insertAdjacentHTML () {}, focus () {}, click () {},
      isConnected: true, textContent: '', title: '', className: '', id: '',
      firstChild: null, lastChild: null, parentNode: null, parentElement: null, nextSibling: null,
      childNodes: [], clientWidth: 100, clientHeight: 100, offsetWidth: 100, offsetHeight: 100,
      width: 100, height: 100, src: '', href: '', value: '', checked: false, disabled: false,
      insertBefore (c) { this.children.push(c); return c }, replaceChild (c) { return c },
      cloneNode () { return elem() }, contains: () => false, matches: () => false,
      closest: () => null, scrollIntoView () {}, animate: () => ({ cancel () {}, finish () {} }),
      toDataURL: () => 'data:,', getAttributeNS: () => null, setAttributeNS () {},
      hasAttribute: () => false, before () {}, after () {}, replaceWith () {}, append () {}, prepend () {},
      set innerHTML (v) { this._html = v }, get innerHTML () { return this._html || '' }
    }
    /* o <template> tem .content, e o main.js constrói TODO o DOM por lá (o helper `el`).
     * Sem isto o arranque trava na primeira linha útil — e o medidor nunca chegava ao relógio. */
    e.content = { firstChild: e, children: [], querySelector: () => e, querySelectorAll: () => [] }
    return e
  }
  const doc = elem()
  doc.createElement = () => elem()
  doc.createElementNS = () => elem()
  doc.getElementById = () => elem()
  doc.getElementsByClassName = () => []
  doc.getElementsByTagName = () => []
  doc.body = elem()
  doc.documentElement = elem()
  doc.head = elem()
  doc.readyState = 'complete'
  const win = {
    document: doc, innerWidth: 1280, innerHeight: 900, devicePixelRatio: 1,
    addEventListener () {}, removeEventListener () {},
    requestAnimationFrame (f) { registados.push(f); return registados.length },
    cancelAnimationFrame () {}, setInterval: () => 0, clearInterval () {},
    setTimeout: (f) => { registados.push(f); return 0 }, clearTimeout () {},
    matchMedia: () => ({ matches: false, addEventListener () {} }),
    location: { href: 'http://localhost/', origin: 'http://localhost', pathname: '/' },
    navigator: { userAgent: 'node' }, console,
    fetch: () => Promise.reject(new Error('sem rede — de propósito')),
    WebAssembly: { instantiate: () => Promise.reject(new Error('sem wasm — de propósito')),
                   instantiateStreaming: () => Promise.reject(new Error('sem wasm')) },
    performance: { now: () => 0 },
    getComputedStyle: () => ({ getPropertyValue: () => '' }),
    IntersectionObserver: class { observe () {} unobserve () {} disconnect () {} },
    ResizeObserver: class { observe () {} unobserve () {} disconnect () {} }
  }
  win.window = win
  win.globalThis = win
  win.self = win
  /* auditoria 14/08: o canvas/tex_tradutor trouxe TextEncoder/TextDecoder ao
   * main — são globais REAIS do navegador e do Node, não DOM falso: passa-se
   * os do Node ao contexto, senão o main morre em «TextEncoder is not
   * defined» e o relógio nunca chega a armar (§A2 caía por arrasto) */
  win.TextEncoder = TextEncoder
  win.TextDecoder = TextDecoder
  return { win, registados }
}

/* corre um bundle IIFE dentro de um contexto com o DOM mínimo, e devolve o que aconteceu */
function corre (codigo) {
  const vm = require('vm')
  const { win, registados } = domMinimo()
  const ctx = vm.createContext(win)
  let erro = null
  try {
    new vm.Script(codigo, { filename: 'bundle.js' }).runInContext(ctx, { timeout: 20000 })
  } catch (e) { erro = e }
  /* A DISTINÇÃO QUE FAZ ESTE MEDIDOR VALER: um ReferenceError é um NOME LIVRE — código que
   * chama o que não existe, e isso é defeito do projeto, aconteça onde acontecer. Um TypeError
   * é quase sempre o DOM falso aqui em baixo a ser pobre demais — defeito do medidor, não do
   * app. Perseguir os TypeError seria reimplementar um navegador; e um medidor que exige um
   * navegador inteiro para dar verde acaba por não correr nunca. */
  const livre = erro && erro.constructor && erro.constructor.name === 'ReferenceError' ? erro : null
  return { erro, livre, registados }
}

function empacota (entrada, nomeGlobal) {
  const saida = path.join(require('os').tmpdir(), 'app_arranca_bundle.js')
  const g = nomeGlobal ? ` --global-name=${nomeGlobal}` : ''
  execSync(`npx esbuild ${JSON.stringify(entrada)} --bundle --format=iife --loader:.json=json${g} ` +
           `--define:import.meta.env.DEV=false --outfile=${JSON.stringify(saida)} --log-level=error`,
           { cwd: raiz, stdio: 'pipe' })
  return fs.readFileSync(saida, 'utf8')
}

/* CHAMAR AS EXPORTS, e não só carregar o módulo.
 *
 * Foi isto que o §A3 me ensinou a meio de o escrever: a primeira versão só CARREGAVA o main, e
 * o main chama initVelocidade() lá dentro de um .then() do WASM — que aqui não resolve. Ou
 * seja: eu tinha escrito um medidor que dava verde sem nunca passar pelo sítio do defeito que
 * o motivou. Carregar um módulo mede o corpo dele; só CHAMAR mede o corpo das suas funções. */
function corridaDoModulo (ficheiro) {
  const codigo = empacota(path.join(raiz, 'src', ficheiro), 'M')
  const vm = require('vm')
  const { win, registados } = domMinimo()
  const ctx = vm.createContext(win)
  let erro = null, chamadas = 0
  try {
    new vm.Script(codigo, { filename: ficheiro }).runInContext(ctx, { timeout: 20000 })
    const M = ctx.M || {}
    for (const k of Object.keys(M)) {
      if (typeof M[k] !== 'function' || !/^init/.test(k)) continue
      chamadas++
      M[k]()                       /* aqui é que um nome livre no CORPO da função aparece */
    }
  } catch (e) { erro = e }
  const livre = erro && erro.constructor && erro.constructor.name === 'ReferenceError' ? erro : null
  return { erro, livre, registados, chamadas }
}

console.log('\n=== O APP ARRANCA, E O RELÓGIO CHEGA A ARRANCAR =============================\n')

const entrada = path.join(raiz, 'src', 'main.js')

console.log('§A1  Cada módulo carrega E as suas exports init* correm — resíduo ZERO.\n')
const modulos = fs.readdirSync(path.join(raiz, 'src')).filter((f) => f.endsWith('.js')).sort()
let livres = 0, totalChamadas = 0
console.log('      módulo                init* chamadas   nome livre?')
for (const f of modulos) {
  let r
  try { r = corridaDoModulo(f) } catch (e) { r = { erro: e, livre: null, chamadas: 0 } }
  totalChamadas += r.chamadas
  if (r.livre) livres++
  console.log('      ' + f.padEnd(22) + String(r.chamadas).padEnd(17) +
              (r.livre ? 'SIM — ' + r.livre.message : 'não'))
}
ok('nenhum nome livre em nenhum módulo', livres === 0)
ok('e as exports init* foram mesmo chamadas', totalChamadas > 0)
console.log('      (' + modulos.length + ' módulos, ' + totalChamadas + ' funções init* invocadas.)')
console.log('\n      É aqui que `controle is not defined` aparece — e é aqui que o build e o 200')
console.log('      da página não apareciam. Compilar não é arrancar; servir não é correr.')

let bundle = null, primeira = null
{
  let erroBuild = null
  try { bundle = empacota(entrada) } catch (e) { erroBuild = e }
  ok('o esbuild empacota o main.js inteiro', erroBuild === null)
  if (bundle) primeira = corre(bundle)
}

console.log('\n§A2  E o relógio chega a ser ARMADO — o resíduo que NÃO pode ser zero.\n')
if (primeira) {
  const n = primeira.registados.length
  console.log('      consumidores/quadros armados no arranque: ' + n)
  ok('alguém se registou — há o que animar', n > 0)
  if (n === 0 && primeira.erro && !primeira.livre)
    console.log('      (zero porque o DOM mínimo travou antes — ver §A1)')
  console.log('\n      A outra metade da medida. Se o §A1 passar e este falhar, o app carrega')
  console.log('      e fica PARADO — que é exactamente o sintoma que se viu no ecrã.')
}

console.log('\n§A3  E a MUTAÇÃO acusa: sem a definição, o §A1 TEM de falhar.\n')
{
  const alvo = path.join(raiz, 'src', 'velocidade.js')
  const original = fs.readFileSync(alvo, 'utf8')
  let acusou = false, erroMut = null
  try {
    /* tira-se a definição de controle() e deixa-se a chamada — o defeito EXACTO que houve */
    const mutado = original.replace(/^function controle \(\) \{[\s\S]*?\n\}\n/m, '')
    if (mutado === original) throw new Error('a mutação não mordeu — controle() já não está lá')
    fs.writeFileSync(alvo, mutado)
    const r = corridaDoModulo('velocidade.js')
    acusou = r.livre !== null && /controle/.test(String(r.livre.message))
    if (r.livre) console.log('      com a definição fora: ReferenceError: ' + r.livre.message)
    else console.log('      com a definição fora: NENHUM nome livre — o medidor não está a medir')
  } catch (e) { erroMut = e } finally {
    fs.writeFileSync(alvo, original)   /* devolve-se SEMPRE, mesmo se algo rebentou */
  }
  if (erroMut) { console.log('      ' + erroMut.message); falhas++ }
  else ok('apagar a definição faz o §A1 falhar, e pelo nome certo', acusou)
  const devolvido = fs.readFileSync(alvo, 'utf8') === original
  ok('o ficheiro foi devolvido ao original', devolvido)
}

console.log('\n=== O ARRANQUE =============================================================')
console.log('  Três verdes que mediam outra coisa — build, 200 e bateria — e o app morto por')
console.log('  um nome livre. O bundler não resolve nomes livres porque não é o trabalho dele;')
console.log('  o servidor devolve 200 porque o HTML existe; a bateria não toca no app.')
console.log('')
console.log('  Aqui o bundle CORRE. E mede-se pelas duas metades: o resíduo que tem de ser zero')
console.log('  (nada lança) e o que não pode ser zero (alguém se registou no relógio). A mutação')
console.log('  fecha: apaga-se a definição e a medida acusa, pelo nome.')
if (falhas) { console.log('\n  FALHAS: ' + falhas + '\n'); process.exit(1) }
console.log('\n  RESÍDUO 0 — o app arranca, e o relógio chega a arrancar.\n')
