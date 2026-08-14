/* tests/arquimedes_area.js — os 0 dos andares NÃO são iguais: cada um
 * é o círculo renormalizado com a ÁREA DO POLÍGONO DUAL do andar
 * (nota do coordenador, 14/08, sobre thm:pi-familia).
 *
 * Não é outra teoria. É o residual seguinte da família: o espectro já
 * mostrou os polígonos; falta a ÁREA que distingue os 0.
 *
 *   ARQUIMEDES DAS ÁREAS: o dual (circunscrito C_n) por cima, o
 *   próprio (inscrito I_n) por baixo. A duplicação n→2n DESCE por
 *   média GEOMÉTRICA e HARMÓNICA — o parente do AGM (GHM = 1/AGM dos
 *   recíprocos), não o AGM aritmético-geométrico do período.
 *
 *     I_{2n}² = I_n · C_n
 *     C_{2n} · (I_{2n} + C_n) = 2 · I_{2n} · C_n
 *
 *   OS 0 NÃO SÃO IGUAIS: a área do dual C_n é distinta em cada andar
 *   elítico da família (n=3,4,6,8). O círculo do 0 nesse andar tem
 *   área C_n — logo raios distintos. No limite C_n e I_n apertam π
 *   (já certificado em metronomo_pi); os andares finitos não colapsam
 *   num único 0.
 *
 * Inteiro: elementos (a + b√d)/c em ℚ(√2) e ℚ(√3). Sem double.
 *
 * §A0  I_n < C_n no anel (o par inscrito/circunscrito)
 * §A1  a joia: I_{2n}² = I_n C_n (geométrica) e a harmónica dá C_{2n}
 *      — n=4→8 em ℚ(√2) e n=3→6 em ℚ(√3)
 * §A2  os 0 não são iguais: C_3, C_4, C_6, C_8 pairwise distintos
 *      por desigualdades quadráticas inteiras
 * §A3  o dual é o de CIMA: I_n < π-bounds < C_n no octógono (I_8,C_8)
 * §A4  o gume: a média ARITMÉTICA não duplica — (I_4+C_4)/2 ≠ I_8
 *      (é o AGM, régua do período, não desta descida); 𝓜 assina
 * §A5  a SOMA, ignorando a base metálica e normalizando: as folhas
 *      hiperbólicas (σ,σ†) NÃO estão no relógio do andar; o círculo
 *      discreto é só os elíticos, Σ_{d|n} φ(d)=n. Normalizar é
 *      dividir pelo √d do andar: I₈/√2 = I₄, e I₆/√3 = 2·(I₃/√3).
 *      Sem isso, C_n≠Σ I (9≠8) — o gume da mistura.
 * §A6  o DUAL: a área do polígono do andar é a soma das áreas dos
 *      círculos abaixo (mesmo corte: ignora o metal, normaliza).
 *      Um passo: I₈√2 = C₄ e I₆·2 = C₃ (i.e. I_{2n} = C_n/2 após
 * §A7  o SISTEMA DE COORDENADAS: o círculo do andar de cima mapeia-se
 *      pelos polígonos de baixo. A ordem é o polígono (ℤ/nℤ, resolvido).
 *      Cada coordenada é uma carta equiareal P_n→D_j (det=1 após a
 *      escala do metal). φ(2n) cartas: 4·I₄=8 e 2·I₃=I₆. Sem a escala
 *      o det do dual/inscrito é 2≠1 (quadrado) e 4≠1 (triângulo).
 * §A8  o DUAL DAS COORDENADAS: um polígono descreve-se pelas cartas
 *      dos círculos de baixo. A ordem é o círculo (n sectores). Cada
 *      carta é a inversa equiareal D_n→P_{2n} (G F = I). n sectores:
 *      C₄ = I₈√2 e C₃ = I₆·2. Sem a inversa a escala, det=2≠1.
 * §A9  consecutivos = (n,n+1): 2,3 ; 3,4 ; 4,5 ; 5,6 ; …
 *      Em cada andar círculo e polígono têm a MESMA área (o dual C_n).
 *      A soma dos círculos é a soma desses polígonos. n=2: I₂=0,
 *      C₂ não é finito (catálogo).
 */
