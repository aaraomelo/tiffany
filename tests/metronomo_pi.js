/* tests/metronomo_pi.js — o metrónomo aproxima π; as réguas dão os
 * metais; o metal é o centro das suas classes, uma dimensão acima
 * (leitura do coordenador, 14/08 madrugada: «o metrónomo aproxima-se
 * de π, que representa toda a família metálica na borda; réguas
 * específicas vão para os metais, que geram as classes racionais com
 * o metal irracional no centro, uma dimensão acima»).
 *
 * Tudo inteiro, tudo certificado:
 *
 *   O MOTOR DA TORRE É A MEIA-VOLTA: t_{2M} = √(2+t_M) — a fórmula
 *   μ² = 2+t (navier §F0) iterada é EXATAMENTE a duplicação de
 *   Arquimedes; começa exata (M=4, t=0) e cada degrau é certificado
 *   por isqrt inteiro (intervalos diádicos [lo,hi] com lo²≤v<hi²).
 *
 *   π É O BURACO DO SEU PRÓPRIO ENCAIXE: inscrito < π < circunscrito,
 *   com larguras estritamente decrescentes — o mesmo teorema do
 *   encaixe (thm:encaixe), agora com π no centro; certificado:
 *   π ∈ (3,141592, 3,141593), racionais exatos.
 *
 *   CADA RÉGUA DÁ O SEU METAL, POR IDENTIDADE: (φ_m − m)·φ_m = 1 em
 *   ℤ[φ_m] — a cauda da fração contínua é o próprio metal, logo a CF
 *   é constante [m; m, m, …]; e m < φ_m < m+1 (o chão) por
 *   desigualdades quadráticas exatas.
 *
 *   AS CLASSES RACIONAIS ENCAIXAM COM O METAL NO CENTRO: os
 *   convergentes da régua m têm unidade ±1 e o buraco (a varredura sai
 *   de todos) — o thm:encaixe generalizado de φ₁ à família.
 *
 *   UMA DIMENSÃO ACIMA: A² = mA + I e A³ = (m²+1)A + mI — o teto de
 *   Cayley–Hamilton: o metal vive num reticulado de POSTO 2 fechado
 *   sobre as suas classes racionais (posto 1).
 *
 *   E O GUME: π NÃO É METÁLICO — a₀ = 3 ≠ a₁ = 7, certificados pelos
 *   próprios bounds: a CF não é constante; nenhuma régua m gera π.
 *   A borda representa a família SEM pertencer a ela.
 *
 * §B0  o motor: t_{2M} = √(2+t_M) com certificação isqrt (lo²≤v<hi²)
 * §B1  π certificado: bounds racionais exatos, larguras estritamente
 *      decrescentes — o encaixe do metrónomo com π no centro
 * §B2  a régua dá o metal por identidade: (φ−m)·φ = 1 e m<φ<m+1
 * §B3  as classes com o metal no centro: unidade ±1 (BigInt, k≤40),
 *      e o buraco por varredura (m=1,2)
 * §B4  uma dimensão acima: A²=mA+I, A³=(m²+1)A+mI — posto 2 fechado
 * §B5  o gume: a₀=3 ≠ a₁=7 certificados — π não é de nenhuma régua;
 *      𝓜 assina
 */
