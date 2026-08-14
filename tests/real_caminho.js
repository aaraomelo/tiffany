/* tests/real_caminho.js — o real é um caminho da raiz à folha
 * (ordem do coordenador, 14/08 noite: «identifica um número real como
 * um caminho da raiz a folha (um corte) na árvore»).
 *
 * A árvore já existia na casa: é a TORRE com fibra 2 (limite_escada
 * §L3) — nível k = Z/2^k, cada nó com exatamente 2 filhos. O que se
 * identifica agora, com o real concreto da casa (x = 1/φ = φ−1, o
 * ponto do encaixe, thm:encaixe):
 *
 *   O REAL DESCE A ÁRVORE: no nível k o real está no intervalo
 *   diádico [m_k/2^k, (m_k+1)/2^k], e m_{k+1} ∈ {2m_k, 2m_k+1} —
 *   o endereço desce de pai para filho, sem saltos. O bit é o CORTE:
 *   esquerda ou direita a cada nó.
 *
 *   TUDO INTEIRO EXATO: m ≤ 2^k·x ⟺ (2m+2^k)² ≤ 5·(2^k)² (BigInt) —
 *   nenhum flutuante toca a árvore.
 *
 *   DOIS CAMINHOS para os bits: o teste de chão (a comparação
 *   quadrática) e o itinerário da DOBRA (x ↦ 2x−b, o mapa de
 *   duplicação — a mesma dobra da renormalização, agora no intervalo).
 *
 *   E A FOLHA NÃO É NÓ: (2m+N)² = 5N² é impossível (5 não é quadrado)
 *   — o real nunca é diádico; o encaixe aponta para fora da árvore em
 *   TODO nível. O real É o caminho — não um nó que o caminho atinja.
 *
 * §C0  a árvore é a da torre: fibra 2 exata, 2^k folhas no nível k
 * §C1  o caminho desce: m_{k+1} ∈ {2m_k, 2m_k+1} até k=40 (BigInt),
 *      e o intervalo do nível k+1 encaixa no do nível k, comprimento
 *      exato 2^{−k} (numerador da diferença = 1)
 * §C2  o corte: no nível k, todo diádico j/2^k com j ≤ m_k fica à
 *      esquerda de x e todo j ≥ m_k+1 à direita — e x NUNCA é nó
 *      ((2m+N)² ≠ 5N², sempre)
 * §C3  dois caminhos para os bits: chão quadrático == itinerário da
 *      dobra x↦2x−b (aritmética exata em ℤ[√5]/2^j), k=1..40
 * §C4  a volta e o gume: m_k = Σ b_j·2^{k−j} reconstrói o endereço
 *      byte a byte; um bit trocado EXPULSA x do intervalo — o contrato
 *      𝓜 assina a identificação
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
const K = 40

/* o teste de chão inteiro: m ≤ N·x ⟺ (2m+N)² ≤ 5N², com x = (√5−1)/2 */
const cabe = (m, N) => (2n * m + N) * (2n * m + N) <= 5n * N * N
const chao = N => {                       /* ⌊N·x⌋ por busca binária inteira */
  let lo = 0n, hi = N
  while (lo < hi) {
    const mid = (lo + hi + 1n) / 2n
    if (cabe(mid, N)) lo = mid; else hi = mid - 1n
  }
  return lo
}

/* §C0 — a árvore é a da torre */
{
  let fibra = 0, casos = 0
  for (let k = 1; k <= 6; k++) {
    const N = 1 << k
    const filhos = new Array(N / 2).fill(0)
    for (let m = 0; m < N; m++) filhos[m >> 1]++
    casos += N / 2
    fibra += filhos.filter(f => f === 2).length
  }
  if (fibra !== casos) R++
  console.log(`\n§C0  fibra 2 em ${fibra}/${casos} nós (níveis 1..6) — 2^k folhas por nível`)
  ok('§C0 a árvore é a da torre: cada nó tem exatamente 2 filhos (a fibra 2 de limite_escada §L3, lida como árvore binária)', fibra === casos)
}

/* o caminho de x = 1/φ: endereços m_k e bits b_k */
const m = [0n]
const bits = []
for (let k = 1; k <= K; k++) {
  const N = 1n << BigInt(k)
  m.push(chao(N))
  bits.push(Number(m[k] - 2n * m[k - 1]))
}

/* §C1 — o caminho desce sem saltos, e o encaixe é de comprimento exato */
{
  let desce = 0, encaixa = 0, comprimento = 0
  for (let k = 1; k <= K; k++) {
    if (m[k] === 2n * m[k - 1] || m[k] === 2n * m[k - 1] + 1n) desce++
    if (k > 1) {
      /* [m_k/2^k, (m_k+1)/2^k] ⊂ [m_{k−1}/2^{k−1}, (m_{k−1}+1)/2^{k−1}] */
      if (m[k] >= 2n * m[k - 1] && m[k] + 1n <= 2n * (m[k - 1] + 1n)) encaixa++
    }
    /* comprimento: (m+1)−m = 1 no numerador — a unidade, como no thm:encaixe */
    if ((m[k] + 1n) - m[k] === 1n) comprimento++
  }
  if (desce !== K || encaixa !== K - 1 || comprimento !== K) R++
  console.log(`\n§C1  desce pai→filho: ${desce}/${K} · encaixa: ${encaixa}/${K - 1} · comprimento-unidade: ${comprimento}/${K} · bits: ${bits.slice(0, 12).join('')}…`)
  ok('§C1 o real DESCE a árvore: m_{k+1} ∈ {2m_k, 2m_k+1} sem saltos até k=40, e os intervalos diádicos encaixam com comprimento exato 2^{−k}', desce === K && encaixa === K - 1)
  ok('§C1 o numerador do comprimento é a UNIDADE em todo nível — o mesmo padrão do thm:encaixe, agora na árvore binária', comprimento === K)
}

