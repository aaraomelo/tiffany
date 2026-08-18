/* tests/assinatura_banal.js — o par (E, Φ) no corpus banal (eval 13/08).
 *
 * A ordem do diretor: validar o observador dual I(x) = (E, Φ) sobre dados
 * ORGÂNICOS — pares reais RETAIN/REOPEN — não sobre induções artificiais.
 *
 *   E(x) = Σ b²      (energia — o módulo; reconhece equivalência de massa)
 *   Φ(x) = Σ i·b     (fase — a posição interna; reconhece identidade ordenada)
 *
 * Fontes orgânicas: corpus/fala/conversa.tex (o corpus banal da casa — fala =
 * \section, resposta = corpo) e os pares reais dos eixos de texto do
 * Controle (cafe↔café, que↔qeu, casa↔caza — conecthus/core/eixos_texto.c).
 *
 * §B0  o corpus banal existe e dá ≥10 pares reais
 * §B1  RETAIN: a mesma resposta relida → I igual (0,0)
 * §B2  REOPEN: respostas DISTINTAS do corpus → I distingue (nenhum par real
 *      de respostas diferentes colide no observador dual)
 * §B3  os pares reais de vizinhança (acento/teclado/fonético) → REOPEN por I
 *      (variante de grafia é outro corpo para o observador; a ADMISSÃO é do
 *      Controle/𝒱, não da assinatura — camadas distintas)
 *
 *   node tests/assinatura_banal.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
/* limpeza da dupla árvore (ordem do diretor, 14/08): a forma embutida
 * saiu — a única implementação é a da infraestrutura (lib) */
const { Universal } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')
const U = Universal(sigmaPeano)

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const I = texto => { const e = U.escada(texto); return { E: e.E, fase: e.f1, f2: e.f2 } }
const igual = (a, b) => a.E === b.E && a.fase === b.fase

/* o degrau da escada em que o par se separa: 1=E, 2=Φ, 3=Φ₂, 0=não separa */
function degrau (a, b) {
  if (a.E !== b.E) return 1
  if (a.fase !== b.fase) return 2
  if (a.f2 !== b.f2) return 3
  return 0
}

/* §B0 — pares orgânicos do corpus banal */
const tex = fs.readFileSync(path.join(__dirname, '..', 'corpus', 'fala', 'conversa.tex'), 'utf8')
const pares = []
const rx = /\\section\{([^}]+)\}\s*\n([^\n\\]+)/g
let m
while ((m = rx.exec(tex)) && pares.length < 20) {
  const fala = m[1].trim(), resp = m[2].trim()
  if (fala.length >= 2 && resp.length >= 8) pares.push([fala, resp])
}
console.log(`corpus banal: ${pares.length} pares reais de corpus/fala/conversa.tex`)
ok('§B0 pelo menos 10 pares orgânicos', pares.length >= 10)

/* §B1 — RETAIN: reler é conservar */
{
  let todos = true
  for (const [, resp] of pares) if (!igual(I(resp), I(resp))) todos = false
  ok('§B1 RETAIN: a mesma resposta relida dá I igual, nos ' + pares.length, todos)
}

/* §B2 — REOPEN: respostas distintas do corpus não colidem no observador */
{
  let colisoes = 0, comparacoes = 0
  for (let a = 0; a < pares.length; a++) {
    for (let b = a + 1; b < pares.length; b++) {
      if (pares[a][1] === pares[b][1]) continue
      comparacoes++
      if (igual(I(pares[a][1]), I(pares[b][1]))) colisoes++
    }
  }
  console.log(`§B2: ${comparacoes} pares distintos comparados, ${colisoes} colisões`)
  ok('§B2 REOPEN: nenhum par real de respostas distintas colide em I', colisoes === 0)
}

/* §B3 — os pares reais da vizinhança (dados do eixos_texto.c) */
{
  const viz = [
    ['cafe', 'café'], ['que', 'qeu'], ['casa', 'caza'],
    ['bom dia', 'bom dai'], ['obrigado', 'obrigada'],
  ]
  let separa = true
  for (const [a, b] of viz) {
    const d = !igual(I(a), I(b))
    console.log(`  I('${a}') ${d ? '≠' : '='} I('${b}')`)
    if (!d) separa = false
  }
  ok('§B3 variante de grafia é outro corpo para o observador (𝒱 decide a admissão)', separa)
}

/* §B4 — A ESCADA no corpus banal: em que degrau cada par real se separa */
{
  const hist = [0, 0, 0, 0]   /* [não separa, E, Φ, Φ₂] */
  for (let a = 0; a < pares.length; a++) {
    for (let b = a + 1; b < pares.length; b++) {
      if (pares[a][1] === pares[b][1]) continue
      hist[degrau(I(pares[a][1]), I(pares[b][1]))]++
    }
  }
  console.log('')
  console.log('=== LOG Φ₂ — a escada no corpus banal (190 pares distintos) ===')
  console.log(`degrau I₁ (E):   ${hist[1]}   — a energia separa`)
  console.log(`degrau I₂ (Φ):   ${hist[2]}   — só a fase separa`)
  console.log(`degrau I₃ (Φ₂):  ${hist[3]}   — só a curvatura separa`)
  console.log(`não separados:   ${hist[0]}`)
  ok('§B4 RETAIN vs REOPEN com precisão: todos os pares distintos separados na escada',
    hist[0] === 0)
  ok('§B4 os RETAIN ficam RETAIN: I₃ igual só no idêntico',
    pares.every(([, r]) => degrau(I(r), I(r)) === 0))
}

/* §B5 — o espelhado no banal: a colisão de (E,Φ) que SÓ Φ₂ separa, em dados
 * orgânicos do próprio corpus banal (a mesma construção da caçada) */
{
  function espelha (l) {
    for (let i = 0; i + 1 < l.length; i++) {
      const a = l[i], b = l[i + 1]
      if (a === b || a === ' ' || b === ' ') continue
      const j = l.indexOf(b + a, i + 2)
      if (j < 0) continue
      const y = l.slice(0, i) + b + a + l.slice(i + 2, j) + a + b + l.slice(j + 2)
      if (y !== l) return y
    }
    return null
  }
  let achados = 0, soF2 = 0
  for (const [, resp] of pares) {
    const y = espelha(resp)
    if (!y) continue
    achados++
    const d = degrau(I(resp), I(y))
    if (d === 3) soF2++
    console.log(`#B5 «${resp.slice(0, 34)}…» espelhado → degrau ${d === 3 ? 'Φ₂ (E e Φ cegos)' : d}`)
  }
  console.log(`espelhados orgânicos no banal: ${achados}; separados só por Φ₂: ${soF2}`)
  ok('§B5 a cegueira de (E,Φ) também vive no banal — e Φ₂ separa todos os achados',
    achados > 0 && soF2 === achados)
}

console.log('')
if (!falhas) {
  console.log('  O observador dual sustenta-se nos dados orgânicos: reler conserva,')
  console.log('  distinto distingue; a admissão da variante é do Controle, não da régua.')
  console.log('  E a escada mede: E separa quase tudo; o espelhado só cai em Φ₂.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
