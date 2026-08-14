/* tests/clifford_pleno.js — Clifford pleno: o andar acima de Cl(1,1)
 * (fila selada da mesa, eval 14/08: Viviani → trial → encaixe →
 * resíduos → CLIFFORD PLENO; docs/MAPA_UNIVERSAL.md: «mais um gerador
 * anticomutante — lacuna pequena, só álgebra inteira de matrizes
 * maiores»).
 *
 * As formas são as dos medidores verdes, byte a byte (clifford_dual.js
 * / dirac_transicao.js): mul2/metalica, bloco/mul4, D=[[0,I],[A,0]],
 * G=[[0,I],[−A,0]], L=A⊕A. O movimento é o da casa — «um andar acima»
 * pela duplicação de bloco — e o que a medida dá:
 *
 *   O GERADOR NOVO ENTRA VESTIDO COM O ESPELHO: no andar M₈, os
 *   geradores velhos sobem como e_i = diag(E_i, −E_i) = diag(E_i,E_i)·S
 *   com S = diag(I,−I) — e S É o espelho: o mesmo deck do recobrimento
 *   duplo que Viviani mediu como troca de folha (viviani_universal §V3).
 *   A duplicação de Clifford é um recobrimento duplo, e o espelho é o
 *   que faz o gerador novo anticomutar.
 *
 *   A DIMENSÃO DOBRA, DOIS DEGRAUS MEDIDOS: os produtos de subconjuntos
 *   têm posto 4 em Cl(1,1), 8 em Cl(2,1), 16 em Cl(2,2) — a lei
 *   dim A_{n+1} = 2·dim A_n (a lição da base incompleta), medida por
 *   eliminação exata no anel, não afirmada.
 *
 *   O VOLUME É CENTRAL NO ÍMPAR E GRADUADO NO PAR: ω = e₁e₂e₃ comuta
 *   com os três geradores de Cl(2,1); em Cl(1,1), ω = DG ANTIcomuta
 *   com D e G. O par (n ímpar: centro; n par: graduação) são as duas
 *   metades da mesma frase — e ω² fecha por dois caminhos (produto
 *   matricial vs sinal (−1)^{k(k−1)/2}·∏e_i²).
 *
 * §P0  as importadas fecham: D²=L, G²=−L, {D,G}=0 (a base de partida)
 * §P1  o andar: e₁²=L₈, e₂²=−L₈, e₃²=I₈ e os TRÊS anticomutadores
 *      zeram, m=1..4; gume: sem o espelho (diag(D,D)) NÃO anticomuta
 * §P2  o espelho é o deck: e_i = diag(E_i,E_i)·S, S²=I, Se₃=−e₃S —
 *      a duplicação é um recobrimento duplo com deck = espelho
 * §P3  a dimensão dobra: posto 4 → 8 → 16 (Cl(1,1) → Cl(2,1) →
 *      Cl(2,2) com e₄²=−I₈), por eliminação exata mod P
 * §P4  o volume: central no ímpar (comuta com e₁,e₂,e₃), graduado no
 *      par (DG anticomuta com D,G); ω² por dois caminhos
 * §P5  a subálgebra par fecha: os 16 produtos de pares ficam no
 *      espaço dos 4 pares — o posto não sobe
 */
'use strict'
const { anel } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

/* ── importadas de clifford_dual.js / dirac_transicao.js (byte a byte) ──── */
const I2 = [1n, 0n, 0n, 1n]
function metalica (m) { return [BigInt(m), 1n, 1n, 0n] }
function mul4 (A, B) {
  const C = new Array(16).fill(0n)
  for (let i = 0; i < 4; i++) {
    for (let k = 0; k < 4; k++) {
      const a = A[4 * i + k]
      if (a === 0n) continue
      for (let j = 0; j < 4; j++) C[4 * i + j] += a * B[4 * k + j]
    }
  }
  return C
}
function eq4 (A, B) { return A.every((v, i) => v === B[i]) }
const Z2 = [0n, 0n, 0n, 0n]
function bloco (P, Q, R, S) {
  return [
    P[0], P[1], Q[0], Q[1],
    P[2], P[3], Q[2], Q[3],
    R[0], R[1], S[0], S[1],
    R[2], R[3], S[2], S[3],
  ]
}
function soma4 (A, B) { return A.map((v, i) => v + B[i]) }
function escala4 (A, c) { return A.map(v => c * v) }

