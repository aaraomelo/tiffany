/* autoria_tradutor.js — UM MEDIDOR: docs únicos + λ + selos + teto.
 *
 * Teorema do Metrónomo: maestro projecta; metrónomo lê; Lyapunov dualizado atesta.
 * Substitui a pergunta espalhada por traduz_pi_dim / volta / unifica numa
 * passagem curta de autoria. Lista = app/src/docs_tradutor.json.
 *
 *   node tests/autoria_tradutor.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execFileSync } = require('child_process')
const { instanciaTex, hitCorpo } = require('./tex_env.js')

const RAIZ = path.resolve(__dirname, '..')
const WASM = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'tex.wasm')
const DOCS_JSON = path.join(RAIZ, 'app', 'src', 'docs_tradutor.json')
const CORPO = path.join(RAIZ, 'app', 'src', 'corpo.json')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const docsJson = JSON.parse(fs.readFileSync(DOCS_JSON, 'utf8'))
const DOCS = { ...docsJson.docs, ...(docsJson.so_teste || {}) }
const man = JSON.parse(fs.readFileSync(CORPO, 'utf8'))

console.log('=== AUTORIA TRADUTOR: docs · λ · selos · teto ===\n')

/* §A0 lista única */
{
  const caminhos = Object.values(docsJson.docs)
  let todos = true
  for (const f of caminhos) {
    if (!fs.existsSync(path.join(RAIZ, f))) { todos = false; console.log('  falta', f) }
  }
  const noCorpo = caminhos.every((f) => man.ficheiros.includes(f))
  ok('§A0 docs_tradutor.json: ficheiros existem e estão no corpo.json', todos && noCorpo)
  console.log(`   ${caminhos.length} docs de produção + ${Object.keys(docsJson.so_teste || {}).length} só-teste\n`)
}

/* §A1 composição amostral: dualsort (rápido), corpo-peano, arquitetura (Gentil) */
const amostra = ['dualsort', 'corpo-peano', 'arquitetura', 'torre-induc']
const bytes = fs.readFileSync(WASM)
const cache = new Map(man.ficheiros.map((f) => [f, fs.readFileSync(path.join(RAIZ, f))]))
/* so_teste pode ainda não estar no corpo.json — põe-se do disco */
for (const f of Object.values(docsJson.so_teste || {})) {
  const p = path.join(RAIZ, f)
  if (fs.existsSync(p) && !cache.has(f)) cache.set(f, fs.readFileSync(p))
}
let E = null
const poeSet = new Set()
const num = (x) => (typeof x === 'bigint' ? Number(x) : x)
function mem () { return new Uint8Array(E.DISCO.buffer) }
function reserva (n) {
  const p = num(E.vfs_reserva(n))
  if (!p) throw new Error('vfs')
  return p
}
function poeStr (s) {
  const nb = Buffer.from(s, 'latin1')
  const p = reserva(nb.length + 1)
  mem().set(nb, p); mem()[p + nb.length] = 0
  return p
}
function poeFich (nome, b) {
  const pN = poeStr(nome)
  const pD = reserva(Math.max(b.length, 0) + 1)
  if (b.length) mem().set(b, pD)
  mem()[pD + b.length] = 0
  if (!E.poe_ficheiro(pN, pD, b.length)) throw new Error('poe ' + nome)
}

E = instanciaTex(bytes, (ptr) => {
  const v = mem()
  let s = ''
  for (let i = ptr; i < v.length && v[i]; i++) s += String.fromCharCode(v[i])
  const h = hitCorpo(cache, s)
  if (!h) return 0
  if (poeSet.has(h.nome)) return 1
  poeFich(h.nome, h.u8)
  poeSet.add(h.nome)
  return 1
}).exports
E.inicia_wasm()

function parseSemente (latin) {
  const i = latin.indexOf('/Type/SementeEstrela')
  if (i < 0) return null
  const chunk = latin.slice(i, i + 400)
  const n = (k) => { const m = chunk.match(new RegExp('/' + k + '\\s+(\\d+)')); return m ? Number(m[1]) : null }
  return { dim: n('Dim'), lado: n('Lado'), iface: n('Interface') }
}

