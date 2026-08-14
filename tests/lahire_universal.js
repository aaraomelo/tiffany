/* tests/lahire_universal.js — o rolamento dual: a ponte de La Hire
 * (ordem da mesa, 14/08 noite: «resolve as questões da secção 14» do
 * corpo universal — La Hire era o candidato geométrico do rotor da
 * Lei 2, à espera da ponte).
 *
 * O que a literatura chama teorema de La Hire (o círculo de raio r a
 * rolar dentro do de raio 2r fecha rotação em translação: o ponto anda
 * num DIÂMETRO) realiza-se inteiro no relógio, e a razão é da casa:
 *
 *   O ROLAMENTO É O PAR (rotação, rotação inversa): o ponto 2:1 é
 *   P_k = z^k + z^{−k} — e ISTO é a dobra H do núcleo aplicada ao par.
 *   A rotação vira translação porque a dobra DIAGONALIZA a troca:
 *   P_k cai no eixo fixo do espelho (y=0) e o ponto irmão Q_k no
 *   anti-eixo (x=0) — os dois diâmetros SÃO os eixos do espelho, e o
 *   rotor J troca-os (a Lei 2 realizada em geometria, como o mapa
 *   previa).
 *
 *   E O GUME É O MESMO FACTO DE VIVIANI: o rolamento 3:1 NÃO é reta —
 *   a componente y = s_k − s_{2k} só zera onde s_k = 0 (2 pontos),
 *   porque o outro zero pediria cos t = 1/2, ordem 6, e 3 ∤ 2^{2^j}:
 *   ausente da escada de Fermat, Euler nos três andares.
 *
 * §H0  o palco: o relógio (c_k, s_k) com c²+s²=1 nos M ticks
 * §H1  La Hire: P_k = (c_k+c_{−k}, s_k+s_{−k}) fica no diâmetro
 *      (y=0, x=2c_k) e Q_k no perpendicular, nos M pontos; DOIS
 *      caminhos: P_x == tr(carta) == z^k + z^{−k}
 * §H2  a dobra faz o rolamento: (P,Q) = nucleo.H no par de pontos;
 *      espelho·P=P, espelho·Q=−Q (os eixos), e J troca os diâmetros
 * §H3  o gume: o rolamento 3:1 tem exatamente 2 zeros em y — o outro
 *      zero pediria ordem 6, ausente nos três andares (Euler)
 * §H4  o contrato assina: 𝓜=(R,G,V) com V = a volta da dobra
 *      (recompor a rotação dos dois diâmetros, byte a byte)
 */
'use strict'
const { anel, mat2, nucleo, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const A = anel(P)
const M = 32
const h = A.powm(3, 65536 / M)
const i4 = A.powm(3, 16384)
const i2 = A.inv(2)
const c = [], s = []
for (let k = 0; k < M; k++) {
  const hk = A.powm(h, k), hki = A.inv(hk)
  c.push(A.mod((hk + hki) * i2))
  s.push(A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4)))
}

let R = 0        /* o invariante acumulado para o contrato (§H4) */

/* §H0 — o palco */
{
  let uni = 0
  for (let k = 0; k < M; k++) if (A.mod(c[k] * c[k] + s[k] * s[k]) === 1) uni++
  if (uni !== M) R++
  console.log(`\n§H0  c²+s²=1 em ${uni}/${M} ticks`)
  ok('§H0 o relógio está no círculo: c²+s²=1 nos M ticks — a membrana +1', uni === M)
}

/* §H1 — La Hire: o ponto do rolamento fica no diâmetro */
{
  let diametro = 0, perpendicular = 0, doisCaminhos = 0
  for (let k = 0; k < M; k++) {
    const mk = (M - k) % M
    const Px = A.mod(c[k] + c[mk]), Py = A.mod(s[k] + s[mk])
    const Qx = A.mod(c[k] - c[mk]), Qy = A.mod(s[k] - s[mk])
    if (Py === 0 && Px === A.mod(2 * c[k])) diametro++
    if (Qx === 0 && Qy === A.mod(2 * s[k])) perpendicular++
    /* dois caminhos: a soma no relógio == o traço da carta da lib */
    const hk = A.powm(h, k)
    if (A.mod(hk + A.inv(hk)) === A.mod(mat2.tr(mat2.carta(c[k], s[k])))) doisCaminhos++
  }
  if (diametro !== M || perpendicular !== M || doisCaminhos !== M) R++
  console.log(`\n§H1  P_k no diâmetro: ${diametro}/${M} · Q_k no perpendicular: ${perpendicular}/${M} · P_x == tr(carta) == z+z⁻¹: ${doisCaminhos}/${M}`)
  ok('§H1 o ponto do rolamento 2:1 fica no DIÂMETRO (y=0, x=2c_k) e o irmão no perpendicular — rotação virou translação, nos M pontos', diametro === M && perpendicular === M)
  ok('§H1 dois caminhos: P_x = z^k + z^{−k} = tr(carta(c,s)) — o ponto de La Hire é o TRAÇO da rotação', doisCaminhos === M)
}