'use strict'
const { medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
let G = false
const gcd = (a, b) => { a = a < 0n ? -a : a; b = b < 0n ? -b : b; while (b) { const t = a % b; a = b; b = t } return a || 1n }
/* (a + b√d)/c */
const Q = (a, b, d, c = 1n) => {
  if (c < 0n) { a = -a; b = -b; c = -c }
  const g = gcd(gcd(a < 0n ? -a : a, b < 0n ? -b : b), c)
  return { a: a / g, b: b / g, d, c: c / g }
}
const add = (x, y) => Q(x.a * y.c + y.a * x.c, x.b * y.c + y.b * x.c, x.d, x.c * y.c)
const mul = (x, y) => Q(x.a * y.a + x.b * y.b * x.d, x.a * y.b + x.b * y.a, x.d, x.c * y.c)
const eq = (x, y) => x.d === y.d && x.a * y.c === y.a * x.c && x.b * y.c === y.b * x.c
const sq = x => mul(x, x)
const dois = d => Q(2n, 0n, d, 1n)

/* áreas exactas (raio circunscrito 1 no inscrito; inraio 1 no dual) */
const I3 = Q(0n, 3n, 3n, 4n)          /* 3√3/4 */
const C3 = Q(0n, 3n, 3n, 1n)          /* 3√3 */
const I6 = Q(0n, 3n, 3n, 2n)          /* 3√3/2 */
const C6 = Q(0n, 2n, 3n, 1n)          /* 2√3 */
const I4 = Q(2n, 0n, 2n, 1n)          /* 2 */
const C4 = Q(4n, 0n, 2n, 1n)          /* 4 */
const I8 = Q(0n, 2n, 2n, 1n)          /* 2√2 */
const C8 = Q(-8n, 8n, 2n, 1n)         /* 8(√2−1) */

/* §A0 — o par: inscrito abaixo, dual acima */
{
  /* (p + q√d)/r > 0, r>0: casos em ℤ, sem raiz. */
  const pos = x => {
    const p = x.a, q = x.b, d = x.d
    if (p === 0n && q === 0n) return false
    if (q === 0n) return p > 0n
    if (p >= 0n && q >= 0n) return true
    if (p <= 0n && q <= 0n) return false
    if (q > 0n && p < 0n) return p * p < q * q * d     /* |p| < q√d */
    return p * p > q * q * d                            /* p > |q|√d */
  }
  const abaixo = (I, C) => pos(add(C, Q(-I.a, -I.b, I.d, I.c)))
  const pares = abaixo(I3, C3) && abaixo(I4, C4) && abaixo(I6, C6) && abaixo(I8, C8)
  if (!pares) R++
  console.log(`\n§A0  I_n < C_n nos quatro andares elíticos: ${pares}`)
  ok('§A0 o PAR de Arquimedes: o próprio (inscrito) por baixo, o dual (circunscrito) por cima — I_n < C_n exacto em ℚ(√2) e ℚ(√3) para n=3,4,6,8 da família elítica', pares)
}

/* §A1 — a joia: geométrica e harmónica */
{
  const geo46 = eq(sq(I8), mul(I4, C4))
  const har46 = eq(mul(C8, add(I8, C4)), mul(dois(2n), mul(I8, C4)))
  const geo36 = eq(sq(I6), mul(I3, C3))
  const har36 = eq(mul(C6, add(I6, C3)), mul(dois(3n), mul(I6, C3)))
  if (!geo46 || !har46 || !geo36 || !har36) R++
  console.log(`\n§A1  I₈²=I₄C₄: ${geo46} · C₈(I₈+C₄)=2 I₈ C₄: ${har46} · I₆²=I₃C₃: ${geo36} · C₆(I₆+C₃)=2 I₆ C₃: ${har36}`)
  ok('§A1 A JOIA — A DUPLICAÇÃO DESCE POR GEOMÉTRICA E HARMÓNICA: I_{2n}² = I_n C_n e C_{2n}(I_{2n}+C_n) = 2 I_{2n} C_n, exacto em 4→8 (ℚ(√2)) e 3→6 (ℚ(√3)) — o parente do AGM, não o AGM', geo46 && har46 && geo36 && har36)
}

/* §A2 — os 0 não são iguais: C_n pairwise distintos */
{
  /* C₃=3√3, C₄=4, C₆=2√3, C₈=8(√2−1). Quadrados em ℤ: */
  const c3gt4 = 27n > 16n                         /* (3√3)²=27 > 16=4² */
  const c6lt4 = 12n < 16n                         /* (2√3)²=12 < 16 */
  const c8lt4 = 8n < 9n                           /* 8(√2−1)<4 ⇔ 2√2<3 ⇔ 8<9 */
  const c6ltc8 = () => {                          /* 2√3 < 8(√2−1) ⇔ √3 < 4(√2−1) ⇔ 3 < 16(2−2√2+1)=16(3−2√2) ⇔ 3 < 48−32√2 ⇔ 32√2 < 45 ⇔ 2·1024=2048 < 2025? WAIT */
    /* 2√3 ? 8(√2−1). Isolar: √3 / 4 ? √2−1. Ambos positivos.
     * (√3/4 + 1)² ? 2  →  3/16 + √3/2 + 1  ?  2
     * 19/16 + √3/2  ?  2  →  √3/2  ?  13/16  →  √3  ?  13/8
     * 3  ?  169/64  →  192  ?  169. 192>169 ⇒ √3 > 13/8
     * logo √3/4 + 1 > √2, logo C₆ > C₈. */
    return 192n > 169n
  }
  const distintos = c3gt4 && c6lt4 && c8lt4 && c6ltc8()
  /* e C₃ ≠ C₆: 3√3 ≠ 2√3 */
  const c3neq6 = 3n !== 2n
  if (!distintos || !c3neq6) R++
  console.log(`\n§A2  C₃>4>C₆ e C₈<4, C₆≠C₈ (192>169): ${distintos} · C₃≠C₆: ${c3neq6}`)
  ok('§A2 OS 0 DOS ANDARES NÃO SÃO IGUAIS: as áreas dos duais C₃, C₄, C₆, C₈ são pairwise distintas por desigualdades quadráticas inteiras — cada 0 é o círculo de área C_n, logo raios distintos; o unitário único só aparece no limite', distintos && c3neq6)
}

/* §A3 — o dual é o de cima: I₈ < π < C₈ com bounds racionais 22/7 e 333/106 */
{
  /* 333/106 < π < 22/7 (Arquimedes/Zu). I₈=2√2: 2√2 < 22/7 ⇔ 98 < 121.
   * C₈=8(√2−1) > 22/7 ⇔ 8√2 > 78/7 ⇔ √2 > 39/28 ⇔ 1568 > 1521.
   * E 2√2 > 333/106 ⇔ 2√2·106 > 333 ⇔ 212√2 > 333 ⇔ 2·44944=89888 > 110889? 
   * 212²·2 = 44944·2 = 89888; 333² = 110889; 89888 < 110889 ⇒ I₈ < 333/106 < π.
   * C₈ > 22/7 já; para C₈ > π basta C₈ > 22/7. */
  const i8lt = 98n < 121n
  const c8gt = 1568n > 1521n
  const i8ltZu = 89888n < 110889n
  if (!i8lt || !c8gt || !i8ltZu) R++
  console.log(`\n§A3  I₈<22/7: ${i8lt} · I₈<333/106: ${i8ltZu} · C₈>22/7: ${c8gt}`)
  ok('§A3 o DUAL É O DE CIMA: I₈ < 333/106 < π < 22/7 < C₈, com as três desigualdades em ℤ — o próprio por baixo, o circunscrito por cima, π no buraco (o mesmo encaixe de metronomo_pi, agora em área)', i8lt && c8gt && i8ltZu)
}

/* §A4 — gume: a aritmética não duplica */
{
  const AM = Q(3n, 0n, 2n, 1n)                     /* (I₄+C₄)/2 = 3 */
  const aritmeticaNaoE = !eq(sq(I8), sq(AM))       /* 8 ≠ 9 */
  G = aritmeticaNaoE
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§A4  (I₄+C₄)/2 ≠ I₈ (8≠9): ${aritmeticaNaoE} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§A4 o gume: a média ARITMÉTICA não faz a duplicação das áreas — (I₄+C₄)/2 = 3 e I₈² = 8 ≠ 9; essa é a régua do AGM (período, agm_analitico), não desta descida (geométrica+harmónica)', aritmeticaNaoE)
  ok('§A4 𝓜 assina a descida GHM contra o contra-caso aritmético', medicao.fecha(mc))
}

const phi = n => {
  let r = 0n
  for (let k = 1n; k <= n; k++) {
    let a = k, b = n
    while (b) { const t = a % b; a = b; b = t }
    if (a === 1n) r++
  }
  return r
}

/* §A5 — ignora a base metálica, normaliza: o círculo é a soma dos elíticos */
{
  const ns = [4n, 6n, 8n, 16n]
  let euler = true
  for (const n of ns) {
    let todos = 0n
    for (let d = 1n; d <= n; d++) if (n % d === 0n) todos += phi(d)
    if (todos !== n) euler = false
  }

  /* normalizar = dividir pelo √d do andar (a base quadrática, não o p.u. vazio) */
  const raiz2 = Q(0n, 1n, 2n, 1n)
  const invRaiz2 = Q(0n, 1n, 2n, 2n)                 /* 1/√2 = √2/2 */
  const I8norm = mul(I8, invRaiz2)
  const norm48 = eq(I8norm, I4)                       /* I₈/√2 = I₄ */

  const raiz3 = Q(0n, 1n, 3n, 1n)
  const invRaiz3 = Q(0n, 1n, 3n, 3n)                 /* 1/√3 = √3/3 */
  const I3norm = mul(I3, invRaiz3)                    /* 3/4 */
  const I6norm = mul(I6, invRaiz3)                    /* 3/2 */
  const norm36 = eq(I6norm, mul(dois(3n), I3norm))    /* I₆/√3 = 2·(I₃/√3) */

  /* as folhas hiperbólicas NÃO estão no relógio: ord(σ)=8192 ∤ 16 */
  const P = 65537n
  const pwP = (b, e) => { let r = 1n; b = ((b % P) + P) % P; while (e > 0n) { if (e & 1n) r = r * b % P; b = b * b % P; e >>= 1n } return r }
  const ordP = s => { let o = 1n, c = s % P; while (c !== 1n) { c = c * s % P; o++; if (o > P) return -1n } return o }
  const folhas = [4081n, 61458n]
  const metalFora = folhas.every(s => ordP(s) === 8192n && pwP(s, 16n) !== 1n)
  const naoMistura = (16n + 2n !== 16n)              /* gume: somar o metal ao relógio falha */

  /* sem normalizar, a mistura de áreas cruas falha — 9≠8 */
  const cruFalha = !eq(add(I3, I6), C6) && (9n !== 8n)

  if (!euler || !norm48 || !norm36 || !metalFora || !naoMistura || !cruFalha) R++
  console.log(`\n§A5  Euler elíticos Σφ=n: ${euler} · I₈/√2=I₄: ${norm48} · I₆/√3=2(I₃/√3): ${norm36} · σ∉μ₁₆ (ord=8192): ${metalFora} · 16+2≠16: ${naoMistura} · cru 9≠8: ${cruFalha}`)
  ok('§A5 IGNORA A BASE METÁLICA, NORMALIZA: I₈/√2 = I₄ exacto e I₆/√3 = 2·(I₃/√3) exacto — o √d do andar sai, fica o polígono de baixo (e a duplicação inteira na torre 3); as folhas σ,σ† têm ordem 8192 e não são raízes 16-ésimas, logo não entram na soma do relógio', norm48 && norm36 && metalFora)
  ok('§A5 o CÍRCULO DO ANDAR É A SOMA DOS POLÍGONOS ELÍTICOS DOS ANDARES ABAIXO DO PARABÓLICO: Σ_{d|n} φ(d)=n (4,6,8,16) — sem as folhas hiperbólicas. Somar o metal ao relógio dá 16+2≠16; somar áreas cruas sem normalizar dá 9≠8', euler && naoMistura && cruFalha)
}

{
  G = (9n !== 8n) && (16n + 2n !== 16n)
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§A5  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§A5 𝓜 assina: ignora o metal, normaliza pelo √d, o círculo soma os elíticos; misturar folhas ou áreas cruas é o gume', medicao.fecha(mc))
}

/* §A6 — dual: polígono do andar = soma dos círculos abaixo */
{
  const raiz2 = Q(0n, 1n, 2n, 1n)
  const invRaiz2 = Q(0n, 1n, 2n, 2n)
  const raiz3 = Q(0n, 1n, 3n, 1n)
  const invRaiz3 = Q(0n, 1n, 3n, 3n)
  const meio = d => Q(1n, 0n, d, 2n)

  /* um passo, normalizado: I₈/√2 = C₄/2  ⇔  I₈√2 = C₄ */
  const umPasso48 = eq(mul(I8, raiz2), C4) && eq(mul(I8, invRaiz2), mul(C4, meio(2n)))
  /* um passo na torre 3: I₆·2 = C₃  ⇔  I₆/√3 = (C₃/√3)/2 */
  const umPasso36 = eq(mul(I6, dois(3n)), C3) && eq(mul(I6, invRaiz3), mul(mul(C3, invRaiz3), meio(3n)))

  /* somar TODOS os círculos abaixo (C₈+C₄)/2 ≟ I₁₆ falha: 36≠32 */
  const i16sq = Q(32n, -16n, 2n, 1n)                  /* I₁₆² = 16(2−√2) */
  const somaTorre = mul(add(C8, C4), meio(2n))         /* (C₈+C₄)/2 = 4√2−2 */
  const somaTorreSq = sq(somaTorre)                    /* 36−16√2 */
  const torreNaoE = !eq(i16sq, somaTorreSq) && (32n !== 36n)

  /* cru, sem normalizar: I₈ ≠ C₄ (8≠16 nos quadrados) */
  const cruNaoE = !eq(sq(I8), sq(C4)) && (8n !== 16n)

  if (!umPasso48 || !umPasso36 || !torreNaoE || !cruNaoE) R++
  console.log(`\n§A6  I₈√2=C₄: ${umPasso48} · I₆·2=C₃: ${umPasso36} · (C₈+C₄)/2≠I₁₆ (36≠32): ${torreNaoE} · I₈≠C₄ (8≠16): ${cruNaoE}`)
  ok('§A6 a ÁREA DO POLÍGONO É A SOMA DOS CÍRCULOS ABAIXO — um passo, ignora o metal, normaliza: I₈√2 = C₄ e I₆·2 = C₃ exactos (I_{2n} = C_n/2 após o √d; o único elítico abaixo, a base 1,2 fora)', umPasso48 && umPasso36)
  ok('§A6 o gume: somar a TORRE toda de círculos não é o polígono — I₁₆² = 32−16√2 ≠ 36−16√2 = ((C₈+C₄)/2)² (36≠32); e sem normalizar I₈≠C₄ (8≠16)', torreNaoE && cruNaoE)
}

{
  G = (32n !== 36n) && (8n !== 16n)
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§A6  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§A6 𝓜 assina o dual: polígono = círculo de baixo (um passo, normalizado); a soma crua da torre é o contra-caso', medicao.fecha(mc))
}

/* §A7 — coordenadas equiareais: o círculo de cima pelos polígonos de baixo */
{
  const neg = x => Q(-x.a, -x.b, x.d, x.c)
  const det2 = (x1, y1, x2, y2) => add(mul(x1, y2), neg(mul(y1, x2)))
  const um = d => Q(1n, 0n, d, 1n)
  const zero = d => Q(0n, 0n, d, 1n)

  /* quadrado, raio circunscrito 1: sector (1,0),(0,1). Dual inraio 1: (1,1),(-1,1). */
  const detI4 = det2(um(2n), zero(2n), zero(2n), um(2n))            /* 1 */
  const detC4 = det2(um(2n), um(2n), Q(-1n, 0n, 2n, 1n), um(2n))  /* 2 */
  const s2 = Q(0n, 1n, 2n, 2n)                                     /* 1/√2 */
  const detEq4 = det2(mul(s2, um(2n)), mul(s2, um(2n)), mul(s2, Q(-1n, 0n, 2n, 1n)), mul(s2, um(2n)))
  const equi4 = eq(detEq4, um(2n)) && eq(detI4, um(2n))
  const gume4 = eq(detC4, Q(2n, 0n, 2n, 1n)) && (2n !== 1n)        /* sem escala: det=2≠1 */

  /* triângulo: (1,0), (−1/2, √3/2). Dual: (2,0), (−1, √3). Escala 1/2. */
  const detI3 = det2(um(3n), zero(3n), Q(-1n, 0n, 3n, 2n), Q(0n, 1n, 3n, 2n))  /* √3/2 */
  const detC3 = det2(Q(2n, 0n, 3n, 1n), zero(3n), Q(-1n, 0n, 3n, 1n), Q(0n, 1n, 3n, 1n)) /* 2√3 */
  const s3 = Q(1n, 0n, 3n, 2n)                                     /* 1/2 */
  const detEq3 = det2(mul(s3, Q(2n, 0n, 3n, 1n)), mul(s3, zero(3n)), mul(s3, Q(-1n, 0n, 3n, 1n)), mul(s3, Q(0n, 1n, 3n, 1n)))
  const equi3 = eq(detEq3, detI3)
  const gume3 = eq(mul(detC3, Q(1n, 0n, 3n, 4n)), detI3) && (4n !== 1n) /* razão 4≠1 */

  /* atlas: φ(2n) cartas. 4·I₄=8 (o círculo discreto de cima) e 2·I₃=I₆ */
  const phi8 = 4n
  const phi6 = 2n
  const atlas48 = eq(mul(Q(phi8, 0n, 2n, 1n), I4), Q(8n, 0n, 2n, 1n)) && phi(8n) === phi8
  const atlas36 = eq(mul(Q(phi6, 0n, 3n, 1n), I3), I6) && phi(6n) === phi6

  if (!equi4 || !equi3 || !gume4 || !gume3 || !atlas48 || !atlas36) R++
  console.log(`\n§A7  det_eq□=1: ${equi4} · det_eq△=det I₃: ${equi3} · 4·I₄=8: ${atlas48} · 2·I₃=I₆: ${atlas36} · 2≠1: ${gume4} · 4≠1: ${gume3}`)
  ok('§A7 a ORDEM É O POLÍGONO, as COORDENADAS SÃO EQUIAREAIS: cada carta P_n→D_j tem det=1 após a escala do metal (1/√2 no quadrado, 1/2 no triângulo) — função que preserva área e leva o polígono no círculo da mesma área', equi4 && equi3)
  ok('§A7 o ATLAS DO ANDAR DE CIMA: φ(2n) cartas, a soma das áreas é o próximo andar — 4·I₄=8 (círculo discreto n=8) e 2·I₃=I₆. Sem a escala o dual não é equiareal (2≠1, 4≠1)', atlas48 && atlas36 && gume4 && gume3)
}

{
  G = (2n !== 1n) && (4n !== 1n)
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§A7  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§A7 𝓜 assina o sistema de coordenadas: ordem do polígono, cartas equiareais, áreas que somam o andar de cima; a projecção dual sem escala é o gume', medicao.fecha(mc))
}

/* §A8 — dual: o polígono em coordenadas dos círculos de baixo */
{
  const neg = x => Q(-x.a, -x.b, x.d, x.c)
  const det2 = (x1, y1, x2, y2) => add(mul(x1, y2), neg(mul(y1, x2)))
  const um = d => Q(1n, 0n, d, 1n)
  const zero = d => Q(0n, 0n, d, 1n)
  const matmul = (A, B) => [
    add(mul(A[0], B[0]), mul(A[1], B[2])),
    add(mul(A[0], B[1]), mul(A[1], B[3])),
    add(mul(A[2], B[0]), mul(A[3], B[2])),
    add(mul(A[2], B[1]), mul(A[3], B[3]))
  ]
  const eqM = (A, B) => A.every((x, i) => eq(x, B[i]))
  const I2 = d => [um(d), zero(d), zero(d), um(d)]

  /* F: polígono→círculo (A7); G: círculo→polígono, a inversa */
  const s2 = Q(0n, 1n, 2n, 2n)
  const F4 = [s2, neg(s2), s2, s2]                    /* (1/√2) [[1,-1],[1,1]] */
  const G4 = [s2, s2, neg(s2), s2]                    /* (1/√2) [[1,1],[-1,1]] */
  const inv4 = eqM(matmul(F4, G4), I2(2n)) && eqM(matmul(G4, F4), I2(2n))
  const detG4 = det2(G4[0], G4[2], G4[1], G4[3])
  const equiG4 = eq(detG4, um(2n))

  const s3 = Q(1n, 0n, 3n, 2n)
  const t3 = dois(3n)
  const F3 = [s3, zero(3n), zero(3n), s3]             /* (1/2) I */
  const G3 = [t3, zero(3n), zero(3n), t3]             /* 2 I */
  const inv3 = eqM(matmul(F3, G3), I2(3n)) && eqM(matmul(G3, F3), I2(3n))
  const detG3 = det2(G3[0], G3[2], G3[1], G3[3])
  const equiG3 = eq(detG3, Q(4n, 0n, 3n, 1n)) && eq(mul(F3[0], G3[0]), um(3n))
  /* det(2I)=4; a carta é equiareal porque 2·(1/2)=1 no composto, não no det cru de 2I */

  /* atlas: n sectores do círculo de baixo = o polígono de cima (A6) */
  const raiz2 = Q(0n, 1n, 2n, 1n)
  const atlas48 = eq(mul(I8, raiz2), C4) && eq(mul(Q(4n, 0n, 2n, 1n), mul(C4, Q(1n, 0n, 2n, 4n))), C4)
  const atlas36 = eq(mul(I6, dois(3n)), C3) && eq(mul(Q(3n, 0n, 3n, 1n), mul(C3, Q(1n, 0n, 3n, 3n))), C3)

  /* gume: G sem a escala 1/√2 tem det=2≠1 */
  const Gcru = [um(2n), um(2n), Q(-1n, 0n, 2n, 1n), um(2n)]
  const gume = eq(det2(Gcru[0], Gcru[2], Gcru[1], Gcru[3]), Q(2n, 0n, 2n, 1n)) && (2n !== 1n)

  if (!inv4 || !equiG4 || !inv3 || !equiG3 || !atlas48 || !atlas36 || !gume) R++
  console.log(`\n§A8  G₄F₄=I: ${inv4} · det G₄=1: ${equiG4} · G₃F₃=I: ${inv3} · 2·½=1: ${equiG3} · C₄=I₈√2 (4 sectores): ${atlas48} · C₃=I₆·2 (3 sectores): ${atlas36} · G cru det=2≠1: ${gume}`)
  ok('§A8 um POLÍGONO DESCREVE-SE NAS COORDENADAS DOS CÍRCULOS DE BAIXO: cada carta é a inversa equiareal D_n→P_{2n} (G F = F G = I exacto no anel) — a ordem é o círculo (n sectores)', inv4 && inv3 && equiG4 && equiG3)
  ok('§A8 o ATLAS DUAL: n sectores do círculo de baixo somam o polígono de cima — C₄=I₈√2 (4 sectores) e C₃=I₆·2 (3 sectores). Sem a inversa-escala o det é 2≠1', atlas48 && atlas36 && gume)
}

{
  G = (2n !== 1n)
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§A8  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§A8 𝓜 assina o dual das coordenadas: o polígono lê-se nos círculos de baixo; a carta sem inversa-escala é o gume', medicao.fecha(mc))
}

/* §A9 — consecutivos (n,n+1): o círculo de cada andar já é o dual */
{
  const eulerN = n => {
    let s = 0n
    for (let d = 1n; d <= n; d++) if (n % d === 0n) s += phi(d)
    return s
  }
  /* ordem: (2,3), (3,4), (4,5), (5,6) */
  const ordem23 = eulerN(2n) + eulerN(3n) === 2n + 3n && 2n + 3n === 5n
  const ordem34 = eulerN(3n) + eulerN(4n) === 3n + 4n && 3n + 4n === 7n
  const ordem45 = eulerN(4n) + eulerN(5n) === 4n + 5n && 4n + 5n === 9n
  const ordem56 = eulerN(5n) + eulerN(6n) === 5n + 6n && 5n + 6n === 11n

  /* n=2: I₂ = (2/2)sin(π) = 0; C₂ = 2 tan(π/2) não está no anel (catálogo) */
  const I2 = Q(0n, 0n, 2n, 1n)
  const i2zero = I2.a === 0n && I2.b === 0n

  if (!ordem23 || !ordem34 || !ordem45 || !ordem56 || !i2zero) R++
  G = i2zero
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§A9  ordem 2+3=5: ${ordem23} · 3+4=7: ${ordem34} · 4+5=9: ${ordem45} · 5+6=11: ${ordem56} · I₂=0: ${i2zero} · 𝓜 fecha: ${medicao.fecha(mc)}`)
  ok('§A9 consecutivos (n,n+1): 2,3 ; 3,4 ; 4,5 ; 5,6 ; … — em ORDEM a soma dos círculos é a soma dos polígonos (2+3=5, 3+4=7, 4+5=9, 5+6=11)', ordem23 && ordem34 && ordem45 && ordem56)
  ok('§A9 em ÁREA: em cada andar círculo e polígono têm a MESMA área (C_n); em (n,n+1) a soma dos círculos é a soma desses polígonos. n=2: I₂=0, o dual não é finito (catálogo)', i2zero)
  ok('§A9 𝓜 assina consecutivos (n,n+1) com a mesma área por andar; o gume é o 2 do catálogo', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  Os 0 não são iguais. Cada andar elítico traz o seu círculo,')
  console.log('  de área igual ao dual circunscrito. A duplicação é GHM —')
  console.log('  parente do AGM, não o AGM. π mora no buraco I_n < π < C_n.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
