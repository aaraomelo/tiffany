/* tests/redes_multifocal_io.js — §RG6 FECHADO. P6 é fechamento (redes.tex).
 *   node tests/redes_multifocal_io.js
 *
 *   G_event ≠ G_focus ≠ G_state (G_event>1 ⇏ G_state>1).
 *   overwrite_single_slot ≠ interferência ≠ Hebb ≠ Hopfield.
 *   Sem RG7, merge, soma, W.
 */
'use strict'
import {
  IO_STATE, PHYS_SLOT, emptyIoExperiment, runIoPipeline, runIoDualWriteX,
  runCaseA, runCaseB, runCaseCMin, runDualSlotSameLogical,
  runCaseSameRep, runCaseDiffRep, ioExperimentReport,
  gRealAggregated, gEvent, gFocus, gStateSurviving, gStateWritten,
  boundaryCombinationOpen, snapshotIo,
} from '../lib/arena_multifocal.mjs'

let falhas = 0, feitas = 0
function ok (q, c) {
  feitas++
  if (!c) falhas++
  console.log(`#UNIT ${c ? 'ok' : 'falha'} ${q}`)
}

const pipe = runIoPipeline(emptyIoExperiment())
const repPipe = ioExperimentReport(pipe, 'pipeline')

ok('§RG6 pipeline: OFF_IN final D', repPipe.offInFinal === 'D')
ok('§RG6 pipeline: F2 leu A antes de escrever B', (() => {
  const ev = pipe.events.find((e) => e.focus === 'F2' && e.op === 'read_in')
  return ev && ev.payload === 'A'
})())
ok('§RG6 pipeline: protocolo G', repPipe.protocoloG)

const dual = runIoDualWriteX(emptyIoExperiment())
const repDual = ioExperimentReport(dual, 'dual-X')

ok('§RG6 dual-X: overwrite observado', repDual.overwrites.length >= 1)
ok('§RG6 dual-X: tipo overwrite_single_slot', repDual.overwrites.every((o) => o.type === 'overwrite_single_slot'))
ok('§RG6 dual-X: OFF_IN final X:F2 (última escrita)', repDual.offInFinal === 'X:F2')
ok('§RG6 dual-X: G_event(X)≥4 (2 write + 2 read)', gEvent(dual.events, IO_STATE.X) >= 4)
ok('§RG6 dual-X: G_focus(X)=4', gFocus(dual.events, IO_STATE.X) === 4)
ok('§RG6 dual-X: G_stateSurviving(X)=1', gStateSurviving(dual, IO_STATE.X) === 1)
ok('§RG6 dual-X: G_stateWritten(X)=2', gStateWritten(dual.events, IO_STATE.X) === 2)
ok('§RG6 dual-X: gRealAggregated conta escrita+leitura', gRealAggregated(dual.journals, IO_STATE.X) >= 4)
ok('§RG6 dual-X: F3 e F4 leem mesmo estado', (() => {
  const r3 = dual.events.find((e) => e.focus === 'F3' && e.op === 'read_in')
  const r4 = dual.events.find((e) => e.focus === 'F4' && e.op === 'read_in')
  return r3 && r4 && r3.payload === r4.payload && r3.payload === 'X:F2'
})())

const caseA = runCaseA(emptyIoExperiment())
const repA = ioExperimentReport(caseA, 'caso-A')
ok('§RG6 caso A: F2 lê X', (() => {
  const r = caseA.events.find((e) => e.focus === 'F2' && e.op === 'read_in')
  return r && r.payload === 'X'
})())
ok('§RG6 caso A: sem overwrite', repA.overwrites.length === 0)
ok('§RG6 caso A: G_event(X)=2 (write+read)', gEvent(caseA.events, IO_STATE.X) === 2)

const caseB = runCaseB(emptyIoExperiment())
const repB = ioExperimentReport(caseB, 'caso-B')
ok('§RG6 caso B: F3 lê Y (estado final)', (() => {
  const r = caseB.events.find((e) => e.focus === 'F3' && e.op === 'read_in')
  return r && r.payload === 'Y'
})())
ok('§RG6 caso B: overwrite X→Y registado', repB.overwrites.length >= 1)

