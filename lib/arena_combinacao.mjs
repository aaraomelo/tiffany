/* arena_combinacao.mjs — família experimental C0–C8, S0–S4, R0–R4, D0–D3 (fronteira RG6).
 *
 * Régua teórica (fisica.tex, sem abrir P6):
 *   fis:def:incid        — W = Zc^I, matriz S subdiagonal; zeta/mu
 *   fis:thm:zetamu       — escrita = convolução; recuperação = μ
 *   fis:thm:troca-realizacao — ∂²=id ⇒ prof invariável; u(Da,Db)=u(a,b)
 *   fis:def:duomorf      — directo (ζ) vs cruzado (μ); perda ≠ composição
 *   fis:thm:central      — rectângulo I×X; Gentil/Hurwitz duais (contagem dom/im)
 *   fis:thm:trial        — distributividade = lado cruzado (não medir antes de D3)
 *   V = Zc^X             — campo G como vetor de coordenadas
 *
 * S0–S4: capacidade do suporte determina preservação (perda por overwrite ≠ lei).
 * R0–R4: redução deliberada R:(x1,x2)→x com capacidade ≥ 2 (combinação ainda aberta).
 * D0–D3: sonda do cruzado — pergunta primitiva antes de postular C ou distributividade.
 * K0–K7: caracterização dos candidatos (lexMax, rectCell) — ainda não f=C.
 * L0–L7: invariantes do suporte — o que restringe a classe admissível de f.
 * M1–M2: autoridade da ordem lex sobre A_min (isolada de ∂; não promove lexMax).
 * E_∂: covariância sob a dobra (isolada da ordem lex; não promove lexMax).
 */

import {
  IO_STATE,
  PHYS_SLOT,
  FOCUS_TAG,
  emptyIoExperiment,
  focusWritePhys,
  focusReadPhys,
  focusWriteIn,
  focusReadIn,
  writeInBuffer,
  snapshotIo,
  gMetrics,
  gStateWritten,
  gStateSurviving,
  gEvent,
  gFocus,
  gRealAggregated,
  verifyMultifocalProtocol,
} from './arena_multifocal.mjs'
import { incG, OFF_IN, CAP, getG } from './arena_disco.mjs'

/** Terceiro slot físico (região livre entre OFF_IN+CAP e OFF_OUT). */
export const OFF_S3 = OFF_IN + CAP
export const OFF_NS3 = 24584
export const PHYS_SLOT_AUX = 'aux'
export const SLOT_TRIPLO = [PHYS_SLOT.IN, PHYS_SLOT.OUT, PHYS_SLOT_AUX]
export const SLOT_DUAL = [PHYS_SLOT.IN, PHYS_SLOT.OUT]

function readAuxBuffer (arena) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  const n = a[OFF_NS3] + a[OFF_NS3 + 1] * 256
  return a.toString('utf8', OFF_S3, OFF_S3 + Math.min(n, CAP))
}

function writeAuxBuffer (arena, text) {
  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)
  const body = Buffer.from(String(text ?? ''), 'utf8')
  const n = Math.min(body.length, CAP)
  body.copy(a, OFF_S3, 0, n)
  a[OFF_NS3] = n & 255
  a[OFF_NS3 + 1] = (n >> 8) & 255
  return a
}

export function snapshotSupport (arena) {
  const io = snapshotIo(arena)
  return { ...io, aux: readAuxBuffer(arena), naux: arena[OFF_NS3] + arena[OFF_NS3 + 1] * 256 }
}

function slotValue (snap, physSlot) {
  if (physSlot === PHYS_SLOT.OUT) return snap.out
  if (physSlot === PHYS_SLOT_AUX) return snap.aux
  return snap.in
}

function recordSupportEvent (exp, focus, op, logicalCell, extra = {}) {
  const f = String(focus)
  const cell = Number(logicalCell) & 255
  if (!exp.journals[f]) exp.journals[f] = []
  incG(exp.arena, cell)
  incG(exp.arena, FOCUS_TAG[f])
  exp.journals[f].push(cell)
  exp.journals[f].push(FOCUS_TAG[f])
  exp.events.push({ focus: f, op, logicalCell: cell, ...extra })
  return exp
}

function noteOverwriteLocal (exp, focus, logicalCell, physSlot, prior, payload) {
  if (prior.length > 0 && prior !== payload) {
    exp.overwrites.push({
      type: 'overwrite_single_slot',
      logicalCell: Number(logicalCell) & 255,
      physSlot,
      focus,
      prior,
      new: payload,
    })
  }
}

export function focusWriteAux (exp, focus, logicalCell, payload) {
  const before = snapshotSupport(exp.arena)
  const prior = before.aux
  writeAuxBuffer(exp.arena, payload)
  const after = snapshotSupport(exp.arena)
  noteOverwriteLocal(exp, focus, logicalCell, PHYS_SLOT_AUX, prior, payload)
  recordSupportEvent(exp, focus, 'write_aux', logicalCell, { before, after, payload, physSlot: PHYS_SLOT_AUX })
  return after
}

export function focusReadAux (exp, focus, logicalCell) {
  const before = snapshotSupport(exp.arena)
  recordSupportEvent(exp, focus, 'read_aux', logicalCell, {
    before,
    after: before,
    payload: before.aux,
    physSlot: PHYS_SLOT_AUX,
  })
  return before.aux
}

export function focusWriteSlot (exp, focus, logicalCell, physSlot, payload) {
  if (physSlot === PHYS_SLOT_AUX) return focusWriteAux(exp, focus, logicalCell, payload)
  return focusWritePhys(exp, focus, logicalCell, physSlot, payload)
}

export function focusReadSlot (exp, focus, logicalCell, physSlot) {
  if (physSlot === PHYS_SLOT_AUX) return focusReadAux(exp, focus, logicalCell)
  return focusReadPhys(exp, focus, logicalCell, physSlot)
}

/** R_surv(X): payloads ainda recuperáveis nos slots do suporte. */
export function rSurv (exp, _logicalCell, physSlots = SLOT_DUAL) {
  const snap = snapshotSupport(exp.arena)
  const out = []
  for (const p of physSlots) {
    const v = slotValue(snap, p)
    if (v.length > 0 && !out.includes(v)) out.push(v)
  }
  return out.sort()
}

export function rSurvMultiset (payloads) {
  return [...payloads].sort().join(',')
}

/** Slots físicos não vazios ligados à célula lógica (≠ payloads distintos). */
export function gSlotsSurviving (exp, _logicalCell, physSlots = SLOT_DUAL) {
  const snap = snapshotSupport(exp.arena)
  let n = 0
  for (const p of physSlots) {
    if (slotValue(snap, p).length > 0) n++
  }
  return n
}

export function gStateSurvSupport (exp, logicalCell, physSlots = SLOT_DUAL) {
  return rSurv(exp, logicalCell, physSlots).length
}

export function combinacaoMetrics (exp, logicalCell, physSlots = SLOT_DUAL) {
  const baseSlots = physSlots.filter((p) => p !== PHYS_SLOT_AUX)
  const m = gMetrics(exp, logicalCell, baseSlots.length ? baseSlots : SLOT_DUAL)
  return {
    ...m,
    gReal: gRealAggregated(exp.journals, logicalCell),
    gSlotsSurviving: gSlotsSurviving(exp, logicalCell, physSlots),
    gStateEsc: gStateWritten(exp.events, logicalCell),
    gStateSurv: gStateSurvSupport(exp, logicalCell, physSlots),
    rSurv: rSurv(exp, logicalCell, physSlots),
  }
}

export function supportReport (exp, label, opts = {}) {
  const rep = combinacaoReport(exp, label, opts)
  const cell = opts.logicalCell ?? IO_STATE.X
  const physSlots = opts.physSlots ?? SLOT_DUAL
  return {
    ...rep,
    gReal: gRealAggregated(exp.journals, cell),
    rSurv: rSurv(exp, cell, physSlots),
  }
}

