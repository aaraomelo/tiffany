/* tests/toro_histerese.js — o espaço de fase da batuta, AUDITADO
 * (ordem da mesa, eval 14/08: gerente «mediria o toro e a histerese;
 * deixaria Riemann para depois»; diretor: «auditar o espaço de fase, não
 * teorizar» — o nome zeta/Riemann NÃO entra; os períodos entram como DADO).
 *
 * INVENTÁRIO (o que já existia; aqui articula-se, nada se re-funda):
 *   - histerese de Peano: H=(X,B,I), batuta I∈{−1,0,+1} — expansão
 *     (B→X), retenção, retração (X→B); Metrónomo λ⁺+λ⁻=0
 *     (corpo_peano thm:histerese)
 *   - Maestro = tick∘batuta∘Π realiza a retração π_k (thm:proj-maestro)
 *   - a companheira A_m (o tick), det=−1: a unidade da borda |N|=1
 *   - a adjunção δ⊣ε (morfologia; abertura α=δε ≤ id ≤ φ=εδ)
 *   - o anel ℤ_65537 (o primo de Fermat da casa)
 *
 * O plano do gerente, em quatro camadas:
 *   §H0  o corpo na borda: |det A_m|=1 (unidade); a DILATAÇÃO radial
 *        multiplica a massa |det| exato (Lei 7) e a contração devolve —
 *        radial = massa; angular = a caminhada que preserva |det|
 *   §H1  O TORO EXISTE: mod p a batuta fecha em círculo (T = ord(A_m),
 *        medido e MINIMAL); a folha é o segundo círculo (det alterna
 *        ±1 a cada tick — período 2); o toro das unidades {±A^k} tem
 *        tamanho medido (2T ou T conforme −I ∈ ⟨A⟩, verificado)
 *   §H2  a batuta caminha e a volta fecha: sobre um estado REAL (escada
 *        de um registo do cristal), a caminhada fechada (λ⁺+λ⁻=0)
 *        devolve o estado EXATO com R_total=(0,0,0,0,0) → RETAIN; a
 *        não-fechada NÃO devolve → REOPEN; e a órbita completa (T
 *        ticks) volta ao mesmo estado — o índice de rotação é 1
 *   §H3  a massa fica no CENTRO: nas 52 fusões reais, M(z)=E(z)−E_∂ é
 *        invariante sob a monodromia (a caminhada troca folhas e paga
 *        só o endereço — o centro não se move); e |det| do estado
 *        matricial é constante ao longo da órbita da batuta
 *   §H4  a histerese SELECIONA a folha admissível: α=δε ≤ id ≤ φ=εδ
 *        sobre o suporte real (entrar ≠ sair — o laço); α idempotente
 *        (a seleção estabiliza); os ABERTOS voltam exatos — são as
 *        folhas admissíveis; os isolados são a memória perdida
 *   §H5  O ESPECTRO COMO DADO (sem nome): T_m = ord(A_m mod p) para
 *        m=1..6, com a dicotomia medida — folhas separadas mod p
 *        (σ existe: T_m | p−1 = 2^16) ou inertes (não existe raiz);
 *        os k com pontos fixos não-triviais de A^k são os múltiplos
 *        do período da folha (det(A^k−I) ≡ 0), verificado
 *
 *   node tests/toro_histerese.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { Universal, mat2 } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')
const U = Universal(sigmaPeano)

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537
const { mul, escala, det, igual, I, Am } = mat2

/* ── aritmética 2×2 mod p ────────────────────────────────────────────────── */
const mod = x => ((x % P) + P) % P
const mmod = X => X.map(mod)
function mulp (X, Y) { return mmod(mul(X, Y)) }
function powp (X, e) {
  let R = [1, 0, 0, 1], B = X.slice()
  while (e > 0) {
    if (e & 1) R = mulp(R, B)
    B = mulp(B, B)
    e >>= 1
  }
  return R
}
/* A⁻¹ inteira: det A_m = −1 ⇒ A⁻¹ = −adj(A) = [0,1,1,−m] (verificada) */
const AmInv = m => mmod([0, 1, 1, -m])

