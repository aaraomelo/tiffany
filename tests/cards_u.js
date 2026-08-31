/* tests/cards_u.js — F3: card é capacidade, não estado.
 *
 * card_GK^original ≡ card_GK^motor. Descrição global; interação ⊥ por tenant.
 * LaTeX/GLSL listados, não ingeridos. Card não usado não pinta.
 *   node tests/cards_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { MAGIA, CHAVE_ESTADO, estadoVazio, leEstado, gravaEstado } from '../app/src/banco_disco.js'
import { discoIsolado, chaveIsolada } from '../app/src/banco_tenant_u.js'
import { CHAVE_NAV } from '../app/src/banco_nav_u.js'
import { CHAVE_SESSAO, ligaIdentidade, idEstavelDaChave } from '../app/src/banco_identidade_u.js'
import { CHAVE_CADEIA } from '../app/src/banco_cristalchain_u.js'
import {
  ID_CARDS_GK, CHAVE_CARD, PECAS_NOMEADAS, CARD_PRIMEIRO,
  indicePecasArt, chaveKernel, catalogoCards, igualCard, igualCatalogo,
  acaoDoCard, selecionaCard, leCardSelecionado,
  cardParaU, uParaCard, catalogoParaU,
} from '../app/src/banco_cards_u.js'
import { igual } from '../app/src/banco_manifesto_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'cards.json')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const MAIN = join(RAIZ, 'app', 'src', 'main.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_cards_u.js')
const DISCO = join(RAIZ, 'app', 'src', 'banco_disco.js')
const MAN_GK = join(RAIZ, 'app', 'src', 'manifesto.json')
const ART = join(RAIZ, 'app', 'src', 'kernels_campo.json')

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
const ponte = readFileSync(PONTE, 'utf8')
const discoSrc = readFileSync(DISCO, 'utf8')
const gkMan = JSON.parse(readFileSync(MAN_GK, 'utf8'))
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'

/** Índice slim: só pecas.{kernel,secao}. Não parseia o objecto kernels (GLSL). */
function indiceDoArtefato (src) {
  const i = String(src).lastIndexOf('"pecas":')
  if (i < 0) return {}
  return indicePecasArt(JSON.parse('{' + src.slice(i)))
}

/** Algoritmo do original (main.js chaveKernel) sobre o índice, não sobre o GLSL. */
function chaveOriginal (p, sec, pecas) {
  if (pecas[p.nome + '@' + sec]) return p.nome + '@' + sec
  const a = pecas[p.nome]
  return (a && a.secao === sec) ? p.nome : null
}

const indice = indiceDoArtefato(readFileSync(ART, 'utf8'))
const cat = catalogoCards(gkMan, indice)

ok('§C0 instancia id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_CARDS_GK && inst.camada === 'capacidade' &&
  !schema.properties.kind.enum.includes('card'))
