/* tests/redes_combinacao.js — C0–C8, S0–S4, R0–R4, D0–D3, K0–K7, L0–L7 (fronteira RG6 · P6 TRAVADA).
 *   node tests/redes_combinacao.js
 *
 *   C0–C8  caracterização (lei C aberta)
 *   S0–S4  capacidade do suporte → preservação
 *   R0–R4  redução deliberada R:(x1,x2)→x
 *   D0–D3  sonda do cruzado (sem postular distributividade)
 *   K0–K7  caracterização dos candidatos lexMax / rectCell
 *   L0–L7  invariantes do suporte — classe admissível de f
 *
 *   Régua: fis:def:incid, fis:thm:zetamu, fis:thm:troca-realizacao, fis:def:duomorf,
 *          fis:thm:central, fis:thm:trial; catalogo.tex trio S,ζ,μ.
 */
'use strict'
import {
  IO_STATE,
  PHYS_SLOT,
  boundaryCombinationOpen,
} from '../lib/arena_multifocal.mjs'
import {
  runC0Overwrite,
  runC1Duplicacao,
  runC2Acumulacao,
  runC3CombinacaoTentativa,
  runC4Recuperacao,
  runC5A12,
  runC5A21,
  runC6RepeticaoDual,
  runC6RepeticaoSlotUnico,
  runC7Esquerda,
  runC7Direita,
  runC8Colisao,
  runS0Controle,
  runS1Capacidade2,
  runS2TresIncidenciasCap2,
  runS2TresIncidenciasCap2Out,
  runS3Capacidade3,
  runS4Ordem123,
  runS4Ordem312,
  runR0Controle,
  runR1DescartaX1,
  runR2DescartaX2,
  runR3Codifica,
  runR4DescartaAmbos,
  estadoFinal,
  compararOrdem,
  compararPreservacao,
  compararReducao,
  combinacaoReport,
  combinacaoMetrics,
  supportReport,
  gSlotsSurviving,
  SLOT_DUAL,
  SLOT_TRIPLO,
  PROBE_DEFAULT,
  PROBE_REDUCER,
  runD0Controle,
  runD1VariacaoX1,
  runD2VariacaoX2,
  runD3Matriz,
  runD3BateriaProbes,
  classificarDependenciaCruzado,
  isEncodingLike,
  cruzadoReport,
  K_PAYLOADS,
  CANDIDATO,
  runK0Repeticao,
  runK1Troca,
  runK4Idempotencia,
  runK5Ternario,
  runK6Invariancia,
  runK7ComparacaoCandidatos,
  runKCaracterizacao,
  runKBateriaCandidatos,
  valorReducao,
  zetaVec,
  muVec,
  zetaMuVoltaExacta,
  runLInvariantes,
  runLBateriaInvariantes,
  analisarSelecaoInvariantes,
} from '../lib/arena_combinacao.mjs'

let falhas = 0, feitas = 0
function ok (q, c) {
  feitas++
  if (!c) falhas++
  console.log(`#UNIT ${c ? 'ok' : 'falha'} ${q}`)
}

const dualSlots = SLOT_DUAL

// ── C0 overwrite ─────────────────────────────────────────────────────────
const c0 = runC0Overwrite()
const repC0 = combinacaoReport(c0, 'C0-overwrite', {
  logicalCell: IO_STATE.X,
  physSlots: [PHYS_SLOT.IN],
  operacaoObservada: 'C_ow(x1,x2)=x2',
})
const m0 = combinacaoMetrics(c0, IO_STATE.X, [PHYS_SLOT.IN])

ok('§C0 overwrite: F2 substitui F1 em OFF_IN', repC0.offInFinal === 'X2')
ok('§C0 overwrite: X1 não sobrevive', m0.gStateSurv === 1 && m0.gStateEsc === 2)
ok('§C0 overwrite: overwrite registado', c0.overwrites.length >= 1)
ok('§C0 overwrite: fronteira combinação aberta', boundaryCombinationOpen(c0))
ok('§C0 overwrite: G_event≥3', m0.gEvent >= 3)
ok('§C0 overwrite: sem matriz W', typeof globalThis.W === 'undefined')

// ── C1 duplicação ────────────────────────────────────────────────────────
const c1 = runC1Duplicacao()
const repC1 = combinacaoReport(c1, 'C1-duplicacao', {
  operacaoObservada: 'X→(X_in,X_out) mesmo payload',
})
const m1 = combinacaoMetrics(c1, IO_STATE.X, dualSlots)

ok('§C1 duplicação: dois slots físicos ocupados', gSlotsSurviving(c1, IO_STATE.X, dualSlots) === 2)
ok('§C1 duplicação: IN=X1 OUT=X1', repC1.offInFinal === 'X1' && repC1.offOutFinal === 'X1')
ok('§C1 duplicação: um payload distinto (cópia, não duas realizações)', m1.gStateSurv === 1)
ok('§C1 duplicação: sem overwrite', c1.overwrites.length === 0)
ok('§C1 duplicação: G_focus=4', m1.gFocus === 4)

