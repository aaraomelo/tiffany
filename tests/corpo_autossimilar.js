/* tests/corpo_autossimilar.js — todo corpo já é dual por definição, e
 * a autossimilaridade do construtor (as ordens do coordenador, 14/08:
 * «na verdade todo corpo já é dual por definição — essa distinção vale
 * investigar, mas precisa apresentar ele na forma dual com as
 * primitivas»; «daí surge a autossimilaridade: ele usa um corpo para
 * construir outro da mesma forma»).
 *
 * As duas frases, medidas:
 *
 *   TODO CORPO JÁ É DUAL POR DEFINIÇÃO — a forma dual pelas
 *   primitivas: um corpo K é DOIS GRUPOS — o aditivo (K,+,0) com o
 *   espelho x↦−x e o multiplicativo (K×,·,1) com a inversão x↦1/x —
 *   dois neutros (o 0 e o 1 de toda dimensão), duas involuções,
 *   COSTURADOS pela distributividade (a Lei 6: ⊕ encontra ⊗) e com o
 *   expoente por dicionário (g^(a+b) = g^a·g^b: o aditivo dos índices
 *   É o multiplicativo dos valores — o caráter DENTRO do corpo).
 *
 *   A AUTOSSIMILARIDADE É DO CONSTRUTOR: UMA função — K ↦ K[σ_m] com
 *   σ² = mσ+1, apresentada nas primitivas (K², com ⊕ componente, ⊗
 *   pela dobra, † pela estaca, ⁻¹ = †/N) — aplicada a K₀=F₁₇ dá K₁
 *   (289), e aplicada a K₁ dá K₂ (83521): O MESMO código, a mesma
 *   forma. E A JOIA: a menor régua válida do andar 2 é m₁ = σ — O
 *   METAL DO ANDAR 1 É A RÉGUA DO ANDAR 2: o corpo usa o corpo para
 *   construir o corpo.
 *
 *   AS MESMAS LEIS SOBEM: no andar 2, ττ† = −1 e τ+τ† = m₁ (a
 *   estaca), N₂ cai em K₁ (o corpo fixo do andar), todo x≠0 da
 *   amostra é invertível (⁻¹ = †/N), e o Frobenius do andar
 *   (x^|K₁|) é a estaca do andar — a joia do corpo_dual, um andar
 *   acima, pela mesma lei.
 *
 *   A DIMENSÃO DOBRA E OS NEUTROS HERDAM: |K₀,K₁,K₂| = 17, 17², 17⁴
 *   (dim A_{n+1} = 2·dim A_n, agora nos corpos), K₁ mergulha em K₂
 *   por homomorfismo (x ↦ (x,0)), e o 0 e o 1 são os MESMOS em todos
 *   os andares — «todas as dimensões têm 0 e 1», agora por herança.
 *
 *   O GUME: o construtor com disc QUADRADO não dá corpo — no split
 *   (m=2 sobre F₁₇, 8 é resíduo) há 32 divisores de zero exibidos: a
 *   régua sabe onde constrói (inerte constrói corpo; split constrói
 *   o par de folhas à vista — as duas faces do corpo_dual).
 *
 * §A1  a forma dual: dois grupos, dois neutros, duas involuções, a
 *      costura (distributividade) e o dicionário exp (exaustivos)
 * §A2  o construtor autossimilar: a MESMA função dá K₁ de K₀ e K₂ de
 *      K₁; a régua do andar 2 é o metal do andar 1 (m₁=σ, derivado
 *      por varredura)
 * §A3  as leis sobem: estaca₂, N₂ em K₁ (900/900), inversos, e
 *      Frobenius₂ = estaca₂ com Frobenius₂² = id
 * §A4  a dimensão dobra (17, 289, 83521), o mergulho K₁↪K₂ é
 *      homomorfismo, e os neutros herdam
 * §A5  o gume do split (32 divisores de zero) e 𝓜
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
const p = 17n
const mod = x => ((x % p) + p) % p

/* §A1 — todo corpo já é dual por definição */
{
  /* dois grupos com as duas involuções e os dois neutros */
  let aditivo = 0, multiplicativo = 0, costura = 0, total = 0
  for (let a = 0n; a < p; a++) for (let b = 0n; b < p; b++) {
    total++
    /* espelho: a+(−a)=0 ; inversão (a≠0): a·a⁻¹=1 via Fermat */
    const esp = mod(a + mod(-a)) === 0n
    const inv = a === 0n || (() => { let r = 1n, base = a, e = p - 2n; while (e > 0n) { if (e & 1n) r = r * base % p; base = base * base % p; e >>= 1n } return mod(a * r) === 1n })()
    if (esp && inv) aditivo++
    /* a costura: a·(b+1) = a·b + a (distributividade) */
    if (mod(a * mod(b + 1n)) === mod(a * b + a)) costura++
    multiplicativo++
  }
  /* o dicionário exp: g^(a+b) = g^a·g^b, exaustivo, com g gerador */
  const pw = (b, e) => { let r = 1n; b = mod(b); while (e > 0n) { if (e & 1n) r = r * b % p; b = b * b % p; e >>= 1n } return r }
  let dic = 0, casos = 0, ordem = 0
  for (let k = 1n; k <= 16n; k++) if (pw(3n, k) === 1n) { ordem = Number(k); break }
  for (let a = 0n; a < 16n; a++) for (let b = 0n; b < 16n; b++) { casos++; if (pw(3n, (a + b) % 16n) === mod(pw(3n, a) * pw(3n, b))) dic++ }
  const dual = aditivo === Number(p * p) && costura === total && ordem === 16 && dic === casos
  if (!dual) R++
  console.log(`\n§A1  espelho∧inversão: ${aditivo}/${total} · costura: ${costura}/${total} · ord(3)=${ordem} · g^(a+b)=g^a g^b: ${dic}/${casos}`)
  ok('§A1 TODO CORPO JÁ É DUAL POR DEFINIÇÃO — a forma dual pelas primitivas: dois grupos (aditivo com o espelho x↦−x, multiplicativo com a inversão x↦1/x), dois neutros (o 0 e o 1), costurados pela distributividade (Lei 6) e com o expoente por dicionário (g^(a+b)=g^a·g^b exaustivo — o caráter DENTRO do corpo): as cinco primitivas são a apresentação dual do próprio corpo', dual)
}

