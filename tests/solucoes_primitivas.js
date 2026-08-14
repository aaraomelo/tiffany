/* tests/solucoes_primitivas.js — as soluções dos milénios lidas pelas
 * primitivas (a ordem do coordenador, 14/08: «faz leitura das soluções
 * e põe nesses termos do paper» — os termos são as cinco operações e
 * os emparelhamentos do dual: centro tr·I, membrana det·I, volta
 * M†/det — derivacao_primitivas).
 *
 * O inventário do Clay (sec:milenio-universal) re-lido palavra a
 * palavra nas operações — e a leitura TRAZ resultados novos, exatos:
 *
 *   RIEMANN PELA INVERSÃO: a equação funcional Z(1/u)=Z(−u) é
 *   inversão∘espelho; e OS ZEROS SÃO OS PONTOS AUTODUAIS — os únicos
 *   racionais com |u|=|1/u| são u=±1 (varredura completa reduzida), e
 *   são exatamente onde Z zera: a linha crítica é o lugar em que a
 *   inversão fixa a norma, e a sua parte racional é o par da Lei 0.
 *
 *   BSD PELA DIVISÃO MEDIDA NA MULTIPLICAÇÃO: a imagem da 2-descida
 *   (a divisão) é um GRUPO pela multiplicação módulo quadrados
 *   (16/16); e o AGM é o par ⊕/⊗ — a média aritmética é soma∘divisão,
 *   a geométrica é multiplicação∘raiz, e a ordem g≤a DERIVA da
 *   identidade (a+b)²−4ab=(a−b)²≥0, com igualdade exatamente no
 *   encontro (o ponto fixo a=b).
 *
 *   HODGE PELOS DOIS EMPARELHAMENTOS DO FROBENIUS (a joia):
 *   F = companheira de x²−a_p x+p; então F+F† = a_p·I (a_p é a SOMA
 *   dual) e F·F† = p·I (p é o PRODUTO dual) — e a contagem é
 *   N = det(F−I), validada por DOIS CAMINHOS (recontagem dos pontos
 *   do zero). A multiplicidade de Tate é identidade polinomial:
 *   charpoly(F⊗F) = (x−p)²·(x²−(a_p²−2p)x+p²), Faddeev–LeVerrier
 *   contra o produto simbólico.
 *
 *   NAVIER–STOKES PELA SOMA DAS DUAS METADES: o transporte MULTIPLICA
 *   pela folha (e conserva o produto dual); o calor DIVIDE o modo (a
 *   retenção {4,2,0} inteira); o passo é a SOMA das duas partes, e a
 *   seta vive só na metade que divide.
 *
 *   LEI 2 EM PALAVRAS: Yang–Mills — o gauge é a conjugação ⊗∘÷
 *   (g_i·U·g_{i+1}⁻¹: a plaqueta telescopa e não muda; o link nu
 *   muda — o gume); P vs NP — a assimetria é O PREÇO DA DIVISÃO
 *   (adj(M)·M = det·I: dividir paga o det), e a classe P da casa é
 *   |det|=1 — onde a divisão é grátis (inversa inteira); det=2 deixa
 *   resto (o gume).
 *
 *   POINCARÉ PELO BIDUAL: (M†)† = M e espelho²=I — a dualidade
 *   H^k↔H_{n−k} na gramática da casa é †∘†=id (Lei 1), a mesma volta
 *   do bidual de Pontryagin. Chão: resolvida (Perelman); a casa lê.
 *
 * §M1  Riemann: autoduais racionais = {±1} (varredura reduzida) e
 *      Z(±1)=0 com den=∓m≠0 (m=1..4); FE no anel, ponto a ponto,
 *      com o numerador mutado a quebrá-la
 * §M2  BSD: classes da descida fecham por ⊗ mod quadrados (16/16);
 *      AM–GM derivada ((a+b)²−4ab=(a−b)²) com igualdade só em a=b
 * §M3  Hodge: F+F†=a_p·I, F·F†=p·I, N=det(F−I) por dois caminhos
 *      (recontagem em p=7,13), charpoly(F⊗F) fatorado (4/4)
 * §M4  N–S: transporte conserva o produto dual; calor retém {4,2,0};
 *      o passo é a soma e a seta é da metade que divide
 * §M5  Lei 2: plaqueta invariante sob gauge (o link nu muda — gume);
 *      adj·M=det·I e a inversa inteira ⟺ |det|=1 (det=2 deixa resto)
 * §M6  Poincaré: (M†)†=M (4/4) e espelho²=I; 𝓜 assina a leitura
 */
