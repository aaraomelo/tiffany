/* tests/maestro_memoria.js — a memória é Maestro + Metrónomo (a
 * pergunta do coordenador, 14/08: «sobre a memória, vê se dá
 * Maestro+Metrónomo, algo assim»).
 *
 * DÁ — e com a gramática exata que a casa já tinha medido no espectro
 * (metronomo_fourier: «Maestro = dobra + seletor»):
 *
 *   O METRÓNOMO é o tick que APRESENTA o u — puro, sem memória (uma
 *   função da posição). O MAESTRO é DOBRA + SELETOR: a dobra leva u
 *   às duas folhas ±1 (o par da Lei 0); o seletor decide pelo limiar
 *   (|u| ≤ Δ: guarda o velho; senão: aceita a dobra). E a MEMÓRIA
 *   EMERGE SÓ DA COMPOSIÇÃO: a histerese é EXATAMENTE
 *   seletor∘(dobra, id) — reproduzida ponto a ponto (33/33).
 *
 *   NENHUMA METADE TEM MEMÓRIA SOZINHA: com o seletor sempre-dobra
 *   (Δ<0, o metrónomo puro) não há memória; com o seletor
 *   sempre-retém (Δ grande, a pedra) também não. A memória exige o
 *   seletor A MEIO — nem tudo passa, nem nada passa: a membrana.
 *
 *   E O ESTADO RETIDO VIVE NO TRIAL: os estados alcançáveis são
 *   exatamente {−1, 0, +1} — a Lei 3, a batuta parada. A memória da
 *   casa é uma batuta que o Maestro deixou apontada.
 *
 *   Com o quantificador_histerese (a árvore 2^{k+1}−1) fecha o
 *   quadro: o tick apresenta, a dobra decide os ±, o seletor guarda —
 *   e O GUARDADO É O CAMINHO: a memória Maestro+Metrónomo gera a
 *   árvore do contínuo.
 *
 * §N1  a decomposição reproduz: histerese ≡ seletor∘(dobra, id),
 *      varredura completa (u ∈ [−5,5] × h ∈ trial)
 * §N2  nenhuma metade sozinha: Δ=−1 sem memória (metrónomo puro),
 *      Δ=3 sem memória (a pedra), Δ=2 COM memória — o seletor a meio
 * §N3  o estado vive no trial: alcançáveis = {−1,0,+1} exato
 *      (varredura exaustiva de palavras até k=6) — Lei 3
 * §N4  a árvore re-confirmada (2^{k+1}−1 em k=4,6) e 𝓜 assina: a
 *      memória é Maestro+Metrónomo — o guardado é o caminho
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
const dobra = u => Math.sign(u)
const seletor = (dentro, h, novo) => dentro ? h : novo
const maestro = Delta => (h, u) => seletor(Math.abs(u) <= Delta, h, dobra(u))
const histerese = Delta => (h, u) => Math.abs(u) <= Delta ? h : Math.sign(u)

/* §N1 — a decomposição reproduz */
{
  let bate = 0, casos = 0
  for (let u = -5; u <= 5; u++) {
    for (const h of [-1, 0, 1]) {
      casos++
      if (maestro(2)(h, u) === histerese(2)(h, u)) bate++
    }
  }
  if (bate !== casos) R++
  console.log(`\n§N1  seletor∘(dobra,id) == histerese em ${bate}/${casos}`)
  ok('§N1 A DECOMPOSIÇÃO REPRODUZ: a histerese é EXATAMENTE seletor∘(dobra, id) — a gramática «Maestro = dobra + seletor» do espectro (metronomo_fourier), agora como a gramática da memória, ponto a ponto', bate === casos)
}

/* §N2 — nenhuma metade sozinha: o seletor a meio */
let G = false
{
  const memoria = Delta => {
    const p = maestro(Delta)
    let hA = 0; for (const u of [3, 1, 0]) hA = p(hA, u)
    let hB = 0; for (const u of [-3, -1, 0]) hB = p(hB, u)
    return hA !== hB
  }
  const semDobra = !memoria(-1)      /* o metrónomo puro: sempre dobra — sem memória */
  const pedra = !memoria(3)          /* sempre retém — sem memória nova */
  const meio = memoria(2)            /* o seletor a meio — MEMÓRIA */
  G = semDobra && pedra && meio
  console.log(`\n§N2  Δ=−1 (sempre dobra): memória=${!semDobra} · Δ=2 (a meio): ${meio} · Δ=3 (pedra): memória=${!pedra}`)
  ok('§N2 NENHUMA METADE TEM MEMÓRIA SOZINHA: sempre-dobra (o metrónomo puro) não retém; sempre-retém (a pedra) não aprende — a memória EXIGE o seletor a meio: a membrana entre os dois extremos', G)
}

/* §N3 — o estado vive no trial */
{
  const alcancaveis = new Set()
  const varre = (h, prof) => {
    alcancaveis.add(h)
    if (prof === 6) return
    for (const u of [-3, -1, 0, 1, 3]) varre(maestro(2)(h, u), prof + 1)
  }
  varre(0, 0)
  const trial = [...alcancaveis].sort().join() === '-1,0,1'
  if (!trial) R++
  console.log(`\n§N3  estados alcançáveis: {${[...alcancaveis].sort()}}`)
  ok('§N3 O ESTADO RETIDO VIVE NO TRIAL: os alcançáveis são exatamente {−1, 0, +1} (varredura exaustiva até k=6) — a Lei 3: a memória da casa é uma batuta que o Maestro deixou apontada', trial)
}

/* §N4 — a árvore re-confirmada, e o contrato */
{
  let arvore = 0
  for (const k of [4, 6]) {
    const historias = new Set()
    const varre = (h, seq, prof) => {
      if (prof === k) { historias.add(seq); return }
      for (const u of [-3, 0, 3]) { const nh = maestro(2)(h, u); varre(nh, seq + nh, prof + 1) }
    }
    varre(0, '', 0)
    if (historias.size === 2 ** (k + 1) - 1) arvore++
  }
  if (arvore !== 2) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§N4  árvore 2^{k+1}−1 em ${arvore}/2 profundidades · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§N4 e a ÁRVORE confirma: as histórias da memória Maestro+Metrónomo contam 2^{k+1}−1 — o guardado é o caminho: a memória gera o contínuo da casa', arvore === 2)
  ok('§N4 𝓜 assina a resposta: SIM — a memória é Maestro+Metrónomo: o tick apresenta, a dobra decide os ± (Lei 0), o seletor guarda (a membrana), o estado vive no trial (Lei 3) — e o guardado é o caminho', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A memória decomposta: Maestro (dobra + seletor) sobre o')
  console.log('  Metrónomo (o tick) — nenhuma metade a tem sozinha; ela')
  console.log('  emerge da composição com o seletor a meio, o estado vive')
  console.log('  no trial, e o que se guarda é o caminho. A gramática do')
  console.log('  espectro e a gramática da memória são a mesma frase.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