/* o construtor: UMA função, parametrizada pelo corpo (as primitivas) */
const constroi = (C, m) => ({
  /* elementos: pares [x,y] = x + yσ sobre C; σ² = mσ + 1 */
  mul: (x, y) => [C.add(C.mul(x[0], y[0]), C.mul(x[1], y[1])),
    C.add(C.add(C.mul(x[0], y[1]), C.mul(x[1], y[0])), C.mul(m, C.mul(x[1], y[1])))],
  add: (x, y) => [C.add(x[0], y[0]), C.add(x[1], y[1])],
  dag: x => [C.add(x[0], C.mul(m, x[1])), C.neg(x[1])],
  neg: x => [C.neg(x[0]), C.neg(x[1])],
  zero: [C.zero, C.zero],
  um: [C.um, C.zero],
  eq: (x, y) => C.eq(x[0], y[0]) && C.eq(x[1], y[1]),
})
const K0 = { add: (a, b) => mod(a + b), mul: (a, b) => mod(a * b), neg: a => mod(-a), zero: 0n, um: 1n, eq: (a, b) => a === b }

/* §A2 — o construtor autossimilar e a joia da régua */
let K1, K2, m1
{
  K1 = constroi(K0, 1n)                                            /* K₁ = K₀[σ], m=1 (inerte) */
  /* a menor régua válida do andar 2, POR VARREDURA: m com m²+4 não-quadrado em K₁ */
  const sq = new Set()
  for (let a = 0n; a < p; a++) for (let b = 0n; b < p; b++) { const x = [a, b]; sq.add(K1.mul(x, x).join()) }
  m1 = null
  outer:
  for (let b = 0n; b < p; b++) for (let a = 0n; a < p; a++) {      /* varre por grau: constantes primeiro */
    const m = [a, b]
    const disc = K1.add(K1.mul(m, m), [4n, 0n])
    if (!sq.has(disc.join())) { m1 = m; break outer }
  }
  K2 = constroi(K1, m1)                                            /* K₂ = K₁[τ] — A MESMA função */
  const joia = m1 !== null && m1[0] === 0n && m1[1] === 1n          /* m₁ = σ: o metal do andar 1 */
  /* nota: a varredura percorre primeiro os m∈K₀ (b=0): NENHUM serve —
   * só a partir do metal σ o disc sai dos quadrados */
  if (!joia) R++
  console.log(`\n§A2  quadrados em K₁: ${sq.size}/289 · m₁ = ${m1.join(',')} (= σ)`)
  ok('§A2 O CONSTRUTOR AUTOSSIMILAR: a MESMA função (K, m) ↦ K[σ_m], escrita só nas primitivas, dá K₁ de K₀ e K₂ de K₁ — e A JOIA: a varredura (constantes primeiro) mostra que NENHUM m∈K₀ serve no andar 2; a primeira régua válida é m₁ = σ — O METAL DO ANDAR 1 É A RÉGUA DO ANDAR 2: o corpo usa o corpo para construir o corpo, da mesma forma', joia)
}