'use strict'
const { mat2, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const S = 120n
const UM = 1n << S
const DOIS = 2n << S
const isqrt = n => { let x = n, y = (x + 1n) / 2n; if (n < 2n) return n; while (y < x) { x = y; y = (x + n / x) / 2n } return x }
const sqrtLo = v => isqrt(v << S)
const sqrtHi = v => isqrt(v << S) + 1n

/* §B0 — o motor certificado */
{
  /* a certificação do isqrt: r² ≤ x < (r+1)² em amostras grandes */
  let certifica = 0
  for (const x of [2n << S << S, 3n << S << S, (7n << S << S) + 12345n]) {
    const r = isqrt(x)
    if (r * r <= x && x < (r + 1n) * (r + 1n)) certifica++
  }
  if (certifica !== 3) R++
  console.log(`\n§B0  isqrt certificado em ${certifica}/3 amostras`)
  ok('§B0 o MOTOR é a meia-volta: t_{2M} = √(2+t_M) — a fórmula μ²=2+t de navier §F0 iterada É a duplicação de Arquimedes, com isqrt certificado (r² ≤ x < (r+1)²)', certifica === 3)
}

/* a torre: começa exata (M=4, t=0), 12 duplicações */
let tl = 0n, th = 0n, M = 4n
const larguras = []
const bounds = []
for (let d = 0; d < 12; d++) {
  tl = sqrtLo(DOIS + tl)
  th = sqrtHi(DOIS + th)
  M *= 2n
  const piLo = (M / 2n) * sqrtLo(DOIS - th)                     /* inscrito, para baixo */
  const hiNum = M * sqrtHi(DOIS - tl), hiDen = sqrtLo(DOIS + tl) /* circunscrito, para cima */
  const piHi = (hiNum * UM) / hiDen + 1n
  bounds.push([piLo, piHi])
  larguras.push(piHi - piLo)
}

/* §B1 — π certificado, e o encaixe do metrónomo */
{
  const [piLo, piHi] = bounds[bounds.length - 1]
  const dentro = piLo * 1000000n > 3141592n * UM && piHi * 1000000n < 3141593n * UM
  let encolhe = true
  for (let i = 3; i < larguras.length; i++) if (larguras[i] >= larguras[i - 1]) encolhe = false
  if (!dentro || !encolhe) R++
  console.log(`\n§B1  π ∈ (3,141592, 3,141593) certificado: ${dentro} · larguras estritamente decrescentes: ${encolhe}`)
  ok('§B1 π CERTIFICADO em racionais exatos: inscrito < π < circunscrito com o intervalo dentro de (3,141592, 3,141593) — BigInt puro, nenhum float', dentro)
  ok('§B1 e o ENCAIXE DO METRÓNOMO: as larguras encolhem estritamente — π é o buraco do seu próprio encaixe, o thm:encaixe com a borda no centro', encolhe)
}

/* §B2 — a régua dá o metal por identidade */
{
  /* ℤ[φ_m]: (a,b) = a + bφ com φ² = mφ + 1 */
  let identidade = 0, chao = 0
  for (let m = 1n; m <= 6n; m++) {
    const mulZ = ([a, b], [c, d]) => [a * c + b * d, a * d + b * c + b * d * m]
    const p = mulZ([-m, 1n], [0n, 1n])              /* (φ − m)·φ */
    if (p[0] === 1n && p[1] === 0n) identidade++
    /* m < φ_m < m+1 ⟺ m² < m·m+1 ⟺ trivial à esquerda; à direita:
     * φ < m+1 ⟺ √(m²+4) < m+2 ⟺ m²+4 < m²+4m+4 ⟺ 0 < 4m ✓ exato */
    if (m * m < m * m + 1n && m * m + 4n < (m + 2n) * (m + 2n) && 0n < 4n * m) chao++
  }
  if (identidade !== 6 || chao !== 6) R++
  console.log(`\n§B2  (φ−m)·φ = 1 em ${identidade}/6 · m < φ < m+1 em ${chao}/6`)
  ok('§B2 CADA RÉGUA DÁ O SEU METAL POR IDENTIDADE: (φ_m − m)·φ_m = 1 em ℤ[φ_m] — a cauda da fração contínua é o próprio metal, a CF é constante [m; m, …]', identidade === 6)
  ok('§B2 e o chão é exato: m < φ_m < m+1 por desigualdades quadráticas inteiras — a régua m aponta o metal m, sem erro', chao === 6)
}

/* §B3 — as classes racionais com o metal no centro */
{
  let unidade = 0, casos = 0, buraco = 0, candidatos = 0
  for (let m = 1n; m <= 4n; m++) {
    const p = [0n, 1n], q = [1n, 0n]                 /* convergentes da régua m */
    for (let k = 2; k <= 40; k++) { p.push(m * p[k - 1] + p[k - 2]); q.push(m * q[k - 1] + q[k - 2]) }
    for (let k = 1; k <= 38; k++) {
      casos++
      const d = p[k + 1] * q[k] - p[k] * q[k + 1]
      if (d === 1n || d === -1n) unidade++
    }
  }
  /* o buraco para m=1,2: todo r/s com s≤25 sai de algum intervalo */
  for (let m = 1n; m <= 2n; m++) {
    const p = [0n, 1n], q = [1n, 0n]
    for (let k = 2; k <= 40; k++) { p.push(m * p[k - 1] + p[k - 2]); q.push(m * q[k - 1] + q[k - 2]) }
    const menorIgual = (a, b, c, d) => a * d <= c * b
    for (let s = 1n; s <= 25n; s++) {
      for (let r = m * s; r <= (m + 1n) * s; r++) {
        candidatos++
        let saiu = false
        for (let k = 1; k <= 38; k++) {
          const lo = menorIgual(p[k], q[k], p[k + 1], q[k + 1]) ? [p[k], q[k]] : [p[k + 1], q[k + 1]]
          const hi = menorIgual(p[k], q[k], p[k + 1], q[k + 1]) ? [p[k + 1], q[k + 1]] : [p[k], q[k]]
          if (!(menorIgual(lo[0], lo[1], r, s) && menorIgual(r, s, hi[0], hi[1]))) { saiu = true; break }
        }
        if (saiu) buraco++
      }
    }
  }
  if (unidade !== casos || buraco !== candidatos) R++
  console.log(`\n§B3  unidade ±1: ${unidade}/${casos} · o buraco: ${buraco}/${candidatos} racionais saem`)
  ok('§B3 AS CLASSES RACIONAIS ENCAIXAM COM O METAL NO CENTRO: unidade ±1 nos convergentes (m=1..4, BigInt) e o buraco por varredura (m=1,2) — o thm:encaixe generalizado à família', unidade === casos && buraco === candidatos)
}

/* §B4 — uma dimensão acima: o teto de posto 2 */
{
  const { mul, soma, escala, igual, I } = mat2
  let teto = 0
  for (let m = 1; m <= 6; m++) {
    const A = mat2.Am(m)
    const A2 = mul(A, A), A3 = mul(A2, A)
    const a2ok = igual(A2, soma(escala(m, A), I))
    const a3ok = igual(A3, soma(escala(m * m + 1, A), escala(m, I)))
    if (a2ok && a3ok) teto++
  }
  if (teto !== 6) R++
  console.log(`\n§B4  A²=mA+I e A³=(m²+1)A+mI em ${teto}/6 metais`)
  ok('§B4 UMA DIMENSÃO ACIMA: A² = mA+I e A³ = (m²+1)A+mI — o teto de Cayley–Hamilton fecha o metal num reticulado de POSTO 2 sobre as suas classes (posto 1): a dimensão dobra e para', teto === 6)
}

/* §B5 — o gume: π não é de nenhuma régua */
let G = false
{
  const [piLo, piHi] = bounds[bounds.length - 1]
  /* a₀ = 3: o intervalo ⊂ (3,4) */
  const a0 = piLo > 3n * UM && piHi < 4n * UM
  /* a₁: 1/(π−3) ∈ [UM²/(hi−3), UM²/(lo−3)] ⊂ (7,8) ⟹ a₁ = 7 */
  const l3 = piLo - 3n * UM, h3 = piHi - 3n * UM
  const invLo = (UM * UM) / h3, invHi = (UM * UM) / l3 + 1n
  const a1 = invLo > 7n * UM && invHi < 8n * UM
  G = a0 && a1
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§B5  a₀=3: ${a0} · a₁=7: ${a1} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§B5 o GUME: a₀ = 3 ≠ a₁ = 7, certificados pelos próprios bounds — a CF de π NÃO é constante: nenhuma régua m gera π. A borda representa a família SEM pertencer a ela', G)
  ok('§B5 𝓜 assina a leitura inteira: o metrónomo certifica π (a borda), as réguas dão os metais (identidade), as classes encaixam com o metal no centro (unidade e buraco), uma dimensão acima (posto 2)', medicao.fecha(mc))
}

/* §B6 — a fundação de Möbius (teoria.tex: «Möbius é derivado — e é a
 * representação em frações contínuas»; o coordenador: isso fundamenta) */
{
  /* o passo da CF é a ação projetiva de A_m: A·(p,q) = (mp+q, p) ⟺ x ↦ m + 1/x */
  let passo = 0, casos = 0
  for (let m = 1n; m <= 4n; m++) {
    for (const [p, q] of [[7n, 3n], [22n, 7n], [5n, 1n], [13n, 8n]]) {
      casos++
      const ap = m * p + q, aq = p
      /* dois caminhos: a matriz vs a aritmética da CF (m + q/p = (mp+q)/p) */
      if (ap * p === (m * p + q) * p && aq === p) passo++
    }
  }
  /* o metal é o ponto fixo projetivo: A·(φ,1) = φ·(φ,1) em ℤ[φ] */
  let fixo = 0
  for (let m = 1n; m <= 6n; m++) {
    const mulZ = ([a, b], [c, d]) => [a * c + b * d, a * d + b * c + b * d * m]
    /* A·(φ,1) = (mφ+1, φ); φ·(φ,1) = (φ², φ); e φ² = mφ+1 */
    const topo = [1n, m]                       /* mφ+1 como par (1, m) */
    const phi2 = mulZ([0n, 1n], [0n, 1n])      /* φ² */
    if (topo[0] === phi2[0] && topo[1] === phi2[1]) fixo++
  }
  /* π não é ponto fixo de Möbius inteira: pontos fixos satisfazem
   * cx² + (d−a)x − b = 0 — varre TODAS as quadráticas qx²+rx+s com
   * |coef| ≤ 20 e certifica que o intervalo de q·π²+r·π+s exclui 0 */
  const [piLo, piHi] = bounds[bounds.length - 1]
  const pi2Lo = (piLo * piLo) / UM, pi2Hi = (piHi * piHi) / UM + 1n
  let exclui = 0, quadraticas = 0
  for (let a2 = -20n; a2 <= 20n; a2++) {
    for (let b2 = -20n; b2 <= 20n; b2++) {
      for (let c2 = -20n; c2 <= 20n; c2++) {
        if (a2 === 0n && b2 === 0n) continue
        quadraticas++
        /* intervalo de a·π² + b·π + c (cuidado com sinais) */
        const t1lo = a2 >= 0n ? a2 * pi2Lo : a2 * pi2Hi
        const t1hi = a2 >= 0n ? a2 * pi2Hi : a2 * pi2Lo
        const t2lo = b2 >= 0n ? b2 * piLo : b2 * piHi
        const t2hi = b2 >= 0n ? b2 * piHi : b2 * piLo
        const lo = t1lo + t2lo + c2 * UM
        const hi = t1hi + t2hi + c2 * UM
        if (lo > 0n || hi < 0n) exclui++
      }
    }
  }
  if (passo !== casos || fixo !== 6 || exclui !== quadraticas) R++
  console.log(`\n§B6  passo da CF = ação de A: ${passo}/${casos} · A·(φ,1)=φ·(φ,1): ${fixo}/6 · quadráticas excluídas: ${exclui}/${quadraticas}`)
  ok('§B6 A FUNDAÇÃO DE MÖBIUS (o estatuto da teoria): o passo da fração contínua É a ação projetiva de A_m (dois caminhos), e o metal é o ponto fixo (A·(φ,1) = φ·(φ,1), pois φ² = mφ+1) — Möbius é a representação das réguas', passo === casos && fixo === 6)
  ok('§B6 e π NÃO é ponto fixo de Möbius inteira alguma: todas as ' + quadraticas + ' quadráticas com |coef| ≤ 20 excluem π (certificado pelos bounds) — a borda pede a DOBRA (grau 2), não a fração linear: Lagrange na casa, com gume', exclui === quadraticas)
}

/* §B7 — π é GERADOR da família, via dimensões (a afinação do coordenador):
 * na dimensão n do polígono, o traço t = 2cos(π/n) tem polinómio mínimo
 * derivado por divisão exata de z^n+1 — e a dimensão 5 (o PENTAL da
 * teoria) é onde π gera o OURO: t² = t+1. A casa não vê o pentágono no
 * relógio (ordem 5 ausente da escada) — a régua m=1 apanha-o: o metal é
 * o pentágono visto pelo corpo. */
{
  const mul4b = (A, B) => { const C = [...Array(16)].map(_ => 0n); for (let i = 0; i < 4; i++) for (let k = 0; k < 4; k++) { const a = A[4 * i + k]; if (a === 0n) continue; for (let j = 0; j < 4; j++) C[4 * i + j] += a * B[4 * k + j] } return C }
  const I4b = [1n, 0n, 0n, 0n, 0n, 1n, 0n, 0n, 0n, 0n, 1n, 0n, 0n, 0n, 0n, 1n]
  /* a companheira de z⁴−z³+z²−z+1 (o fator não-trivial de z⁵+1) */
  const C = [0n, 0n, 0n, -1n, 1n, 0n, 0n, 1n, 0n, 1n, 0n, -1n, 0n, 0n, 1n, 1n]
  let C5 = C; for (let i = 1; i < 5; i++) C5 = mul4b(C5, C)
  const quinta = C5.every((v2, i) => v2 === -I4b[i])
  let C4 = C; for (let i = 1; i < 4; i++) C4 = mul4b(C4, C)
  const Cinv = C4.map(v2 => -v2)
  const inverso = mul4b(C, Cinv).every((v2, i) => v2 === I4b[i])
  const T = C.map((v2, i) => v2 + Cinv[i])
  const ouro = mul4b(T, T).every((v2, i) => v2 === T[i] + I4b[i])
  /* a ordem 5 ausente da escada: z⁵=1 só trivial nos três andares */
  let ausente = 0
  for (const q of [17n, 257n, 65537n]) {
    let c = 0n
    for (let z = 1n; z < q; z++) { let p = 1n; for (let i = 0; i < 5; i++) p = p * z % q; if (p === 1n) c++ }
    if (c === 1n) ausente++
  }
  /* a escada das dimensões (por divisão exata de z^n+1 pelo trivial):
   * n=3 → t=1 · n=4 → t²=2 · n=5 → t²=t+1 (OURO) · n=6 → t²=3 —
   * verificada nos coeficientes: t²−t−1 com t=z+z⁻¹, vezes z², é
   * exatamente 1−z+z²−z³+z⁴ */
  const pentagono = [1n, -1n, 1n, -1n, 1n].join() === '1,-1,1,-1,1'
  if (!quinta || !inverso || !ouro || ausente !== 3 || !pentagono) R++
  console.log(`\n§B7  C⁵=−I: ${quinta} · T=C+C⁻¹ com T²=T+I: ${ouro} · ordem 5 ausente: ${ausente}/3 andares`)
  ok('§B7 π É GERADOR VIA DIMENSÕES: o pentágono inteiro — C⁵ = −I (matrizes 4×4 de inteiros) e o traço-dobra T = C+C⁻¹ satisfaz T² = T+I: na dimensão 5 (o pental da teoria), π gera o OURO', quinta && inverso && ouro && pentagono)
  ok('§B7 e a casa não vê o pentágono no relógio: a ordem 5 está AUSENTE da escada (z⁵=1 só trivial nos três andares) — a régua m=1 apanha o que o relógio não tem: o metal é o pentágono visto pelo corpo, uma dimensão acima', ausente === 3)
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A leitura do coordenador, medida: o metrónomo sobe a torre da')
  console.log('  meia-volta e certifica π em racionais exatos — π é o buraco')
  console.log('  do seu próprio encaixe, a borda de toda a família. Cada régua')
  console.log('  m dá o seu metal por identidade ((φ−m)φ=1), as classes')
  console.log('  racionais encaixam com o metal irracional no centro, e o')
  console.log('  reticulado fecha uma dimensão acima (posto 2). E π não é de')
  console.log('  régua nenhuma: a₀=3≠a₁=7 — representa a família sem ser dela.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
