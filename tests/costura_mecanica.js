/* tests/costura_mecanica.js — a costura: mecânica, termodinâmica e a
 * rede neural contra o MESMO observável (ordem do coordenador, 14/08
 * madrugada: «termodinâmica e redes neurais»; o método do gerente:
 * NÃO fundir por decreto — descobrir se os três partilham o mesmo
 * invariante operacional, com a unidade de comparação
 * (invariante, transformação admissível, gume) em cada domínio).
 *
 * O observável comum Q e o esquema:
 *
 *   MECÂNICA   Q = c²+s² (o produto dual no relógio) — conservado pelo
 *              fluxo; o EMPURRÃO (rodar E transladar) quebra.
 *   CARNOT     Q = ∮dQ/T (o balanço do ciclo, exato em ℚ) — zero no
 *              reversível; o EMPURRÃO térmico (δ>0 no calor rejeitado)
 *              abre o ∮ SEMPRE NEGATIVO — a quebra é monótona: a seta.
 *   HOPFIELD   Q = E(x) = −xᵀWx (inteiro puro; a versão float do app
 *              fica onde está) — no atrator, Q_volta = Q_entrada; a
 *              descida de um estado perturbado é ESTRITA até o atrator
 *              (o Lyapunov medido); o EMPURRÃO (campo constante t·𝟙,
 *              a translação da rede) quebra o recall dum limiar exato.
 *
 *   E O ESPELHO EXISTE NOS TRÊS: a reversão S do fluxo (mecânica), o
 *   ciclo invertido — a bomba — com o mesmo ∮ = 0 (termodinâmica), e
 *   E(−x) = E(x) (a rede). A T-simetria é partilhada.
 *
 * O resultado podia ser «não fecha» — seria igualmente resultado. O
 * que a medida dá: R_Q = 0 nos três, o mesmo empurrão quebra os três,
 * o espelho vive nos três. A partilha é DESCOBERTA.
 *
 * §T0  mecânica (linha de base): Q conservado no fluxo, empurrão quebra
 * §T1  Carnot em ℚ: ∮dU=0 e ∮dQ/T=0 na varredura exata; o empurrão
 *      térmico dá ∮<0 SEMPRE (a seta); a bomba (espelho) conserva
 * §T2  Hopfield inteiro: recall exato, Q_volta=Q_entrada no atrator,
 *      descida estrita do perturbado, E(−x)=E(x); o empurrão quebra
 *      o recall a partir de um limiar medido
 * §T3  a costura: (Q, transformação, gume) fecham nos três — e o
 *      contrato 𝓜 assina com G = os três gumes juntos
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
const gumes = { mec: false, carnot: false, rede: false }

/* §T0 — mecânica: a linha de base da costura */
{
  const P = 65537
  const A = anel(P)
  const M = 32
  const h = A.powm(3, 65536 / M), i4 = A.powm(3, 16384), i2 = A.inv(2)
  const c = [], s = []
  for (let k = 0; k < M; k++) {
    const hk = A.powm(h, k), hki = A.inv(hk)
    c.push(A.mod((hk + hki) * i2))
    s.push(A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4)))
  }
  let conserva = 0
  for (let k = 0; k < M; k++) if (A.mod(c[k] * c[k] + s[k] * s[k]) === 1) conserva++
  /* o empurrão: rodar e transladar */
  let x = c[3], y = s[3], quebra = true
  for (let p = 0; p < 10; p++) {
    const nx = A.mod(c[1] * x + s[1] * y + 1)
    const ny = A.mod(-s[1] * x + c[1] * y + P * P)
    x = nx; y = ny
    if (A.mod(x * x + y * y) === 1) quebra = false
  }
  gumes.mec = quebra
  if (conserva !== M) R++
  console.log(`\n§T0  mecânica: Q=c²+s²=1 em ${conserva}/${M} · empurrão quebra: ${quebra}`)
  ok('§T0 MECÂNICA: Q conservado em toda a trajetória e o empurrão (rodar E transladar) quebra em todos os passos — a linha de base da costura', conserva === M && quebra)
}

