/* tests/fusao_conceitos.js — a fusão de conceitos: o primeiro caso REAL
 * (eval 13/08 — a quarentena dizia: «sem objeto, não há física»; o objeto
 * apareceu nos dados).
 *
 * A definição, pelas leis já medidas:
 *   fusão  = soma direta com contorno: z guarda AS DUAS partes intactas
 *            («a dualidade é a memória da divisão» — fundir sem guardar
 *            os dois lados é dissipar);
 *   conservação (a condição de contorno do coordenador):
 *            E(z) = E(x) + E(y) + E_∂ — e o termo de contorno E_∂ é a
 *            MOLDURA, exato e medido por dois caminhos;
 *   fibra  = a divisão dos conceitos: F⁻¹(z) devolve (x,y) byte a byte.
 *
 * §F0  O OBJETO EXISTE: pares singular/plural e títulos duplicados REAIS
 *      no cristal — o primeiro caso não se inventou, encontrou-se
 * §F1  a fusão conserva: E(z) = E(x)+E(y)+E_∂, com E_∂ = E(esqueleto)
 *      — dois caminhos que têm de concordar
 * §F2  a fibra devolve: as partes voltam byte a byte; a escada I₃ de
 *      cada parte devolvida é a do original
 * §F3  no corpus: a fusão é TRANSAÇÃO declarada (2 saem, 1 entra —
 *      R_endereço = 3, contável), e desfazê-la devolve o corpus a R=0
 * §F4  o lote real: os 7 pares singular/plural fundem e voltam, todos
 * §F5  nada se apaga: cada byte dos originais está em z (a memória da
 *      divisão, medida)
 *
 * NOTA DE CURADORIA (13/08, «resolve a curadoria»): a curadoria foi
 * EXECUTADA — tools/cristal_cura.py aplicou 52 fusões à fonte e o medidor
 * é tests/cristal_curadoria.js. Aqui a operação continua a provar-se em
 * memória, sobre um par que a curadoria MANTEVE de propósito (arte/artes:
 * prosa distinta — fundem em memória, não no corpus).
 *
 *   node tests/fusao_conceitos.js
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
const RAIZ = path.join(__dirname, '..')
const linhas = fs.readFileSync(path.join(RAIZ, 'cristal', 'cristal.jsonl'), 'utf8')
  .split('\n').filter(l => l.length)
const regs = new Map()
for (const l of linhas) regs.set(JSON.parse(l).id, l)

const E = s => U.escada(s).E
const I3 = s => { const e = U.escada(s); return e.E + '|' + e.f1 + '|' + e.f2 }

/* ── a fusão: soma direta com contorno; as partes viajam INTACTAS ────────── */
/* a fusão, a moldura e a fibra: as da infraestrutura */
const funde = U.funde
const esqueleto = U.esqueleto
const fibra = U.fibra

/* §F0 — o objeto existe, nos dados */
const paresSP = []
for (const id of regs.keys()) {
  if (regs.has(id + 's')) paresSP.push([id, id + 's'])
}
const porTitulo = new Map()
let titulosDup = 0
for (const [id, l] of regs) {
  const t = (JSON.parse(l).titulo || '').trim().toLowerCase()
  if (!t) continue
  if (porTitulo.has(t)) titulosDup++
  else porTitulo.set(t, id)
}
console.log(`candidatos reais: ${paresSP.length} pares singular/plural · ${titulosDup} títulos duplicados`)
ok('§F0 O OBJETO EXISTE: candidatos reais de fusão no cristal (não inventados)',
  paresSP.length >= 5 && titulosDup >= 30)

/* §F1/§F2 — o caso concreto: arte ↔ artes (par real, mantido na curadoria) */
{
  const lx = regs.get('arte'), ly = regs.get('artes')
  const z = funde('arte', lx, ly)
  const Edz = E(z) - E(lx) - E(ly)
  const Eesq = E(esqueleto('arte'))
  console.log(`E(x)=${E(lx)} E(y)=${E(ly)} E(z)=${E(z)} · E_∂=${Edz} = E(esqueleto)=${Eesq}`)
  ok('§F1 a fusão conserva: E(z) = E(x)+E(y)+E_∂, e E_∂ = E(esqueleto) — dois caminhos',
    Edz === Eesq && Edz > 0)
  const partes = fibra(z)
  ok('§F2 a fibra devolve as partes BYTE A BYTE (a divisão dos conceitos)',
    partes && partes[0] === lx && partes[1] === ly)
  ok('§F2 a escada I₃ de cada parte devolvida é a do original',
    partes && I3(partes[0]) === I3(lx) && I3(partes[1]) === I3(ly))
}

/* §F3 — no corpus: a fusão é transação declarada, e desfaz-se a R=0 */
{
  const rEnd = (fonte, cand) => U.rEndereco(fonte, cand)
  const lx = regs.get('arte'), ly = regs.get('artes')
  const z = funde('arte_fundida', lx, ly)
  const fundido = linhas.filter(l => l !== lx && l !== ly).concat([z])
  const R1 = rEnd(linhas, fundido)
  const partes = fibra(z)
  const desfeito = fundido.filter(l => l !== z).concat(partes)
  const R2 = rEnd(linhas, desfeito)
  console.log(`fusão no corpus: R_endereço=${R1} (2 saem + 1 entra) · desfeita: R=${R2}`)
  ok('§F3 a fusão é TRANSAÇÃO declarada: R_endereço = 3 exato (2 faltantes + 1 excedente)',
    R1 === 3)
  ok('§F3 desfazer pela fibra devolve o corpus a R = 0 — a fusão reverte, não paga',
    R2 === 0)
}

/* §F4 — o lote real: os pares singular/plural todos */
{
  let todos = true
  for (const [a, b] of paresSP) {
    const lx = regs.get(a), ly = regs.get(b)
    const z = funde(a, lx, ly)
    const partes = fibra(z)
    if (!partes || partes[0] !== lx || partes[1] !== ly) todos = false
    if (E(z) - E(lx) - E(ly) !== E(esqueleto(a))) todos = false
  }
  ok(`§F4 o lote real: os ${paresSP.length} pares singular/plural fundem e voltam, com E_∂ exato`,
    todos)
}

/* §F5 — nada se apaga: a memória da divisão */
{
  const lx = regs.get('arte'), ly = regs.get('artes')
  const z = funde('arte', lx, ly)
  ok('§F5 cada byte dos originais está em z, contíguo e intacto — fundir não apaga',
    z.includes(lx) && z.includes(ly))
}

console.log('')
if (!falhas) {
  console.log('  A fusão de conceitos saiu da quarentena PELOS DADOS, e a curadoria')
  console.log('  foi RESOLVIDA (tools/cristal_cura.py, tests/cristal_curadoria.js):')
  console.log('  a fusão conserva com termo de contorno exato, a fibra devolve byte')
  console.log('  a byte, e no corpus ela é transação declarada que reverte a R=0.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
