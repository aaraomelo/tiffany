/* tests/nucleo_unificado.js — a árvore dual da unificação (ordem da
 * mesa, eval 14/08: «migrar para sistema unificado»; contrato do
 * gerente: o Universal é dono da ESTRUTURA — a quádrupla (X,S,H,J) e
 * as suas relações — e cada domínio continua dono da realização;
 * protocolo: núcleo novo → adaptadores → equivalência CASO A CASO
 * (trajetória e objeto, não só «ambos deram zero») → --refaz → só
 * depois a limpeza).
 *
 * Este medidor É a árvore dual: constrói cada objeto pelos DOIS
 * caminhos — o antigo (local, como os medidores legados fazem) e o
 * novo (o nucleo da lib) — e exige igualdade entrada a entrada.
 *
 * §N0  o contrato do núcleo: nucleo.verifica() e a DERIVAÇÃO — X e H
 *      re-derivados localmente de espelho/J batem entrada a entrada
 *      (a referência não é cópia: mudar o dado mudaria os dois lados);
 *      gume: uma quádrupla adulterada falha o contrato
 * §N1  o adaptador da ÁLGEBRA: o levantamento ⊗I₄ do núcleo reproduz
 *      byte a byte os blocos de clifford_pleno (e₃, S₈, H₈) e as
 *      relações valem no 8×8 com as mesmas constantes; gume: o
 *      levantamento errado (⊗espelho) não reproduz
 * §N2  o adaptador da GEOMETRIA/ESPECTRO: aplicar nucleo.H ao par de
 *      folhas dá (soma, diferença) em todos os k do palco de viviani,
 *      e a dobra de Cooley–Tukey fecha ATRAVÉS do núcleo, exata
 * §N3  o adaptador de PEANO (a instância original): no corpus de
 *      ensaio, ν∘ν = id byte a byte, RETAIN só quando TUDO zera,
 *      REOPEN acusa a mutação, e a fibra recompõe verbatim — a
 *      trajetória, não o escore
 * §N4  a torre inteira: verificaLeis() 8/8 E nucleo.verifica() — as
 *      leis e o núcleo de pé ao mesmo tempo
 */
'use strict'
const { Universal, mat2, nucleo, verificaLeis, anel, dft } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

/* §N0 — o contrato e a derivação */
{
  const { mul, soma, igual } = mat2
  /* o caminho ANTIGO: como teorema_universal.js construía, localmente */
  const trocaLocal = mul(mat2.espelho, mat2.J)
  const Hlocal = soma(trocaLocal, mat2.espelho)
  const derivado = igual(nucleo.X, trocaLocal) && igual(nucleo.H, Hlocal) &&
    igual(nucleo.S, mat2.espelho) && igual(nucleo.J, mat2.J)
  /* gume: uma quádrupla adulterada não passa o contrato */
  const adulterado = { ...nucleo, X: [0, 1, -1, 0] }
  const contratoAdulterado = (() => {
    const { mul: m, escala: e, igual: g } = mat2
    return g(adulterado.X, m(adulterado.S, adulterado.J)) &&
      g(m(m(adulterado.H, adulterado.X), adulterado.H), e(2, adulterado.S))
  })()
  console.log(`\n§N0  nucleo.verifica(): ${nucleo.verifica()} · derivação == local: ${derivado} · quádrupla adulterada passa? ${contratoAdulterado}`)
  ok('§N0 o contrato do núcleo fecha: as cinco relações verificadas pela própria lib', nucleo.verifica())
  ok('§N0 U_novo == U_antigo na origem: X e H do núcleo batem entrada a entrada com a construção local dos medidores legados', derivado)
  ok('§N0 o gume: a troca adulterada ([0,1,−1,0] = J) não satisfaz o contrato — a verificação tem dente', !contratoAdulterado)
}

