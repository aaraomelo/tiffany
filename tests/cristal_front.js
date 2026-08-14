/* tests/cristal_front.js — chip X (xtal): a proveniência do cristal na UI.
 *   node tests/cristal_front.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

;(async () => {
  const { pedeCristal, extraiProveniencia, assinatura, rejeitaPrimeiroErro, passoCristal } =
    await import('../app/src/cristal.js')
  console.log('=== CRISTAL FRONT (chip X) ===\n')

  /* §X0 — a fala que pede */
  ok('§X0 pede: mostra o cristal', pedeCristal('mostra o cristal'))
  ok('§X0 pede: de onde veio', pedeCristal('de onde veio o reticulado'))
  ok('§X0 pede: proveniência', pedeCristal('qual a proveniência disto'))
  ok('§X0 não pede: bom dia', !pedeCristal('bom dia'))

  /* §X1 — a proveniência real (o formato que o ingere leva ao banco) */
  const resposta = 'Estrutura parcialmente ordenada em que join e meet existem. ' +
    'proveniência: manual fato confiança 1.0 domínio matematica história 160 versões no jornal'
  const prov = extraiProveniencia(resposta)
  ok('§X1 extrai origem', prov && prov.origem === 'manual fato')
  ok('§X1 extrai confiança', prov && prov.confianca === 1.0)
  ok('§X1 extrai domínio', prov && prov.dominio === 'matematica')
  ok('§X1 extrai história', prov && prov.versoes === 160)
  ok('§X1 sem proveniência → null', extraiProveniencia('resposta banal') === null)

  /* §X2 — o passo do chip */
  const cx = passoCristal('de onde veio o reticulado', resposta)
  ok('§X2 chip ok com proveniência', cx && cx.ok && cx.prov.versoes === 160)
  const cx2 = passoCristal('de onde veio isto', 'não sei')
  ok('§X2 pediu e não há → ok=false, sem inventar', cx2 && !cx2.ok)
  ok('§X2 não pediu e não há → chip quieto', passoCristal('bom dia', 'Bom dia!') === null)

  /* §X3 — a assinatura: energia + fase (a fase apanha a transposição) */
  const a = assinatura('abc')
  const b = assinatura('acb')     /* transposição: mesma energia, outra fase */
  ok('§X3 transposição conserva a energia', a.E === b.E)
  ok('§X3 e a FASE acusa', a.fase !== b.fase)

  /* §X4 — o portão do coordenador: o primeiro byte errado rejeita */
  const g1 = rejeitaPrimeiroErro('cristal', 'cristal')
  const g2 = rejeitaPrimeiroErro('cristal', 'crisTal')
  ok('§X4 iguais passam', g1.ok && g1.pos === -1)
  ok('§X4 primeiro erro na posição 4, rejeita já', !g2.ok && g2.pos === 4)

  console.log('')
  if (!falhas) {
    console.log('  Chip X: o recibo da história na tela — origem · domínio · confiança ·')
    console.log('  versões; assinatura energia+fase; o primeiro byte errado rejeita.')
  }
  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})()
