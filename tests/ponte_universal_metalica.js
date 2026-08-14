/* tests/ponte_universal_metalica.js — a ponte: transformada universal ↔
 * família metálica (eval 13/08, o experimento único do gerente/diretor).
 *
 * A metodologia: aplicar a transformada à família, medir a conservação,
 * testar a reconstrução contra a escada de observadores, e REGISTRAR o que
 * emerge sem forçar (frações contínuas, Pisot, Möbius) e o que NÃO aparece
 * (Dirac, convolução universal — ficam no mapa).
 *
 * As lições da casa mandam no desenho: a transformada universal é a
 * AVALIAÇÃO NAS FOLHAS (σ, σ†), não uma DFT — o √N não sobrevive porque o
 * objeto é MULTIPLICATIVO. A realização inteira é a matriz companheira:
 *   x = a+bσ  ↔  M = aI + bA_m,   A_m = [[m,1],[1,0]]
 *   N(x) = det(M) = a² + mab − b²   (a norma do corpo — inteira)
 * A conservação é por PRODUTO (N(xy)=N(x)N(y)); a inversa é INTEIRA nas
 * unidades (|det|=1 — o fator de potência); e a adjunta inteira dá a fibra
 * quando det≠±1 (adj·M = det·I — a divisão outra vez).
 *
 * §P0  Parseval dourado: det(A_m^k) = (−1)^k exato, m=1..6, k=1..12
 * §P1  conservação multiplicativa: det(MN)=det(M)det(N) em 100 pares LCG
 * §P2  a volta: inversa INTEIRA nas unidades (M⁻¹M=I); adjunta = fibra
 * §P3  reconstrução devolve MASSA e ORDEM: a escada (E,Φ,Φ₂) da sequência
 *      de corpos é idêntica após T⁻¹∘T — não só a energia
 * §P4  o que EMERGE (registrado, não forçado):
 *        frações contínuas — A_m^k carrega os convergentes de [m;m,m,…]
 *        Möbius — compor matrizes = compor transformações fracionárias
 *        Pisot — L_k² − Δ·F_k² = 4·(−1)^k exato (o vazamento compensado
 *                pelo dual, em inteiros — sem um float)
 *      e o que NÃO aparece nesta ponte: Dirac, convolução universal
 * §P5  a mesma família no anel da Lei 8: NTT dos F_k mod p, Parseval fecha
 *      — as duas leituras da cruz: multiplicativa (det) e aditiva (Σ²)
 *
 * Inteiro puro; LCG determinístico; sem um double.
 *
 *   node tests/ponte_universal_metalica.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
let SEED = 21
function lcg () { SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff; return SEED >>> 4 }

/* matrizes 2×2 inteiras (BigInt: as potências metálicas crescem) */
const I2 = [1n, 0n, 0n, 1n]
function mul (A, B) {
  return [
    A[0] * B[0] + A[1] * B[2], A[0] * B[1] + A[1] * B[3],
    A[2] * B[0] + A[3] * B[2], A[2] * B[1] + A[3] * B[3],
  ]
}
function det (A) { return A[0] * A[3] - A[1] * A[2] }
function metalica (m) { return [BigInt(m), 1n, 1n, 0n] }
function corpo (a, b, m) {   /* x = a + bσ  ↔  aI + bA_m */
  return [BigInt(a) + BigInt(b) * BigInt(m), BigInt(b), BigInt(b), BigInt(a)]
}

/* §P0 — Parseval dourado: det(A_m^k) = (−1)^k */
{
  let todos = true
  for (let m = 1; m <= 6; m++) {
    let Ak = I2
    const Am = metalica(m)
    for (let k = 1; k <= 12; k++) {
      Ak = mul(Ak, Am)
      if (det(Ak) !== (k % 2 === 0 ? 1n : -1n)) todos = false
    }
  }
  ok('§P0 det(A_m^k) = (−1)^k exato, m=1..6, k=1..12 — o Parseval dourado', todos)
}

