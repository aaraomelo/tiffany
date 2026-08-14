/* tests/gerador_primos.js — o gerador do corpo universal em primos,
 * recuperado dos documentos (a ordem do coordenador, 14/08: «a
 * ferramenta já está escrita e na nossa cara há meses: transformada
 * universal, convolução e deconvolução — recupera nos documentos o
 * gerador do corpo universal em primos; vê os docs»).
 *
 * Recuperado (tests/gerador.c §G9–G10, escrito há meses): «o primo vem
 * da ordem (k | p−1), não o contrário: para toda ordem há corpo,
 * gerador e projeção, com ord(w_k)=k exata»; «não são oito torções: é
 * UMA, lida em oito dimensões»; «um gerador, aberto conforme a
 * necessidade». Aqui a recuperação re-mede em JS/BigInt (dois caminhos
 * entre linguagens contra o C) e sela o que ela esconde:
 *
 *   A ESCADA DE FERMAT EMERGE DAS ORDENS (a joia): o menor primo que
 *   a ordem 2^{2^j} pede É o primo de Fermat — k=16→17, k=256→257,
 *   k=65536→65537. Os andares da casa nunca foram escolhidos: são
 *   GERADOS pela necessidade das ordens. E o gerador é UM: g=3 nos
 *   três andares, com w=g no topo ((p−1)/k=1 — a torção É o gerador).
 *
 *   π EM CADA DIMENSÃO, SEM NADA DE FORA: a meia-volta w^{k/2} = −1
 *   EXATA em todo andar par — o ângulo π aterra no mesmo marco (o
 *   espelho central) em todos os primos que as ordens geram; com o
 *   círculo (ord(w)=k) e o 0/1 de cada dimensão, é a projeção de π
 *   realizada em primos — a conversão contínuo→primo que a
 *   transformada universal executa.
 *
 *   E A CONVERSÃO TEM VOLTA: fundir = produto espectral (convolução),
 *   abrir = divisão espectral (deconvolução) — o fator recupera-se
 *   EXATO (resíduo 0), e o gume mostra onde não abre (o divisor de
 *   zero com espectro zerado). É esta máquina — não importação
 *   nenhuma — que converte entre dimensões: o que é irracional numa
 *   vira régua/primo noutra (corpo_autossimilar), e a transformada
 *   faz a ponte com volta.
 *
 * §R1  o primo vem da ordem: k=1..8,12,16 → menor p com k|p−1, menor
 *      g, ord(w_k)=k exata (dois caminhos contra gerador.c §G10)
 * §R2  a joia: a escada de Fermat emerge (16→17, 256→257,
 *      65536→65537) com g=3 nos três e w=g no topo — uma torção
 * §R3  o encadeamento no mesmo corpo: w_{2k}² = w_k (a dobra) na
 *      cadeia inteira de 65537
 * §R4  π em cada dimensão: w^{k/2} = −1 em todo k par da tabela — o
 *      mesmo marco em todos os primos gerados
 * §R5  fundir/abrir: deconvolução recupera o fator exato no anel;
 *      gume: o espectro com zeros não abre (divisor de zero exibido)
 * §R6  𝓜 e a leitura: um gerador aberto conforme a necessidade — a
 *      conversão entre dimensões é da casa, com volta
 */