ok('§C0 ponte_cards no motor',
  man.corpos?.motor?.ponte_cards === 'app/src/banco_cards_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_cards)))
ok('§C0 schema_cards no disco',
  man.corpos?.motor?.schema_cards === 'conecthus/schema/cards.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_cards)))
ok('§C0 nucleo: card = capacidade; GLSL nao executa',
  /card = capacidade/.test(man.corpos?.motor?.nucleo?.cards_gk || '') &&
  /sem executar GLSL/.test(man.corpos?.motor?.nucleo?.cards_gk || ''))

ok('§C1 descoberta: catalogo = pecas do manifesto, sem GLSL no indice',
  cat.length === gkMan.secoes.reduce((n, s) => n + (s.pecas || []).length, 0) &&
  cat.length > 0 &&
  !JSON.stringify(indice).includes('#version') &&
  Object.values(indice).every((p) => p.kernel && p.secao))

{
  let n = 0
  let iguais = true
  for (const s of gkMan.secoes) {
    for (const p of s.pecas || []) {
      const orig = chaveOriginal(p, s.id, indice) || p.nome
      const mot = cat[n]
      if (!mot || mot.id !== orig || mot.nome !== p.nome || mot.secao !== s.id ||
        mot.titulo !== p.titulo || mot.kernel !== (chaveKernel(p, s.id, indice) && indice[chaveKernel(p, s.id, indice)].kernel || '')) {
        iguais = false
      }
      n++
    }
  }
  ok('§C1 card_GK original ≡ motor (chave, nome, secao, titulo, kernel)', iguais && n === cat.length)
}

ok('§C1 MOTOR nomeado sem kernel emitido',
  PECAS_NOMEADAS.every((nome) => {
    const c = cat.find((x) => x.nome === nome)
    return c && c.nomeado && !c.kernel && c.id === nome
  }) &&
  /const MOTOR = new Set\(\['coracao_revela', 'captura'\]\)/.test(mainJs))

const rainha = cat.find((c) => c.nome === CARD_PRIMEIRO.nome && c.secao === CARD_PRIMEIRO.secao)
ok('§C1 rainha_tiffany@corte kernel=aura (lista, nao compila)',
  rainha && rainha.id === 'rainha_tiffany' && rainha.kernel === 'aura' &&
  rainha.refs.glsl === true && rainha.refs.latex === false && rainha.refs.wasm === false)

ok('§C2 card != S_ESTADO != livro != sessao',
  CHAVE_CARD !== CHAVE_ESTADO &&
  CHAVE_CARD !== CHAVE_SESSAO &&
  CHAVE_CARD !== CHAVE_NAV &&
  !CHAVE_CARD.startsWith(CHAVE_CADEIA) &&
  chaveIsolada(CHAVE_CARD) &&
  !chaveIsolada(CHAVE_ESTADO) &&
  /GKBANCO/.test(discoSrc) &&
  !/gravaEstado/.test(ponte) &&
  !/CHAVE_ESTADO/.test(ponte) &&
  !/banco_disco/.test(ponte))

{
  const U = cardParaU(rainha)
  const P = uParaCard(U)
  ok('§C2 P→U→P id/nome/secao/kernel; kind=ficheiro nao estado',
    U.kind === 'ficheiro' && U.id === rainha.id &&
    P.id === rainha.id && P.nome === rainha.nome &&
    P.secao === rainha.secao && P.kernel === rainha.kernel &&
    igual(cardParaU(P).id, U.id) &&
    U.slots.glsl === 'ref' && U.slots.latex === 'N/A')
}

{
  const CU = catalogoParaU(cat)
  ok('§C2 catalogo kind=pagina id=gk camada=capacidade',
    CU.kind === 'pagina' && CU.id === ID_CARDS_GK && CU.camada === 'capacidade' &&
    CU.filhos.length === cat.length &&
    /card != S_ESTADO/.test(CU.proibicao))
}

{
  const a = acaoDoCard(rainha)
  const nomeado = cat.find((c) => c.nome === 'coracao_revela')
  ok('§C3 card → accao → capacidade; GLSL listado, nao executado',
    a.acao === 'selecionar' &&
    a.chama.join(' ') === 'glsl' &&
    a.executou === false &&
    acaoDoCard(nomeado).chama.length === 0 &&
    acaoDoCard(nomeado).executou === false)
}

{
  const foo = discoIsolado(memoriaLS(), 'foo')
  const a = selecionaCard(foo, rainha)
  ok('§C3 seleciona persiste id; executou continua false',
    a.executou === false &&
    leCardSelecionado(foo) === rainha.id)
}

ok('§C3 ponte nao importa kernels/cards/motor/tex; indice nao e o GLSL',
  !/from ['"][^'"]*kernels_campo/.test(ponte) &&
  !/from ['"][^'"]*cards_kernel/.test(ponte) &&
  !/from ['"][^'"]*cards_campo/.test(ponte) &&
  !/from ['"][^'"]*motor_campo/.test(ponte) &&
  !/from ['"][^'"]*motor_wasm/.test(ponte) &&
  !/from ['"][^'"]*tex_tradutor/.test(ponte) &&
  !/#version/.test(ponte) &&
  !/from ['"]\.\/main\.js['"]/.test(ponte))

{
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  const catFoo = catalogoCards(gkMan, indice)
  const catBar = catalogoCards(gkMan, indice)
  const rei = cat.find((c) => c.nome === 'rei_cifra' && c.secao === 'corte')
  selecionaCard(foo, rainha)
  selecionaCard(bar, rei)
  const e = estadoVazio()
  gravaEstado(e, ls)
  ok('§C4 descricao(foo) = descricao(bar)', igualCatalogo(catFoo, catBar) &&
    igualCard(catFoo[0], catBar[0]))
  ok('§C4 interacao(foo) perp interacao(bar); S_ESTADO intocado',
    leCardSelecionado(foo) === rainha.id &&
    leCardSelecionado(bar) === rei.id &&
    leCardSelecionado(foo) !== leCardSelecionado(bar) &&
    ls.getItem(CHAVE_CARD) === null &&
    ls.getItem(prefixo('foo')) === JSON.stringify({ id: rainha.id, acao: 'selecionar' }) &&
    !chaveIsolada(CHAVE_ESTADO) &&
    JSON.parse(ls.getItem(CHAVE_ESTADO)).magia === MAGIA &&
    !JSON.stringify(leEstado(ls)).includes(rainha.id) &&
    !JSON.stringify(leEstado(foo)).includes('rainha'))
}

function prefixo (t) {
  return 'gk:t:' + t + ':' + CHAVE_CARD
}

const idFoo = await ligaIdentidade(discoIsolado(memoriaLS(), 'foo'), { chave: pub })
const idBar = await ligaIdentidade(discoIsolado(memoriaLS(), 'bar'), { chave: pub })
const idHex = await idEstavelDaChave(pub)
ok('§C4 mesma chave → mesmo id(K) nos dois tenants',
  idFoo.id === idHex && idBar.id === idHex && idFoo.id !== 'foo')

ok('§C5 card nao usado nao pinta: import() so em hospedaGk, depois do quadro',
  !/from ['"]\.\/banco_cards_u\.js['"]/.test(front) &&
  front.includes("import('./banco_cards_u.js')") &&
  front.indexOf('function hospedaGk') < front.indexOf("import('./banco_cards_u.js')") &&
  front.indexOf("import('./banco_cards_u.js')") < front.indexOf('function ligaNavGk') &&
  front.indexOf('montaDom(') < front.indexOf('hospedaGk(') &&
  front.indexOf('aposQuadro') < front.indexOf('hospedaGk('))
ok('§C5 front nao puxa cards_kernel/campo, kernels_campo, motor, tex',
  !/from ['"]\.\/cards_kernel\.js['"]/.test(front) &&
  !/from ['"]\.\/cards_campo\.js['"]/.test(front) &&
  !/kernels_campo/.test(front) &&
  !/from ['"]\.\/motor_campo\.js['"]/.test(front) &&
  !/from ['"]\.\/motor_wasm\.js['"]/.test(front) &&
  !/from ['"]\.\/tex_tradutor\.js['"]/.test(front) &&
  !/from ['"]\.\/main\.js['"]/.test(front) &&
  !/app\/src\/manifesto\.json/.test(front))

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
  walk(catalogoParaU(cat))
  ok('§C6 U do catalogo cabe no schema', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  F3: card = capacidade; original ≡ motor; GLSL/LaTeX fora; iframe oraculo.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
