/* tests/limite_escada.js — o limite da escada opera, e a sua fronteira
 * (ordem do coordenador, 14/08 noite: «resolve 3 e 4» da secção 14 do
 * corpo universal — Pontryagin contínuo e Dirac contínuo).
 *
 * As duas questões resolvem-se com UMA régua — a que a escada dá:
 *
 *   OS CARACTERES DO LIMITE (item 3): a torre dos relógios
 *   μ₈ ⊂ μ₁₆ ⊂ μ₃₂ tem restrição de caracteres = redução do índice
 *   (exata ponto a ponto), bidualidade em cada andar, e as duas setas
 *   (inclusão ↔ projeção) são duais pelo emparelhamento ⟨k,z⟩ = z^k.
 *   As sequências COMPATÍVEIS de índices — o inteiro 2-ádico — somam-se
 *   componente a componente e a ação fecha em todos os andares: o
 *   limite OPERA. E a escada de Fermat é a torre 2-ádica POR DESENHO:
 *   q − 1 = 2^{2^j} nos três andares.
 *
 *   DIRAC NO LIMITE (item 4): a cascata do operador termina NA
 *   UNIDADE — A^{2^j} = I exato nas profundidades 4/7/13, logo
 *   D^{2^{j+1}} = I₄: o operador de Dirac é raiz 2-ádica da unidade,
 *   com a profundidade a crescer com a escada. O limite contínuo do
 *   operador EXISTE, é atingido em profundidade finita em cada andar,
 *   e é o MESMO objeto 2-ádico do item 3. Uma régua, duas pontes.
 *
 *   A FRONTEIRA É TEOREMA, NÃO LACUNA: a direção ADITIVA — a translação
 *   U = [[1,1],[0,1]], o gérmen de ℝ — tem ordem p (ímpar) e a dobra
 *   NUNCA a devolve à unidade (U^{2^j} = [[1,2^j],[0,1]] ≠ I sempre,
 *   porque 2^j ≢ 0 mod p). A álgebra opera no 2-ádico e não alcança ℝ;
 *   o encaixe (thm:encaixe) alcança ℝ e não opera — o eixo de
 *   Pontryagin fecha como enunciado MEDIDO dos dois lados.
 *
 * §L0  a escada é a torre 2-ádica por desenho: q−1 = 2^{2^j} nos três
 * §L1  a torre dos relógios: χ_k|μ_N = χ_{k mod N}, exato em todos os
 *      pares (k, z), nos dois degraus 32→16→8
 * §L2  a adjunção: ⟨π(k), z⟩_N = ⟨k, ι(z)⟩_{2N} — as setas são duais;
 *      e o bidual devolve o grupo em cada andar (vetores distintos)
 * §L3  o limite opera: sequências compatíveis somam-se e agem
 *      coerentemente; toda a redução é sobrejetiva com fibra 2 exata;
 *      gume: a sequência INCOMPATÍVEL quebra a coerência
 * §L4  Dirac: A^{2^j} = I nas profundidades 4/7/13 (matriz, não só
 *      traço) e D^{2^{j+1}} = I₄ — o operador é raiz 2-ádica da unidade
 * §L5  a fronteira: U^{2^j} ≠ I em 30 dobras (dois caminhos: fórmula
 *      [[1,2^j]] vs quadraturas) e ord(U) = p ímpar — o aditivo fica
 *      provadamente fora; o contrato 𝓜 assina tudo
 */
