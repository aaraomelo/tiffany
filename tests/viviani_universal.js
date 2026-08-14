/* tests/viviani_universal.js — a curva de Viviani NO CORPO (ordem da mesa,
 * eval 14/08: «ver como a curva de Viviani se relaciona com o corpo
 * universal», lida na Lei trial; fila selada Viviani → Lei trial →
 * intervalos encaixantes → resíduos → Clifford).
 *
 * A curva (esfera ∩ cilindro tangente) realiza-se INTEIRA no anel pelo
 * relógio da meia-volta: h de ordem 2N dá o meio-ângulo u, h² o ângulo
 * cheio t=2u, e
 *     x = a(1+cos t),  y = a sin t,  z = 2a sin u.
 * O que a medida dá, e os nomes que ela autoriza:
 *
 *   A DOBRA É A LEI: 2cos(2u) = (2cos u)² − 2 é exatamente o R da lib
 *   (renormaliza com d=1) — a curva é o desenho de UMA dobra da
 *   renormalização, e a cascata inteira (j dobras) segue no parâmetro.
 *
 *   O RECOBRIMENTO É O ESPELHO: a base (x,y) fecha em N; o ponto 3D só
 *   em 2N, e a transformação de folha é (y,z) ↦ (y,−z) = o espelho da
 *   lib (R²=I: ν∘ν=id). No espectro (DFT da lib): x,y vivem nas
 *   frequências PARES (a base), z nas ÍMPARES (a folha) — a graduação
 *   ℤ/2 do recobrimento lida em Fourier, com suportes EXATOS.
 *
 *   O NÓ É O PONTO FIXO: a auto-interseção (2a,0,0) é o único ponto da
 *   curva com t=2cos u = 2, e R(2)=2 é o ponto fixo da renormalização
 *   (metronomo_autossimilar §A4). O OUTRO ponto fixo (t=−1) exigiria
 *   h³=1 — e 3 ∤ 2^{2^j}: em NENHUM andar de Fermat (17, 257, 65537)
 *   existe; medido pelo critério de Euler nos três.
 *
 *   VIVIANI NA LEI TRIAL: a altura z satisfaz z³=4a²z (o trial x³=x na
 *   escala 2a) em EXATAMENTE 4 parâmetros — o subgrupo {1,i,−1,−i}:
 *   o espectro trial da altura é realizado pelo bit i (Lei 5).
 *
 * §V0  o palco: ordens exatas de h e h² no anel da lib
 * §V1  esfera E cilindro fecham nos 2N pontos; o cilindro DESLOCADO tem
 *      ZERO pontos (2+2cos t=1 pede ordem 3, ausente) e o acaso falha
 * §V2  a dobra = renormaliza da lib, ponto a ponto e em cascata j=1..5;
 *      gume: a membrana errada (d=2) falha em TODOS os 2N
 * §V3  o recobrimento duplo: base fecha em N, deck = espelho da lib,
 *      espelho²=I, e os pontos fixos do deck são 2 parâmetros → 1 nó
 * §V4  a Lei trial na curva: z³=4a²z em exatamente {1,i,−1,−i}; t=2 só
 *      no nó; t=−1 inexistente nos TRÊS andares (Euler = q−1)
 * §V5  as projeções: parábola z²=2a(2a−x) e Gerono 4a²y²=z²(4a²−z²)
 *      nos 2N; gume: coeficiente mutado falha em exatamente 2N−2
 * §V6  o espectro (dft da lib): suporte de x = {0,2,2N−2}, de y =
 *      {2,2N−2}, de z = {1,2N−1} — pares/ímpares = base/folha
 */
'use strict'
const { anel, dft, renormaliza, mat2, leis } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const A = anel(P)
const N = 16, M = 2 * N                     /* o relógio: 2N = 32 divide 65536 */
const a = 7
const h = A.powm(3, 65536 / M)              /* ordem 2N: o MEIO-ângulo */
const i4 = A.powm(3, 16384)                 /* ordem 4: i² = −1 (o bit) */
const i2 = A.inv(2)

