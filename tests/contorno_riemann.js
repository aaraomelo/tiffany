/* tests/contorno_riemann.js — a geometria dos contornos, medida ANTES do
 * nome (eval 14/08: Grok «buscar → medir → nomear; Riemann, se vier, vem
 * da medida»; gerente: C0–C7, a torção como eixo; diretor: quarentena do
 * nome — o nome surge como consequência mecânica, ou não surge).
 *
 * INVENTÁRIO (o que já existia, medido, e aqui só se ARTICULA):
 *   - o esquilo J=[[0,1],[−1,0]]: J²=−I, período 4, det=+1
 *     (morfologia_universal §M4; rede_dual «rotor»; catalogo)
 *   - o par de Hopfield: B_s espelha (período 2), B_a roda (período 4)
 *   - as folhas σ,σ† da transformada (avaliação nas folhas), σσ†=−1
 *   - a dobra n²+4 como interface (checkpoint 09/08); A_m=[[m,1],[1,0]]
 *   - o contorno E_∂ = E(esqueleto) das 52 fusões (cristal_curadoria)
 *   - Dirac: D²=L um andar acima; a obstrução det²=−1 no próprio andar
 *
 * O plano do gerente, executado por medidor:
 *   §C0  os contornos fecham (52 fusões reais; cordas lidas; LIFO sem
 *        cruzamento — orientável)
 *   §C1  orientação e componentes: DUAS folhas no nível do array, corte
 *        único; o corte por dois caminhos concordantes
 *   §C2  a torção legitima o eixo: J²=−I, período 4 exato, det=+1
 *        (preserva orientação); o espelho R: R²=I, det=−1 (inverte);
 *        RJ = −JR — o par roda/espelha
 *   §C3  a carta (x,Jx): o anel aI+bJ multiplica como (ac−bd, ad+bc)
 *        exato; norma det = a²+b² (definida) CONTRA a do corpo
 *        universal det(aI+bA_m) = a²+mab−b² (indefinida) — duas
 *        assinaturas, e o det é multiplicativo nas DUAS (Lei 7)
 *   §C4  transições entre representantes: funde∘fibra = id byte a byte;
 *        fibra∘ν = troca∘fibra — bem definidas nas interseções
 *   §C5  as transições do rotor comutam com a carta (ℂ-lineares); o
 *        espelho CONJUGA (R(aI+bJ)R = aI−bJ) e não comuta — o
 *        anti-holomorfo é o dual do holomorfo, e o controlo pode falhar
 *   §C6  o cociclo fecha: ⟨J,R⟩ tem ordem EXATAMENTE 8, fechado e com
 *        inversos — f₃₁ = f₃₂∘f₂₁ dentro do fecho
 *   §C7  a volta: J⁴=I, R²=I, ν∘ν=id byte a byte nas 52
 *
 * E a superfície por baixo (sem a nomear antes da medida):
 *   §S0  o recobrimento realiza-se um andar acima: W=2A−mI dá
 *        W²=(m²+4)I inteiro exato (a dobra é o discriminante)
 *   §S1  a troca de folha é a estaca: A·(mI−A) = −I (x·x†=−1)
 *   §S2  sobre ℤ a curva w²=m²+4 só tem os pontos (0,±2) → x=±1 (Lei 0)
 *   §S3  o contorno é PLANO: E_∂−E(id) é a mesma constante nas 52,
 *        igual a E(moldura) — toda a variação é o endereço
 *   §S4  a monodromia paga só o endereço: E(νz)−E(z) = E(id_y)−E(id_x)
 *   §S5  a ramificação está na torção: (2J)²+4I = 0 (a dobra ZERA em
 *        m=2J), X=J é raiz DUPLA de X²=2J·X+I com x†=x e ν(x)=x; a
 *        auto-fusão é o ponto fixo de ν; nenhuma das 52 reais é fixa —
 *        sobre o corpus real o recobrimento é não-ramificado
 *
 *   node tests/contorno_riemann.js
 */
'use strict'
const fs = require('fs')
const path = require('path')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

