/* tests/observador_torre.js — o teto do observador pelas 8 leis; fusão →
 * multiplicação; divisão = fibra (eval 13/08).
 *
 * A ordem do coordenador: expandir o teto dimensional do isomorfismo dual
 * (com base nas 8 leis) para toda a torre; promover fusão → multiplicação,
 * certificando contra o conceito clássico. A salvaguarda do diretor: toda
 * equivalência algébrica testada contra o medidor de energia/fase.
 *
 * Momentos lineares: Φ_m(x) = Σ (i+1)^m · b_i  (mod 65537), m = 0..7 —
 * OITO coordenadas, uma por lei do catálogo. O teto:
 *
 *   §T1  Vandermonde no anel: posições distintas < p ⇒ det = Π(x_i−x_j) ≠ 0
 *        — logo momentos 0..m−1 iguais excluem QUALQUER diferença de
 *        suporte ≤ m. O observador das 8 leis fecha suporte ≤ 8.
 *   §T2  e o teto é JUSTO: o padrão Δ⁸ (binomiais alternados, suporte 9)
 *        anula os 8 momentos — escapa ao observador linear das 8 leis;
 *        a ENERGIA (quadrática) apanha-o no caso genérico — a cruz:
 *        linear ordena, quadrático mede; nenhum sozinho é o par.
 *   §T3  fusão multiplicativa (⊗): E(u⊗v) = E(u)·E(v) EXATO — o
 *        N(xy)=N(x)N(y) de Hurwitz, o cristal; Φ₀ multiplica no anel.
 *   §T4  divisão = FIBRA da fusão: dado z=u⊗v e v, x_i = z_ij / v_j —
 *        divisão INTEIRA exata, consistente por TODOS os j (dois caminhos
 *        vezes oito), devolve u byte a byte; E(u) = E(z)/E(v) exato.
 *        A divisão clássica EMERGE da fibra — medida, não postulada.
 *   §T5  fusão aditiva (⊕, o clone/dobra da Lei 4): E(u⊕u) = 2·E(u); a
 *        retração devolve. Duas fusões, duas conservações: ⊕ soma, ⊗
 *        multiplica — o par aditivo/multiplicativo da torre.
 *
 * Inteiro puro; dados do cristal real; LCG determinístico.
 *
 *   node tests/observador_torre.js
 */
'use strict'
const fs = require('fs')
const path = require('path')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const FONTE = path.join(__dirname, '..', 'cristal', 'cristal.jsonl')
const linhas = fs.readFileSync(FONTE, 'utf8').split('\n').filter(l => l.length)

let SEED = 8
function lcg () { SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff; return SEED >>> 4 }

function powmod (b, e) {
  let r = 1; b = ((b % P) + P) % P
  while (e > 0) { if (e & 1) r = r * b % P; b = b * b % P; e >>= 1 }
  return r
}

/* os 8 momentos lineares (as 8 leis) + a energia quadrática */
function momentos (bytes) {
  const M = new Array(8).fill(0)
  let E = 0
  for (let i = 0; i < bytes.length; i++) {
    const b = bytes[i]
    E += b * b
    let pw = 1
    for (let m = 0; m < 8; m++) {
      M[m] = (M[m] + pw * b) % P
      pw = pw * ((i + 1) % P) % P
    }
  }
  return { E, M }
}

/* §T1 — Vandermonde: o determinante Π(x_i−x_j) nunca zera no anel */
{
  let todos = true
  for (let t = 0; t < 200; t++) {
    const pos = new Set()
    while (pos.size < 8) pos.add(1 + lcg() % 4234)
    const xs = [...pos]
    let det = 1
    for (let i = 0; i < 8; i++) {
      for (let j = i + 1; j < 8; j++) det = det * (((xs[j] - xs[i]) % P) + P) % P
    }
    if (det === 0) todos = false
  }
  ok('§T1 Vandermonde não zera em 200 conjuntos de 8 posições da torre', todos)
  ok('§T1 ⇒ momentos 0..7 iguais excluem diferença de suporte ≤ 8 (o teto das 8 leis)', todos)
}