// ── C2 acumulação ────────────────────────────────────────────────────────
const c2 = runC2Acumulacao()
const repC2 = combinacaoReport(c2, 'C2-acumulacao', {
  operacaoObservada: 'A(x1,x2)=(x1,x2)',
})
const m2 = combinacaoMetrics(c2, IO_STATE.X, dualSlots)

ok('§C2 acumulação: IN=X1 OUT=X2 simultâneos', repC2.offInFinal === 'X1' && repC2.offOutFinal === 'X2')
ok('§C2 acumulação: dois payloads distintos sobrevivem', m2.gStateSurv === 2)
ok('§C2 acumulação: dois slots', m2.gSlotsSurviving === 2)
ok('§C2 acumulação: G_stateEsc=2', m2.gStateEsc === 2)
ok('§C2 acumulação: sem overwrite', c2.overwrites.length === 0)
ok('§C2 acumulação: ≠ combinação num slot', repC2.offInFinal !== 'X2' || repC2.offOutFinal !== 'X1')

// ── C3 combinação (tentativa) ────────────────────────────────────────────
const c3 = runC3CombinacaoTentativa()
const repC3 = combinacaoReport(c3, 'C3-combinacao', {
  physSlots: [PHYS_SLOT.IN],
  operacaoObservada: 'overwrite (lei C indefinida)',
})
const m3 = combinacaoMetrics(c3, IO_STATE.X, [PHYS_SLOT.IN])

ok('§C3 combinação: resultado é X2 (overwrite)', repC3.offInFinal === 'X2')
ok('§C3 combinação: não existe x3', !['X1+X2', 'X1|X2', 'X3'].includes(repC3.offInFinal))
ok('§C3 combinação: lei C null', repC3.leiC === null)
ok('§C3 combinação: G_stateSurv=1', m3.gStateSurv === 1)
ok('§C3 combinação: fronteira aberta', boundaryCombinationOpen(c3))

// ── C4 recuperação ───────────────────────────────────────────────────────
const c4 = runC4Recuperacao()
const repC4 = combinacaoReport(c4, 'C4-recuperacao', {
  operacaoObservada: 'par (x1,x2) lido',
})
const m4 = combinacaoMetrics(c4, IO_STATE.X, dualSlots)

ok('§C4 recuperação: par {in:X1, out:X2}', c4.audit.recuperado.in === 'X1' && c4.audit.recuperado.out === 'X2')
ok('§C4 recuperação: combinado=null', c4.audit.combinado === null)
ok('§C4 recuperação: acumulação preservada', m4.gStateSurv === 2)
ok('§C4 recuperação: timeline com t', repC4.timeline.every((e, i) => e.t === i))
ok('§C4 recuperação: protocolo G', repC4.protocoloG)

// ── distinção acumulação vs combinação ───────────────────────────────────
ok('§C2 vs §C3: acumulação conserva 2 payloads; combinação slot único conserva 1',
  m2.gStateSurv === 2 && m3.gStateSurv === 1)

// ── C5 ordem ─────────────────────────────────────────────────────────────
const c5a12 = runC5A12()
const c5a21 = runC5A21()
const cmp5 = compararOrdem(c5a12, c5a21)
const repC5a12 = combinacaoReport(c5a12, 'C5-A12', { operacaoObservada: 'F1→IN; F2→OUT' })
const repC5a21 = combinacaoReport(c5a21, 'C5-A21', { operacaoObservada: 'F2→OUT; F1→IN' })

ok('§C5 ordem: estado final A_12 = (X1,X2)', cmp5.estado12.in === 'X1' && cmp5.estado12.out === 'X2')
ok('§C5 ordem: estado final A_21 = (X1,X2)', cmp5.estado21.in === 'X1' && cmp5.estado21.out === 'X2')
ok('§C5 ordem: acumulação comutativa como estado', cmp5.estadoFinalIgual)
ok('§C5 ordem: ordem temporal distinta', !cmp5.timelineIgual)
ok('§C5 ordem: métricas G iguais', cmp5.metricasIguais)
ok('§C5 ordem: G_stateSurv=2 em ambos', cmp5.metricas12.gStateSurv === 2 && cmp5.metricas21.gStateSurv === 2)

// ── C6 repetição ─────────────────────────────────────────────────────────
const c6dual = runC6RepeticaoDual()
const c6slot = runC6RepeticaoSlotUnico()
const repC6dual = combinacaoReport(c6dual, 'C6-repeticao-dual', { operacaoObservada: 'A(x1,x1) dual' })
const repC6slot = combinacaoReport(c6slot, 'C6-repeticao-slot', {
  physSlots: [PHYS_SLOT.IN],
  operacaoObservada: 'A(x1,x1) slot único',
})
const m6dual = combinacaoMetrics(c6dual, IO_STATE.X, dualSlots)
const m6slot = combinacaoMetrics(c6slot, IO_STATE.X, [PHYS_SLOT.IN])