const caseC = runCaseCMin(emptyIoExperiment())
const repC = ioExperimentReport(caseC, 'caso-C-min')
ok('§RG6 caso C: X inicial seed', caseC.audit.xInicial === 'X')
ok('§RG6 caso C: F1 escreve X1', caseC.audit.x1 === 'X1')
ok('§RG6 caso C: F2 escreve X2', caseC.audit.x2 === 'X2')
ok('§RG6 caso C: ordem temporal F1→F2→F3', (() => {
  const steps = caseC.audit.ordem.filter((s) => s.focus).map((s) => s.focus)
  return steps.join(',') === 'F1,F2,F3'
})())
ok('§RG6 caso C: overwrite F1/F2 sobre X', repC.overwrites.length >= 1)
ok('§RG6 caso C: F3 observa X2 (última escrita)', caseC.audit.f3Leu === 'X2')
ok('§RG6 caso C: X1 e X2 não coexistem em OFF_IN', !caseC.audit.x1ConservadoSimultaneamenteComX2 &&
  snapshotIo(caseC.arena).in === 'X2')
ok('§RG6 caso C: G_stateWritten=2, G_stateSurviving=1', gStateWritten(caseC.events, IO_STATE.X) === 2 &&
  gStateSurviving(caseC, IO_STATE.X) === 1)
ok('§RG6 caso C: fronteira combinação ABERTA', boundaryCombinationOpen(caseC))
ok('§RG6 caso C: sem matriz W', typeof globalThis.W === 'undefined')

const dualSlot = runDualSlotSameLogical(emptyIoExperiment())
const repDualSlot = ioExperimentReport(dualSlot, 'dual-slot', { physSlots: [PHYS_SLOT.IN, PHYS_SLOT.OUT] })
ok('§RG6 dual-slot: OFF_IN=X1 e OFF_OUT=X2 simultâneos', repDualSlot.offInFinal === 'X1' && repDualSlot.offOutFinal === 'X2')
ok('§RG6 dual-slot: sem overwrite (slots físicos distintos)', repDualSlot.overwrites.length === 0)
ok('§RG6 dual-slot: G_stateSurviving(X)=2', gStateSurviving(dualSlot, IO_STATE.X, [PHYS_SLOT.IN, PHYS_SLOT.OUT]) === 2)
ok('§RG6 dual-slot: mesma célula lógica, dois payloads', gStateWritten(dualSlot.events, IO_STATE.X) === 2)

const sameRep = runCaseSameRep(emptyIoExperiment())
const diffRep = runCaseDiffRep(emptyIoExperiment())
const repSame = ioExperimentReport(sameRep, 'inverso-sameRep')
const repDiff = ioExperimentReport(diffRep, 'inverso-diffRep')

ok('§RG6 inverso sameRep: overwrite em X', repSame.overwrites.length >= 1)
ok('§RG6 inverso diffRep: overwrite físico OFF_IN (células lógicas distintas)', repDiff.overwrites.length >= 1)
ok('§RG6 inverso diffRep: fronteira fechada (sem dupla escrita na mesma célula lógica)', !boundaryCombinationOpen(diffRep))
ok('§RG6 inverso: sameRep G_focus(X)=4', gFocus(sameRep.events, IO_STATE.X) === 4)
ok('§RG6 inverso: diffRep G_focus(X)=2, G_focus(Y)=2', gFocus(diffRep.events, IO_STATE.X) === 2 &&
  gFocus(diffRep.events, IO_STATE.Y) === 2)
ok('§RG6 inverso: fronteira só em sameRep', boundaryCombinationOpen(sameRep) && !boundaryCombinationOpen(diffRep))

console.log('#RG6 pipeline', JSON.stringify(repPipe))
console.log('#RG6 dual-X', JSON.stringify(repDual))
console.log('#RG6 caso-A', JSON.stringify(repA))
console.log('#RG6 caso-B', JSON.stringify(repB))
console.log('#RG6 caso-C-min', JSON.stringify({ ...repC, audit: caseC.audit }))
console.log('#RG6 dual-slot', JSON.stringify(repDualSlot))
console.log('#RG6 inverso-sameRep', JSON.stringify(repSame))
console.log('#RG6 inverso-diffRep', JSON.stringify(repDiff))
console.log('#RG6 fronteira', JSON.stringify({
  mensagem: boundaryCombinationOpen(caseC)
    ? 'a arquitetura existente determina overwrite, mas não determina uma lei de combinação entre duas incidências concorrentes sobre a mesma representação.'
    : null,
  regraArquitectural: 'OFF_IN é estado mutável de valor único; duas escritas sucessivas produzem overwrite.',
  p6: 'TRAVADA',
}))
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
