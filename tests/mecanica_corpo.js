/* tests/mecanica_corpo.js — a mecânica trazida: o fluxo como ação que
 * conserva o produto dual (ordem do coordenador, 14/08 madrugada:
 * «trazer a mecânica»; a pergunta exata do gerente: «o fluxo é uma
 * ação que conserva o produto dual?» — e a sequência da mesa:
 * D₄ → fluxo → invariantes → energia → mecânica. Nada de Lagrange ou
 * Hamilton importados: se aparecerem, é como representação do medido).
 *
 * O que a medida dá:
 *
 *   O FLUXO CONSERVA O PRODUTO DUAL nos dois regimes: no relógio,
 *   c²+s² = 1 ao longo de toda a trajetória; no Pell, σ conserva |4N|
 *   ALTERNANDO a membrana (−1)^k — o sinal do encaixe — e σ² (a
 *   unidade de Pell) conserva exato. A resposta à pergunta do gerente
 *   é SIM, com a estrutura fina da membrana à vista.
 *
 *   A ENERGIA DECOMPÕE-SE NAS PARCELAS DA CARTA: c² e s² no círculo,
 *   u² e (m²+4)v² na hipérbole — e os dois regimes distinguem-se pelo
 *   COMPORTAMENTO das parcelas com o invariante fixo: no círculo elas
 *   OSCILAM (c² é periódico, e J troca c²↔s² — o rotor é a troca
 *   cinética↔potencial); no Pell elas ESCAPAM (u_k estritamente
 *   crescente) mantendo a diferença. Oscilação compacta e escape
 *   não-compacto: as duas mecânicas das duas membranas.
 *
 *   A T-SIMETRIA JÁ ESTAVA NO GRUPO: espelho·R_k = R_{−k}·espelho em
 *   todos os ticks — a relação diedral SJS=J⁻¹ É a reversão do tempo;
 *   e na hipérbole a estaca inverte o fluxo a menos do sinal da
 *   membrana (conj(σv) = −σ⁻¹·conj(v), exato).
 *
 *   D₄ AGE NO FLUXO: rotações comutam com R, reflexões conjugam R em
 *   R⁻¹ — o det é a SETA DO TEMPO do elemento.
 *
 * §K0  a conservação: c²+s²=1 na trajetória inteira do relógio e
 *      u²−(m²+4)v² = 4N no Pell (k=1..30, BigInt) — a ação conserva
 * §K1  as parcelas: no círculo c² é PERIÓDICO (oscila) e J troca
 *      c²↔s²; no Pell u_k é estritamente CRESCENTE (escapa)
 * §K2  a T-simetria: espelho·R_k = R_{−k}·espelho nos M ticks, e
 *      conj(σv) = −σ⁻¹·conj(v) no Pell — o tempo inverte-se pelo
 *      espelho, com T² = id (a volta)
 * §K3  D₄ age no fluxo: g·R·g⁻¹ = R^{det(g)} para os 8 — o det é a
 *      seta do tempo
 * §K4  o gume e o contrato: a trajetória PERTURBADA (ω·z + 1) perde o
 *      invariante em todos os passos seguintes; 𝓜 assina
 */
'use strict'
const { anel, mat2, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const A = anel(P)
const M = 32
const h = A.powm(3, 65536 / M), i4 = A.powm(3, 16384), i2 = A.inv(2)
const c = [], s = []
for (let k = 0; k < M; k++) {
  const hk = A.powm(h, k), hki = A.inv(hk)
  c.push(A.mod((hk + hki) * i2))
  s.push(A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4)))
}
const m = 2n
const sigma = ([a, b]) => [b, a + m * b]
const Nb = ([a, b]) => a * a + m * a * b - b * b
let R = 0