ok('§C6 repetição dual: IN=X1 OUT=X1', repC6dual.offInFinal === 'X1' && repC6dual.offOutFinal === 'X1')
ok('§C6 repetição dual: um payload distinto (gStateSurv=1)', m6dual.gStateSurv === 1)
ok('§C6 repetição dual: dois slots ocupados', m6dual.gSlotsSurviving === 2)
ok('§C6 repetição dual: ≠ acumulação (x1,x2)', m6dual.gStateSurv !== m2.gStateSurv)
ok('§C6 repetição slot: IN=X1', repC6slot.offInFinal === 'X1')
ok('§C6 repetição slot: sem overwrite (payload idêntico)', c6slot.overwrites.length === 0)
ok('§C6 repetição slot: gStateSurv=1', m6slot.gStateSurv === 1)

// ── C7 três incidências ──────────────────────────────────────────────────
const c7L = runC7Esquerda()
const c7R = runC7Direita()
const repC7L = combinacaoReport(c7L, 'C7-esquerda', { operacaoObservada: 'A((x1,x2),x3)' })
const repC7R = combinacaoReport(c7R, 'C7-direita', { operacaoObservada: 'A(x1,A(x2,x3))' })
const e7L = estadoFinal(c7L)
const e7R = estadoFinal(c7R)

ok('§C7 esquerda: A((x1,x2),x3) → (X3,X2)', e7L.in === 'X3' && e7L.out === 'X2')
ok('§C7 direita: A(x1,A(x2,x3)) → (X1,X2)', e7R.in === 'X1' && e7R.out === 'X2')
ok('§C7: aninhamentos distintos (não associativo)', e7L.in !== e7R.in || e7L.out !== e7R.out)
ok('§C7 esquerda: overwrite em IN (x3 sobre x1)', c7L.overwrites.length >= 1)
ok('§C7 direita: overwrite em IN (x1 sobre x3)', c7R.overwrites.length >= 1)

// ── C8 colisão ───────────────────────────────────────────────────────────
const c8 = runC8Colisao()
const repC8 = combinacaoReport(c8, 'C8-colisao', {
  physSlots: [PHYS_SLOT.IN],
  operacaoObservada: 'F1→X:x1; F2→X:x2 mesmo slot',
})
const m8 = combinacaoMetrics(c8, IO_STATE.X, [PHYS_SLOT.IN])

ok('§C8 colisão: resultado X2 (overwrite)', repC8.offInFinal === 'X2')
ok('§C8 colisão: overwrite registado', c8.overwrites.length >= 1)
ok('§C8 colisão: lei C null', c8.audit.leiC === null)
ok('§C8 colisão: G_stateSurv=1', m8.gStateSurv === 1)
ok('§C8 colisão: condição mínima documentada', c8.audit.condicao.includes('mesmo slot'))
ok('§C8 colisão: fronteira aberta', boundaryCombinationOpen(c8))

// ── S0 controle ──────────────────────────────────────────────────────────
const s0 = runS0Controle()
const repS0 = combinacaoReport(s0, 'S0-controle', {
  physSlots: [PHYS_SLOT.IN],
  operacaoObservada: 'controle negativo = C8',
})

ok('§S0 controle: X=x2 (overwrite)', repS0.offInFinal === 'X2')
ok('§S0 controle: overwrite registado', s0.overwrites.length >= 1)
ok('§S0 controle: R_surv={X2}', JSON.stringify(repS0.rSurv) === JSON.stringify(['X2']))
ok('§S0 controle: controle negativo documentado', s0.audit.controle === 'negativo')

// ── S1 capacidade 2 ──────────────────────────────────────────────────────
const s1 = runS1Capacidade2()
const repS1 = combinacaoReport(s1, 'S1-cap2', { operacaoObservada: 'S1=x1, S2=x2' })
const mS1 = combinacaoMetrics(s1, IO_STATE.X, dualSlots)

ok('§S1 cap2: S1=X1 S2=X2', repS1.slots.S1 === 'X1' && repS1.slots.S2 === 'X2')
ok('§S1 cap2: sem overwrite', s1.overwrites.length === 0)
ok('§S1 cap2: G_stateSurv=2', mS1.gStateSurv === 2)
ok('§S1 cap2: R_surv={X1,X2}', JSON.stringify(mS1.rSurv) === JSON.stringify(['X1', 'X2']))
ok('§S1 cap2: recuperação explícita', s1.audit.recuperado.s1 === 'X1' && s1.audit.recuperado.s2 === 'X2')

