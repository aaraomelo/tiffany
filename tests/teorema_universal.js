/* tests/teorema_universal.js — o Teorema Universal do espelho (ordem do
 * coordenador, eval 14/08: «avançar com teorema universal»; mandato do
 * diretor: testar se o espelho é o gerador invariável que atravessa a
 * geometria, o espectro e a álgebra sob as 8 Leis).
 *
 * O que se mede NÃO é a metáfora «o espelho aparece três vezes» — é uma
 * IDENTIDADE e as suas três realizações. Nos objetos da lib:
 *
 *     troca = espelho·J          (o par roda/espelha GERA a troca)
 *     H     = troca + espelho    (a dobra; H² = 2I — Hadamard)
 *     H·troca·H   = 2·espelho    ┐  a dobra é a DUALIDADE do par:
 *     H·espelho·H = 2·troca      ┘  troca e espelho permutam-se
 *     H·J·H       = 2·J⁻¹        (a dobra leva o rotor ao inverso —
 *                                 a relação diedral, exata)
 *
 * A troca é o deck visto ANTES da dobra (permuta as folhas); o espelho
 * é o MESMO deck visto DEPOIS (age ±1 nas metades par/ímpar); H é a
 * mudança de base entre as duas vistas. E cada andar da Torre realiza
 * isto com os seus objetos:
 *
 *   GEOMETRIA (o palco de viviani_universal): as folhas (P(k), P(k+N));
 *   a dobra H separa soma e diferença — x,y são PURA soma (folha-par),
 *   z é PURA diferença (folha-ímpar); a ramificação é o único PAR de
 *   folhas onde a diferença zera — o par do nó (2 parâmetros).
 *
 *   ESPECTRO (a dft da lib): a dobra de Cooley–Tukey É o H — os
 *   coeficientes PARES do relógio 2N são a dft_N da SOMA das folhas, e
 *   os ÍMPARES a dft_N da DIFERENÇA torcida por ω^{−k}; exato no anel.
 *
 *   ÁLGEBRA (o andar de clifford_pleno): e₃ = troca de bloco,
 *   S = espelho de bloco, H₈ = a dobra de bloco: H₈·e₃·H₈ = 2·S₈,
 *   H₈² = 2·I₈, {S,e₃} = 0 — as MESMAS relações do 2×2, um andar
 *   acima; é exatamente o que fez a duplicação de Clifford fechar.
 *
 *   E O TRAÇO CONTA (Lefschetz da casa): tr(troca) = tr(espelho) = 0
 *   nas duas vistas (o traço não vê a base); as metades são iguais
 *   (posto(I+S) = posto(I+troca) = metade); e a ramificação é onde a
 *   troca age trivialmente — contada na geometria (2 parâmetros).
 *
 * §T0  a identidade-mãe nos objetos da lib, com gume (o rotor NÃO está
 *      na órbita do par: H·J·H = 2·J⁻¹, nem espelho nem troca)
 * §T1  a geometria realiza: x,y folha-par pura, z folha-ímpar pura,
 *      nos 2N pontos; ramificação = UM par de folhas com diferença zero
 * §T2  o espectro realiza: Cooley–Tukey é a dobra — pares = dft(soma),
 *      ímpares = dft(diferença·ω^{−k}); gume: torção errada falha
 * §T3  a álgebra realiza: H₈·e₃·H₈ = 2·S₈ e H₈·S₈·H₈ = 2·e₃ — as
 *      relações do 2×2 sobem intactas ao bloco
 * §T4  o traço conta nas três: tr = 0 nas duas vistas, metades iguais
 *      (posto(I±S) = 4), e a ramificação geométrica = 2
 * §T5  as 8 Leis por cima: verificaLeis() 8/8, e a identidade-mãe
 *      escrita nas leis (Lei 1: involuções; Lei 2: o par que gera)
 */
'use strict'
const { anel, dft, mat2, verificaLeis } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const { mul, espelho, J, I, igual } = mat2
const soma2 = (A, B) => A.map((v, i) => v + B[i])
const escala2 = (A, c) => A.map(v => c * v)