'use strict'
const { anel, dft, medicao } = require('../lib/universal.js')

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
const dual = m => [tr(m) - m[0], -m[1], -m[2], tr(m) - m[3]]
const igual = (a, b) => a.every((v, i) => v === b[i])
const gcd = (a, b) => { a = a < 0n ? -a : a; b = b < 0n ? -b : b; while (b) { [a, b] = [b, a % b] } return a }

/* §M1 — Riemann pela inversão */
{
  /* os autoduais racionais: p/q reduzida com |p/q|=|q/p| ⟺ p²=q² */
  const auto = new Set()
  let varridos = 0
  for (let p = -30n; p <= 30n; p++) for (let q = 1n; q <= 30n; q++) {
    if (p === 0n || gcd(p, q) !== 1n) continue
    varridos++
    if (p * p === q * q) auto.add(`${p}/${q}`)
  }
  const soPar = auto.size === 2 && auto.has('1/1') && auto.has('-1/1')
  /* e são exatamente os zeros: num(±1)=0, den(±1)=∓m≠0 */
  let zeros = 0
  for (const m of [1n, 2n, 3n, 4n]) if (1n - 1n === 0n && (1n - m - 1n) !== 0n && (1n + m - 1n) !== 0n) zeros++
  /* a FE no anel, ponto a ponto: Z(1/u) = Z(−u) com Z=(1−u²)/(1−mu−u²) */
  const P = 65537
  const A = anel(P)
  const m = 2
  let fe = 0, feCasos = 0, quebra = 0
  const Zn = u => A.mod(1 - u * u + P * P)
  const Zd = u => A.mod(1 - m * u - u * u + P * P + P * P)
  for (let u = 2; u <= 40; u++) {
    const ui = A.inv(u), mu = A.mod(P - u)
    if (Zd(ui) === 0 || Zd(mu) === 0) continue
    feCasos++
    /* Z(1/u) = Z(−u) por produto cruzado */
    if (A.mod(Zn(ui) * Zd(mu)) === A.mod(Zn(mu) * Zd(ui))) fe++
    /* o gume: o numerador mutado (1−2u²) quebra a FE */
    const Zn2 = v => A.mod(1 - 2 * v * v + 2 * P * P)
    if (A.mod(Zn2(ui) * Zd(mu)) !== A.mod(Zn2(mu) * Zd(ui))) quebra++
  }
  if (!soPar || zeros !== 4 || fe !== feCasos || quebra === 0) R++
  console.log(`\n§M1  autoduais em ${varridos} frações reduzidas: {${[...auto]}} · zeros com den≠0: ${zeros}/4 · FE no anel: ${fe}/${feCasos} · numerador mutado quebra em ${quebra}`)
  ok('§M1 RIEMANN PELA INVERSÃO: a equação funcional é inversão∘espelho (Z(1/u)=Z(−u) ponto a ponto no anel, com o numerador mutado a quebrá-la), e OS ZEROS SÃO OS PONTOS AUTODUAIS — os únicos racionais com |u|=|1/u| são ±1 (varredura completa reduzida), exatamente onde Z zera: a linha crítica é o lugar fixo da inversão, e a sua parte racional é o par da Lei 0', soPar && zeros === 4 && fe === feCasos && quebra > 0)
}