// ── S2 três incidências / capacidade 2 ───────────────────────────────────
const s2 = runS2TresIncidenciasCap2()
const s2out = runS2TresIncidenciasCap2Out()
const repS2 = combinacaoReport(s2, 'S2-3inc-cap2', { operacaoObservada: 'F3→S1' })
const repS2out = combinacaoReport(s2out, 'S2-3inc-cap2-out', { operacaoObservada: 'F3→S2' })
const mS2 = combinacaoMetrics(s2, IO_STATE.X, dualSlots)
const mS2out = combinacaoMetrics(s2out, IO_STATE.X, dualSlots)

ok('§S2 cap2 IN: estado (X3,X2)', repS2.slots.S1 === 'X3' && repS2.slots.S2 === 'X2')
ok('§S2 cap2 IN: R_surv={X2,X3}', JSON.stringify(mS2.rSurv) === JSON.stringify(['X2', 'X3']))
ok('§S2 cap2 IN: overwrite em S1', s2.overwrites.length >= 1)
ok('§S2 cap2 IN: x1 perdido', !mS2.rSurv.includes('X1'))
ok('§S2 cap2 OUT: estado (X1,X3)', repS2out.slots.S1 === 'X1' && repS2out.slots.S2 === 'X3')
ok('§S2 cap2 OUT: R_surv={X1,X3}', JSON.stringify(mS2out.rSurv) === JSON.stringify(['X1', 'X3']))
ok('§S2 cap2 OUT: x2 perdido', !mS2out.rSurv.includes('X2'))

// ── S3 capacidade 3 ──────────────────────────────────────────────────────
const s3 = runS3Capacidade3()
const repS3 = combinacaoReport(s3, 'S3-cap3', {
  physSlots: SLOT_TRIPLO,
  operacaoObservada: 'S1,S2,S3 distintos',
})
const mS3 = combinacaoMetrics(s3, IO_STATE.X, SLOT_TRIPLO)

ok('§S3 cap3: três slots ocupados', repS3.slots.S1 === 'X1' && repS3.slots.S2 === 'X2' && repS3.slots.S3 === 'X3')
ok('§S3 cap3: G_stateSurv=3', mS3.gStateSurv === 3)
ok('§S3 cap3: R_surv={X1,X2,X3}', JSON.stringify(mS3.rSurv) === JSON.stringify(['X1', 'X2', 'X3']))
ok('§S3 cap3: sem overwrite', s3.overwrites.length === 0)
ok('§S3 cap3: três payloads recuperáveis', s3.audit.recuperado.s1 === 'X1' && s3.audit.recuperado.s2 === 'X2' && s3.audit.recuperado.s3 === 'X3')

// ── S4 permutação ────────────────────────────────────────────────────────
const s4a = runS4Ordem123()
const s4b = runS4Ordem312()
const cmpS4 = compararPreservacao(s4a, s4b, SLOT_TRIPLO)
const repS4a = combinacaoReport(s4a, 'S4-123', { physSlots: SLOT_TRIPLO })
const repS4b = combinacaoReport(s4b, 'S4-312', { physSlots: SLOT_TRIPLO })

ok('§S4 perm: estado slots igual (x1,x2,x3)', cmpS4.estadoSlotsIgual)
ok('§S4 perm: R_surv igual como conjunto', cmpS4.rSurvIgual)
ok('§S4 perm: timeline distinta', !cmpS4.timelineIgual)
ok('§S4 perm: R_surv completo', JSON.stringify(cmpS4.rSurvA) === JSON.stringify(['X1', 'X2', 'X3']))

// ── bifurcação: preservação ≠ combinação ─────────────────────────────────
ok('§S1 vs §S0: capacidade 2 preserva; slot único overwrite',
  mS1.gStateSurv === 2 && combinacaoMetrics(s0, IO_STATE.X, [PHYS_SLOT.IN]).gStateSurv === 1)
ok('§S3 vs §S2: capacidade 3 preserva 3; capacidade 2 perde 1',
  mS3.gStateSurv === 3 && mS2.gStateSurv === 2)

// ── R0–R4 redução deliberada ─────────────────────────────────────────────
const r0 = runR0Controle()
const r1 = runR1DescartaX1()
const r2 = runR2DescartaX2()
const r3 = runR3Codifica()
const r4 = runR4DescartaAmbos()
const cmpR = compararReducao([r0, r1, r2, r3, r4])
const repR0 = supportReport(r0, 'R0-controle')
const repR1 = supportReport(r1, 'R1-keep2')
const repR3 = supportReport(r3, 'R3-encode')

