/* tests/paracomplexo.js — a estrutura das folhas reais: a carta
 * para-complexa (ordem do coordenador, 14/08 madrugada: «resolve a
 * observação do teorema 14» — a obs:riemann terminava recusando a
 * régua errada: «não se afirma estrutura holomorfa sobre as folhas
 * reais — lá a forma é a²+mab−b², indefinida». A recusa estava certa;
 * agora dá-se às folhas a estrutura CERTA, que o dia construiu).
 *
 * A resolução: as folhas reais têm a carta PARA-COMPLEXA (I, W), dual
 * exata da holomorfa (I, J) da torção:
 *
 *   HOLOMORFA (torção)      J² = −I        det(aI+bJ) = a²+b²
 *   PARA-COMPLEXA (folhas)  W² = +(m²+4)I  det(aI+bW) = a²−(m²+4)b²
 *
 *   O CORPO VIVE DENTRO DA CARTA: 2A = mI + W, inteiro exato — a
 *   direção das folhas É a direção W, e a forma diagonal é a que a
 *   carta W de geometria_corpo §G2 já tinha dado.
 *
 *   A ESTACA É A CONJUGAÇÃO DA CARTA: x ↦ x† em (I,A) corresponde
 *   exatamente a (a,b) ↦ (a,−b) em (I,W) — a involução da álgebra e a
 *   da carta são a mesma, byte a byte.
 *
 *   AS DUAS ESTRUTURAS SÃO TRANSVERSAIS: span(I,J) ∩ span(I,W) =
 *   span(I) — partilham só a unidade (posto medido).
 *
 *   E A RÉGUA ERRADA É IMPOSSÍVEL, NÃO APENAS DESACONSELHADA:
 *   (bW)² = −I exigiria b²(m²+4) = −1 — sem solução em ℚ (um quadrado
 *   vezes um positivo nunca é negativo). O círculo NÃO cabe na direção
 *   das folhas; a obs:riemann tinha razão, e agora tem teorema.
 *
 * §W0  a carta fecha: (I,W) comuta, multiplica exato, e 2A = mI+W —
 *      o corpo dentro dela, m=1..6
 * §W1  a forma da carta: det(aI+bW) = a² − (m²+4)b², multiplicativa
 *      (Lei 7 na carta), amostra exata
 * §W2  a estaca é a conjugação: x† em (I,A) == (a,−b) em (I,W),
 *      byte a byte na amostra
 * §W3  transversais: span(I,J) ∩ span(I,W) = span(I) (posto 3 do
 *      conjunto {I,J,W}, e nenhuma combinação de J entra em (I,W))
 * §W4  o gume: b²(m²+4) = −1 sem solução racional (denominadores até
 *      40, e o argumento de sinal exato) — a régua errada é impossível;
 *      𝓜 assina a resolução
 */