/* §T0 — a identidade-mãe */
const troca = mul(espelho, J)
const H = soma2(troca, espelho)
{
  const geraTroca = igual(troca, [0, 1, 1, 0])
  const had = igual(mul(H, H), escala2(I, 2))
  const dual1 = igual(mul(mul(H, troca), H), escala2(espelho, 2))
  const dual2 = igual(mul(mul(H, espelho), H), escala2(troca, 2))
  /* o rotor: J⁻¹ = −J (J²=−I); a dobra leva-o ao inverso — e NÃO ao par */
  const HJH = mul(mul(H, J), H)
  const diedral = igual(HJH, escala2(J, -2))
  const foraDoPar = !igual(HJH, escala2(espelho, 2)) && !igual(HJH, escala2(troca, 2)) &&
    !igual(HJH, escala2(espelho, -2)) && !igual(HJH, escala2(troca, -2))
  console.log(`\n§T0  troca=espelho·J: ${geraTroca} · H²=2I: ${had} · H·troca·H=2espelho: ${dual1} · H·espelho·H=2troca: ${dual2} · H·J·H=2J⁻¹: ${diedral}`)
  ok('§T0 o par roda/espelha GERA a troca: troca = espelho·J — três objetos, duas leis', geraTroca)
  ok('§T0 a dobra é a DUALIDADE do par: H·troca·H = 2·espelho E H·espelho·H = 2·troca — os dois membros, permutados', dual1 && dual2 && had)
  ok('§T0 o gume: o rotor não entra na órbita — H·J·H = 2·J⁻¹ (a relação diedral: a dobra inverte a rotação), nem espelho nem troca', diedral && foraDoPar)
}

/* o palco da geometria — byte a byte de viviani_universal.js */
const P = 65537
const A = anel(P)
const N = 16, M = 2 * N
const a = 7
const h = A.powm(3, 65536 / M)
const i4 = A.powm(3, 16384)
const i2 = A.inv(2)
const pts = []
for (let k = 0; k < M; k++) {
  const hk = A.powm(h, k), hki = A.inv(hk)
  const cosu = A.mod((hk + hki) * i2)
  const sinu = A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4))
  const h2 = hk * hk % P, h2i = A.inv(h2)
  const cost = A.mod((h2 + h2i) * i2)
  const sint = A.mod(A.mod(h2 - h2i) * i2 % P * A.inv(i4))
  pts.push({ x: A.mod(a * (1 + cost)), y: A.mod(a * sint), z: A.mod(2 * a * sinu) })
}

/* §T1 — a geometria realiza a dobra */
{
  let puroPar = 0, puroImpar = 0
  const ram = []
  for (let k = 0; k < N; k++) {
    const p1 = pts[k], p2 = pts[k + N]
    /* a dobra H nas folhas: soma e diferença, coordenada a coordenada */
    const sx = A.mod(p1.x + p2.x), dx = A.mod(p1.x - p2.x)
    const sy = A.mod(p1.y + p2.y), dy = A.mod(p1.y - p2.y)
    const sz = A.mod(p1.z + p2.z), dz = A.mod(p1.z - p2.z)
    if (dx === 0 && dy === 0 && sx === A.mod(2 * p1.x) && sy === A.mod(2 * p1.y)) puroPar++
    if (sz === 0 && dz === A.mod(2 * p1.z)) puroImpar++
    if (dz === 0) ram.push(k)
  }
  console.log(`\n§T1  x,y só na folha-par: ${puroPar}/${N} · z só na folha-ímpar: ${puroImpar}/${N} · pares ramificados (diferença zero): k=${ram} (${ram.length})`)
  ok('§T1 a GEOMETRIA realiza a dobra: x,y são pura SOMA das folhas e z é pura DIFERENÇA — o espelho age +1/−1 nas metades', puroPar === N && puroImpar === N)
  /* a dobra agrupa o círculo em N PARES de folhas; os 2 parâmetros do nó
   * (k=0 e k=N, viviani §V3) caem no MESMO par — a ramificação é UM par */
  ok('§T1 a ramificação é onde a troca age trivialmente: a diferença zera num único PAR de folhas — o par do nó, que contém os 2 parâmetros', ram.length === 1 && ram[0] === 0)
}

