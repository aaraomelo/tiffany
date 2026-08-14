/* tests/migracao_universal.js — O TESTE DECISIVO da migração (fase 2,
 * eval 14/08: gerente autoriza «em modo cirúrgico», diretor sela a Regra
 * de Ouro da Equivalência).
 *
 * A condição não é «ambos retornam 0»: é R_antigo == R_universal_peano
 * POR CASO — E, Φ, Φ₂, R_endereço, R_D e as decisões RETAIN/REOPEN têm
 * de ser IDÊNTICOS, componente a componente, sobre os objetos reais.
 *
 * Dupla árvore: o lado NOVO é lib/universal.js + lib/peano.js (a única
 * implementação, promovida); o lado ANTIGO são as formas embutidas dos
 * medidores da instância, copiadas byte a byte (não adaptadas). Nada se
 * apagou: os medidores antigos continuam no lugar até a mesa mandar.
 *
 * §M0  o Universal é agnóstico: a fonte de lib/universal.js não contém
 *      a palavra da instância (medido no texto, não prometido)
 * §M1  a escada POR CASO: (E,Φ,Φ₂) idênticos nas 4234 linhas do cristal
 *      e nos 40 textos do banal (falas e respostas) — 4274 casos × 3
 * §M2  a energia total do cristal: U == P inteiro exato
 * §M3  R_endereço sob a bateria de induções de cristal_volta (permutar,
 *      byte, categoria, remover, duplicar, corromper): U == P caso a caso
 * §M4  o vetor total e a DECISÃO nas cinco classes de residuos_totais
 *      (mesmo corpo, permutação, conteúdo, endereço, espelhado):
 *      (R_end,R_E,R_Φ,R_Φ₂,R_D) e RETAIN/REOPEN idênticos
 * §M5  a fusão: fibra_U == fibra_P byte a byte nas 52 reais; funde∘fibra
 *      = id nos dois lados; a monodromia coincide
 * §M6  a geometria: mat2 (J, espelho, A_m, W, estaca, cartas) reproduz a
 *      forma antiga em grelha −3..3 e m=0..8, entrada a entrada
 *
 *   node tests/migracao_universal.js
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

const RAIZ = path.join(__dirname, '..')
const { Universal, mat2 } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')
const U = Universal(sigmaPeano)
const P = 65537

/* ── o lado ANTIGO: formas embutidas, copiadas byte a byte ───────────────── */
/* de assinatura_banal.js / residuos_totais.js */
function IP (texto) {
  const b = Buffer.from(String(texto), 'utf8')
  let E = 0, fase = 0, f2 = 0
  for (let i = 0; i < b.length; i++) {
    E += b[i] * b[i]
    fase = (fase + (i + 1) * b[i]) % P
    f2 = (f2 + ((i + 1) * (i + 1) % P) * b[i]) % P
  }
  return { E, fase, f2 }
}
/* de cristal_energia.js */
function energiaP (linhas) {
  let E = 0
  for (const l of linhas) {
    const b = Buffer.from(l, 'utf8')
    for (let i = 0; i < b.length; i++) E += b[i] * b[i]
  }
  return E
}
/* de cristal_volta.js (residuoV2) */
function idDe (linha, i) {
  try { return JSON.parse(linha).id } catch { return '￿ corrompido ' + i }
}
function residuoV2P (fonteLinhas, regs) {
  const fontePorId = new Map()
  for (const l of fonteLinhas) fontePorId.set(idDe(l, 0), l)
  const conteudo = new Map(), vezes = new Map()
  for (let i = 0; i < regs.length; i++) {
    const a = idDe(regs[i], i)
    if (!conteudo.has(a)) conteudo.set(a, regs[i])
    vezes.set(a, (vezes.get(a) || 0) + 1)
  }
  let r = 0
  for (const [a, l] of fontePorId) {
    const v = conteudo.get(a)
    if (v === undefined) r++
    else if (v !== l) r++
  }
  for (const [a, n] of vezes) {
    if (!fontePorId.has(a)) r++
    if (n > 1) r += n - 1
  }
  return r
}
/* de residuos_totais.js */
function transicoesP (s) {
  const b = Buffer.from(String(s), 'utf8')
  const t = []
  for (let i = 0; i + 1 < b.length; i++) t.push(b[i] * 256 + b[i + 1])
  return t
}
function residuoTotalP (fonte, cand) {
  const F = new Map(fonte)
  const C = new Map(), vezes = new Map()
  for (const [a, s] of cand) {
    if (!C.has(a)) C.set(a, s)
    vezes.set(a, (vezes.get(a) || 0) + 1)
  }
  let Rend = 0, RE = 0, RF1 = 0, RF2 = 0, RD = 0
  for (const [a, s0] of F) {
    const s1 = C.get(a)
    if (s1 === undefined) { Rend++; continue }
    if (s1 !== s0) Rend++
    const i0 = IP(s0), i1 = IP(s1)
    RE += Math.abs(i1.E - i0.E)
    RF1 += (i1.fase - i0.fase + P) % P
    RF2 += (i1.f2 - i0.f2 + P) % P
    const t0 = transicoesP(s0), t1 = transicoesP(s1)
    if (t0.length !== t1.length) RD++
    else if (t0.some((v, i) => v !== t1[i])) RD++
  }
  for (const [a, n] of vezes) {
    if (!F.has(a)) Rend++
    if (n > 1) Rend += n - 1
  }
  return { Rend, RE, RF1, RF2, RD }
}
const retainP = R =>
  R.Rend === 0 && R.RE === 0 && R.RF1 === 0 && R.RF2 === 0 && R.RD === 0
