/* tests/estacao_front.js — volta contra o mundo na UI.
 *   node tests/estacao_front.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

;(async () => {
  const { residualEstacao, estacaoFecha, passoEstacao, documentoImplante, parseImplante, pedeBanco } = await import('../app/src/estacao.js')
  console.log('=== ESTACAO FRONT (UI) ===\n')
  ok('§E0 R=0', residualEstacao(65, 65) === 0)
  ok('§E0 R=25', residualEstacao(65, 40) === 25)
  ok('§E1 fecha só com MES', estacaoFecha(65, 65, 1) && !estacaoFecha(65, 65, 0))
  const d = passoEstacao('mostra a estação')
  ok('§E2 demo', d && d.ok && d.R === 0)
  const g = passoEstacao('lead:65 estacao:40')
  ok('§E2 desvio', g && !g.ok && g.R === 25)
  ok('§E3 ignora banal', passoEstacao('bom dia') === null)
  {
    const doc = documentoImplante({ proj: 65, meas: 40, external: 1, id: '04' })
    const back = parseImplante(doc.resposta)
    ok('§E4 emit∘parse=id', back && back.proj === 65 && back.meas === 40 && back.R === 25 && back.external === 1)
    ok('§E4 fala no banco', doc.fala === 'implante 04')
    ok('§E5 pede banco', pedeBanco('mostra o banco') && pedeBanco('o que e a volta no banco') && !pedeBanco('mostra a estação'))
  }
  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((e) => { console.error(e); process.exit(1) })