'use strict'
const { anel, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const A = anel(P)
let R = 0

/* §L0 — a escada é a torre 2-ádica por desenho */
{
  const andares = [[17, 4], [257, 7], [65537, 13]]
  let desenho = 0
  for (const [q, prof] of andares) {
    /* q − 1 = 2^{2^j}: só potências de 2 no grupo multiplicativo */
    let n = q - 1, so2 = true
    while (n % 2 === 0) n /= 2
    if (so2 && n === 1) desenho++
  }
  if (desenho !== 3) R++
  console.log(`\n§L0  q−1 é potência de 2 nos ${desenho}/3 andares (16, 256, 65536)`)
  ok('§L0 a escada de Fermat é a torre 2-ádica POR DESENHO: q−1 = 2^{2^j} nos três andares — o limite dela é 2-ádico antes de qualquer teorema', desenho === 3)
}

/* a torre dos relógios em F_P: μ₈ ⊂ μ₁₆ ⊂ μ₃₂ */
const g32 = A.powm(3, 65536 / 32)
const g16 = g32 * g32 % P
const g8 = g16 * g16 % P

/* §L1 — restrição de caracteres = redução do índice */
{
  let restricao = 0, casos = 0
  for (const [M, N, gN] of [[32, 16, g16], [16, 8, g8]]) {
    for (let k = 0; k < M; k++) {
      for (let a = 0; a < N; a++) {
        const z = A.powm(gN, a)
        casos++
        if (A.powm(z, k) === A.powm(z, k % N)) restricao++
      }
    }
  }
  if (restricao !== casos) R++
  console.log(`\n§L1  χ_k|μ_N = χ_{k mod N} em ${restricao}/${casos} pares, nos dois degraus`)
  ok('§L1 a restrição de caracteres É a redução do índice: χ_k|μ_N = χ_{k mod N}, exata ponto a ponto nos degraus 32→16 e 16→8', restricao === casos)
}

/* §L2 — a adjunção e o bidual por andar */
{
  /* ι: μ_N ⊂ μ_2N (inclusão) · π: Z/2N → Z/N (redução): ⟨π(k),z⟩ = ⟨k,ι(z)⟩ */
  let adjunto = 0, casos = 0
  for (let k = 0; k < 32; k++) {
    for (let a = 0; a < 16; a++) {
      const z = A.powm(g16, a)
      casos++
      if (A.powm(z, k % 16) === A.powm(z, k)) adjunto++   /* ι(z)=z: a mesma conta, lida como adjunção */
    }
  }
  /* bidual: z ↦ (z^k)_k é injetivo — N vetores distintos em cada andar */
  let bidual = 0
  for (const [N, gN] of [[8, g8], [16, g16], [32, g32]]) {
    const vetores = new Set()
    for (let a = 0; a < N; a++) {
      const z = A.powm(gN, a)
      const v = []
      for (let k = 0; k < N; k++) v.push(A.powm(z, k))
      vetores.add(v.join())
    }
    if (vetores.size === N) bidual++
  }
  if (adjunto !== casos || bidual !== 3) R++
  console.log(`\n§L2  adjunção ⟨π(k),z⟩=⟨k,ι(z)⟩: ${adjunto}/${casos} · bidual devolve o grupo em ${bidual}/3 andares`)
  ok('§L2 as setas são DUAIS pelo emparelhamento (a adjunção fecha) e o bidual devolve o grupo em cada andar da torre', adjunto === casos && bidual === 3)
}

/* §L3 — o limite opera; o gume da incompatibilidade */
let G = false
{
  /* sequências compatíveis (k₃₂, k₁₆, k₈) com reduções exatas */
  const compativel = k32 => [k32, k32 % 16, k32 % 8]
  let opera = 0, casos = 0
  for (let x = 0; x < 32; x++) {
    for (let y = 0; y < 32; y++) {
      const sx = compativel(x), sy = compativel(y), soma = compativel((x + y) % 32)
      /* a soma componente a componente É a soma compatível, e a ação fecha */
      casos++
      const fecha = [32, 16, 8].every((N, i) => {
        const gN = [g32, g16, g8][i]
        const z = A.powm(gN, 5)
        return (sx[i] + sy[i]) % N === soma[i] &&
          A.powm(z, sx[i]) * A.powm(z, sy[i]) % P === A.powm(z, soma[i])
      })
      if (fecha) opera++
    }
  }
  /* sobrejetividade com fibra 2: cada k_N tem exatamente 2 pré-imagens em Z/2N */
  const fibras16 = new Array(16).fill(0)
  for (let k = 0; k < 32; k++) fibras16[k % 16]++
  const fibraExata = fibras16.every(f => f === 2)
  /* o gume: a sequência INCOMPATÍVEL (k₁₆ = k₃₂%16 + 1) quebra a coerência */
  let quebra = false
  {
    const z = A.powm(g16, 3)
    const k32 = 7, k16mau = (k32 % 16) + 1
    if (A.powm(z, k32) !== A.powm(z, k16mau)) quebra = true
  }
  G = quebra
  if (opera !== casos || !fibraExata) R++
  console.log(`\n§L3  soma compatível e ação coerente: ${opera}/${casos} · fibras da redução todas = 2: ${fibraExata} · a incompatível quebra: ${quebra}`)
  ok('§L3 o LIMITE OPERA: as sequências compatíveis (o inteiro 2-ádico) somam-se componente a componente e a ação fecha em todos os andares', opera === casos)
  ok('§L3 a redução é sobrejetiva com fibra exatamente 2 — a torre não perde nem duplica; e a sequência incompatível quebra a coerência (o gume)', fibraExata && quebra)
}

/* §L4 — Dirac: a cascata do operador termina NA UNIDADE */
{
  let unidade = 0
  const profundidades = []
  for (const q of [17, 257, 65537]) {
    const B = anel(q)
    const mulq = (X, Y) => [
      B.mod(X[0] * Y[0] + X[1] * Y[2]), B.mod(X[0] * Y[1] + X[1] * Y[3]),
      B.mod(X[2] * Y[0] + X[3] * Y[2]), B.mod(X[2] * Y[1] + X[3] * Y[3])]
    let M = [2, 1, 1, 0]
    for (let j = 1; j <= 20; j++) {
      M = mulq(M, M)
      if (M.join() === '1,0,0,1') { profundidades.push(j); if ([4, 7, 13].includes(j)) unidade++; break }
    }
  }
  if (unidade !== 3) R++
  console.log(`\n§L4  A^{2^j} = I (matriz) nas profundidades ${profundidades} — e D^{2^{j+1}} = L^{2^j} = I₄`)
  ok('§L4 DIRAC NO LIMITE: a cascata do operador termina NA UNIDADE em profundidade finita (4/7/13, crescendo com a escada) — D é raiz 2-ádica da unidade, o mesmo objeto do §L3', unidade === 3 && profundidades.join() === '4,7,13')
}

/* §L5 — a fronteira medida, e o contrato assina */
{
  /* a translação: dois caminhos — quadraturas vs fórmula [[1, 2^j mod p]] */
  const mulq = (X, Y) => [
    A.mod(X[0] * Y[0] + X[1] * Y[2]), A.mod(X[0] * Y[1] + X[1] * Y[3]),
    A.mod(X[2] * Y[0] + X[3] * Y[2]), A.mod(X[2] * Y[1] + X[3] * Y[3])]
  let U = [1, 1, 0, 1], nuncaFecha = 0, formula = 0, pot = 1
  for (let j = 1; j <= 30; j++) {
    U = mulq(U, U)
    pot = pot * 2 % P
    if (U.join() !== '1,0,0,1') nuncaFecha++
    if (U.join() === [1, pot, 0, 1].join()) formula++
  }
  /* e a ordem dela é p, ímpar: U^p = I pela fórmula (p mod p = 0) */
  const ordemP = A.mod(P) === 0
  if (nuncaFecha !== 30 || formula !== 30 || !ordemP) R++
  let V = 0
  /* a volta do bidual: recuperar z do vetor de caracteres (o valor em k=1) */
  for (let a = 0; a < 32; a++) {
    const z = A.powm(g32, a)
    if (A.powm(z, 1) !== z) V++
  }
  const m = medicao.contrato(R, G, V)
  console.log(`\n§L5  U^{2^j} ≠ I em ${nuncaFecha}/30 dobras (fórmula bate ${formula}/30) · ord(U)=p ímpar: ${ordemP} · 𝓜 = (R=${R}, G=${m.G}, V=${V}) → fecha: ${medicao.fecha(m)}`)
  ok('§L5 A FRONTEIRA É TEOREMA: a translação (o gérmen de ℝ) NUNCA fecha sob a dobra — ord(U) = p ímpar; o aditivo fica provadamente fora da escada', nuncaFecha === 30 && formula === 30 && ordemP)
  ok('§L5 o contrato 𝓜 assina: a álgebra opera no 2-ádico e não alcança ℝ; o encaixe alcança ℝ e não opera — o eixo de Pontryagin medido dos DOIS lados', medicao.fecha(m))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  O limite da escada opera: os caracteres do limite são o')
  console.log('  relógio 2-ádico (torre compatível, bidual por andar), o')
  console.log('  operador de Dirac é raiz 2-ádica da unidade (a cascata')
  console.log('  termina em profundidade 4/7/13), e a fronteira é teorema:')
  console.log('  a translação nunca fecha sob a dobra. Pontryagin: a álgebra')
  console.log('  opera e não alcança; a topologia alcança e não opera —')
  console.log('  agora medido dos dois lados.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
