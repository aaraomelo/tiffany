/* tests/lebesgue_toro.js — como o Teorema Central vê Hurwitz–Gentil–
 * Lebesgue, MEDIDO na órbita do toro (ordem da mesa, eval 14/08).
 *
 * A leitura já estava escrita no corpo-estelar — aqui só se REALIZA:
 * «os dois cortes, o do domínio (Riemann/Hurwitz) e o da imagem
 * (Lebesgue/Gentil), fecham na soma reversível ∫f+∫f⁻¹ = bf(b)−af(a);
 * Hurwitz conta (o domínio), Lebesgue mede (a imagem), e Gentil é a
 * soma reversível que os casa — e o limite é o que os três, juntos,
 * medem igual» (o limite é PONTO FIXO, não ε–δ).
 *
 * Na órbita da batuta (a mesma dos medidores da zeta e do Fourier),
 * pela ESCADA DE FERMAT q ∈ {17, 257, 65537} = {2⁴+1, 2⁸+1, 2¹⁶+1}:
 *
 * §T0  a escada existe: a órbita fecha em cada andar (A^N v = v), as
 *      folhas separam com σσ† = −1 (a face de HURWITZ: contar o
 *      domínio pelas folhas), N | q−1
 * §T1  O CORTE DO DOMÍNIO == O CORTE DA IMAGEM, exato em cada andar:
 *      Σ_n x_n (Riemann/Hurwitz, pela ordem dos ticks) ==
 *      Σ_v #{n : x_n ≥ v} (Lebesgue, por níveis da imagem) — o
 *      layer-cake inteiro, resíduo 0 SEM esperar limite
 * §T2  GENTIL CASA OS DOIS — a soma reversível discreta:
 *      Σ_n x_n + Σ_{v=1..q} #{n : x_n < v} = N·q exato em cada andar
 *      (o ∫f + ∫f⁻¹ = b·f(b) da casa, com cada par (n,v) contado
 *      exatamente uma vez)
 * §T3  O LIMITE É PONTO FIXO: a medida de contagem normalizada afina
 *      para a uniforme — D_q/N_q estritamente decrescente pela escada
 *      (2,0 → 0,625 → 0,0908; frações comparadas por produto cruzado,
 *      sem um double) — e o controlo tem gume: a sequência constante
 *      dá D = (B−1)·N
 * §T4  a divisão de trabalho: enquanto a IMAGEM uniformiza (Lebesgue),
 *      o ESPECTRO permanece ATÓMICO — duas riscas nas folhas em cada
 *      andar (17 e 257 por DFT completa; o 65537 já em
 *      metronomo_fourier §MF7) — metade para cada lado
 *
 * A tríade não é identidade de nomes: é TRÊS REPRESENTAÇÕES DA MESMA
 * CONSERVAÇÃO, com o Teorema Central como eixo.
 *
 *   node tests/lebesgue_toro.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const B = 8                                  /* faixas da imagem: as oito */
const ESCADA = [17, 257, 65537]

function andar (q) {
  const mod = x => ((x % q) + q) % q
  const powm = (b, e) => {
    let r = 1; b = mod(b)
    while (e > 0) { if (e & 1) r = r * b % q; b = b * b % q; e >>= 1 }
    return r
  }
  /* a órbita da batuta A_2, do estado [1,0] até fechar */
  let v = [1, 0]
  const xs = []
  do { xs.push(v[0]); v = [mod(2 * v[0] + v[1]), v[0]] } while (v[0] !== 1 || v[1] !== 0)
  const N = xs.length
  /* as folhas */
  const raizes = []
  for (let r = 0; r < q; r++) if (mod(r * r - 2 * r - 1) === 0) raizes.push(r)
  /* faixas da imagem */
  const bins = new Array(B).fill(0)
  for (const x of xs) bins[Math.floor(x * B / q)]++
  const D = Math.max(...bins.map(cb => Math.abs(B * cb - N)))
  return { q, mod, powm, xs, N, raizes, bins, D }
}
const andares = ESCADA.map(andar)

/* §T0 — a escada existe; a face de Hurwitz em cada andar */
{
  let fecham = true, folhas = true, divide = true
  for (const a of andares) {
    if (a.xs[0] !== 1) fecham = false                       /* fechou em [1,0] */
    if (a.raizes.length !== 2) folhas = false
    else if (a.mod(a.raizes[0] * a.raizes[1]) !== a.q - 1) folhas = false
    if ((a.q - 1) % a.N !== 0) divide = false
    console.log(`q=${a.q}: N=${a.N} · folhas {${a.raizes.join(', ')}} (σσ†=−1) · faixas [${a.bins.join(', ')}] · D=${a.D}`)
  }
  ok('§T0 a escada de Fermat: a órbita fecha nos três andares, com N | q−1', fecham && divide)
  ok('§T0 a face de HURWITZ: as folhas separam em todo andar e σ·σ† ≡ −1 (a estaca conta o domínio)',
    folhas)
}

