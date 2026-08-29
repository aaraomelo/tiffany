/* tests/orbitas_hopfield.js — órbitas sql/latex/node na matriz Hopfield (Hebb dual).
 *
 * papers/redes.tex: Hebb é fis:cor:hebb; dual é W=W_s+W_a (fis:thm:duastorres);
 * λ⁺+λ⁻=0 (thm:rede-dual). A fita continua G=1; isto mede a declaração das órbitas.
 * W_s/W_a ≠ duomorfismo (a,b) — eixos distintos (redes:eixos).
 *
 *   node tests/orbitas_hopfield.js
 */
import { readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const man = JSON.parse(readFileSync(
  join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const N = 32
const H = N >> 1
const ORBITAS = man.orbitas || []
const hp = man.hopfield || {}

function lang (nome) {
  return man.linguagens.find((l) => l.nome === nome)
}

function xiLingua (nome, suporte) {
  const L = lang(nome)
  const s = new Int8Array(N)
  let seed = 5381
  for (const c of nome) seed = ((seed << 5) + seed + c.charCodeAt(0)) | 0
  for (let i = 0; i < N; i++) {
    const b = (seed >>> (i % 24)) ^ ((L.p | 0) << 2) ^ ((L.q | 0) << 1) ^ (L.r | 0)
    s[i] = ((b ^ (i * 7) ^ (suporte === 'canal' ? 1 : 0)) & 1) ? 1 : -1
  }
  s[0] = L.p ? 1 : -1
  s[1] = L.q ? 1 : -1
  s[2] = L.r ? 1 : -1
  s[3] = suporte === 'canal' ? 1 : -1
  s[4] = nome === 'sql' ? 1 : -1
  return s
}

function hexBits (hex) {
  const s = new Int8Array(N)
  const h = hex.replace(/^0x/i, '').padEnd(N / 4, '0').slice(0, N / 4)
  for (let i = 0; i < N; i++) {
    const nib = parseInt(h[i >> 2], 16)
    s[i] = (nib >> (3 - (i & 3))) & 1 ? 1 : -1
  }
  return s
}

function hebb (padroes) {
  const W = Array.from({ length: N }, () => Array(N).fill(0))
  for (const xi of padroes) {
    for (let i = 0; i < N; i++) {
      for (let j = 0; j < N; j++) {
        if (i !== j) W[i][j] += xi[i] * xi[j]
      }
    }
  }
  return W
}

function waCanal () {
  const A = Array.from({ length: N }, () => Array(N).fill(0))
  for (let i = 0; i < H; i++) {
    A[i][i + H] = 1
    A[i + H][i] = -1
  }
  return A
}

function simetrica (W) {
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < N; j++) if (W[i][j] !== W[j][i]) return false
  }
  return true
}

function antissimetrica (W) {
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < N; j++) if (W[i][j] + W[j][i] !== 0) return false
  }
  return true
}

function formaQuad (W, s) {
  let e = 0
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < N; j++) e += W[i][j] * s[i] * s[j]
  }
  return e
}

function campo (W, s, i) {
  let h = 0
  for (let j = 0; j < N; j++) h += W[i][j] * s[j]
  return h
}

function recupera (W, s0, limite = 24) {
  const s = Int8Array.from(s0)
  for (let v = 0; v < limite; v++) {
    let mudou = 0
    for (let i = 0; i < N; i++) {
      const novo = campo(W, s, i) >= 0 ? 1 : -1
      if (novo !== s[i]) { s[i] = novo; mudou++ }
    }
    if (!mudou) break
  }
  return s
}

function passoSync (W, s) {
  const n = new Int8Array(N)
  for (let i = 0; i < N; i++) n[i] = campo(W, s, i) >= 0 ? 1 : -1
  return n
}

function periodo (W, s0, max = 16) {
  let s = Int8Array.from(s0)
  const visto = new Map()
  for (let t = 0; t < max; t++) {
    const k = Buffer.from(s).toString('hex')
    if (visto.has(k)) return t - visto.get(k)
    visto.set(k, t)
    s = passoSync(W, s)
  }
  return 0
}

function igual (a, b) {
  return a.length === b.length && a.every((x, i) => x === b[i])
}

