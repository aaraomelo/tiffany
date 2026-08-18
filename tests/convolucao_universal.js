/* tests/convolucao_universal.js — a convolução universal: emerge da
 * transformada, ou fica no mapa? (o ÚNICO laboratório autorizado pela
 * mesa, eval 14/08: «um laboratório, uma pergunta»; gerente: «não
 * assumir previamente que o * correto é a convolução clássica — deixar
 * o Universal definir o candidato»; diretor: «testar se a convolução
 * emerge de forma nativa da Transformada Universal e das operações de
 * anel, preservando a conservação e os invariantes».)
 *
 * O CANDIDATO NÃO SE IMPORTA — DERIVA-SE: a transformada universal é a
 * avaliação nas folhas, e a multiplicação do corpo realiza-se por
 * matrizes (f ↦ f(A_m)). O produto de corpos f(A)·g(A) INDUZ uma
 * operação nas sequências de coeficientes; o medidor pergunta que
 * operação é essa, e mede as propriedades que a mesa listou.
 *
 * O que a medida dá: o candidato induzido É a soma sobre i+j=k (a
 * forma aditiva da multiplicação), associativo, comutativo, com
 * identidade δ; a transformada casa nas DUAS realizações (folhas e
 * anel cíclico); a conservação é MULTIPLICATIVA (a massa multiplica;
 * as autocorrelações convolvem); a deconvolução é a divisão espectral
 * — exata fora dos divisores de zero, e os divisores de zero são
 * EXIBIDOS (b⊛x=0 com b,x≠0: o preço do espectro zerado, o tema do
 * corpo_analitico); e a lei é INVARIANTE DE ESCALA na cascata.
 *
 * §V0  o candidato emerge: f(A)·g(A) == (Σ_{i+j=k} a_i b_j)(A) —
 *      matrizes inteiras exatas, m=1..3, coeficientes reais do
 *      cristal; e a dobra algébrica σ²=mσ+1 reduz ao corpo a+bσ
 * §V1  álgebra: associativa, comutativa, identidade δ — pelo caminho
 *      das matrizes (que não sabe o que é convolução)
 * §V2  a transformada casa: nas folhas, eval_σ(a*b) = eval_σ(a)·
 *      eval_σ(b) mod q (σ E σ†); no anel cíclico, dft(a⊛b) =
 *      dft(a)·dft(b) ponto a ponto (a dft da lib)
 * §V3  a conservação é multiplicativa (Lei 7): a massa multiplica
 *      (Σ(a⊛b) = Σa·Σb); as autocorrelações convolvem
 *      (c⊛c̃ = (a⊛ã)⊛(b⊛b̃)) — a energia do produto é a convolução
 *      das energias
 * §V4  A DECONVOLUÇÃO É A DIVISÃO ESPECTRAL: com espectro de b sem
 *      zeros, a recupera-se EXATA (byte a byte, R_total=0) — o dual
 *      da convolução nomeado com as duas partes
 * §V5  o gume: os divisores de zero — b=1⃗ e x=[1,−1,0,…] dão
 *      b⊛x=0 com ambos ≠0; a e a+x colidem (a⊛b = (a+x)⊛b); a
 *      deconvolução falha EXATAMENTE onde o espectro zera (medido:
 *      dft(1⃗) tem N−1 zeros)
 * §V6  a lei é invariante de escala: o teorema da convolução fecha em
 *      N=16, 8, 4 — cada nível da cascata com o seu ω
 *
 *   node tests/convolucao_universal.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { Universal, mat2, anel, dft, idft } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')
const U = Universal(sigmaPeano)

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const { mul, soma, escala, igual, I, Am } = mat2

/* ── as sequências reais: coeficientes dos bytes do cristal ──────────────── */
const bytes = Buffer.from(fs.readFileSync(
  path.join(__dirname, '..', 'cristal', 'cristal.jsonl'), 'utf8').slice(0, 64), 'utf8')
const seq = (ini, n) => Array.from(bytes.slice(ini, ini + n))

/* o corpo pelo caminho das MATRIZES: f ↦ Σ a_i·A^i (não sabe convolução) */
function corpoDe (coefs, A) {
  let M = escala(0, I)
  let Ai = I
  for (const a of coefs) {
    M = soma(M, escala(a, Ai))
    Ai = mul(Ai, A)
  }
  return M
}
/* o candidato induzido: a soma sobre i+j=k */
function estrela (a, b) {
  const c = new Array(a.length + b.length - 1).fill(0)
  for (let i = 0; i < a.length; i++) {
    for (let j = 0; j < b.length; j++) c[i + j] += a[i] * b[j]
  }
  return c
}