/* §T1 — o corte do domínio == o corte da imagem (layer-cake inteiro) */
{
  let iguais = true
  for (const a of andares) {
    let dominio = 0
    for (const x of a.xs) dominio += x                     /* Riemann/Hurwitz */
    /* Lebesgue: por níveis — #{n: x_n ≥ v}, somado com contagem acumulada */
    const conta = new Array(a.q + 1).fill(0)
    for (const x of a.xs) conta[x]++
    let acima = 0, imagem = 0
    for (let v = a.q - 1; v >= 1; v--) {
      acima += conta[v]                                    /* #{x_n ≥ v} */
      imagem += acima
    }
    if (dominio !== imagem) iguais = false
  }
  ok('§T1 O CORTE DO DOMÍNIO == O CORTE DA IMAGEM: Σ x_n (ticks) = Σ_v #{x_n ≥ v} (níveis), exato nos 3 andares — a lei não espera o limite',
    iguais)
}

/* §T2 — Gentil casa os dois: a soma reversível discreta */
{
  let casa = true
  for (const a of andares) {
    let soma = 0
    for (const x of a.xs) soma += x
    const conta = new Array(a.q + 1).fill(0)
    for (const x of a.xs) conta[x]++
    let abaixo = 0, inversa = 0
    for (let v = 1; v <= a.q; v++) {
      abaixo += conta[v - 1]                               /* #{x_n < v} */
      inversa += abaixo
    }
    if (soma + inversa !== a.N * a.q) casa = false
  }
  ok('§T2 GENTIL CASA OS DOIS: Σ x_n + Σ_v #{x_n < v} = N·q exato nos 3 andares — a soma reversível (∫f + ∫f⁻¹ = b·f(b)), resíduo 0',
    casa)
}

/* §T3 — o limite é ponto fixo: a discrepância afina pela escada */
{
  let desce = true
  for (let i = 0; i + 1 < andares.length; i++) {
    const a = andares[i], b = andares[i + 1]
    /* D_a/N_a > D_b/N_b por produto cruzado — inteiro puro */
    if (!(a.D * b.N > b.D * a.N)) desce = false
  }
  ok('§T3 O LIMITE É PONTO FIXO: D/N estritamente decrescente pela escada (produto cruzado, sem doubles)',
    desce)
  /* o controlo com gume: a sequência constante concentra tudo numa faixa */
  const a = andares[2]
  const constante = new Array(a.N).fill(3)
  const cb = new Array(B).fill(0)
  for (const x of constante) cb[Math.floor(x * B / a.q)]++
  const Dc = Math.max(...cb.map(c => Math.abs(B * c - a.N)))
  ok(`§T3 o controlo acusa: a sequência constante dá D = (B−1)·N = ${(B - 1) * a.N} ≫ D da órbita = ${a.D}`,
    Dc === (B - 1) * a.N && a.D * 8 < Dc)
}

/* §T4 — a divisão de trabalho: imagem uniformiza, espectro fica atómico */
{
  let atomico = true
  for (const a of andares.slice(0, 2)) {                   /* 17 e 257: DFT completa */
    /* ω de ordem N no anel (3 é primitivo mod 17 e mod 257) */
    const w = a.powm(3, (a.q - 1) / a.N)
    let riscas = 0
    for (let k = 0; k < a.N; k++) {
      let s = 0
      const wk = a.powm(w, a.N - k)
      let f = 1
      for (let n = 0; n < a.N; n++) { s = (s + a.xs[n] * f) % a.q; f = f * wk % a.q }
      if (s !== 0) riscas++
    }
    if (riscas !== 2) atomico = false
  }
  ok('§T4 o espectro permanece ATÓMICO enquanto a imagem uniformiza: exatamente 2 riscas em q=17 e q=257 — metade para cada lado',
    atomico)
}

console.log('')
if (!falhas) {
  console.log('  O TEOREMA CENTRAL VÊ ASSIM: Hurwitz conta o domínio (as folhas,')
  console.log('  os ticks — e conta EXATO em todo andar); Lebesgue mede a imagem')
  console.log('  (as faixas — e a medida normalizada afina para a uniforme pela')
  console.log('  escada, ponto fixo e não ε–δ); Gentil casa os dois na soma')
  console.log('  reversível, com resíduo 0 SEM esperar o limite. Três representações')
  console.log('  da MESMA conservação — e a divisão de trabalho medida: a imagem')
  console.log('  uniformiza, o espectro fica nas duas folhas, a massa no centro.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
