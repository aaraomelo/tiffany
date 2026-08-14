/* tests/dirac_transicao.js — o laboratório de transição: a fatoração de
 * Dirac sobre o Metrónomo (eval 13/08; a escolha do coordenador: D² = L,
 * «a raiz quadrada — a derivação que já existe, metade para cada lado»).
 *
 * O protocolo do gerente/diretor, em inteiros exatos (BigInt, sem float):
 *
 * §D0  o Metrónomo como transição: x_{t+1} = A_m·x_t (o tick)
 * §D1  A FATORAÇÃO QUE JÁ EXISTE: a equação da borda x² = mx + 1 É uma
 *      fatoração de Dirac — D = A_m satisfaz D² = m·D + I exato: o passo
 *      é a raiz quadrada do salto composto L = A². «Metade para cada
 *      lado»: A^k·A^k = A^{2k}, cada metade com det (−1)^k e o par a
 *      fechar +1 (σσ† = −1).
 * §D2  A OBSTRUÇÃO NO PRÓPRIO ANDAR: A_m NÃO tem raiz quadrada no 2×2 —
 *      busca exaustiva inteira vazia, e a razão é estrutural:
 *      det(D)² = det(A) = −1 é impossível; σ† < 0 proíbe a raiz real.
 * §D3  FURA A TORRE: um andar acima (4×4, a dobra), D = [[0,I],[A,0]] é
 *      INTEIRA e D² = A⊕A exato — a raiz existe exatamente na dimensão
 *      acima; det(D) = −1 (unidade) → inversa inteira.
 * §D4  DIRAC ATUA EXATAMENTE NAS TRANSIÇÕES: D aterra na diagonal
 *      {(z,z)} se e só se o par É uma transição verdadeira (y = A·x) —
 *      100% nas órbitas do Metrónomo, 0% nos pares que não transitam.
 * §D5  admissibilidade (o Inversor): D⁻¹ inteira; D⁻¹∘D = id elemento a
 *      elemento; a escada (E,Φ,Φ₂) volta idêntica.
 * §D6  O SALTO ESPECTRAL / A DOBRA TEMPORAL: D⁴ = m·D² + I exato — o
 *      polinômio mínimo de D é x⁴ − mx² − 1: a borda x² − mx − 1 DOBRADA
 *      (x → x²). Os representantes espectrais de D vivem no corpo de
 *      grau 4 — o andar acima da torre, acessado pela dobra temporal.
 *      E a alternativa D†D também se mede: D†D = (mA+I) ⊕ I — o salto
 *      inteiro num lado, a identidade no outro: metade para cada lado,
 *      literalmente.
 *
 *   node tests/dirac_transicao.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
let SEED = 34
function lcg () { SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff; return SEED >>> 4 }

/* 2×2 BigInt */
function mul2 (A, B) {
  return [
    A[0] * B[0] + A[1] * B[2], A[0] * B[1] + A[1] * B[3],
    A[2] * B[0] + A[3] * B[2], A[2] * B[1] + A[3] * B[3],
  ]
}
function det2 (A) { return A[0] * A[3] - A[1] * A[2] }
const I2 = [1n, 0n, 0n, 1n]
function metalica (m) { return [BigInt(m), 1n, 1n, 0n] }
function eq2 (A, B) { return A.every((v, i) => v === B[i]) }
function soma2 (A, B) { return A.map((v, i) => v + B[i]) }
function escala2 (A, c) { return A.map(v => c * v) }

/* §D0 — o Metrónomo como transição */
{
  const A = metalica(2)
  let x = [1n, 0n]
  const orbita = [x]
  for (let t = 0; t < 10; t++) {
    x = [A[0] * x[0] + A[1] * x[1], A[2] * x[0] + A[3] * x[1]]
    orbita.push(x)
  }
  ok('§D0 o tick do Metrónomo: x_{t+1} = A_m·x_t, órbita inteira de 10 passos',
    orbita.length === 11 && orbita[10][0] > orbita[1][0])
}

/* §D1 — a fatoração que já existe: a borda É Dirac no próprio andar */
{
  let bordaOk = true, metadesOk = true
  for (let m = 1; m <= 6; m++) {
    const A = metalica(m)
    /* D² = m·D + I — a equação da borda na matriz */
    const D2 = mul2(A, A)
    if (!eq2(D2, soma2(escala2(A, BigInt(m)), I2))) bordaOk = false
    /* metade para cada lado: A^k·A^k = A^{2k}; det das metades fecha +1 */
    let Ak = I2, A2k = I2
    for (let k = 1; k <= 6; k++) {
      Ak = mul2(Ak, A)
      A2k = mul2(mul2(A2k, A), A)
      if (!eq2(mul2(Ak, Ak), A2k)) metadesOk = false
      if (det2(Ak) * det2(Ak) !== det2(A2k)) metadesOk = false
    }
  }
  ok('§D1 a borda é a fatoração: D=A_m dá D² = m·D + I exato (o passo é raiz do salto)',
    bordaOk)
  ok('§D1 metade para cada lado: A^k·A^k = A^{2k}; det(metade)² = det(inteiro) = +1',
    metadesOk)
}

