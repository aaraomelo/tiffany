/* tests/residuo_universal.js — o teorema dos resíduos no corpo (ordem do
 * coordenador, eval 14/08: «incluir derivação do teorema dos resíduos»;
 * enquadramento do gerente: «o resíduo pode ser a versão local do que a
 * monodromia mede globalmente» — aqui isso deixa de ser analogia e vira
 * asserção com gume. Fila: Viviani → trial → encaixe → RESÍDUOS → Clifford).
 *
 * O contorno da casa é o relógio: a órbita μ_M = {ω^k} no anel, a volta
 * é z ↦ ωz, e o «2πi» é M — a volta completa conta o relógio inteiro.
 * O que a medida dá:
 *
 *   A VOLTA SÓ VÊ UM MODO: Σ_{z∈μ_M} z^j = M·[M|j]. Para o integrando
 *   f(z)·z isso seleciona c_{−1}: o RESÍDUO é a única coordenada que a
 *   soma de contorno lê — Σ f(z)·z = M·c_{−1}, exato.
 *
 *   E É O PONTO FIXO DA MONODROMIA: a volta age no espectro (dft da
 *   lib) por c_k ↦ ω^k·c_k; o modo invariante (ω^k=1) é exatamente o
 *   que carrega o resíduo. «Local» (o coeficiente no polo) e «global»
 *   (o invariante da volta) são o MESMO número — o enquadramento do
 *   gerente, medido.
 *
 *   O POLO SIMPLES FECHA POR DOIS CAMINHOS: o resultante do contorno é
 *   ∏_{z∈μ_M}(a−z) = a^M−1, e a derivada logarítmica dá
 *   Σ 1/(z−a) = −M·a^{M−1}/(a^M−1) — a soma direta no corpo confirma.
 *   Um polo EM CIMA do contorno (a^M=1) é detectado: o denominador
 *   zera no tick em que o relógio o toca.
 *
 *   A CONSERVAÇÃO É GLOBAL: os resíduos de 1/((z−a)(z−b)) somam zero
 *   (frações parciais inteiras no corpo), e a soma de contorno bate
 *   com a reconstrução por parciais — dois caminhos.
 *
 *   O RESÍDUO LOGARÍTMICO CONTA: g'/g tem por resíduos as
 *   MULTIPLICIDADES dos zeros, e a soma é o grau — o princípio do
 *   argumento como aritmética de polinómios no corpo. (É a mesma
 *   contagem da zeta dinâmica: derivada do log ↔ censo — a ponte fica
 *   apontada para zeta_universal.js, já medida.)
 *
 * §R0  a volta só vê o modo 0: Σ z^j = M·[M|j], dois caminhos
 * §R1  o resíduo é o que a soma lê: Σ f(z)·z = M·c_{−1} para f de
 *      banda limitada; gume: só mexer em c_{−1} move a soma
 * §R2  o resíduo é o ponto fixo da monodromia: a volta roda o espectro
 *      por ω^k (dft da lib, tick a tick) e o invariante É o resíduo
 * §R3  o polo simples por dois caminhos: soma direta == −M·a^{M−1}/(a^M−1)
 *      (derivada do log do resultante); e o polo NO contorno é visto
 * §R4  a conservação: res_a + res_b = 0 exato, e a soma de contorno
 *      fecha pela decomposição em parciais — dois caminhos
 * §R5  o princípio do argumento: g=(z−r)²(z−s) tem g'/g com resíduos
 *      2 e 1 (identidade polinomial em TODO o corpo amostrado) e a
 *      soma é o grau; gume: multiplicidade errada quebra a identidade
 */
'use strict'
const { anel, dft } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const A = anel(P)
const M = 16
const w = A.powm(3, (P - 1) / M)             /* o relógio: ω de ordem M */
const orb = []
for (let k = 0; k < M; k++) orb.push(A.powm(w, k))

/* §R0 — a volta só vê o modo 0 */
{
  let certos = 0
  for (let j = -20; j <= 20; j++) {
    let s = 0
    for (const z of orb) s = (s + (j >= 0 ? A.powm(z, j) : A.inv(A.powm(z, -j)))) % P
    const previsto = (j % M === 0) ? M : 0            /* soma geométrica: ω^j≠1 anula */
    if (s === previsto) certos++
  }
  console.log(`\n§R0  Σ_{z∈μ_${M}} z^j = M·[${M}|j] em ${certos}/41 valores de j ∈ [−20,20]`)
  ok('§R0 a volta completa só vê o modo 0: Σ z^j = M quando M|j e ZERO senão — o «2πi» da casa é M', certos === 41)
}