/* §P1 — a conservação da transformada é MULTIPLICATIVA: N(xy)=N(x)N(y) */
{
  let todos = true
  for (let t = 0; t < 100; t++) {
    const m = 1 + lcg() % 6
    const M = corpo(lcg() % 200 - 100, lcg() % 200 - 100, m)
    const N = corpo(lcg() % 200 - 100, lcg() % 200 - 100, m)
    if (det(mul(M, N)) !== det(M) * det(N)) todos = false
  }
  ok('§P1 N(xy) = N(x)·N(y) exato em 100 pares — a conservação é por produto', todos)
}

/* §P2 — a volta: inversa inteira nas unidades; adjunta = fibra */
{
  let voltaOk = true, fibraOk = true
  for (let m = 1; m <= 6; m++) {
    const Am = metalica(m)
    /* unidade: det=−1 → inversa INTEIRA = −adj (fator de potência 1) */
    const inv = [-Am[3], Am[1], Am[2], -Am[0]]     /* −adj(A_m), det=−1 */
    const id = mul(inv, Am)
    if (!(id[0] === 1n && id[1] === 0n && id[2] === 0n && id[3] === 1n)) voltaOk = false
    /* corpo geral: adj(M)·M = det(M)·I — o det é a fibra (o preço) */
    const M = corpo(3 + m, 2, m)
    const adj = [M[3], -M[1], -M[2], M[0]]
    const dI = mul(adj, M)
    const d = det(M)
    if (!(dI[0] === d && dI[3] === d && dI[1] === 0n && dI[2] === 0n)) fibraOk = false
  }
  ok('§P2 a inversa é INTEIRA nas unidades: A_m⁻¹·A_m = I (FP=1)', voltaOk)
  ok('§P2 adj(M)·M = det(M)·I — a fibra: dividir custa exatamente o det', fibraOk)
}

/* §P3 — reconstrução devolve massa E ordem (a escada da sequência) */
function escada (seq) {
  let E = 0n
  let f1 = 0, f2 = 0
  for (let i = 0; i < seq.length; i++) {
    const v = seq[i]
    E += v * v
    const r = Number(((v % 65537n) + 65537n) % 65537n)
    f1 = (f1 + (i + 1) * r) % P
    f2 = (f2 + ((i + 1) * (i + 1) % P) * r) % P
  }
  return { E, f1, f2 }
}
{
  const m = 2
  const Am = metalica(m)
  const inv = [-Am[3], Am[1], Am[2], -Am[0]]
  const corpos = []
  for (let t = 0; t < 40; t++) corpos.push(corpo(lcg() % 100 - 50, lcg() % 100 - 50, m))
  const seq0 = corpos.flatMap(M => M)
  const frente = corpos.map(M => mul(Am, M))       /* T: o passo da dinâmica */
  const volta = frente.map(M => mul(inv, M))       /* T⁻¹ */
  const seq1 = volta.flatMap(M => M)
  const e0 = escada(seq0), e1 = escada(seq1)
  const mudou = escada(frente.flatMap(M => M))
  ok('§P3 T muda o corpo (a frente não é a identidade)', mudou.E !== e0.E || mudou.f1 !== e0.f1)
  ok('§P3 T⁻¹∘T devolve a MASSA: E idêntica (BigInt exato)', e1.E === e0.E)
  ok('§P3 T⁻¹∘T devolve a ORDEM: Φ e Φ₂ idênticas — identidade ordenada, não só energia',
    e1.f1 === e0.f1 && e1.f2 === e0.f2)
  ok('§P3 e é identidade elemento a elemento (o observador completo)',
    seq1.every((v, i) => v === seq0[i]))
}