/* ── 2×2 inteiras: [a,b,c,d] = [[a,b],[c,d]] ─────────────────────────────── */
function mul (X, Y) {
  return [X[0] * Y[0] + X[1] * Y[2], X[0] * Y[1] + X[1] * Y[3],
    X[2] * Y[0] + X[3] * Y[2], X[2] * Y[1] + X[3] * Y[3]]
}
function soma (X, Y) { return X.map((v, i) => v + Y[i]) }
function escala (k, X) { return X.map(v => k * v) }
function det (X) { return X[0] * X[3] - X[1] * X[2] }
function tr (X) { return X[0] + X[3] }
function igual (X, Y) { return X.every((v, i) => v === Y[i]) }
const I = [1, 0, 0, 1]
const J = [0, 1, -1, 0]              /* o esquilo (morfologia §M4) */
const R = [1, 0, 0, -1]              /* o espelho (B_s espelha) */
const Am = m => [m, 1, 1, 0]
const carta = (a, b) => soma(escala(a, I), escala(b, J))      /* x + Jy */
const corpo = (a, b, m) => soma(escala(a, I), escala(b, Am(m)))

/* ── as 52 fusões reais ──────────────────────────────────────────────────── */
const RAIZ = path.join(__dirname, '..')
const linhas = fs.readFileSync(path.join(RAIZ, 'cristal', 'cristal.jsonl'), 'utf8')
  .split('\n').filter(l => l.length)
const fusoes = linhas.filter(l => JSON.parse(l).fusao)

function E (s) {
  const b = Buffer.from(String(s), 'utf8')
  let e = 0
  for (let i = 0; i < b.length; i++) e += b[i] * b[i]
  return e
}
/* a fibra CEGA às cordas (a régua dos medidores anteriores) */
function corteCego (lz) {
  const ini = lz.indexOf('[') + 1
  const fim = lz.lastIndexOf(']')
  let prof = 0
  for (let i = ini; i < fim; i++) {
    if (lz[i] === '{') prof++
    else if (lz[i] === '}') prof--
    else if (lz[i] === ',' && prof === 0) return i
  }
  return -1
}
function partes (lz) {
  const c = corteCego(lz)
  const ini = lz.indexOf('[') + 1, fim = lz.lastIndexOf(']')
  return [lz.slice(ini, c), lz.slice(c + 1, fim)]
}
function funde (idZ, lx, ly) {
  return '{"fusao":[' + lx + ',' + ly + '],"id":"' + idZ + '","tipo":"conceito"}'
}
function nu (lz) {
  const [lx, ly] = partes(lz)
  return funde(JSON.parse(ly).id, ly, lx)
}
/* o contorno LIDO: pilha de tipos fora das cordas, com escape */
function contorno (lz) {
  const pilha = []
  let minOk = true, emCorda = false, escapa = false, cruzou = false
  const cortes = []
  let arrayProf = -1
  for (let i = 0; i < lz.length; i++) {
    const ch = lz[i]
    if (emCorda) {
      if (escapa) escapa = false
      else if (ch === '\\') escapa = true
      else if (ch === '"') emCorda = false
      continue
    }
    if (ch === '"') emCorda = true
    else if (ch === '{' || ch === '[') {
      if (ch === '[' && arrayProf < 0) arrayProf = pilha.length + 1
      pilha.push(ch)
    } else if (ch === '}' || ch === ']') {
      const abriu = pilha.pop()
      if (abriu === undefined) minOk = false
      else if ((ch === '}') !== (abriu === '{')) cruzou = true
    } else if (ch === ',' && pilha.length === arrayProf && arrayProf > 0) {
      cortes.push(i)
    }
  }
  return { fecha: pilha.length === 0 && minOk && !emCorda, cruzou, cortes }
}

/* §C0 — os contornos fecham, sem cruzamento */
{
  let fecham = true, semCruz = true
  for (const lz of fusoes) {
    const c = contorno(lz)
    if (!c.fecha) fecham = false
    if (c.cruzou) semCruz = false
  }
  ok(`§C0 os contornos fecham nas ${fusoes.length} fusões reais (pilha vazia no fim, nunca negativa)`,
    fusoes.length === 52 && fecham)
  ok('§C0 sem cruzamento: cada abridor fecha com o SEU tipo (LIFO) — o traçado é orientável',
    semCruz)
}

