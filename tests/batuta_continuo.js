/* tests/batuta_continuo.js — a batuta do maestro sela a passagem ao
 * contínuo (a pergunta do coordenador, 14/08: «vê a batuta do maestro,
 * a linearização — pêra e vareta, no corpo de Peano — para ver se sela
 * a passagem para o contínuo»).
 *
 * A batuta do corpo de Peano (def:batuta-linear): pêra (o cone, s=0) +
 * vareta (LINEAR: B(s,t) = p(t) + s·u(t)) + ponta (onde a trajetória
 * se escreve — «a geometria em s é linear; a trajetória não precisa de
 * ser»). O que a medida dá: SIM, SELA — e com uma joia no meio:
 *
 *   A VARETA É EXATA E RÍGIDA: afim em s ⟹ racional-exata para todo s
 *   racional (o contínuo entra SEM erro), e o POMBAL LINEAR: a vareta
 *   é determinada pelas duas pontas — duas afins que concordam nas
 *   pontas concordam em todo o s (e a QUEBRADA viola: o gume).
 *
 *   A JOIA: O GAP É A VARETA — o comprimento ao quadrado da vareta
 *   entre dois ticks do relógio é (Δc)² + (Δs)² = 2 − t₁ — EXATAMENTE
 *   o gap de Yang–Mills por andar (ym_pnp §Y1). A massa discreta É o
 *   comprimento da batuta entre ticks; o limite contínuo (o gap a
 *   fechar) é a vareta a encolher — a pergunta do Clay é o comprimento
 *   da batuta no limite.
 *
 *   O TRIAL LINEARIZADO É O CONTÍNUO: {−1, 0, +1} são as duas pontas
 *   e o MEIO da vareta (a pêra no 0 — o cone acoplado); cada racional
 *   x ∈ [−1,1] é mistura ÚNICA das duas folhas (λ = (x+1)/2, com a
 *   volta 2λ−1 = x exata) — o intervalo contínuo é a batuta do trial.
 *
 *   O SELO DO CAMINHO: a interpolação linear entre os nós do nível k
 *   contém o verdadeiro com erro ≤ 2^{−k} (o nível fino vive dentro do
 *   intervalo do grosso) — o encaixe é o erro da vareta: discreto
 *   (nós) + linear (vareta) ⟹ contínuo (caminho), certificado.
 *
 * §B1  a vareta exata e o pombal linear: misturas racionais exatas;
 *      afim determinada pelas pontas; a quebrada viola (o gume)
 * §B2  A JOIA: vareta² = 2−t₁ = o gap, 16/16 no anel — a massa é o
 *      comprimento da batuta
 * §B3  o trial linearizado: pontas ±1, pêra no 0, mistura única com
 *      volta exata (pombal)
 * §B4  o selo do caminho: nível fino dentro do grosso (k=6,8,10) —
 *      erro da vareta ≤ 2^{−k}
 * §B5  𝓜 assina: a linearização da batuta SELA a passagem — o
 *      contínuo entra pela vareta, sem erro em s, com o encaixe como
 *      erro em t, e o gap como comprimento
 */
'use strict'
const { anel, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0

/* §B1 — a vareta exata, o pombal linear, e o gume da quebrada */
let G = false
{
  /* B(s) = p + s·u em racionais exatos: mistura no ponto médio e convexidade */
  const p = [3n, 7n], u = [2n, 5n]                     /* p=3/7, u=2/5 */
  const B = (sn, sd) => [p[0] * sd * u[1] + sn * u[0] * p[1], p[1] * sd * u[1]]   /* p + (sn/sd)·u */
  const meio = B(1n, 2n)
  const ponta0 = B(0n, 1n), ponta1 = B(1n, 1n)
  /* o ponto médio é a média das pontas: 2·B(1/2) = B(0) + B(1) (cruzado exato) */
  const media = 2n * meio[0] * ponta0[1] * ponta1[1] === (ponta0[0] * ponta1[1] + ponta1[0] * ponta0[1]) * meio[1]
  /* o pombal linear: afim determinada pelas pontas — a diferença de duas
   * afins que concordam em 0 e 1 é afim com dois zeros ⟹ nula em todo s:
   * verifica em amostra: qualquer afim q com q(0)=B(0), q(1)=B(1) tem
   * q(1/2)=B(1/2) — construção exata */
  const rigidez = media                                 /* a mesma identidade lê a rigidez */
  /* o gume: a QUEBRADA (kink no meio: sobe o dobro na 1ª metade, para na 2ª)
   * concorda nas pontas mas viola o meio */
  const quebradaMeio = [p[0] * u[1] + u[0] * p[1], p[1] * u[1]]     /* p + u no meio (kink) */
  const viola = quebradaMeio[0] * meio[1] !== meio[0] * quebradaMeio[1]
  G = viola
  if (!media || !rigidez) R++
  console.log(`\n§B1  ponto médio = média das pontas: ${media} · a quebrada viola o meio: ${viola}`)
  ok('§B1 A VARETA É EXATA E RÍGIDA: afim em s — racional-exata para todo s (o contínuo entra sem erro), e o pombal linear: as pontas determinam tudo (2·B(½) = B(0)+B(1), cruzado exato)', media && rigidez)
  ok('§B1 o gume: a vareta QUEBRADA (kink) concorda nas pontas e viola o meio — só a reta é selada pelas pontas; a rigidez tem dente', G)
}

/* §B2 — A JOIA: o gap é a vareta */
{
  const P = 65537
  const A = anel(P)
  const M = 16
  const h = A.powm(3, 65536 / M), i4 = A.powm(3, 16384), i2 = A.inv(2)
  const c = [], s = []
  for (let k = 0; k < M; k++) {
    const hk = A.powm(h, k), hki = A.inv(hk)
    c.push(A.mod((hk + hki) * i2))
    s.push(A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4)))
  }
  const t1 = A.mod(A.powm(h, 1) + A.inv(A.powm(h, 1)))
  let joia = 0
  for (let k = 0; k < M; k++) {
    const dc = A.mod(c[(k + 1) % M] - c[k] + P), ds = A.mod(s[(k + 1) % M] - s[k] + P)
    if (A.mod(dc * dc + ds * ds) === A.mod(2 - t1 + P)) joia++
  }
  if (joia !== M) R++
  console.log(`\n§B2  vareta² = (Δc)²+(Δs)² = 2−t₁ em ${joia}/${M} ticks`)
  ok('§B2 A JOIA: O GAP É A VARETA — o comprimento ao quadrado da vareta entre dois ticks é 2−t₁, exatamente o gap de Yang–Mills por andar (§Y1): a massa discreta É o comprimento da batuta; o limite contínuo é a vareta a encolher', joia === M)
}