function sobrepoe (a, b) {
  let s = 0
  for (let i = 0; i < N; i++) s += a[i] * b[i]
  return s
}

/* §H0 — manifesto: órbitas ≠ fios HTTP */
{
  ok('§H0 sql/latex/node em orbitas', ['sql', 'latex', 'node'].every(
    (n) => ORBITAS.find((o) => o.nome === n)))
  ok('§H0 node ingerido (linguagem+orbita, nao fio)',
    !!lang('node')?.absorcao &&
    !!ORBITAS.find((o) => o.nome === 'node') &&
    !(man.fios || []).find((f) => f.nome === 'node'))
  ok('§H0 fios HTTP nao absorvidos',
    (man.fios || []).length >= 3 &&
    (man.fios || []).every((f) => f.absorvido === false) &&
    ['post', 'http', 'fetch'].every((n) => (man.fios || []).find((f) => f.nome === n)))
  ok('§H0 absorcao.orbita (chave fio apagada)', ['sql', 'latex', 'node'].every((n) => {
    const A = lang(n)?.absorcao
    return A && A.orbita && A.fio === undefined
  }))
  ok('§H0 hopfield dual declarado',
    hp.dual === 'W = W_s + W_a' &&
    hp.fecho.includes('lambda') &&
    hp.eixos.includes('duomorfismo') &&
    hp.fita.includes('G=1'))
  ok('§H0 hopfield e o candidato Neural (uma matriz, as tres orbitas)',
    hp.canonico === 'Corpo Neural' &&
    hp.estatuto === 'candidato' &&
    Array.isArray(hp.orbitas) &&
    hp.orbitas.join(',') === 'sql,latex,node')
}

/* §H1 — eixos distintos: aresta a/b não é W_s/W_a */
{
  const e = (man.arestas || []).find((x) => x.de === 'sql' && x.para === 'node')
  ok('§H1 sql→node e duo (a=1) — eixo do duomorfismo', e && e.a === 1 && e.b === 1)
  ok('§H1 hopfield nao identifica a com W_a',
    hp.eixos.includes('W_s/W_a') && !hp.eixos.includes('a=W_a'))
}

const sqlO = ORBITAS.find((o) => o.nome === 'sql')
const corpusPath = join(RAIZ, ...(sqlO.corpus.split('/')))
const corpusTxt = readFileSync(corpusPath, 'utf8')
const corpusXi = []
for (const line of corpusTxt.split(/\r?\n/)) {
  const m = line.match(/^orbita\s+\S+\|.*c([0-9a-f]+)\s*$/i)
  if (!m) continue
  const xiSql = hexBits(m[1])
  const xiTex = xiSql.map((x) => -x)
  corpusXi.push({ sql: xiSql, latex: Int8Array.from(xiTex) })
}

/* §H2 — órbitas SQL (corpus) e LaTeX (face dual = bits invertidos) */
{
  ok('§H2 corpus idioma/pt/orbita ingerido', corpusXi.length >= 8)
  ok('§H2 face latex = −face sql (Hebb dual da escrita)',
    corpusXi.length > 0 &&
    corpusXi.every((p) => sobrepoe(p.sql, p.latex) === -N))
  const chave = sqlO.chave
  ok('§H2 chave SQL mapeada', chave.startsWith('idioma/pt/orbita/'))
}

const xiSql = xiLingua('sql', 'arena')
const xiTex = xiLingua('latex', 'arena')
const xiNode = xiLingua('node', 'canal')

/* §H3 — três línguas como padrões ξ na mesma matriz */
{
  ok('§H3 sql/latex/node padroes distintos',
    !igual(xiSql, xiTex) && !igual(xiSql, xiNode) && !igual(xiTex, xiNode))
  ok('§H3 node marca canal (s[3]=+1)', xiNode[3] === 1 && xiSql[3] === -1)
  ok('§H3 sql marca hub (s[4]=+1)', xiSql[4] === 1 && xiNode[4] === -1)
}

const padroesLang = [xiSql, xiTex, xiNode]
const Ws = hebb(padroesLang)
const Wa = waCanal()
const uns = new Int8Array(N).fill(1)