/** Linha do tempo com índice t (ordem dos eventos). */
export function combinacaoReport (exp, label, opts = {}) {
  const cell = opts.logicalCell ?? IO_STATE.X
  const physSlots = opts.physSlots ?? SLOT_DUAL
  const m = combinacaoMetrics(exp, cell, physSlots)
  const snap = snapshotSupport(exp.arena)
  return {
    label,
    protocolo: label,
    cell,
    /** Candidato observado — não postula lei algébrica. */
    operacaoObservada: opts.operacaoObservada ?? null,
    timeline: exp.events.map((e, t) => ({
      t,
      focus: e.focus,
      op: e.op,
      logicalCell: e.logicalCell,
      physSlot: e.physSlot ?? PHYS_SLOT.IN,
      payload: e.payload ?? null,
    })),
    offInFinal: snap.in,
    offOutFinal: snap.out,
    offAuxFinal: snap.aux,
    slots: {
      S1: snap.in,
      S2: snap.out,
      S3: snap.aux,
    },
    rSurv: m.rSurv,
    metricas: m,
    overwrites: exp.overwrites,
    audit: exp.audit,
    protocoloG: verifyMultifocalProtocol(exp.arena, exp.journals),
    p6: 'TRAVADA',
    /** fis:thm:zetamu — recuperação por diferença finita reservada para C4+. */
    leiC: null,
  }
}

/** C0 — overwrite medido: C_ow(x1,x2) = x2 (slot único). */
export function runC0Overwrite (exp = emptyIoExperiment()) {
  writeInBuffer(exp.arena, 'X0')
  focusWriteIn(exp, 'F1', IO_STATE.X, 'X1')
  focusWriteIn(exp, 'F2', IO_STATE.X, 'X2')
  focusReadIn(exp, 'F3', IO_STATE.X)
  exp.audit = {
    x0: 'X0',
    x1: 'X1',
    x2: 'X2',
    sobrevive: snapshotIo(exp.arena).in,
    operacao: 'C_ow(x1,x2)=x2',
    referencia: 'fis:def:incid — uma coordenada por slot; segunda escrita substitui',
  }
  return exp
}

/** C1 — duplicação: mesmo payload em dois suportes físicos. */
export function runC1Duplicacao (exp = emptyIoExperiment()) {
  focusWritePhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWritePhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X1')
  focusReadPhys(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN)
  focusReadPhys(exp, 'F4', IO_STATE.X, PHYS_SLOT.OUT)
  exp.audit = {
    payload: 'X1',
    pergunta: 'cópia ou segunda realização?',
    slots: gSlotsSurviving(exp, IO_STATE.X),
    payloadsDistintos: gStateSurviving(exp, IO_STATE.X, [PHYS_SLOT.IN, PHYS_SLOT.OUT]),
  }
  return exp
}

/** C2 — acumulação: A(x1,x2)=(x1,x2) em suportes distintos (não substituir). */
export function runC2Acumulacao (exp = emptyIoExperiment()) {
  focusWritePhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWritePhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  focusReadPhys(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN)
  focusReadPhys(exp, 'F4', IO_STATE.X, PHYS_SLOT.OUT)
  exp.audit = {
    par: { in: 'X1', out: 'X2' },
    operacao: 'A(x1,x2)=(x1,x2)',
    distintoDe: 'C(x1,x2)=x3',
    referencia: 'V=Zc^X — duas coordenadas físicas, mesma célula lógica',
  }
  return exp
}

/** C3 — tentativa de combinação num slot único: observa overwrite, não x3. */
export function runC3CombinacaoTentativa (exp = emptyIoExperiment()) {
  focusWriteIn(exp, 'F1', IO_STATE.X, 'X1')
  focusWriteIn(exp, 'F2', IO_STATE.X, 'X2')
  focusReadIn(exp, 'F3', IO_STATE.X)
  exp.audit = {
    tentativa: 'C(X1,X2)=?',
    resultado: snapshotIo(exp.arena).in,
    overwrite: exp.overwrites.length >= 1,
    leiC: null,
    nota: 'sem operador +; slot único não produz x3',
  }
  return exp
}

/** C4 — recuperação: ler par acumulado (mu sobre par, não fusão). */
export function runC4Recuperacao (exp = emptyIoExperiment()) {
  runC2Acumulacao(exp)
  const x1 = focusReadPhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN)
  const x2 = focusReadPhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT)
  exp.audit = {
    ...exp.audit,
    recuperado: { in: x1, out: x2 },
    combinado: null,
    referencia: 'fis:thm:zetamu — leitura do par; mu reservada para série finita',
  }
  return exp
}

export function estadoFinal (exp) {
  const s = snapshotIo(exp.arena)
  return { in: s.in, out: s.out }
}

export function timelineOps (exp) {
  return exp.events.map((e) => `${e.focus}:${e.op}:${e.physSlot ?? PHYS_SLOT.IN}:${e.payload}`)
}

/** C5 — ordem: A_12 vs A_21 (mesmos focos, slots e payloads). */
export function runC5A12 (exp = emptyIoExperiment()) {
  focusWritePhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWritePhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  exp.audit = { ordem: 'A_12', sequencia: 'F1→IN(X1); F2→OUT(X2)' }
  return exp
}

export function runC5A21 (exp = emptyIoExperiment()) {
  focusWritePhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  focusWritePhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  exp.audit = { ordem: 'A_21', sequencia: 'F2→OUT(X2); F1→IN(X1)' }
  return exp
}

/** C6 — repetição: (x1,x1) dual vs (x1,x2) vs slot único. */
export function runC6RepeticaoDual (exp = emptyIoExperiment()) {
  focusWritePhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWritePhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X1')
  exp.audit = { par: '(x1,x1)', pergunta: 'A(x,x)=?' }
  return exp
}

export function runC6RepeticaoSlotUnico (exp = emptyIoExperiment()) {
  focusWriteIn(exp, 'F1', IO_STATE.X, 'X1')
  focusWriteIn(exp, 'F2', IO_STATE.X, 'X1')
  exp.audit = { par: '(x1,x1) slot único', pergunta: 'A(x,x)=?' }
  return exp
}

/** C7 — três incidências: A((x1,x2),x3) vs A(x1,A(x2,x3)) no suporte dual. */
export function runC7Esquerda (exp = emptyIoExperiment()) {
  focusWritePhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWritePhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  focusWritePhys(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN, 'X3')
  exp.audit = {
    aninhamento: 'A((x1,x2),x3)',
    interpretacao: 'par (X1,X2) depois x3 em IN',
  }
  return exp
}

export function runC7Direita (exp = emptyIoExperiment()) {
  focusWritePhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  focusWritePhys(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN, 'X3')
  focusWritePhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  exp.audit = {
    aninhamento: 'A(x1,A(x2,x3))',
    interpretacao: 'par (X3,X2) depois x1 em IN',
  }
  return exp
}

/** C8 — colisão mínima: duas fontes, mesmo slot físico, payloads distintos. */
export function runC8Colisao (exp = emptyIoExperiment()) {
  focusWriteIn(exp, 'F1', IO_STATE.X, 'X1')
  focusWriteIn(exp, 'F2', IO_STATE.X, 'X2')
  exp.audit = {
    colisao: { F1: 'X1', F2: 'X2', slot: 'OFF_IN', logical: IO_STATE.X },
    pergunta: 'C(x1,x2)=?',
    resultado: snapshotIo(exp.arena).in,
    leiC: null,
    condicao: 'duas fontes · mesma célula · mesmo slot · payloads distintos',
  }
  return exp
}

export function compararOrdem (exp12, exp21, cell = IO_STATE.X, physSlots = [PHYS_SLOT.IN, PHYS_SLOT.OUT]) {
  const e12 = estadoFinal(exp12)
  const e21 = estadoFinal(exp21)
  const m12 = combinacaoMetrics(exp12, cell, physSlots)
  const m21 = combinacaoMetrics(exp21, cell, physSlots)
  return {
    estadoFinalIgual: e12.in === e21.in && e12.out === e21.out,
    timelineIgual: timelineOps(exp12).join('|') === timelineOps(exp21).join('|'),
    metricasIguais:
      m12.gEvent === m21.gEvent &&
      m12.gFocus === m21.gFocus &&
      m12.gStateEsc === m21.gStateEsc &&
      m12.gStateSurv === m21.gStateSurv,
    estado12: e12,
    estado21: e21,
    metricas12: m12,
    metricas21: m21,
  }
}

// ── S0–S4: suporte e preservação (sem definir C) ─────────────────────────

/** S0 — controle negativo: reproduz C8 (mesmo slot, overwrite). */
export function runS0Controle (exp = emptyIoExperiment()) {
  runC8Colisao(exp)
  exp.audit = {
    ...exp.audit,
    protocolo: 'S0-controle',
    pergunta: 'duas incidências · slot único',
    esperado: 'X=x2',
    controle: 'negativo',
  }
  return exp
}