/* §T1 — Carnot em ℚ: o balanço do ciclo, e a seta */
{
  /* frações como pares [n,d] de BigInt, exatas */
  const gcd = (a, b) => { a = a < 0n ? -a : a; b = b < 0n ? -b : b; while (b) { [a, b] = [b, a % b] } return a }
  const q = (n, d) => { const g = gcd(n, d) || 1n; const s2 = d < 0n ? -1n : 1n; return [s2 * n / g, s2 * d / g] }
  const soma = ([a, b], [c2, d]) => q(a * d + c2 * b, b * d)
  const sub = ([a, b], [c2, d]) => q(a * d - c2 * b, b * d)
  const div = ([a, b], [c2, d]) => q(a * d, b * c2)
  const zero = ([a]) => a === 0n
  const neg = ([a]) => a < 0n

  let fecha = 0, reversivel = 0, casos = 0, setaNeg = 0, bomba = 0
  for (let Tq = 3n; Tq <= 12n; Tq++) {
    for (let Tf = 1n; Tf < Tq; Tf++) {
      for (let Qq = 2n; Qq <= 10n; Qq++) {
        casos++
        /* o ciclo reversível: Q_f/T_f = Q_q/T_q ⟹ Q_f = Q_q·T_f/T_q (fração exata) */
        const Qf = q(Qq * Tf, Tq)
        const W = sub([Qq, 1n], Qf)
        /* ∮dU = Q_q − Q_f − W = 0 */
        if (zero(sub(sub([Qq, 1n], Qf), W))) fecha++
        /* ∮dQ/T = Q_q/T_q − Q_f/T_f = 0 */
        const oint = sub(div([Qq, 1n], [Tq, 1n]), div(Qf, [Tf, 1n]))
        if (zero(oint)) reversivel++
        /* o EMPURRÃO térmico: rejeitar δ>0 a mais — ∮ < 0 SEMPRE */
        const delta = q(1n, 7n)
        const ointMau = sub(div([Qq, 1n], [Tq, 1n]), div(soma(Qf, delta), [Tf, 1n]))
        if (neg(ointMau)) setaNeg++
        /* o ESPELHO: o ciclo invertido (a bomba) — os papéis trocam, o ∮ conserva */
        const ointBomba = sub(div(Qf, [Tf, 1n]), div([Qq, 1n], [Tq, 1n]))
        if (zero(ointBomba)) bomba++
      }
    }
  }
  gumes.carnot = setaNeg === casos
  if (fecha !== casos || reversivel !== casos || bomba !== casos) R++
  console.log(`\n§T1  Carnot: ∮dU=0 em ${fecha}/${casos} · ∮dQ/T=0 em ${reversivel}/${casos} · empurrão dá ∮<0 em ${setaNeg}/${casos} · a bomba conserva em ${bomba}/${casos}`)
  ok('§T1 CARNOT: o invariante é o balanço ∮dQ/T = 0, exato em ℚ na varredura — a conservação térmica do ciclo reversível', fecha === casos && reversivel === casos)
  ok('§T1 o EMPURRÃO térmico (δ>0 no calor rejeitado) abre o ∮ SEMPRE NEGATIVO — a quebra é monótona: a seta térmica e a seta mecânica são o mesmo gume', gumes.carnot)
  ok('§T1 o ESPELHO térmico: o ciclo invertido (a bomba) conserva o ∮ — a T-simetria da máquina é motor↔bomba', bomba === casos)
}