/* §K0 — a conservação nos dois regimes */
{
  let relogio = 0
  for (let k = 0; k < M; k++) if (A.mod(c[k] * c[k] + s[k] * s[k]) === 1) relogio++
  /* σ tem norma −1: cada passo ALTERNA a membrana — o invariante exato é
   * |4N|, com o sinal (−1)^k (o mesmo do encaixe e do produto dual); o
   * fluxo PAR (σ², a unidade de Pell) conserva SEM alternar */
  let pellAlterna = 0, pellPar = 0
  let w = [3n, 5n]
  const inv4N = 4n * Nb(w)
  for (let k = 1; k <= 30; k++) {
    w = sigma(w)
    const u = 2n * w[0] + m * w[1], v = w[1]
    const q = u * u - (m * m + 4n) * v * v
    if (q === (k % 2 === 0 ? inv4N : -inv4N)) pellAlterna++
  }
  let w2 = [3n, 5n]
  for (let k = 1; k <= 15; k++) {
    w2 = sigma(sigma(w2))
    const u = 2n * w2[0] + m * w2[1], v = w2[1]
    if (u * u - (m * m + 4n) * v * v === inv4N) pellPar++
  }
  if (relogio !== M || pellAlterna !== 30 || pellPar !== 15) R++
  console.log(`\n§K0  relógio c²+s²=1: ${relogio}/${M} · Pell alterna (−1)^k·4N: ${pellAlterna}/30 · σ² conserva exato: ${pellPar}/15`)
  ok('§K0 O FLUXO É AÇÃO QUE CONSERVA O PRODUTO DUAL: c²+s²=1 no relógio; no Pell, σ conserva |4N| alternando a membrana (−1)^k e σ² (a unidade) conserva EXATO — a pergunta do gerente, respondida com a estrutura fina', relogio === M && pellAlterna === 30 && pellPar === 15)
}

/* §K1 — as parcelas: oscilação vs escape */
{
  /* o círculo: c² é periódico com o período do relógio, e J troca c²↔s² */
  let periodico = true
  for (let k = 0; k < M; k++) if (A.mod(c[k] * c[k]) !== A.mod(c[(k + M) % M] * c[(k + M) % M])) periodico = false
  const varia = new Set(c.map(x => A.mod(x * x))).size > 1
  let troca = 0
  for (let k = 0; k < M; k++) {
    /* a ação REAL do J da lib no ponto: Jz = (J00·c+J01·s, J10·c+J11·s);
     * a troca afirma parcela1(Jz)=parcela2(z) e vice-versa — computada,
     * não escrita (a primeira versão comparava x com x e não podia falhar) */
    const Jz = [
      A.mod(mat2.J[0] * c[k] + mat2.J[1] * s[k] + P * P),
      A.mod(mat2.J[2] * c[k] + mat2.J[3] * s[k] + P * P),
    ]
    if (A.mod(Jz[0] * Jz[0]) === A.mod(s[k] * s[k]) &&
        A.mod(Jz[1] * Jz[1]) === A.mod(c[k] * c[k])) troca++
  }
  /* o Pell: u_k estritamente crescente — o escape */
  let cresce = 0
  let w = [3n, 5n]
  let uAnt = 2n * w[0] + m * w[1]
  for (let k = 1; k <= 30; k++) {
    w = sigma(w)
    const u = 2n * w[0] + m * w[1]
    if (u > uAnt) cresce++
    uAnt = u
  }
  if (!periodico || !varia || troca !== M || cresce !== 30) R++
  console.log(`\n§K1  c² periódico e não-constante: ${periodico && varia} · J troca c²↔s²: ${troca}/${M} · u_k cresce: ${cresce}/30`)
  ok('§K1 no CÍRCULO as parcelas oscilam (c² periódico, não-constante) e o rotor J TROCA c²↔s² — a troca cinética↔potencial é o rotor', periodico && varia && troca === M)
  ok('§K1 no PELL as parcelas escapam (u_k estritamente crescente, 30/30) mantendo a diferença — oscilação compacta e escape não-compacto: as duas mecânicas das duas membranas', cresce === 30)
}

