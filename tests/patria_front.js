/* tests/patria_front.js — portão Pátria na UI.
 *   node tests/patria_front.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

;(async () => {
  const { residualPatria, patriaFecha, passoPatria, pedePatria, medePatriaLive } =
    await import('../app/src/patria.js')
  console.log('=== PATRIA FRONT (UI) ===\n')
  ok('§P0 R=0 só com os dois', residualPatria(1, 1) === 0 && residualPatria(1, 0) === 1)
  ok('§P0 0∧0 não fecha', residualPatria(0, 0) === 1 && !patriaFecha(0, 0, 1))
  ok('§P1 fecha só com fetch', patriaFecha(1, 1, 1) && !patriaFecha(1, 1, 0))
  const d = passoPatria('mostra o deploy')
  ok('§P2 demo REOPEN (live=0)', d && !d.ok && d.R === 1)
  const g = passoPatria('mostra a pátria', { local: 1, live: 1, external: 1 })
  ok('§P2 no ar CLOSE', g && g.ok && g.R === 0)
  ok('§P3 ignora banal', passoPatria('bom dia') === null)
  ok('§P3 pede', pedePatria('no ar?') && pedePatria('o que e o deploy'))
  {
    const live = await medePatriaLive(async () => ({ ok: true }))
    ok('§P4 fetch mock 200', live === 1)
    const morto = await medePatriaLive(async () => { throw new Error('net') })
    ok('§P4 fetch falha = 0', morto === 0)
  }
  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})()
