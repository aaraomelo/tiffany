/* tests/fronteira_front.js — fronteira na UI.
 *   node tests/fronteira_front.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

;(async () => {
  const {
    fronteiraGut5w2h, fronteiraWhyIshikawa, fronteiraPdcaVsm, passoFronteira,
  } = await import('../app/src/fronteira.js')

  console.log('=== FRONTEIRA FRONT (UI) ===\n')

  ok('§F0 consistente', fronteiraGut5w2h([60, 48, 30], [0, 1, 2]) === 0)
  ok('§F0 swap', fronteiraGut5w2h([60, 48, 30], [1, 0, 2]) > 0)
  ok('§F1 why hit', fronteiraWhyIshikawa(2, [1, 2, 3]) === 0)
  ok('§F1 why miss', fronteiraWhyIshikawa(9, [1, 2, 3]) === 1)
  ok('§F2 pdca eq', fronteiraPdcaVsm(10, 10) === 0)

  const p = passoFronteira('mostra a fronteira')
  ok('§F3 passo demo', p && p.ok && p.R === 0)

  const g = passoFronteira('gut:60,48,12 ordem:1,0,2')
  ok('§F3 gut explícito com inversão', g && g.tipo === 'gut' && g.R > 0)

  const d = passoFronteira('pdca:10 vsm:10')
  ok('§F3 pdca explícito', d && d.ok)

  ok('§F4 ignora fala banal', passoFronteira('bom dia') === null)

  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => { console.error(e); process.exit(1) })