/* ── as fixtures reais ───────────────────────────────────────────────────── */
const RAIZ = path.join(__dirname, '..')
const linhas = fs.readFileSync(path.join(RAIZ, 'cristal', 'cristal.jsonl'), 'utf8')
  .split('\n').filter(l => l.length)
const fusoes = linhas.filter(l => JSON.parse(l).fusao)

/* §H0 — o corpo na borda; radial = massa, pela Lei 7 */
{
  let unidades = true, radial = true
  for (let m = 1; m <= 6; m++) {
    if (Math.abs(det(Am(m))) !== 1) unidades = false
    if (!igual(mmod(mul(Am(m), AmInv(m))), I)) unidades = false
  }
  /* a dilatação radial: X não-unidade multiplica a massa EXATO */
  const X = [2, 1, 1, 1]                       /* |det| = 1? det=1 — unidade! */
  const Y = [2, 0, 0, 1]                       /* det = 2: dilata */
  const Z = mul(Am(1), Am(2))
  if (Math.abs(det(mul(Y, Z))) !== 2 * Math.abs(det(Z))) radial = false
  if (Math.abs(det(mul(Am(1), Z))) !== Math.abs(det(Z))) radial = false
  ok('§H0 o corpo na borda: |det A_m|=1 e a inversa é inteira (m=1..6) — a unidade da borda',
    unidades)
  ok('§H0 radial = massa: a dilatação (|det|=2) multiplica a massa exato; a batuta (unidade) preserva-a',
    radial && Math.abs(det(X)) === 1)
}

/* §H1 — O TORO: a batuta fecha em círculo mod p; a folha é o 2.º círculo */
function ordem (X) {
  let A = X.slice(), k = 1
  const CAP = 4 * (P + 1)
  while (!igual(A, I) && k <= CAP) { A = mulp(A, X); k++ }
  return k > CAP ? -1 : k
}
const T1 = ordem(mmod(Am(1)))
{
  const A = mmod(Am(1))
  /* minimalidade: nenhum divisor primo q de T dá A^(T/q) = I */
  const minimal = (() => {
    let resto = T1
    const primos = []
    for (let q = 2; q * q <= resto; q++) {
      if (resto % q === 0) { primos.push(q); while (resto % q === 0) resto /= q }
    }
    if (resto > 1) primos.push(resto)
    return primos.every(q => !igual(powp(A, T1 / q), I))
  })()
  ok(`§H1 a batuta fecha: ord(A_1 mod ${P}) = ${T1}, com A^T = I verificado e T minimal`,
    T1 > 0 && igual(powp(A, T1), I) && minimal)
  /* a folha: det alterna −1,+1 a cada tick — o círculo de período 2 */
  let alterna = true
  let Acc = I
  for (let k = 1; k <= 8; k++) {
    Acc = mulp(Acc, A)
    const d = mod(det(Acc))
    if (k % 2 === 1 && d !== P - 1) alterna = false
    if (k % 2 === 0 && d !== 1) alterna = false
  }
  ok('§H1 a folha é o segundo círculo: det(A^k) alterna −1,+1 — a batuta troca a folha a cada tick',
    alterna)
  /* o toro das unidades {±A^k}: tamanho medido, conforme −I ∈ ⟨A⟩ */
  const temMenosI = (() => {
    let B = A.slice()
    for (let k = 1; k <= T1; k++) {
      if (igual(B, mmod(escala(-1, I)))) return true
      B = mulp(B, A)
    }
    return false
  })()
  const vistos = new Set()
  {
    let B = I
    for (let k = 0; k < T1; k++) {
      vistos.add(B.join(','))
      vistos.add(mmod(escala(-1, B)).join(','))
      B = mulp(B, A)
    }
  }
  const esperado = temMenosI ? T1 : 2 * T1
  console.log(`toro das unidades: |{±A^k}| = ${vistos.size} · −I ∈ ⟨A⟩: ${temMenosI} · esperado ${esperado}`)
  ok('§H1 o toro das unidades tem o tamanho previsto pela pertença de −I (as duas voltas fecham)',
    vistos.size === esperado)
}

