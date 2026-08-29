/* tests/redes_g.js — G_visit vs G_real (Redes multifocais, ramo árvore).
 *   node tests/redes_g.js
 *
 *   §RG2 — protocolo válido  ⇒ verifyGVisitEqualsReal true
 *   §RG3 — violações controladas ⇒ verify false, G_visit ≠ G_real
 */
'use strict'
import { join, dirname } from 'node:path'
import { readFileSync } from 'node:fs'
import { fileURLToPath } from 'node:url'
import {
  seedScriptArena, incG, getG, sumG, gsumTotal, G_CELL,
  recordCorreVisit, gRealFromJournal, verifyGVisitEqualsReal,
} from '../lib/arena_disco.mjs'
import { execMoveMetal } from '../tools/banco_metal.mjs'

const __dir = dirname(fileURLToPath(import.meta.url))
const BASE = join(__dir, '..', '.torre', 'reino_redes_g')
const BASE2 = join(__dir, '..', '.torre', 'reino_redes_g2')

let falhas = 0, feitas = 0
function ok (q, c) { feitas++; if (!c) falhas++; console.log(`#UNIT ${c ? 'ok' : 'falha'} ${q}`) }

ok('§RG0 incG local', (() => {
  const a = seedScriptArena('x', 'node')
  incG(a, G_CELL.node)
  incG(a, G_CELL.node)
  return getG(a, G_CELL.node) === 2 && gsumTotal(a) === 2
})())

ok('§RG0 sumG', (() => {
  const a = seedScriptArena('', 'bash')
  incG(a, 1); incG(a, 2); incG(a, 2)
  return sumG(a) === 3 && gsumTotal(a) === 3
})())

ok('§RG2 protocolo journal (local)', (() => {
  const journal = []
  const a = seedScriptArena('', 'node')
  recordCorreVisit(a, 'node', journal)
  recordCorreVisit(a, 'bash', journal)
  recordCorreVisit(a, 'node', journal)
  return verifyGVisitEqualsReal(a, journal) &&
    getG(a, G_CELL.node) === gRealFromJournal(journal, G_CELL.node) &&
    getG(a, G_CELL.bash) === gRealFromJournal(journal, G_CELL.bash)
})())

ok('§RG3 incremento fantasma', (() => {
  const x = G_CELL.node
  const journal = []
  const a = seedScriptArena('', 'node')
  recordCorreVisit(a, 'node', journal)
  incG(a, x)
  const gV = getG(a, x)
  const gR = gRealFromJournal(journal, x)
  return journal.length === 1 &&
    gV === 2 && gR === 1 &&
    gV !== gR &&
    !verifyGVisitEqualsReal(a, journal)
})())

ok('§RG3 entrada extra no diário', (() => {
  const x = G_CELL.bash
  const journal = []
  const a = seedScriptArena('', 'bash')
  recordCorreVisit(a, 'bash', journal)
  journal.push(x)
  const gV = getG(a, x)
  const gR = gRealFromJournal(journal, x)
  return gV === 1 && gR === 2 &&
    gV !== gR &&
    !verifyGVisitEqualsReal(a, journal)
})())

try {
  const journal1 = []
  execMoveMetal(BASE, 'node', "console.log('a')", journal1)
  execMoveMetal(BASE, 'node', "console.log('b')", journal1)
  ok('§RG1 G(node) após 2 correr (diário)', gRealFromJournal(journal1, G_CELL.node) >= 2)
  ok('§RG1 células distintas', (() => {
    const rn = execMoveMetal(BASE, 'bash', 'echo x\n')
    return rn.meta.gVisit >= 1 && G_CELL.node !== G_CELL.bash
  })())

  const journal = []
  execMoveMetal(BASE2, 'node', "console.log('1')", journal)
  execMoveMetal(BASE2, 'node', "console.log('2')", journal)
  execMoveMetal(BASE2, 'node', "console.log('3')", journal)
  const arenaPath = join(BASE2 + '_node', 'arena.bin')
  const arena = readFileSync(arenaPath)
  ok('§RG2 G_visit=G_real (última corrida metal)', verifyGVisitEqualsReal(arena, [journal[journal.length - 1]]))
  ok('§RG2 diário agrega 3 visitas', journal.length === 3 && gRealFromJournal(journal, G_CELL.node) === 3)
} catch (e) {
  ok('§RG1/RG2 metal: ' + (e.message || e), false)
}

console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