/* §N1 — o adaptador da álgebra: o levantamento ⊗I₄ */
function bloco8 (P4, Q4, R4, S4) {
  const C = new Array(64)
  for (let i = 0; i < 4; i++) {
    for (let j = 0; j < 4; j++) {
      C[8 * i + j] = P4[4 * i + j]
      C[8 * i + j + 4] = Q4[4 * i + j]
      C[8 * (i + 4) + j] = R4[4 * i + j]
      C[8 * (i + 4) + j + 4] = S4[4 * i + j]
    }
  }
  return C
}
function mul8 (X, Y) {
  const C = new Array(64).fill(0n)
  for (let i = 0; i < 8; i++) {
    for (let k = 0; k < 8; k++) {
      const v = X[8 * i + k]
      if (v === 0n) continue
      for (let j = 0; j < 8; j++) C[8 * i + j] += v * Y[8 * k + j]
    }
  }
  return C
}
const eq8 = (X, Y) => X.every((v, i) => v === Y[i])
const Z4 = new Array(16).fill(0n)
const I4 = [1n, 0n, 0n, 0n, 0n, 1n, 0n, 0n, 0n, 0n, 1n, 0n, 0n, 0n, 0n, 1n]
/* o levantamento: M(2×2) ⊗ I₄ — cada entrada vira um bloco escalar */
const kronI4 = M => bloco8(
  I4.map(v => BigInt(M[0]) * v), I4.map(v => BigInt(M[1]) * v),
  I4.map(v => BigInt(M[2]) * v), I4.map(v => BigInt(M[3]) * v))
{
  /* o caminho ANTIGO: os blocos como clifford_pleno.js os escreve */
  const nI4 = I4.map(v => -v)
  const S8velho = bloco8(I4, Z4, Z4, nI4)
  const e3velho = bloco8(Z4, I4, I4, Z4)
  const H8velho = bloco8(I4, I4, I4, nI4)
  /* o caminho NOVO: levantar o núcleo */
  const S8novo = kronI4(nucleo.S), e3novo = kronI4(nucleo.X), H8novo = kronI4(nucleo.H)
  const bate = eq8(S8novo, S8velho) && eq8(e3novo, e3velho) && eq8(H8novo, H8velho)
  const relacoes = eq8(mul8(H8novo, H8novo), kronI4(mat2.escala(2, mat2.I))) &&
    eq8(mul8(mul8(H8novo, e3novo), H8novo), S8novo.map(v => 2n * v)) &&
    eq8(mul8(mul8(H8novo, S8novo), H8novo), e3novo.map(v => 2n * v))
  /* gume: o levantamento errado (⊗espelho em vez de ⊗I) não reproduz */
  const esp4 = [1n, 0n, 0n, 0n, 0n, -1n, 0n, 0n, 0n, 0n, 1n, 0n, 0n, 0n, 0n, -1n]
  const kronEsp = M => bloco8(
    esp4.map(v => BigInt(M[0]) * v), esp4.map(v => BigInt(M[1]) * v),
    esp4.map(v => BigInt(M[2]) * v), esp4.map(v => BigInt(M[3]) * v))
  const errado = eq8(kronEsp(nucleo.X), e3velho)
  console.log(`\n§N1  levantamento ⊗I₄ == blocos legados: ${bate} · relações no 8×8: ${relacoes} · ⊗espelho reproduz? ${errado}`)
  ok('§N1 o adaptador da ÁLGEBRA: nucleo⊗I₄ reproduz byte a byte os blocos de clifford_pleno (e₃, S₈, H₈)', bate)
  ok('§N1 e as relações sobem com as mesmas constantes: H₈²=2I₈, H₈e₃H₈=2S₈, H₈S₈H₈=2e₃ — a estrutura é do núcleo, o andar é da instância', relacoes)
  ok('§N1 o gume: o levantamento errado (⊗espelho) não reproduz a troca de bloco', !errado)
}

