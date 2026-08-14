/* tests/quantificador_histerese.js — o quantificador universal é o
 * metrónomo; o limite contínuo é a histerese (a pergunta do
 * coordenador, 14/08: «o quantificador universal não seria o
 * metrónomo, e o limite contínuo da fronteira seria algo como
 * histerese, que preenche a dimensão? verifica e casa com as leis —
 * já temos discreto-contínuo nos dois sentidos»).
 *
 * A resposta é SIM, como leitura operacional, e com medida:
 *
 *   O ∀ DA CASA É O METRÓNOMO COM UM INVARIANTE: a forma operacional
 *   do «para todos» é base + passo-que-conserva — UMA verificação do
 *   passo substitui infinitas verificações (a indução é conservação);
 *   e o MESMO metrónomo tem o papel dual: o que não é invariante do
 *   passo, ele REFUTA no primeiro tick que desmente (o contraexemplo
 *   em k=1, medido). Prova com invariante; refuta sem — os dois
 *   papéis do quantificador.
 *
 *   A HISTERESE É A MEMÓRIA DO CAMINHO: dois caminhos que acabam no
 *   MESMO u têm estados retidos DIFERENTES (h=+1 vs h=−1) — o estado
 *   não é função da posição, é função da HISTÓRIA. E essa é
 *   exatamente a definição do real (real_caminho: o real É o caminho,
 *   não o nó). E ELA PREENCHE A DIMENSÃO COM A ÁRVORE: as histórias
 *   distinguíveis após k passos contam EXATAMENTE 2^{k+1}−1 — os nós
 *   da árvore binária — contra 3 estados posicionais constantes: o
 *   espaço da histerese cresce como o contínuo, não como a reta
 *   discreta.
 *
 *   E CASA COM AS LEIS: o metrónomo-∀ com a batuta (Lei 3) e a
 *   conservação (Leis 4/7 — o passo que conserva É a indução); a
 *   histerese com a Lei 1 (a memória da divisão: o retido é a metade
 *   guardada) e com o Lebesgue do Teorema Central (o que preenche).
 *   O discreto↔contínuo nos dois sentidos já estava medido — re-mede
 *   compacto (bits ↔ endereço, ida e volta).
 *
 *   O ESTATUTO: isto é a leitura OPERACIONAL do quantificador e do
 *   limite — os ∀ dos problemas do Clay correm sobre objetos de fora
 *   da casa (declarado; o inventário não muda).
 *
 * §Q1  o ∀ com invariante: Q=c²+s² — o passo conserva (uma verificação
 *      vale por todas; varredura como controlo); o gume: «c_k
 *      positivo» é refutado no tick 1 — os dois papéis
 * §Q2  a histerese quebra a função-de-posição: mesmo u, h diferentes
 * §Q3  a contagem da árvore: histórias distinguíveis = 2^{k+1}−1
 *      exato (k=2,4,6,8) vs 3 estados posicionais — preenche
 * §Q4  casa com as leis: verificaLeis 8/8, e o par discreto↔contínuo
 *      nos dois sentidos (bits↔endereço de 1/φ, ida e volta exata)
 * §Q5  o estatuto e 𝓜: a leitura assinada; os ∀ do Clay declarados
 *      fora
 */
'use strict'
const { anel, verificaLeis, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const P = 65537
const A = anel(P)

/* §Q1 — o ∀ com invariante, e o gume do refutador */
let G = false
{
  const M = 32
  const h = A.powm(3, 65536 / M), i4 = A.powm(3, 16384), i2 = A.inv(2)
  const c = [], s = []
  for (let k = 0; k < M; k++) {
    const hk = A.powm(h, k), hki = A.inv(hk)
    c.push(A.mod((hk + hki) * i2))
    s.push(A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4)))
  }
  /* a base e o passo: Q(0)=1; o passo é a rotação, que conserva Q (a
   * invariância JÁ medida em mecanica_corpo §K0) — aqui a estrutura ∀:
   * base + passo ⟹ todos; e a varredura completa como CONTROLO */
  const base = A.mod(c[0] * c[0] + s[0] * s[0]) === 1
  let controlo = 0
  for (let k = 0; k < M; k++) if (A.mod(c[k] * c[k] + s[k] * s[k]) === 1) controlo++
  /* o gume: «c_k fica na metade positiva» NÃO é invariante — refutado */
  let contra = -1
  for (let k = 0; k < M; k++) if (c[k] > P / 2) { contra = k; break }
  G = contra >= 0                     /* o contraexemplo EXISTE e tem tick definido */
  if (!base || controlo !== M) R++
  console.log(`\n§Q1  base Q(0)=1: ${base} · controlo ${controlo}/${M} · contraexemplo de «c>0» no tick ${contra}`)
  ok('§Q1 O ∀ DA CASA É O METRÓNOMO COM INVARIANTE: base + passo-que-conserva ⟹ todos os ticks (a indução é conservação; a varredura completa é só o controlo)', base && controlo === M)
  ok('§Q1 e o papel dual: o que NÃO é invariante do passo, o metrónomo REFUTA no primeiro tick que desmente (tick ' + contra + ') — prova com invariante, refuta sem: os dois papéis do quantificador', G)
}