/* de fusao_conceitos.js / cristal_curadoria.js (a fibra cega) */
function fibraP (lz) {
  const o = JSON.parse(lz)
  if (!Array.isArray(o.fusao) || o.fusao.length !== 2) return null
  const ini = lz.indexOf('[') + 1
  const fim = lz.lastIndexOf(']')
  const miolo = lz.slice(ini, fim)
  let prof = 0, corte = -1
  for (let i = 0; i < miolo.length; i++) {
    if (miolo[i] === '{') prof++
    else if (miolo[i] === '}') prof--
    else if (miolo[i] === ',' && prof === 0) { corte = i; break }
  }
  if (corte < 0) return null
  return [miolo.slice(0, corte), miolo.slice(corte + 1)]
}
function fundeP (idZ, lx, ly) {
  return '{"fusao":[' + lx + ',' + ly + '],"id":"' + idZ + '","tipo":"conceito"}'
}
/* de contorno_riemann.js (a geometria inline) */
function mulP (X, Y) {
  return [X[0] * Y[0] + X[1] * Y[2], X[0] * Y[1] + X[1] * Y[3],
    X[2] * Y[0] + X[3] * Y[2], X[2] * Y[1] + X[3] * Y[3]]
}
const somaP = (X, Y) => X.map((v, i) => v + Y[i])
const escalaP = (k, X) => X.map(v => k * v)
const IP2 = [1, 0, 0, 1]
const JP = [0, 1, -1, 0]
const RP = [1, 0, 0, -1]
const AmP = m => [m, 1, 1, 0]
const WP = m => somaP(escalaP(2, AmP(m)), escalaP(-m, IP2))
const estacaP = m => somaP(escalaP(m, IP2), escalaP(-1, AmP(m)))
const cartaP = (a, b) => somaP(escalaP(a, IP2), escalaP(b, JP))

/* ── as fixtures reais ────────────────────────────────────────────────────── */
const cristal = fs.readFileSync(path.join(RAIZ, 'cristal', 'cristal.jsonl'), 'utf8')
  .split('\n').filter(l => l.length)
const fusoes = cristal.filter(l => JSON.parse(l).fusao)
const tex = fs.readFileSync(path.join(RAIZ, 'papers', 'conversa.tex'), 'utf8')
const pares = []
{
  const rx = /\\section\{([^}]+)\}\s*\n([^\n\\]+)/g
  let m
  while ((m = rx.exec(tex)) && pares.length < 20) {
    const fala = m[1].trim(), resp = m[2].trim()
    if (fala.length >= 2 && resp.length >= 8) pares.push([fala, resp])
  }
}

/* §M0 — o Universal é agnóstico (medido no texto da fonte) */
{
  const fonte = fs.readFileSync(path.join(RAIZ, 'lib', 'universal.js'), 'utf8')
  ok('§M0 lib/universal.js não contém a palavra da instância — o 𝒰 não sabe que o Peano existe',
    !/peano/i.test(fonte))
}