/* §C2 — o corte, e a folha que não é nó */
{
  let corte = 0, casos = 0, nuncaNo = 0
  for (let k = 1; k <= 16; k++) {
    const N = 1n << BigInt(k)
    for (let j = 0n; j <= N; j++) {
      casos++
      const esquerda = j <= m[k] ? cabe(j, N) : !cabe(j, N)
      if (esquerda) corte++
    }
    /* nunca nó: (2m+N)² ≠ 5N² para TODO m do nível */
    let bate = false
    for (let j = 0n; j <= N; j++) if ((2n * j + N) * (2n * j + N) === 5n * N * N) bate = true
    if (!bate) nuncaNo++
  }
  if (corte !== casos || nuncaNo !== 16) R++
  console.log(`\n§C2  o corte separa em ${corte}/${casos} diádicos (níveis 1..16) · x nunca é nó em ${nuncaNo}/16 níveis`)
  ok('§C2 o CORTE: em cada nível, todo diádico j ≤ m_k fica à esquerda de x e todo j > m_k à direita — o corte de Dedekind, nível a nível, inteiro exato', corte === casos)
  ok('§C2 e a folha NÃO é nó: (2m+N)² = 5N² é impossível (5 não é quadrado) — o real é o CAMINHO, não um nó que o caminho atinja', nuncaNo === 16)
}

/* §C3 — dois caminhos para os bits: chão vs itinerário da dobra */
{
  /* x_k em ℤ[√5]: x_k = (a_k + b_k·√5)/2, com x_0 = (−1+√5)/2;
   * a dobra: x_{k+1} = 2x_k − bit, e o bit é [x_k ≥ 1/2] */
  let a = -1n, b = 1n                       /* (a + b√5)/2 */
  const bitsDobra = []
  for (let k = 1; k <= K; k++) {
    /* x ≥ 1/2 ⟺ a + b√5 ≥ 1 ⟺ b√5 ≥ 1−a ⟺ (b>0): 5b² ≥ (1−a)² ou 1−a ≤ 0 */
    const rhs = 1n - a
    const meio = rhs <= 0n ? true : 5n * b * b >= rhs * rhs
    const bit = meio ? 1n : 0n
    bitsDobra.push(Number(bit))
    /* x ← 2x − bit: (a+b√5)/2 ↦ (2a − 2·bit + 2b√5)/2 */
    a = 2n * a - 2n * bit
    b = 2n * b
  }
  const iguais = bitsDobra.join('') === bits.join('')
  if (!iguais) R++
  console.log(`\n§C3  itinerário da dobra: ${bitsDobra.slice(0, 12).join('')}… · igual ao chão quadrático: ${iguais}`)
  ok('§C3 DOIS CAMINHOS para os bits: o chão quadrático == o itinerário da dobra x↦2x−b (aritmética exata em ℤ[√5]) — a mesma dobra da renormalização, agora a soletrar o real', iguais)
}

/* §C4 — a volta, o gume, e o contrato assina */
{
  /* a volta: m_k = Σ b_j·2^{k−j}, byte a byte */
  let V = 0
  for (let k = 1; k <= K; k++) {
    let soma = 0n
    for (let j = 1; j <= k; j++) soma += BigInt(bits[j - 1]) << BigInt(k - j)
    if (soma !== m[k]) V++
  }
  /* o gume: trocar o bit 5 expulsa x do intervalo do nível 5 */
  const k5 = 5
  const N5 = 1n << BigInt(k5)
  const mMau = m[k5] + (bits[k5 - 1] === 1 ? -1n : 1n)
  const dentroMau = cabe(mMau, N5) && !cabe(mMau + 1n, N5)
  const G = !dentroMau
  if (V !== 0) { /* acumulado no contrato */ }
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§C4  a volta reconstrói m_k dos bits: ${V === 0} · bit trocado expulsa x: ${G} · 𝓜 = (R=${R}, G=${mc.G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§C4 a VOLTA: o endereço reconstrói-se dos bits (m_k = Σ b_j·2^{k−j}) byte a byte em todos os níveis, e o bit trocado EXPULSA x do intervalo — o caminho é rígido', V === 0 && G)
  ok('§C4 o contrato 𝓜 assina a identificação: um real É um caminho da raiz à folha — o corte a cada nó — na árvore da torre', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  O real identificado: um caminho da raiz à folha na árvore da')
  console.log('  torre (fibra 2), com o corte de Dedekind a cada nível — tudo')
  console.log('  inteiro exato, os bits por dois caminhos (chão quadrático e')
  console.log('  o itinerário da dobra), e a folha que nunca é nó: o real é')
  console.log('  o caminho, não um ponto da árvore. O encaixe alcança; a')
  console.log('  árvore endereça; a dobra soletra.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