/** S1 — capacidade 2: concorrência como problema de suporte, não acumulação. */
export function runS1Capacidade2 (exp = emptyIoExperiment()) {
  focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  const rec = {
    s1: focusReadSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN),
    s2: focusReadSlot(exp, 'F4', IO_STATE.X, PHYS_SLOT.OUT),
  }
  exp.audit = {
    protocolo: 'S1-capacidade-2',
    slots: { S1: 'X1', S2: 'X2' },
    recuperado: rec,
    rSurv: rSurv(exp, IO_STATE.X, SLOT_DUAL),
    pergunta: 'preservação simultânea sem overwrite',
    gStateSurv: gStateSurvSupport(exp, IO_STATE.X, SLOT_DUAL),
  }
  return exp
}

/** S2 — 3 incidências, capacidade 2: observa mecanismo de perda. */
export function runS2TresIncidenciasCap2 (exp = emptyIoExperiment()) {
  focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  focusWriteSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN, 'X3')
  const snap = snapshotSupport(exp.arena)
  exp.audit = {
    protocolo: 'S2-3inc-cap2',
    capacidade: 2,
    sequencia: 'F1→S1(X1); F2→S2(X2); F3→S1(X3)',
    estadoFinal: { S1: snap.in, S2: snap.out },
    rSurv: rSurv(exp, IO_STATE.X, SLOT_DUAL),
    mecanismoPerda: 'overwrite em S1: x1 substituído por x3',
    pergunta: 'capacidade=2, terceira incidência → ?',
  }
  return exp
}

/** S2b — variante: terceira incidência em S2. */
export function runS2TresIncidenciasCap2Out (exp = emptyIoExperiment()) {
  focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  focusWriteSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT.OUT, 'X3')
  const snap = snapshotSupport(exp.arena)
  exp.audit = {
    protocolo: 'S2-3inc-cap2-out',
    capacidade: 2,
    sequencia: 'F1→S1(X1); F2→S2(X2); F3→S2(X3)',
    estadoFinal: { S1: snap.in, S2: snap.out },
    rSurv: rSurv(exp, IO_STATE.X, SLOT_DUAL),
    mecanismoPerda: 'overwrite em S2: x2 substituído por x3',
  }
  return exp
}

/** S3 — capacidade 3: três slots, três incidências preservadas. */
export function runS3Capacidade3 (exp = emptyIoExperiment()) {
  focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  focusWriteSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT_AUX, 'X3')
  const rec = {
    s1: focusReadSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN),
    s2: focusReadSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT),
    s3: focusReadSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT_AUX),
  }
  exp.audit = {
    protocolo: 'S3-capacidade-3',
    capacidade: 3,
    slots: { S1: 'X1', S2: 'X2', S3: 'X3' },
    recuperado: rec,
    rSurv: rSurv(exp, IO_STATE.X, SLOT_TRIPLO),
    gStateSurv: gStateSurvSupport(exp, IO_STATE.X, SLOT_TRIPLO),
    pergunta: 'não-associatividade de C7 era limitação do suporte?',
  }
  return exp
}

/** S4 — ordem (x1,x2,x3). */
export function runS4Ordem123 (exp = emptyIoExperiment()) {
  focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  focusWriteSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT_AUX, 'X3')
  exp.audit = { ordem: '(x1,x2,x3)', sequencia: 'F1→S1; F2→S2; F3→S3' }
  return exp
}

/** S4 — ordem permutada (x3,x1,x2). */
export function runS4Ordem312 (exp = emptyIoExperiment()) {
  focusWriteSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT_AUX, 'X3')
  focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  exp.audit = { ordem: '(x3,x1,x2)', sequencia: 'F3→S3; F1→S1; F2→S2' }
  return exp
}

export function compararPreservacao (expA, expB, physSlots = SLOT_TRIPLO) {
  const snapA = snapshotSupport(expA.arena)
  const snapB = snapshotSupport(expB.arena)
  const slotsA = Object.fromEntries(physSlots.map((p, i) => [`S${i + 1}`, slotValue(snapA, p)]))
  const slotsB = Object.fromEntries(physSlots.map((p, i) => [`S${i + 1}`, slotValue(snapB, p)]))
  const rA = rSurv(expA, IO_STATE.X, physSlots)
  const rB = rSurv(expB, IO_STATE.X, physSlots)
  return {
    estadoSlotsIgual: JSON.stringify(slotsA) === JSON.stringify(slotsB),
    rSurvIgual: rSurvMultiset(rA) === rSurvMultiset(rB),
    timelineIgual: timelineOps(expA).join('|') === timelineOps(expB).join('|'),
    slotsA,
    slotsB,
    rSurvA: rA,
    rSurvB: rB,
  }
}

// ── R0–R4: redução deliberada R:(x1,x2)→x (capacidade ≥ 2) ───────────────

/** Candidatos observáveis — não postulam lei C. */
export const REDUCER = {
  /** controle: sem redução */
  noop: null,
  /** descarta x1 */
  keep2: (x1, x2) => x2,
  /** descarta x2 */
  keep1: (x1, _x2) => x1,
  /** codificação explícita (≠ combinação algébrica) */
  encode: (x1, x2) => `${x1}|${x2}`,
  /** descarta ambos */
  discard: (_x1, _x2) => '',
}

function fasePreservacaoPar (exp) {
  focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')
  focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')
  return {
    slots: {
      S1: snapshotSupport(exp.arena).in,
      S2: snapshotSupport(exp.arena).out,
    },
    rSurv: rSurv(exp, IO_STATE.X, SLOT_DUAL),
  }
}

/**
 * Fase 2: lê par preservado, aplica R, escreve x em S1; opcionalmente limpa S2.
 * fis:def:duomorf — redução explícita separa directo (preservar) de cruzado (fundir).
 */
export function aplicarReducao (exp, reducer, opts = {}) {
  const x1 = focusReadSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN)
  const x2 = focusReadSlot(exp, 'F4', IO_STATE.X, PHYS_SLOT.OUT)
  const x = reducer(x1, x2)
  focusWriteSlot(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN, x)
  if (opts.limpaS2) focusWriteSlot(exp, 'F4', IO_STATE.X, PHYS_SLOT.OUT, '')
  return { par: { x1, x2 }, x, limpaS2: !!opts.limpaS2 }
}

export function runReducao (reducer, meta, opts = {}) {
  const exp = emptyIoExperiment()
  const antes = fasePreservacaoPar(exp)
  let reducao = null
  if (reducer != null) {
    reducao = aplicarReducao(exp, reducer, opts)
  }
  const depois = {
    slots: {
      S1: snapshotSupport(exp.arena).in,
      S2: snapshotSupport(exp.arena).out,
    },
    rSurv: rSurv(exp, IO_STATE.X, SLOT_DUAL),
  }
  exp.audit = {
    protocolo: meta.protocolo,
    R: meta.R,
    operacaoObservada: meta.operacaoObservada ?? null,
    pergunta: meta.pergunta ?? 'R(x1,x2) combinação ou codificação?',
    leiC: null,
    capacidade: 2,
    antes,
    reducao,
    depois,
    referencia: meta.referencia ?? 'fis:def:duomorf — redução ≠ perda por capacidade (S2/S3)',
  }
  return exp
}

/** R0 — controle: preservar (x1,x2), sem redução. */
export function runR0Controle () {
  return runReducao(REDUCER.noop, {
    protocolo: 'R0-controle',
    R: 'id',
    operacaoObservada: 'preservar (x1,x2)',
    pergunta: 'baseline antes de R',
  })
}

/** R1 — R(x1,x2)=x2 (descarta x1). */
export function runR1DescartaX1 () {
  return runReducao(REDUCER.keep2, {
    protocolo: 'R1-keep2',
    R: '(x1,x2)↦x2',
    operacaoObservada: 'descarta x1',
    referencia: 'perda deliberada — não confundir com overwrite acidental (S0)',
  }, { limpaS2: true })
}

/** R2 — R(x1,x2)=x1 (descarta x2). */
export function runR2DescartaX2 () {
  return runReducao(REDUCER.keep1, {
    protocolo: 'R2-keep1',
    R: '(x1,x2)↦x1',
    operacaoObservada: 'descarta x2',
  }, { limpaS2: true })
}

/** R3 — R(x1,x2)=encode(x1,x2) (codificação, não C). */
export function runR3Codifica () {
  return runReducao(REDUCER.encode, {
    protocolo: 'R3-encode',
    R: '(x1,x2)↦x1|x2',
    operacaoObservada: 'codificação explícita',
    pergunta: 'codificação ≠ combinação algébrica',
    referencia: 'fis:thm:zetamu — ζ acumula; μ recupera; aqui R funde por concatenação',
  }, { limpaS2: true })
}