/* ── o andar M₈: a mesma duplicação de bloco, um degrau acima ───────────── */
function mul8 (A, B) {
  const C = new Array(64).fill(0n)
  for (let i = 0; i < 8; i++) {
    for (let k = 0; k < 8; k++) {
      const a = A[8 * i + k]
      if (a === 0n) continue
      for (let j = 0; j < 8; j++) C[8 * i + j] += a * B[8 * k + j]
    }
  }
  return C
}
function eq8 (A, B) { return A.every((v, i) => v === B[i]) }
function soma8 (A, B) { return A.map((v, i) => v + B[i]) }
function escala8 (A, c) { return A.map(v => c * v) }
function bloco8 (P, Q, R, S) {                 /* quatro 4×4 → um 8×8 */
  const C = new Array(64)
  for (let i = 0; i < 4; i++) {
    for (let j = 0; j < 4; j++) {
      C[8 * i + j] = P[4 * i + j]
      C[8 * i + j + 4] = Q[4 * i + j]
      C[8 * (i + 4) + j] = R[4 * i + j]
      C[8 * (i + 4) + j + 4] = S[4 * i + j]
    }
  }
  return C
}
const Z4 = new Array(16).fill(0n)
const I4 = [1n, 0n, 0n, 0n, 0n, 1n, 0n, 0n, 0n, 0n, 1n, 0n, 0n, 0n, 0n, 1n]
const I8 = bloco8(I4, Z4, Z4, I4)
const S8 = bloco8(I4, Z4, Z4, escala4(I4.map(v => v), -1n))   /* o ESPELHO de bloco: diag(I,−I) */

const anti8 = (X, Y) => soma8(mul8(X, Y), mul8(Y, X))
const zera8 = X => X.every(v => v === 0n)

/* o posto por eliminação exata no anel (a régua inteira) */
const P = 65537
const A9 = anel(P)
function posto (vetores) {
  const linhas = vetores.map(v => v.map(x => A9.mod(Number(((x % BigInt(P)) + BigInt(P)) % BigInt(P)))))
  let r = 0
  const n = linhas[0].length
  for (let col = 0; col < n && r < linhas.length; col++) {
    let piv = -1
    for (let i = r; i < linhas.length; i++) if (linhas[i][col] !== 0) { piv = i; break }
    if (piv < 0) continue
    ;[linhas[r], linhas[piv]] = [linhas[piv], linhas[r]]
    const inv = A9.inv(linhas[r][col])
    for (let j = 0; j < n; j++) linhas[r][j] = linhas[r][j] * inv % P
    for (let i = 0; i < linhas.length; i++) {
      if (i === r || linhas[i][col] === 0) continue
      const f = linhas[i][col]
      for (let j = 0; j < n; j++) linhas[i][j] = A9.mod(linhas[i][j] - f * linhas[r][j])
    }
    r++
  }
  return r
}

/* a base de partida (m=2, o mesmo palco do clifford_dual) */
const m = 2
const Am = metalica(m)
const D = bloco(Z2, I2, Am, Z2)
const G = bloco(Z2, I2, Am.map(v => -v), Z2)
const L = bloco(Am, Z2, Z2, Am)

/* §P0 — as importadas fecham */
{
  const fecha = eq4(mul4(D, D), L) && eq4(mul4(G, G), escala4(L, -1n)) &&
    soma4(mul4(D, G), mul4(G, D)).every(v => v === 0n)
  console.log(`\n§P0  D²=L, G²=−L, {D,G}=0 no palco m=${m}`)
  ok('§P0 a base de partida fecha: Cl(1,1) das importadas, byte a byte', fecha)
}

/* o andar: os geradores de Cl(2,1) em M₈ */
const e1 = bloco8(D, Z4, Z4, escala4(D, -1n))          /* diag(D, −D) */
const e2 = bloco8(G, Z4, Z4, escala4(G, -1n))          /* diag(G, −G) */
const e3 = bloco8(Z4, I4, I4, Z4)                       /* a troca de folha */
const L8 = bloco8(L, Z4, Z4, L)