/* §H2 — a dobra faz o rolamento, e os diâmetros são os eixos do espelho */
{
  const { espelho, J } = mat2
  const H = nucleo.H
  let pelaDobra = 0, eixoFixo = 0, eixoAnti = 0, rotorTroca = 0
  for (let k = 0; k < M; k++) {
    const mk = (M - k) % M
    /* H aplicado ao PAR de pontos, componente a componente */
    const Px = A.mod(H[0] * c[k] + H[1] * c[mk]), Py = A.mod(H[0] * s[k] + H[1] * s[mk])
    const Qx = A.mod(H[2] * c[k] + H[3] * c[mk]), Qy = A.mod(H[2] * s[k] + H[3] * s[mk])
    if (Px === A.mod(c[k] + c[mk]) && Py === A.mod(s[k] + s[mk]) &&
        Qx === A.mod(c[k] - c[mk]) && Qy === A.mod(s[k] - s[mk])) pelaDobra++
    /* espelho·P = P (eixo fixo), espelho·Q = −Q (anti-eixo) */
    if (A.mod(espelho[0] * Px + espelho[1] * Py) === Px &&
        A.mod(espelho[2] * Px + espelho[3] * Py) === A.mod(-Py + P)) eixoFixo++
    if (A.mod(espelho[0] * Qx + espelho[1] * Qy) === Qx &&
        A.mod(espelho[2] * Qx + espelho[3] * Qy) === A.mod(-Qy + P) && Qx === 0) eixoAnti++
    /* o rotor troca os diâmetros: J·(x,0) tem x-parte 0 */
    const JPx = A.mod(J[0] * Px + J[1] * Py)
    const JPy = A.mod(J[2] * Px + J[3] * Py)
    if (JPx === Py && A.mod(JPy + Px) % P === 0) rotorTroca++
  }
  if (pelaDobra !== M || eixoFixo !== M || eixoAnti !== M || rotorTroca !== M) R++
  console.log(`\n§H2  (P,Q) = H·(par): ${pelaDobra}/${M} · espelho fixa P: ${eixoFixo}/${M} · anti-fixa Q: ${eixoAnti}/${M} · J troca os diâmetros: ${rotorTroca}/${M}`)
  ok('§H2 o rolamento É a dobra: (P,Q) = nucleo.H aplicado ao par (rotação, rotação inversa) — o mesmo H de Viviani e do espectro', pelaDobra === M)
  ok('§H2 os dois diâmetros são os EIXOS do espelho (fixa P, anti-fixa Q) e o rotor J troca-os — a Lei 2 realizada em geometria, como o mapa previa', eixoFixo === M && eixoAnti === M && rotorTroca === M)
}

/* §H3 — o gume: o rolamento 3:1 não é reta, e a razão é a escada */
let G = false
{
  let zeros = 0
  for (let k = 0; k < M; k++) if (A.mod(s[k] - s[(2 * k) % M] + P) === 0) zeros++
  const andares = [17, 257, 65537]
  const semOrdem6 = andares.every(q => { const B = anel(q); return B.powm(q - 3, (q - 1) / 2) === q - 1 })
  G = zeros === 2 && semOrdem6
  console.log(`\n§H3  zeros de y no 3:1: ${zeros} (só s=0) · ordem 6 ausente nos 3 andares: ${semOrdem6}`)
  ok('§H3 o GUME: o rolamento 3:1 falha a reta em M−2 pontos — o outro zero pediria cos t = 1/2, ordem 6, e 3∤2^{2^j} (Euler nos três andares): o mesmo facto de Viviani', G)
}

/* §H4 — o contrato assina, com a volta da dobra */
{
  let V = 0
  for (let k = 0; k < M; k++) {
    const mk = (M - k) % M
    const Px = A.mod(c[k] + c[mk]), Py = A.mod(s[k] + s[mk])
    const Qx = A.mod(c[k] - c[mk]), Qy = A.mod(s[k] - s[mk])
    /* a volta da dobra: H⁻¹ = H/2 — recompõe a rotação dos dois diâmetros */
    if (A.mod((Px + Qx) * i2) !== c[k] || A.mod((Py + Qy) * i2) !== s[k]) V++
    if (A.mod(A.mod(Px - Qx + 2 * P) * i2) !== c[mk] || A.mod(A.mod(Py - Qy + 2 * P) * i2) !== s[mk]) V++
  }
  const m = medicao.contrato(R, G, V)
  console.log(`\n§H4  𝓜 = (R=${R}, G=${m.G}, V=${V}) → fecha: ${medicao.fecha(m)}`)
  ok('§H4 a volta da dobra recompõe a rotação dos dois diâmetros, byte a byte, e o contrato 𝓜 assina a ponte', medicao.fecha(m))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A ponte de La Hire fechou: o rolamento 2:1 é a dobra H no par')
  console.log('  (rotação, rotação inversa) — rotação vira translação porque a')
  console.log('  dobra diagonaliza a troca, os diâmetros são os eixos do')
  console.log('  espelho, o rotor troca-os (Lei 2), e o 3:1 falha a reta pelo')
  console.log('  mesmo facto de Viviani: a ordem 6 não vive na escada.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