/** R4 — R(x1,x2)=∅ (descarta ambos). */
export function runR4DescartaAmbos () {
  return runReducao(REDUCER.discard, {
    protocolo: 'R4-discard',
    R: '(x1,x2)↦∅',
    operacaoObservada: 'descarta ambos',
    referencia: 'limite inferior — perda total deliberada',
  }, { limpaS2: true })
}

export function compararReducao (experiments) {
  return experiments.map((exp) => {
    const m = combinacaoMetrics(exp, IO_STATE.X, SLOT_DUAL)
    return {
      protocolo: exp.audit.protocolo,
      R: exp.audit.R,
      rSurv: m.rSurv,
      gReal: m.gReal,
      gEvent: m.gEvent,
      gFocus: m.gFocus,
      gStateSurv: m.gStateSurv,
      gStateEsc: m.gStateEsc,
      slots: exp.audit.depois?.slots ?? exp.audit.antes?.slots,
      x: exp.audit.reducao?.x ?? null,
    }
  })
}

// ── D0–D3: sonda do cruzado (sem postular C nem distributividade) ─────────

/** Payloads padrão da matriz 2×2. */
export const PROBE_DEFAULT = {
  x1: 'X1',
  x2: 'X2',
  x1p: 'A1',
  x2p: 'B2',
}

/** Candidatos R_θ observáveis — classificação vem dos dados, não do nome. */
export const PROBE_REDUCER = {
  ...REDUCER,
  /** max lex — depende de ambos quando x1≠x2 */
  lexMax: (x1, x2) => (x1 >= x2 ? x1 : x2),
  lexMin: (x1, x2) => (x1 <= x2 ? x1 : x2),
  /** analogia discreta fis:thm:central — rectângulo completo, não só a célula */
  rectCell: (x1, x2) => encodeRectCell(x1, x2),
}

/**
 * Rectângulo I×X de π (fis:def:objeto + fis:thm:central).
 *
 * π: I → X, «o índice i ocupa a célula π(i)». I é a lista de índices
 * (ordem da lista, thm:BI), não o payload. X é finito e totalmente ordenado.
 * Se X omite-se, X = im π (π sobrejectiva — fis:thm:tresgraus).
 *
 * Metade 1 — cada (i,x) ∈ I×X: x≤π(i) ou π(i)<x, exactamente uma.
 * Metade 2 — Σ_dom + Σ_im = |I|·|X| em TODAS as células, vazias incluídas.
 * Dual: 𝔐 = |I|−|supp G| (folga); ℰ = Σ G² (grau 2). Não é C.
 */
export function retanguloPi (pi, X = null) {
  const cells = [...pi].map((v) => String(v ?? ''))
  const nI = cells.length
  const I = cells.map((_, i) => i)
  const supp = [...new Set(cells)].sort((u, v) => (u === v ? 0 : u < v ? -1 : 1))
  const Xord = X == null
    ? supp
    : [...X].map((v) => String(v ?? ''))
  const nX = Xord.length
  let sDom = 0
  for (const i of I) {
    const fi = cells[i]
    for (const x of Xord) {
      if (x <= fi) sDom++
    }
  }
  let sIm = 0
  for (const x of Xord) {
    for (const i of I) {
      if (cells[i] < x) sIm++
    }
  }
  const G = Xord.map((x) => cells.filter((fi) => fi === x).length)
  const nSupp = supp.length
  const area = nI * nX
  const massa = nI - nSupp
  const energia = G.reduce((s, g) => s + g * g, 0)
  const a = cells[0] ?? ''
  const b = cells[1] ?? cells[0] ?? ''
  const celula =
    (a <= b ? 'dom:le' : 'dom:gt') + '|' + (b <= a ? 'im:le' : 'im:gt')
  return {
    celula,
    sDom,
    sIm,
    area,
    fecha: sDom + sIm === area,
    G,
    massa,
    energia,
    nI,
    nX,
    nSupp,
    sobrejectiva: nSupp === nX,
    vazio: massa === 0,
    I,
    X: Xord,
    pi: cells,
    referencia: 'fis:def:objeto π:I→X; fis:thm:central; fis:thm:tresgraus',
  }
}

/** Par (x1,x2) como π com |I|=2. X omisso = im π (ocupado, sobrejectivo). */
export function retanguloCentral (x1, x2, X = null) {
  return retanguloPi([x1, x2], X)
}

/** Codifica as duas metades no caso ocupado (π sobrejectiva em im π). */
export function encodeRectCell (x1, x2) {
  const r = retanguloCentral(x1, x2)
  return `${r.celula}#${r.sDom}+${r.sIm}=${r.area}#m${r.massa}e${r.energia}`
}

function fasePreservacaoParCustom (exp, x1, x2) {
  focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, x1)
  focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, x2)
  return {
    par: { x1, x2 },
    slots: {
      S1: snapshotSupport(exp.arena).in,
      S2: snapshotSupport(exp.arena).out,
    },
    rSurv: rSurv(exp, IO_STATE.X, SLOT_DUAL),
  }
}

function cruzadoDepois (exp) {
  const m = combinacaoMetrics(exp, IO_STATE.X, SLOT_DUAL)
  const snap = snapshotSupport(exp.arena)
  return {
    slots: { S1: snap.in, S2: snap.out },
    rSurv: m.rSurv,
    metricas: {
      gReal: m.gReal,
      gEvent: m.gEvent,
      gFocus: m.gFocus,
      gStateEsc: m.gStateEsc,
      gStateSurv: m.gStateSurv,
    },
    timeline: exp.events.map((e, t) => ({
      t,
      focus: e.focus,
      op: e.op,
      physSlot: e.physSlot ?? PHYS_SLOT.IN,
      payload: e.payload ?? null,
    })),
  }
}

/** Uma célula: preservar par, aplicar R_θ, registar métricas completas. */
export function runCruzadoCell (reducer, x1, x2, meta = {}, opts = {}) {
  const exp = emptyIoExperiment()
  const antes = fasePreservacaoParCustom(exp, x1, x2)
  let reducao = null
  if (reducer != null) {
    reducao = aplicarReducao(exp, reducer, { limpaS2: opts.limpaS2 !== false })
  }
  const depois = cruzadoDepois(exp)
  exp.audit = {
    protocolo: meta.protocolo ?? 'D-celula',
    celula: meta.celula ?? null,
    R: meta.R ?? null,
    x1,
    x2,
    leiC: null,
    capacidade: 2,
    antes,
    reducao,
    depois,
    referencia: meta.referencia ?? 'fis:thm:trial — cruzado medido; distributividade não postulada',
    pergunta: meta.pergunta ?? '∃ f:X×X→X dependente de ambos?',
  }
  return exp
}

export function cruzadoReport (exp) {
  const rep = supportReport(exp, exp.audit.protocolo ?? 'D-cruzado')
  return {
    ...rep,
    x1: exp.audit.x1,
    x2: exp.audit.x2,
    x: exp.audit.reducao?.x ?? null,
    R: exp.audit.R,
    antes: exp.audit.antes,
    depois: exp.audit.depois,
  }
}

/** Controle de codificação: resultado carrega x1 e x2 como campos concatenados. */
export function isEncodingLike (x, x1, x2) {
  if (x == null || x === '') return false
  const pipe = `${x1}|${x2}`
  const pipeRev = `${x2}|${x1}`
  if (x === pipe || x === pipeRev) return true
  if (x.includes(x1) && x.includes(x2) && (x === x1 + x2 || x === x2 + x1)) return true
  return false
}

/**
 * Classifica dependência observada na matriz 2×2.
 * Não afirma distributividade — só separa seleção, codificação e candidato.
 */
