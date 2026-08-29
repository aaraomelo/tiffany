/* arena_multifocal.mjs — quatro focos, uma arena, um X (§RG4–§RG6).
 * §RG6 FECHADO. P6 é postulado de fechamento (papers/redes.tex), não medição.
 *
 * Três multiplicidades (não iguais):
 *   G_event  — eventos/visitas (escrita+leitura separados)
 *   G_focus  — focos distintos sobre x
 *   G_state  — estados preservados no suporte (G_event>1 ⇏ G_state>1)
 *
 * overwrite_single_slot ≠ lei de combinação ≠ Hebb ≠ Hopfield
 * Sem soma, W, energia, merge, RG7.
 */

import {

  seedScriptArena, incG, getG, gsumTotal, gRealFromJournal, G_CELLS,

  OFF_IN, OFF_OUT, OFF_NIN, OFF_NOUT, CAP,

} from './arena_disco.mjs'



export const FOCUS_IDS = ['F1', 'F2', 'F3', 'F4']

export const FOCUS_TAG = { F1: 41, F2: 42, F3: 43, F4: 44 }

export const STATE_CELL = { A: 51, B: 52, C: 53, D: 54 }

export const ISOLATED_CELL = { F1: 61, F2: 62, F3: 63, F4: 64 }

export const IO_STATE = { A: 51, B: 52, C: 53, D: 54, X: 71, Y: 72 }



/** Slots físicos mapeáveis à mesma célula lógica. */

export const PHYS_SLOT = { IN: 'in', OUT: 'out' }



export function emptyMultifocalJournals () {

  return { F1: [], F2: [], F3: [], F4: [] }

}



export function focusTouch (arena, journals, focus, cell) {

  const f = String(focus)

  if (!journals[f]) journals[f] = []

  incG(arena, cell)

  journals[f].push(Number(cell) & 255)

  return arena

}



/** G_event(x): eventos (escrita + leitura) com célula lógica x. */

export function gEvent (events, logicalCell) {

  const c = Number(logicalCell) & 255

  let n = 0

  for (const e of events || []) {

    if ((e.logicalCell & 255) === c) n++

  }

  return n

}



/** G_focus(x): focos distintos que incidiram sobre x. */

export function gFocus (events, logicalCell) {

  const c = Number(logicalCell) & 255

  const s = new Set()

  for (const e of events || []) {

    if ((e.logicalCell & 255) === c) s.add(e.focus)

  }

  return s.size

}



/** G_state(x): payloads distintos sobreviventes nos slots físicos ligados a x. */

export function gStateSurviving (exp, logicalCell, physSlots = [PHYS_SLOT.IN]) {

  const snap = snapshotIo(exp.arena)

  const payloads = new Set()

  for (const p of physSlots) {

    const v = p === PHYS_SLOT.OUT ? snap.out : snap.in

    if (v.length > 0) payloads.add(v)

  }

  return payloads.size

}



/** Payloads distintos escritos (histórico) para célula lógica x. */

export function gStateWritten (events, logicalCell) {

  const c = Number(logicalCell) & 255

  const s = new Set()

  for (const e of events || []) {

    if ((e.logicalCell & 255) === c && String(e.op || '').includes('write') && e.payload != null) {

      s.add(String(e.payload))

    }

  }

  return s.size

}



export function gMetrics (exp, logicalCell, physSlots = [PHYS_SLOT.IN]) {

  return {

    gEvent: gEvent(exp.events, logicalCell),

    gFocus: gFocus(exp.events, logicalCell),

    gStateSurviving: gStateSurviving(exp, logicalCell, physSlots),

    gStateWritten: gStateWritten(exp.events, logicalCell),

    gVisit: getG(exp.arena, logicalCell),

  }

}



/** Σ_a |π_a^{-1}(x)| no diário (conta escrita+leitura como eventos separados). */

export function gRealAggregated (journals, cell) {

  let n = 0

  for (const id of FOCUS_IDS) {

    n += gRealFromJournal(journals[id] || [], cell)

  }

  return n

}