/* §P1 — o andar fecha, e o gume */
{
  let fecha = true, gume = true
  for (let mm = 1; mm <= 4; mm++) {
    const Amm = metalica(mm)
    const Dm = bloco(Z2, I2, Amm, Z2), Gm = bloco(Z2, I2, Amm.map(v => -v), Z2)
    const Lm = bloco(Amm, Z2, Z2, Amm)
    const f1 = bloco8(Dm, Z4, Z4, escala4(Dm, -1n))
    const f2 = bloco8(Gm, Z4, Z4, escala4(Gm, -1n))
    const L8m = bloco8(Lm, Z4, Z4, Lm)
    if (!eq8(mul8(f1, f1), L8m)) fecha = false
    if (!eq8(mul8(f2, f2), escala8(L8m, -1n))) fecha = false
    if (!eq8(mul8(e3, e3), I8)) fecha = false
    if (!zera8(anti8(f1, f2)) || !zera8(anti8(f1, e3)) || !zera8(anti8(f2, e3))) fecha = false
    /* SEM o espelho: diag(D,D) comuta com a troca em vez de anticomutar */
    const nu = bloco8(Dm, Z4, Z4, Dm)
    if (zera8(anti8(nu, e3))) gume = false
  }
  console.log(`\n§P1  e₁²=L₈, e₂²=−L₈, e₃²=I₈ e os três {e_i,e_j}=0, m=1..4 · sem espelho: anticomutador ≠ 0`)
  ok('§P1 o andar fecha: três geradores anticomutantes com assinatura (+L,−L,+I) — Cl(2,1) sobre o corpo metálico', fecha)
  ok('§P1 o gume: diag(D,D) — o gerador SEM o espelho — não anticomuta com a troca; o espelho não é decoração', gume)
}

/* §P2 — o espelho é o deck da duplicação */
{
  const vestido1 = eq8(e1, mul8(bloco8(D, Z4, Z4, D), S8))
  const vestido2 = eq8(e2, mul8(bloco8(G, Z4, Z4, G), S8))
  const involui = eq8(mul8(S8, S8), I8)
  const anticomuta = zera8(anti8(S8, e3))
  console.log(`\n§P2  e_i = diag(E_i,E_i)·S: ${vestido1 && vestido2} · S²=I: ${involui} · {S,e₃}=0: ${anticomuta}`)
  ok('§P2 o gerador velho sobe VESTIDO com o espelho: e_i = diag(E_i,E_i)·S, com S = diag(I,−I)', vestido1 && vestido2)
  ok('§P2 S²=I e S anticomuta com a troca de folha — a duplicação é um recobrimento duplo e o espelho é o deck (o mesmo de viviani §V3)', involui && anticomuta)
}

/* §P3 — a dimensão dobra: 4 → 8 → 16 */
{
  /* Cl(1,1): produtos dos subconjuntos de {D,G} em M₄ */
  const I4v = I4
  const baseA = [I4v, D, G, mul4(D, G)]
  const pA = posto(baseA)
  /* Cl(2,1): subconjuntos de {e₁,e₂,e₃} em M₈ */
  const prods = [I8, e1, e2, e3, mul8(e1, e2), mul8(e1, e3), mul8(e2, e3), mul8(mul8(e1, e2), e3)]
  const pB = posto(prods)
  /* Cl(2,2): junta e₄ = [[0,I],[−I,0]], e₄² = −I₈ */
  const e4 = bloco8(Z4, I4, escala4(I4.map(v => v), -1n), Z4)
  const e4fecha = eq8(mul8(e4, e4), escala8(I8, -1n)) &&
    zera8(anti8(e4, e1)) && zera8(anti8(e4, e2)) && zera8(anti8(e4, e3))
  const prods16 = []
  for (let s = 0; s < 16; s++) {
    let Mp = I8
    if (s & 1) Mp = mul8(Mp, e1)
    if (s & 2) Mp = mul8(Mp, e2)
    if (s & 4) Mp = mul8(Mp, e3)
    if (s & 8) Mp = mul8(Mp, e4)
    prods16.push(Mp)
  }
  const pC = posto(prods16)
  console.log(`\n§P3  posto Cl(1,1)=${pA} · Cl(2,1)=${pB} · e₄²=−I e anticomuta: ${e4fecha} · Cl(2,2)=${pC}`)
  ok('§P3 a dimensão DOBRA a cada gerador: 4 → 8 → 16, por eliminação exata — dim A_{n+1} = 2·dim A_n, medida em dois degraus', pA === 4 && pB === 8 && pC === 16)
  ok('§P3 e o quarto gerador fecha a assinatura (2,2): e₄²=−I₈ e anticomuta com os três', e4fecha)
}