/* §M1 — a escada POR CASO */
{
  let casos = 0, divergencias = 0
  const textos = [...cristal]
  for (const [fala, resp] of pares) textos.push(fala, resp)
  for (const t of textos) {
    const u = U.escada(t), p = IP(t)
    casos++
    if (u.E !== p.E || u.f1 !== p.fase || u.f2 !== p.f2) divergencias++
  }
  console.log(`escada por caso: ${casos} textos × 3 componentes · divergências: ${divergencias}`)
  ok(`§M1 (E,Φ,Φ₂) idênticos POR CASO nos ${casos} textos reais (cristal + banal)`,
    casos === cristal.length + 2 * pares.length && divergencias === 0)
}

/* §M2 — a energia total */
{
  const eU = U.energia(cristal), eP = energiaP(cristal)
  console.log(`E_U = ${eU} · E_P = ${eP}`)
  ok('§M2 a energia total do cristal: U == P inteiro exato', eU === eP && eU > 0)
}

/* §M3 — R_endereço sob a bateria de induções de cristal_volta */
{
  const meio = Math.floor(cristal.length / 2)
  const inducoes = [
    ['íntegro', m => m],
    ['permutação física', m => m.reverse()],
    ['1 byte na descricao', m => {
      const alvo = m[meio]
      const i = alvo.indexOf('"descricao":"') + 13
      m[meio] = alvo.slice(0, i) +
        String.fromCharCode(alvo.charCodeAt(i) ^ 1) + alvo.slice(i + 1)
      return m
    }],
    ['categoria (meta.dominio)', m => {
      for (let i = 0; i < m.length; i++) {
        const t = m[i].replace(/"dominio":"[^"]*"/, '"dominio":"induzido"')
        if (t !== m[i]) { m[i] = t; break }
      }
      return m
    }],
    ['remoção de endereço', m => { m.splice(meio, 1); return m }],
    ['duplicação de endereço', m => { m.push(m[meio]); return m }],
    ['endereço destruído', m => {
      m[meio] = m[meio].slice(0, 20) + '<<<corrompido>>>'
      return m
    }],
  ]
  let iguais = true, acusam = 0
  for (const [nome, f] of inducoes) {
    const copia = f([...cristal])
    const rU = U.rEndereco(cristal, copia)
    const rP = residuoV2P(cristal, copia)
    console.log(`#CASO R_end ${nome.padEnd(24)} U=${rU} P=${rP} ${rU === rP ? '==' : 'DIVERGE'}`)
    if (rU !== rP) iguais = false
    if (rU > 0) acusam++
  }
  ok('§M3 R_endereço: U == P em TODAS as induções, caso a caso (e as não-admissíveis acusam)',
    iguais && acusam === 5)
}

/* §M4 — o vetor total e a DECISÃO nas cinco classes */
{
  const fonte = pares.map(([a, s]) => [a, s])
  const espelha = s => {
    /* a dupla transposição espelhada (assinatura_banal §B5): achar
     * «ab…ba» e trocar os dois sítios — conserva E (multiconjunto) e Φ
     * (as fases cancelam), só a curvatura Φ₂ e a membrana R_D acusam */
    const b = [...s]
    for (let i = 0; i + 1 < b.length; i++) {
      if (b[i] === b[i + 1]) continue
      for (let j = i + 2; j + 1 < b.length; j++) {
        if (b[j] === b[i + 1] && b[j + 1] === b[i]) {
          ;[b[i], b[i + 1]] = [b[i + 1], b[i]]
          ;[b[j], b[j + 1]] = [b[j + 1], b[j]]
          return b.join('')
        }
      }
    }
    return null
  }
  /* o primeiro par do banal que admite o espelho orgânico */
  let iEsp = -1, sEsp = null
  for (let i = 0; i < pares.length; i++) {
    const e = espelha(pares[i][1])
    if (e && e !== pares[i][1]) { iEsp = i; sEsp = e; break }
  }
  const classes = [
    ['mesmo corpo', pares.map(([a, s]) => [a, s])],
    ['permutação admissível', [...pares].reverse().map(([a, s]) => [a, s])],
    ['conteúdo alterado', pares.map(([a, s], i) => [a, i === 3 ? s + '!' : s])],
    ['endereço alterado', pares.map(([a, s], i) => [i === 5 ? a + '~' : a, s])],
    ['espelhado (E e Φ cegos)', pares.map(([a, s], i) => [a, i === iEsp ? sEsp : s])],
  ]
  ok('§M4 o espelho orgânico existe no banal (pré-condição da classe crítica)', iEsp >= 0)
  let vetoresIguais = true, decisoesIguais = true
  for (const [nome, cand] of classes) {
    const rU = U.residuoTotal(fonte, cand)
    const rP = residuoTotalP(fonte, cand)
    const dU = U.retain(rU) ? 'RETAIN' : 'REOPEN'
    const dP = retainP(rP) ? 'RETAIN' : 'REOPEN'
    const mesmo = rU.Rend === rP.Rend && rU.RE === rP.RE &&
      rU.RF1 === rP.RF1 && rU.RF2 === rP.RF2 && rU.RD === rP.RD
    console.log(`#CASO R_total ${nome.padEnd(22)} U=(${rU.Rend},${rU.RE},${rU.RF1},${rU.RF2},${rU.RD})→${dU} · P=(${rP.Rend},${rP.RE},${rP.RF1},${rP.RF2},${rP.RD})→${dP}`)
    if (!mesmo) vetoresIguais = false
    if (dU !== dP) decisoesIguais = false
  }
  ok('§M4 o vetor total (R_end,R_E,R_Φ,R_Φ₂,R_D) é idêntico nas cinco classes, componente a componente',
    vetoresIguais)
  ok('§M4 as DECISÕES coincidem: RETAIN/REOPEN iguais dos dois lados em todas as classes',
    decisoesIguais)
}

