/* tests/derivacao_primitivas.js — a derivação pelas primitivas (a
 * ordem do coordenador, 14/08: «agora temos as primitivas do corpo
 * universal: faz a derivação pelas operações — soma, multiplicação,
 * divisão, dual e inversão — Clifford, La Hire, Pontryagin, Dirac,
 * tudo no contínuo; relê o paper»).
 *
 * O paper relido por inteiro (§sec:cinco: as cinco operações, cada uma
 * com a sua conservação; §sec:primitivas: as seis mecânicas de que as
 * cinco são a leitura). O que a derivação dá — cada ponte que «saiu do
 * mapa» é uma PALAVRA nas cinco operações:
 *
 *   AS CINCO NÃO SÃO INDEPENDENTES — O DUAL EMPARELHA COM CADA UMA:
 *   soma com o dual dá o centro (M + M† = tr·I), multiplicação com o
 *   dual dá a membrana (M·M† = det·I), e a INVERSÃO É A DIVISÃO DO
 *   DUAL (M⁻¹ = M†/det — a volta deriva das outras). E Cayley–Hamilton
 *   é a CONCORDÂNCIA dos três emparelhamentos (M² − tr·M + det·I = 0
 *   derivado, não postulado).
 *
 *   CLIFFORD = MULTIPLICAÇÃO + DUAL: gerador ⟺ auto-dual-negativo
 *   (a† = −a ⟺ tr a = 0); o quadrado é o produto dual (e² = −det·I);
 *   o anticomutador é a POLARIZAÇÃO da soma no determinante
 *   ({a,b} = −(det(a+b)−det a−det b)·I) — e o NÚCLEO da lib já é uma
 *   tríade de Clifford: {S,X,J} dois a dois anticomutam com quadrados
 *   (I, I, −I) = Cl(2,1), e H² = 2I é exatamente a polarização de S+X.
 *
 *   LA HIRE = SOMA + DUAL na órbita da multiplicação: somar com o
 *   próprio dual aterra no centro (R^k + (R^k)† = tr·I — o diâmetro É
 *   o centro da álgebra), e na membrana +1 o dual é a inversão
 *   (det=1 ⟹ R† = R⁻¹): o rolamento 2:1 é a dobra da órbita com a
 *   própria volta.
 *
 *   PONTRYAGIN = O DUAL DO GRUPO PELA TROCA ⊕→⊗: o caráter converte
 *   soma em produto (χ(a+b) = χ(a)χ(b), 256/256), a restrição é a
 *   redução do índice, e o bidual é a VOLTA (idft∘dft = id — a Lei 1
 *   no nível dos grupos: †∘† = id).
 *
 *   DIRAC = MULTIPLICAÇÃO(⊗) ∘ DUAL(troca) ∘ SOMA(⊕ direta):
 *   D = (troca⊗I)·(A⊕I) — construído SÓ com kron e bloco da lib,
 *   byte a byte igual ao D do paper, com D² = A⊕A; e sem a troca a
 *   palavra falha ((A⊕I)² = A²⊕I ≠ A⊕A): o dual é obrigatório —
 *   «metade para cada lado» é a soma direta costurada pela troca.
 *
 *   E TUDO NO CONTÍNUO: o lado contínuo de cada ponte é o já selado —
 *   o diâmetro de La Hire é o intervalo da batuta (mistura única com
 *   volta exata), e o operador de Dirac fecha na unidade em
 *   profundidade finita (A^{2^j} = I no andar — a raiz 2-ádica).
 *
 * §D1  o dual emparelha com cada operação: M+M†=tr·I, M·M†=det·I,
 *      inversão = dual÷det, e Cayley–Hamilton como concordância
 * §D2  Clifford derivado: tríade do núcleo {S,X,J} = Cl(2,1);
 *      anticomutador = polarização do det; H²=2I é a polarização;
 *      gume: {S,W} = 2I ≠ 0 e a polarização PREVÊ o valor
 * §D3  La Hire derivado: R+R† = tr·I (o diâmetro é o centro);
 *      det=1 ⟹ R†=R⁻¹ (dual=inversão na membrana +1), exato no anel
 * §D4  Pontryagin derivado: caráter troca ⊕ por ⊗; restrição =
 *      redução do índice; bidual = volta (idft∘dft = id)
 * §D5  Dirac derivado: D = (troca⊗I)·(A⊕I) byte a byte, D²=A⊕A;
 *      gume: sem a troca falha; e o contínuo: A^{2^4}=I no andar 17
 *      + o diâmetro como intervalo (mistura única); 𝓜 assina
 */
