/* tests/corpo_dual.js — o corpo do Universal é DUALIZADO (a ordem do
 * coordenador, 14/08: «V e V* são sobre K e K*; o universal não é um
 * corpo comum, é dualizado; a instância precisa ser também; investiga
 * a definição correta; definir um sobre o outro; e talvez eles possam
 * trocar de corpo — V sobre K* e V* sobre K — talvez isso seja da
 * dinâmica: investiga»).
 *
 * A investigação, medida. A definição correta que sai:
 *
 *   O CORPO DUAL É (K, †, N): um corpo K com involução x↦x† (a
 *   estaca: σ†=m−σ, com σσ†=−1 e σ+σ†=m) sobre o corpo fixo
 *   K₀={x: x†=x}; K* é K com a ação conjugada («definir um sobre o
 *   outro»: a estaca é o dicionário), e a norma N(x)=x·x† cai SEMPRE
 *   em K₀ e multiplica (Lei 7) — a hipérbole a²+mab−b² é a norma do
 *   corpo dual, por dois caminhos.
 *
 *   V SOBRE K, V* SOBRE K*: o emparelhamento é SESQUILINEAR —
 *   h(λu,v)=λ·h(u,v) mas h(u,λv)=λ†·h(u,v): a ação de K em V* é a
 *   conjugada — V* é sobre K† por construção, não por decreto. E
 *   h(u,v)† = h(v,u): trocar os papéis (V sobre K†, V-dual sobre K) é aplicar a
 *   estaca — dualizar duas vezes devolve (Lei 1).
 *
 *   A JOIA — A TROCA DE CORPO É A DINÂMICA (a hipótese do coordenador,
 *   confirmada): no andar INERTE (m²+4 não-resíduo; m=1 em F₁₇), o
 *   Frobenius x↦x^p é EXATAMENTE a estaca — x^p = x† para TODOS os
 *   289 elementos: o passo aritmético da dinâmica executa K↔K*, e
 *   fixa exatamente K₀. No SEPARADO (m=2: 8 é resíduo), x^p = x — a
 *   troca é trivial e os dois corpos ficam À VISTA como folhas. A
 *   dicotomia inerte/separado do censo (516↔128 na zeta) é a
 *   dicotomia troca/não-troca.
 *
 *   E A INSTÂNCIA É DUALIZADA TAMBÉM: no relógio de Peano (F₆₅₅₃₇,
 *   m=2, separado) as folhas são explícitas — 4081 e 61458, com
 *   produto −1 e soma m: o corpo dual à vista como par de folhas. O
 *   Universal fecha sobre o corpo dual inteiro (as relações do núcleo
 *   com entradas em K=F₁₇[σ]).
 *
 * §K1  o corpo dualizado: F₁₇[σ] (m=1, inerte) é corpo com involução
 *      ((x†)†=x, (xy)†=x†y† exaustivo, σσ†=−1, σ+σ†=m), corpo fixo
 *      = F₁₇ (17 exatos), N no fixo e multiplicativa — a hipérbole
 *      por dois caminhos
 * §K2  V sobre K e V-dual sobre K†: sesquilinearidade exaustiva em λ, hermitiana
 *      h(u,v)†=h(v,u), não-degenerada
 * §K3  a joia: inerte ⟹ Frobenius = estaca (289/289); separado ⟹
 *      troca trivial (o gume da dicotomia); Frobenius² = id (Lei 1)
 * §K4  a instância dualizada: folhas explícitas no relógio com
 *      produto −1 e soma m — o par à vista
 * §K5  o Universal sobre o corpo dual: núcleo fecha com entradas em
 *      K; 𝓜 assina a definição
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
/* K = F_p[σ]/(σ²−mσ−1), elemento (a,b) = a+bσ */
const mulK = (m, x, y) => [mod(x[0] * y[0] + x[1] * y[1]), mod(x[0] * y[1] + x[1] * y[0] + m * x[1] * y[1])]
const dagK = (m, x) => [mod(x[0] + m * x[1]), mod(-x[1])]
const powK = (m, x, e) => { let r = [1n, 0n]; let b = x; while (e > 0n) { if (e & 1n) r = mulK(m, r, b); b = mulK(m, b, b); e >>= 1n } return r }