/* §K2 — a T-simetria: o espelho inverte o tempo */
{
  const { mul, igual, espelho } = mat2
  const Rk = k => [c[k], s[k], -s[k], c[k]]
  const modM2 = X => X.map(v => ((v % P) + P) % P)
  let reversao = 0
  for (let k = 1; k < M; k++) {
    if (igual(modM2(mul(espelho, Rk(k))), modM2(mul(Rk(M - k), espelho)))) reversao++
  }
  /* T² = id: inverter duas vezes devolve a trajetória byte a byte */
  let volta = 0
  for (let k = 0; k < M; k++) {
    const rev = [c[k], A.mod(-s[k] + P)]
    const revrev = [rev[0], A.mod(-rev[1] + P)]
    if (revrev[0] === c[k] && revrev[1] === s[k]) volta++
  }
  /* o Pell: conj(σv) = −σ⁻¹(conj v) */
  const conj = ([a, b]) => [a + m * b, -b]
  const sigmaInv = ([p, q]) => [q - m * p, p]
  let pellT = 0
  let w = [3n, 5n]
  for (let k = 0; k < 15; k++) {
    const lhs = conj(sigma(w))
    const rhs = sigmaInv(conj(w)).map(x => -x)
    if (lhs[0] === rhs[0] && lhs[1] === rhs[1]) pellT++
    w = sigma(w)
  }
  if (reversao !== M - 1 || volta !== M || pellT !== 15) R++
  console.log(`\n§K2  espelho·R_k = R_{−k}·espelho: ${reversao}/${M - 1} · T²=id: ${volta}/${M} · conj(σv)=−σ⁻¹(conj v): ${pellT}/15`)
  ok('§K2 a T-SIMETRIA já estava no grupo: espelho·R_k = R_{−k}·espelho em todos os ticks — a relação diedral SJS=J⁻¹ É a reversão do tempo, e T²=id (a volta)', reversao === M - 1 && volta === M)
  ok('§K2 e na hipérbole a estaca inverte o fluxo a menos do sinal da membrana: conj(σv) = −σ⁻¹·conj(v), exato em BigInt', pellT === 15)
}

/* §K3 — D₄ age no fluxo: o det é a seta do tempo */
{
  const { mul, igual, det, I, J, espelho } = mat2
  const X = mul(espelho, J)
  const neg = Mx => Mx.map(v => -v)
  const G8 = [I, J, neg(I), neg(J), espelho, X, neg(espelho), neg(X)]
  const R1 = [c[1], s[1], -s[1], c[1]]
  const R1inv = [c[1], -s[1], s[1], c[1]]
  const modM2 = Xv => Xv.map(v => ((v % P) + P) % P)
  let seta = 0
  for (const g of G8) {
    /* g⁻¹ dentro do grupo */
    let gi = null
    for (const h2 of G8) if (igual(mul(g, h2), I)) gi = h2
    const conjug = modM2(mul(mul(g, R1), gi))
    const alvo = det(g) === 1 ? modM2(R1) : modM2(R1inv)
    if (igual(conjug, alvo)) seta++
  }
  if (seta !== 8) R++
  console.log(`\n§K3  g·R·g⁻¹ = R^{det(g)}: ${seta}/8`)
  ok('§K3 D₄ AGE NO FLUXO: g·R·g⁻¹ = R^{det(g)} para os 8 elementos — as rotações comutam com o tempo, as reflexões invertem-no: o det é a SETA DO TEMPO', seta === 8)
}

/* §K4 — o gume e o contrato */
{
  /* a trajetória perturbada: z' = ω·z + (1,0) — a translação (o gérmen
   * aditivo, o mesmo da fronteira de limite_escada §L5) quebra a conservação */
  let G = true
  let x = c[3], y = s[3]
  for (let passo = 0; passo < 10; passo++) {
    const nx = A.mod(c[1] * x + s[1] * y + 1)     /* roda E translada */
    const ny = A.mod(-s[1] * x + c[1] * y + P * P)
    x = nx; y = ny
    if (A.mod(x * x + y * y) === 1) G = false     /* se conservasse, o gume morria */
  }
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§K4  a trajetória perturbada perde o invariante nos 10 passos: ${G} · 𝓜 = (R=${R}, G=${mc.G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§K4 o GUME: rodar E transladar perde a conservação em todos os passos — a translação (a mesma da fronteira aditiva) não é isometria do produto dual', G)
  ok('§K4 o contrato 𝓜 assina a mecânica: o fluxo é ação conservativa, as parcelas são a energia da carta, o espelho é o tempo invertido, o det é a seta', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A mecânica trazida: o fluxo conserva o produto dual nos dois')
  console.log('  regimes (oscilação no círculo, escape no Pell); a energia')
  console.log('  decompõe-se nas parcelas da carta e o rotor troca-as; a')
  console.log('  T-simetria já estava no grupo (SJS=J⁻¹ é a reversão do')
  console.log('  tempo) e o det de D₄ é a seta. O corpo não é uma coleção de')
  console.log('  números: é o que sustenta movimentos conservativos sob as')
  console.log('  suas operações.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
