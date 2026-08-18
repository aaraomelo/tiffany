/* volta_compila.js — 1 BIT ENTRE COMPOSIÇÕES: o monte recua, o banco é o LS/Map.
 *
 * O Aarão: dissipação exponencial = memória a refrescar / estado a alimentar o
 * próximo passo. O paradigma (corpo_analitico, estado_caos): sem recorrência, λ=0.
 * Banco = Map (corpo no LS); fopen miss → poe 1; volta zera FICH.
 *
 *   node tests/volta_compila.js
 */
'use strict'
const fs = require('fs')
const path = require('path')

const RAIZ = path.resolve(__dirname, '..')
const WASM = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'tex.wasm')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
  console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`)
}

const bytes = fs.readFileSync(WASM)
const man = JSON.parse(fs.readFileSync(path.join(RAIZ, 'app', 'src', 'corpo.json'), 'utf8'))
const { instanciaTex, hitCorpo } = require('./tex_env.js')
const num = (x) => typeof x === 'bigint' ? Number(x) : x
const now = () => Number(process.hrtime.bigint()) / 1e6

const cache = new Map(man.ficheiros.map((f) => [f, fs.readFileSync(path.join(RAIZ, f))]))
let missN = 0
const poeSet = new Set()
let E = null

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
function hit (nome) {
  return hitCorpo(cache, nome)
}

E = instanciaTex(bytes, (ptr) => {
  const v = mem()
  let s = ''
  for (let i = ptr; i < v.length && v[i]; i++) s += String.fromCharCode(v[i])
  const h = hit(s)
  if (!h) return 0
  if (poeSet.has(h.nome)) return 1
  poeFich(h.nome, h.u8)
  poeSet.add(h.nome)
  missN++
  return 1
}).exports

function compoe (nome) {
  E.limpa_saida()
  const t0 = now()
  const rc = num(E.compila_ficheiro(poeStr(nome), poeStr('saida.pdf')))
  const ms = Math.round(now() - t0)
  const tam = num(E.tam_saida())
  const end = num(E.MOVE(14, 1))
  const pdf = tam > 100 ? Buffer.from(mem().slice(end, end + tam)) : Buffer.alloc(0)
  const latin = pdf.toString('latin1')
  return {
    nome, rc, ms, tam, miss: missN,
    forms: (latin.match(/\/Subtype\/Form/g) || []).length,
    eof: latin.includes('%%EOF'),
    disco: E.DISCO.buffer.byteLength,
  }
}

console.log('=== 1 BIT + miss: volta_compila, Map → fopen → poe 1 ===\n')

ok('a porta tem marca_vfs e volta_compila — o bit da reversão',
  typeof E.marca_vfs === 'function' && typeof E.volta_compila === 'function')

E.inicia_wasm()
const discoInicia = E.DISCO.buffer.byteLength
console.log(`   inicia DISCO ${(discoInicia / 1048576).toFixed(2)} MiB (banco 0–2, sem PDF)`)
ok('§V0 inicia não reserva 128 MiB — só o banco', discoInicia < 20 * 1048576)

E.marca_vfs()
console.log(`   marca_vfs (sem poe): DISCO ${(E.DISCO.buffer.byteLength / 1048576).toFixed(2)} MiB`)

const seq = []
for (const d of ['catalogo.tex', 'papers/corpo_computacional.tex', 'enredo.tex']) {
  E.volta_compila()
  missN = 0; poeSet.clear()
  console.log(`   a compor ${d} via miss`)
  let r
  try { r = compoe(d) } catch (e) {
    r = { nome: d, rc: -99, ms: 0, tam: 0, forms: 0, eof: false, miss: missN, disco: E.DISCO.buffer.byteLength, trap: String(e.message || e).slice(0, 80) }
  }
  seq.push(r)
  console.log(`      rc=${r.rc} tam=${r.tam} Forms=${r.forms} miss=${r.miss} ${r.ms} ms disco=${(r.disco / 1048576).toFixed(1)} MiB${r.trap ? ' trap=' + r.trap : ''}`)
}

ok('§V1 catálogo via miss: rc=0 e %%EOF',
  seq[0].rc === 0 && seq[0].eof && seq[0].tam > 1e6 && seq[0].miss > 10)
ok('§V2 o computacional a seguir via miss: rc=0 e tam fresco',
  seq[1].rc === 0 && seq[1].eof && seq[1].tam > 1000 && seq[1].tam < 2.2e6 && seq[1].forms < 800 && seq[1].miss > 5)
ok('§V3 enredo via miss: fresco (~385 pág.)',
  seq[2].rc === 0 && seq[2].eof && seq[2].tam > 20e6 && seq[2].tam < 80e6
  && seq[2].forms > 700 && seq[2].forms < 1200 && seq[2].miss > 10)

E.volta_compila()
missN = 0; poeSet.clear()
console.log('   a compor catalogo.tex depois do enredo (miss de novo)')
let cat2
try { cat2 = compoe('catalogo.tex') } catch (e) {
  cat2 = { rc: -99, tam: 0, forms: 0, eof: false, ms: 0, miss: missN, disco: E.DISCO.buffer.byteLength, trap: String(e.message || e).slice(0, 80) }
}
console.log(`      rc=${cat2.rc} tam=${cat2.tam} Forms=${cat2.forms} miss=${cat2.miss} ${cat2.ms} ms`)
ok('§V4 catálogo depois do enredo: gato∘esquilo=id — mesmo tam/Forms',
  cat2.rc === 0 && cat2.eof && cat2.tam > 50e6
  && Math.abs(cat2.tam - seq[0].tam) < 1000
  && Math.abs(cat2.forms - seq[0].forms) < 5
  && cat2.miss === seq[0].miss)

E.volta_compila()
missN = 0; poeSet.clear()
const d1 = compoe('papers/corpo_computacional.tex')
const m1 = d1.miss
E.volta_compila()
missN = 0; poeSet.clear()
const d2 = compoe('papers/corpo_computacional.tex')
console.log(`   §V5 computacional×2 miss=${m1}/${d2.miss} tam=${d1.tam}/${d2.tam}`)
ok('§V5 computacional×2 via miss: id após volta (FICH zera, Map fica)',
  d1.rc === 0 && d2.rc === 0 && d1.tam === d2.tam && d1.forms === d2.forms
  && m1 > 5 && d2.miss === m1)

console.log('\n==========================================================================')
if (!falhas) {
  console.log('  1 bit = CURSOR ← MARCO + N_FICH=0. Banco = Map/LS; miss põe 1 ficheiro.')
} else console.log(`  FALHOU: ${falhas}`)
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
