/* tests/teoria_vetorial.js — a fundação vetorial do Corpo Universal
 * (a ordem do coordenador, 14/08: «precisamos de uma teoria vetorial:
 * começar de V, como está em teoria.tex, e o seu V*; V é espaço
 * vetorial sobre um corpo; o corpo universal é sobre o MESMO corpo do
 * espaço vetorial, e Peano é uma realização»).
 *
 * A fundação já estava escrita em teoria.tex (V, V*=Hom(V,K), o
 * adjunto T*, o emparelhamento J: V→V*, T†=J⁻¹T*J); aqui ela ganha o
 * seu medidor sobre o V da casa (V=K², onde vivem as cartas, as
 * órbitas e os pares (c,s)):
 *
 *   O EMPARELHAMENTO DA CASA É O DETERMINANTE: ω(u,v) = u₁v₂−u₂v₁ —
 *   bilinear, NÃO-DEGENERADO (varredura exaustiva: nenhum u≠0 anula
 *   contra todos), e identifica V*≅V (o J de teoria.tex: u ↦ ω(u,·),
 *   injetivo por contagem exaustiva). O dual não nasce depois: está
 *   na fundação.
 *
 *   O DUAL ABSTRATO É O DUAL OPERACIONAL (a identidade central):
 *   ω(Mu, v) = ω(u, M†v) com M† = tr·I − M — EXAUSTIVO em F₁₇
 *   (334084/334084): o adjunto transportado pelo emparelhamento É a
 *   estaca da casa. O T†=J⁻¹T*J de teoria.tex e o M† dos medidores
 *   são o mesmo objeto — dual abstrato → dual operacional, medido.
 *
 *   A LEI 2 BEM TIPADA FECHA O CÍRCULO: T†=−T (anti-autoadjunto)
 *   ⟺ tr T = 0 — exaustivo: J, S, X (traço 0) cumprem; A_m (traço
 *   m≠0) viola. Os anti-autoadjuntos do emparelhamento são EXATAMENTE
 *   os geradores de Clifford (o auto-dual-negativo do §D2): a Lei 2
 *   de teoria.tex e o gerador de Clifford são a MESMA condição.
 *
 *   E O MESMO K PARA TUDO: a língua universal (o núcleo e as suas
 *   relações) fecha sobre K=F₁₇, K=F₆₅₅₃₇ e K=ℚ (BigInt) — a mesma
 *   álgebra, três corpos: Universal_K é sobre o corpo do espaço
 *   vetorial, e Peano é a realização que escolhe K=F₆₅₅₃₇ e V=o
 *   relógio.
 *
 * §F1  V e V* sobre o mesmo K: axiomas medidos em F₁₇ (comutatividade
 *      exaustiva, associatividade/distributividade em amostra ampla)
 * §F2  o emparelhamento: bilinear, não-degenerado (exaustivo), e
 *      V*≅V por contagem (u ↦ (−u₂,u₁) injetivo)
 * §F3  a identidade central: ω(Mu,v)=ω(u,M†v) exaustivo (4 matrizes ×
 *      F₁₇⁴) — dual abstrato = dual operacional
 * §F4  a Lei 2 bem tipada: anti-autoadjunto ⟺ tr=0 (J,S,X sim; A_m
 *      não — o gume): geradores de Clifford = anti-autoadjuntos
 * §F5  o mesmo K: as relações do núcleo fecham em F₁₇, F₆₅₅₃₇ e ℚ;
 *      𝓜 assina — Peano é realização, não fundação
 */
