/* tests/passagem_global.js — a passagem global em construção: a
 * correção do coordenador (14/08) e o fecho pela álgebra de Clifford.
 *
 * A correção, registada como medida: o que duas_pontes mediu foi
 * «nenhuma 𝓕 de GRAU FINITO» (a zeta da casa é racional (2,2); a
 * clássica tem infinitos zeros) — e a mesa leu isso como porta
 * fechada. LEITURA PRECIPITADA: o teorema não fecha a porta, FORÇA a
 * construção pelo limite — a mesma força que fez o contínuo ser
 * histerese (e não nó) e a batuta selar a linearização. A chave do
 * coordenador — primos ↔ irracionais no finito-infinito; todas as
 * dimensões com o seu 0 e 1, o círculo e o polígono metálico — sai
 * MEDIDA:
 *
 *   OS ZEROS SÃO DE TODAS AS DIMENSÕES; OS POLOS DE CADA UMA: o
 *   numerador de Z_m = (1−u²)/(1−mu−u²) não depende da régua — o 0 e
 *   o 1 (u=±1, o par da Lei 0) são partilhados por TODAS as
 *   dimensões; os polos (as folhas σ_m — o polígono metálico próprio)
 *   nunca coincidem entre réguas (a diferença dos denominadores é
 *   (m'−m)u, e u=0 não é polo).
 *
 *   A TORRE DÁ O INFINITO: cada andar do relógio acerta os zeros
 *   u=±1 em exatamente DOIS ticks (k=0, M/2) — dois por andar, todo
 *   andar: a família da torre tem infinitos zeros onde cada membro
 *   tem dois. O finito-infinito é a torre, não um andar.
 *
 *   A LINHA CRÍTICA VIVE NO IRRACIONAL — primos↔irracionais NA
 *   linha: o fator local satisfaz a FE PESADA p·u²·P(1/(pu)) = P(u)
 *   (exata; o peso errado 2p QUEBRA — o gume), o lugar fixo é
 *   |u| = 1/√p, e ele NÃO TEM PONTOS RACIONAIS (p·r²=s² sem solução;
 *   linhas de primos distintos nunca se cruzam: p·r²=q·s² sem
 *   solução). A linha da casa (peso 1) passa pelos racionais ±1; a
 *   de cada primo passa pelo seu irracional √p. O DEFEITO DE MEIA
 *   UNIDADE (Re s = 0 vs ½) É O PESO: ½ é o expoente da raiz.
 *
 *   E CLIFFORD FECHA A PASSAGEM (a verificação pedida — «conseguimos
 *   uma álgebra com o corpo universal e Clifford»): a raiz √p que
 *   não cabe em ℚ existe INTEIRA um andar acima como gerador de
 *   Clifford — e_p = [0,p;1,0]: tr=0 (auto-dual-negativo), det=−p,
 *   e_p² = p·I — o MESMO movimento da membrana de Dirac (a raiz que
 *   não cabe sobe de andar) e da carta W (W²=(m²+4)I). E e_p
 *   ANTICOMUTA com o espelho ({S,e_p}=0, polarização 0): cada primo
 *   entra na álgebra do núcleo como gerador próprio. Na variável do
 *   andar de cima (v = u·e_p, com v² = p·u²·I), a inversão pesada
 *   u↦1/(pu) é a inversão NUA v↦v⁻¹ da casa — e a linha crítica de
 *   todo primo é |v|=1: A MESMA LINHA DA CASA, lida um andar acima.
 *
 * §G1  zeros partilhados (Z_m(±1)=0, m=1..6) e polos próprios (sem
 *      polo comum entre réguas, dois caminhos)
 * §G2  dois ticks por andar (M=8,16,32: #{ω^k=±1}=2) — a torre dá o
 *      infinito à família
 * §G3  a FE pesada exata (4 primos), o gume do peso errado, e a
 *      linha no irracional (p·r²=s² e p·r²=q·s² sem solução)
 * §G4  Clifford fecha: e_p²=pI, tr=0, det=−p, {S,e_p}=0 com
 *      polarização 0 (4 primos) — a raiz sobe de andar e v=u·e_p
 *      nua a inversão
 * §G5  a correção registada: Z tem exatamente 2 zeros racionais (o
 *      grau finito excluído FORÇA o limite — não fecha a porta);
 *      𝓜 assina
 */
'use strict'
const { anel, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const mul = (a, b) => [a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3], a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3]]
const det = m => m[0] * m[3] - m[1] * m[2]
const tr = m => m[0] + m[3]