/* §V0 — o palco */
{
  let ordemCerta = A.powm(h, M) === 1 && A.powm(h, N) === P - 1
  for (let k = 1; k < M; k++) if (A.powm(h, k) === 1) ordemCerta = false
  console.log(`\n§V0  h = ${h} tem ordem ${M} no anel(${P}); h^N = −1; i = ${i4}, i² = ${A.mod(i4 * i4)}`)
  ok('§V0 o relógio da meia-volta: h tem ordem 2N exata e h^N=−1 — o ângulo cheio é h²', ordemCerta && A.mod(i4 * i4) === P - 1)
}

/* a curva, ponto a ponto — cos/sin pelo par h^k + h^{−k} (inteiro puro) */
const pts = []
for (let k = 0; k < M; k++) {
  const hk = A.powm(h, k), hki = A.inv(hk)
  const cosu = A.mod((hk + hki) * i2)
  const sinu = A.mod(A.mod(hk - hki) * i2 % P * A.inv(i4))
  const h2 = hk * hk % P, h2i = A.inv(h2)
  const cost = A.mod((h2 + h2i) * i2)
  const sint = A.mod(A.mod(h2 - h2i) * i2 % P * A.inv(i4))
  pts.push({ k, cosu, sinu, x: A.mod(a * (1 + cost)), y: A.mod(a * sint), z: A.mod(2 * a * sinu) })
}

/* §V1 — as duas equações, e os dois controlos */
{
  let esfera = 0, cilindro = 0, deslocado = 0
  for (const p of pts) {
    if (A.mod(p.x * p.x + p.y * p.y + p.z * p.z) === A.mod(4 * a * a)) esfera++
    if (A.mod(A.mod(p.x - a + P) * A.mod(p.x - a + P) + p.y * p.y) === A.mod(a * a)) cilindro++
    if (A.mod(p.x * p.x + p.y * p.y) === A.mod(a * a)) deslocado++          /* centro na ORIGEM */
  }
  /* o controlo a três linhas: o mesmo objeto ao acaso, mesma magnitude */
  let lcg = 5, foraDaEsfera = 0
  for (let n = 0; n < 8; n++) {
    const rnd = () => { lcg = (lcg * 75 + 74) % 65537; return lcg }
    const rx = rnd(), ry = rnd(), rz = rnd()
    if (A.mod(rx * rx + ry * ry + rz * rz) !== A.mod(4 * a * a)) foraDaEsfera++
  }
  console.log(`\n§V1  esfera ${esfera}/${M} · cilindro ${cilindro}/${M} · cilindro DESLOCADO ${deslocado}/${M} · acaso fora da esfera ${foraDaEsfera}/8`)
  ok('§V1 a curva é inteira no anel: esfera x²+y²+z²=4a² E cilindro (x−a)²+y²=a² nos 2N pontos', esfera === M && cilindro === M)
  ok('§V1 o cilindro deslocado (centro 0) tem ZERO pontos — 2+2cos t=1 pede cos t=−1/2, ordem 3, ausente do andar', deslocado === 0)
  ok('§V1 e o acaso com a mesma magnitude falha a esfera — as equações PODEM falhar', foraDaEsfera === 8)
}