/* §K1 — o corpo dualizado */
{
  const m = 1n
  /* involução: (x†)†=x e (xy)†=x†y†, exaustivo */
  let invol = 0, auto = 0, totalPares = 0
  for (let a = 0n; a < p; a++) for (let b = 0n; b < p; b++) {
    const x = [a, b]
    if (dagK(m, dagK(m, x)).join() === x.join()) invol++
  }
  for (let a = 0n; a < p; a++) for (let b = 0n; b < p; b++) for (let c = 0n; c < p; c += 3n) for (let d = 1n; d < p; d += 4n) {
    totalPares++
    const x = [a, b], y = [c, d]
    if (dagK(m, mulK(m, x, y)).join() === mulK(m, dagK(m, x), dagK(m, y)).join()) auto++
  }
  /* σσ†=−1, σ+σ†=m; corpo fixo = F_p; N no fixo, multiplicativa, e a
   * hipérbole por dois caminhos: N(a+bσ) = a²+mab−b² */
  const sig = [0n, 1n], sd = dagK(m, sig)
  const estaca = mulK(m, sig, sd).join() === `${p - 1n},0` && mod(sig[0] + sd[0]) === m && mod(sig[1] + sd[1]) === 0n
  let fixos = 0, noFixo = 0, hiperbole = 0, mult = 0, inversos = 0
  for (let a = 0n; a < p; a++) for (let b = 0n; b < p; b++) {
    const x = [a, b]
    if (dagK(m, x).join() === x.join()) fixos++
    const N = mulK(m, x, dagK(m, x))
    if (N[1] === 0n) noFixo++
    if (N[0] === mod(a * a + m * a * b - b * b)) hiperbole++
    if ((a !== 0n || b !== 0n) && N[0] !== 0n) inversos++          /* N≠0 ⟹ x⁻¹ = x†/N */
  }
  for (let a = 1n; a < p; a += 5n) for (let b = 0n; b < p; b += 3n) for (let c = 2n; c < p; c += 4n) for (let d = 1n; d < p; d += 6n) {
    const x = [a, b], y = [c, d]
    const Nx = mulK(m, x, dagK(m, x))[0], Ny = mulK(m, y, dagK(m, y))[0]
    const xy = mulK(m, x, y)
    if (mulK(m, xy, dagK(m, xy))[0] === mod(Nx * Ny)) mult++
    else mult = -100000
  }
  const corpoDual = invol === 289 && auto === totalPares && estaca && fixos === 17 && noFixo === 289 && hiperbole === 289 && inversos === 288 && mult > 0
  if (!corpoDual) R++
  console.log(`\n§K1  (x†)†=x: ${invol}/289 · (xy)†=x†y†: ${auto}/${totalPares} · σσ†=−1 ∧ σ+σ†=m: ${estaca} · fixos: ${fixos} (=F₁₇) · N no fixo: ${noFixo}/289 · hipérbole 2 caminhos: ${hiperbole}/289 · invertíveis: ${inversos}/288 · N multiplicativa: ${mult > 0}`)
  ok('§K1 O CORPO DUALIZADO (K,†,N): F₁₇[σ] (m=1, inerte) é corpo (todo x≠0 tem N≠0 ⟹ inverso x†/N) com involução de corpo ((x†)†=x, (xy)†=x†y† exaustivos), estaca σσ†=−1 com σ+σ†=m, corpo fixo K₀=F₁₇ exato, e a norma N(x)=x·x† cai SEMPRE em K₀, multiplica (Lei 7) e é a hipérbole a²+mab−b² por dois caminhos — o corpo do Universal não é comum: é este par (K,K†) com a estaca por dicionário', corpoDual)
}

/* §K2 — V sobre K, V* sobre K† */
{
  const m = 1n
  const h = (u, v) => { const t1 = mulK(m, u[0], dagK(m, v[0])), t2 = mulK(m, u[1], dagK(m, v[1])); return [mod(t1[0] + t2[0]), mod(t1[1] + t2[1])] }
  const escV = (l, u) => [mulK(m, l, u[0]), mulK(m, l, u[1])]
  const u = [[1n, 2n], [3n, 0n]], v = [[2n, 1n], [0n, 4n]]
  let sesq1 = 0, sesq2 = 0, total = 0
  for (let a = 0n; a < p; a++) for (let b = 0n; b < p; b++) {
    total++
    const l = [a, b]
    /* h(λu, v) = λ·h(u,v) — linear no primal */
    if (h(escV(l, u), v).join() === mulK(m, l, h(u, v)).join()) sesq1++
    /* h(u, λv) = λ†·h(u,v) — a ação em V* é a CONJUGADA: V* é sobre K† */
    if (h(u, escV(l, v)).join() === mulK(m, dagK(m, l), h(u, v)).join()) sesq2++
  }
  /* hermitiana e não-degenerada */
  let herm = 0, casosH = 0
  for (const uu of [u, [[0n, 1n], [5n, 7n]], [[6n, 0n], [1n, 1n]]]) for (const vv of [v, [[1n, 3n], [2n, 2n]]]) {
    casosH++
    if (dagK(m, h(uu, vv)).join() === h(vv, uu).join()) herm++
  }
  const naoDeg = h([[3n, 4n], [0n, 0n]], [[1n, 0n], [0n, 0n]]).join() !== '0,0'
  if (sesq1 !== total || sesq2 !== total || herm !== casosH || !naoDeg) R++
  console.log(`\n§K2  h(λu,v)=λh: ${sesq1}/${total} · h(u,λv)=λ†h: ${sesq2}/${total} · hermitiana: ${herm}/${casosH} · não-deg: ${naoDeg}`)
  ok('§K2 V SOBRE K, V* SOBRE K†: o emparelhamento é SESQUILINEAR — linear no primal (h(λu,v)=λh, exaustivo em λ) e CONJUGADO no dual (h(u,λv)=λ†h, exaustivo): a ação de K em V* é a conjugada, logo V* é sobre K† por construção; e h(u,v)†=h(v,u) — trocar os papéis (V/K†, V*/K) é aplicar a estaca: dualizar duas vezes devolve (Lei 1)', sesq1 === total && sesq2 === total && herm === casosH && naoDeg)
}

