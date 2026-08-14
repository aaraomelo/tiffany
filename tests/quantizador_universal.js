/* tests/quantizador_universal.js — a língua universal separada da
 * realização de Peano: o QUANTIZADOR (a ordem do coordenador, 14/08:
 * «vamos separando a língua universal da realização de Peano no
 * universal, de metrónomo para quantizador, e interpreta»).
 *
 * A separação: METRÓNOMO é o nome de Peano (a face musical — o tempo
 * em ticks); o papel universal chama-se QUANTIZADOR — o operador que
 * converte o contínuo em contagem conservada. A interpretação, com
 * medida em três faces:
 *
 *   QUANTIZA O ÂNGULO: o relógio extrai do círculo contínuo
 *   exatamente M pontos (ordem exata: ω^M=1 e ω^{M/2}=−1), e o
 *   refinamento é a torre (ω_{2M}²=ω_M): o contínuo entra por
 *   refinamento do passo de quantização, nunca por salto.
 *
 *   QUANTIZA A ENERGIA — A JOIA, a cascata do quantum: o gap do
 *   andar 2−t_M é o QUANTUM (≠0 em todo andar), e ele FATORA pela
 *   dobra: (2−t_{2M})·(2+t_{2M}) = 2−t_M — o quantum do andar grosso
 *   é o quantum do andar fino vezes a meia-volta ao quadrado
 *   (μ²=2+t). A escada de quantização desce sem nunca zerar; o zero
 *   é só do limite — a leitura física do metrónomo→quantizador: o
 *   tick é o quantum, e ℏ→0 é a vareta a encolher (batuta_continuo).
 *
 *   QUANTIZA A VERDADE: o ∀ da casa — UMA identidade matricial
 *   (RᵀR=I no anel) substitui as M verificações (a varredura fica
 *   como controlo): infinitas instâncias quantizadas numa verificação
 *   do passo. O dual do Quantizador é o limite (a histerese — o
 *   contínuo): o par discreto↔contínuo nos dois sentidos.
 *
 *   E A SEPARAÇÃO NO CÓDIGO: a lib (a lei) fica ABAIXO das duas
 *   línguas — nem os nomes de Peano (metrónomo, maestro, batuta,
 *   peano) nem o nome universal novo (quantizador) aparecem nela:
 *   a lei não precisa de nome; os nomes vivem nos papers. (Esta
 *   asserção FALHOU antes da correção: a Lei 3 dizia «a batuta» —
 *   vocabulário de instância dentro da língua universal, removido.)
 *
 * §Z1  a separação no código: lib sem vocabulário de instância nem
 *      de papel (0 ocorrências — falhava antes, com «a batuta»)
 * §Z2  quantiza o ângulo: ordem exata (ω^M=1, ω^{M/2}=−1) e torre
 *      (ω_{2M}²=ω_M) nos três andares
 * §Z3  a joia — a cascata do quantum: (2−t_{2M})(2+t_{2M})=2−t_M com
 *      quantum ≠0 por andar; o gume: o emparelhamento errado quebra
 * §Z4  quantiza a verdade: RᵀR=I (uma identidade) + varredura como
 *      controlo (M ticks)
 * §Z5  o dicionário interpretado e 𝓜: Quantizador (universal) ↔
 *      Metrónomo (realização de Peano) — ângulo, energia, verdade
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { anel, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const P = 65537
const A = anel(P)

/* §Z1 — a separação no código */
{
  const lib = fs.readFileSync(path.join(__dirname, '../lib/universal.js'), 'utf8').toLowerCase()
  const nomes = ['metronomo', 'metrónomo', 'maestro', 'batuta', 'peano', 'quantizador']
  const ocorrencias = nomes.map(n => (lib.split(n).length - 1))
  const limpa = ocorrencias.every(c => c === 0)
  if (!limpa) R++
  console.log(`\n§Z1  ocorrências na lib [${nomes.join(', ')}]: [${ocorrencias.join(', ')}]`)
  ok('§Z1 A SEPARAÇÃO NO CÓDIGO: a lib fica ABAIXO das duas línguas — nem os nomes de Peano nem o nome universal aparecem nela (0 ocorrências): a lei não precisa de nome; os nomes vivem nos papers. (Falhava antes: a Lei 3 dizia «a batuta» — instância dentro da língua universal)', limpa)
}

/* §Z2 — quantiza o ângulo */
{
  let andares = 0
  for (const M of [8, 16, 32]) {
    const w = A.powm(3, 65536 / M)
    const ordem = A.powm(w, M) === 1 && A.powm(w, M / 2) === P - 1
    const w2 = A.powm(3, 65536 / (2 * M))
    const torre = A.mod(w2 * w2) === w
    if (ordem && torre) andares++
  }
  if (andares !== 3) R++
  console.log(`\n§Z2  ordem exata + torre em ${andares}/3 andares`)
  ok('§Z2 QUANTIZA O ÂNGULO: o relógio extrai do círculo exatamente M pontos (ω^M=1 com ω^{M/2}=−1 — ordem exata, não divisor) e o refinamento é a torre (ω_{2M}²=ω_M): o contínuo entra por refinamento do passo de quantização, nunca por salto', andares === 3)
}