/* §M5 — a fusão: fibra, volta e monodromia coincidem */
{
  let fibras = true, voltas = true, monos = true
  for (const lz of fusoes) {
    const pU = U.fibra(lz), pP = fibraP(lz)
    if (!pU || !pP || pU[0] !== pP[0] || pU[1] !== pP[1]) fibras = false
    else {
      if (U.funde(JSON.parse(lz).id, pU[0], pU[1]) !== lz) voltas = false
      if (fundeP(JSON.parse(lz).id, pP[0], pP[1]) !== lz) voltas = false
      const nU = U.monodromia(lz)
      const nP = fundeP(JSON.parse(pP[1]).id, pP[1], pP[0])
      if (nU !== nP) monos = false
    }
  }
  ok(`§M5 a fibra U == fibra P byte a byte nas ${fusoes.length} fusões, e funde∘fibra = id nos DOIS lados`,
    fusoes.length === 52 && fibras && voltas)
  ok('§M5 a monodromia coincide: ν_U == ν_P byte a byte nas 52', monos)
}

/* §M6 — a geometria: mat2 reproduz a forma antiga, entrada a entrada */
{
  let iguais = true
  for (let m = 0; m <= 8; m++) {
    if (!mat2.igual(mat2.Am(m), AmP(m))) iguais = false
    if (!mat2.igual(mat2.W(m), WP(m))) iguais = false
    if (!mat2.igual(mat2.estaca(m), estacaP(m))) iguais = false
    if (!mat2.igual(mat2.mul(mat2.Am(m), mat2.estaca(m)),
      mulP(AmP(m), estacaP(m)))) iguais = false
  }
  for (let a = -3; a <= 3; a++) {
    for (let b = -3; b <= 3; b++) {
      if (!mat2.igual(mat2.carta(a, b), cartaP(a, b))) iguais = false
      if (mat2.det(mat2.carta(a, b)) !==
        cartaP(a, b)[0] * cartaP(a, b)[3] - cartaP(a, b)[1] * cartaP(a, b)[2]) iguais = false
    }
  }
  ok('§M6 a geometria: J, espelho, A_m, W, estaca e cartas idênticos entrada a entrada (m=0..8, grelha −3..3)',
    iguais && mat2.igual(mat2.J, JP) && mat2.igual(mat2.espelho, RP))
}

/* §M7 — FASE 3: as oito leis como interface normativa (a promoção) */
{
  const { verificaLeis } = require('../lib/universal.js')
  const v = verificaLeis()
  for (const l of v) console.log(`#LEI ${l.n} ${l.nome.padEnd(52)} ${l.ok ? 'ok' : 'FALHA'}`)
  ok('§M7 as OITO leis verificam na infraestrutura: catálogo completo, todas operacionais',
    v.length === 8 && v.every(l => l.ok) &&
    v.map(l => l.n).join() === '0,1,2,3,4,5,6,7')
}

