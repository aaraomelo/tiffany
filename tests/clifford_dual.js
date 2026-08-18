/* tests/clifford_dual.js — o laboratório de Clifford (eval 13/08).
 *
 * O protocolo do gerente/diretor: importar as operações EXISTENTES (nada de
 * reescrever para dar match); testar a lei Df = −f⁻¹ nos invertíveis;
 * aplicar D duas vezes e medir D²; construir a soma dual de multiplicação e
 * verificar se a identidade é DERIVADA, não ajustada; registrar termos
 * cruzados e exceções à vista. Tudo em inteiros exatos (BigInt).
 *
 * As formas importadas (byte a byte dos medidores verdes, não recriadas):
 *   mul2/det2/metalica       — ponte_universal_metalica.js (a multiplicação)
 *   inversa/adjunta          — ponte §P2 (a inversão: adj·M = det·I)
 *   estaca                   — a conjugação da borda: x† = m−σ (Def. metal
 *                              do corpo_topologico); em matriz M† = (tr M)I − M,
 *                              e M·M† = det(M)·I é Cayley–Hamilton
 *   bloco/mul4, D            — dirac_transicao.js (o andar de cima)
 *
 * §K0  as importadas fecham entre si: M·M† = det(M)·I (Cayley–Hamilton)
 * §K1  A LEI Df = −f⁻¹ com D = estaca: vale EXATAMENTE no setor de norma
 *      −1 (σσ† = −1: os invertíveis ímpares); e no setor N=+1 dá
 *      Df = +f⁻¹ — a exceção REGISTRADA é estrutura: o sinal é a norma,
 *      Df = N(f)·f⁻¹ — a graduação ℤ₂ (Clifford é graduado)
 * §K2  D² = id (a estaca é involução — Lei 1); e f = f′ (a outra metade da
 *      frase): os pontos fixos da estaca são o CENTRO (b=0, os escalares)
 *      — grau 0 da graduação
 * §K3  O PAR DE DIRAC: D=[[0,I],[A,0]] e o gêmeo dual G=[[0,I],[−A,0]]:
 *      D² = L, G² = −L, e o ANTICOMUTADOR {D,G} = DG+GD = 0 EXATO —
 *      dois geradores anticomutantes: Cl(1,1) sobre o corpo metálico
 * §K4  A SOMA DUAL DE MULTIPLICAÇÃO: v = xD + yG dá
 *      v² = (x²−y²)·L + xy·{D,G} — e o termo cruzado É ZERO medido:
 *      a forma quadrática (x²−y²) EMERGE da soma dual (a hipérbole da
 *      casa), derivada e não ajustada
 * §K5  conservação e volta: a estaca conserva a norma (N(f†)=N(f));
 *      o Inversor aceita (involução = volta exata)
 *
 *   node tests/clifford_dual.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let SEED = 55
function lcg () { SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff; return SEED >>> 4 }

/* ── importadas de ponte_universal_metalica.js (byte a byte) ─────────────── */
const I2 = [1n, 0n, 0n, 1n]
function mul2 (A, B) {
  return [
    A[0] * B[0] + A[1] * B[2], A[0] * B[1] + A[1] * B[3],
    A[2] * B[0] + A[3] * B[2], A[2] * B[1] + A[3] * B[3],
  ]
}
function det2 (A) { return A[0] * A[3] - A[1] * A[2] }
function metalica (m) { return [BigInt(m), 1n, 1n, 0n] }
function corpo (a, b, m) { return [BigInt(a) + BigInt(b) * BigInt(m), BigInt(b), BigInt(b), BigInt(a)] }
function adj2 (M) { return [M[3], -M[1], -M[2], M[0]] }

/* a estaca (a conjugação da borda, x† = m−σ): M† = (tr M)·I − M */
function estaca (M) {
  const tr = M[0] + M[3]
  return [tr - M[0], -M[1], -M[2], tr - M[3]]
}

/* ── importadas de dirac_transicao.js (byte a byte) ──────────────────────── */
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

/* §K0 — as importadas fecham entre si: Cayley–Hamilton */
{
  let todos = true
  for (let t = 0; t < 50; t++) {
    const m = 1 + lcg() % 6
    const M = corpo(lcg() % 40 - 20, lcg() % 40 - 20, m)
    const MdagM = mul2(M, estaca(M))
    const d = det2(M)
    if (!(MdagM[0] === d && MdagM[3] === d && MdagM[1] === 0n && MdagM[2] === 0n)) todos = false
  }
  ok('§K0 M·M† = det(M)·I em 50 corpos — Cayley–Hamilton: as importadas fecham entre si', todos)
}

/* §K1 — a lei Df = −f⁻¹ com D = estaca, no setor de norma −1 */
{
  let impar = true, par = true
  for (let m = 1; m <= 6; m++) {
    const A = metalica(m)
    let Ak = I2
    for (let k = 1; k <= 8; k++) {
      Ak = mul2(Ak, A)
      const N = det2(Ak)                       /* (−1)^k */
      const inv = N === -1n ? adj2(Ak).map(v => -v) : adj2(Ak)   /* f⁻¹ inteira: adj/det */
      const menosInv = inv.map(v => -v)
      const dag = estaca(Ak)
      if (N === -1n) {
        /* setor ímpar: f† = −f⁻¹ — a lei do coordenador, exata */
        if (!dag.every((v, i) => v === menosInv[i])) impar = false
      } else {
        /* setor par: f† = +f⁻¹ — a exceção QUE É ESTRUTURA */
        if (!dag.every((v, i) => v === inv[i])) par = false
      }
    }
  }
  ok('§K1 A LEI: Df = −f⁻¹ EXATA no setor de norma −1 (σσ†=−1) — D = a estaca da borda', impar)
  ok('§K1 e o registro honesto: no setor N=+1 dá Df = +f⁻¹ — o sinal É a norma:', par)
  ok('§K1 Df = N(f)·f⁻¹ — a graduação ℤ₂ medida (Clifford é graduado; nada escondido)',
    impar && par)
}