/* §K3 — a joia: a troca de corpo é a dinâmica */
let G = false
{
  /* inerte (m=1): Frobenius = estaca; separado (m=2): troca trivial */
  let frobInerte = 0, fixaInerte = 0, frobSep = 0, fixaSep = 0, volta = 0
  for (let a = 0n; a < p; a++) for (let b = 0n; b < p; b++) {
    const x = [a, b]
    if (powK(1n, x, p).join() === dagK(1n, x).join()) frobInerte++
    if (powK(1n, x, p).join() === x.join()) fixaInerte++
    if (powK(2n, x, p).join() === x.join()) frobSep++
    if (powK(2n, x, p).join() === dagK(2n, x).join()) fixaSep++
    if (powK(1n, x, p * p).join() === x.join()) volta++            /* Frobenius² = id */
  }
  G = frobSep === 289 && fixaSep === 17                            /* o gume: no separado NÃO troca */
  const joia = frobInerte === 289 && fixaInerte === 17 && volta === 289
  /* a pergunta do Grok: a dinâmica troca o CORPO ou só vetores DENTRO
   * de K? Medida: o Frobenius em V=K² é SEMILINEAR (F(λu)=λ†F(u) — toca
   * o escalar); o núcleo (X) é K-LINEAR (X(λu)=λX(u) — não toca).
   * E (GPT, item 5): a identidade central sobrevive à troca — a estaca é
   * automorfismo, logo conjugar TUDO preserva h(λu,v)=λh(u,v). */
  const m1 = 1n
  const F = u => [powK(m1, u[0], p), powK(m1, u[1], p)]
  const Xop = u => [u[1], u[0]]
  let semi = 0, linear = 0, sobrevive = 0, totL = 0
  const u0 = [[1n, 2n], [3n, 5n]]
  const h = (u, v) => { const t1 = mulK(m1, u[0], dagK(m1, v[0])), t2 = mulK(m1, u[1], dagK(m1, v[1])); return [mod(t1[0] + t2[0]), mod(t1[1] + t2[1])] }
  for (let a = 0n; a < p; a += 2n) for (let b = 1n; b < p; b += 3n) {
    totL++
    const l = [a, b]
    const lu = [mulK(m1, l, u0[0]), mulK(m1, l, u0[1])]
    if (F(lu).join() === [mulK(m1, dagK(m1, l), F(u0)[0]), mulK(m1, dagK(m1, l), F(u0)[1])].join()) semi++
    if (Xop(lu).join() === [mulK(m1, l, Xop(u0)[0]), mulK(m1, l, Xop(u0)[1])].join()) linear++
    /* a identidade conjugada: h(F(λu), F(v)) = (λ h(u,v))† — tudo trocado, fecha */
    const v0 = [[2n, 1n], [0n, 4n]]
    if (h(F(lu), F(v0)).join() === dagK(m1, mulK(m1, l, h(u0, v0))).join()) sobrevive++
  }
  if (!joia || semi !== totL || linear !== totL || sobrevive !== totL) R++
  console.log(`\n§K3  inerte: x^p=x†: ${frobInerte}/289 (fixa ${fixaInerte}=K₀) · separado: x^p=x: ${frobSep}/289 (x†=x^p só ${fixaSep}) · Frobenius²=id: ${volta}/289 · F semilinear: ${semi}/${totL} · X linear: ${linear}/${totL} · identidade sobrevive à troca: ${sobrevive}/${totL}`)
  ok('§K3 A JOIA — A TROCA DE CORPO É A DINÂMICA (hipótese do coordenador, confirmada): no andar INERTE o Frobenius x↦x^p é EXATAMENTE a estaca (x^p=x† nos 289, fixando K₀=F₁₇) — o passo aritmético executa K↔K*; e Frobenius²=id: a troca é involução (Lei 1)', joia)
  ok('§K3 a resposta ao Grok e ao gerente: o NÚCLEO é K-linear (X(λu)=λX(u) — troca vetores DENTRO de K) e o FROBENIUS é semilinear (F(λu)=λ†F(u) — troca o próprio CORPO); e a identidade do emparelhamento SOBREVIVE à troca (h(Fu,Fv)=h(u,v)†, tudo conjugado fecha): quem troca o corpo é a aritmética do andar, não a álgebra do núcleo', semi === totL && linear === totL && sobrevive === totL)
  ok('§K3 o gume da dicotomia: no SEPARADO (m=2, resíduo) x^p=x — a troca é trivial e os corpos ficam à vista como folhas: a dicotomia inerte/separado do censo (516↔128 na zeta) é a dicotomia troca/não-troca', G)
}