/* §H2 — a caminhada da batuta sobre um estado REAL */
{
  const A = mmod(Am(1)), Ai = AmInv(1)
  const e0 = U.escada(linhas[0])
  const v0 = [mod(e0.f1), mod(e0.f2)]
  const passo = (v, i) => {
    const M = i === 1 ? A : i === -1 ? Ai : I
    return [mod(M[0] * v[0] + M[1] * v[1]), mod(M[2] * v[0] + M[3] * v[1])]
  }
  const anda = (v, palavra) => {
    for (const i of palavra) v = passo(v, i)
    return v
  }
  const fechada = [1, 1, 0, -1, 1, -1, -1]        /* λ⁺+λ⁻ = 3−3 = 0 */
  const aberta = [1, 1, 0, -1, 1, -1]             /* λ⁺+λ⁻ = 3−2 = 1 */
  const vF = anda(v0.slice(), fechada)
  const vA = anda(v0.slice(), aberta)
  const rot = (a, b) => U.residuoTotal([['estado', a.join(',')]], [['estado', b.join(',')]])
  const RF = rot(v0, vF), RA = rot(v0, vA)
  console.log(`caminhada fechada: R=(${RF.Rend},${RF.RE},${RF.RF1},${RF.RF2},${RF.RD}) · aberta: R=(${RA.Rend},${RA.RE},${RA.RF1},${RA.RF2},${RA.RD})`)
  ok('§H2 a caminhada FECHADA (λ⁺+λ⁻=0) devolve o estado exato: R_total=0 → RETAIN',
    vF[0] === v0[0] && vF[1] === v0[1] && U.retain(RF))
  ok('§H2 a caminhada ABERTA (λ⁺+λ⁻≠0) não devolve: R_total≠0 → REOPEN — o gume',
    !U.retain(RA))
  /* a órbita completa: T ticks = 1 volta no toro — o índice de rotação */
  let v = v0.slice()
  for (let k = 0; k < T1; k++) v = passo(v, 1)
  ok(`§H2 a órbita completa: ${T1} ticks devolvem o estado real exato — uma volta inteira no toro`,
    v[0] === v0[0] && v[1] === v0[1])
}

/* §H3 — a massa fica no centro */
{
  /* (a) nas 52 fusões: M(z) = E(z) − E_∂ invariante sob a monodromia */
  let central = fusoes.length === 52
  for (const lz of fusoes) {
    const [lx, ly] = U.fibra(lz)
    const M = z => {
      const [px, py] = U.fibra(z)
      return U.escada(px).E + U.escada(py).E
    }
    if (M(lz) !== U.escada(lx).E + U.escada(ly).E) central = false
    if (M(U.monodromia(lz)) !== M(lz)) central = false
  }
  ok('§H3 a massa central M(z)=E(x)+E(y) é INVARIANTE sob a monodromia nas 52 — o centro não se move',
    central)
  /* (b) |det| constante ao longo da órbita da batuta (a massa radial) */
  let constante = true
  let Acc = [3, 1, 1, 2]                          /* det = 5: massa 5 */
  const m0 = Math.abs(det(Acc)) % P
  const A = mmod(Am(1))
  for (let k = 0; k < 12; k++) {
    Acc = mulp(A, Acc)
    const d = mod(det(Acc))
    if (d !== m0 && d !== P - m0) constante = false   /* ±5: a folha alterna, a massa não */
  }
  ok('§H3 |det| do estado é constante ao longo da órbita (±massa; o sinal é a folha) — radial parada, angular a andar',
    constante && m0 === 5)
}

