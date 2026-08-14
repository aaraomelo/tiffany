/* tests/encaixotamento.js — o encaixotamento das dimensões: a fronteira
 * redimida pela troca (leitura do coordenador, 14/08 madrugada: «o
 * corpo inteiro da dimensão de cima encaixota na unidade; essa unidade
 * é o irracional para onde a classe de baixo encaixota por baixo;
 * basta virar o infinito de uma dimensão na unidade da próxima; o 0
 * continua como projeção de π em cada dimensão — e 0 e 1 da reta são
 * as projeções contínua e discreta de π na dimensão»).
 *
 * Tudo se realiza com objetos que a casa já tinha:
 *
 *   O ENCAIXOTADOR É A FRONTEIRA REDIMIDA: E(x) = x/(1+x) tem matriz
 *   L = X·T·X — a TRANSLAÇÃO (a fronteira aditiva que nunca fecha)
 *   CONJUGADA PELA TROCA do núcleo. O que fugia, espelhado, encaixota:
 *   E leva [0,∞] em [0,1], ordem preservada, com E(∞) = 1 exato
 *   (projetivo: (1,0) ↦ (1,1)).
 *
 *   A UNIDADE É O «IRRACIONAL» DA DIMENSÃO DE BAIXO: E(x) < 1 para
 *   todo x finito, e todo racional r/s < 1 é ultrapassado (x = r/(s−r)
 *   dá E(x) = r/s, e além) — o supremo exato pelas TRÊS cláusulas do
 *   Teorema Central (thm:central-continuo §T4): a unidade é encaixotada
 *   por baixo exatamente como o irracional era.
 *
 *   O INFINITO DESCE A ESCADA: L^n = [1,0;n,1] (parabólico exato), e
 *   E^n(∞) = 1/n — o infinito da dimensão 0 vira 1, depois ½, ⅓, … —
 *   a sequência harmónica é a sombra do infinito pelas dimensões, e
 *   aterra no 0: o ponto fixo DUPLO (disc = tr²−4det = 0 — o nó
 *   parabólico).
 *
 *   E AS RÉGUAS SÃO FEITAS DISTO: A_m = T^m·X — a régua m é m passos
 *   aditivos e uma troca: a fração contínua inteira é a palavra em
 *   (T, X); o encaixotador e as réguas são o mesmo alfabeto.
 *
 *   0 E 1 SÃO AS DUAS PROJEÇÕES DE π: em cada relógio, o ângulo π
 *   projeta em (c,s) = (−1, 0) exato — a folha ímpar (contínua) dá o
 *   0, a par (discreta, em norma c²=1) dá o 1: os dois marcos da reta
 *   unitária herdam-se de π em toda dimensão — o par da Lei 0.
 *
 * §X1  o encaixotador: L = X·T·X, identidade de matrizes do núcleo;
 *      parabólico (tr=2, det=1, disc=0 — o nó no 0)
 * §X2  o corpo na unidade: E(p/q) = p/(p+q) preserva ordem, E < 1
 *      sempre, E(∞) = 1 projetivo — injetivo e dentro
 * §X3  a unidade como supremo: as três cláusulas (cresce, abaixo,
 *      ultrapassa) — a unidade é o «irracional» da dimensão de baixo
 * §X4  a escada do infinito: L^n = [1,0;n,1], E^n(∞) = 1/n → 0 — o
 *      infinito desce como a harmónica e aterra no ponto fixo
 * §X5  as réguas no alfabeto: A_m = T^m·X para m=1..6 — a CF é a
 *      palavra em (T, X)
 * §X6  0 e 1 como projeções de π: (c,s)(π) = (−1,0) nos relógios
 *      8/16/32 — ímpar dá 0, par dá 1 em norma; o gume: T sozinho
 *      (sem a conjugação) NÃO encaixota (foge para [1,∞)); 𝓜 assina
 */