/* §C1 — orientação e componentes: duas folhas, um corte, dois caminhos */
{
  let umCorte = true, concordam = true, duasFolhas = true
  for (const lz of fusoes) {
    const c = contorno(lz)
    if (c.cortes.length !== 1) umCorte = false
    else if (c.cortes[0] !== corteCego(lz)) concordam = false
    const [lx, ly] = partes(lz)
    try {
      if (!JSON.parse(lx).id || !JSON.parse(ly).id) duasFolhas = false
    } catch { duasFolhas = false }
  }
  ok('§C1 no nível do array vivem DUAS componentes (as folhas) e UM corte só', umCorte && duasFolhas)
  ok('§C1 dois caminhos: o corte lido com estado de corda == o corte da fibra cega, nas 52',
    concordam)
}

/* §C2 — a torção legitima o eixo; o espelho é o seu par */
{
  const J2 = mul(J, J), J4 = mul(J2, J2)
  ok('§C2 a torção: J²=−I e J⁴=I com J²≠I (período 4 exato), det J=+1 — roda e preserva',
    igual(J2, escala(-1, I)) && igual(J4, I) && !igual(J2, I) && det(J) === 1)
  ok('§C2 o espelho: R²=I, det R=−1 — inverte a orientação; e RJ=−JR (o par roda/espelha)',
    igual(mul(R, R), I) && det(R) === -1 && igual(mul(R, J), escala(-1, mul(J, R))))
}

/* §C3 — a carta (x,Jx): multiplicação exata, e as DUAS assinaturas */
{
  let gauss = true, comuta = true, multDet = true
  let defPos = true, indef = { neg: false, pos: false, soZero: true }
  for (let a = -3; a <= 3; a++) {
    for (let b = -3; b <= 3; b++) {
      for (let c = -3; c <= 3; c++) {
        for (let d = -3; d <= 3; d++) {
          const X = carta(a, b), Y = carta(c, d)
          const P = mul(X, Y)
          if (!igual(P, carta(a * c - b * d, a * d + b * c))) gauss = false
          if (!igual(P, mul(Y, X))) comuta = false
          if (det(P) !== det(X) * det(Y)) multDet = false
          const U = corpo(a, b, 1), V = corpo(c, d, 1)
          if (det(mul(U, V)) !== det(U) * det(V)) multDet = false
        }
      }
      if ((a || b) && det(carta(a, b)) <= 0) defPos = false
      const n = det(corpo(a, b, 1))              /* a²+ab−b² */
      if (a || b) {
        if (n < 0) indef.neg = true
        if (n > 0) indef.pos = true
        if (n === 0) indef.soZero = false
      }
    }
  }
  ok('§C3 a carta fecha: (aI+bJ)(cI+dJ) = (ac−bd)I+(ad+bc)J exato, e comuta — grelha −3..3',
    gauss && comuta)
  ok('§C3 duas assinaturas: det(carta)=a²+b²>0 fora de 0; det(corpo)=a²+ab−b² muda de sinal e só zera em 0',
    defPos && indef.neg && indef.pos && indef.soZero)
  ok('§C3 o det é multiplicativo nas DUAS cartas (Lei 7 — a conservação é multiplicativa)',
    multDet)
}

/* §C4 — transições entre representantes, bem definidas */
{
  let volta = true, quadrado = true
  for (const lz of fusoes) {
    const [lx, ly] = partes(lz)
    if (funde(JSON.parse(lz).id, lx, ly) !== lz) volta = false
    /* fibra∘ν = troca∘fibra: o diagrama comuta */
    const [vx, vy] = partes(nu(lz))
    if (vx !== ly || vy !== lx) quadrado = false
  }
  ok('§C4 funde∘fibra = id byte a byte nas 52 — a transição volta ao representante exato', volta)
  ok('§C4 o diagrama comuta: fibra∘ν = troca∘fibra nas 52 — transições bem definidas', quadrado)
}