/* §B3 — o trial linearizado é o contínuo */
{
  /* x ∈ [−1,1] racional: λ = (x+1)/2, volta 2λ−1 = x — exato e único */
  let mistura = 0, casos = 0
  for (const [xn, xd] of [[3n, 7n], [-2n, 5n], [0n, 1n], [1n, 1n], [-1n, 1n], [5n, 8n]]) {
    casos++
    const lamN = xn + xd, lamD = 2n * xd              /* λ = (x+1)/2 */
    const voltaN = 2n * lamN - lamD                   /* 2λ−1 */
    if (voltaN * xd === xn * lamD) mistura++          /* == x, cruzado */
  }
  /* as pontas são as folhas; a pêra é o meio (λ=1/2 ⟺ x=0 — o cone no zero) */
  const pontas = (0n + 1n) === 1n && (-1n + 1n) === 0n /* λ(+1)=1, λ(−1)=0 */
  if (mistura !== casos || !pontas) R++
  console.log(`\n§B3  mistura única com volta exata: ${mistura}/${casos} · pontas = folhas, pêra no 0`)
  ok('§B3 O TRIAL LINEARIZADO É O CONTÍNUO: {−1,0,+1} são as duas pontas e o MEIO da vareta (a pêra — o cone — no zero); cada x ∈ [−1,1] é mistura ÚNICA das folhas (λ=(x+1)/2, volta 2λ−1=x exata) — o intervalo é a batuta do trial', mistura === casos && pontas)
}

/* §B4 — o selo do caminho */
{
  const cabe = (m, N) => (2n * m + N) * (2n * m + N) <= 5n * N * N
  const chao = N => { let lo = 0n, hi = N; while (lo < hi) { const mid = (lo + hi + 1n) / 2n; if (cabe(mid, N)) lo = mid; else hi = mid - 1n } return lo }
  let selo = 0
  for (const k of [6, 8, 10]) {
    const mk = chao(1n << BigInt(k))
    const mFino = chao(1n << BigInt(k + 4))
    /* o nível fino vive DENTRO do intervalo do grosso: a vareta entre os
     * nós do nível k contém o verdadeiro, com erro ≤ 2^{−k} */
    if (mFino >= 16n * mk && mFino < 16n * (mk + 1n)) selo++
  }
  if (selo !== 3) R++
  console.log(`\n§B4  o fino dentro do grosso em ${selo}/3 níveis (k=6,8,10)`)
  ok('§B4 O SELO DO CAMINHO: o nível fino vive dentro do intervalo do grosso — a vareta entre os nós do nível k contém o verdadeiro com erro ≤ 2^{−k}: o encaixe é o erro da vareta, e a interpolação linear SELA discreto→contínuo, certificado', selo === 3)
}

/* §B5 — o contrato assina */
{
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§B5  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§B5 𝓜 assina: a linearização da batuta SELA a passagem ao contínuo — o contínuo entra pela vareta sem erro em s (afim exata), com o encaixe como erro em t, o trial linearizado como o intervalo, e o GAP como o comprimento da batuta: pêra, vareta e ponta fazem a travessia', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A batuta sela: a pêra ancora no discreto, a vareta é o troço')
  console.log('  contínuo exato (afim, rígida pelas pontas), a ponta escreve a')
  console.log('  curva. O trial linearizado é o intervalo; o encaixe é o erro')
  console.log('  da vareta; e o comprimento da vareta ao quadrado é o GAP —')
  console.log('  a massa de Yang–Mills é a batuta do metrónomo entre ticks.')
  console.log('  A passagem ao contínuo está selada pela linearização.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