export function cellsOfFocus (journal) {

  const s = new Set()

  for (let i = 0; i < journal.length; i++) s.add(journal[i] & 255)

  return s

}



export function focusesDisjoint (journals) {

  const sets = FOCUS_IDS.map((id) => cellsOfFocus(journals[id] || []))

  for (let a = 0; a < sets.length; a++) {

    for (let b = a + 1; b < sets.length; b++) {

      for (const c of sets[a]) {

        if (sets[b].has(c)) return false

      }

    }

  }

  return true

}



export function verifyMultifocalProtocol (arena, journals) {

  const seen = new Set()

  for (const id of FOCUS_IDS) {

    const j = journals[id] || []

    for (let i = 0; i < j.length; i++) seen.add(j[i] & 255)

  }

  for (const c of seen) {

    if (getG(arena, c) !== gRealAggregated(journals, c)) return false

  }

  for (let c = 0; c < G_CELLS; c++) {

    const v = getG(arena, c)

    if (v > 0 && !seen.has(c)) return false

  }

  let total = 0

  for (const id of FOCUS_IDS) total += (journals[id] || []).length

  return total === gsumTotal(arena)

}



export function journalEventCounts (journals) {

  const out = {}

  for (const id of FOCUS_IDS) out[id] = (journals[id] || []).length

  return out

}



export function runConversationCycle (arena, journals) {

  const { A, B, C, D } = STATE_CELL

  focusTouch(arena, journals, 'F1', A)

  focusTouch(arena, journals, 'F2', A)

  focusTouch(arena, journals, 'F2', B)

  focusTouch(arena, journals, 'F3', B)

  focusTouch(arena, journals, 'F3', C)

  focusTouch(arena, journals, 'F4', C)

  focusTouch(arena, journals, 'F4', D)

  focusTouch(arena, journals, 'F1', D)

  return arena

}



export function createSharedBrainArena () {

  return seedScriptArena('', null)

}



export function runIsolatedFoci (arena, journals) {

  for (const id of FOCUS_IDS) {

    focusTouch(arena, journals, id, ISOLATED_CELL[id])

    focusTouch(arena, journals, id, FOCUS_TAG[id])

  }

  return arena

}



export function readInBuffer (arena) {

  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)

  const n = a[OFF_NIN] + a[OFF_NIN + 1] * 256

  return a.toString('utf8', OFF_IN, OFF_IN + Math.min(n, CAP))

}



export function readOutBuffer (arena) {

  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)

  const n = a[OFF_NOUT] + a[OFF_NOUT + 1] * 256

  return a.toString('utf8', OFF_OUT, OFF_OUT + Math.min(n, CAP))

}



export function writeInBuffer (arena, text) {

  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)

  const body = Buffer.from(String(text ?? ''), 'utf8')

  const n = Math.min(body.length, CAP)

  body.copy(a, OFF_IN, 0, n)

  a[OFF_NIN] = n & 255

  a[OFF_NIN + 1] = (n >> 8) & 255

  return a

}



export function writeOutBuffer (arena, text) {

  const a = Buffer.isBuffer(arena) ? arena : Buffer.from(arena)

  const body = Buffer.from(String(text ?? ''), 'utf8')

  const n = Math.min(body.length, CAP)

  body.copy(a, OFF_OUT, 0, n)

  a[OFF_NOUT] = n & 255

  a[OFF_NOUT + 1] = (n >> 8) & 255

  return a

}



export function snapshotIo (arena) {

  return {

    in: readInBuffer(arena),

    out: readOutBuffer(arena),

    nin: arena[OFF_NIN] + arena[OFF_NIN + 1] * 256,

    nout: arena[OFF_NOUT] + arena[OFF_NOUT + 1] * 256,

  }

}



export function emptyIoExperiment () {

  return {

    arena: createSharedBrainArena(),

    journals: emptyMultifocalJournals(),

    events: [],

    overwrites: [],

    audit: null,

  }

}