/* §P4 — o volume: central no ímpar, graduado no par; ω² por dois caminhos */
{
  const w3 = mul8(mul8(e1, e2), e3)
  const comuta = [e1, e2, e3].every(e => eq8(mul8(w3, e), mul8(e, w3)))
  /* ω² por dois caminhos: produto matricial vs (−1)^{3·2/2}·e₁²e₂²e₃² */
  const caminho1 = mul8(w3, w3)
  const caminho2 = escala8(mul8(mul8(L8, escala8(L8, -1n)), I8), -1n)
  const doisCaminhos = eq8(caminho1, caminho2)
  /* e no andar de baixo (n par): ω = DG ANTIcomuta */
  const w2 = mul4(D, G)
  const antiPar = [D, G].every(E => soma4(mul4(w2, E), mul4(E, w2)).every(v => v === 0n))
  console.log(`\n§P4  ω=e₁e₂e₃ comuta com os 3: ${comuta} · ω² dois caminhos: ${doisCaminhos} · no par, DG anticomuta com D,G: ${antiPar}`)
  ok('§P4 no ÍMPAR o volume é central: ω=e₁e₂e₃ comuta com os três geradores de Cl(2,1)', comuta)
  ok('§P4 ω² fecha por dois caminhos: matricial == (−1)^{k(k−1)/2}·∏e_i² = L₈²', doisCaminhos)
  ok('§P4 no PAR o volume gradua: DG anticomuta com D e G — as duas metades da frase, uma por andar', antiPar)
}

/* §P5 — a subálgebra par fecha SOBRE O CORPO METÁLICO */
{
  /* os coeficientes não são ℤ: são R = ℤ[A] (A² = mA + I, o corpo
   * metálico) — o L₈ é ESCALAR do corpo, não matriz a mais. Primeiro
   * mediu-se sobre ℤ e NÃO fechava ((e₁e₂)(e₁e₃) = −L₈·e₂e₃ sai do
   * span): a falha era a medida a apontar o corpo certo. */
  const pares = [I8, mul8(e1, e2), mul8(e1, e3), mul8(e2, e3)]
  const soZ = posto(pares)
  const sobreR = [...pares, ...pares.map(X => mul8(L8, X))]
  const base = posto(sobreR)
  let fechada = true, sobreZfecha = true
  for (const X of pares) {
    for (const Y of pares) {
      const p2 = mul8(X, Y)
      if (posto([...sobreR, p2]) !== base) fechada = false
      if (posto([...pares, p2]) !== soZ) sobreZfecha = false
    }
  }
  console.log(`\n§P5  posto sobre ℤ: ${soZ} (não fecha: ${!sobreZfecha}) · sobre R=ℤ[A]: ${base} · os 16 produtos ficam: ${fechada}`)
  ok('§P5 a subálgebra PAR fecha sobre o CORPO METÁLICO: posto 8 = 4 pares × 2 do corpo, e nenhum produto sobe — a graduação ℤ₂ é estrutura', base === 8 && fechada)
  ok('§P5 o gume que apontou o corpo: sobre ℤ os pares NÃO fecham ((e₁e₂)(e₁e₃) = −L₈·e₂e₃ sai do span) — os coeficientes são ℤ[A], não ℤ', !sobreZfecha && soZ === 4)
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  Clifford pleno: o andar acima existe e fecha — o gerador novo')
  console.log('  entra vestido com o espelho (o deck do recobrimento duplo),')
  console.log('  a dimensão dobra em dois degraus medidos (4→8→16), o volume')
  console.log('  é central no ímpar e graduado no par, e a subálgebra par')
  console.log('  fecha. A lacuna «pequena» do mapa está paga.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