export function classificarDependenciaCruzado (valores, payloads) {
  const { x11, x12, x21, x22 } = valores
  const { x1, x2, x1p, x2p } = payloads
  const efeitoX1 = x11 !== x21 || x12 !== x22
  const efeitoX2 = x11 !== x12 || x21 !== x22
  const encoding11 = isEncodingLike(x11, x1, x2)
  const encoding12 = isEncodingLike(x12, x1, x2p)
  const encoding21 = isEncodingLike(x21, x1p, x2)
  const encoding22 = isEncodingLike(x22, x1p, x2p)
  const encoding = encoding11 && encoding12 && encoding21 && encoding22
  const empacotado = [x11, x12, x21, x22].every((v, i) => {
    const a = i % 2 === 0 ? x1 : x1p
    const b = i < 2 ? x2 : x2p
    return v != null && v.includes(a) && v.includes(b)
  })
  let leitura
  if (!efeitoX1 && !efeitoX2) leitura = 'constante'
  else if (efeitoX1 && !efeitoX2) leitura = 'selecao_x1'
  else if (!efeitoX1 && efeitoX2) leitura = 'selecao_x2'
  else if (encoding) leitura = 'codificacao_R3'
  else if (empacotado && !encoding) leitura = 'codificacao'
  else if (efeitoX1 && efeitoX2) leitura = 'candidato_combinacao'
  else leitura = 'indefinido'
  return {
    efeitoX1,
    efeitoX2,
    efeitoMensuravelX1: efeitoX1,
    efeitoMensuravelX2: efeitoX2,
    dependeAmbos: efeitoX1 && efeitoX2 && !encoding,
    encoding,
    empacotado,
    leitura,
    matriz: valores,
  }
}

function valorCelula (exp) {
  return exp.audit.reducao?.x ?? null
}

/** D0 — controle: suporte preservador, R_θ = identidade. */
export function runD0Controle (payloads = PROBE_DEFAULT) {
  const exp = runCruzadoCell(null, payloads.x1, payloads.x2, {
    protocolo: 'D0-controle',
    R: 'id',
    celula: 'baseline',
    operacaoObservada: 'preservar (x1,x2) sem R',
    referencia: 'fis:thm:central — baseline antes da sonda cruzada',
  })
  return {
    protocolo: 'D0-controle',
    R: 'id',
    payloads,
    celula: cruzadoReport(exp),
    rSurv: exp.audit.depois.rSurv,
    gReal: exp.audit.depois.metricas.gReal,
  }
}

/** D1 — fixa x2, varia x1 → mede Δx. */
export function runD1VariacaoX1 (reducer, meta, payloads = PROBE_DEFAULT, opts = {}) {
  const base = runCruzadoCell(reducer, payloads.x1, payloads.x2, {
    protocolo: 'D1-base',
    R: meta.R,
    celula: 'x11',
  }, opts)
  const variante = runCruzadoCell(reducer, payloads.x1p, payloads.x2, {
    protocolo: 'D1-variante',
    R: meta.R,
    celula: 'x21',
  }, opts)
  const xBase = valorCelula(base)
  const xVar = valorCelula(variante)
  return {
    protocolo: 'D1-var-x1',
    R: meta.R,
    fixo: { x2: payloads.x2 },
    base: { x1: payloads.x1, x: xBase, report: cruzadoReport(base) },
    variante: { x1: payloads.x1p, x: xVar, report: cruzadoReport(variante) },
    deltaX: xBase !== xVar,
    efeitoMensuravel: xBase !== xVar,
  }
}

/** D2 — fixa x1, varia x2 → mede Δx. */
export function runD2VariacaoX2 (reducer, meta, payloads = PROBE_DEFAULT, opts = {}) {
  const base = runCruzadoCell(reducer, payloads.x1, payloads.x2, {
    protocolo: 'D2-base',
    R: meta.R,
    celula: 'x11',
  }, opts)
  const variante = runCruzadoCell(reducer, payloads.x1, payloads.x2p, {
    protocolo: 'D2-variante',
    R: meta.R,
    celula: 'x12',
  }, opts)
  const xBase = valorCelula(base)
  const xVar = valorCelula(variante)
  return {
    protocolo: 'D2-var-x2',
    R: meta.R,
    fixo: { x1: payloads.x1 },
    base: { x2: payloads.x2, x: xBase, report: cruzadoReport(base) },
    variante: { x2: payloads.x2p, x: xVar, report: cruzadoReport(variante) },
    deltaX: xBase !== xVar,
    efeitoMensuravel: xBase !== xVar,
  }
}

/** D3 — matriz 2×2 completa + classificação (sem postular lei). */
export function runD3Matriz (reducer, meta, payloads = PROBE_DEFAULT, opts = {}) {
  const e11 = runCruzadoCell(reducer, payloads.x1, payloads.x2, { protocolo: 'D3', R: meta.R, celula: 'x11' }, opts)
  const e12 = runCruzadoCell(reducer, payloads.x1, payloads.x2p, { protocolo: 'D3', R: meta.R, celula: 'x12' }, opts)
  const e21 = runCruzadoCell(reducer, payloads.x1p, payloads.x2, { protocolo: 'D3', R: meta.R, celula: 'x21' }, opts)
  const e22 = runCruzadoCell(reducer, payloads.x1p, payloads.x2p, { protocolo: 'D3', R: meta.R, celula: 'x22' }, opts)
  const valores = {
    x11: valorCelula(e11),
    x12: valorCelula(e12),
    x21: valorCelula(e21),
    x22: valorCelula(e22),
  }
  const classificacao = classificarDependenciaCruzado(valores, payloads)
  const celulas = {
    x11: cruzadoReport(e11),
    x12: cruzadoReport(e12),
    x21: cruzadoReport(e21),
    x22: cruzadoReport(e22),
  }
  return {
    protocolo: 'D3-matriz',
    R: meta.R,
    payloads,
    valores,
    classificacao,
    celulas,
    pergunta: '∃ f:X×X→X observável e genuinamente dependente de ambos?',
    resposta: classificacao.dependeAmbos ? 'sim_candidato' : 'nao_ou_outra_leitura',
    referencia: 'fis:thm:central — rectângulo 2×2; fis:thm:trial — cruzado, não distributividade a priori',
  }
}

/** Bateria D3 sobre todos os candidatos PROBE_REDUCER nomeados. */
export function runD3BateriaProbes (payloads = PROBE_DEFAULT) {
  const catalogo = [
    { nome: 'keep1', reducer: PROBE_REDUCER.keep1, R: '(x1,x2)↦x1' },
    { nome: 'keep2', reducer: PROBE_REDUCER.keep2, R: '(x1,x2)↦x2' },
    { nome: 'encode', reducer: PROBE_REDUCER.encode, R: '(x1,x2)↦x1|x2' },
    { nome: 'discard', reducer: PROBE_REDUCER.discard, R: '(x1,x2)↦∅' },
    { nome: 'lexMax', reducer: PROBE_REDUCER.lexMax, R: '(x1,x2)↦max(x1,x2)' },
    { nome: 'lexMin', reducer: PROBE_REDUCER.lexMin, R: '(x1,x2)↦min(x1,x2)' },
    { nome: 'rectCell', reducer: PROBE_REDUCER.rectCell, R: '(x1,x2)↦rectângulo I×X' },
  ]
  return catalogo.map(({ nome, reducer, R }) => ({
    nome,
    ...runD3Matriz(reducer, { R, nome }),
  }))
}

// ── K0–K7: caracterização dos candidatos (lexMax, rectCell) ───────────────

/** Payloads estendidos para K5 (ternário) e perturbações que alteram f. */
export const K_PAYLOADS = {
  x1: 'X1',
  x2: 'B2',
  x1p: 'A1',
  x2p: 'X2',
  x3: 'X3',
}

/** Candidatos D3 que passaram o filtro de dependência dupla. */
export const CANDIDATO = {
  lexMax: { nome: 'lexMax', reducer: PROBE_REDUCER.lexMax, R: '(x1,x2)↦max(x1,x2)' },
  rectCell: { nome: 'rectCell', reducer: PROBE_REDUCER.rectCell, R: '(x1,x2)↦rectângulo I×X' },
}

/** Valor observado f(x1,x2) numa célula isolada. */
export function valorReducao (reducer, x1, x2, meta = {}) {
  return valorCelula(runCruzadoCell(reducer, x1, x2, { protocolo: 'K-valor', ...meta }))
}

/** Preservação + redução com ordem de escrita permutada (K6). */
export function runCruzadoCellOrdem (reducer, x1, x2, ordem = 'F1F2', meta = {}) {
  const exp = emptyIoExperiment()
  if (ordem === 'F2F1') {
    focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, x2)
    focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, x1)
  } else {
    focusWriteSlot(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, x1)
    focusWriteSlot(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, x2)
  }
  const antes = {
    par: { x1, x2 },
    ordem,
    slots: {
      S1: snapshotSupport(exp.arena).in,
      S2: snapshotSupport(exp.arena).out,
    },
    rSurv: rSurv(exp, IO_STATE.X, SLOT_DUAL),
  }
  const reducao = aplicarReducao(exp, reducer, { limpaS2: true })
  const depois = cruzadoDepois(exp)
  exp.audit = {
    protocolo: meta.protocolo ?? 'K-ordem',
    ordem,
    x1,
    x2,
    R: meta.R ?? null,
    leiC: null,
    antes,
    reducao,
    depois,
    referencia: 'fis:thm:troca-realizacao — mesmo par, realização distinta',
  }
  return exp
}