/* f de banda limitada: c_{−3..3} fixos (o dado; a asserção deriva dele) */
const cs = { '-3': 9, '-2': 4, '-1': 12, 0: 7, 1: 3, 2: 8, 3: 5 }
const avalia = (z, c) => {
  let s = 0
  for (const [k, ck] of Object.entries(c)) {
    const j = Number(k)
    s = (s + ck * (j >= 0 ? A.powm(z, j) : A.inv(A.powm(z, -j)))) % P
  }
  return s
}

/* §R1 — o resíduo é o que a soma lê */
{
  const soma = c => orb.reduce((s, z) => (s + avalia(z, c) * z) % P, 0)
  const S = soma(cs)
  const c2 = { ...cs, 2: cs[2] + 1 }                   /* mexe fora do resíduo */
  const cr = { ...cs, '-1': cs['-1'] + 1 }             /* mexe NO resíduo */
  console.log(`\n§R1  Σ f(z)·z = ${S} · M·c_{−1} = ${A.mod(M * cs['-1'])} · mexer c_2: ${soma(c2)} · mexer c_{−1}: ${soma(cr)}`)
  ok('§R1 a soma de contorno lê UMA coordenada: Σ f(z)·z = M·c_{−1} — o resíduo, exato no anel', S === A.mod(M * cs['-1']))
  ok('§R1 o gume: mexer em c_2 não move a soma; mexer em c_{−1} move exatamente M — só o resíduo pesa', soma(c2) === S && soma(cr) === A.mod(S + M))
}

/* §R2 — o resíduo é o ponto fixo da monodromia */
{
  const v = orb.map(z => A.mod(avalia(z, cs) * z))     /* o integrando, amostrado */
  const r = v.map((_, j) => v[(j + 1) % M])            /* a VOLTA: um tick, z ↦ ωz */
  const cv = dft(v, A, w), cr2 = dft(r, A, w)
  let roda = 0
  for (let k = 0; k < M; k++) if (cr2[k] === A.mod(cv[k] * A.powm(w, k))) roda++
  /* o invariante: ω^k=1 ⟺ k=0, e ĉ_0(v) = Σv = a soma de contorno = M·c_{−1} */
  const invariantes = cv.map((c, k) => (c !== 0 && A.powm(w, k) === 1) ? k : -1).filter(k => k >= 0)
  console.log(`\n§R2  a volta roda o espectro: ĉ_k ↦ ω^k·ĉ_k em ${roda}/${M} modos · modos invariantes: {${invariantes}} · ĉ_0 = ${cv[0]} = M·c_{−1} = ${A.mod(M * cs['-1'])}`)
  ok('§R2 a monodromia age no espectro por c_k ↦ ω^k c_k, tick a tick — medido pela dft da lib', roda === M)
  ok('§R2 o único modo que a volta NÃO move é o que carrega o resíduo: ĉ_0 = M·c_{−1} — local e global são o mesmo número', invariantes.join() === '0' && cv[0] === A.mod(M * cs['-1']))
}

/* §R3 — o polo simples por dois caminhos */
{
  const a = 5                                          /* fora do contorno: 5^M ≠ 1 */
  const foraDoContorno = A.powm(a, M) !== 1
  let direta = 0
  for (const z of orb) direta = (direta + A.inv(A.mod(z - a + P))) % P
  const aM = A.powm(a, M)
  const fechada = A.mod(P - M) * A.powm(a, M - 1) % P * A.inv(A.mod(aM - 1 + P)) % P
  /* e o resultante, também por dois caminhos: ∏(a−z) = a^M − 1 */
  let prod = 1
  for (const z of orb) prod = prod * A.mod(a - z + P) % P
  /* o polo EM CIMA do contorno: a' = ω³ — o relógio TOCA o polo no tick 3 */
  const a2 = orb[3]
  const toca = orb.some(z => A.mod(z - a2 + P) === 0)
  console.log(`\n§R3  a=${a} (a^M=${aM}≠1: ${foraDoContorno}) · Σ 1/(z−a) direta=${direta} · fechada=−M·a^{M−1}/(a^M−1)=${fechada} · ∏(a−z)=${prod} vs a^M−1=${A.mod(aM - 1 + P)} · polo em ω³ tocado: ${toca}`)
  ok('§R3 o resultante do contorno fecha: ∏_{z∈μ_M}(a−z) = a^M−1, medido no corpo', prod === A.mod(aM - 1 + P))
  ok('§R3 o polo simples por DOIS caminhos: a soma direta dos inversos == a derivada logarítmica do resultante', foraDoContorno && direta === fechada)
  ok('§R3 e o polo NO contorno não se esconde: com a=ω³ o denominador zera no tick 3 — o relógio toca o polo', toca)
}