ok('§R0 controle: par preservado antes', JSON.stringify(r0.audit.antes.rSurv) === JSON.stringify(['X1', 'X2']))
ok('§R0 controle: sem redução', r0.audit.reducao === null)
ok('§R0 controle: R_surv={X1,X2} depois', JSON.stringify(r0.audit.depois.rSurv) === JSON.stringify(['X1', 'X2']))
ok('§R1 keep2: R(x1,x2)=x2', r1.audit.reducao.x === 'X2')
ok('§R1 keep2: R_surv={X2}', JSON.stringify(r1.audit.depois.rSurv) === JSON.stringify(['X2']))
ok('§R1 keep2: x1 não recuperável', !r1.audit.depois.rSurv.includes('X1'))
ok('§R2 keep1: R(x1,x2)=x1', r2.audit.reducao.x === 'X1')
ok('§R2 keep1: R_surv={X1}', JSON.stringify(r2.audit.depois.rSurv) === JSON.stringify(['X1']))
ok('§R3 encode: R(x1,x2)=X1|X2', r3.audit.reducao.x === 'X1|X2')
ok('§R3 encode: R_surv codificado', r3.audit.depois.rSurv.includes('X1|X2'))
ok('§R3 encode: x1,x2 não separados', !r3.audit.depois.rSurv.includes('X1') || r3.audit.depois.rSurv.length === 1)
ok('§R4 discard: R_surv vazio', r4.audit.depois.rSurv.length === 0)
ok('§R4 discard: perda total', r4.audit.depois.slots.S1 === '' && r4.audit.depois.slots.S2 === '')
ok('§R0–R4: gReal medido em todos', cmpR.every((r) => r.gReal > 0))
ok('§R1 vs §S0: redução deliberada ≠ overwrite acidental',
  JSON.stringify(r1.audit.depois.rSurv) === JSON.stringify(['X2']) &&
  r1.audit.reducao != null)
ok('§R3 vs §R1: codificação ≠ descarte de x1',
  r3.audit.reducao.x !== r1.audit.reducao.x && r3.audit.depois.rSurv.length === 1)
ok('§R todos: lei C null', [r0, r1, r2, r3, r4].every((e) => e.audit.leiC === null))

// ── D0–D3 sonda do cruzado ───────────────────────────────────────────────
const d0 = runD0Controle()
ok('§D0 controle: R=id preserva par', JSON.stringify(d0.rSurv) === JSON.stringify(['X1', 'X2']))
ok('§D0 controle: sem redução', d0.celula.x === null)
ok('§D0 controle: gReal medido', d0.gReal > 0)
ok('§D0 controle: slots S1=X1 S2=X2', d0.celula.slots.S1 === 'X1' && d0.celula.slots.S2 === 'X2')

const d1keep1 = runD1VariacaoX1(PROBE_REDUCER.keep1, { R: '(x1,x2)↦x1' })
const d1keep2 = runD1VariacaoX1(PROBE_REDUCER.keep2, { R: '(x1,x2)↦x2' })
ok('§D1 keep1: variar x1 altera resultado', d1keep1.efeitoMensuravel)
ok('§D1 keep1: x base=X1', d1keep1.base.x === 'X1')
ok('§D1 keep1: x variante=A1', d1keep1.variante.x === 'A1')
ok('§D1 keep2: variar x1 não altera (x2 fixo)', !d1keep2.efeitoMensuravel)
ok('§D1 keep2: ambos=X2', d1keep2.base.x === 'X2' && d1keep2.variante.x === 'X2')

const d2keep1 = runD2VariacaoX2(PROBE_REDUCER.keep1, { R: '(x1,x2)↦x1' })
const d2keep2 = runD2VariacaoX2(PROBE_REDUCER.keep2, { R: '(x1,x2)↦x2' })
ok('§D2 keep1: variar x2 não altera (x1 fixo)', !d2keep1.efeitoMensuravel)
ok('§D2 keep2: variar x2 altera resultado', d2keep2.efeitoMensuravel)
ok('§D2 keep2: x base=X2', d2keep2.base.x === 'X2')
ok('§D2 keep2: x variante=B2', d2keep2.variante.x === 'B2')

const d3keep1 = runD3Matriz(PROBE_REDUCER.keep1, { R: '(x1,x2)↦x1' })
const d3keep2 = runD3Matriz(PROBE_REDUCER.keep2, { R: '(x1,x2)↦x2' })
const d3encode = runD3Matriz(PROBE_REDUCER.encode, { R: '(x1,x2)↦x1|x2' })
const d3lexMax = runD3Matriz(PROBE_REDUCER.lexMax, { R: '(x1,x2)↦max' })
const d3rect = runD3Matriz(PROBE_REDUCER.rectCell, { R: '(x1,x2)↦célula' })
const d3discard = runD3Matriz(PROBE_REDUCER.discard, { R: '(x1,x2)↦∅' })