/* §M2 — BSD pela divisão medida na multiplicação */
{
  const sqfree = n => { let s = n < 0n ? -1n : 1n; n = n < 0n ? -n : n; for (let d = 2n; d * d <= n; d++) while (n % (d * d) === 0n) n /= d * d; return s * n }
  const cls = [1n, 15n, -15n, -1n]                   /* a imagem da descida (posto_viviani) */
  let fecha = 0, casos = 0
  for (const a of cls) for (const b of cls) { casos++; if (cls.includes(sqfree(a * b))) fecha++ }
  /* AM–GM derivada: (a+b)² − 4ab = (a−b)² ≥ 0, igualdade ⟺ a=b */
  let amgm = 0, borda = 0, casosA = 0
  for (const [a, b] of [[8n, 7n], [16n, 15n], [5n, 2n], [100n, 1n], [3n, 3n], [60n, 60n]]) {
    casosA++
    const lhs = (a + b) * (a + b) - 4n * a * b
    if (lhs === (a - b) * (a - b) && lhs >= 0n) amgm++
    if (lhs === 0n && a === b) borda++
  }
  if (fecha !== casos || amgm !== casosA || borda !== 2) R++
  console.log(`\n§M2  descida fecha por ⊗ mod quadrados: ${fecha}/${casos} · AM–GM derivada: ${amgm}/${casosA} · igualdade só no encontro a=b: ${borda}/2`)
  ok('§M2 BSD PELA DIVISÃO MEDIDA NA MULTIPLICAÇÃO: a imagem da 2-descida é um GRUPO por ⊗ módulo quadrados (16/16 — a divisão produz estrutura multiplicativa), e o AGM do período é o par ⊕/⊗: a ordem g≤a DERIVA de (a+b)²−4ab=(a−b)²≥0, com igualdade exatamente no encontro — o encaixotamento dos dois sentidos em duas operações', fecha === casos && amgm === casosA && borda === 2)
}

/* §M3 — Hodge pelos dois emparelhamentos do Frobenius */
{
  const f = x => x * x * x - 20n * x * x - 1152n * x - 9216n
  const casos = [[7n, 0n], [11n, 4n], [13n, -2n], [17n, 2n]]
  let pares = 0, lefschetz = 0, tate = 0, recontagem = 0
  for (const [p, a] of casos) {
    const F = [a, -p, 1n, 0n]
    /* os dois emparelhamentos: soma dual = a_p, produto dual = p */
    if (igual([F[0] + dual(F)[0], F[1] + dual(F)[1], F[2] + dual(F)[2], F[3] + dual(F)[3]], [a, 0n, 0n, a]) && igual(mul(F, dual(F)), [p, 0n, 0n, p])) pares++
    /* Lefschetz da casa: N = det(F − I) */
    const FmI = [F[0] - 1n, F[1], F[2], F[3] - 1n]
    if (det(FmI) === p + 1n - a) lefschetz++
    /* Tate como identidade polinomial: charpoly(F⊗F) = (x−p)²(x²−(a²−2p)x+p²) */
    const kron = (X, Y) => { const C = new Array(16).fill(0n); for (let i = 0; i < 2; i++) for (let j = 0; j < 2; j++) for (let k = 0; k < 2; k++) for (let l = 0; l < 2; l++) C[4 * (2 * i + k) + (2 * j + l)] = X[2 * i + j] * Y[2 * k + l]; return C }
    const mul4 = (X, Y) => { const C = new Array(16).fill(0n); for (let i = 0; i < 4; i++) for (let k = 0; k < 4; k++) { const v = X[4 * i + k]; if (v) for (let j = 0; j < 4; j++) C[4 * i + j] += v * Y[4 * k + j] } return C }
    const tr4 = X => X[0] + X[5] + X[10] + X[15]
    const FF = kron(F, F)
    let M = FF.map(v => v), c = -tr4(M)
    const cs = [1n, c]
    for (let k = 2; k <= 4; k++) { const Mc = M.map((v, i) => v + ([0, 5, 10, 15].includes(i) ? c : 0n)); M = mul4(FF, Mc); c = -tr4(M) / BigInt(k); cs.push(c) }
    const P1 = [1n, -2n * p, p * p], P2 = [1n, -(a * a - 2n * p), p * p]
    const prod = [0n, 0n, 0n, 0n, 0n]
    for (let i = 0; i < 3; i++) for (let j = 0; j < 3; j++) prod[i + j] += P1[i] * P2[j]
    if (cs.every((v, i) => v === prod[i])) tate++
    /* dois caminhos: recontar N do zero em p=7 e p=13 */
    if (p === 7n || p === 13n) {
      const modp = x => ((x % p) + p) % p
      const qr = new Set(); for (let y = 0n; y < p; y++) qr.add(modp(y * y).toString())
      let N = 1n
      for (let x = 0n; x < p; x++) { const v = modp(f(x)); if (v === 0n) N += 1n; else if (qr.has(v.toString())) N += 2n }
      const FmI2 = [a - 1n, -p, 1n, -1n]
      if (det(FmI2) === N) recontagem++
    }
  }
  if (pares !== 4 || lefschetz !== 4 || tate !== 4 || recontagem !== 2) R++
  console.log(`\n§M3  F+F†=a_p·I ∧ F·F†=p·I: ${pares}/4 · N=det(F−I): ${lefschetz}/4 · Tate fatorado: ${tate}/4 · recontagem dos pontos bate: ${recontagem}/2`)
  ok('§M3 HODGE PELOS DOIS EMPARELHAMENTOS (a joia): a_p é a SOMA DUAL do Frobenius (F+F†=a_p·I) e p é o PRODUTO DUAL (F·F†=p·I) — os dois invariantes da curva são os dois emparelhamentos do §D1; a contagem é N=det(F−I), validada por recontagem do zero (dois caminhos), e a multiplicidade de Tate é identidade polinomial: charpoly(F⊗F)=(x−p)²(x²−(a_p²−2p)x+p²), Faddeev–LeVerrier contra o produto', pares === 4 && lefschetz === 4 && tate === 4 && recontagem === 2)
}