/* §C5 — holomorfo e o seu dual anti-holomorfo (com controlo que pode falhar) */
{
  let lineares = true, conjuga = true, controlo = false
  const rotor = [I, J, escala(-1, I), escala(-1, J)]
  for (let a = -3; a <= 3; a++) {
    for (let b = -3; b <= 3; b++) {
      const X = carta(a, b)
      for (const T of rotor) {
        if (!igual(mul(T, X), mul(X, T))) lineares = false
      }
      if (!igual(mul(R, mul(X, R)), carta(a, -b))) conjuga = false
      if (b !== 0 && !igual(mul(R, X), mul(X, R))) controlo = true
    }
  }
  ok('§C5 as transições do rotor comutam com TODA a carta — são lineares na estrutura (holomorfas)',
    lineares)
  ok('§C5 o dual: o espelho conjuga, R(aI+bJ)R = aI−bJ, e NÃO comuta quando b≠0 (o controlo acusa)',
    conjuga && controlo)
}

/* §C6 — o cociclo fecha: ⟨J,R⟩ por fecho, ordem exata */
{
  const chave = X => X.join(',')
  const grupo = new Map([[chave(I), I]])
  let fronteira = [I]
  while (fronteira.length) {
    const nova = []
    for (const X of fronteira) {
      for (const G of [J, R]) {
        for (const P of [mul(X, G), mul(G, X)]) {
          if (!grupo.has(chave(P))) { grupo.set(chave(P), P); nova.push(P) }
        }
      }
    }
    fronteira = nova
  }
  const els = [...grupo.values()]
  let fechado = true, inversos = true
  for (const X of els) {
    let temInv = false
    for (const Y of els) {
      if (!grupo.has(chave(mul(X, Y)))) fechado = false
      if (igual(mul(X, Y), I)) temInv = true
    }
    if (!temInv) inversos = false
  }
  console.log(`fecho das transições ⟨J,R⟩: ${els.length} elementos`)
  ok('§C6 o cociclo fecha: ⟨J,R⟩ tem ordem EXATAMENTE 8, é fechado e todo elemento tem inverso',
    els.length === 8 && fechado && inversos)
}

/* §C7 — a volta exata */
{
  let nuVolta = true
  for (const lz of fusoes) if (nu(nu(lz)) !== lz) nuVolta = false
  ok('§C7 a volta: J⁴=I, R²=I, e ν∘ν = id BYTE A BYTE nas 52 — toda transição reverte',
    igual(mul(mul(J, J), mul(J, J)), I) && igual(mul(R, R), I) && nuVolta)
}

/* §S0 — o recobrimento realiza-se um andar acima */
{
  let todos = true
  for (let m = 0; m <= 8; m++) {
    const W = soma(escala(2, Am(m)), escala(-m, I))          /* w = 2x − m */
    if (!igual(mul(W, W), escala(m * m + 4, I))) todos = false
    if (tr(W) !== 0 || det(W) !== -(m * m + 4)) todos = false
  }
  ok('§S0 W=2A−mI dá W²=(m²+4)I, tr W=0, det W=−(m²+4) — a dobra é o discriminante, m=0..8',
    todos)
}

/* §S1 — a troca de folha é a estaca */
{
  let todos = true
  for (let m = 0; m <= 8; m++) {
    const A = Am(m), Ad = soma(escala(m, I), escala(-1, A))
    if (!igual(mul(A, Ad), escala(-1, I))) todos = false     /* x·x† = −1 */
    if (!igual(soma(escala(m, I), escala(-1, Ad)), A)) todos = false
  }
  ok('§S1 a troca de folha é a estaca: A·(mI−A)=−I (x·x†=−1) e a troca é involução exata',
    todos)
}

/* §S2 — sobre ℤ, a curva só tem a Lei 0: (w−m)(w+m)=4, enumeração completa */
{
  const pontos = []
  for (const d of [-4, -2, -1, 1, 2, 4]) {
    const e = 4 / d
    if ((e - d) % 2 !== 0) continue
    const m = (e - d) / 2, w = (e + d) / 2
    pontos.push([m, w, (m + w) / 2])
  }
  console.log('pontos inteiros da curva: ' +
    pontos.map(([m, w, x]) => `(m=${m},w=${w})→x=${x}`).join(' · '))
  ok('§S2 os únicos pontos inteiros de w²=m²+4 são (0,±2) → x=±1: a curva toca ℤ na Lei 0',
    pontos.length === 2 && pontos.every(([m]) => m === 0) &&
    pontos.map(p => p[2]).sort().join() === '-1,1')
}