/* §V2 — a dobra É o R da lib, ponto a ponto e em cascata */
{
  let pontoAPonto = 0, cascata = 0, gumeErrado = 0
  for (const p of pts) {
    const t1 = A.mod(2 * p.cosu)
    /* caminho 1: o ângulo dobrado lido DIRETO do relógio (h²)^k */
    const g = A.powm(h, 2 * p.k)
    const dobradoRelogio = A.mod(g + A.inv(g))
    /* caminho 2: o R da lib com a membrana d=1 */
    const r = renormaliza({ t: t1, d: 1 })
    if (A.mod(r.t) === dobradoRelogio && r.d === 1) pontoAPonto++
    /* a membrana ERRADA (d=2): difere por 2 sempre — falha nos 2N */
    if (A.mod(renormaliza({ t: t1, d: 2 }).t) !== dobradoRelogio) gumeErrado++
    /* a cascata: j dobras = ângulo 2^j, para j=1..5 — a lei é da lib e a
     * INSTÂNCIA reduz no anel a cada passo (t² de t≈P estoura 2^53 à
     * segunda dobra; reduzir é o papel do anel, não folga da lei) */
    let est = { t: t1, d: 1 }, fecha = true
    for (let j = 1; j <= 5; j++) {
      est = renormaliza(est)
      est = { t: A.mod(est.t), d: A.mod(est.d) }
      const gj = A.powm(h, (p.k << j) % M)
      if (A.mod(est.t) !== A.mod(gj + A.inv(gj))) fecha = false
    }
    if (fecha) cascata++
  }
  console.log(`\n§V2  dobra = R da lib: ${pontoAPonto}/${M} · cascata j=1..5: ${cascata}/${M} · membrana errada falha: ${gumeErrado}/${M}`)
  ok('§V2 2cos(2u) = R(2cos u) com d=1 — a curva desenha UMA dobra da renormalização da lib', pontoAPonto === M)
  ok('§V2 e a cascata segue no parâmetro: R^j lê o ângulo 2^j·u, j=1..5, nos 2N pontos', cascata === M)
  ok('§V2 o gume: a membrana errada (d=2) falha em TODOS os 2N — a lei não é folga', gumeErrado === M)
}

/* §V3 — o recobrimento duplo, e o deck é o espelho da lib */
{
  const E = mat2.espelho                       /* [1,0,0,−1] — dono: a lib */
  let baseFecha = 0, deckEspelho = 0
  for (let k = 0; k < N; k++) {
    const p = pts[k], q = pts[k + N]
    if (p.x === q.x && p.y === q.y) baseFecha++
    /* (y,z) da outra folha = espelho·(y,z) desta */
    const y2 = A.mod(E[0] * p.y + E[1] * p.z), z2 = A.mod(E[2] * p.y + E[3] * p.z)
    if (q.y === y2 && q.z === z2) deckEspelho++
  }
  const involui = mat2.igual(mat2.mul(E, E), mat2.I)
  const fixos = pts.filter(p => p.z === 0)
  const nos = new Set(fixos.map(p => `${p.x},${p.y},${p.z}`))
  console.log(`\n§V3  base fecha em N: ${baseFecha}/${N} · deck=espelho: ${deckEspelho}/${N} · espelho²=I: ${involui} · z=0 em ${fixos.length} parâmetros → ${nos.size} ponto: ${[...nos][0]}`)
  ok('§V3 a base (x,y) fecha em N e o ponto 3D não — o recobrimento é DUPLO', baseFecha === N && pts[1].z !== pts[N + 1].z)
  ok('§V3 a troca de folha é (y,z)↦(y,−z) = o ESPELHO da lib, e espelho²=I — ν∘ν=id, dualidade e não degeneração', deckEspelho === N && involui)
  ok('§V3 os pontos fixos do deck são 2 parâmetros (h=±1) e UM só ponto (2a,0,0) — a auto-interseção é a ramificação', fixos.length === 2 && nos.size === 1 && [...nos][0] === `${2 * a},0,0`)
}