/** K0 — repetição f(x,x). */
export function runK0Repeticao (reducer, meta, x = 'X1') {
  const exp = runCruzadoCell(reducer, x, x, { protocolo: 'K0-repeticao', R: meta.R })
  const fx = valorCelula(exp)
  return {
    protocolo: 'K0-repeticao',
    R: meta.R,
    x,
    fxx: fx,
    report: cruzadoReport(exp),
  }
}

/** K1 — troca f(x1,x2) vs f(x2,x1). */
export function runK1Troca (reducer, meta, payloads = K_PAYLOADS) {
  const fx1x2 = valorReducao(reducer, payloads.x1, payloads.x2, { R: meta.R })
  const fx2x1 = valorReducao(reducer, payloads.x2, payloads.x1, { R: meta.R })
  return {
    protocolo: 'K1-troca',
    R: meta.R,
    fx1x2,
    fx2x1,
    comuta: fx1x2 === fx2x1,
  }
}

/** K4 — idempotência f(y,y)=y com y=f(x1,x2). */
export function runK4Idempotencia (reducer, meta, payloads = K_PAYLOADS) {
  const y = valorReducao(reducer, payloads.x1, payloads.x2, { R: meta.R })
  const fyy = valorReducao(reducer, y, y, { R: meta.R })
  return {
    protocolo: 'K4-idempotencia',
    R: meta.R,
    y,
    fyy,
    idempotente: y === fyy,
  }
}

/** K5 — composição ternária f(f(x1,x2),x3) vs f(x1,f(x2,x3)). */
export function runK5Ternario (reducer, meta, payloads = K_PAYLOADS) {
  const midEsq = valorReducao(reducer, payloads.x1, payloads.x2, { R: meta.R })
  const midDir = valorReducao(reducer, payloads.x2, payloads.x3, { R: meta.R })
  const esquerda = valorReducao(reducer, midEsq, payloads.x3, { R: meta.R })
  const direita = valorReducao(reducer, payloads.x1, midDir, { R: meta.R })
  return {
    protocolo: 'K5-ternario',
    R: meta.R,
    payloads: { x1: payloads.x1, x2: payloads.x2, x3: payloads.x3 },
    midEsq,
    midDir,
    esquerda,
    direita,
    associativo: esquerda === direita,
    nota: 'suporte preservador — capacidade não explica diferença (≠ C7)',
  }
}

/**
 * K6 — invariância de representação: mesmo (x1,x2) → mesmo resultado
 * sob permutação da ordem de escrita (timeline distinta).
 * Divisor de águas: se falhar, f ainda não é operação em X×X.
 */
export function runK6Invariancia (reducer, meta, payloads = K_PAYLOADS) {
  const expA = runCruzadoCellOrdem(reducer, payloads.x1, payloads.x2, 'F1F2', {
    protocolo: 'K6-F1F2',
    R: meta.R,
  })
  const expB = runCruzadoCellOrdem(reducer, payloads.x1, payloads.x2, 'F2F1', {
    protocolo: 'K6-F2F1',
    R: meta.R,
  })
  const resultadoA = valorCelula(expA)
  const resultadoB = valorCelula(expB)
  const timelineDistinta = timelineOps(expA).join('|') !== timelineOps(expB).join('|')
  const invariavel = resultadoA === resultadoB
  return {
    protocolo: 'K6-invariancia',
    R: meta.R,
    par: { x1: payloads.x1, x2: payloads.x2 },
    resultadoA,
    resultadoB,
    timelineDistinta,
    invariavel,
    bemDefinida: invariavel,
    reportA: cruzadoReport(expA),
    reportB: cruzadoReport(expB),
    referencia: 'fis:thm:troca-realizacao — ∂²=id; prof invariável, realização pode mudar',
  }
}

/** K7 — comparação lexMax × rectCell no mesmo par. */
export function runK7ComparacaoCandidatos (payloads = K_PAYLOADS) {
  const pares = [
    { label: 'x1,x2', a: payloads.x1, b: payloads.x2 },
    { label: 'x2,x1', a: payloads.x2, b: payloads.x1 },
    { label: 'x1p,x2', a: payloads.x1p, b: payloads.x2 },
    { label: 'x1,x2p', a: payloads.x1, b: payloads.x2p },
    { label: 'x1,x3', a: payloads.x1, b: payloads.x3 },
    { label: 'x1p,x2p', a: payloads.x1p, b: payloads.x2p },
  ]
  const comparacoes = pares.map(({ label, a, b }) => {
    const lex = valorReducao(PROBE_REDUCER.lexMax, a, b)
    const rect = valorReducao(PROBE_REDUCER.rectCell, a, b)
    return { label, a, b, lexMax: lex, rectCell: rect, iguais: lex === rect }
  })
  const mesmaLei = comparacoes.every((c) => c.iguais)
  return {
    protocolo: 'K7-comparacao',
    comparacoes,
    mesmaLei,
    leisDistintas: !mesmaLei,
    leitura: mesmaLei ? 'mesma_realizacao' : 'duas_leis_ou_duas_representacoes',
  }
}

/** Bateria K0–K6 para um candidato; K7 é transversal. */
export function runKCaracterizacao (candidato, payloads = K_PAYLOADS) {
  const { reducer, R, nome } = candidato
  const meta = { R, nome }
  const K0 = runK0Repeticao(reducer, meta, payloads.x1)
  const K1 = runK1Troca(reducer, meta, payloads)
  const K2 = runD1VariacaoX1(reducer, meta, payloads)
  const K3 = runD2VariacaoX2(reducer, meta, payloads)
  const K4 = runK4Idempotencia(reducer, meta, payloads)
  const K5 = runK5Ternario(reducer, meta, payloads)
  const K6 = runK6Invariancia(reducer, meta, payloads)
  return {
    nome,
    R,
    K0,
    K1,
    K2,
    K3,
    K4,
    K5,
    K6,
    sintese: {
      bemDefinida: K6.bemDefinida,
      comutativa: K1.comuta,
      associativa: K5.associativo,
      idempotente: K4.idempotente,
      dependeX1: K2.efeitoMensuravel,
      dependeX2: K3.efeitoMensuravel,
      fxx: K0.fxx,
    },
    leiC: null,
    referencia: 'candidato a realização dependente de duas incidências — não f=C',
  }
}

/** K0–K7 completa sobre os dois candidatos D3. */
export function runKBateriaCandidatos (payloads = K_PAYLOADS) {
  return {
    lexMax: runKCaracterizacao(CANDIDATO.lexMax, payloads),
    rectCell: runKCaracterizacao(CANDIDATO.rectCell, payloads),
    K7: runK7ComparacaoCandidatos(payloads),
  }
}

// ── L0–L7: invariantes do suporte (classe admissível de f) ────────────────
//
// Catálogo resgatado (catalogo.tex, lib/incidencia.h): o grupo discreto gerado
// por S (deslocamento nilpotente) produz ζ=ΣS^j (convolução/acumulação) e
// μ=1−S (deconvolução/diferença finita), com μζ=ζμ=id — a transformada
// universal diagonaliza a álgebra de convolução; aqui medimos compatibilidade
// operacional no suporte RG6, não postulamos f=C.

/** ζ: prefixo acumulado — (ζa)(t)=Σ_{u≤t}a(u). lib/incidencia.h in_zeta */
export function zetaVec (a) {
  const out = []
  let acc = 0
  for (let i = 0; i < a.length; i++) {
    acc += a[i]
    out.push(acc)
  }
  return out
}

/** μ: diferença finita — (μb)(t)=b(t)−b(t−1). lib/incidencia.h in_mu */
export function muVec (b) {
  const out = []
  for (let i = 0; i < b.length; i++) {
    out.push(i === 0 ? b[0] : b[i] - b[i - 1])
  }
  return out
}

/** Volta exacta μ(ζa)=a e ζ(μb)=b — cat: incidencia trio, tests/pgwire §W153. */
export function zetaMuVoltaExacta (a) {
  const z = zetaVec(a)
  const v1 = muVec(z)
  if (!a.every((x, i) => x === v1[i])) return false
  const m = muVec(a)
  const v2 = zetaVec(m)
  return a.every((x, i) => x === v2[i])
}

