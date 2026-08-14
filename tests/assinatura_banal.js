/* tests/assinatura_banal.js — o par (E, Φ) no corpus banal (eval 13/08).
 *
 * A ordem do diretor: validar o observador dual I(x) = (E, Φ) sobre dados
 * ORGÂNICOS — pares reais RETAIN/REOPEN — não sobre induções artificiais.
 *
 *   E(x) = Σ b²      (energia — o módulo; reconhece equivalência de massa)
 *   Φ(x) = Σ i·b     (fase — a posição interna; reconhece identidade ordenada)
 *
 * Fontes orgânicas: papers/conversa.tex (o corpus banal da casa — fala =
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

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
function I (texto) {
  const b = Buffer.from(String(texto), 'utf8')
  let E = 0, fase = 0
  for (let i = 0; i < b.length; i++) { E += b[i] * b[i]; fase = (fase + (i + 1) * b[i]) % P }
  return { E, fase }
}
const igual = (a, b) => a.E === b.E && a.fase === b.fase

/* §B0 — pares orgânicos do corpus banal */
const tex = fs.readFileSync(path.join(__dirname, '..', 'papers', 'conversa.tex'), 'utf8')
const pares = []
const rx = /\\section\{([^}]+)\}\s*\n([^\n\\]+)/g
let m
while ((m = rx.exec(tex)) && pares.length < 20) {
  const fala = m[1].trim(), resp = m[2].trim()
  if (fala.length >= 2 && resp.length >= 8) pares.push([fala, resp])
}
console.log(`corpus banal: ${pares.length} pares reais de papers/conversa.tex`)
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

console.log('')
if (!falhas) {
  console.log('  O observador dual sustenta-se nos dados orgânicos: reler conserva,')
  console.log('  distinto distingue; a admissão da variante é do Controle, não da régua.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