function compoe (id) {
  const fonte = DOCS[id]
  if (!fonte) throw new Error('doc ' + id)
  if (typeof E.volta_compila === 'function') E.volta_compila()
  poeSet.clear()
  if (typeof E.marca_vfs === 'function') E.marca_vfs()
  E.limpa_saida()
  const t0 = process.hrtime.bigint()
  const rc = num(E.compila_ficheiro(poeStr(fonte), poeStr('saida.pdf')))
  const ms = Number(process.hrtime.bigint() - t0) / 1e6
  const tam = num(E.tam_saida())
  const end = num(E.MOVE(14, 1))
  const pdf = Buffer.from(mem().slice(end, end + tam))
  const latin = pdf.toString('latin1')
  const s = parseSemente(latin)
  const temTorre = latin.includes('/Type/AssinaturaTorre')
  const forms = (latin.match(/\/Subtype\s*\/Form/g) || []).length
  return {
    id, ms: Math.round(ms), rc, tam,
    eof: latin.includes('%%EOF'),
    semente: latin.includes('/Type/SementeEstrela'),
    oito: latin.includes('/Type/AssinaturaOito'),
    torre: temTorre,
    forms,
    dim: s && s.dim, lado: s && s.lado, iface: s && s.iface,
  }
}

console.log('doc            ms   dim if L  Forms  selos')
console.log('──────────────────────────────────────────────────')
const rows = []
for (const id of amostra) {
  const r = compoe(id)
  rows.push(r)
  const L = r.lado === 1 ? 'G' : 'H'
  const selos = [r.semente && 'S', r.oito && '8', r.torre && 'T'].filter(Boolean).join('')
  console.log(
    `${r.id.padEnd(14)} ${String(r.ms).padStart(4)}  ${String(r.dim).padStart(3)} ${String(r.iface).padStart(2)} ${L}  ${String(r.forms).padStart(5)}  ${selos}`
  )
}
console.log('')

/* P3: gargalo = Forms únicos (já 1 Form/glifo partilhado), não a dimensão */
{
  const cat = compoe('catalogo')
  rows.push(cat)
  console.log(`   catalogo Forms=${cat.forms} ms=${cat.ms} dim=${cat.dim} (glifos únicos, já partilhados)`)
  ok('§A7 P3: catalogo tem muitos Forms; custo ≠ dim (cf. torre-induc dim128)',
    cat.rc === 0 && cat.forms > 500 && cat.ms > 1000)
}

const todosOk = rows.filter((r) => r.id !== 'torre-induc').every((r) => r.rc === 0 && r.eof && r.semente && r.oito)
ok('§A1 amostra (prod) compõe com Semente+AssinaturaOito e %%EOF', todosOk)

const peano = rows.find((r) => r.id === 'corpo-peano')
const arq = rows.find((r) => r.id === 'arquitetura')
const ti = rows.find((r) => r.id === 'torre-induc')
ok('§A2 corpo-peano no lado Hurwitz (dim≤8 típico) ou selos intactos',
  peano && peano.rc === 0 && peano.semente)
ok('§A3 Gentil: arquitetura dim≥16 com AssinaturaTorre',
  arq && arq.dim >= 16 && arq.lado === 1 && arq.torre)
ok('§A4 sem teto do objecto: torre-induc dim>8 (Gentil continua)',
  ti && ti.rc === 0 && ti.dim > 8 && ti.lado === 1 && ti.torre)

/* §A5 volta: 1 bit — recompõe dualsort e volta */
{
  const r = compoe('dualsort')
  const antes = E.DISCO.buffer.byteLength
  const endAntes = num(E.MOVE(14, 1))
  if (typeof E.volta_compila === 'function') E.volta_compila()
  const endDepois = num(E.MOVE(14, 1))
  const depois = E.DISCO.buffer.byteLength
  ok('§A5 volta_compila: slot 14→0 (1 bit); banco de páginas pode ficar',
    r.rc === 0 && endAntes > 0 && endDepois === 0)
  console.log(`   DISCO ${ (antes/1e6).toFixed(1) }→${ (depois/1e6).toFixed(1) } MiB · PDF ${endAntes}→${endDepois}\n`)
}

/* §A6 portão dissipação (resumo) */
try {
  const out = execFileSync('sh', ['tools/dissipa.sh'], { cwd: RAIZ, encoding: 'utf8' })
  const m = out.match(/bits apagados por corrida[^\d]*(\d+)/)
  const bits = m ? Number(m[1]) : -1
  console.log(out.split('\n').filter((l) => l.includes('───') || l.includes('bits apagados') || l.includes('REGUA PROPRIA') || l.includes('corpos com')).join('\n'))
  ok('§A6 portão dissipação corre; medidores top com REGUA própria saem da conta Landauer',
    bits >= 0)
} catch (e) {
  ok('§A6 portão dissipação', false)
  console.log(e.message)
}

console.log('==========================================================================')
console.log('  Teorema do Metrónomo: projecta / lê / atesta · docs únicos · Hurwitz8≠teto')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
