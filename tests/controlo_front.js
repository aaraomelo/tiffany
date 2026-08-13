/* tests/controlo_front.js — Controlo na UI (espelha app/src/controlo.js).
 * §U0 RETAIN  §U1 RETRACT  §U2 MOVE  §U3 nomes
 * §U5 cafe↔café caixa  §U6 fonético  §U7 teclado
 *
 *   node tests/controlo_front.js
 */
'use strict'

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

;(async () => {
  const {
    CTRL, CTRL_NOME, distControlo, decideControlo, passoControlo, admissivel, THETA_DEFAULT,
    eixosCaixa, eixosTeclado, eixosFonetico, eixosForma, cenarioAcaso,
  } = await import('../app/src/controlo.js')

  console.log('=== CONTROLO FRONT (UI + eixos texto) ===\n')

  {
    const d = distControlo({ x: 0.1 }, { x: 0.1 })
    const r = decideControlo({ r: 0, D: d })
    ok('§U0 identidade → RETAIN', r.act === CTRL.RETAIN && r.nome === 'RETAIN')
  }

  {
    const r = decideControlo({ r: 3, D: [0, 0, 0, 0, 0, 0, 0, 0] })
    ok('§U1 r≠0 → RETRACT', r.act === CTRL.RETRACT)
  }

  {
    const d = distControlo({ x: 0 }, { x: 0.5 })
    const r = decideControlo({ r: 0, D: d })
    ok('§U2 longe → MOVE', r.act === CTRL.MOVE && !admissivel(d, THETA_DEFAULT))
  }

  {
    ok('§U3 nomes', CTRL_NOME[CTRL.RETAIN] === 'RETAIN' && CTRL_NOME[2] === 'RETRACT')
  }

  {
    const p = passoControlo({ x: 0.2 }, { x: 0.2 }, { ok: true, r: 0 })
    ok('§U4 passoControlo fecho', p.act === CTRL.RETAIN)
  }

  {
    ok('§U5 caixa cafe↔café = 1', eixosCaixa('cafe', 'café') === 1)
    ok('§U5 forma cafe≡café', eixosForma('cafe', 'café') === 0)
    ok('§U6 fonético cafe≡café', eixosFonetico('cafe', 'café') === 0)
    ok('§U6 fonético casa≠caza', eixosFonetico('casa', 'caza') > 0)
    ok('§U7 teclado id=0', eixosTeclado('casa', 'casa') === 0)
    ok('§U7 teclado que≠qeu', eixosTeclado('que', 'qeu') > 0)
  }

  {
    const d = distControlo(
      { x: 0.1, texto: 'cafe' },
      { x: 0.1, texto: 'café' },
    )
    const r = decideControlo({ r: 0, D: d })
    ok('§U8 texto café → RETAIN (caixa≤Θ)', r.act === CTRL.RETAIN && d[4] === 1)
  }

  {
    const d = distControlo(
      { x: 0.1, texto: 'cafe' },
      { x: 0.1, texto: 'computador' },
    )
    const r = decideControlo({ r: 0, D: d })
    ok('§U9 texto longe → MOVE', r.act === CTRL.MOVE)
  }

  ok('§U10 THETA 8 eixos', THETA_DEFAULT.length === 8)

  {
    const d = cenarioAcaso({ nSorteios: 40, empatesDegen: 40, discordaL1: 30 })
    ok('§U11 acaso degen', d.degen && /degenerada/.test(d.motivo))
    const i = cenarioAcaso({ nSorteios: 40, empatesDegen: 10, discordaL1: 30 })
    ok('§U11 acaso informativo', !i.degen)
  }

  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => {
  console.error(e)
  process.exit(1)
})