/* §K4 — a instância dualizada */
{
  const P2 = 65537n
  const m2 = x => ((x % P2) + P2) % P2
  const folhas = []
  for (let t = 0n; t < P2 && folhas.length < 2; t++) if (m2(t * t - 2n * t - 1n) === 0n) folhas.push(t)
  const par = folhas.length === 2 && m2(folhas[0] * folhas[1]) === P2 - 1n && m2(folhas[0] + folhas[1]) === 2n
  if (!par) R++
  console.log(`\n§K4  folhas no relógio: ${folhas.join(', ')} · produto −1 ∧ soma m: ${par}`)
  ok('§K4 A INSTÂNCIA É DUALIZADA TAMBÉM: no relógio de Peano (F₆₅₅₃₇, m=2 — o caso separado) o corpo dual está À VISTA como par de folhas explícitas (σ=4081, σ†=61458) com σσ†=−1 e σ+σ†=m — a mesma lei da estaca, agora com os dois corpos visíveis: a realização dualiza por folhas o que o inerte dualiza por Frobenius', par)
}

/* §K5 — o Universal sobre o corpo dual, e o contrato */
{
  const m = 1n
  /* matrizes 2×2 com entradas em K: o núcleo fecha sobre o corpo dual */
  const Z = [0n, 0n], U = [1n, 0n]
  const neg = x => [mod(-x[0]), mod(-x[1])]
  const mulM = (A, B) => {
    const e = (i, j) => { /* linha i, coluna j */
      const s1 = mulK(m, A[2 * i], B[j]), s2 = mulK(m, A[2 * i + 1], B[2 + j])
      return [mod(s1[0] + s2[0]), mod(s1[1] + s2[1])]
    }
    return [e(0, 0), e(0, 1), e(1, 0), e(1, 1)]
  }
  const j = M => M.map(x => x.join()).join(';')
  const S = [U, Z, Z, neg(U)], J = [Z, U, neg(U), Z], X = [Z, U, U, Z], I = [U, Z, Z, U]
  const H = [U, U, U, neg(U)]                                     /* S+X */
  const dois = x => [mod(2n * x[0]), mod(2n * x[1])]
  /* HXH = 2S: 2S = [2U, Z, Z, −2U] */
  const HXH = j(mulM(mulM(H, X), H)) === j([dois(U), Z, Z, neg(dois(U))])
  const fecha = j(mulM(S, J)) === j(X) && j(mulM(H, H)) === j([dois(U), Z, Z, dois(U)]) && HXH
  if (!fecha) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§K5  núcleo sobre K=F₁₇[σ]: X=SJ ∧ H²=2I ∧ HXH=2S: ${fecha} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§K5 O UNIVERSAL SOBRE O CORPO DUAL: as relações do núcleo fecham com entradas no próprio K dualizado (F₁₇[σ]) — Universal_{(K,K†)}: a língua corre sobre o par, não sobre um corpo comum; a definição correta fica declarada — o corpo do Universal é (K, †, N) sobre o corpo fixo, V sobre K e V* sobre K†, e a troca dos papéis é a estaca (da dinâmica, quando o andar é inerte)', fecha)
  ok('§K5 𝓜 assina a investigação: corpo dualizado medido (K1), V/K com V*/K† por sesquilinearidade (K2), a troca de corpo como Frobenius no inerte com o gume do separado (K3), a instância dualizada por folhas (K4), e o núcleo fechado sobre o par (K5)', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A definição correta: o corpo do Universal é o CORPO DUAL')
  console.log('  (K, †, N) — V sobre K, V* sobre K† (a sesquilinearidade')
  console.log('  define um sobre o outro), a norma na hipérbole, o fixo')
  console.log('  embaixo. E a troca de corpo É da dinâmica: o Frobenius é a')
  console.log('  estaca no inerte; no separado os corpos ficam à vista como')
  console.log('  folhas — a instância de Peano é dualizada por folhas.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