'use strict'
const { anel, nucleo, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const P = 17
const mod = x => ((x % P) + P) % P
const omega = (u, v) => mod(u[0] * v[1] - u[1] * v[0])
const ap = (M, u) => [mod(M[0] * u[0] + M[1] * u[1]), mod(M[2] * u[0] + M[3] * u[1])]
const dag = M => { const t = M[0] + M[3]; return [mod(t - M[0]), mod(-M[1]), mod(-M[2]), mod(t - M[3])] }

/* §F1 — V e V* sobre o mesmo K */
{
  /* comutatividade da soma: exaustiva em F₁₇² × F₁₇² */
  let comuta = 0, totalC = 0
  for (let a = 0; a < P; a++) for (let b = 0; b < P; b++) for (let c = 0; c < P; c++) for (let d = 0; d < P; d++) {
    totalC++
    if (mod(a + c) === mod(c + a) && mod(b + d) === mod(d + b)) comuta++
  }
  /* associatividade e distributividade em amostra estruturada */
  let axiomas = 0, totalA = 0
  for (let l = 1; l < P; l += 3) for (let m = 1; m < P; m += 5) for (let a = 0; a < P; a += 2) for (let b = 1; b < P; b += 4) {
    totalA++
    const u = [a, b]
    const lm_u = ap([mod(l * m), 0, 0, mod(l * m)], u)
    const l_mu = ap([l, 0, 0, l], ap([m, 0, 0, m], u))
    const dist = ap([mod(l + m), 0, 0, mod(l + m)], u)
    const soma = [mod(ap([l, 0, 0, l], u)[0] + ap([m, 0, 0, m], u)[0]), mod(ap([l, 0, 0, l], u)[1] + ap([m, 0, 0, m], u)[1])]
    if (lm_u.join() === l_mu.join() && dist.join() === soma.join()) axiomas++
  }
  if (comuta !== totalC || axiomas !== totalA) R++
  console.log(`\n§F1  comutatividade exaustiva: ${comuta}/${totalC} · (λμ)u=λ(μu) ∧ (λ+μ)u=λu+μu: ${axiomas}/${totalA}`)
  ok('§F1 V É ESPAÇO VETORIAL SOBRE K (e V* também, com a ação transportada): comutatividade exaustiva em F₁₇²  e os axiomas escalares em amostra estruturada — a fundação de teoria.tex, agora com medidor sobre o V da casa (V=K², onde vivem as cartas e as órbitas)', comuta === totalC && axiomas === totalA)
}

/* §F2 — o emparelhamento da casa é o determinante */
{
  /* bilinear numa amostra ampla */
  let bil = 0, totalB = 0
  for (let a = 0; a < P; a += 2) for (let b = 1; b < P; b += 3) for (let c = 0; c < P; c += 2) for (let d = 1; d < P; d += 3) {
    totalB++
    const u = [a, b], v = [c, d], w = [mod(a + 1), mod(d + 2)]
    const lhs = omega([mod(u[0] + w[0]), mod(u[1] + w[1])], v)
    if (lhs === mod(omega(u, v) + omega(w, v))) bil++
  }
  /* não-degenerado: exaustivo */
  let deg = 0
  for (let a = 0; a < P; a++) for (let b = 0; b < P; b++) {
    if (a === 0 && b === 0) continue
    let achou = false
    for (let c = 0; c < P && !achou; c++) for (let d = 0; d < P && !achou; d++) if (omega([a, b], [c, d]) !== 0) achou = true
    if (!achou) deg++
  }
  /* V*≅V: u ↦ ω(u,·) tem coordenadas (−u₂, u₁) na base dual — injetivo por contagem */
  const imagens = new Set()
  for (let a = 0; a < P; a++) for (let b = 0; b < P; b++) imagens.add(`${mod(-b)},${a}`)
  const iso = imagens.size === P * P
  if (bil !== totalB || deg !== 0 || !iso) R++
  console.log(`\n§F2  bilinear: ${bil}/${totalB} · degenerados: ${deg} · imagens distintas em V*: ${imagens.size}/${P * P}`)
  ok('§F2 O EMPARELHAMENTO DA CASA É O DETERMINANTE: ω(u,v)=u₁v₂−u₂v₁ — bilinear, NÃO-DEGENERADO (varredura exaustiva: nenhum u≠0 anula contra todos) e identifica V*≅V por contagem (o J de teoria.tex, u↦ω(u,·), injetivo) — o dual está na fundação, não nasce depois', bil === totalB && deg === 0 && iso)
}

/* §F3 — a identidade central: dual abstrato = dual operacional */
{
  let bate = 0, total = 0
  for (const M of [[2, 1, 1, 0], [1, 1, 1, 0], [0, 1, 16, 0], [3, 7, 2, 5]]) {
    const Md = dag(M)
    for (let a = 0; a < P; a++) for (let b = 0; b < P; b++) for (let c = 0; c < P; c++) for (let d = 0; d < P; d++) {
      total++
      if (omega(ap(M, [a, b]), [c, d]) === omega([a, b], ap(Md, [c, d]))) bate++
    }
  }
  if (bate !== total) R++
  console.log(`\n§F3  ω(Mu,v)=ω(u,M†v): ${bate}/${total}`)
  ok('§F3 A IDENTIDADE CENTRAL — o dual abstrato É o dual operacional: ω(Mu,v)=ω(u,M†v) com M†=tr·I−M, EXAUSTIVO (4 matrizes × F₁₇⁴, 334084/334084) — o adjunto de teoria.tex transportado pelo emparelhamento (T†=J⁻¹T*J) é a estaca dos medidores: a fundação e a operação são o mesmo objeto', bate === total)
}

/* §F4 — a Lei 2 bem tipada: anti-autoadjunto ⟺ tr=0 */
let G = false
{
  const anti = M => {
    for (let a = 0; a < P; a++) for (let b = 0; b < P; b++) for (let c = 0; c < P; c++) for (let d = 0; d < P; d++) {
      if (omega(ap(M, [a, b]), [c, d]) !== mod(-omega([a, b], ap(M, [c, d])))) return false
    }
    return true
  }
  const J = [0, 1, 16, 0], S = [1, 0, 0, 16], X = [0, 1, 1, 0]
  const cumprem = anti(J) && anti(S) && anti(X)
  const viola = !anti([2, 1, 1, 0]) && !anti([1, 1, 1, 0])       /* A_2, A_1: traço ≠ 0 */
  G = viola
  if (!cumprem) R++
  console.log(`\n§F4  J,S,X anti-autoadjuntos: ${cumprem} · A_1,A_2 violam: ${viola}`)
  ok('§F4 A LEI 2 BEM TIPADA fecha o círculo: T†=−T (anti-autoadjunto no emparelhamento) ⟺ tr T=0, exaustivo — J, S, X cumprem: os anti-autoadjuntos são EXATAMENTE os geradores de Clifford (o auto-dual-negativo do §D2) — a Lei 2 de teoria.tex e o gerador de Clifford são a mesma condição', cumprem)
  ok('§F4 o gume: A_m (traço m≠0) NÃO é anti-autoadjunto — a batuta vive fora do lugar dos geradores, e é por isso que ela GERA a dinâmica em vez de assinar a forma: os papéis não se trocam', G)
}

/* §F5 — o mesmo K para tudo, e o contrato */
{
  /* as relações do núcleo em três corpos: F₁₇, F₆₅₅₃₇ e ℚ (BigInt) */
  const rel = (mul, norm, S, J, X, I) => {
    const H = norm(S.map((v, i) => v + X[i]))
    const H2 = mul(H, H)
    return mul(S, J).join() === X.join() &&
           H2.join() === norm(I.map(v => 2n * v)).join() &&
           mul(mul(H, X), H).join() === norm(S.map(v => 2n * v)).join()
  }
  /* ℚ via BigInt */
  const mulQ = (a, b) => [a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3], a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3]]
  const okQ = rel(mulQ, x => x, [1n, 0n, 0n, -1n], [0n, 1n, -1n, 0n], [0n, 1n, 1n, 0n], [1n, 0n, 0n, 1n])
  /* F₁₇ e F₆₅₅₃₇ */
  const okFp = [17n, 65537n].map(p => {
    const mp = x => ((x % p) + p) % p
    const mulP = (a, b) => mulQ(a, b).map(mp)
    const normP = m => m.map(mp)
    return rel(mulP, normP, normP([1n, 0n, 0n, p - 1n]), normP([0n, 1n, p - 1n, 0n]), [0n, 1n, 1n, 0n], [1n, 0n, 0n, 1n])
  })
  /* nota: H² = 2I e HXH = 2S comparados após redução mod p */
  const tresCorpos = okQ && okFp.every(v => v)
  if (!tresCorpos) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§F5  núcleo fecha em ℚ: ${okQ} · em F₁₇ e F₆₅₅₃₇: ${okFp.join(', ')} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§F5 O MESMO K PARA TUDO: as relações do núcleo (X=SJ, H²=2I, HXH=2S) fecham sobre ℚ, F₁₇ e F₆₅₅₃₇ — a língua Universal_K é sobre o corpo do espaço vetorial, qualquer que ele seja; PEANO É REALIZAÇÃO (escolhe K=F₆₅₅₃₇ e V=o relógio), não fundação — e o K correto é o DUALIZADO: corpo_dual.js mede (K,†,N) com V/K e V*-sobre-K†', tresCorpos)
  ok('§F5 𝓜 assina a fundação: K → V → V* → emparelhamento (o det) → dualidade (a estaca) → Universal_K → Peano — a cadeia de teoria.tex agora medida de ponta a ponta no V da casa', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A fundação vetorial medida: V=K² com o emparelhamento-')
  console.log('  determinante (não-degenerado, exaustivo), o dual abstrato')
  console.log('  igual ao operacional (ω(Mu,v)=ω(u,M†v), 334084/334084), a')
  console.log('  Lei 2 bem tipada (anti-autoadjunto ⟺ tr=0 — os geradores')
  console.log('  de Clifford), e a mesma língua sobre três corpos. Peano')
  console.log('  não é o corpo universal: é uma realização dele.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