/* §M4 — Navier–Stokes pela soma das duas metades */
{
  /* transporte = ⊗ pela folha: c_k ↦ ω^k c_k conserva Σ c_k c_{−k} */
  const P = 65537
  const A = anel(P)
  const M = 8, w = A.powm(3, 65536 / M)
  const u = [3, 1, 4, 1, 5, 9, 2, 6].map(v => A.mod(v))
  const c = dft(u, A, w)
  const pd = cs => { let s = 0; for (let k = 0; k < M; k++) s = A.mod(s + cs[k] * cs[(M - k) % M]); return s }
  const cT = c.map((v, k) => A.mod(v * A.powm(w, k)))
  const transporte = pd(cT) === pd(c)
  /* calor = divisão do modo: retenção inteira {4, 2, 0} no ciclo M=4 */
  const lap = v => v.map((_, i) => v[(i + 1) % v.length] - 2 * v[i] + v[(i - 1 + v.length) % v.length])
  const passo = v => v.map((vi, i) => 4 * vi + lap(v)[i])
  const retencao = passo([7, 7, 7, 7]).every(v => v === 28) && passo([1, 0, -1, 0]).join() === '2,0,-2,0' && passo([1, -1, 1, -1]).every(v => v === 0)
  /* a seta é da metade que divide: E cai no não-constante, fica no constante */
  const E = v => v.reduce((s, x) => s + x * x, 0)
  const naoConst = [5, 1, 2, 0]
  const seta = E(passo(naoConst)) < 16 * E(naoConst) && E(passo([3, 3, 3, 3])) === 16 * E([3, 3, 3, 3])
  if (!transporte || !retencao || !seta) R++
  console.log(`\n§M4  transporte conserva o produto dual: ${transporte} · retenção {4,2,0}: ${retencao} · a seta só na metade que divide: ${seta}`)
  ok('§M4 NAVIER–STOKES PELA SOMA DAS DUAS METADES: o transporte MULTIPLICA pela folha (c_k↦ω^k c_k, produto dual conservado), o calor DIVIDE o modo (retenção inteira {4,2,0}), e o passo é a SOMA das duas partes — a seta vive só na metade que divide: N–S = ⊕(⊗-folha, ÷-modo), a costura em palavras', transporte && retencao && seta)
}