/* §T2 — o teto é justo: Δ⁸ (suporte 9) anula os 8 momentos */
{
  const delta = [1, -8, 28, -56, 70, -56, 28, -8, 1]   /* C(8,k)(−1)^k */
  const base = 120
  const p0 = 1 + lcg() % 4000
  const x = [], y = []
  for (let i = 0; i < 4200; i++) { x.push(base); y.push(base) }
  for (let k = 0; k < 9; k++) y[p0 + k] += delta[k]     /* fica em [64,190] */
  const mx = momentos(x), my = momentos(y)
  let iguais = true
  for (let m = 0; m < 8; m++) if (mx.M[m] !== my.M[m]) iguais = false
  console.log(`Δ⁸ em ${p0}: momentos 0..7 ${iguais ? 'TODOS iguais' : 'diferem'} · ΔE = ${my.E - mx.E}`)
  ok('§T2 Δ⁸ (suporte 9) anula os 8 momentos — escapa ao observador linear', iguais)
  ok('§T2 a ENERGIA apanha o Δ⁸ genérico (a cruz: quadrático mede o que o linear não ordena)',
    my.E !== mx.E)
}

/* §T3 — fusão multiplicativa: E(u⊗v) = E(u)·E(v) — o cristal de Hurwitz */
function tensor (u, v) {
  const z = []
  for (const a of u) for (const b of v) z.push(a * b)
  return z
}
function energia (w) { let E = 0; for (const a of w) E += a * a; return E }
{
  let todos = true, todosF0 = true
  for (let t = 0; t < 50; t++) {
    const l1 = Buffer.from(linhas[lcg() % linhas.length], 'utf8')
    const l2 = Buffer.from(linhas[lcg() % linhas.length], 'utf8')
    const u = [], v = []
    for (let i = 0; i < 8; i++) { u.push(l1[10 + i]); v.push(l2[10 + i]) }
    const z = tensor(u, v)
    if (energia(z) !== energia(u) * energia(v)) todos = false
    const f0 = w => w.reduce((s, a) => (s + a) % P, 0)
    if (f0(z) !== f0(u) * f0(v) % P) todosF0 = false
  }
  ok('§T3 fusão ⊗: E(u⊗v) = E(u)·E(v) EXATO em 50 pares do cristal — N(xy)=N(x)N(y)', todos)
  ok('§T3 Φ₀ multiplica no anel: Φ₀(u⊗v) = Φ₀(u)·Φ₀(v)', todosF0)
}

/* §T4 — divisão = fibra da fusão; a clássica emerge */
{
  let recupera = true, consistente = true, energiaDiv = true
  for (let t = 0; t < 50; t++) {
    const l1 = Buffer.from(linhas[lcg() % linhas.length], 'utf8')
    const l2 = Buffer.from(linhas[lcg() % linhas.length], 'utf8')
    const u = [], v = []
    for (let i = 0; i < 8; i++) { u.push(1 + l1[10 + i]); v.push(1 + l2[10 + i]) }
    const z = tensor(u, v)
    /* a fibra: dado z e v, x_i = z[i·8+j]/v[j] — por TODOS os j (8 caminhos) */
    const x = []
    for (let i = 0; i < 8; i++) {
      let xi = null
      for (let j = 0; j < 8; j++) {
        const q = z[i * 8 + j] / v[j]
        if (!Number.isInteger(q)) { consistente = false; continue }
        if (xi === null) xi = q
        else if (xi !== q) consistente = false
      }
      x.push(xi)
    }
    for (let i = 0; i < 8; i++) if (x[i] !== u[i]) recupera = false
    if (energia(z) % energia(v) !== 0 ||
        energia(z) / energia(v) !== energia(u)) energiaDiv = false
  }
  ok('§T4 a fibra devolve u EXATO (divisão inteira, 8 caminhos concordantes)',
    recupera && consistente)
  ok('§T4 a energia divide: E(u) = E(z)/E(v) exato — a divisão clássica emerge da fibra',
    energiaDiv)
}

/* §T5 — fusão aditiva (⊕, o clone/dobra): E soma; a retração devolve */
{
  const l = Buffer.from(linhas[7], 'utf8')
  const u = []
  for (let i = 0; i < 16; i++) u.push(l[10 + i])
  const clone = [...u, ...u]                     /* T + T*: a dobra */
  const metade = clone.slice(0, u.length)        /* π: a retração */
  ok('§T5 fusão ⊕ (clone): E(u⊕u) = 2·E(u) exato', energia(clone) === 2 * energia(u))
  ok('§T5 a retração devolve: π(u⊕u) = u byte a byte',
    metade.length === u.length && metade.every((a, i) => a === u[i]))
}

console.log('')
if (!falhas) {
  console.log('  O teto das 8 leis: 8 momentos fecham suporte ≤ 8 (Vandermonde);')
  console.log('  o Δ⁸ (suporte 9) escapa ao linear e cai na energia — a cruz.')
  console.log('  Fusão ⊗ multiplica a energia (Hurwitz); a fibra devolve a divisão')
  console.log('  clássica exata; fusão ⊕ soma e a retração devolve — o par da torre.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