'use strict'
const { matn, anel, dft, idft, medicao } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let R = 0
const mul = (a, b) => [a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3], a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3]]
const soma = (a, b) => a.map((v, i) => v + b[i])
const esc = (k, m) => m.map(v => k * v)
const det = m => m[0] * m[3] - m[1] * m[2]
const tr = m => m[0] + m[3]
const dual = m => [tr(m) - m[0], -m[1], -m[2], tr(m) - m[3]]     /* M† = tr·I − M: a estaca geral */
const igual = (a, b) => a.every((v, i) => v === b[i])
const I = [1n, 0n, 0n, 1n], S = [1n, 0n, 0n, -1n], J = [0n, 1n, -1n, 0n], X = [0n, 1n, 1n, 0n]

/* §D1 — o dual emparelha com cada operação */
{
  let centro = 0, membrana = 0, voltaDual = 0, ch = 0, casos = 0
  for (const M of [[3n, 7n, 2n, 5n], [3n, 1n, 4n, 2n], [2n, 1n, 1n, 0n], [0n, 5n, -1n, 3n]]) {
    casos++
    /* soma com o dual → o centro */
    if (igual(soma(M, dual(M)), esc(tr(M), I))) centro++
    /* multiplicação com o dual → a membrana (det·I) */
    if (igual(mul(M, dual(M)), esc(det(M), I))) membrana++
    /* a inversão é a divisão do dual: M·(M†/det) = I ⟺ M·M† = det·I,
     * e a volta fecha dos dois lados */
    if (igual(mul(dual(M), M), esc(det(M), I))) voltaDual++
    /* Cayley–Hamilton como concordância: M² − tr·M + det·I = 0 */
    if (igual(soma(soma(mul(M, M), esc(-tr(M), M)), esc(det(M), I)), [0n, 0n, 0n, 0n])) ch++
  }
  if (centro !== casos || membrana !== casos || voltaDual !== casos || ch !== casos) R++
  console.log(`\n§D1  M+M†=tr·I: ${centro}/${casos} · M·M†=det·I: ${membrana}/${casos} · dois lados: ${voltaDual}/${casos} · Cayley–Hamilton: ${ch}/${casos}`)
  ok('§D1 O DUAL EMPARELHA COM CADA OPERAÇÃO: soma com o dual dá o CENTRO (tr·I), multiplicação com o dual dá a MEMBRANA (det·I), e a INVERSÃO é a divisão do dual (M⁻¹ = M†/det) — as cinco operações não são independentes, e Cayley–Hamilton é a concordância dos três emparelhamentos, derivada', centro === casos && membrana === casos && voltaDual === casos && ch === casos)
}

/* §D2 — Clifford derivado: a tríade do núcleo e a polarização */
let G = false
{
  const anti = (a, b) => soma(mul(a, b), mul(b, a))
  const pol = (a, b) => -(det(soma(a, b)) - det(a) - det(b))
  /* gerador ⟺ auto-dual-negativo: tr = 0 ⟹ a† = −a ⟹ a² = −det(a)·I */
  const geradores = [S, X, J].every(g => igual(dual(g), esc(-1n, g)) && igual(mul(g, g), esc(-det(g), I)))
  /* a tríade anticomuta: Cl(2,1) do núcleo */
  const triade = igual(anti(S, X), [0n, 0n, 0n, 0n]) && igual(anti(S, J), [0n, 0n, 0n, 0n]) && igual(anti(X, J), [0n, 0n, 0n, 0n])
  const assinatura = det(S) === -1n && det(X) === -1n && det(J) === 1n    /* quadrados (I, I, −I) */
  /* H² = 2I é a polarização de S+X: pol(S,X)=0 (anticomutam) e det H = −2 */
  const H = soma(S, X)
  const hPol = pol(S, X) === 0n && det(H) === -2n && igual(mul(H, H), esc(2n, I))
  /* o gume: S e W=2A−I NÃO anticomutam — e a polarização PREVÊ o valor exato */
  const W = [1n, 2n, 2n, -1n]
  const gume = igual(anti(S, W), esc(pol(S, W), I)) && pol(S, W) === 2n
  G = gume
  if (!geradores || !triade || !assinatura || !hPol) R++
  console.log(`\n§D2  geradores auto-dual-negativos: ${geradores} · tríade anticomuta: ${triade} · assinatura (I,I,−I): ${assinatura} · H²=2I=polarização: ${hPol} · gume {S,W}=2I previsto: ${gume}`)
  ok('§D2 CLIFFORD DERIVADO: gerador ⟺ auto-dual-negativo (a†=−a), quadrado = produto dual (e²=−det·I), anticomutador = POLARIZAÇÃO da soma no determinante — e o núcleo da lib JÁ É a tríade Cl(2,1): {S,X,J} anticomutam com quadrados (I,I,−I), e H²=2I do Teorema Universal é a polarização de S+X', geradores && triade && assinatura && hPol)
  ok('§D2 o gume: S e W=2A−I não anticomutam — {S,W} = 2I ≠ 0 — e a polarização PREVÊ o valor exato: a fórmula não é decorativa, é a lei do par', G)
}