/* §M8 — FASE 3: as primitivas dinâmicas contra as formas embutidas
 * (anel/DFT de metronomo_fourier; a lei da cascata de
 * metronomo_autossimilar; a morfologia de toro_histerese) */
{
  const { anel, dft, idft, renormaliza, morfo } = require('../lib/universal.js')
  /* forma antiga do anel/DFT (metronomo_fourier, byte a byte) */
  function anelP (q) {
    const mod = x => ((x % q) + q) % q
    const powm = (b, e) => {
      let r = 1; b = mod(b)
      while (e > 0) { if (e & 1) r = r * b % q; b = b * b % q; e >>= 1 }
      return r
    }
    return { mod, powm }
  }
  const q = 257
  const Aq = anel(q), Ap = anelP(q)
  const w = Aq.powm(3, 2)                               /* ord 128 */
  /* a órbita real m=2 */
  let v2 = [1, 0]
  const xs = []
  do { xs.push(v2[0]); v2 = [Aq.mod(2 * v2[0] + v2[1]), v2[0]] } while (v2[0] !== 1 || v2[1] !== 0)
  function dftP (zs) {
    const M = zs.length
    const c = []
    for (let k = 0; k < M; k++) {
      let s = 0
      const wk = Ap.powm(w, M - (k % M))
      let f = 1
      for (let n = 0; n < M; n++) { s = (s + zs[n] * f) % q; f = f * wk % q }
      c.push(s)
    }
    return c
  }
  const cU = dft(xs, Aq, w), cP = dftP(xs)
  const voltaU = idft(cU, Aq, w)
  ok('§M8 a DFT da lib == a forma embutida, coeficiente a coeficiente, na órbita real (128 casos)',
    cU.length === cP.length && cU.every((s, k) => s === cP[k]))
  ok('§M8 a inversa da lib devolve a órbita byte a byte (idft∘dft = id)',
    voltaU.every((s, n) => s === xs[n]))
  /* a lei da cascata: renormaliza == quadraturas de matriz (BigInt) */
  const mulB = (X, Y) => [X[0] * Y[0] + X[1] * Y[2], X[0] * Y[1] + X[1] * Y[3],
    X[2] * Y[0] + X[3] * Y[2], X[2] * Y[1] + X[3] * Y[3]]
  let M2 = [2n, 1n, 1n, 0n]
  let est = { t: 2n, d: -1n }
  let cascataOk = true
  for (let j = 1; j <= 8; j++) {
    M2 = mulB(M2, M2)
    est = renormaliza(est)
    if (M2[0] + M2[3] !== est.t) cascataOk = false
    if (M2[0] * M2[3] - M2[1] * M2[2] !== est.d) cascataOk = false
  }
  ok('§M8 a renormalização da lib == traço/det de A^{2^j} por quadraturas (BigInt, j=1..8)',
    cascataOk)
  /* a morfologia: lib == forma embutida de toro_histerese, no suporte real */
  const b = Buffer.from(cristal[1], 'utf8')
  const S = new Set()
  for (let i = 0; i < b.length; i++) if (b[i] & 1) S.add(i)
  const dilataP = X => { const R2 = new Set(); for (const i of X) { R2.add(i); R2.add(i + 1) } return R2 }
  const erodeP = X => { const R2 = new Set(); for (const i of X) if (X.has(i + 1)) R2.add(i); return R2 }
  const igualS = (A2, B2) => A2.size === B2.size && [...A2].every(i => B2.has(i))
  ok('§M8 a morfologia da lib == a forma embutida (δ, ε, α, φ) no suporte real, elemento a elemento',
    igualS(morfo.dilata(S), dilataP(S)) && igualS(morfo.erode(S), erodeP(S)) &&
    igualS(morfo.abre(S), dilataP(erodeP(S))) && igualS(morfo.fecha(S), erodeP(dilataP(S))))
}

console.log('')
if (!falhas) {
  console.log('  A EQUIVALÊNCIA FECHOU POR CASO: a única implementação universal')
  console.log('  (lib/universal.js, agnóstica — medido) reproduz as formas antigas')
  console.log('  embutidas nos medidores, componente a componente: escada, energia,')
  console.log('  R_endereço sob indução, o vetor total COM as decisões RETAIN/REOPEN,')
  console.log('  a fusão com a sua volta, e a geometria. Peano antigo ≡ 𝒰[σ_Peano].')
  console.log('  A árvore antiga fica no lugar até a mesa mandar a limpeza.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