/* §R4 — a conservação global: os resíduos somam zero */
{
  const a = 5, b = 11
  const resA = A.inv(A.mod(a - b + P)), resB = A.inv(A.mod(b - a + P))
  /* frações parciais como identidade em TODO o contorno + 14 pontos fora */
  const teste = [...orb]
  for (let x = 20; x < 34; x++) teste.push(x)
  let identidade = 0
  for (const z of teste) {
    const lhs = A.inv(A.mod(z - a + P) * A.mod(z - b + P) % P)
    const rhs = (resA * A.inv(A.mod(z - a + P)) + resB * A.inv(A.mod(z - b + P))) % P
    if (lhs === rhs) identidade++
  }
  /* a soma de contorno por dois caminhos: direta vs parciais + §R3 */
  let direta = 0
  for (const z of orb) direta = (direta + z * A.inv(A.mod(z - a + P) * A.mod(z - b + P) % P)) % P
  const somaPolo = t => { let s = 0; for (const z of orb) s = (s + z * A.inv(A.mod(z - t + P))) % P; return s }
  const porParciais = (resA * somaPolo(a) + resB * somaPolo(b)) % P
  console.log(`\n§R4  res_a=${resA}, res_b=${resB}, soma=${(resA + resB) % P} · parciais valem em ${identidade}/${teste.length} pontos · contorno direto=${direta} vs por parciais=${porParciais}`)
  ok('§R4 a CONSERVAÇÃO: res_a + res_b = 0 exato — o que um polo emite o outro absorve, o total fecha', (resA + resB) % P === 0)
  ok('§R4 as frações parciais são identidade no corpo (contorno + fora) e a soma de contorno fecha pelos DOIS caminhos', identidade === teste.length && direta === porParciais)
}

/* §R5 — o princípio do argumento: o resíduo logarítmico é a multiplicidade */
{
  const r = 5, s2 = 11                                 /* g = (z−r)²(z−s) — grau 3 */
  /* g' = 2(z−r)(z−s) + (z−r)²; a decomposição g'/g = A/(z−r) + B/(z−s)
   * com A, B CONSTANTES é a afirmação; a identidade polinomial
   * g'(z) = A·(z−r)(z−s) + B·(z−r)² tem de valer em TODO z amostrado */
  const Ares = 2, Bres = 1
  const gl = z => (2 * A.mod(z - r + P) * A.mod(z - s2 + P) + A.mod(z - r + P) * A.mod(z - r + P)) % P
  const teste = [...orb]
  for (let x = 20; x < 40; x++) teste.push(x)
  let identidade = 0, gumeQuebra = 0
  for (const z of teste) {
    const rhs = (Ares * A.mod(z - r + P) % P * A.mod(z - s2 + P) + Bres * A.mod(z - r + P) * A.mod(z - r + P)) % P
    if (gl(z) === rhs) identidade++
    const errado = (3 * A.mod(z - r + P) % P * A.mod(z - s2 + P) + Bres * A.mod(z - r + P) * A.mod(z - r + P)) % P
    if (gl(z) !== errado) gumeQuebra++
  }
  /* o gume «errado» coincide onde (z−r)(z−s)=0: z=r e z=s — se estiverem na amostra */
  const coincidencias = teste.filter(z => A.mod(z - r + P) === 0 || A.mod(z - s2 + P) === 0).length
  console.log(`\n§R5  g=(z−r)²(z−s): g'/g = 2/(z−r) + 1/(z−s) em ${identidade}/${teste.length} pontos · soma dos resíduos = ${Ares + Bres} = grau 3 · multiplicidade errada falha ${gumeQuebra}/${teste.length}`)
  ok('§R5 o resíduo logarítmico É a multiplicidade: g\'/g = 2/(z−r) + 1/(z−s), identidade em todo o corpo amostrado', identidade === teste.length)
  ok('§R5 e a soma dos resíduos é o GRAU: 2+1=3 — o princípio do argumento como aritmética; a multiplicidade errada (3) quebra em todos os pontos fora dos zeros', Ares + Bres === 3 && gumeQuebra === teste.length - coincidencias)
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  O teorema dos resíduos, derivado no corpo: a volta completa')
  console.log('  (o relógio μ_M) só vê um modo, e esse modo É o resíduo — o')
  console.log('  ponto fixo da monodromia no espectro. O polo simples fecha')
  console.log('  pela derivada do log do resultante, a conservação soma zero,')
  console.log('  e o resíduo logarítmico conta multiplicidades até ao grau.')
  console.log('  Local (coeficiente no polo) e global (invariante da volta)')
  console.log('  são o mesmo número: o enquadramento do gerente, medido.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