/* §H4 — a histerese seleciona a folha admissível (δ⊣ε no suporte real) */
{
  /* o primeiro registo real cujo suporte tem isolados E buracos — o laço
   * só aparece onde a borda é irregular (procurado nos dados, não fixado) */
  let b = null, X = null, N = 0
  for (const l of linhas) {
    const bb = Buffer.from(l, 'utf8')
    const S = new Set()
    for (let i = 0; i < bb.length; i++) if (bb[i] & 1) S.add(i)
    let isolado = false, buraco = false
    for (const i of S) if (!S.has(i - 1) && !S.has(i + 1)) isolado = true
    for (let i = 1; i + 1 < bb.length; i++) {
      if (!S.has(i) && S.has(i - 1) && S.has(i + 1)) buraco = true
    }
    if (isolado && buraco) { b = bb; X = S; N = bb.length; break }
  }
  ok('§H4 o suporte irregular existe nos dados reais (isolados E buracos)', X !== null)
  /* a adjunção vive em ℤ (sem truncar na borda do buffer — truncar quebra
   * φ ≥ id no último ponto, medido) */
  const dilata = S => { const R = new Set(); for (const i of S) { R.add(i); R.add(i + 1) } return R }
  const erode = S => { const R = new Set(); for (const i of S) if (S.has(i + 1)) R.add(i); return R }
  const igualS = (A2, B2) => A2.size === B2.size && [...A2].every(i => B2.has(i))
  const abre = S => dilata(erode(S))
  const fecha = S => erode(dilata(S))
  const aX = abre(X), fX = fecha(X)
  const contido = (A2, B2) => [...A2].every(i => B2.has(i))
  ok('§H4 o laço: α=δε ≤ id ≤ φ=εδ no suporte real — ENTRAR ≠ SAIR (a memória da borda)',
    contido(aX, X) && contido(X, fX) && !igualS(aX, fX) && !igualS(aX, X))
  ok('§H4 a seleção estabiliza: α∘α = α (idempotente) — a folha admissível é ponto fixo',
    igualS(abre(aX), aX))
  ok('§H4 o aberto volta EXATO pela dilatação∘erosão — a histerese seleciona os admissíveis',
    igualS(abre(aX), aX) && contido(erode(aX), erode(X)))
}

/* §H5 — o espectro dos períodos, como DADO (sem nome) */
{
  console.log('')
  console.log('=== O ESPECTRO DA BATUTA (dados; o nome fica de fora) ===')
  let dicotomia = true, fixosOk = true
  for (let m = 1; m <= 6; m++) {
    const A = mmod(Am(m))
    const T = ordem(A)
    /* as folhas mod p: raízes de x² − mx − 1 */
    const raizes = []
    for (let r = 0; r < P; r++) {
      if (mod(r * r - m * r - 1) === 0) raizes.push(r)
    }
    let ordSigma = 0
    if (raizes.length) {
      let x = raizes[0], k = 1
      while (x !== 1 && k <= P) { x = (x * raizes[0]) % P; k++ }
      ordSigma = k
    }
    const separadas = raizes.length === 2
    if (separadas && (P - 1) % T !== 0 && (2 * (P - 1)) % T !== 0) dicotomia = false
    if (!separadas && raizes.length !== 0) dicotomia = false
    /* pontos fixos de A^k não-triviais ⟺ det(A^k − I) ≡ 0 ⟺ k múltiplo do
     * período da folha */
    if (separadas) {
      for (let k = 1; k <= Math.min(T, 64); k++) {
        const Ak = powp(A, k)
        const dk = mod(det([Ak[0] - 1, Ak[1], Ak[2], Ak[3] - 1]))
        const nulo = dk === 0
        const multiplo = k % ordSigma === 0
        if (nulo !== multiplo) fixosOk = false
      }
    }
    console.log(`m=${m}: T=ord(A_m)=${T} · folhas mod p: ${separadas ? 'SEPARADAS (σ=' + raizes[0] + ', ord ' + ordSigma + ')' : 'inertes'} `)
  }
  ok('§H5 a dicotomia mede-se: folhas separadas (T | 2(p−1)) ou inertes — sem exceção em m=1..6',
    dicotomia)
  ok('§H5 os pontos fixos não-triviais de A^k caem EXATAMENTE nos múltiplos do período da folha (det(A^k−I)=0 ⟺ ordSigma | k)',
    fixosOk)
}

console.log('')
if (!falhas) {
  console.log('  O ESPAÇO DE FASE AUDITADO: o toro operacional existe — a batuta')
  console.log('  fecha em círculo mod p e a folha (det=−1 a alternar) é a segunda')
  console.log('  volta; a caminhada fechada devolve o estado exato (RETAIN) e a')
  console.log('  aberta acusa (REOPEN); a massa fica no centro (M invariante sob a')
  console.log('  monodromia; |det| parado na órbita); a histerese é o laço α ≤ id ≤ φ')
  console.log('  que SELECIONA os admissíveis. Os períodos ficam como DADO — o nome')
  console.log('  espectral, se vier, vem da medida seguinte.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