/* §N2 — o adaptador da geometria/espectro: H do núcleo nas folhas */
{
  const P = 65537
  const A = anel(P)
  const N = 16, M = 2 * N
  const a = 7
  const h = A.powm(3, 65536 / M)
  const i4g = A.powm(3, 16384)
  const i2 = A.inv(2)
  const zs = [], xs = []
  for (let k = 0; k < M; k++) {
    const hk = A.powm(h, k), hki = A.inv(hk)
    const sinu = A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4g))
    const h2 = hk * hk % P, h2i = A.inv(h2)
    const cost = A.mod((h2 + h2i) * i2)
    xs.push(A.mod(a * (1 + cost)))
    zs.push(A.mod(2 * a * sinu))
  }
  /* aplicar o H DO NÚCLEO ao par de folhas (v(k), v(k+N)) */
  const dobraH = (v, k) => {
    const par = [v[k], v[k + N]]
    return [
      A.mod(nucleo.H[0] * par[0] + nucleo.H[1] * par[1]),
      A.mod(nucleo.H[2] * par[0] + nucleo.H[3] * par[1]),
    ]
  }
  let bateFolhas = 0
  for (let k = 0; k < N; k++) {
    for (const v of [xs, zs]) {
      const [s, d] = dobraH(v, k)
      if (s === A.mod(v[k] + v[k + N]) && d === A.mod(v[k] - v[k + N])) bateFolhas++
    }
  }
  /* e a dobra de Cooley–Tukey ATRAVÉS do núcleo */
  let ct = 0
  for (const v of [xs, zs]) {
    const cheio = dft(v, A, h)
    const somas = [], difs = []
    for (let k = 0; k < N; k++) {
      const [s, d] = dobraH(v, k)
      somas.push(s)
      difs.push(d * A.inv(A.powm(h, k)) % P)
    }
    const w2 = h * h % P
    const pares = dft(somas, A, w2), impares = dft(difs, A, w2)
    let bate = true
    for (let j = 0; j < N; j++) {
      if (cheio[2 * j] !== pares[j] || cheio[2 * j + 1] !== impares[j]) bate = false
    }
    if (bate) ct++
  }
  console.log(`\n§N2  H do núcleo nas folhas: ${bateFolhas}/${2 * N} · Cooley–Tukey via núcleo: ${ct}/2 vetores`)
  ok('§N2 o adaptador da GEOMETRIA: nucleo.H aplicado ao par de folhas dá (soma, diferença) em todos os k — a dobra é a do núcleo', bateFolhas === 2 * N)
  ok('§N2 o adaptador do ESPECTRO: a dobra de Cooley–Tukey fecha ATRAVÉS do núcleo, exata no anel, x e z', ct === 2)
}

/* §N3 — o adaptador de Peano: a trajetória da instância original */
{
  const U = Universal(sigmaPeano)
  const l1 = '{"id":"a1","tipo":"conceito","corpo":"o reino"}'
  const l2 = '{"id":"b2","tipo":"conceito","corpo":"dourado"}'
  const z = U.funde('z9', l1, l2)
  /* ν∘ν = id, byte a byte (a volta exata) */
  const volta = U.monodromia(U.monodromia(z))
  const nuNu = volta === U.funde('a1', l1, l2)      /* o id externo muda de dono, o corpo é o mesmo */
  const fib = U.fibra(z)
  const recompoe = fib && fib[0] === l1 && fib[1] === l2
  /* RETAIN só quando TUDO zera; REOPEN acusa a mutação */
  const fonte = [['a1', l1], ['b2', l2]]
  const R0 = U.residuoTotal(fonte, [['a1', l1], ['b2', l2]])
  const R1 = U.residuoTotal(fonte, [['a1', l1], ['b2', l2.replace('dourado', 'dourAdo')]])
  const retemLimpo = U.retain(R0)
  const acusaMutado = !U.retain(R1) && R1.RE > 0
  console.log(`\n§N3  ν∘ν=id: ${nuNu} · fibra recompõe: ${recompoe} · RETAIN limpo: ${retemLimpo} · REOPEN no mutado: ${acusaMutado} (R=${JSON.stringify(R1)})`)
  ok('§N3 a instância PEANO de pé no núcleo novo: ν∘ν = id byte a byte e a fibra recompõe verbatim — a trajetória, não o escore', nuNu && recompoe)
  ok('§N3 RETAIN só quando TUDO zera, e a mutação de um byte acusa (REOPEN com R_E > 0) — o vetor completo, como atestado', retemLimpo && acusaMutado)
}

/* §N4 — a torre inteira */
{
  const leis = verificaLeis()
  const todas = leis.every(l => l.ok)
  console.log(`\n§N4  leis: ${leis.filter(l => l.ok).length}/8 · nucleo: ${nucleo.verifica()}`)
  ok('§N4 a torre inteira de pé: as 8 Leis E o contrato do núcleo, verdes ao mesmo tempo — a unificação não derrubou nada', todas && nucleo.verifica())
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A árvore dual fecha: o núcleo (X, S, H, J) é da lib, e os')
  console.log('  quatro adaptadores — álgebra (⊗I₄), geometria (folhas),')
  console.log('  espectro (Cooley–Tukey) e Peano (a instância original) —')
  console.log('  reproduzem os legados entrada a entrada. Unifica sem apagar')
  console.log('  as diferenças; a limpeza dos duplicados espera o --refaz.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