/* §V0 — o candidato emerge do produto de corpos */
{
  const a = seq(0, 4), b = seq(4, 4)
  let emerge = true, dobra = true
  for (let m = 1; m <= 3; m++) {
    const A = Am(m)
    if (!igual(mul(corpoDe(a, A), corpoDe(b, A)), corpoDe(estrela(a, b), A))) emerge = false
    /* a dobra algébrica: σ²=mσ+1 reduz qualquer grau ao corpo r0+r1σ —
     * redução por recorrência contra a matriz cheia */
    const c = estrela(a, b)
    let r = c.slice()
    for (let k = r.length - 1; k >= 2; k--) {
      /* σ^k = m·σ^{k−1} + σ^{k−2} */
      r[k - 1] += m * r[k]
      r[k - 2] += r[k]
      r[k] = 0
    }
    if (!igual(corpoDe(c, A), soma(escala(r[0], I), escala(r[1], A)))) dobra = false
  }
  ok('§V0 O CANDIDATO EMERGE: f(A)·g(A) == (Σ_{i+j=k} a_i b_j)(A) — matrizes inteiras exatas, m=1..3, bytes reais',
    emerge)
  ok('§V0 a dobra algébrica: σ²=mσ+1 reduz o produto ao corpo r₀+r₁σ, recorrência == matriz cheia',
    dobra)
}

/* §V1 — a álgebra, pelo caminho que não sabe o que é convolução */
{
  const a = seq(8, 3), b = seq(11, 3), c = seq(14, 3)
  const A = Am(2)
  const delta = [1]
  ok('§V1 associativa: (a*b)*c == a*(b*c), pelo produto de matrizes e pelo candidato',
    igual(mul(mul(corpoDe(a, A), corpoDe(b, A)), corpoDe(c, A)),
      corpoDe(estrela(estrela(a, b), c), A)) &&
    estrela(estrela(a, b), c).join() === estrela(a, estrela(b, c)).join())
  ok('§V1 comutativa e com identidade: a*b == b*a e a*δ == a',
    estrela(a, b).join() === estrela(b, a).join() &&
    estrela(a, delta).join() === a.join())
}

/* ── o anel cíclico: q=257, N=16, ω de ordem 16 ──────────────────────────── */
const q = 257
const AN = anel(q)
const N = 16
const w16 = AN.powm(3, 256 / N)
const aC = seq(16, N).map(AN.mod)
const bC = seq(32, N).map(AN.mod)
function cicla (a, b) {
  const c = new Array(N).fill(0)
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < N; j++) c[(i + j) % N] = (c[(i + j) % N] + a[i] * b[j]) % q
  }
  return c
}

/* §V2 — a transformada casa, nas duas realizações */
{
  /* nas folhas (m=2: σ=61, σ†=198 — medidas em metronomo_fourier) */
  const a = seq(0, 4), b = seq(4, 4)
  const avalia = (coefs, x) => {
    let s = 0, xi = 1
    for (const co of coefs) { s = (s + co * xi) % q; xi = xi * x % q }
    return s
  }
  let folhas = true
  for (const sig of [61, 198]) {
    if (avalia(estrela(a, b), sig) !== avalia(a, sig) * avalia(b, sig) % q) folhas = false
  }
  ok('§V2 nas FOLHAS: eval_σ(a*b) = eval_σ(a)·eval_σ(b) mod q, em σ E em σ† — a transformada casa',
    folhas)
  /* no anel cíclico, com a dft da lib */
  const cA = dft(aC, AN, w16), cB = dft(bC, AN, w16)
  const cAB = dft(cicla(aC, bC), AN, w16)
  ok('§V2 no ANEL: dft(a⊛b) = dft(a)·dft(b) ponto a ponto, nos 16 modos — o teorema da convolução emerge',
    cAB.every((s, k) => s === cA[k] * cB[k] % q))
}