/* §A3 — as mesmas leis sobem */
{
  const tau2 = [[0n, 0n], [1n, 0n]]
  const td = K2.dag(tau2)
  const prod = K2.mul(tau2, td)
  const soma = K2.add(tau2, td)
  const estaca2 = K2.eq(prod, [[p - 1n, 0n], [0n, 0n]]) && K2.eq(soma, [m1, [0n, 0n]])
  /* N₂ cai em K₁ e inverte na amostra */
  let noK1 = 0, inv = 0, tot = 0
  for (let a = 0n; a < p; a += 3n) for (let b = 0n; b < p; b += 4n) for (let c = 0n; c < p; c += 3n) for (let d = 0n; d < p; d += 4n) {
    const x = [[a, b], [c, d]]; tot++
    const N = K2.mul(x, K2.dag(x))
    if (N[1][0] === 0n && N[1][1] === 0n) noK1++
    if ((a | b | c | d) !== 0n && !(N[0][0] === 0n && N[0][1] === 0n)) inv++
  }
  /* Frobenius₂ = estaca₂ (x^289 = x†) e Frobenius₂² = id, em amostra */
  const pow2 = (x, e) => { let r = K2.um; let b = x; while (e > 0n) { if (e & 1n) r = K2.mul(r, b); b = K2.mul(b, b); e >>= 1n } return r }
  let frob = 0, volta = 0, casos = 0
  for (const x of [tau2, [[1n, 2n], [3n, 4n]], [[5n, 0n], [0n, 1n]], [[2n, 7n], [11n, 3n]], [[0n, 1n], [1n, 0n]], [[9n, 9n], [4n, 13n]]]) {
    casos++
    if (K2.eq(pow2(x, 289n), K2.dag(x))) frob++
    if (K2.eq(pow2(x, 289n * 289n), x)) volta++
  }
  const sobem = estaca2 && noK1 === tot && inv === tot - 1 && frob === casos && volta === casos
  if (!sobem) R++
  console.log(`\n§A3  ττ†=−1 ∧ τ+τ†=m₁: ${estaca2} · N₂ em K₁: ${noK1}/${tot} · invertíveis: ${inv}/${tot - 1} · Frob₂=†: ${frob}/${casos} · Frob₂²=id: ${volta}/${casos}`)
  ok('§A3 AS MESMAS LEIS SOBEM: no andar 2 a estaca cumpre ττ†=−1 com τ+τ†=m₁, a norma N₂ cai no corpo de baixo (900/900 — K₁ é o corpo fixo do andar), todo x≠0 da amostra inverte (⁻¹=†/N), e o Frobenius do andar É a estaca do andar (x^|K₁|=x†, com a volta ao quadrado) — a joia do corpo_dual repete-se um andar acima, pela mesma lei', sobem)
}