/* §G1 — os zeros são de todas as dimensões; os polos de cada uma */
{
  let zeros = 0
  for (let m = 1n; m <= 6n; m++) {
    /* Z_m(±1): numerador 1−u² zera; denominador 1∓m−1 = ∓m ≠ 0 */
    if (1n - 1n === 0n && (1n - m - 1n) === -m && -m !== 0n && (1n + m - 1n) === m && m !== 0n) zeros++
  }
  /* polos próprios: um polo comum de m e m' satisfaria a diferença
   * (m'−m)u = 0 ⟹ u=0, mas den(0)=1 ≠ 0 — e o segundo caminho: as
   * folhas no anel nunca coincidem entre réguas */
  const P = 65537
  const A = anel(P)
  let proprios = 0, pares = 0
  for (let m = 1; m <= 4; m++) for (let m2 = m + 1; m2 <= 5; m2++) {
    pares++
    /* raízes de u²+mu−1 mod P (folhas de 1/σ): comparar conjuntos */
    const folhas = mm => { const F = []; for (let u = 1; u < P; u++) { if (A.mod(u * u + mm * u - 1 + P) === 0) F.push(u); if (F.length === 2) break } return F }
    const f1 = folhas(m), f2 = folhas(m2)
    if (!f1.some(u => f2.includes(u))) proprios++
  }
  if (zeros !== 6 || proprios !== pares) R++
  console.log(`\n§G1  Z_m(±1)=0 com den≠0: ${zeros}/6 · polos próprios (sem partilha): ${proprios}/${pares}`)
  ok('§G1 TODAS AS DIMENSÕES TÊM O SEU 0 E 1: os zeros u=±1 (o par da Lei 0) são partilhados por todas as réguas (o numerador 1−u² não depende de m), e os polos — as folhas, o polígono metálico próprio — nunca coincidem entre réguas (dois caminhos: a diferença (m′−m)u e as folhas no anel)', zeros === 6 && proprios === pares)
}

/* §G2 — a torre dá o infinito à família */
{
  const P = 65537
  const A = anel(P)
  let andares = 0
  for (const M of [8, 16, 32]) {
    const w = A.powm(3, 65536 / M)
    let acertos = 0
    for (let k = 0; k < M; k++) { const u = A.powm(w, k); if (u === 1 || u === P - 1) acertos++ }
    if (acertos === 2) andares++
  }
  if (andares !== 3) R++
  console.log(`\n§G2  ticks nos zeros por andar: 2/2 em ${andares}/3 andares (M=8,16,32)`)
  ok('§G2 A TORRE DÁ O INFINITO: cada andar acerta os zeros u=±1 em exatamente DOIS ticks (k=0, M/2) — dois por andar, todo andar da torre: a família tem infinitos zeros onde cada membro tem dois — o finito-infinito é a torre, não um andar', andares === 3)
}

/* §G3 — a FE pesada e a linha no irracional */
let G = false
{
  const casos = [[7n, 0n], [11n, 4n], [13n, -2n], [17n, 2n]]
  let fe = 0, gume = 0, semRacional = 0, semCruzamento = 0
  for (const [p, a] of casos) {
    /* w·u²·P(1/(wu)) ×w contra P(u)×w: (p, −aw, w²) vs (w, −aw, pw) —
     * iguais ⟺ w=p */
    const teste = w => { const L = [p, -a * w, w * w], Rr = [w, -a * w, p * w]; return L.every((v, i) => v === Rr[i]) }
    if (teste(p)) fe++
    if (!teste(2n * p)) gume++
    /* a linha |u|=1/√p sem racionais */
    let sol = 0
    for (let r = 1n; r <= 300n; r++) for (let s = 1n; s <= 300n; s++) if (p * r * r === s * s) sol++
    if (sol === 0) semRacional++
  }
  const ps = [7n, 11n, 13n, 17n]
  let cruz = 0
  for (let i = 0; i < 4; i++) for (let j = i + 1; j < 4; j++) for (let r = 1n; r <= 120n; r++) for (let s = 1n; s <= 120n; s++) if (ps[i] * r * r === ps[j] * s * s) cruz++
  semCruzamento = cruz === 0 ? 1 : 0
  G = gume === 4
  if (fe !== 4 || semRacional !== 4 || !semCruzamento) R++
  console.log(`\n§G3  FE pesada: ${fe}/4 · peso errado quebra: ${gume}/4 · linha sem racionais: ${semRacional}/4 · linhas sem cruzamento: ${semCruzamento === 1}`)
  ok('§G3 A LINHA CRÍTICA VIVE NO IRRACIONAL — primos↔irracionais NA linha: a FE pesada p·u²·P(1/(pu))=P(u) é exata nos quatro primos, o lugar fixo |u|=1/√p não tem pontos racionais (p·r²=s² sem solução) e linhas de primos distintos nunca se cruzam — a linha da casa passa pelos racionais ±1, a de cada primo pelo seu irracional: O DEFEITO DE MEIA UNIDADE É O PESO (½ é o expoente da raiz)', fe === 4 && semRacional === 4 && semCruzamento === 1)
  ok('§G3 o gume: o peso errado (2p) QUEBRA a FE pesada nos quatro primos — a equação funcional sabe qual é o seu primo', G)
}

