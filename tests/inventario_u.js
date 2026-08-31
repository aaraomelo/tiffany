/* tests/inventario_u.js — mapa do Reino: leis na legenda, não no título op.
 *
 * F3 = contrato do card. Inventário = tags+desc+kernel. Sem GLSL.
 *   node tests/inventario_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { catalogoCards } from '../app/src/banco_cards_u.js'
import { CAPACIDADES_TARDAS } from '../app/src/banco_estado_gk_u.js'
import {
  ID_INVENTARIO_GK, CHIPS_HERO, PECAS_NOMEADAS, ETAPAS, METODO, FASE_AURA,
  fatiaPecasArt, fatiaPintorMapa, inventarioDe, igualInventario,
  realiza, relacoesKernels, legendasDe, faseNaDesc,
  inventarioParaU, uParaInventario,
} from '../app/src/banco_inventario_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'inventario.json')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_inventario_u.js')
const CK = join(RAIZ, 'app', 'src', 'cards_kernel.js')
const CARDS = join(RAIZ, 'app', 'src', 'banco_cards_u.js')
const MAN_GK = join(RAIZ, 'app', 'src', 'manifesto.json')
const ART = join(RAIZ, 'app', 'src', 'kernels_campo.json')
const DISCO = join(RAIZ, 'app', 'src', 'banco_disco.js')
const ESTADO = join(RAIZ, 'app', 'src', 'banco_estado_gk_u.js')

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
const ponte = readFileSync(PONTE, 'utf8')
const ck = readFileSync(CK, 'utf8')
const cardsSrc = readFileSync(CARDS, 'utf8')
const discoSrc = readFileSync(DISCO, 'utf8')
const estadoSrc = readFileSync(ESTADO, 'utf8')
const gkMan = JSON.parse(readFileSync(MAN_GK, 'utf8'))
const artSrc = readFileSync(ART, 'utf8')

const pecas = fatiaPecasArt(artSrc)
const { pintor, mapa } = fatiaPintorMapa(artSrc)
const inv = inventarioDe(gkMan, pecas, pintor, mapa)
const catF3 = catalogoCards(gkMan, pecas)
const dump = JSON.stringify(inv)

ok('§I0 instancia id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_INVENTARIO_GK && inst.camada === 'descricao' &&
  !schema.properties.kind.enum.includes('inventario'))
ok('§I0 ponte_inventario no motor',
  man.corpos?.motor?.ponte_inventario === 'app/src/banco_inventario_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_inventario)))
ok('§I0 schema_inventario no disco',
  man.corpos?.motor?.schema_inventario === 'conecthus/schema/inventario.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_inventario)))
ok('§I0 nucleo: tomografia; leis na legenda; F8 ainda nao',
  /tomografia/.test(man.corpos?.motor?.nucleo?.inventario_gk || '') &&
  /op != regua != tag/.test(man.corpos?.motor?.nucleo?.inventario_gk || '') &&
  /F8 ainda nao/.test(man.corpos?.motor?.nucleo?.inventario_gk || ''))

ok('§I1 F3 nao reaberto; inventario e descricao, nao execucao',
  CAPACIDADES_TARDAS.map((c) => c.id).join(' ') === 'cards latex glsl' &&
  !/ciclo: 'F8'/.test(estadoSrc) &&
  ETAPAS.join('>') === 'F3>inventario>semantica>lei>F8' &&
  METODO.join('>') === 'observar>relacionar>provar>promover' &&
  ponte.startsWith('// banco_inventario'))

ok('§I1 nCards manifesto ≡ F3 ≡ inventario',
  inv.nCards === catF3.length &&
  inv.nCards === gkMan.secoes.reduce((a, s) => a + (s.pecas || []).length, 0) &&
  inv.nSecoes === 13)

ok('§I1 92 cards != 12 kernels',
  inv.nCards === 92 && inv.nKernels === 12 && inv.nNomeados === 2 &&
  inv.nCards > inv.nKernels &&
  PECAS_NOMEADAS.join(' ') === inv.nomeados.map((n) => n.nome).join(' '))

ok('§I1 GLSL nao entra no mapa',
  !dump.includes('#version') &&
  !dump.includes('precision highp') &&
  !/from ['"]\.\/kernels_campo/.test(ponte) &&
  !ponte.includes('art.kernels') &&
  fatiaPintorMapa(artSrc).pintor.aura.lim === 1.6)

{
  const byK = Object.fromEntries(inv.kernels.map((k) => [k.kernel, k]))
  ok('§I1 kernels observados (nao categorias a priori)',
    byK.textura.n === 35 && byK.textura.dim === '3d' &&
    byK.respira.n === 16 && byK.respira.dim === '3d' &&
    byK.aura.n === 15 && byK.aura.dim === '2d' && byK.aura.chip === true &&
    byK.galaxia.n === 6 && byK.espiral.n === 6 &&
    byK.costura.n === 3 && byK.venom_rev.n === 3 &&
    byK.ferramenta.n === 2 &&
    byK.floco.n === 1 && byK.julia.n === 1 && byK.caelum.n === 1 && byK.pulso.n === 1 &&
    inv.kernels.filter((k) => k.chip).length === 6 &&
    CHIPS_HERO.every((n) => byK[n] && byK[n].chip) &&
    byK.textura.campos.includes('spec') &&
    byK.textura.campos.includes('shadow') &&
    byK.costura.campos.includes('chiB') &&
    byK.julia.escalaA && byK.julia.escalaA.includes('pcx'))
}

ok('§I1 um programa por kernel (cards_kernel), nao por card',
  ck.includes('mesa.progs.has(kernel)') &&
  ck.includes('art.kernels[kernel]') &&
  ck.includes('registra(quadro)'))

ok('§I1 relogio e WASM sao do pulso, nao por card',
  inv.kernels.every((k) => k.relogio && k.wasm === 'painel_motor' && k.glsl === false) &&
  inv.nomeados.some((n) => n.nome === 'coracao_revela' && n.secao === 'coracao') &&
  inv.nomeados.some((n) => n.nome === 'captura' && n.secao === 'dinamica'))

ok('§I2 P→U→P nCards/nKernels; camada descricao',
  inventarioParaU(inv).kind === 'pagina' &&
  inventarioParaU(inv).camada === 'descricao' &&
  inventarioParaU(inv).filhos.length === 12 &&
  uParaInventario(inventarioParaU(inv)).nCards === 92 &&
  uParaInventario(inventarioParaU(inv)).nKernels === 12)

ok('§I2 descricao(foo) = descricao(bar)', igualInventario(inv, inventarioDe(gkMan, pecas, pintor, mapa)))

{
  const byK = Object.fromEntries(inv.kernels.map((k) => [k.kernel, k]))
  const opsAura = new Set(byK.aura.ops)
  const rainha = catF3.find((c) => c.nome === 'rainha_tiffany')
  ok('§I5 demonstracao != lei != instrumento; K realiza op',
    inv.nOps === 63 &&
    inv.nArestas === inv.kernels.reduce((a, k) => a + k.ops.length, 0) &&
    inv.arestas.length === inv.nArestas &&
    rainha && rainha.kernel === 'aura' && opsAura.has(rainha.op) &&
    realiza(byK.aura).some((e) => e.op === rainha.op && e.kernel === 'aura') &&
    byK.textura.ops.length <= byK.textura.n &&
    byK.textura.cards.length === 35 &&
    byK.respira.cards.length === 16 &&
    inv.kernels.every((k) => k.cards.length === k.n) &&
    inv.nArestas + inv.opsNomeadas.length === inv.nOps)

  ok('§I5 relacoes observadas (nao geometria/iluminacao/campo)',
    inv.relacoes.some((r) => r.de === 'venom_rev' && r.para === 'aura' && r.tipo === 'entradas') &&
    inv.relacoes.some((r) => r.de === 'venom_rev' && r.tipo === 'chi') &&
    inv.relacoes.some((r) => r.de === 'costura' && r.tipo === 'dual_AB') &&
    inv.relacoes.some((r) => r.de === 'textura' && r.tipo === 'parametros_luz' && /spec/.test(r.evidencia)) &&
    relacoesKernels(inv.kernels).length === inv.relacoes.length &&
    !JSON.stringify(inv.relacoes).includes('geometria') &&
    !JSON.stringify(inv.relacoes).includes('iluminacao'))
}

{
  const L = inv.legendas
  const byK = Object.fromEntries(inv.kernels.map((k) => [k.kernel, k]))
  ok('§I5 leis na legenda: op != regua != tag != formula',
    L.cardsComTags === 92 && L.cardsComFP === 92 &&
    L.nTags === 408 && L.nRots === 63 && L.nPares === 177 &&
    L.nReguas === 32 && L.nReguas !== inv.nOps &&
    L.reguaVsOp.iguais === 8 && L.reguaVsOp.diferentes === 54 && L.reguaVsOp.semRegua === 30 &&
    L.nFP === 9 && L.nRevela === 5 &&
    L.nFaseDesc === 12 && L.nFaseUniq === 1 &&
    L.faseAura === FASE_AURA &&
    byK.aura.nFaseDesc === 12 &&
    L.fases.every((f) => f.secao === 'corte' || f.secao === 'elenco') &&
    L.fases.every((f) => faseNaDesc(f.fase) === FASE_AURA || f.fase === FASE_AURA) &&
    inv.relacoes.some((r) => r.de === 'aura' && r.tipo === 'fase_desc') &&
    legendasDe(gkMan).nTags === 408 &&
    /op != regua != tag/.test(inventarioParaU(inv).proibicao))
}

ok('§I3 ponte nao puxa cadeia de cena; front nao importa inventario',
  !/from ['"]\.\/motor_campo\.js['"]/.test(ponte) &&
  !/from ['"]\.\/cards_kernel\.js['"]/.test(ponte) &&
  !/from ['"]\.\/motor_wasm\.js['"]/.test(ponte) &&
  !/banco_inventario_u/.test(front) &&
  !/gravaEstado/.test(ponte) &&
  /GKBANCO/.test(discoSrc) &&
  /from ['"]\.\/banco_cards_u\.js['"]/.test(ponte))

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
  walk(inventarioParaU(inv))
  ok('§I4 U do inventario cabe no schema', extra.length === 0)
}

console.log('')
if (!falhas) {
  console.log('  inventario: 92 cards; leis na legenda (408 tags, 32 reguas, fase aura ×12).')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