/* §V3 — a conservação é multiplicativa (Lei 7) */
{
  const somaDe = a => a.reduce((s, v) => (s + v) % q, 0)
  const c = cicla(aC, bC)
  ok('§V3 a MASSA multiplica: Σ(a⊛b) = Σa·Σb mod q — a conservação da Lei 7, aditiva vista de fora',
    somaDe(c) === somaDe(aC) * somaDe(bC) % q)
  /* as autocorrelações convolvem: c⊛c̃ = (a⊛ã)⊛(b⊛b̃) */
  const til = a => a.map((_, n) => a[(N - n) % N])
  const auto = a => cicla(a, til(a))
  ok('§V3 as autocorrelações CONVOLVEM: (a⊛b)⊛(a⊛b)̃ = (a⊛ã)⊛(b⊛b̃) — a energia do produto é a convolução das energias',
    auto(c).join() === cicla(auto(aC), auto(bC)).join())
}

/* §V4 — a deconvolução é a divisão espectral (o dual, com as duas partes) */
{
  const cB = dft(bC, AN, w16)
  const invertivel = cB.every(s => s !== 0)
  const c = cicla(aC, bC)
  const cC = dft(c, AN, w16)
  const rec = idft(cC.map((s, k) => s * AN.inv(cB[k]) % q), AN, w16)
  const R = U.residuoTotal([['a', aC.join(',')]], [['a', rec.join(',')]])
  console.log(`espectro de b: ${invertivel ? 'sem zeros' : 'COM zeros'} · deconvolução R=(${R.Rend},${R.RE},${R.RF1},${R.RF2},${R.RD})`)
  ok('§V4 A DECONVOLUÇÃO É A DIVISÃO ESPECTRAL: com espectro sem zeros, a volta é EXATA — R_total=0, RETAIN',
    invertivel && rec.every((v, n) => v === aC[n]) && U.retain(R))
}

/* §V5 — o gume: os divisores de zero (o tema do corpo_analitico) */
{
  const uns = new Array(N).fill(1)
  const x = new Array(N).fill(0); x[0] = 1; x[1] = q - 1        /* [1,−1,0,…] */
  const zero = cicla(uns, x)
  ok('§V5 divisores de zero EXIBIDOS: 1⃗ ⊛ [1,−1,0,…] = 0 com ambos ≠ 0 — dois não-nulos, produto nulo',
    zero.every(v => v === 0) && uns.some(v => v) && x.some(v => v))
  const a2 = aC.map((v, n) => (v + x[n]) % q)                   /* a' = a + x */
  ok('§V5 a colisão: a ≠ a′ mas a⊛1⃗ == a′⊛1⃗ — onde o espectro zera, a deconvolução PERDE informação',
    a2.join() !== aC.join() && cicla(aC, uns).join() === cicla(a2, uns).join())
  const cUns = dft(uns, AN, w16)
  ok('§V5 o porquê, medido: dft(1⃗) tem exatamente N−1 zeros — a falha vive TODA no espectro zerado',
    cUns.filter(s => s === 0).length === N - 1 && cUns[0] === N)
}

/* §V6 — a lei é invariante de escala (a cascata) */
{
  let invariante = true
  for (const M of [16, 8, 4]) {
    const wM = AN.powm(3, 256 / M)
    const a = seq(0, M).map(AN.mod), b = seq(M, M).map(AN.mod)
    const cic = (u, v) => {
      const c = new Array(M).fill(0)
      for (let i = 0; i < M; i++) for (let j = 0; j < M; j++) c[(i + j) % M] = (c[(i + j) % M] + u[i] * v[j]) % q
      return c
    }
    const dA = dft(a, AN, wM), dB = dft(b, AN, wM), dC = dft(cic(a, b), AN, wM)
    if (!dC.every((s, k) => s === dA[k] * dB[k] % q)) invariante = false
  }
  ok('§V6 a lei é INVARIANTE DE ESCALA: o teorema da convolução fecha em N=16, 8 e 4 — cada nível da cascata com o seu ω',
    invariante)
}

console.log('')
if (!falhas) {
  console.log('  A PONTE EXISTE: a convolução EMERGE da transformada — é a forma')
  console.log('  aditiva da multiplicação do corpo (o produto de matrizes induz a')
  console.log('  soma sobre i+j=k, sem a importar), associativa e com identidade;')
  console.log('  a transformada casa nas folhas e no anel; a conservação é')
  console.log('  multiplicativa (a massa multiplica, as autocorrelações convolvem);')
  console.log('  a DECONVOLUÇÃO é a divisão espectral — exata fora dos divisores')
  console.log('  de zero, que ficam exibidos com a colisão à vista; e a lei é a')
  console.log('  mesma em todos os níveis da cascata. Sai do mapa, entra medida.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