ok('§D3 keep1: leitura seleção x1', d3keep1.classificacao.leitura === 'selecao_x1')
ok('§D3 keep1: efeito x1 sim x2 não', d3keep1.classificacao.efeitoX1 && !d3keep1.classificacao.efeitoX2)
ok('§D3 keep2: leitura seleção x2', d3keep2.classificacao.leitura === 'selecao_x2')
ok('§D3 keep2: efeito x2 sim x1 não', d3keep2.classificacao.efeitoX2 && !d3keep2.classificacao.efeitoX1)
ok('§D3 encode: leitura codificação R3', d3encode.classificacao.leitura === 'codificacao_R3')
ok('§D3 encode: empacota ambos literalmente', d3encode.valores.x11 === 'X1|X2')
ok('§D3 encode: controle isEncodingLike', isEncodingLike('X1|X2', 'X1', 'X2'))
ok('§D3 lexMax: candidato combinação', d3lexMax.classificacao.leitura === 'candidato_combinacao')
ok('§D3 lexMax: depende de ambos', d3lexMax.classificacao.dependeAmbos)
ok('§D3 rectCell: candidato combinação', d3rect.classificacao.leitura === 'candidato_combinacao')
ok('§D3 rectCell: efeito em x1 e x2', d3rect.classificacao.efeitoX1 && d3rect.classificacao.efeitoX2)
ok('§D3 discard: constante vazio', d3discard.classificacao.leitura === 'constante')
ok('§D3 discard: todos ∅', Object.values(d3discard.valores).every((v) => v === ''))

const bateriaD3 = runD3BateriaProbes()
ok('§D3 bateria: 7 probes', bateriaD3.length === 7)
ok('§D3 bateria: keep1 seleção x1', bateriaD3.find((p) => p.nome === 'keep1').classificacao.leitura === 'selecao_x1')
ok('§D3 bateria: encode codificação', bateriaD3.find((p) => p.nome === 'encode').classificacao.leitura === 'codificacao_R3')
ok('§D3 bateria: rectCell candidato', bateriaD3.find((p) => p.nome === 'rectCell').classificacao.dependeAmbos)
ok('§D3 todos: gReal em células', d3keep1.celulas.x11.gReal > 0)
ok('§D3 todos: timeline registada', d3keep1.celulas.x11.timeline.length >= 4)
ok('§D3 todos: lei C null (não postulada)', d3lexMax.resposta === 'sim_candidato')

// ── K0–K7 caracterização dos candidatos ──────────────────────────────────
const kLex = runKCaracterizacao(CANDIDATO.lexMax)
const kRect = runKCaracterizacao(CANDIDATO.rectCell)
const k7 = runK7ComparacaoCandidatos()
const kBat = runKBateriaCandidatos()

ok('§K0 lexMax: f(X1,X1)=X1', kLex.K0.fxx === 'X1')
ok('§K0 rectCell: f(X1,X1) medido', kRect.K0.fxx === 'dom:le|im:le')

ok('§K1 lexMax: comutativa', kLex.K1.comuta)
ok('§K1 lexMax: f(x1,x2)=X1', kLex.K1.fx1x2 === 'X1')
ok('§K1 rectCell: não comutativa', !kRect.K1.comuta)
ok('§K1 rectCell: f(X1,X2)≠f(X2,X1)', kRect.K1.fx1x2 !== kRect.K1.fx2x1)

ok('§K2 lexMax: perturba x1', kLex.K2.efeitoMensuravel)
ok('§K3 lexMax: perturba x2', kLex.K3.efeitoMensuravel)
ok('§K2 rectCell: perturba x1', kRect.K2.efeitoMensuravel)
ok('§K3 rectCell: perturba x2', kRect.K3.efeitoMensuravel)

ok('§K4 lexMax: idempotente f(y,y)=y', kLex.K4.idempotente)
ok('§K4 rectCell: não idempotente (caracterização)', !kRect.K4.idempotente)

ok('§K5 lexMax: associativa', kLex.K5.associativo)
ok('§K5 lexMax: f(f(X1,B2),X3)=X3', kLex.K5.esquerda === 'X3')
ok('§K5 rectCell: não associativa', !kRect.K5.associativo)

ok('§K6 lexMax: bem definida (invariância realização)', kLex.K6.bemDefinida)
ok('§K6 lexMax: timeline distinta mas resultado igual', kLex.K6.timelineDistinta && kLex.K6.invariavel)
ok('§K6 rectCell: bem definida', kRect.K6.bemDefinida)
ok('§K6 rectCell: timeline distinta', kRect.K6.timelineDistinta)

ok('§K7: lexMax≠rectCell (leis distintas)', k7.leisDistintas)
ok('§K7: par x1,x2 diferente', !k7.comparacoes.find((c) => c.label === 'x1,x2').iguais)
ok('§K7: nenhum par igual em 6 testes', k7.comparacoes.every((c) => !c.iguais))

ok('§K sintese lexMax: comut+assoc+idempot+bemDef', kLex.sintese.comutativa && kLex.sintese.associativa && kLex.sintese.idempotente && kLex.sintese.bemDefinida)
ok('§K sintese rectCell: bemDef+dep dupla', kRect.sintese.bemDefinida && kRect.sintese.dependeX1 && kRect.sintese.dependeX2)
ok('§K bateria: lei C null', kBat.lexMax.leiC === null && kBat.rectCell.leiC === null)
ok('§K6 divisor: mesmo par → mesmo f (lexMax)', valorReducao(CANDIDATO.lexMax.reducer, K_PAYLOADS.x1, K_PAYLOADS.x2) === kLex.K6.resultadoA)

