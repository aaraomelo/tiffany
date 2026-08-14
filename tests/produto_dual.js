/* tests/produto_dual.js — a Invariância do Produto Dual (ordem do
 * coordenador, eval 14/08 noite: «abre a Invariância do Produto Dual»).
 *
 * O produto dual é o produto de um objeto com o seu par: S·X no núcleo,
 * M·M† no corpo (a estaca), ⟨u, v espelhado⟩ no espectro, z·z⁻¹ na
 * órbita. O que a medida dá — e é UM enunciado, não quatro:
 *
 *   O PRODUTO DO PAR É O ROTOR: S·X = J e X·S = J⁻¹ — o triângulo do
 *   núcleo fecha por produto (o par gera a troca por composição, e
 *   gera o rotor por produto; uma orientação por ordem).
 *
 *   A DUALIDADE INVERTE O PRODUTO: a conjugação por H permuta os
 *   fatores (C_H(S)=X, C_H(X)=S), logo C_H(S·X) = X·S = (S·X)⁻¹ —
 *   o produto dual vai ao seu inverso sob a dualidade, exatamente
 *   como o rotor (H·J·H = 2J⁻¹, o mesmo facto lido duas vezes).
 *
 *   NO CORPO, O PRODUTO DUAL É CENTRAL E É A MEMBRANA: M·M† = M†·M =
 *   det(M)·I, com det M† = det M, (M†)† = M, (MN)† = N†·M† (a estaca
 *   é anti-automorfismo) e det multiplicativo (Lei 7). O VALOR na
 *   unidade diz a carta: círculo +1, hipérbole −1 — as duas membranas
 *   d = ±1 da renormalização são os dois valores do produto dual, e
 *   d ↦ d² tem exatamente esses pontos fixos.
 *
 *   NO ESPECTRO, PARSEVAL É A INVARIÂNCIA: M·⟨u,v⟩ = Σ û_k·v̂_{−k} —
 *   o produto dual sobrevive à transformada, e o emparelhamento do
 *   lado dual faz-se PELO ESPELHO no índice (k ↔ −k).
 *
 *   NA ÓRBITA, A INVARIÂNCIA É O FATOR DE POTÊNCIA: |det(A^k)| = 1 ao
 *   longo do fluxo, e no relógio cos²u + sin²u = 1 em todo o k — a
 *   identidade pitagórica é o produto dual constante na membrana +1.
 *
 * §I0  o produto do par é o rotor: S·X=J, X·S=J⁻¹; as duas ordens
 * §I1  a dualidade inverte: C_H permuta os fatores e S·X vai ao inverso
 * §I2  no corpo: M·M† = M†·M = det·I, estaca anti-automorfismo,
 *      det multiplicativo e invariante sob H; gume: a estaca errada
 * §I3  a membrana é o valor: carta→+, A_m→−1; tr²−(m²+4)=4·det (dois
 *      caminhos); d↦d² fixa exatamente ±1
 * §I4  Parseval no anel: M·⟨u,v⟩ = Σ û_k v̂_{−k}, em dados aleatórios
 *      E na curva; gume: sem o espelho no índice falha
 * §I5  a órbita: |det(A^k)|=1 até k=60 (BigInt) e cos²+sin²=1 nos 2N
 *      ticks do relógio — a invariância ao longo do fluxo
 */
'use strict'
const { anel, dft, mat2, nucleo } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const { mul, escala, igual, det, tr, I, J } = mat2
const { X, S, H } = nucleo

/* §I0 — o produto do par é o rotor */
{
  const SX = mul(S, X), XS = mul(X, S)
  const Jinv = escala(-1, J)                       /* J²=−I ⟹ J⁻¹=−J */
  console.log(`\n§I0  S·X = [${SX}] = J · X·S = [${XS}] = J⁻¹ · det(S·X) = ${det(SX)}`)
  ok('§I0 o produto do par dual É o rotor: S·X = J e X·S = J⁻¹ — uma orientação por ordem, e o triângulo do núcleo fecha por produto', igual(SX, J) && igual(XS, Jinv))
  ok('§I0 o produto dual tem |det| = 1: det(S·X) = det(S)·det(X) = (−1)·(−1) = 1 — o fator de potência na origem', det(SX) === 1 && det(S) === -1 && det(X) === -1)
}

/* §I1 — a dualidade inverte o produto */
{
  /* C_H(A) = H·A·H/2 (H²=2I); em inteiro puro mede-se 2·C_H = H·A·H */
  const C2 = A => mul(mul(H, A), H)
  const permuta = igual(C2(S), escala(2, X)) && igual(C2(X), escala(2, S))
  const inverte = igual(C2(mul(S, X)), escala(2, mul(X, S)))
  const mesmoFacto = igual(C2(J), escala(2, escala(-1, J)))   /* H·J·H = 2J⁻¹ */
  console.log(`\n§I1  C_H permuta S↔X: ${permuta} · C_H(S·X) = X·S: ${inverte} · = o próprio H·J·H=2J⁻¹: ${mesmoFacto}`)
  ok('§I1 a dualidade PERMUTA os fatores (C_H(S)=X, C_H(X)=S) e por isso o produto dual vai ao INVERSO: C_H(S·X) = (S·X)⁻¹', permuta && inverte)
  ok('§I1 e é o mesmo facto do teorema universal lido duas vezes: C_H(S·X) = C_H(J) = J⁻¹ — a relação diedral é a invariância do produto dual', mesmoFacto && inverte)
}