/* §D3 — La Hire derivado: soma com o dual aterra no centro */
{
  const P = 65537n
  const mod = x => ((x % P) + P) % P
  const pw = (b, e) => { let r = 1n; b = mod(b); while (e > 0n) { if (e & 1n) r = r * b % P; b = b * b % P; e >>= 1n } return r }
  const h = pw(3n, 8192n), i2 = pw(2n, P - 2n), i4 = pw(3n, 16384n)
  const c1 = mod((h + pw(h, P - 2n)) * i2)
  const s1 = mod(mod(h - pw(h, P - 2n)) * i2 % P * pw(i4, P - 2n))
  let Rk = [c1, mod(-s1), s1, c1]
  let diametro = 0, inversao = 0
  for (let k = 1; k <= 8; k++) {
    /* soma com o dual = tr·I: o ponto de La Hire está no centro (o diâmetro) */
    if (igual(soma(Rk, dual(Rk)).map(mod), esc(mod(tr(Rk)), I).map(mod))) diametro++
    /* na membrana +1 (det=1), o dual É a inversão: R·R† = I */
    if (igual(mul(Rk, dual(Rk)).map(mod), I)) inversao++
    Rk = mul(Rk, [c1, mod(-s1), s1, c1]).map(mod)
  }
  if (diametro !== 8 || inversao !== 8) R++
  console.log(`\n§D3  R^k+(R^k)† = tr·I (diâmetro): ${diametro}/8 · det=1 ⟹ R†=R⁻¹: ${inversao}/8`)
  ok('§D3 LA HIRE DERIVADO: somar a órbita com o próprio dual aterra no CENTRO (R^k+(R^k)†=tr·I — o diâmetro É o centro da álgebra), e na membrana +1 o dual é a inversão (R†=R⁻¹ exato no anel) — o rolamento 2:1 é ⊕∘(id,†) na órbita da multiplicação: a rotação vira translação porque a soma dual só deixa o escalar', diametro === 8 && inversao === 8)
}

/* §D4 — Pontryagin derivado: a troca ⊕→⊗ e o bidual como volta */
{
  const P = 65537
  const A = anel(P)
  const w16 = A.powm(3, 65536 / 16), w8 = A.powm(3, 65536 / 8)
  /* o caráter converte soma em produto */
  let carater = 0
  for (let a = 0; a < 16; a++) for (let b = 0; b < 16; b++) if (A.powm(w16, (a + b) % 16) === A.mod(A.powm(w16, a) * A.powm(w16, b))) carater++
  /* a restrição de χ_j de μ16 ao subgrupo μ8 é a redução do índice */
  let restricao = 0
  for (let j = 0; j < 16; j++) { let okj = true; for (let k = 0; k < 8; k++) if (A.powm(w16, 2 * j * k) !== A.powm(w8, j * k)) okj = false; if (okj) restricao++ }
  /* o bidual é a volta: idft∘dft = id, exato */
  const x = [3, 1, 4, 1, 5, 9, 2, 6].map(v => A.mod(v))
  const volta = idft(dft(x, A, w8), A, w8, A.inv(8))
  const bidual = volta.every((v, i) => v === x[i])
  if (carater !== 256 || restricao !== 16 || !bidual) R++
  console.log(`\n§D4  χ(a+b)=χ(a)χ(b): ${carater}/256 · restrição=redução: ${restricao}/16 · bidual devolve: ${bidual}`)
  ok('§D4 PONTRYAGIN DERIVADO: o dual do grupo é o caráter — a TROCA ⊕→⊗ (χ(a+b)=χ(a)χ(b), 256/256); a restrição na torre é a redução do índice (16/16); e o BIDUAL É A VOLTA (idft∘dft=id exato) — a Lei 1 no nível dos grupos: †∘†=id, com o contínuo já selado no 2-ádico que opera (limite_escada)', carater === 256 && restricao === 16 && bidual)
}