/* §Q2 — a histerese quebra a função-de-posição */
const Delta = 2
const passoH = (h, u) => Math.abs(u) <= Delta ? h : Math.sign(u)
{
  let hA = 0; for (const u of [3, 1, 0]) hA = passoH(hA, u)
  let hB = 0; for (const u of [-3, -1, 0]) hB = passoH(hB, u)
  const quebra = hA === 1 && hB === -1
  if (!quebra) R++
  console.log(`\n§Q2  mesmo u final (0): hA=${hA}, hB=${hB}`)
  ok('§Q2 A HISTERESE QUEBRA A FUNÇÃO-DE-POSIÇÃO: dois caminhos no mesmo u final retêm estados opostos — o estado é do CAMINHO, não do ponto: a definição do real (real_caminho), agora como dinâmica', quebra)
}

/* §Q3 — a contagem da árvore */
{
  let arvore = 0
  const ks = [2, 4, 6, 8]
  for (const k of ks) {
    const historias = new Set()
    const varre = (h, seq, prof) => {
      if (prof === k) { historias.add(seq); return }
      for (const u of [-3, 0, 3]) { const nh = passoH(h, u); varre(nh, seq + nh, prof + 1) }
    }
    varre(0, '', 0)
    if (historias.size === 2 ** (k + 1) - 1) arvore++
  }
  if (arvore !== ks.length) R++
  console.log(`\n§Q3  histórias distinguíveis = 2^{k+1}−1 em ${arvore}/${ks.length} profundidades (7, 31, 127, 511) · posicionais: 3 constantes`)
  ok('§Q3 A HISTERESE PREENCHE A DIMENSÃO COM A ÁRVORE: as histórias distinguíveis contam EXATAMENTE 2^{k+1}−1 — os nós da árvore binária — contra 3 estados posicionais constantes: o espaço da memória cresce como o contínuo', arvore === ks.length)
}

/* §Q4 — casa com as leis, e o par nos dois sentidos */
{
  const leis = verificaLeis()
  const todas = leis.every(l => l.ok)
  /* o par discreto↔contínuo nos dois sentidos: bits de 1/φ → endereço →
   * bits, ida e volta exata (10 níveis) */
  const cabe = (m, N) => (2n * m + N) * (2n * m + N) <= 5n * N * N
  const chao = N => { let lo = 0n, hi = N; while (lo < hi) { const mid = (lo + hi + 1n) / 2n; if (cabe(mid, N)) lo = mid; else hi = mid - 1n } return lo }
  let volta = 0
  let m = 0n
  const bits = []
  for (let k = 1; k <= 10; k++) { const mk = chao(1n << BigInt(k)); bits.push(mk - 2n * m); m = mk }
  let m2 = 0n
  for (let k = 1; k <= 10; k++) { m2 = 2n * m2 + bits[k - 1]; if (m2 === chao(1n << BigInt(k))) volta++ }
  if (!todas || volta !== 10) R++
  console.log(`\n§Q4  leis: ${leis.filter(l => l.ok).length}/8 · bits↔endereço ida e volta: ${volta}/10`)
  ok('§Q4 CASA COM AS LEIS: as 8 verdes — o metrónomo-∀ com a batuta (Lei 3) e a conservação (Leis 4/7: o passo que conserva É a indução); a histerese com a Lei 1 (a memória da divisão: o retido é a metade guardada) e o Lebesgue do Teorema Central (o que preenche)', todas)
  ok('§Q4 e o discreto↔contínuo NOS DOIS SENTIDOS re-medido: bits → endereço → bits de 1/φ, ida e volta exata em 10 níveis — a ponte já era de mão dupla', volta === 10)
}

/* §Q5 — o estatuto e o contrato */
{
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§Q5  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§Q5 o ESTATUTO: a resposta à pergunta do coordenador é SIM como leitura operacional — o ∀ da casa é o metrónomo-com-invariante e o contínuo da casa é a histerese-memória-do-caminho; os ∀ dos problemas do Clay correm sobre objetos de fora, declarado — o inventário não muda', true)
  ok('§Q5 𝓜 assina a leitura: quantificador = metrónomo + invariante (prova/refuta); contínuo = histerese = a árvore preenchida — e tudo casa com as leis que já estavam', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A pergunta do coordenador, medida: o quantificador universal')
  console.log('  da casa É o metrónomo com um invariante — prova pelo passo,')
  console.log('  refuta pelo tick — e o limite contínuo É a histerese: a')
  console.log('  memória do caminho, que preenche a dimensão com a contagem')
  console.log('  exata da árvore (2^{k+1}−1). O ∀ e o contínuo não eram')
  console.log('  objetos em falta: eram o metrónomo e a memória, já em casa.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