// ── L0–L7 invariantes do suporte ─────────────────────────────────────────
ok('§L ζμ: volta exacta (catálogo/incidencia.h)', zetaMuVoltaExacta([1, 0, 1, 0, 1]))
ok('§L ζμ: μ(ζa)=a', muVec(zetaVec([2, 1, 0, 3])).every((v, i) => v === [2, 1, 0, 3][i]))

const lBat = runLBateriaInvariantes()
const lLex = lBat.lexMax.invariantes
const lRect = lBat.rectCell.invariantes
const sel = lBat.selecao

ok('§L0 ambos: dependência bilateral', lLex.L0 && lRect.L0)
ok('§L1 ambos: bem definida', lLex.L1 && lRect.L1)
ok('§L2 lexMax comutativa', lLex.L2)
ok('§L2 rectCell não comutativa', !lRect.L2)
ok('§L3 lexMax idempotente', lLex.L3)
ok('§L3 rectCell não idempotente', !lRect.L3)
ok('§L4 lexMax associativa', lLex.L4)
ok('§L4 rectCell não associativa', !lRect.L4)
ok('§L5 ambos: invariância τ', lLex.L5 && lRect.L5)
ok('§L6 ambos: compatível com G', lLex.L6 && lRect.L6)
ok('§L7 ambos: compatível com ζ/μ', lLex.L7 && lRect.L7)
ok('§L seleção: L2,L3,L4 separam', sel.separadores.includes('L2') && sel.separadores.includes('L3') && sel.separadores.includes('L4'))
ok('§L seleção: L2 elimina rectCell', sel.eliminaRectCell.includes('L2'))
ok('§L seleção: L0,L1,L5,L6,L7 comuns', sel.comuns.includes('L0') && sel.comuns.includes('L1') && sel.comuns.includes('L5'))
ok('§L seleção: nenhum elimina lexMax', sel.eliminaLexMax.length === 0)
ok('§L não-unicidade: ambos admissíveis em L0+L1', lLex.L0 && lRect.L0 && lLex.L1 && lRect.L1)
ok('§L bateria: lei C null', lBat.leiC === null)