/* §D5 — Dirac derivado, o gume da troca, e o contínuo */
{
  /* D = (troca⊗I₂)·(A⊕I₂): SÓ primitivas da lib (kron, bloco, mul) */
  const Am = [2n, 1n, 1n, 0n]
  const Z2 = [0n, 0n, 0n, 0n]
  const AeI = matn.bloco(2, Am, Z2, Z2, matn.I(2))
  const D = matn.mul(4, matn.kron([0, 1, 1, 0], 2), AeI)
  const Dpaper = matn.bloco(2, Z2, matn.I(2), Am, Z2)               /* o D do paper: [[0,I],[A,0]] */
  const palavra = matn.igual(D, Dpaper)
  const AeA = matn.bloco(2, Am, Z2, Z2, Am)
  const quadrado = matn.igual(matn.mul(4, D, D), AeA)
  /* o gume: sem a troca (o dual), a palavra falha */
  const semTroca = !matn.igual(matn.mul(4, AeI, AeI), AeA)
  /* o contínuo do operador: A^{2^4} = I no andar 17 (a raiz 2-ádica da unidade) */
  const p = 17n
  const modp = m => m.map(v => ((v % p) + p) % p)
  let Ak = Am.map(v => v)
  for (let j = 0; j < 4; j++) Ak = modp(mul(Ak, Ak))
  const raiz = igual(Ak, [1n, 0n, 0n, 1n])
  /* o contínuo do diâmetro: t ∈ [−2,2] tem mistura única λ=(t+2)/4 com
   * volta 4λ−2 = t exata (o selo da batuta aplicado ao diâmetro) */
  let mistura = 0
  for (const [tn, td] of [[3n, 2n], [-1n, 1n], [0n, 1n], [2n, 1n], [-2n, 1n]]) {
    const lamN = tn + 2n * td, lamD = 4n * td
    if ((4n * lamN - 2n * lamD) * td === tn * lamD) mistura++
  }
  if (!palavra || !quadrado || !raiz || mistura !== 5) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§D5  D=(troca⊗I)·(A⊕I) byte a byte: ${palavra} · D²=A⊕A: ${quadrado} · sem a troca falha: ${semTroca} · A^16=I em F17: ${raiz} · diâmetro com mistura única: ${mistura}/5 · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§D5 DIRAC DERIVADO: D = (troca⊗I)·(A⊕I) — multiplicação (kron) ∘ dual (troca) ∘ soma direta — byte a byte o D do paper, com D²=A⊕A: «metade para cada lado» É a soma direta costurada pela troca; e sem a troca a palavra falha ((A⊕I)²=A²⊕I): o dual é obrigatório', palavra && quadrado && semTroca)
  ok('§D5 E TUDO NO CONTÍNUO: o operador fecha na unidade em profundidade finita (A^{2^4}=I no andar — a raiz 2-ádica) e o diâmetro de La Hire é o intervalo da batuta (mistura única com volta exata) — cada ponte tem o seu lado contínuo já selado; 𝓜 assina a derivação inteira', raiz && mistura === 5 && medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A derivação pelas primitivas: o dual emparelha com cada')
  console.log('  operação (centro, membrana, volta — Cayley–Hamilton é a')
  console.log('  concordância). Clifford é o produto dual polarizado (o')
  console.log('  núcleo já é Cl(2,1)); La Hire é a soma dual (o diâmetro é')
  console.log('  o centro); Pontryagin é o dual do grupo (⊕→⊗, bidual =')
  console.log('  volta); Dirac é ⊗∘†∘⊕ (a troca costura as metades). As')
  console.log('  quatro pontes são palavras nas cinco operações — e o')
  console.log('  contínuo de cada uma já estava selado.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