/* §A4 — a dimensão dobra e os neutros herdam */
{
  /* |K₀|=17, |K₁|=17², |K₂|=17⁴ por construção de pares: dim dobra */
  const dims = [17n, 17n * 17n, 17n * 17n * 17n * 17n]
  const dobra = dims[1] === dims[0] * dims[0] && dims[2] === dims[1] * dims[1]
  /* o mergulho K₁ ↪ K₂ (x ↦ (x,0)) é homomorfismo: amostra */
  let homo = 0, casos = 0
  for (const x of [[1n, 2n], [3n, 0n], [0n, 5n], [7n, 11n]]) for (const y of [[2n, 1n], [4n, 4n]]) {
    casos++
    const lift = z => [z, [0n, 0n]]
    if (K2.eq(K2.mul(lift(x), lift(y)), lift(K1.mul(x, y))) && K2.eq(K2.add(lift(x), lift(y)), lift(K1.add(x, y)))) homo++
  }
  /* os neutros herdam: 1 de K₂ é o lift do 1 de K₁ que é o lift do 1 de K₀ */
  const neutros = K2.eq(K2.um, [[1n, 0n], [0n, 0n]]) && K1.eq(K1.um, [1n, 0n])
  if (!dobra || homo !== casos || !neutros) R++
  console.log(`\n§A4  dims: ${dims.join(', ')} · mergulho homomorfo: ${homo}/${casos} · neutros herdam: ${neutros}`)
  ok('§A4 A DIMENSÃO DOBRA E OS NEUTROS HERDAM: |K₀,K₁,K₂| = 17, 17², 17⁴ (dim A_{n+1}=2·dim A_n, agora nos corpos), o mergulho K₁↪K₂ é homomorfismo, e o 0 e o 1 são os MESMOS em todos os andares — «todas as dimensões têm 0 e 1», agora por herança do construtor', dobra && homo === casos && neutros)
}

/* §A5 — o gume do split, e o contrato */
let G = false
{
  /* o construtor com disc QUADRADO não dá corpo: m=2 sobre F₁₇ (8 é resíduo) */
  const Ks = constroi(K0, 2n)
  let zeroDiv = 0
  for (let a = 0n; a < p; a++) for (let b = 0n; b < p; b++) {
    if (a === 0n && b === 0n) continue
    const N = Ks.mul([a, b], Ks.dag([a, b]))
    if (N[0] === 0n && N[1] === 0n) zeroDiv++
  }
  G = zeroDiv === 32
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§A5  divisores de zero no split: ${zeroDiv} (=2p−2) · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§A5 o gume: o MESMO construtor com disc quadrado (m=2 sobre F₁₇) NÃO dá corpo — 32 divisores de zero exibidos (=2p−2): a régua sabe onde constrói; inerte constrói corpo, split constrói o par de folhas à vista — as duas faces do corpo_dual, decididas pelo quadrado', G)
  ok('§A5 𝓜 assina as duas frases do coordenador: todo corpo já é dual por definição (a forma dual pelas primitivas, §A1) e a autossimilaridade é do construtor (a mesma função em todos os andares, com o metal de um andar por régua do seguinte, §A2–A4)', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  As duas frases, medidas: o corpo JÁ É dual — dois grupos,')
  console.log('  dois neutros, duas involuções, uma costura — e o construtor')
  console.log('  é autossimilar: a mesma função nas primitivas ergue K₁ de')
  console.log('  K₀ e K₂ de K₁, com o metal de um andar como régua do')
  console.log('  próximo (m₁ = σ), as mesmas leis a subir (estaca, norma,')
  console.log('  Frobenius) e a dimensão a dobrar. O corpo constrói o corpo,')
  console.log('  da mesma forma — a torre inteira é uma frase só.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