/* §P4 — o que EMERGE, registrado sem forçar */
{
  /* frações contínuas: A_m^k = [[p_k,p_{k−1}],[q_k,q_{k−1}]] de [m;m,…] */
  let cfOk = true
  for (let m = 1; m <= 4; m++) {
    let Ak = metalica(m)
    let p2 = 1n, p1 = BigInt(m), q2 = 0n, q1 = 1n   /* convergentes */
    for (let k = 2; k <= 10; k++) {
      Ak = mul(Ak, metalica(m))
      const p = BigInt(m) * p1 + p2, q = BigInt(m) * q1 + q2
      if (Ak[0] !== p || Ak[2] !== q) cfOk = false
      p2 = p1; p1 = p; q2 = q1; q1 = q
    }
  }
  ok('§P4 EMERGE frações contínuas: A_m^k carrega os convergentes de [m;m,m,…]', cfOk)

  /* Möbius: compor matrizes = compor transformações fracionárias
   * A(z)=(az+b)/(cz+d) com z=u/v inteiro: comparar por produto cruzado */
  let mobOk = true
  for (let t = 0; t < 50; t++) {
    const A = corpo(lcg() % 20 - 10, 1 + lcg() % 5, 1 + lcg() % 3)
    const B = corpo(lcg() % 20 - 10, 1 + lcg() % 5, 1 + lcg() % 3)
    const u = BigInt(lcg() % 50 + 1), v = BigInt(lcg() % 50 + 1)
    /* B(z) como fração */
    const u1 = B[0] * u + B[1] * v, v1 = B[2] * u + B[3] * v
    /* A(B(z)) */
    const u2 = A[0] * u1 + A[1] * v1, v2 = A[2] * u1 + A[3] * v1
    /* (A·B)(z) */
    const AB = mul(A, B)
    const u3 = AB[0] * u + AB[1] * v, v3 = AB[2] * u + AB[3] * v
    if (u2 * v3 !== u3 * v2) mobOk = false
  }
  ok('§P4 EMERGE Möbius: compor matrizes = compor fracionárias (produto cruzado exato)', mobOk)

  /* Pisot em inteiros: L_k² − Δ·F_k² = 4·(−1)^k — o vazamento compensado */
  let pisotOk = true
  for (let m = 1; m <= 6; m++) {
    const D = BigInt(m * m + 4)
    let F1 = 0n, F2 = 1n   /* F_0=0, F_1=1 (metálica) */
    let L1 = 2n, L2 = BigInt(m)
    for (let k = 1; k <= 20; k++) {
      const L = k === 1 ? L2 : null
      if (k > 1) {
        const Fn = BigInt(m) * F2 + F1; F1 = F2; F2 = Fn
        const Ln = BigInt(m) * L2 + L1; L1 = L2; L2 = Ln
      }
      const lhs = L2 * L2 - D * F2 * F2
      if (lhs !== (k % 2 === 0 ? 4n : -4n)) pisotOk = false
      void L
    }
  }
  ok('§P4 EMERGE Pisot inteiro: L_k² − Δ·F_k² = 4·(−1)^k, m=1..6, k=1..20 (sem um float)',
    pisotOk)

  console.log('#P4 NÃO aparecem nesta ponte: Dirac, convolução universal — ficam no mapa.')
}

/* §P5 — a mesma família lida pelo anel da Lei 8: Parseval fecha */
{
  const F = [0, 1]
  for (let k = 2; k < 256; k++) F.push((F[k - 1] + F[k - 2]) % P)   /* m=1 mod p */
  function powmod (b, e) {
    let r = 1; b %= P
    while (e > 0) { if (e & 1) r = r * b % P; b = b * b % P; e >>= 1 }
    return r
  }
  const g = powmod(3, 256)
  const X = new Array(256).fill(0)
  for (let k = 0; k < 256; k++) {
    let acc = 0
    const gk = powmod(g, k)
    let w = 1
    for (let i = 0; i < 256; i++) { acc = (acc + F[i] * w) % P; w = w * gk % P }
    X[k] = acc
  }
  let LHS = 0
  for (let k = 0; k < 256; k++) LHS = (LHS + X[k] * X[(256 - k) % 256]) % P
  const RHS = 256 * F.reduce((s, a) => (s + a * a) % P, 0) % P
  console.log(`anel: Σ X·X₋ = ${LHS} · 256·ΣF² = ${RHS}`)
  ok('§P5 a família metálica no anel: Parseval fecha — a leitura aditiva da cruz', LHS === RHS)
}

console.log('')
if (!falhas) {
  console.log('  A ponte aguenta peso: a transformada universal (avaliação nas folhas,')
  console.log('  realizada inteira pela matriz) conserva por PRODUTO (N(xy)=N(x)N(y)),')
  console.log('  a volta devolve massa E ordem, e da dinâmica emergem — sem forçar —')
  console.log('  as frações contínuas, Möbius e Pisot. Dirac e convolução: no mapa.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
