/* tests/medicao_normativa.js — a meta-medição: o contrato normativo da
 * medição, medido ele próprio (ordem da mesa, eval 14/08: «promover a
 * ferramenta de medição no sistema»; condição do analista: a ferramenta
 * só sobe COM medidor dela mesma).
 *
 * O contrato (lib/universal.js, medicao):
 *
 *     𝓜(O) = (R, G, V)   e   fecha(𝓜) ⟺ R=0 ∧ G ∧ V=0
 *
 * R = o invariante (resíduo da afirmação); G = o GUME (o contra-caso
 * construído FALHOU como previsto); V = a volta (reconstrução exata).
 * O critério é mais forte que «deu zero»: um teste que só sabe passar
 * não atesta — o falso verde do cards (14/08) tinha R=0 e G ausente.
 *
 * §Q0  a tabela completa: dos 8 cantos (R∈{0,1}, G∈{V,F}, V∈{0,1}),
 *      EXATAMENTE UM fecha — o contrato não tem canto morto
 * §Q1  o contrato numa medição REAL: as cinco relações do núcleo como
 *      R, a quádrupla adulterada como G, a dupla conjugação como V —
 *      fecha; e cada componente sabotado reabre
 * §Q2  as formas históricas de falso verde são RECUSADAS: (0, sem
 *      gume, 0) é a forma do cards; (0, G, V≠0) é a volta que não
 *      fecha — nenhuma passa
 * §Q3  o gume do gume: G truthy que não é true ('sim', 1) NÃO fecha —
 *      o relato desleixado do gume é recusado pelo tipo
 */
'use strict'
const { medicao, nucleo, mat2 } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

/* §Q0 — a tabela completa */
{
  let fecham = 0, oQueFecha = null
  for (const R of [0, 1]) {
    for (const G of [true, false]) {
      for (const V of [0, 1]) {
        if (medicao.fecha(medicao.contrato(R, G, V))) { fecham++; oQueFecha = [R, G, V] }
      }
    }
  }
  console.log(`\n§Q0  dos 8 cantos fecha ${fecham}: ${JSON.stringify(oQueFecha)}`)
  ok('§Q0 dos 8 cantos da tabela EXATAMENTE UM fecha: (R=0, G=true, V=0) — o contrato não tem canto morto', fecham === 1 && oQueFecha.join() === '0,true,0')
}

/* §Q1 — o contrato numa medição real: o núcleo */
{
  const { mul, escala, igual } = mat2
  const { X, S, H, J } = nucleo
  /* R: as cinco relações do núcleo, contadas como resíduo */
  let R = 0
  if (!igual(X, mul(S, J))) R++
  if (!igual(mul(H, H), escala(2, mat2.I))) R++
  if (!igual(mul(mul(H, X), H), escala(2, S))) R++
  if (!igual(mul(mul(H, S), H), escala(2, X))) R++
  if (!igual(mul(mul(H, J), H), escala(-2, J))) R++
  /* G: a quádrupla adulterada FALHA como previsto */
  const Xmau = [0, 1, -1, 0]
  const G = !igual(Xmau, mul(S, J))
  /* V: a volta — a dupla conjugação por H devolve 4·A, exata, em 20 matrizes */
  let V = 0
  let lcg = 13
  const rnd = () => { lcg = (lcg * 75 + 74) % 65537; return (lcg % 21) - 10 }
  for (let t = 0; t < 20; t++) {
    const A = [rnd(), rnd(), rnd(), rnd()]
    if (!igual(mul(mul(H, mul(mul(H, A), H)), H), escala(4, A))) V++
  }
  const m = medicao.contrato(R, G, V)
  console.log(`\n§Q1  núcleo: R=${R}, G=${m.G}, V=${V} → fecha: ${medicao.fecha(m)}`)
  ok('§Q1 o contrato fecha na medição real do núcleo: R=0 (cinco relações), G (a adulterada falha), V=0 (a dupla conjugação volta 4·A em 20 matrizes)', medicao.fecha(m))
  ok('§Q1 e cada componente sabotado REABRE: R=1, ou G ausente, ou V=1 — nenhum fecha', !medicao.fecha(medicao.contrato(1, G, V)) && !medicao.fecha(medicao.contrato(R, false, V)) && !medicao.fecha(medicao.contrato(R, G, 1)))
}

/* §Q2 — as formas históricas de falso verde, recusadas */
{
  const formaCards = medicao.contrato(0, false, 0)     /* R=0 e nenhum gume: o cards de 14/08 */
  const semVolta = medicao.contrato(0, true, 3)        /* a volta que não fecha */
  console.log(`\n§Q2  a forma do cards (R=0, sem gume): fecha? ${medicao.fecha(formaCards)} · sem volta: ${medicao.fecha(semVolta)}`)
  ok('§Q2 a forma do falso verde histórico é RECUSADA: R=0 sem gume não fecha — um teste que só sabe passar não atesta', !medicao.fecha(formaCards))
  ok('§Q2 e a volta é obrigatória: R=0 com gume mas V≠0 não fecha — reconstruir é parte da medida', !medicao.fecha(semVolta))
}

/* §Q3 — o gume do gume: o tipo é estrito */
{
  const desleixos = ['sim', 1, 'true', {}, []].map(g => medicao.fecha(medicao.contrato(0, g, 0)))
  console.log(`\n§Q3  G desleixado ('sim', 1, 'true', {}, []): ${JSON.stringify(desleixos)}`)
  ok('§Q3 o gume do gume: G que não é `true` estrito (truthy desleixado) NÃO fecha — o relato do contra-caso é recusado pelo tipo', desleixos.every(f => f === false))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A ferramenta de medição é infraestrutura: fecha(𝓜) ⟺')
  console.log('  R=0 ∧ G ∧ V=0 — a afirmação, o seu contra-caso e a volta,')
  console.log('  os três obrigatórios. A simetria com o produto dual é de')
  console.log('  desenho: o espelho no índice conserva a informação; o gume')
  console.log('  na medição conserva a falsificabilidade.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
