/* tests/redes_multifocal.js — §RG4/§RG5: quatro focos, uma arena, um X.
 *   node tests/redes_multifocal.js
 *
 *   Não cria W, Hebb, energia nem descida.
 */
'use strict'
import {
  FOCUS_IDS, FOCUS_TAG, STATE_CELL, ISOLATED_CELL,
  emptyMultifocalJournals, gRealAggregated,
  focusesDisjoint, verifyMultifocalProtocol, runConversationCycle,
  createSharedBrainArena, runIsolatedFoci, multifocalReport, focusTouch,
} from '../lib/arena_multifocal.mjs'

let falhas = 0, feitas = 0
function ok (q, c) {
  feitas++
  if (!c) falhas++
  console.log(`#UNIT ${c ? 'ok' : 'falha'} ${q}`)
}

ok('§RG4 uma arena para quatro focos', (() => {
  const arena = createSharedBrainArena()
  const journals = emptyMultifocalJournals()
  focusTouch(arena, journals, 'F1', FOCUS_TAG.F1)
  focusTouch(arena, journals, 'F2', FOCUS_TAG.F2)
  return FOCUS_IDS.every((id) => Array.isArray(journals[id])) &&
    journals.F1.length === 1 && journals.F2.length === 1
})())

const isoArena = createSharedBrainArena()
const isoJ = emptyMultifocalJournals()
runIsolatedFoci(isoArena, isoJ)

ok('§RG4 isolado: focos disjuntos', focusesDisjoint(isoJ))

ok('§RG4 isolado: G_real(x)=1 por célula privada', (() => {
  for (const id of FOCUS_IDS) {
    const c = ISOLATED_CELL[id]
    if (gRealAggregated(isoJ, c) !== 1) return false
  }
  return true
})())

ok('§RG4 isolado: protocolo multifocal', verifyMultifocalProtocol(isoArena, isoJ))

const convArena = createSharedBrainArena()
const convJ = emptyMultifocalJournals()
runConversationCycle(convArena, convJ)

ok('§RG4 conversação: 8 eventos (2 por foco)', (() => {
  const c = multifocalReport(convArena, convJ, 'conv')
  return c.totalEventos === 8 && FOCUS_IDS.every((id) => c.eventos[id] === 2)
})())

ok('§RG4 conversação: ciclo F1→F2→F3→F4→F1', (() => {
  return convJ.F1[0] === STATE_CELL.A &&
    convJ.F1[1] === STATE_CELL.D &&
    convJ.F2[0] === STATE_CELL.A &&
    convJ.F2[1] === STATE_CELL.B &&
    convJ.F3[0] === STATE_CELL.B &&
    convJ.F3[1] === STATE_CELL.C &&
    convJ.F4[0] === STATE_CELL.C &&
    convJ.F4[1] === STATE_CELL.D
})())

ok('§RG4 partilhado: G_real(A)>1', gRealAggregated(convJ, STATE_CELL.A) >= 2)

ok('§RG4 partilhado: G_real≥2 em A,B,C,D', (() => {
  for (const c of Object.values(STATE_CELL)) {
    if (gRealAggregated(convJ, c) < 2) return false
  }
  return true
})())

ok('§RG4 partilhado: focos NÃO disjuntos', !focusesDisjoint(convJ))

ok('§RG4 partilhado: protocolo multifocal', verifyMultifocalProtocol(convArena, convJ))

ok('§RG4 G_real>1 ⇏ Hopfield', (() => {
  return gRealAggregated(convJ, STATE_CELL.A) > 1 &&
    typeof globalThis.W === 'undefined'
})())

ok('§RG5 conversação ≠ interferência Hebb', (() => {
  const r = multifocalReport(convArena, convJ, 'rg5')
  return r.celulasPartilhadas.some((x) => x.gReal >= 2) &&
    typeof globalThis.W === 'undefined' &&
    r.protocolo
})())

ok('§RG5 compartilhar X ≠ somar representações', (() => {
  return gRealAggregated(convJ, STATE_CELL.A) >= 2 &&
    !focusesDisjoint(convJ) &&
    verifyMultifocalProtocol(convArena, convJ)
})())

const repIso = multifocalReport(isoArena, isoJ, 'isolado')
const repConv = multifocalReport(convArena, convJ, 'partilhado')

console.log('#RG4 isolado', JSON.stringify(repIso))
console.log('#RG4 partilhado', JSON.stringify(repConv))
console.log('#RG4 max G_real isolado', Math.max(...FOCUS_IDS.map((id) =>
  gRealAggregated(isoJ, ISOLATED_CELL[id]))))
console.log('#RG4 max G_real partilhado', Math.max(...Object.values(STATE_CELL).map((c) =>
  gRealAggregated(convJ, c))))
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