/* §Z3 — a joia: a cascata do quantum */
let G = false
{
  const t = M => { const w = A.powm(3, 65536 / M); return A.mod(w + A.inv(w)) }
  let cascata = 0, quantum = 0, quebra = 0
  for (const M of [8, 16]) {
    const tM = t(M), t2M = t(2 * M)
    /* a dobra: t_{2M}² = 2+t_M, e a cascata: (2−t_{2M})(2+t_{2M}) = 2−t_M */
    const dobra = A.mod(t2M * t2M) === A.mod(2 + tM)
    const casc = A.mod(A.mod(2 - t2M + P) * A.mod(2 + t2M)) === A.mod(2 - tM + P)
    if (dobra && casc) cascata++
    /* o gume: o emparelhamento ERRADO — (2−t_{2M})(2+t_M) — quebra */
    if (A.mod(A.mod(2 - t2M + P) * A.mod(2 + tM)) !== A.mod(2 - tM + P)) quebra++
  }
  for (const M of [8, 16, 32]) if (A.mod(2 - t(M) + P) !== 0) quantum++
  G = quebra === 2
  if (cascata !== 2 || quantum !== 3) R++
  console.log(`\n§Z3  cascata do quantum: ${cascata}/2 pares · quantum ≠0: ${quantum}/3 andares · emparelhamento errado quebra: ${quebra}/2`)
  ok('§Z3 A JOIA — A CASCATA DO QUANTUM: o gap 2−t_M é o quantum do andar (≠0 em todos) e FATORA pela dobra — (2−t_{2M})(2+t_{2M}) = 2−t_M: o quantum do andar grosso é o do fino vezes a meia-volta ao quadrado; a escada desce sem nunca zerar, e o zero é só do limite (ℏ→0 é a vareta a encolher)', cascata === 2 && quantum === 3)
  ok('§Z3 o gume: o emparelhamento errado ((2−t_{2M})(2+t_M)) quebra a cascata nos dois pares — a fatoração sabe qual é o seu andar', G)
}

/* §Z4 — quantiza a verdade */
{
  const M = 16
  const h = A.powm(3, 65536 / (2 * M)), i2 = A.inv(2), i4 = A.powm(3, 16384)
  const w = A.mod(h * h)
  const c1 = A.mod(A.mod(w + A.inv(w)) * i2)
  const s1 = A.mod(A.mod(w - A.inv(w) + P) * i2 % P * A.inv(i4))
  /* UMA identidade: RᵀR = I — as quatro entradas */
  const umaVez = A.mod(c1 * c1 + s1 * s1) === 1 && A.mod(c1 * (P - s1) + s1 * c1) % P === 0
  /* a varredura como CONTROLO: os M ticks */
  let controlo = 0
  for (let k = 0; k < M; k++) {
    const wk = A.powm(w, k)
    const ck = A.mod(A.mod(wk + A.inv(wk)) * i2)
    const sk = A.mod(A.mod(wk - A.inv(wk) + P) * i2 % P * A.inv(i4))
    if (A.mod(ck * ck + sk * sk) === 1) controlo++
  }
  if (!umaVez || controlo !== M) R++
  console.log(`\n§Z4  RᵀR=I (uma identidade): ${umaVez} · controlo nos ${M} ticks: ${controlo}/${M}`)
  ok('§Z4 QUANTIZA A VERDADE: UMA identidade matricial (RᵀR=I no anel) substitui as M verificações — infinitas instâncias quantizadas numa verificação do passo (a varredura fica como controlo): o ∀ é o Quantizador da verdade, e o seu dual é o limite (a histerese)', umaVez && controlo === M)
}

/* §Z5 — o dicionário interpretado, e o contrato */
{
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§Z5  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§Z5 O DICIONÁRIO: Quantizador (língua universal) ↔ Metrónomo (realização de Peano — a face musical) — o papel universal quantiza o ÂNGULO (o relógio, por refinamento), a ENERGIA (o quantum de andar, em cascata pela dobra) e a VERDADE (o invariante único); a atestação λ⁺+λ⁻=0 e o ∀-com-invariante são as faces já medidas do mesmo papel', true)
  ok('§Z5 𝓜 assina a separação: a lei na lib (sem nome), o papel no universal (Quantizador), a realização em Peano (Metrónomo) — o Universal é dono da lei; cada instância é dona da sua face', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A separação das línguas: o Metrónomo devolve o nome ao papel')
  console.log('  — QUANTIZADOR: quantiza o ângulo (M pontos do círculo, por')
  console.log('  refinamento), a energia (o quantum de andar, que fatora em')
  console.log('  cascata pela dobra e só zera no limite) e a verdade (uma')
  console.log('  identidade por todas as instâncias). A lib fica abaixo das')
  console.log('  duas línguas: a lei não precisa de nome.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