function recordIoEvent (exp, focus, op, logicalCell, extra = {}) {

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



function noteOverwrite (exp, focus, logicalCell, physSlot, prior, payload) {

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



export function focusReadPhys (exp, focus, logicalCell, physSlot = PHYS_SLOT.IN) {

  const before = snapshotIo(exp.arena)

  const payload = physSlot === PHYS_SLOT.OUT ? before.out : before.in

  const op = physSlot === PHYS_SLOT.OUT ? 'read_out' : 'read_in'

  recordIoEvent(exp, focus, op, logicalCell, { before, after: before, payload, physSlot })

  return payload

}



export function focusWritePhys (exp, focus, logicalCell, physSlot, payload) {

  const before = snapshotIo(exp.arena)

  const prior = physSlot === PHYS_SLOT.OUT ? before.out : before.in

  if (physSlot === PHYS_SLOT.OUT) writeOutBuffer(exp.arena, payload)

  else writeInBuffer(exp.arena, payload)

  const after = snapshotIo(exp.arena)

  noteOverwrite(exp, focus, logicalCell, physSlot, prior, payload)

  const op = physSlot === PHYS_SLOT.OUT ? 'write_out' : 'write_in'

  recordIoEvent(exp, focus, op, logicalCell, { before, after, payload, physSlot })

  return after

}



export function focusReadIn (exp, focus, logicalCell) {

  return focusReadPhys(exp, focus, logicalCell, PHYS_SLOT.IN)

}



export function focusWriteIn (exp, focus, logicalCell, payload) {

  return focusWritePhys(exp, focus, logicalCell, PHYS_SLOT.IN, payload)

}



export function fociOnCell (events, cell) {

  const c = Number(cell) & 255

  const out = []

  for (const id of FOCUS_IDS) {

    for (const e of events || []) {

      if (e.focus === id && (e.logicalCell & 255) === c) {

        out.push(id)

        break

      }

    }

  }

  return out

}



export function ioExperimentReport (exp, label, opts = {}) {

  const physSlots = opts.physSlots || [PHYS_SLOT.IN]

  const cells = [...new Set(exp.events.map((e) => e.logicalCell))]

  const perCell = cells.map((c) => ({

    cell: c,

    ...gMetrics(exp, c, physSlots),

    foci: fociOnCell(exp.events, c),

  }))

  return {

    label,

    arenaUnica: true,

    eventosPorFoco: journalEventCounts(exp.journals),

    ordemEventos: exp.events.map((e) => ({

      focus: e.focus,

      op: e.op,

      cell: e.logicalCell,

      physSlot: e.physSlot ?? PHYS_SLOT.IN,

      payload: e.payload ?? null,

    })),

    offInFinal: snapshotIo(exp.arena).in,

    offOutFinal: snapshotIo(exp.arena).out,

    gsum: gsumTotal(exp.arena),

    celulas: perCell,

    overwrites: exp.overwrites,

    audit: exp.audit,

    protocoloG: verifyMultifocalProtocol(exp.arena, exp.journals),

  }

}



export function runIoPipeline (exp) {

  focusWriteIn(exp, 'F1', IO_STATE.A, 'A')

  focusReadIn(exp, 'F2', IO_STATE.A)

  focusWriteIn(exp, 'F2', IO_STATE.B, 'B')

  focusReadIn(exp, 'F3', IO_STATE.B)

  focusWriteIn(exp, 'F3', IO_STATE.C, 'C')

  focusReadIn(exp, 'F4', IO_STATE.C)

  focusWriteIn(exp, 'F4', IO_STATE.D, 'D')

  focusReadIn(exp, 'F1', IO_STATE.D)

  return exp

}



export function runIoDualWriteX (exp) {

  focusWriteIn(exp, 'F1', IO_STATE.X, 'X:F1')

  focusWriteIn(exp, 'F2', IO_STATE.X, 'X:F2')

  focusReadIn(exp, 'F3', IO_STATE.X)

  focusReadIn(exp, 'F4', IO_STATE.X)

  return exp

}



export function runCaseA (exp) {

  focusWriteIn(exp, 'F1', IO_STATE.X, 'X')

  focusReadIn(exp, 'F2', IO_STATE.X)

  return exp

}



export function runCaseB (exp) {

  focusWriteIn(exp, 'F1', IO_STATE.X, 'X')

  focusWriteIn(exp, 'F2', IO_STATE.Y, 'Y')

  focusReadIn(exp, 'F3', IO_STATE.Y)

  return exp

}



export function runCaseCMin (exp) {

  writeInBuffer(exp.arena, 'X')

  const audit = {

    xInicial: 'X',

    ordem: [{ step: 'seed', offIn: 'X', focus: null }],

    x1: null,

    x2: null,

    estadoFinal: null,

    f3Leu: null,

    x1ConservadoSimultaneamenteComX2: false,

  }

  focusWriteIn(exp, 'F1', IO_STATE.X, 'X1')

  audit.x1 = 'X1'

  audit.ordem.push({ step: 'F1', payload: 'X1', focus: 'F1', offIn: snapshotIo(exp.arena).in })

  focusWriteIn(exp, 'F2', IO_STATE.X, 'X2')

  audit.x2 = 'X2'

  audit.ordem.push({ step: 'F2', payload: 'X2', focus: 'F2', offIn: snapshotIo(exp.arena).in })

  audit.f3Leu = focusReadIn(exp, 'F3', IO_STATE.X)

  audit.estadoFinal = snapshotIo(exp.arena).in

  audit.ordem.push({ step: 'F3', read: audit.f3Leu, focus: 'F3' })

  audit.x1ConservadoSimultaneamenteComX2 =

    gStateWritten(exp.events, IO_STATE.X) >= 2 &&

    gStateSurviving(exp, IO_STATE.X, [PHYS_SLOT.IN]) >= 2

  exp.audit = audit

  return exp

}



export function runCaseC (exp) {

  return runCaseCMin(exp)

}



export function runDualSlotSameLogical (exp) {

  focusWritePhys(exp, 'F1', IO_STATE.X, PHYS_SLOT.IN, 'X1')

  focusWritePhys(exp, 'F2', IO_STATE.X, PHYS_SLOT.OUT, 'X2')

  focusReadPhys(exp, 'F3', IO_STATE.X, PHYS_SLOT.IN)

  focusReadPhys(exp, 'F4', IO_STATE.X, PHYS_SLOT.OUT)

  return exp

}



export function runCaseSameRep (exp) {

  focusWriteIn(exp, 'F1', IO_STATE.X, 'X|F1')

  focusWriteIn(exp, 'F2', IO_STATE.X, 'X|F2')

  focusReadIn(exp, 'F3', IO_STATE.X)

  focusReadIn(exp, 'F4', IO_STATE.X)

  return exp

}



export function runCaseDiffRep (exp) {

  focusWriteIn(exp, 'F1', IO_STATE.X, 'X')

  focusWriteIn(exp, 'F2', IO_STATE.Y, 'Y')

  focusReadIn(exp, 'F3', IO_STATE.X)

  focusReadIn(exp, 'F4', IO_STATE.Y)

  return exp

}



/** Slot único + duas escritas na mesma representação lógica → fronteira aberta. */

export function boundaryCombinationOpen (exp, logicalCell = IO_STATE.X) {

  const m = gMetrics(exp, logicalCell, [PHYS_SLOT.IN])

  const writes = (exp.events || []).filter(

    (e) => (e.logicalCell & 255) === (logicalCell & 255) && String(e.op).includes('write_in'),

  )

  const sameSlot = writes.length >= 2

  return sameSlot && gStateWritten(exp.events, logicalCell) >= 2 && m.gStateSurviving === 1

}



export function multifocalReport (arena, journals, label) {

  const counts = journalEventCounts(journals)

  const shared = [STATE_CELL.A, STATE_CELL.B, STATE_CELL.C, STATE_CELL.D]

    .map((c) => ({ cell: c, gReal: gRealAggregated(journals, c), gVisit: getG(arena, c) }))

  return {

    label,

    eventos: counts,

    totalEventos: Object.values(counts).reduce((a, b) => a + b, 0),

    gsum: gsumTotal(arena),

    celulasPartilhadas: shared,

    disjoint: focusesDisjoint(journals),

    protocolo: verifyMultifocalProtocol(arena, journals),

  }

}