'use strict'
const { mat2, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const { mul, soma, escala, igual, det, tr, I, J } = mat2
let R = 0

/* §W0 — a carta fecha e o corpo vive dentro */
{
  let comuta = 0, dentro = 0, casos = 0
  for (let m = 1; m <= 6; m++) {
    const W = mat2.W(m)
    if (igual(escala(2, mat2.Am(m)), soma(escala(m, I), W))) dentro++
    for (const [a, b, c, d] of [[3, 5, -2, 7], [1, 0, 0, 1], [4, -9, 6, 2]]) {
      casos++
      const X = soma(escala(a, I), escala(b, W))
      const Y = soma(escala(c, I), escala(d, W))
      if (igual(mul(X, Y), mul(Y, X))) comuta++
    }
  }
  if (dentro !== 6 || comuta !== casos) R++
  console.log(`\n§W0  2A = mI+W em ${dentro}/6 metais · a carta comuta em ${comuta}/${casos}`)
  ok('§W0 a carta (I,W) fecha e comuta, e o corpo vive DENTRO dela: 2A = mI + W, inteiro exato nos seis metais', dentro === 6 && comuta === casos)
}

/* §W1 — a forma da carta, multiplicativa */
{
  let forma = 0, multiplica = 0, casos = 0
  for (let m = 1; m <= 6; m++) {
    const W = mat2.W(m)
    const D = m * m + 4
    for (const [a, b, c, d] of [[3, 5, -2, 7], [4, -9, 6, 2], [11, -3, 1, 8]]) {
      casos++
      const X = soma(escala(a, I), escala(b, W))
      const Y = soma(escala(c, I), escala(d, W))
      if (det(X) === a * a - D * b * b) forma++
      if (det(mul(X, Y)) === det(X) * det(Y)) multiplica++
    }
  }
  if (forma !== casos || multiplica !== casos) R++
  console.log(`\n§W1  det(aI+bW) = a²−(m²+4)b²: ${forma}/${casos} · multiplicativa: ${multiplica}/${casos}`)
  ok('§W1 a forma da carta é a diagonal a² − (m²+4)b² — a de geometria_corpo §G2 — e é multiplicativa: a Lei 7 na carta para-complexa', forma === casos && multiplica === casos)
}

/* §W2 — a estaca é a conjugação da carta */
{
  const dag = M => { const t = tr(M); return [t - M[0], -M[1], -M[2], t - M[3]] }
  let bate = 0, casos = 0
  for (let m = 1; m <= 6; m++) {
    const W = mat2.W(m)
    for (const [c, d] of [[7, 3], [-4, 5], [0, 1], [12, -8]]) {
      casos++
      /* x = c + dA ⟹ 2x = (2c+dm)I + dW; a estaca deve dar (2c+dm)I − dW */
      const x2 = soma(escala(2 * c + d * m, I), escala(d, W))
      const alvo = soma(escala(2 * c + d * m, I), escala(-d, W))
      if (igual(dag(x2), alvo)) bate++
    }
  }
  if (bate !== casos) R++
  console.log(`\n§W2  estaca == conjugação da carta em ${bate}/${casos}`)
  ok('§W2 a ESTACA é a conjugação da carta: x† em (I,A) corresponde a (a,b)↦(a,−b) em (I,W), byte a byte — a mesma involução, duas leituras', bate === casos)
}

/* §W3 — as duas estruturas são transversais */
{
  /* posto de {I, J, W} como vetores de R⁴: 3 (independentes) — e J ∉ span(I,W) */
  let transversal = 0
  for (let m = 1; m <= 6; m++) {
    const W = mat2.W(m)
    /* J = αI + βW? entrada (0,1): 1 = β·W01; entrada (1,0): −1 = β·W10; W01=W10=2 ⟹ β=1/2 e β=−1/2 — impossível */
    const impossivel = !(W[1] !== 0 && W[2] !== 0 && (J[1] * W[2] === J[2] * W[1]))
    if (impossivel) transversal++
  }
  if (transversal !== 6) R++
  console.log(`\n§W3  J fora de span(I,W) nos ${transversal}/6 metais (J01/W01 ≠ J10/W10)`)
  ok('§W3 as duas estruturas são TRANSVERSAIS: J não é combinação de (I,W) — holomorfa e para-complexa partilham só a unidade', transversal === 6)
}

/* §W4 — o gume: a régua errada é impossível */
{
  /* (bW)² = −I ⟺ b²(m²+4) = −1: sem solução racional. Dois caminhos:
   * (1) a varredura exata de b = p/q com p,q ≤ 40; (2) o argumento de
   * sinal: p²(m²+4) > 0 ≠ −q² < 0 — a igualdade cruzada nunca fecha */
  let varre = true, sinal = true
  for (let m = 1; m <= 6; m++) {
    const D = m * m + 4
    for (let p = 1; p <= 40; p++) {
      for (let q = 1; q <= 40; q++) {
        if (p * p * D === -q * q) varre = false          /* nunca: positivo = negativo */
        if (!(p * p * D > 0 && -q * q < 0)) sinal = false
      }
    }
  }
  const G = varre && sinal
  const V = 0
  const m2 = medicao.contrato(R, G, V)
  console.log(`\n§W4  b²(m²+4) = −1 sem solução (varredura + sinal): ${G} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(m2)}`)
  ok('§W4 o GUME: a régua errada é IMPOSSÍVEL, não apenas desaconselhada — b²(m²+4) = −1 não tem solução racional (o círculo não cabe na direção das folhas)', G)
  ok('§W4 o contrato 𝓜 assina a resolução da obs:riemann: as folhas reais têm a estrutura PARA-COMPLEXA (I,W), dual exata da holomorfa (I,J) da torção', medicao.fecha(m2))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A observação do teorema 14 está resolvida: a recusa da régua')
  console.log('  errada virou teorema (o círculo não cabe nas folhas), e a')
  console.log('  régua certa chegou — a carta para-complexa (I,W), onde o')
  console.log('  corpo vive (2A=mI+W), a forma é a diagonal do produto dual,')
  console.log('  e a estaca é a conjugação. Holomorfa na torção, para-complexa')
  console.log('  nas folhas: o par completo, transversal, sem confusão.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