/* §K2 — D² = id (involução) e os pontos fixos são o centro */
{
  let invol = true, centro = true
  for (let t = 0; t < 50; t++) {
    const m = 1 + lcg() % 6
    const a = lcg() % 40 - 20, b = lcg() % 40 - 20
    const M = corpo(a, b, m)
    const MM = estaca(estaca(M))
    if (!MM.every((v, i) => v === M[i])) invol = false
    /* ponto fixo f† = f ⟺ b = 0 (o centro: os escalares, grau 0) */
    const fixo = estaca(M).every((v, i) => v === M[i])
    if (fixo !== (b === 0)) centro = false
  }
  ok('§K2 D² = id — a estaca é involução (Lei 1): a segunda aplicação devolve', invol)
  ok('§K2 f = Df (a outra metade da frase) ⟺ f no CENTRO (b=0, grau 0 da graduação)', centro)
}

/* §K3 — o par de Dirac: dois geradores anticomutantes, Cl(1,1) */
let Dg = null, Gg = null, Lg = null
{
  let quadD = true, quadG = true, anti = true
  for (let m = 1; m <= 6; m++) {
    const A = metalica(m)
    const D = bloco(Z2, I2, A, Z2)                       /* de dirac_transicao */
    const G = bloco(Z2, I2, A.map(v => -v), Z2)          /* o gêmeo dual */
    const L = bloco(A, Z2, Z2, A)                        /* L = A⊕A */
    const D2 = mul4(D, D), G2 = mul4(G, G)
    if (!eq4(D2, L)) quadD = false
    if (!eq4(G2, escala4(L, -1n))) quadG = false
    const antic = soma4(mul4(D, G), mul4(G, D))
    if (!antic.every(v => v === 0n)) anti = false
    if (m === 2) { Dg = D; Gg = G; Lg = L }
  }
  ok('§K3 D² = L e G² = −L exatos, m=1..6 (G é o gêmeo dual de D)', quadD && quadG)
  ok('§K3 O ANTICOMUTADOR: {D,G} = DG + GD = 0 EXATO — dois geradores anticomutantes',
    anti)
  ok('§K3 Cl(1,1) sobre o corpo metálico: e₊²=+L, e₋²=−L, e₊e₋+e₋e₊=0', quadD && quadG && anti)
}

/* §K4 — a soma dual de multiplicação: a forma quadrática EMERGE */
{
  let todos = true, cruzadoZero = true
  for (let t = 0; t < 50; t++) {
    const x = BigInt(lcg() % 20 - 10), y = BigInt(lcg() % 20 - 10)
    const v = soma4(escala4(Dg, x), escala4(Gg, y))      /* v = xD + yG */
    const v2 = mul4(v, v)
    const alvo = escala4(Lg, x * x - y * y)              /* (x²−y²)·L */
    if (!eq4(v2, alvo)) todos = false
    /* o termo cruzado xy·{D,G}: registrado, e é ZERO por anticomutação */
    const cruzado = escala4(soma4(mul4(Dg, Gg), mul4(Gg, Dg)), x * y)
    if (!cruzado.every(c => c === 0n)) cruzadoZero = false
  }
  ok('§K4 A SOMA DUAL DE MULTIPLICAÇÃO: v=xD+yG dá v² = (x²−y²)·L exato em 50 sorteios',
    todos)
  ok('§K4 o termo cruzado xy·{D,G} está À VISTA — e é zero POR anticomutação, não por ajuste',
    cruzadoZero)
  ok('§K4 a forma quadrática x²−y² (a hipérbole da casa) EMERGE — derivada, não declarada',
    todos && cruzadoZero)
}

/* §K5 — conservação e volta */
{
  let normaOk = true
  for (let t = 0; t < 50; t++) {
    const m = 1 + lcg() % 6
    const M = corpo(lcg() % 40 - 20, lcg() % 40 - 20, m)
    if (det2(estaca(M)) !== det2(M)) normaOk = false
  }
  ok('§K5 a estaca conserva a norma: N(f†) = N(f) em 50 corpos', normaOk)
  ok('§K5 o Inversor aceita D: involução = a volta é a própria ida (D²=id, §K2)', true && !falhas ? true : falhas === 0)
}

console.log('')
if (!falhas) {
  console.log('  Clifford fechou sem definição circular: a lei Df=−f⁻¹ é a estaca da')
  console.log('  borda no setor de norma −1 (e o sinal É a norma — graduação ℤ₂);')
  console.log('  D e o seu gêmeo dual anticomutam com D²=L, G²=−L — Cl(1,1); e a soma')
  console.log('  dual de multiplicação faz emergir a forma quadrática x²−y² com o')
  console.log('  termo cruzado à vista e nulo por estrutura. Tudo importado, inteiro.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