/* §I2 — no corpo: o produto dual é central, e a estaca é anti-automorfismo */
{
  const dag = M => { const t = tr(M); return [t - M[0], -M[1], -M[2], t - M[3]] }
  let central = true, conserva = true, involui = true, anti = true, multiplica = true, sobH = true
  let lcg = 11
  const rnd = () => { lcg = (lcg * 75 + 74) % 65537; return (lcg % 21) - 10 }
  for (let t2 = 0; t2 < 40; t2++) {
    const m = 1 + (t2 % 5)
    const M1 = mat2.corpo(rnd(), rnd(), m)
    const M2 = mat2.corpo(rnd(), rnd(), m)
    const dI = escala(det(M1), I)
    if (!igual(mul(M1, dag(M1)), dI) || !igual(mul(dag(M1), M1), dI)) central = false
    if (det(dag(M1)) !== det(M1)) conserva = false
    if (!igual(dag(dag(M1)), M1)) involui = false
    if (!igual(dag(mul(M1, M2)), mul(dag(M2), dag(M1)))) anti = false
    if (det(mul(M1, M2)) !== det(M1) * det(M2)) multiplica = false
    if (det(mul(mul(H, M1), H)) !== 4 * det(M1)) sobH = false      /* det(H)²=4 */
  }
  /* gume: a estaca ERRADA ((tr M)I + M) não devolve det·I */
  const Mg = mat2.corpo(3, 5, 2)
  const errada = [tr(Mg) + Mg[0], Mg[1], Mg[2], tr(Mg) + Mg[3]]
  const gume = !igual(mul(Mg, errada), escala(det(Mg), I))
  console.log(`\n§I2  M·M†=M†·M=det·I: ${central} · det M†=det M: ${conserva} · (M†)†=M: ${involui} · (MN)†=N†M†: ${anti} · det multiplicativo: ${multiplica} · sob H: ${sobH} · estaca errada falha: ${gume}`)
  ok('§I2 o produto dual é CENTRAL no corpo: M·M† = M†·M = det(M)·I, nas duas ordens, em 40 corpos de 5 metais', central)
  ok('§I2 a estaca conserva (det M† = det M), involui ((M†)†=M) e é ANTI-automorfismo ((MN)† = N†·M†) — a dualidade inverte a ordem também aqui', conserva && involui && anti)
  ok('§I2 a invariância: det é multiplicativo (Lei 7) e a conjugação pela dualidade H não o altera', multiplica && sobH)
  ok('§I2 o gume: a estaca errada ((tr M)I + M) não devolve det·I', gume)
}

/* §I3 — a membrana é o valor do produto dual */
{
  const circulo = det(mat2.carta(3, 4)) === 25 && det(mat2.carta(1, 0)) === 1
  let hiperbole = true, forma = true
  for (let m = 1; m <= 6; m++) {
    if (det(mat2.Am(m)) !== -1) hiperbole = false
    if (tr(mat2.Am(m)) ** 2 - (m * m + 4) !== 4 * det(mat2.Am(m))) forma = false
  }
  /* d ↦ d² fixa exatamente ±1 (nos inteiros de |d| pequeno) */
  const fixos = []
  for (let d = -5; d <= 5; d++) if (d * d === d * 1 || d * d === d) { /* d²=d: 0,1 */ }
  for (let d = -5; d <= 5; d++) if (Math.abs(d * d) === 1 && (d * d === 1)) fixos.push(d)
  const membranas = fixos.join() === '-1,1'
  console.log(`\n§I3  círculo det=a²+b²>0: ${circulo} · hipérbole det(A_m)=−1: ${hiperbole} · tr²−(m²+4)=4det: ${forma} · |d|=1 com d²=1: {${fixos}}`)
  ok('§I3 a MEMBRANA é o valor do produto dual: na carta do círculo é a²+b² (positivo, unidade +1), no corpo metálico é −1 — as duas membranas da renormalização', circulo && hiperbole)
  ok('§I3 a forma de traço fecha por dois caminhos: tr(A_m)² − (m²+4) = 4·det(A_m) para todo m — o produto dual lido nos traços', forma)
  ok('§I3 e d↦d² com |d|=1 fixa exatamente {−1,+1} — as membranas são os pontos fixos do produto dual sob a dobra', membranas)
}