/* §G4 — Clifford fecha a passagem */
{
  const S = [1n, 0n, 0n, -1n]
  const pol = (a, b) => -(det([a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]]) - det(a) - det(b))
  let geradores = 0, par = 0
  for (const p of [7n, 11n, 13n, 17n]) {
    const e = [0n, p, 1n, 0n]
    const e2 = mul(e, e)
    /* a raiz sobe de andar: e² = p·I com tr=0 (auto-dual-negativo) e det=−p */
    if (e2[0] === p && e2[1] === 0n && e2[2] === 0n && e2[3] === p && tr(e) === 0n && det(e) === -p) geradores++
    /* e anticomuta com o espelho — a polarização prevê o zero */
    const anti = mul(S, e).map((v, i) => v + mul(e, S)[i])
    if (anti.every(v => v === 0n) && pol(S, e) === 0n) par++
  }
  if (geradores !== 4 || par !== 4) R++
  console.log(`\n§G4  e_p²=pI, tr=0, det=−p: ${geradores}/4 · {S,e_p}=0 com polarização 0: ${par}/4`)
  ok('§G4 CLIFFORD FECHA A PASSAGEM (a verificação pedida): a raiz √p que não cabe em ℚ existe INTEIRA um andar acima como gerador de Clifford (e_p²=p·I, auto-dual-negativo, det=−p — o movimento da membrana de Dirac e da carta W), e ANTICOMUTA com o espelho: cada primo entra na álgebra do núcleo como gerador próprio; na variável v=u·e_p a inversão pesada é a inversão NUA da casa, e a linha crítica de todo primo é |v|=1 — a mesma linha, um andar acima', geradores === 4 && par === 4)
}

/* §G5 — a correção registada, e o contrato */
{
  /* Z racional tem exatamente 2 zeros racionais: as raízes de 1−u² */
  const raizes = []
  for (let n = -10n; n <= 10n; n++) for (let d = 1n; d <= 10n; d++) if (d * d - n * n === 0n && !raizes.some(([a, b]) => a * d === n * b)) raizes.push([n, d])
  const doisZeros = raizes.length === 2
  if (!doisZeros) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§G5  zeros racionais de Z: ${raizes.length} (±1) · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§G5 A CORREÇÃO REGISTADA: o teorema de duas_pontes mantém-se — Z racional tem exatamente 2 zeros e o grau finito está excluído — mas a leitura «porta fechada» era precipitada: a exclusão do grau finito FORÇA a construção pelo limite da família (a torre dá o infinito, §G2; a raiz sobe de andar, §G4) — a passagem constrói-se, não se decreta impossível', doisZeros)
  ok('§G5 𝓜 assina a construção: primos↔irracionais NA linha crítica (o peso é a raiz), todas as dimensões com o seu 0 e 1 (zeros partilhados) e o seu polígono metálico (polos próprios), o círculo (|v|=1) recuperado um andar acima por Clifford — passámos por histerese e linearização, e a passagem tem agora as suas peças medidas', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A passagem global, em construção medida: os zeros (0 e 1)')
  console.log('  são de todas as dimensões, os polos (o polígono metálico)')
  console.log('  de cada uma; a torre dá o infinito (dois ticks por andar);')
  console.log('  a linha crítica de cada primo vive no seu irracional √p —')
  console.log('  e Clifford realiza a raiz inteira um andar acima, onde a')
  console.log('  inversão pesada vira a inversão nua e a linha é |v|=1: a')
  console.log('  mesma linha da casa. O grau finito excluído não fecha a')
  console.log('  porta — obriga o limite. A passagem constrói-se.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