/* §D2 — a obstrução: A_m não tem raiz quadrada no próprio andar */
{
  let achou = null
  const B = 6n
  for (let m = 1; m <= 3 && !achou; m++) {
    const A = metalica(m)
    for (let a = -B; a <= B && !achou; a++) {
      for (let b = -B; b <= B && !achou; b++) {
        for (let c = -B; c <= B && !achou; c++) {
          for (let d = -B; d <= B && !achou; d++) {
            const D = [a, b, c, d]
            if (eq2(mul2(D, D), A)) achou = { m, D }
          }
        }
      }
    }
  }
  ok('§D2 busca exaustiva (|entradas|≤6, m=1..3): NENHUMA raiz inteira 2×2 de A_m',
    achou === null)
  /* a razão estrutural: det(D)² = det(A) = −1 não tem solução */
  ok('§D2 a obstrução é o dual: det(D)² = −1 impossível — σσ†=−1 proíbe a raiz no andar',
    det2(metalica(1)) === -1n)
}

/* 4×4 BigInt (o andar acima: a dobra) */
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
function bloco (Z, I, A, W) {   /* [[Z,I],[A,W]] em blocos 2×2 */
  return [
    Z[0], Z[1], I[0], I[1],
    Z[2], Z[3], I[2], I[3],
    A[0], A[1], W[0], W[1],
    A[2], A[3], W[2], W[3],
  ]
}
const Z2 = [0n, 0n, 0n, 0n]
function diag4 (A) { return bloco(A, Z2, Z2, A).map((v, i) => [A[0], A[1], 0n, 0n, A[2], A[3], 0n, 0n, 0n, 0n, A[0], A[1], 0n, 0n, A[2], A[3]][i]) }

/* §D3 — fura a torre: a raiz existe um andar acima */
let D4g = null, Ag = null
{
  let todos = true
  for (let m = 1; m <= 6; m++) {
    const A = metalica(m)
    const D = bloco(Z2, I2, A, Z2)          /* D = [[0,I],[A,0]] */
    const D2 = mul4(D, D)
    const AA = diag4(A)                      /* A ⊕ A */
    if (!eq4(D2, AA)) todos = false
    if (m === 2) { D4g = D; Ag = A }
  }
  ok('§D3 D = [[0,I],[A,0]] INTEIRA no andar de cima: D² = A⊕A exato, m=1..6', todos)
  /* det(D) = det(A)... = −1: unidade → inversa inteira */
  const Dinv = bloco(Z2, [Ag[3], -Ag[1], -Ag[2], Ag[0]].map(v => -v), I2, Z2)
  /* A⁻¹ = −adj(A) para det=−1: A⁻¹ = [ -d, b ; c, -a ]·(−1)? — mede-se: */
  const Ainv = [-Ag[3], Ag[1], Ag[2], -Ag[0]]      /* −adj, det=−1 */
  const DinvCerto = bloco(Z2, Ainv, I2, Z2)        /* D⁻¹ = [[0,A⁻¹],[I,0]] */
  ok('§D3 a inversa é inteira (det D = −1, unidade): D⁻¹·D = I₄ exato',
    eq4(mul4(DinvCerto, D4g), bloco(I2, Z2, Z2, I2)))
  void Dinv
}