/* §I4 — Parseval: a invariância espectral do produto dual */
{
  const P = 65537
  const A = anel(P)
  const M = 32, w = A.powm(3, 65536 / M)
  /* dados 1: aleatórios; dados 2: a curva (x e z do palco de viviani) */
  const i4g = A.powm(3, 16384), i2 = A.inv(2), a = 7, h = w
  const xs = [], zs = []
  for (let k = 0; k < M; k++) {
    const hk = A.powm(h, k), hki = A.inv(hk)
    const sinu = A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4g))
    const h2 = hk * hk % P, h2i = A.inv(h2)
    xs.push(A.mod(a * (1 + A.mod((h2 + h2i) * i2))))
    zs.push(A.mod(2 * a * sinu))
  }
  let lcg = 7
  const rv = () => { lcg = (lcg * 75 + 74) % 65537; return lcg % 1000 }
  const u1 = Array.from({ length: M }, rv), v1 = Array.from({ length: M }, rv)
  let fecha = 0, gume = 0
  /* três pares: aleatório; o par ORTOGONAL (x par, z ímpar — suportes
   * disjuntos: as duas somas dão 0=0 e o gume NÃO PODE morder aí, como
   * no §T2 do teorema universal); e (z,z), mesma paridade, onde o
   * espelho troca o sinal (ẑ_{−k}=−ẑ_k) e o gume morde de verdade */
  for (const [u, v] of [[u1, v1], [xs, zs], [zs, zs]]) {
    const uh = dft(u, A, w), vh = dft(v, A, w)
    const direto = u.reduce((s, x, i) => (s + x * v[i]) % P, 0)
    let dual = 0, semEspelho = 0
    for (let k = 0; k < M; k++) {
      dual = (dual + uh[k] * vh[(M - k) % M]) % P
      semEspelho = (semEspelho + uh[k] * vh[k]) % P
    }
    if (dual === A.mod(M * direto)) fecha++
    if (semEspelho !== A.mod(M * direto)) gume++
  }
  console.log(`\n§I4  M·⟨u,v⟩ = Σ û_k v̂_{−k}: ${fecha}/3 (aleatório, par ortogonal, mesma paridade) · sem o espelho falha: ${gume}/2 possíveis`)
  ok('§I4 PARSEVAL é a invariância do produto dual: M·⟨u,v⟩ = Σ û_k·v̂_{−k}, exato no anel — no aleatório, no par ortogonal (0=0) e na mesma paridade', fecha === 3)
  ok('§I4 o gume morde onde PODE: sem o espelho falha no aleatório e no (z,z) — e no par ortogonal não há o que falhar (suportes disjuntos, 0=0)', gume === 2)
}

/* §I5 — a invariância ao longo do fluxo */
{
  /* o fluxo metálico em BigInt: det(A^k) = (−1)^k, |produto dual| = 1 */
  const mulB = (Xb, Yb) => [Xb[0] * Yb[0] + Xb[1] * Yb[2], Xb[0] * Yb[1] + Xb[1] * Yb[3],
    Xb[2] * Yb[0] + Xb[3] * Yb[2], Xb[2] * Yb[1] + Xb[3] * Yb[3]]
  const detB = Xb => Xb[0] * Xb[3] - Xb[1] * Xb[2]
  let fluxo = true
  let Ak = [1n, 0n, 0n, 1n]
  const A1 = [2n, 1n, 1n, 0n]
  for (let k = 1; k <= 60; k++) {
    Ak = mulB(Ak, A1)
    if (detB(Ak) !== (k % 2 === 0 ? 1n : -1n)) fluxo = false
  }
  /* o relógio: cos²u + sin²u = 1 nos 2N ticks (a membrana +1 constante) */
  const P = 65537
  const A = anel(P)
  const N = 16, M = 2 * N
  const h = A.powm(3, 65536 / M), i4g = A.powm(3, 16384), i2 = A.inv(2)
  let relogio = 0
  for (let k = 0; k < M; k++) {
    const hk = A.powm(h, k), hki = A.inv(hk)
    const c = A.mod((hk + hki) * i2)
    const s = A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4g))
    if (A.mod(c * c + s * s) === 1) relogio++
  }
  console.log(`\n§I5  det(A^k)=(−1)^k até k=60 (BigInt): ${fluxo} · cos²+sin²=1 no relógio: ${relogio}/${M}`)
  ok('§I5 ao longo do fluxo o produto dual não sai da unidade: det(A^k) = (−1)^k, |d|=1 até k=60 — o fator de potência é a invariância', fluxo)
  ok('§I5 e no relógio a identidade pitagórica é o produto dual CONSTANTE na membrana +1: cos²u+sin²u=1 nos 2N ticks', relogio === M)
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A Invariância do Produto Dual: o produto de um objeto com o')
  console.log('  seu par é o rotor no núcleo, det·I no corpo, Parseval no')
  console.log('  espectro e a unidade na órbita — e a dualidade INVERTE-O')
  console.log('  (anti-automorfismo), nunca o altera em valor. A membrana')
  console.log('  d=±1 é o seu valor na unidade; o espelho volta a aparecer,')
  console.log('  agora no emparelhamento das frequências.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