/* §V4 — Viviani na Lei trial */
{
  ok('§V4 a Lei 3 da lib fecha: x³=x caracteriza {−1,0,1}', leis.find(l => l.n === 3).verifica())
  /* o trial da ALTURA: z³ = 4a²·z ⟺ sin u ∈ {−1,0,1} */
  const trial = pts.filter(p => A.mod(p.z * p.z % P * p.z) === A.mod(4 * a * a % P * p.z))
  const emI = trial.every(p => [1, i4, P - 1, A.mod(-i4)].includes(A.powm(h, p.k)))
  /* o ponto fixo do R na curva: t=2cos u = 2 */
  const tFixo = pts.filter(p => A.mod(2 * p.cosu) === 2)
  /* e o OUTRO ponto fixo (t=−1 ⟺ h²+h+1=0) não vive em NENHUM andar: Euler(−3) = q−1 */
  const andares = [17, 257, 65537]
  const semOrdem3 = andares.every(q => { const B = anel(q); return B.powm(q - 3, (q - 1) / 2) === q - 1 })
  console.log(`\n§V4  z³=4a²z em ${trial.length} parâmetros (h^k ∈ {1,i,−1,−i}: ${emI}) · t=2 em ${tFixo.length} (k=${tFixo.map(p => p.k)}) · (−3) não-resíduo nos 3 andares: ${semOrdem3}`)
  ok('§V4 o trial da altura é o BIT: z³=4a²z em exatamente 4 parâmetros — o subgrupo {1,i,−1,−i} da Lei 5', trial.length === 4 && emI)
  ok('§V4 o NÓ é o ponto fixo: t=2 só em h=1, e R(2)=2 — a auto-interseção é onde a renormalização pára', tFixo.length === 1 && tFixo[0].k === 0)
  ok('§V4 o outro fixo (t=−1) exigiria ordem 3: (−3)^{(q−1)/2}=q−1 nos TRÊS andares de Fermat — não existe na escada', semOrdem3)
}

/* §V5 — as projeções: a parábola e a Gerono */
{
  let parab = 0, gerono = 0, gumeParab = 0
  for (const p of pts) {
    if (A.mod(p.z * p.z) === A.mod(2 * a % P * A.mod(2 * a - p.x + P))) parab++
    if (A.mod(4 * a * a % P * (p.y * p.y % P)) === A.mod(p.z * p.z % P * A.mod(4 * a * a - p.z * p.z % P + P))) gerono++
    if (A.mod(p.z * p.z) !== A.mod(3 * a % P * A.mod(2 * a - p.x + P))) gumeParab++   /* 3a: só escapa onde x=2a */
  }
  console.log(`\n§V5  parábola ${parab}/${M} · Gerono ${gerono}/${M} · coeficiente mutado falha ${gumeParab}/${M} (só o nó escapa: x=2a nos 2 parâmetros dele)`)
  ok('§V5 o corte xz é a parábola z²=2a(2a−x) e o corte yz é a Gerono 4a²y²=z²(4a²−z²), nos 2N pontos', parab === M && gerono === M)
  ok('§V5 o gume: 3a no lugar de 2a falha em exatamente 2N−2 — os 2 que escapam são os parâmetros do nó', gumeParab === M - 2)
}

/* §V6 — o espectro pela dft da lib: pares = base, ímpares = folha */
{
  const xs = pts.map(p => p.x), ys = pts.map(p => p.y), zs = pts.map(p => p.z)
  const sup = v => dft(v, A, h).map((c, f) => [c, f]).filter(([c]) => c !== 0).map(([, f]) => f).join()
  const sx = sup(xs), sy = sup(ys), sz = sup(zs)
  console.log(`\n§V6  suporte espectral: x → {${sx}} · y → {${sy}} · z → {${sz}}  (M=${M})`)
  ok('§V6 x e y vivem nas frequências PARES {0,2,2N−2}/{2,2N−2} — o tom da BASE (ângulo cheio)', sx === `0,2,${M - 2}` && sy === `2,${M - 2}`)
  ok('§V6 z vive nas ÍMPARES {1,2N−1} — o tom da FOLHA (meio-ângulo): a graduação ℤ/2 do recobrimento lida em Fourier', sz === `1,${M - 1}`)
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A curva de Viviani é o desenho de UMA dobra da renormalização:')
  console.log('  o nó é o ponto fixo R(2)=2, o recobrimento duplo é o espelho')
  console.log('  (pares/ímpares no espectro), e o trial da altura é o bit i.')
  console.log('  O outro ponto fixo (t=−1) não vive na escada de Fermat: 3∤2^{2^j}.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