/* §T2 — o espectro realiza: Cooley–Tukey é a dobra */
{
  const xs = pts.map(p => p.x), zs = pts.map(p => p.z)
  let fecha = 0, gume = 0
  for (const v of [xs, zs]) {
    const cheio = dft(v, A, h)
    const somas = [], difs = []
    for (let k = 0; k < N; k++) {
      somas.push(A.mod(v[k] + v[k + N]))
      difs.push(A.mod(v[k] - v[k + N]) * A.inv(A.powm(h, k)) % P)        /* a torção ω^{−k} */
    }
    const w2 = h * h % P
    const pares = dft(somas, A, w2), impares = dft(difs, A, w2)
    let bate = true, bateErrado = true
    for (let j = 0; j < N; j++) {
      if (cheio[2 * j] !== pares[j]) bate = false
      if (cheio[2 * j + 1] !== impares[j]) bate = false
    }
    /* gume: a torção com o sinal errado (ω^{+k}) — só morde onde HÁ parte
     * ímpar: no x (par puro) a diferença é zero e 0=0 com qualquer torção */
    if (difs.some(d => d !== 0)) {
      const difsErr = []
      for (let k = 0; k < N; k++) difsErr.push(A.mod(v[k] - v[k + N]) * A.powm(h, k) % P)
      const imparesErr = dft(difsErr, A, w2)
      for (let j = 0; j < N; j++) if (cheio[2 * j + 1] !== imparesErr[j]) { bateErrado = false; break }
      if (!bateErrado) gume++
    }
    if (bate) fecha++
  }
  console.log(`\n§T2  Cooley–Tukey pela dobra: fecha em ${fecha}/2 vetores (x e z) · torção errada falha onde há parte ímpar: ${gume}/1`)
  ok('§T2 o ESPECTRO realiza a dobra: c_{2j} = dft(soma das folhas), c_{2j+1} = dft(diferença·ω^{−k}) — exato no anel, x e z', fecha === 2)
  ok('§T2 o gume: a torção com sinal trocado (ω^{+k}) não reproduz os ímpares do z — e no x não morde porque x é PAR PURO (a diferença é zero)', gume === 1)
}