/** Indicador de visitas à célula lógica ao longo da timeline. */
export function indicadorTimeline (exp, logicalCell = IO_STATE.X) {
  const c = Number(logicalCell) & 255
  return exp.events.map((e) => ((e.logicalCell & 255) === c ? 1 : 0))
}

/** L6 — compatibilidade com G: protocolo multifocal e gReal coerente. */
export function avaliarCompatG (exp, logicalCell = IO_STATE.X) {
  const protocoloG = verifyMultifocalProtocol(exp.arena, exp.journals)
  const gArena = getG(exp.arena, logicalCell)
  const gJournal = gRealAggregated(exp.journals, logicalCell)
  return {
    protocoloG,
    gRealCoerente: gArena === gJournal,
    gArena,
    gJournal,
    compativel: protocoloG && gArena === gJournal,
  }
}

/** L7 — compatibilidade com ζ/μ: deconvolução fecha sobre a timeline de G. */
export function avaliarCompatZetaMu (exp, logicalCell = IO_STATE.X) {
  const indicador = indicadorTimeline(exp, logicalCell)
  const fecha = indicador.length > 0 && zetaMuVoltaExacta(indicador)
  const z = zetaVec(indicador)
  const muZ = muVec(z)
  return {
    fecha,
    indicador,
    zeta: z,
    muZeta: muZ,
    compativel: fecha,
    referencia: 'catalogo.tex trio S,ζ,μ — acumulação=convolução, μ=deconvolução',
  }
}

/**
 * L0–L7 para um candidato (usa síntese K + medição numa célula representativa).
 * L2–L4 são propriedades algébricas; L5=τ invariância de realização (K6).
 */
export function runLInvariantes (candidato, kChar, payloads = K_PAYLOADS) {
  const exp = runCruzadoCell(candidato.reducer, payloads.x1, payloads.x2, {
    protocolo: 'L-representante',
    R: candidato.R,
  })
  const g = avaliarCompatG(exp)
  const zm = avaliarCompatZetaMu(exp)
  const s = kChar.sintese
  const inv = {
    L0: s.dependeX1 && s.dependeX2,
    L1: s.bemDefinida,
    L2: s.comutativa,
    L3: s.idempotente,
    L4: s.associativa,
    L5: kChar.K6.invariavel,
    L6: g.compativel,
    L7: zm.compativel,
  }
  return {
    nome: candidato.nome,
    R: candidato.R,
    invariantes: inv,
    detalhe: { g, zm, sintese: s },
    leiC: null,
    referencia: 'existência de combinação ≠ unicidade da combinação',
  }
}

/** Quais invariantes separam lexMax de rectCell. */
export function analisarSelecaoInvariantes (lexL, rectL) {
  const chaves = ['L0', 'L1', 'L2', 'L3', 'L4', 'L5', 'L6', 'L7']
  const separadores = []
  const comuns = []
  for (const k of chaves) {
    if (lexL.invariantes[k] && rectL.invariantes[k]) comuns.push(k)
    else if (lexL.invariantes[k] !== rectL.invariantes[k]) separadores.push(k)
  }
  const eliminaRect = separadores.filter((k) => lexL.invariantes[k] && !rectL.invariantes[k])
  const eliminaLex = separadores.filter((k) => !lexL.invariantes[k] && rectL.invariantes[k])
  return {
    comuns,
    separadores,
    eliminaRectCell: eliminaRect,
    eliminaLexMax: eliminaLex,
    classeAdmissivelAmbos: comuns,
    leitura: eliminaRect.length > 0
      ? `L${eliminaRect.join(',L')} eliminam rectCell; lexMax sobrevive se exigidos`
      : 'nenhum invariante L0–L7 separa os candidatos sozinho',
  }
}

/** Bateria L0–L7 sobre os dois candidatos D3 + análise de seleção. */
export function runLBateriaInvariantes (payloads = K_PAYLOADS) {
  const kBat = runKBateriaCandidatos(payloads)
  const lexL = runLInvariantes(CANDIDATO.lexMax, kBat.lexMax, payloads)
  const rectL = runLInvariantes(CANDIDATO.rectCell, kBat.rectCell, payloads)
  const selecao = analisarSelecaoInvariantes(lexL, rectL)
  return {
    lexMax: lexL,
    rectCell: rectL,
    selecao,
    pergunta: 'quais invariantes do suporte restringem a classe de C?',
    congelado: {
      existe: '∃ f bem definido com dependência bilateral',
      naoUnico: 'a condição não determina f — lexMax ≠ rectCell',
      existenciaVsUnicidade: 'existência de combinação ≠ unicidade da combinação',
    },
    catalogo: 'S gera ζ (convolução) e μ (deconvolução); μζ=id — catalogo.tex, incidencia.h',
    leiC: null,
  }
}

// ── M1–M2: autoridade da ordem lex (isolada de ∂) ──────────────────────
//
// thm:rn: a ordem sobe por indução (total). Realização discreta em Word:
// ordem lexicográfica de strings (UTF-16; ASCII = lex).
//
// M1:  x ≺_lex x'  ⇒  f(x,y) ≼_lex f(x',y)
// M2:  y ≺_lex y'  ⇒  f(x,y) ≼_lex f(x,y')
//
// Só sobre CANDIDATO ⊆ A_min (lexMax, rectCell). Não misturar com ∂.
// Não promover lexMax. Central fora. leiC permanece null.

/** Ordem lex em Word. Não é C. */
export function cmpLex (a, b) {
  const sa = String(a ?? '')
  const sb = String(b ?? '')
  if (sa === sb) return 0
  return sa < sb ? -1 : 1
}

export function precLex (a, b) {
  return cmpLex(a, b) < 0
}

export function preceqLex (a, b) {
  return cmpLex(a, b) <= 0
}

/** Cadeia total estrita — controle da ordem (M0). Independente de ∂. */
export const M_CADEIA = Object.freeze(['A0', 'B0', 'C0', 'D0'])

function paresPrecLex (cadeia) {
  const pares = []
  for (let i = 0; i < cadeia.length; i++) {
    for (let j = i + 1; j < cadeia.length; j++) {
      pares.push([cadeia[i], cadeia[j]])
    }
  }
  return pares
}

/** M0 — a cadeia é ordem total estrita crescente. */
export function runM0ControleOrdem (cadeia = M_CADEIA) {
  const xs = [...cadeia]
  const distinctos = xs.every((x, i) => xs.every((y, j) => i === j || cmpLex(x, y) !== 0))
  const crescente = xs.every((x, i) => i === 0 || precLex(xs[i - 1], x))
  return {
    protocolo: 'M0-controle-ordem',
    cadeia: xs,
    distinctos,
    crescente,
    totalEstrita: distinctos && crescente,
    referencia: 'thm:rn — ordem total; realização Word lex',
  }
}

/**
 * M1 — monotonia no 1º argumento: x ≺ x' ⇒ f(x,y) ≼ f(x',y), y varrido na cadeia.
 */
export function runM1MonoArg1 (candidato, cadeia = M_CADEIA) {
  const xs = [...cadeia]
  const pares = paresPrecLex(xs)
  const amostras = []
  for (const y of xs) {
    for (const [x, xp] of pares) {
      const fx = valorReducao(candidato.reducer, x, y, { R: candidato.R })
      const fxp = valorReducao(candidato.reducer, xp, y, { R: candidato.R })
      amostras.push({
        x,
        xp,
        y,
        fx,
        fxp,
        precArg: precLex(x, xp),
        ok: preceqLex(fx, fxp),
      })
    }
  }
  const violacoes = amostras.filter((a) => !a.ok)
  return {
    protocolo: 'M1-mono-arg1',
    nome: candidato.nome,
    R: candidato.R,
    cadeia: xs,
    amostras,
    nAmostras: amostras.length,
    nViolacoes: violacoes.length,
    violacoes,
    mono: violacoes.length === 0,
    leiC: null,
    pergunta: 'x ≺_lex x′ ⇒ f(x,y) ≼_lex f(x′,y) ?',
  }
}

/**
 * M2 — monotonia no 2º argumento: y ≺ y' ⇒ f(x,y) ≼ f(x,y'), x varrido na cadeia.
 */