/* §H4 — Hebb W_s: simétrica, diagonal nula, viva */
{
  let diag = 0
  let vivas = 0
  for (let i = 0; i < N; i++) {
    diag += Math.abs(Ws[i][i])
    for (let j = 0; j < N; j++) if (Ws[i][j] !== 0) vivas++
  }
  ok('§H4 Hebb simetrica', simetrica(Ws))
  ok('§H4 diagonal nula', diag === 0)
  ok('§H4 matriz viva', vivas > N)
}

/* §H5 — dual Hebb: W_a antissimétrica, sᵀ W_a s = 0, ambas metades vivas */
{
  ok('§H5 W_a antissimetrica', antissimetrica(Wa))
  const probes = [xiSql, xiTex, xiNode, uns]
  let quad0 = true
  for (const s of probes) {
    if (formaQuad(Wa, s) !== 0) quad0 = false
    let lp = 0
    let lm = 0
    for (let i = 0; i < H; i++) lp += s[i] * campo(Wa, s, i)
    for (let i = H; i < N; i++) lm += s[i] * campo(Wa, s, i)
    if (lp + lm !== 0) quad0 = false
  }
  let lpU = 0
  let lmU = 0
  for (let i = 0; i < H; i++) lpU += uns[i] * campo(Wa, uns, i)
  for (let i = H; i < N; i++) lmU += uns[i] * campo(Wa, uns, i)
  ok('§H5 s^T W_a s = 0 e lambda+ + lambda- = 0', quad0)
  ok('§H5 dual vivo (cada face nao nula)', lpU !== 0 && lmU !== 0 && lpU + lmU === 0)
}

/* §H6 — partição única W = W_s + W_a (em dobro: 2W = 2W_s + 2W_a) */
{
  let soma = 0
  let sim = 0
  let anti = 0
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < N; j++) {
      const Q = Ws[i][j] + Wa[i][j]
      const Sn = Ws[i][j] + Ws[j][i]
      const An = Wa[i][j] - Wa[j][i]
      if (Sn + An !== 2 * Q && Wa[i][j] !== -Wa[j][i]) soma++
      if (Ws[i][j] !== Ws[j][i]) sim++
      if (Wa[i][j] + Wa[j][i] !== 0) anti++
    }
  }
  ok('§H6 particao W_s + W_a unica (simetrica + antissimetrica)',
    soma === 0 && sim === 0 && anti === 0)
}

/* §H7 — torre branca: recuperar ξ a partir de ruído (descida Hebb) */
{
  const ruido = Int8Array.from(xiNode)
  for (let i = 0; i < 3; i++) ruido[i] = -ruido[i]
  const rec = recupera(Ws, ruido)
  ok('§H7 recupera padrao node (torre branca / W_s)', igual(rec, xiNode))
  const ruidoSql = Int8Array.from(xiSql)
  for (let i = 0; i < 2; i++) ruidoSql[i] = -ruidoSql[i]
  ok('§H7 recupera padrao sql', igual(recupera(Ws, ruidoSql), xiSql))
  const Wc = hebb([corpusXi[0].sql, corpusXi[0].latex])
  const ruidoC = Int8Array.from(corpusXi[0].sql)
  ruidoC[0] = -ruidoC[0]
  ok('§H7 recupera orbita corpus (sql/latex dual)',
    igual(recupera(Wc, ruidoC), corpusXi[0].sql))
}

/* §H8 — torre negra: W_a sincrono tem periodo 4 (roda, nao desce) */
{
  const pA = periodo(Wa, xiNode, 16)
  const pS = periodo(Ws, xiNode, 16)
  ok('§H8 W_a periodo 4 (torre negra)', pA === 4)
  ok('§H8 W_s periodo 1 ou 2 (torre branca espelha)', pS === 1 || pS === 2)
}

/* §H9 — fecho dual: magnetização ξ e −ξ somam 0 depois da recuperação */
{
  const rec = recupera(Ws, xiSql)
  const mMais = sobrepoe(rec, xiSql)
  const mMenos = sobrepoe(rec, xiSql.map((x) => -x))
  ok('§H9 lambda+ + lambda- = 0 na recuperacao', mMais + mMenos === 0)
  ok('§H9 recuperacao alinha com xi (nao com -xi)', mMais === N && mMenos === -N)
}

console.log(`\n=== orbitas_hopfield: ${feitas - falhas}/${feitas} OK ===`)
process.exit(falhas ? 1 : 0)