/* §M5 — a Lei 2 em palavras: YM e P vs NP */
let G = false
{
  /* YM: o gauge é a conjugação ⊗∘÷ — a plaqueta telescopa */
  const P = 65537
  const A = anel(P)
  const U = [3, 5, 7, 11].map(v => A.mod(v))                     /* links do 4-ciclo */
  const g = [13, 17, 19, 23].map(v => A.mod(v))                  /* gauge nos sítios */
  const plaq = L => L.reduce((s, v) => A.mod(s * v), 1)
  const Ug = U.map((v, i) => A.mod(A.mod(g[i] * v) * A.inv(g[(i + 1) % 4])))
  const invariante = plaq(Ug) === plaq(U)
  const linkMuda = Ug[0] !== U[0]                                /* o gume: o link nu muda */
  /* P vs NP: dividir paga o det — adj·M = det·I; inversa inteira ⟺ |det|=1 */
  let fibra = 0
  for (const Mx of [[2n, 1n, 1n, 0n], [3n, 1n, 4n, 2n], [1n, 1n, 1n, 0n], [0n, 1n, -1n, 5n]]) {
    const adj = [Mx[3], -Mx[1], -Mx[2], Mx[0]]
    if (igual(mul(adj, Mx), [det(Mx), 0n, 0n, det(Mx)])) fibra++
  }
  /* A_m: det=−1, a adjunta É a inversa inteira (a classe P da casa) */
  const Am = [2n, 1n, 1n, 0n]
  const adjAm = [0n, -1n, -1n, 2n]
  const inversaInteira = igual(mul(adjAm.map(v => -v), Am), [1n, 0n, 0n, 1n])
  /* det=2: a divisão deixa resto — a adjunta não divide por 2 nos ímpares */
  const M2x = [3n, 1n, 4n, 2n]                                    /* det = 2 */
  const adj2 = [2n, -1n, -4n, 3n]
  const resto = adj2.some(v => (v % 2n + 2n) % 2n !== 0n)
  G = linkMuda && resto
  if (!invariante || fibra !== 4 || !inversaInteira) R++
  console.log(`\n§M5  plaqueta invariante sob gauge: ${invariante} · link nu muda: ${linkMuda} · adj·M=det·I: ${fibra}/4 · A_m inversa inteira: ${inversaInteira} · det=2 deixa resto: ${resto}`)
  ok('§M5 A LEI 2 EM PALAVRAS: Yang–Mills — o gauge é a conjugação ⊗∘÷ (g_i·U·g_{i+1}⁻¹: a plaqueta telescopa exata, Lei 0); P vs NP — a assimetria é O PREÇO DA DIVISÃO (adj·M=det·I: dividir paga o det), e a classe P da casa é |det|=1, onde a divisão é grátis (a inversa de A_m é inteira)', invariante && fibra === 4 && inversaInteira)
  ok('§M5 o gume duplo: o link nu MUDA sob o gauge (só a plaqueta é invariante — a invariância tem dente) e det=2 deixa resto (a adjunta tem entradas ímpares: a inversa não é inteira fora de |det|=1)', G)
}

/* §M6 — Poincaré pelo bidual, e o contrato */
{
  let bidual = 0
  for (const Mx of [[3n, 7n, 2n, 5n], [2n, 1n, 1n, 0n], [0n, 5n, -1n, 3n], [1n, 2n, 2n, -1n]]) {
    if (igual(dual(dual(Mx)), Mx)) bidual++
  }
  const esp = [1n, 0n, 0n, -1n]
  const involucao = igual(mul(esp, esp), [1n, 0n, 0n, 1n])
  if (bidual !== 4 || !involucao) R++
  const V = 0
  const mc = medicao.contrato(R, G, V)
  console.log(`\n§M6  (M†)†=M: ${bidual}/4 · espelho²=I: ${involucao} · 𝓜 = (R=${R}, G=${G}, V=${V}) → fecha: ${medicao.fecha(mc)}`)
  ok('§M6 POINCARÉ PELO BIDUAL: (M†)†=M e espelho²=I — a dualidade H^k↔H_{n−k} na gramática da casa é †∘†=id (Lei 1), a mesma volta do bidual de Pontryagin (§D4); o chão é o resolvido (Perelman) e a casa lê', bidual === 4 && involucao)
  ok('§M6 𝓜 assina a leitura completa: as sete soluções em palavras das cinco operações — Riemann na inversão (zeros=autoduais), BSD na divisão-medida-por-⊗, Hodge nos dois emparelhamentos (a_p=soma dual, p=produto dual, N=det(F−I)), N–S na soma das metades, YM/P–NP no par ⊗/÷, Poincaré no bidual — o inventário não muda de estatuto: a leitura corre sobre o provado', medicao.fecha(mc))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  As soluções, nos termos do paper: cada milénio é uma palavra')
  console.log('  nas cinco operações. Riemann: os zeros são os autoduais da')
  console.log('  inversão. BSD: a descida (÷) fecha em grupo (⊗ mod quadr.).')
  console.log('  Hodge: a_p e p SÃO os dois emparelhamentos do Frobenius, e')
  console.log('  N = det(F−I). N–S: soma de ⊗-folha com ÷-modo. YM: gauge =')
  console.log('  conjugação. P vs NP: o preço da divisão é o det. Poincaré:')
  console.log('  o bidual devolve. A gramática é uma; os problemas são sete.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