export function runM2MonoArg2 (candidato, cadeia = M_CADEIA) {
  const xs = [...cadeia]
  const pares = paresPrecLex(xs)
  const amostras = []
  for (const x of xs) {
    for (const [y, yp] of pares) {
      const fy = valorReducao(candidato.reducer, x, y, { R: candidato.R })
      const fyp = valorReducao(candidato.reducer, x, yp, { R: candidato.R })
      amostras.push({
        x,
        y,
        yp,
        fy,
        fyp,
        precArg: precLex(y, yp),
        ok: preceqLex(fy, fyp),
      })
    }
  }
  const violacoes = amostras.filter((a) => !a.ok)
  return {
    protocolo: 'M2-mono-arg2',
    nome: candidato.nome,
    R: candidato.R,
    cadeia: xs,
    amostras,
    nAmostras: amostras.length,
    nViolacoes: violacoes.length,
    violacoes,
    mono: violacoes.length === 0,
    leiC: null,
    pergunta: 'y ≺_lex y′ ⇒ f(x,y) ≼_lex f(x,y′) ?',
  }
}

function sinteseM (m1, m2) {
  return {
    M1: m1.mono,
    M2: m2.mono,
    ambos: m1.mono && m2.mono,
    nViolacoesM1: m1.nViolacoes,
    nViolacoesM2: m2.nViolacoes,
  }
}

/**
 * Bateria M1–M2 sobre A_min + registo (M1, M2, K).
 * K entra como caracterização já medida — não se reabre K0–K7.
 * Independência (M não selecciona C) é resultado válido.
 */
export function runMBateriaOrdem (payloads = K_PAYLOADS, cadeia = M_CADEIA) {
  const m0 = runM0ControleOrdem(cadeia)
  const lexM1 = runM1MonoArg1(CANDIDATO.lexMax, cadeia)
  const lexM2 = runM2MonoArg2(CANDIDATO.lexMax, cadeia)
  const rectM1 = runM1MonoArg1(CANDIDATO.rectCell, cadeia)
  const rectM2 = runM2MonoArg2(CANDIDATO.rectCell, cadeia)
  const kBat = runKBateriaCandidatos(payloads)
  const sintese = {
    lexMax: sinteseM(lexM1, lexM2),
    rectCell: sinteseM(rectM1, rectM2),
  }
  const separa = sintese.lexMax.ambos !== sintese.rectCell.ambos
  return {
    m0,
    lexMax: { M1: lexM1, M2: lexM2, sintese: sintese.lexMax },
    rectCell: { M1: rectM1, M2: rectM2, sintese: sintese.rectCell },
    registro: {
      M1: { lexMax: lexM1.mono, rectCell: rectM1.mono },
      M2: { lexMax: lexM2.mono, rectCell: rectM2.mono },
      K: {
        lexMax: kBat.lexMax.sintese,
        rectCell: kBat.rectCell.sintese,
        K7leisDistintas: kBat.K7.leisDistintas,
      },
    },
    separa,
    pergunta: 'a ordem lex tem autoridade sobre f ∈ A_min?',
    congelado: {
      autoridade: separa
        ? 'M1/M2 separam lexMax de rectCell — classificação, não escolha de C'
        : 'M1/M2 não separam os candidatos de A_min',
      naoUnico: 'a ordem lex restringe; não determina uma lei única C',
      naoPromove: 'lexMax sobrevivente sob monotonia ≠ C',
    },
    referencia: 'thm:rn (ordem total); A_min = L0–L7; K0–K7 caracterização',
    leiC: null,
  }
}

// ── E_∂: covariância sob a dobra (isolada da ordem lex) ──────────────
//
// E_∂:  ∂( f(∂x, ∂y) )  ~  f(x,y)
// ~ = igualdade de resultados. Sem cmpLex. Sem M1/M2.
//
// ∂ realiza D(i)=(N−1)−i no alfabeto finito (fis:thm:troca-realizacao).
// Central fora. Não promover lexMax. leiC permanece null.

/** Carrier finito da dobra — distinto da cadeia lex M_CADEIA. */
export const E_ALFABETO = Object.freeze(['P0', 'P1', 'P2', 'P3'])

/** D(i)=(N−1)−i. Fora do alfabeto → null (não inventar extensão). */
export function dobraPartial (x, alfabeto = E_ALFABETO) {
  const xs = alfabeto
  const i = xs.indexOf(x)
  if (i < 0) return null
  return xs[xs.length - 1 - i]
}

/** E0 — ∂²=id e o alfabeto fecha. */
export function runE0ControleDobra (alfabeto = E_ALFABETO) {
  const xs = [...alfabeto]
  const imagens = xs.map((x) => dobraPartial(x, xs))
  const fecha = imagens.every((y) => y != null && xs.includes(y))
  const involucao = xs.every((x) => dobraPartial(dobraPartial(x, xs), xs) === x)
  const semPontoFixo = xs.length % 2 === 0 && xs.every((x) => dobraPartial(x, xs) !== x)
  return {
    protocolo: 'E0-controle-dobra',
    alfabeto: xs,
    imagens,
    fecha,
    involucao,
    semPontoFixo,
    referencia: 'fis:thm:troca-realizacao — D(i)=(N−1)−i, D²=id',
  }
}

/**
 * E_∂ — covariância: ∂(f(∂x,∂y)) ~ f(x,y) em todo o alfabeto.
 * Se f(∂x,∂y) cai fora do carrier, a amostra é foraDoSuporte (não aplicável).
 */
export function runEPartialCovariancia (candidato, alfabeto = E_ALFABETO) {
  const xs = [...alfabeto]
  const amostras = []
  for (const x of xs) {
    for (const y of xs) {
      const dx = dobraPartial(x, xs)
      const dy = dobraPartial(y, xs)
      const fdxdy = valorReducao(candidato.reducer, dx, dy, { R: candidato.R })
      const dfd = dobraPartial(fdxdy, xs)
      const fxy = valorReducao(candidato.reducer, x, y, { R: candidato.R })
      const foraDoSuporte = dfd == null
      const igual = !foraDoSuporte && dfd === fxy
      amostras.push({ x, y, dx, dy, fdxdy, dfd, fxy, foraDoSuporte, igual })
    }
  }
  const aplicavel = amostras.every((a) => !a.foraDoSuporte)
  const nIguais = amostras.filter((a) => a.igual).length
  const nFora = amostras.filter((a) => a.foraDoSuporte).length
  return {
    protocolo: 'E-partial-covariancia',
    nome: candidato.nome,
    R: candidato.R,
    alfabeto: xs,
    amostras,
    nAmostras: amostras.length,
    nIguais,
    nFora,
    aplicavel,
    covariante: aplicavel && nIguais === amostras.length,
    leiC: null,
    pergunta: '∂(f(∂x,∂y)) ~ f(x,y) ?',
  }
}

function sinteseE (e) {
  return {
    aplicavel: e.aplicavel,
    covariante: e.covariante,
    nIguais: e.nIguais,
    nFora: e.nFora,
    nAmostras: e.nAmostras,
  }
}

/**
 * Bateria E_∂ sobre A_min. Não chama M1/M2 nem cmpLex.
 * K entra como caracterização já medida.
 */
export function runEBateriaDobra (payloads = K_PAYLOADS, alfabeto = E_ALFABETO) {
  const e0 = runE0ControleDobra(alfabeto)
  const lexE = runEPartialCovariancia(CANDIDATO.lexMax, alfabeto)
  const rectE = runEPartialCovariancia(CANDIDATO.rectCell, alfabeto)
  const kBat = runKBateriaCandidatos(payloads)
  const sintese = {
    lexMax: sinteseE(lexE),
    rectCell: sinteseE(rectE),
  }
  const independentesDeIgualdade = !(sintese.lexMax.covariante && sintese.rectCell.covariante)
  return {
    e0,
    lexMax: { E: lexE, sintese: sintese.lexMax },
    rectCell: { E: rectE, sintese: sintese.rectCell },
    registro: {
      E_partial: {
        lexMax: { aplicavel: lexE.aplicavel, covariante: lexE.covariante },
        rectCell: { aplicavel: rectE.aplicavel, covariante: rectE.covariante },
      },
      K: {
        lexMax: kBat.lexMax.sintese,
        rectCell: kBat.rectCell.sintese,
        K7leisDistintas: kBat.K7.leisDistintas,
      },
    },
    independentesDeIgualdade,
    pergunta: '∂(f(∂x,∂y)) ~ f(x,y) em A_min?',
    congelado: {
      leitura: independentesDeIgualdade
        ? 'E_∂ não selecciona uma lei única em A_min — independência é resultado válido'
        : 'ambos covariantes sob ∂ — ainda não implica C único',
      naoPromove: 'covariância sob a dobra ≠ C',
    },
    referencia: 'fis:thm:troca-realizacao (D²=id); isolado da ordem lex',
    leiC: null,
  }
}