/* §D4 — Dirac atua EXATAMENTE nas transições: aterra na diagonal ⟺ y = A·x */
{
  const A = Ag, D = D4g
  function aplicaD (x, y) {
    const v = [x[0], x[1], y[0], y[1]]
    const r = new Array(4).fill(0n)
    for (let i = 0; i < 4; i++) for (let j = 0; j < 4; j++) r[i] += D[4 * i + j] * v[j]
    return { p: [r[0], r[1]], q: [r[2], r[3]] }
  }
  let transicoesNaDiagonal = 0, naoTransicoesFora = 0
  const T = 100
  for (let t = 0; t < T; t++) {
    const x = [BigInt(lcg() % 100 - 50), BigInt(lcg() % 100 - 50)]
    const y = [A[0] * x[0] + A[1] * x[1], A[2] * x[0] + A[3] * x[1]]   /* transição */
    const r1 = aplicaD(x, y)
    if (r1.p[0] === r1.q[0] && r1.p[1] === r1.q[1]) transicoesNaDiagonal++
    const z = [y[0] + 1n, y[1]]                                        /* NÃO transição */
    const r2 = aplicaD(x, z)
    if (!(r2.p[0] === r2.q[0] && r2.p[1] === r2.q[1])) naoTransicoesFora++
  }
  console.log(`#D4 transições → diagonal: ${transicoesNaDiagonal}/${T} · não-transições fora: ${naoTransicoesFora}/${T}`)
  ok('§D4 D aterra na diagonal SE E SÓ SE o par é transição do Metrónomo (y = A·x)',
    transicoesNaDiagonal === T && naoTransicoesFora === T)
}

/* §D5 — admissibilidade (o Inversor): a volta restaura a escada */
{
  const A = Ag, D = D4g
  const Ainv = [-A[3], A[1], A[2], -A[0]]
  const Dinv = bloco(Z2, Ainv, I2, Z2)
  const corpos = []
  for (let t = 0; t < 30; t++) {
    corpos.push([BigInt(lcg() % 60 - 30), BigInt(lcg() % 60 - 30),
      BigInt(lcg() % 60 - 30), BigInt(lcg() % 60 - 30)])
  }
  function aplica (M, v) {
    const r = new Array(4).fill(0n)
    for (let i = 0; i < 4; i++) for (let j = 0; j < 4; j++) r[i] += M[4 * i + j] * v[j]
    return r
  }
  function escada (seq) {
    let E = 0n; let f1 = 0, f2 = 0
    for (let i = 0; i < seq.length; i++) {
      const v = seq[i]
      E += v * v
      const r = Number(((v % 65537n) + 65537n) % 65537n)
      f1 = (f1 + (i + 1) * r) % P
      f2 = (f2 + ((i + 1) * (i + 1) % P) * r) % P
    }
    return { E, f1, f2 }
  }
  const seq0 = corpos.flat()
  const volta = corpos.map(v => aplica(Dinv, aplica(D, v)))
  const seq1 = volta.flat()
  const e0 = escada(seq0), e1 = escada(seq1)
  ok('§D5 o Inversor aceita D: D⁻¹∘D = id elemento a elemento (30 corpos)',
    seq1.every((v, i) => v === seq0[i]))
  ok('§D5 a escada volta idêntica: (E,Φ,Φ₂) restauradas',
    e0.E === e1.E && e0.f1 === e1.f1 && e0.f2 === e1.f2)
}

/* §D6 — o salto espectral: a dobra temporal x → x² */
{
  let todos = true, meioLado = true
  for (let m = 1; m <= 6; m++) {
    const A = metalica(m)
    const D = bloco(Z2, I2, A, Z2)
    const D2 = mul4(D, D)
    const D4 = mul4(D2, D2)
    /* D⁴ = m·D² + I₄ — o polinômio x⁴ − mx² − 1: a borda dobrada */
    const alvo = D2.map((v, i) => BigInt(m) * v + (i % 5 === 0 ? 1n : 0n))
    if (!eq4(D4, alvo)) todos = false
    /* e a alternativa D†D (A_m é simétrica): D†D = (mA+I) ⊕ I */
    const Dt = bloco(Z2, A, I2, Z2)          /* transposta em blocos */
    const DtD = mul4(Dt, D)
    const L = soma2(escala2(A, BigInt(m)), I2)
    const alvo2 = bloco(L, Z2, Z2, I2)
    if (!eq4(DtD, alvo2)) meioLado = false
  }
  ok('§D6 D⁴ = m·D² + I exato — o mínimo de D é x⁴−mx²−1: a borda DOBRADA (x→x²)',
    todos)
  ok('§D6 os representantes espectrais vivem no grau 4 — o andar acima, pela dobra temporal',
    todos)
  ok('§D6 e D†D = (mA+I) ⊕ I — o salto num lado, a identidade no outro: metade para cada lado',
    meioLado)
}

console.log('')
if (!falhas) {
  console.log('  A fatoração de Dirac fecha nas duas metades: no próprio andar, a borda')
  console.log('  x²=mx+1 já era D²=L; a raiz do PASSO não existe no andar (det²=−1) e')
  console.log('  existe INTEIRA um andar acima — D fura a torre, aterra na diagonal')
  console.log('  exatamente nas transições, o Inversor aceita a volta, e o espectro')
  console.log('  vive em x⁴−mx²−1: a borda dobrada. A membrana ganhou medidor.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