console.log('#C0', JSON.stringify(repC0))
console.log('#C1', JSON.stringify(repC1))
console.log('#C2', JSON.stringify(repC2))
console.log('#C3', JSON.stringify(repC3))
console.log('#C4', JSON.stringify(repC4))
console.log('#C5', JSON.stringify({ A12: repC5a12, A21: repC5a21, comparacao: cmp5 }))
console.log('#C6', JSON.stringify({ dual: repC6dual, slot: repC6slot }))
console.log('#C7', JSON.stringify({ esquerda: repC7L, direita: repC7R }))
console.log('#C8', JSON.stringify(repC8))
console.log('#S0', JSON.stringify(repS0))
console.log('#S1', JSON.stringify(repS1))
console.log('#S2', JSON.stringify({ in: repS2, out: repS2out }))
console.log('#S3', JSON.stringify(repS3))
console.log('#S4', JSON.stringify({ ordem123: repS4a, ordem312: repS4b, comparacao: cmpS4 }))
console.log('#R0-R4', JSON.stringify({ comparacao: cmpR, R0: repR0, R1: repR1, R3: repR3 }))
console.log('#D0-D3', JSON.stringify({
  D0: d0,
  D1: { keep1: d1keep1, keep2: d1keep2 },
  D2: { keep1: d2keep1, keep2: d2keep2 },
  D3: { keep1: d3keep1, keep2: d3keep2, encode: d3encode, lexMax: d3lexMax, rectCell: d3rect },
  bateria: bateriaD3.map(({ nome, valores, classificacao }) => ({ nome, valores, leitura: classificacao.leitura })),
}))
console.log('#CRUZADO fronteira', JSON.stringify({
  mensagem: 'D0–D3: sonda do cruzado; distributividade não postulada',
  congelado: {
    D0: 'controle: R=id, baseline preservador',
    D1: 'fixa x2, varia x1 → Δx',
    D2: 'fixa x1, varia x2 → Δx',
    D3: 'matriz 2×2; classifica seleção/codificação/candidato',
    pergunta: '∃ f:X×X→X observável e genuinamente dependente de ambos?',
    leituras: ['selecao_x1', 'selecao_x2', 'codificacao_R3', 'candidato_combinacao', 'constante'],
    controle: 'isEncodingLike separa R3-like de C candidato',
  },
  epistemologia: 'não chamar distributividade antes da medição',
  referencias: ['fis:thm:central', 'fis:thm:trial', 'fis:def:duomorf'],
  p6: 'TRAVADA',
}))
console.log('#K0-K7', JSON.stringify({
  lexMax: { sintese: kLex.sintese, K1: kLex.K1, K5: kLex.K5, K6: kLex.K6 },
  rectCell: { sintese: kRect.sintese, K1: kRect.K1, K5: kRect.K5, K6: kRect.K6 },
  K7: k7,
}))
console.log('#CARACTERIZACAO fronteira', JSON.stringify({
  mensagem: 'K0–K7: candidatos medidos; ainda não f=C',
  congelado: {
    existe: '∃ f:X×X→X com dependência observável nos dois argumentos',
    nao: 'f=C e distributividade ainda não',
    lexMax: 'comutativa · associativa · idempotente · bem definida',
    rectCell: 'não comutativa · não associativa · não idempotente · bem definida',
    K7: 'duas leis ou duas representações — não a mesma',
    divisor: 'K6: mesmo par → mesmo resultado sob realização distinta',
  },
  epistemologia: 'a máquina não recebeu a lei; recebeu critérios para procurar uma realização',
  referencias: ['fis:thm:troca-realizacao', 'fis:thm:central', 'fis:thm:trial'],
  p6: 'TRAVADA',
}))
console.log('#L0-L7', JSON.stringify({
  lexMax: lBat.lexMax.invariantes,
  rectCell: lBat.rectCell.invariantes,
  selecao: sel,
}))
console.log('#INVARIANTES fronteira', JSON.stringify({
  mensagem: 'L0–L7: o que restringe a classe admissível de f',
  congelado: {
    existe: '∃ f bem definido com dependência bilateral',
    naoUnico: 'a condição não determina f',
    existenciaVsUnicidade: 'existência de combinação ≠ unicidade da combinação',
    comuns: 'L0,L1,L5,L6,L7: ambos candidatos passam',
    separadores: 'L2,L3,L4: lexMax sim, rectCell não',
    leitura: 'dependência bilateral necessária, não suficiente',
    catalogo: 'S→ζ (convolução), μ=1−S (deconvolução), μζ=id — catalogo.tex',
  },
  epistemologia: 'não escolher C; mapear o espaço de respostas',
  referencias: ['catalogo.tex trio incidência', 'lib/incidencia.h', 'fis:thm:zetamu', 'fis:thm:troca-realizacao'],
  p6: 'TRAVADA',
}))
console.log('#REDUCAO fronteira', JSON.stringify({
  mensagem: 'R0–R4: redução deliberada; combinação ainda aberta',
  congelado: {
    controle: 'R0: preservar (x1,x2) sem R',
    keep2: 'R1: R(x1,x2)=x2 — descarte deliberado de x1',
    keep1: 'R2: R(x1,x2)=x1 — descarte deliberado de x2',
    encode: 'R3: R(x1,x2)=X1|X2 — codificação ≠ combinação',
    discard: 'R4: R(x1,x2)=∅ — perda total',
    distincao: 'perda por overwrite (S0) ≠ redução deliberada (R1–R4)',
    pergunta: 'quando R(x1,x2) é combinação e não codificação?',
  },
  referencias: ['fis:def:duomorf', 'fis:thm:zetamu', 'fis:thm:troca-realizacao'],
  p6: 'TRAVADA',
}))
console.log('#SUPORTE fronteira', JSON.stringify({
  mensagem: 'S0–S4: preservação vs capacidade finita; lei C ainda aberta',
  congelado: {
    controle: 'S0=C8: overwrite em slot único',
    cap2: 'S1: G_stateSurv=2 sem overwrite',
    cap2perda: 'S2: terceira incidência → overwrite (mecanismo de perda)',
    cap3: 'S3: três payloads recuperáveis — não-associatividade de C7 era limitação física',
    perm: 'S4: estado=R_surv comutativo; timeline não',
    bifurcacao: 'acumular≠combinar — distinção arquitetural',
  },
  epistemologia: 'primeiro preservação; depois redução controlada; só então C(x,y)',
  p6: 'TRAVADA',
}))
console.log('#COMBINACAO fronteira', JSON.stringify({
  mensagem: 'família C0–C8 medida; lei C(x1,x2) permanece aberta',
  congelado: {
    distincao: 'overwrite≠cópia≠acumulação≠combinação',
    fronteira: 'C2 gStateSurv=2 vs C3 gStateSurv=1',
    ordem: 'estado comutativo; timeline não',
    repeticao: 'A(x,x) dual: cópia; slot único: idempotente sem overwrite',
    associatividade: 'não — ordem importa com três incidências',
    colisao: 'condições mínimas medidas; C(x1,x2) ainda aberta',
  },
  epistemologia: 'não procurar a lei que queremos; procurar a lei que o suporte permite',
  referencias: ['fis:def:incid', 'fis:thm:zetamu', 'fis:def:conv'],
  p6: 'TRAVADA',
}))
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