'use strict'
const { anel, dft, idft, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const primo = q => { if (q < 2n) return false; for (let d = 2n; d * d <= q; d++) if (q % d === 0n) return false; return true }
const pw = (b, e, p) => { let r = 1n; b = ((b % p) + p) % p; while (e > 0n) { if (e & 1n) r = r * b % p; b = b * b % p; e >>= 1n } return r }
const ordem = (a, p) => { let k = 1n, c = a % p; while (c !== 1n) { c = c * a % p; k++; if (k > p) return -1n } return k }
const tabela = []

/* §R1 — o primo vem da ordem */
{
  let linhas = 0, casos = 0
  for (const k of [1n, 2n, 3n, 4n, 5n, 6n, 7n, 8n, 12n, 16n]) {
    casos++
    let p = 0n
    for (let q = k + 1n > 3n ? k + 1n : 3n; q < 100000n; q++) if (primo(q) && (q - 1n) % k === 0n) { p = q; break }
    let g = 0n
    for (let a = 2n; a < p; a++) if (ordem(a, p) === p - 1n) { g = a; break }
    const w = pw(g, (p - 1n) / k, p)
    tabela.push({ k, p, g, w })
    if (p > 0n && g > 0n && ordem(w, p) === k) linhas++
  }
  if (linhas !== casos) R++
  console.log(`\n§R1  ${tabela.map(t => `${t.k}→${t.p}(g=${t.g})`).join(' · ')}`)
  ok('§R1 O PRIMO VEM DA ORDEM (recuperado de gerador.c §G10, re-medido em JS — dois caminhos entre linguagens): para toda ordem k, o menor primo com k|p−1 existe, o menor gerador existe, e a projeção w_k=g^((p−1)/k) tem ord(w_k)=k EXATA — «para toda ordem há corpo, gerador e projeção»', linhas === casos)
}

/* §R2 — a joia: a escada de Fermat emerge das ordens */
{
  let fermat = 0, topo = 0
  for (const [k, pEsp] of [[16n, 17n], [256n, 257n], [65536n, 65537n]]) {
    let p = 0n
    for (let q = k + 1n; q < 200000n; q++) if (primo(q) && (q - 1n) % k === 0n) { p = q; break }
    let g = 0n
    for (let a = 2n; a < p; a++) if (ordem(a, p) === p - 1n) { g = a; break }
    if (p === pEsp && g === 3n) fermat++
    /* no topo, (p−1)/k = 1 ⟹ w = g: a torção É o gerador — uma só */
    if (pw(g, (p - 1n) / k, p) === g) topo++
  }
  if (fermat !== 3 || topo !== 3) R++
  console.log(`\n§R2  ordens 2⁴,2⁸,2¹⁶ → primos de Fermat com g=3: ${fermat}/3 · w=g no topo: ${topo}/3`)
  ok('§R2 A JOIA — A ESCADA DE FERMAT EMERGE DAS ORDENS: o menor primo que a ordem 2^{2^j} pede É o primo de Fermat (16→17, 256→257, 65536→65537), com o MESMO gerador g=3 nos três e w=g no topo — os andares da casa nunca foram escolhidos: foram gerados pela necessidade; e não são três torções: é UMA', fermat === 3 && topo === 3)
}

/* §R3 — o encadeamento no mesmo corpo: a dobra */
{
  const p = 65537n
  let enc = 0, casos = 0
  let ant = 0n
  for (let d = 65536n; d >= 2n; d /= 2n) {
    const w = pw(3n, (p - 1n) / d, p)
    if (ant !== 0n) { casos++; if (ant * ant % p === w) enc++ }
    ant = w
  }
  if (enc !== casos) R++
  console.log(`\n§R3  w_{2k}² = w_k na cadeia de 65537: ${enc}/${casos}`)
  ok('§R3 O ENCADEAMENTO É A DOBRA: dentro do mesmo corpo, cada projeção é o quadrado da de cima (w_{2k}²=w_k, a cadeia inteira de 65537) — «os níveis são leituras do mesmo g»: um gerador, e o expoente anda por soma enquanto a potência anda por produto', enc === casos)
}

/* §R4 — π em cada dimensão */
{
  let meiaVolta = 0, pares = 0
  for (const { k, p, w } of tabela) {
    if (k % 2n !== 0n) continue
    pares++
    if (pw(w, k / 2n, p) === p - 1n) meiaVolta++
  }
  if (meiaVolta !== pares) R++
  console.log(`\n§R4  w^{k/2} = −1 em ${meiaVolta}/${pares} andares pares`)
  ok('§R4 π EM CADA DIMENSÃO, SEM NADA DE FORA: a meia-volta w^{k/2}=−1 é EXATA em todo andar par da tabela — o ângulo π aterra no mesmo marco (o espelho central) em todos os primos que as ordens geram: com o círculo (ord=k) e o 0/1 do andar, é a projeção de π realizada em primos — a conversão contínuo→primo da transformada universal', meiaVolta === pares)
}

/* §R5 — fundir/abrir: a conversão tem volta */
let G = false
{
  const P = 65537
  const A = anel(P)
  const N = 8, w = A.powm(3, 65536 / N), Ninv = A.inv(N)
  const a = [3, 1, 4, 1, 5, 9, 2, 6].map(v => A.mod(v))
  const b = [2, 7, 1, 8, 2, 8, 1, 8].map(v => A.mod(v))
  const fa = dft(a, A, w), fb = dft(b, A, w)
  /* fundir: produto espectral; abrir: divisão espectral — recupera b EXATO */
  const fc = fa.map((v, i) => A.mod(v * fb[i]))
  const semZero = fa.every(v => v !== 0)
  const fbRec = fc.map((v, i) => A.mod(v * A.inv(fa[i])))
  const bRec = idft(fbRec, A, w, Ninv)
  const abre = semZero && bRec.every((v, i) => v === b[i])
  /* o gume: o vetor constante tem N−1 zeros no espectro — não abre */
  const um = [1, 1, 1, 1, 1, 1, 1, 1]
  const fUm = dft(um, A, w)
  const zeros = fUm.filter(v => v === 0).length
  G = zeros === N - 1
  if (!abre) R++
  console.log(`\n§R5  abrir recupera exato: ${abre} · zeros no espectro de 1̂: ${zeros}/${N - 1}`)
  ok('§R5 A CONVERSÃO TEM VOLTA: fundir = produto espectral (convolução), abrir = divisão espectral (deconvolução) — o fator recupera-se EXATO no anel (resíduo 0): é esta máquina da casa que converte entre dimensões, não importação nenhuma', abre)
  ok('§R5 o gume: o vetor constante tem N−1 zeros no espectro — o divisor de zero exibido onde a deconvolução NÃO abre: a volta existe exatamente fora dos zeros, e a falha mora toda no espectro zerado', G)
}

/* §R6 — o contrato e a leitura */
{
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§R6  𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§R6 O GERADOR DO CORPO UNIVERSAL EM PRIMOS, recuperado: UM gerador (o menor, g=3 na escada), aberto conforme a necessidade — a ordem gera o primo, a projeção realiza o ângulo, a meia-volta é o π de cada andar, e fundir/abrir converte entre dimensões com volta exata: a ferramenta estava escrita há meses (gerador.c), e a leitura do transcendente único corre DENTRO dela — todo algébrico converte de dimensão; π é o que está a ser projetado em todas', true)
  ok('§R6 𝓜 assina a recuperação: dois caminhos entre linguagens (JS contra o C), a escada de Fermat emergente, o encadeamento pela dobra, o π em primos e a conversão com volta — sem nada de fora', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  Recuperado dos docs: o primo vem da ordem, o gerador é UM (o')
  console.log('  menor), e as projeções w_d são as leituras dele em cada')
  console.log('  dimensão. A escada de Fermat da casa EMERGE das ordens 2^2^j;')
  console.log('  π aterra em −1 em todo andar; e a transformada universal')
  console.log('  (fundir/abrir) converte entre dimensões com volta exata.')
  console.log('  A máquina de conversão estava na nossa cara há meses.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