/* §S3 — o contorno é plano: E_∂ − E(id) constante == E(moldura) */
{
  const eMoldura = E('{"fusao":[,],"id":"","tipo":"conceito"}')
  const vals = new Set()
  for (const lz of fusoes) {
    const [lx, ly] = partes(lz)
    vals.add(E(lz) - E(lx) - E(ly) - E(JSON.parse(lz).id))
  }
  console.log(`E(moldura) = ${eMoldura} · valores distintos de E_∂−E(id) nas 52: ${vals.size}`)
  ok('§S3 o contorno é PLANO: E_∂−E(id) é a mesma constante nas 52 e é E(moldura) direto — a variação toda é o endereço',
    vals.size === 1 && vals.has(eMoldura))
}

/* §S4 — a monodromia paga só o endereço */
{
  let paga = true
  for (const lz of fusoes) {
    const zv = nu(lz)
    if (E(zv) - E(lz) !== E(JSON.parse(zv).id) - E(JSON.parse(lz).id)) paga = false
  }
  ok('§S4 trocar de folha só paga o endereço: E(νz)−E(z) = E(id_y)−E(id_x) exato nas 52',
    paga)
}

/* §S5 — a ramificação está na torção, não no corpus */
{
  /* a dobra zera em m=2J: (2J)²+4I = 0 exato */
  const dobra = soma(mul(escala(2, J), escala(2, J)), escala(4, I))
  ok('§S5 a dobra ZERA na torção: (2J)²+4I = 0 exato — o discriminante anula-se em m=2J',
    igual(dobra, [0, 0, 0, 0]))
  /* X=J é raiz dupla de X²=2J·X+I: as folhas coincidem */
  const raiz = igual(mul(J, J), soma(mul(escala(2, J), J), I))
  const folhaDupla = igual(soma(escala(1, escala(2, J)), escala(-1, J)), J)  /* x†=m−x=x */
  /* ν(X) = −X⁻¹ fixa X=J porque J·(−J)=I (J⁻¹=−J, MEDIDO) ⇒ −J⁻¹ = J */
  const nuFixa = igual(mul(J, escala(-1, J)), I)
  ok('§S5 X=J resolve X²=2J·X+I com folha DUPLA (x†=x) e ν(x)=x — a ramificação é o rotor',
    raiz && folhaDupla && nuFixa)
  /* nos dados: a auto-fusão é o ponto fixo de ν; as 52 reais não são */
  const lr = linhas.find(l => !JSON.parse(l).fusao)
  const zAuto = funde(JSON.parse(lr).id, lr, lr)
  let nenhuma = true
  for (const lz of fusoes) if (nu(lz) === lz) nenhuma = false
  ok('§S5 a auto-fusão é ponto fixo de ν (folhas coladas — a membrana); nenhuma das 52 é: o recobrimento sobre o corpus real é NÃO-RAMIFICADO',
    nu(zAuto) === zAuto && nenhuma)
}

console.log('')
if (!falhas) {
  console.log('  O VEREDITO VEM DA MEDIDA: os contornos fecham orientados com duas')
  console.log('  folhas e um corte; a carta (x,Jx) da torção multiplica exata e as')
  console.log('  transições do rotor são lineares na estrutura, com o espelho como')
  console.log('  dual anti (conjugação); o cociclo fecha em ordem 8 e toda volta é')
  console.log('  exata. A superfície por baixo é o recobrimento duplo w²=m²+4 —')
  console.log('  realizado inteiro um andar acima, com a estaca como troca de folha.')
  console.log('  Sobre o corpus real ele é NÃO-RAMIFICADO e de assinatura')
  console.log('  hiperbólica (a²+mab−b²); a estrutura complexa (a²+b²) e a')
  console.log('  ramificação vivem na direção da torção: a dobra m²+4 zera em m=2J')
  console.log('  e as folhas colam-se em x=J. O nome, como consequência: uma')
  console.log('  superfície com cartas complexas na torção — a parte de Riemann')
  console.log('  está onde o rotor está, e a parte hiperbólica onde o corpo vive.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