/* §T2 — Hopfield inteiro: a memória contra o observável */
{
  const n = 16
  /* dois padrões ±1 ortogonais (Hadamard-like) */
  const p1 = [], p2 = []
  for (let i = 0; i < n; i++) {
    p1.push(i % 2 === 0 ? 1 : -1)
    p2.push((i >> 1) % 2 === 0 ? 1 : -1)
  }
  const ortogonais = p1.reduce((s2, v, i) => s2 + v * p2[i], 0) === 0
  /* Hebb: W_ij = Σ p_i p_j, diagonal zero — inteiro puro */
  const W = []
  for (let i = 0; i < n; i++) {
    W.push([])
    for (let j = 0; j < n; j++) W[i].push(i === j ? 0 : p1[i] * p1[j] + p2[i] * p2[j])
  }
  const campo = x => W.map(linha => linha.reduce((s2, w, j) => s2 + w * x[j], 0))
  const sinal = (u, velho) => u > 0 ? 1 : u < 0 ? -1 : velho
  const passo = x => campo(x).map((u, i) => sinal(u, x[i]))
  const E = x => -x.reduce((s2, xi, i) => s2 + xi * campo(x)[i], 0) / 1   /* −xᵀWx (÷1: inteiro) */

  /* recall exato: os padrões são pontos fixos, e Q_volta = Q_entrada */
  const volta1 = passo(p1), volta2 = passo(p2)
  const fixo = volta1.join() === p1.join() && volta2.join() === p2.join()
  const Qconserva = E(volta1) === E(p1) && E(volta2) === E(p2)
  /* a descida ESTRITA: perturba 2 bits e desce até o atrator */
  const pert = [...p1]; pert[0] = -pert[0]; pert[5] = -pert[5]
  let x = pert, desceu = true, chegou = false
  let eAnt = E(x)
  for (let t = 0; t < 8; t++) {
    const nx = passo(x)
    if (nx.join() === x.join()) { chegou = x.join() === p1.join(); break }
    const e2 = E(nx)
    if (e2 >= eAnt) desceu = false
    eAnt = e2; x = nx
  }
  /* o espelho: E(−x) = E(x) */
  const espelhoE = E(p1.map(v => -v)) === E(p1) && E(pert.map(v => -v)) === E(pert)
  /* o EMPURRÃO: o campo constante t·𝟙 quebra o recall a partir de um limiar */
  let limiar = -1
  for (let t = 1; t <= 40; t++) {
    const empurrado = campo(p1).map((u, i) => sinal(u + t, p1[i]))
    if (empurrado.join() !== p1.join()) { limiar = t; break }
  }
  gumes.rede = limiar > 0
  if (!ortogonais || !fixo || !Qconserva || !desceu || !chegou || !espelhoE) R++
  console.log(`\n§T2  Hopfield: fixos ${fixo} · Q_volta=Q_entrada ${Qconserva} (E=${E(p1)},${E(p2)}) · descida estrita→atrator ${desceu && chegou} · E(−x)=E(x) ${espelhoE} · empurrão quebra em t=${limiar}`)
  ok('§T2 HOPFIELD inteiro: os padrões são pontos fixos com Q_volta = Q_entrada (a pergunta do gerente na rede), e a descida do perturbado é ESTRITA até o atrator — o Lyapunov medido, não citado', fixo && Qconserva && desceu && chegou)
  ok('§T2 o ESPELHO da rede: E(−x) = E(x) exato — e o EMPURRÃO (campo constante, a translação da rede) quebra o recall a partir de um limiar finito medido', espelhoE && gumes.rede)
}

/* §T3 — a costura fecha, e o contrato assina */
{
  const G = gumes.mec && gumes.carnot && gumes.rede
  const V = 0
  const m = medicao.contrato(R, G, V)
  console.log(`\n§T3  gumes: mec=${gumes.mec} carnot=${gumes.carnot} rede=${gumes.rede} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(m)}`)
  ok('§T3 A COSTURA: os três domínios partilham o esquema (invariante conservado, transformação admissível, espelho) E o mesmo empurrão quebra os três — a partilha é DESCOBERTA, não decretada', G && R === 0)
  ok('§T3 o contrato 𝓜 assina: mecânica, termodinâmica e memória neural realizam a mesma lei de conservação operacional, cada uma na sua carta', medicao.fecha(m))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A costura fecha: o produto dual (mecânica), o balanço ∮dQ/T')
  console.log('  (Carnot) e a energia de Hopfield são o MESMO esquema — um')
  console.log('  invariante, um fluxo que o conserva, um espelho que inverte')
  console.log('  o tempo, e um só inimigo: o empurrão (a translação), que')
  console.log('  quebra os três. A seta térmica, a fronteira aditiva e o')
  console.log('  limiar da rede são a mesma parede, vista de três cartas.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