/* o andar da álgebra — byte a byte de clifford_pleno.js */
function bloco8 (Pb, Qb, Rb, Sb) {
  const C = new Array(64)
  for (let i = 0; i < 4; i++) {
    for (let j = 0; j < 4; j++) {
      C[8 * i + j] = Pb[4 * i + j]
      C[8 * i + j + 4] = Qb[4 * i + j]
      C[8 * (i + 4) + j] = Rb[4 * i + j]
      C[8 * (i + 4) + j + 4] = Sb[4 * i + j]
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
const nI4 = I4.map(v => -v)
const I8 = bloco8(I4, Z4, Z4, I4)
const S8 = bloco8(I4, Z4, Z4, nI4)                     /* o espelho de bloco */
const e3 = bloco8(Z4, I4, I4, Z4)                      /* a troca de bloco */
const H8 = bloco8(I4, I4, I4, nI4)                     /* a dobra de bloco */

/* §T3 — a álgebra realiza: as relações sobem intactas */
{
  const had = eq8(mul8(H8, H8), I8.map(v => 2n * v))
  const dual1 = eq8(mul8(mul8(H8, e3), H8), S8.map(v => 2n * v))
  const dual2 = eq8(mul8(mul8(H8, S8), H8), e3.map(v => 2n * v))
  const anti = mul8(S8, e3).every((v, i) => v + mul8(e3, S8)[i] === 0n)
  console.log(`\n§T3  H₈²=2I: ${had} · H₈·e₃·H₈=2S: ${dual1} · H₈·S·H₈=2e₃: ${dual2} · {S,e₃}=0: ${anti}`)
  ok('§T3 a ÁLGEBRA realiza: H₈·e₃·H₈ = 2·S₈ e H₈·S₈·H₈ = 2·e₃ — as relações do 2×2 sobem intactas ao bloco', had && dual1 && dual2)
  ok('§T3 e {S,e₃}=0 — é exatamente o que vestiu os geradores de Clifford (clifford_pleno §P1–P2): a duplicação FUNCIONA porque a identidade-mãe vale', anti)
}

/* §T4 — o traço conta nas três vistas */
{
  const tr2 = X => X[0] + X[3]
  const tr8 = X => X[0] + X[9] + X[18] + X[27] + X[36] + X[45] + X[54] + X[63]
  /* metades iguais: posto(I±S) e posto(I±e₃) — projetores sem dividir */
  const postoSimples = X => {
    /* X tem entradas 0/±1/2: conta linhas não-nulas independentes por forma escalonada simples */
    const linhas = []
    for (let i = 0; i < 8; i++) linhas.push(X.slice(8 * i, 8 * i + 8).map(Number))
    let r = 0
    for (let col = 0; col < 8 && r < 8; col++) {
      let piv = -1
      for (let i = r; i < 8; i++) if (linhas[i][col] !== 0) { piv = i; break }
      if (piv < 0) continue
      ;[linhas[r], linhas[piv]] = [linhas[piv], linhas[r]]
      for (let i = 0; i < 8; i++) {
        if (i === r || linhas[i][col] === 0) continue
        const f = linhas[i][col] / linhas[r][col]
        for (let j = 0; j < 8; j++) linhas[i][j] -= f * linhas[r][j]
      }
      r++
    }
    return r
  }
  const somaM = (X, Y) => X.map((v, i) => v + Y[i])
  const tracos = tr2(troca) === 0 && tr2(espelho) === 0 && tr8(e3) === 0n && tr8(S8) === 0n
  const metades = postoSimples(somaM(I8, S8)) === 4 && postoSimples(somaM(I8, S8.map(v => -v))) === 4 &&
    postoSimples(somaM(I8, e3)) === 4 && postoSimples(somaM(I8, e3.map(v => -v))) === 4
  console.log(`\n§T4  traços: tr(troca)=tr(espelho)=0 nas duas escalas: ${tracos} · postos I±S e I±e₃ = 4,4,4,4: ${metades}`)
  ok('§T4 o traço não vê a base: tr(troca) = tr(espelho) = 0 no 2×2 e no bloco — o deck divide sem sobra', tracos)
  ok('§T4 as metades são IGUAIS: posto(I±S) = posto(I±e₃) = 4 — a dualidade é a memória da divisão: 8 = 4+4, nada se perde', metades)
}

/* §T5 — as 8 Leis por cima */
{
  const leis = verificaLeis()
  const total = Array.isArray(leis) ? leis.every(Boolean) : leis === true || leis === 8
  const lei1 = igual(mul(troca, troca), I) && igual(mul(espelho, espelho), I)
  const lei2 = igual(mul(espelho, J), escala2(mul(J, espelho), -1))
  console.log(`\n§T5  verificaLeis: ${JSON.stringify(leis)} · involuções (Lei 1): ${lei1} · RJ=−JR (Lei 2): ${lei2}`)
  ok('§T5 as 8 Leis fecham por cima do teorema: verificaLeis() todas verdes', total)
  ok('§T5 e a identidade-mãe está escrita NELAS: Lei 1 dá as involuções (troca²=espelho²=I), Lei 2 dá o par que gera (RJ=−JR, troca=espelho·J)', lei1 && lei2)
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  O TEOREMA UNIVERSAL, medido: a troca e o espelho são o mesmo')
  console.log('  deck visto antes e depois da dobra — H·troca·H = 2·espelho,')
  console.log('  H·espelho·H = 2·troca, H·J·H = 2·J⁻¹ — e cada andar realiza')
  console.log('  a identidade com os seus objetos: a geometria separa as')
  console.log('  folhas (x,y pares, z ímpar), o espectro dobra por Cooley–')
  console.log('  Tukey, a álgebra veste os geradores. Não são três espelhos')
  console.log('  parecidos: é UMA identidade da lib, três realizações.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