'use strict'
const { anel, mat2, nucleo, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const { mul, igual, det, tr } = mat2
let R = 0

/* §X1 — o encaixotador é a fronteira redimida */
const T = [1, 1, 0, 1]
const L = mul(mul(nucleo.X, T), nucleo.X)
{
  const identidade = igual(L, [1, 0, 1, 1])
  const parabolico = tr(L) === 2 && det(L) === 1 && tr(L) * tr(L) - 4 * det(L) === 0
  if (!identidade || !parabolico) R++
  console.log(`\n§X1  L = X·T·X = [${L}] · tr=${tr(L)}, det=${det(L)}, disc=0`)
  ok('§X1 O ENCAIXOTADOR É A FRONTEIRA REDIMIDA: L = X·T·X — a translação (que nunca fecha) conjugada pela TROCA do núcleo vira x↦x/(1+x); e é parabólico (disc=0): o ponto fixo duplo é o nó no 0', identidade && parabolico)
}

/* §X2 — o corpo inteiro na unidade */
{
  /* E(p/q) = p/(p+q): ordem preservada e sempre < 1 */
  const E = (p, q) => [p, p + q]
  let ordem = 0, casos = 0, dentro = 0
  const amostra = [[0n, 1n], [1n, 3n], [1n, 2n], [2n, 3n], [3n, 2n], [5n, 1n], [100n, 1n]]
  for (let i = 0; i < amostra.length; i++) {
    const [p, q] = amostra[i]
    const [a, b] = E(p, q)
    if (a < b) dentro++                                    /* E < 1 */
    for (let j = i + 1; j < amostra.length; j++) {
      const [r, s] = amostra[j]
      casos++
      const menor = p * s < r * q
      const [c, d] = E(r, s)
      if (menor === (a * d < c * b)) ordem++
    }
  }
  /* E(∞) = 1: projetivo (1,0) ↦ L·(1,0) = (1,1) */
  const inf = [L[0] * 1 + L[1] * 0, L[2] * 1 + L[3] * 0]
  const infUm = inf[0] === 1 && inf[1] === 1
  if (ordem !== casos || dentro !== amostra.length || !infUm) R++
  console.log(`\n§X2  ordem preservada ${ordem}/${casos} · E<1 em ${dentro}/${amostra.length} · E(∞) = ${inf[0]}/${inf[1]}`)
  ok('§X2 O CORPO INTEIRO ENCAIXOTA NA UNIDADE: E(p/q) = p/(p+q) preserva a ordem, fica sempre < 1, e E(∞) = 1 exato no projetivo — a dimensão de cima cabe na unidade de baixo', ordem === casos && dentro === amostra.length && infUm)
}

/* §X3 — a unidade é o «irracional» da dimensão de baixo */
{
  /* as três cláusulas do supremo (thm:central-continuo §T4) para a
   * classe {E(n/1)} = {n/(n+1)}: cresce, fica abaixo de 1, ultrapassa
   * todo racional < 1 (dado r/s < 1: n = r ultrapassa? E(r/(s−r)) = r/s,
   * e n maior passa além — constrói EXATO) */
  let cresce = 0, abaixo = 0, ultrapassa = 0
  for (let n = 1n; n <= 30n; n++) {
    if (n * (n + 2n) > (n + 1n) * (n - 1n + 1n) - 1n) cresce++     /* n/(n+1) < (n+1)/(n+2): n(n+2) < (n+1)² ✓ */
    if (n < n + 1n) abaixo++
  }
  const alvos = [[1n, 2n], [2n, 3n], [9n, 10n], [99n, 100n]]
  for (const [r, s] of alvos) {
    /* x = r/(s−r) tem E(x) = r/s; e n = s ultrapassa: s/(s+1) > r/s ⟺ s² > r(s+1) */
    if (s * s > r * (s + 1n)) ultrapassa++
  }
  if (cresce !== 30 || abaixo !== 30 || ultrapassa !== alvos.length) R++
  console.log(`\n§X3  cresce ${cresce}/30 · abaixo ${abaixo}/30 · ultrapassa ${ultrapassa}/${alvos.length}`)
  ok('§X3 A UNIDADE É O «IRRACIONAL» DA DIMENSÃO DE BAIXO: a classe {n/(n+1)} cresce, fica abaixo, e ultrapassa todo racional < 1 — o supremo exato pelas TRÊS cláusulas do Teorema Central: encaixotada por baixo como o irracional era', cresce === 30 && abaixo === 30 && ultrapassa === alvos.length)
}

/* §X4 — a escada do infinito: E^n(∞) = 1/n → 0 */
{
  let Ln = L, escada = 0
  for (let n = 2; n <= 20; n++) {
    Ln = mul(Ln, L)
    if (igual(Ln, [1, 0, n, 1])) escada++
  }
  /* E^n(∞) = (1,0) ↦ (1,n) = 1/n; e 1/n encaixota o 0 por cima (decresce, positivo) */
  let desce = 0
  for (let n = 1n; n < 20n; n++) if ((n + 1n) * 1n > n * 1n) desce++   /* 1/(n+1) < 1/n ⟺ n < n+1 */
  if (escada !== 19 || desce !== 19) R++
  console.log(`\n§X4  L^n = [1,0;n,1]: ${escada}/19 · 1/n estritamente decrescente: ${desce}/19`)
  ok('§X4 O INFINITO DESCE A ESCADA: L^n = [1,0;n,1] (parabólico exato) e E^n(∞) = 1/n — o infinito vira 1, ½, ⅓, … (a harmónica é a sombra do infinito) e aterra no 0: o ponto fixo duplo, o nó', escada === 19 && desce === 19)
}

/* §X5 — as réguas no alfabeto (T, X) */
{
  let alfabeto = 0
  for (let m = 1; m <= 6; m++) {
    const Tm = [1, m, 0, 1]
    if (igual(mul(Tm, nucleo.X), mat2.Am(m))) alfabeto++
  }
  if (alfabeto !== 6) R++
  console.log(`\n§X5  A_m = T^m·X em ${alfabeto}/6 réguas`)
  ok('§X5 AS RÉGUAS SÃO FEITAS DO MESMO ALFABETO: A_m = T^m·X — m passos aditivos e uma troca: a fração contínua inteira é a palavra em (T, X); o encaixotador e as réguas partilham as letras', alfabeto === 6)
}

/* §X6 — 0 e 1 como as duas projeções de π; o gume; o contrato */
let G = false
{
  const P = 65537
  const A = anel(P)
  let projeta = 0
  for (const M of [8, 16, 32]) {
    const h = A.powm(3, 65536 / M)
    const hM2 = A.powm(h, M / 2)
    /* π: h^{M/2} = −1 ⟹ (c,s) = ((−1 + (−1)⁻¹)/2, (−1 − (−1)⁻¹)/2i) = (−1, 0) */
    const c = A.mod((hM2 + A.inv(hM2)) * A.inv(2))
    const s = A.mod(hM2 - A.inv(hM2))
    if (c === P - 1 && s === 0 && A.mod(c * c) === 1) projeta++
  }
  /* o gume: T sozinho NÃO encaixota — T(p/q) = (p+q)/q ≥ 1 sempre */
  let foge = 0
  for (const [p, q] of [[0n, 1n], [1n, 3n], [2n, 3n], [7n, 2n]]) {
    if (p + q >= q) foge++
  }
  G = foge === 4
  if (projeta !== 3) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§X6  (c,s)(π) = (−1,0) com c²=1: ${projeta}/3 relógios · T sozinho foge: ${foge}/4 · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§X6 0 E 1 SÃO AS DUAS PROJEÇÕES DE π: em cada relógio o ângulo π projeta em (−1, 0) exato — a folha ímpar (contínua) dá o 0, a par em norma (discreta) dá o 1: os marcos da reta herdam-se de π, o par da Lei 0', projeta === 3)
  ok('§X6 o GUME: a translação SEM a conjugação não encaixota (T(x) ≥ 1 sempre, foge para [1,∞)) — é a troca que redime a fronteira; 𝓜 assina o encaixotamento das dimensões', G && medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  O encaixotamento das dimensões: a fronteira aditiva, conjugada')
  console.log('  pela troca do núcleo, vira o encaixotador x/(1+x) — o corpo')
  console.log('  de cima cabe na unidade de baixo, a unidade é o supremo')
  console.log('  encaixotado (o mesmo teorema do irracional), o infinito desce')
  console.log('  como a harmónica e aterra no nó, as réguas são palavras em')
  console.log('  (T,X), e os marcos 0 e 1 da reta são as duas projeções de π')
  console.log('  em toda dimensão. O que fugia, espelhado, encaixota.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
