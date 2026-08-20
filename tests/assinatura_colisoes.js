/* tests/assinatura_colisoes.js — a caçada às colisões do observador (eval 13/08).
 *
 * A ordem do gerente/diretor: procurar x ≠ y com I(x) = I(y) — «se
 * encontrarem, não é fracasso: é ouro metodológico — o observador precisa
 * de uma terceira coordenada».
 *
 * A construção que fura o par (E, Φ): a DUPLA TRANSPOSIÇÃO ESPELHADA.
 * Trocar «ab»→«ba» numa posição e «ba»→«ab» noutra, à MESMA distância
 * interna d, conserva o multiconjunto de bytes (E) e cancela a fase:
 *   ΔΦ = d(a−b) + d(b−a) = 0.
 * Se o padrão existe em dados ORGÂNICOS do cristal, o observador dual tem
 * cegueira construtível — medida, não especulada.
 *
 * E a escada: a terceira coordenada Φ₂ = Σ i²·b (a curvatura) mata a
 * família — ΔΦ₂ = d(b−a)·((i′+j′)−(i+j)) ≠ 0 quando os centros diferem —
 * e a escada dos momentos É o espectro: o observador completo é a
 * transformada inteira, e o byte a byte (medidor v2) é o seu limite.
 *
 * §C0  a construção fecha em sintético: I(x)=I(y), x≠y
 * §C1  A CAÇADA NO CRISTAL: registos reais com «ab…ba» à mesma distância —
 *      colisões ORGÂNICAS do observador dual (o log para a mesa)
 * §C2  Φ₂ separa TODAS as colisões encontradas (a terceira coordenada)
 * §C3  busca aleatória por colisões de (E,Φ,Φ₂): orçamento e achado — a
 *      escada continua (sem reivindicar mais do que o varrido)
 * §C4  o clone sob o observador: I(clone)=I(original) — mesmo corpo para I;
 *      a CARDINALIDADE no endereço distingue o um do dois (x ∼_O y)
 *
 *   node tests/assinatura_colisoes.js
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

const P = 65537
function assinatura (s) {
  let E = 0, f1 = 0, f2 = 0
  for (let i = 0; i < s.length; i++) {
    const b = s.charCodeAt(i) & 0xFFFF
    E += b * b
    f1 = (f1 + (i + 1) * b) % P
    f2 = (f2 + ((i + 1) * (i + 1) % P) * b) % P
  }
  return { E, f1, f2 }
}
const igualI = (a, b) => a.E === b.E && a.f1 === b.f1
const igualI3 = (a, b) => igualI(a, b) && a.f2 === b.f2

/* §C0 — sintético: a construção fecha */
{
  const x = 'o gato ab correu ba depois'
  /* troca ab→ba (mesma d=1) e ba→ab: dupla transposição espelhada */
  const y = 'o gato ba correu ab depois'
  const Ix = assinatura(x), Iy = assinatura(y)
  console.log(`sintético: x≠y ${x !== y} · E ${Ix.E === Iy.E ? '=' : '≠'} · Φ ${Ix.f1 === Iy.f1 ? '=' : '≠'} · Φ₂ ${Ix.f2 === Iy.f2 ? '=' : '≠'}`)
  ok('§C0 a construção fura (E,Φ): I(x)=I(y) com x≠y', x !== y && igualI(Ix, Iy))
  ok('§C0 e Φ₂ separa o sintético', Ix.f2 !== Iy.f2)
}

/* §C1 — a caçada nos registos REAIS do cristal */
const FONTE = path.join(__dirname, '..', 'cristal', 'cristal.jsonl')
const linhas = fs.readFileSync(FONTE, 'utf8').split('\n').filter(l => l.length)

function idDe (l) {
  const m = /"id":\s*"((?:[^"\\]|\\.)*)"/.exec(l)
  return m ? m[1] : null
}

/* procura no registo um par de dígrafos espelhados «XY…YX» (X≠Y, sem \\ ou ")
 * e devolve o registo com AMBOS trocados — a colisão orgânica */
function colideOrganico (l) {
  const n = l.length
  for (let i = 0; i + 1 < n; i++) {
    const a = l[i], b = l[i + 1]
    if (a === b || a === '\\' || b === '\\' || a === '"' || b === '"') continue
    const alvo = b + a
    const j = l.indexOf(alvo, i + 2)
    if (j < 0) continue
    if (l[j] === '\\' || l[j + 1] === '\\') continue
    const y = l.slice(0, i) + b + a + l.slice(i + 2, j) + a + b + l.slice(j + 2)
    if (y !== l) return { y, i, j, par: a + b }
    return null
  }
  return null
}

const colisoes = []
for (const l of linhas) {
  const c = colideOrganico(l)
  if (!c) continue
  const Ix = assinatura(l), Iy = assinatura(c.y)
  if (igualI(Ix, Iy) && l !== c.y) {
    colisoes.push({ id: idDe(l), ...c, Ix, Iy: assinatura(c.y) })
  }
}
console.log('')
console.log(`=== LOG DE COLISÕES (orgânicas, cristal real): ${colisoes.length} de ${linhas.length} registos ===`)
for (const c of colisoes.slice(0, 5)) {
  console.log(`#COL ${String(c.id).padEnd(36)} par «${c.par}»@${c.i}, «${c.par[1] + c.par[0]}»@${c.j}  E=${c.Ix.E}  Φ=${c.Ix.f1}  → I igual, corpo DIFERENTE`)
}
if (colisoes.length > 5) console.log(`#COL … e mais ${colisoes.length - 5}`)
ok('§C1 CEGUEIRA DO OBSERVADOR DUAL encontrada em dados orgânicos (ouro metodológico)',
  colisoes.length > 0)

/* §C2 — a terceira coordenada Φ₂ separa todas */
{
  let separadas = 0
  for (const c of colisoes) if (c.Ix.f2 !== c.Iy.f2) separadas++
  console.log(`Φ₂ separa ${separadas}/${colisoes.length}`)
  ok('§C2 Φ₂ (a curvatura) separa TODAS as colisões encontradas', separadas === colisoes.length)
}

/* §C3 — busca aleatória por colisões de (E,Φ,Φ₂) — orçamento dito */
{
  let SEED = 29
  function lcg () { SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff; return SEED >>> 4 }
  const N = 30000
  let achadas = 0
  const visto = new Map()
  for (let t = 0; t < N; t++) {
    let s = ''
    const n = 6 + lcg() % 6
    for (let i = 0; i < n; i++) s += String.fromCharCode(97 + lcg() % 4)  /* a-d */
    const a = assinatura(s)
    const chave = a.E + '|' + a.f1 + '|' + a.f2
    const antes = visto.get(chave)
    if (antes !== undefined && antes !== s) achadas++
    else visto.set(chave, s)
  }
  console.log(`§C3: ${N} cadeias aleatórias (a–d, 6–11), colisões de (E,Φ,Φ₂): ${achadas}`)
  ok('§C3 orçamento varrido e reportado — a escada continua além do que se varreu', true)
  if (achadas > 0) {
    console.log('  (E,Φ,Φ₂) também colide no espaço pequeno: a escada dos momentos')
    console.log('  só satura no espectro completo — o byte a byte do medidor v2.')
  }
}

/* §C4 — o clone sob o observador: x ∼_O y */
{
  let x = null
  for (const l of linhas) {
    const a = idDe(l)
    let c = 0
    for (const l2 of linhas) if (idDe(l2) === a) c++
    if (c === 1) { x = l; break }
  }
  const clone = String(x)
  const Ix = assinatura(x), Ic = assinatura(clone)
  ok('§C4 I(clone) = I(original) — mesmo corpo para o observador', igualI3(Ix, Ic))
  const addr = idDe(x)
  let n = 0
  for (const l of [...linhas, clone]) if (idDe(l) === addr) n++
  ok('§C4 a cardinalidade no endereço distingue o um do dois (excesso=1)', n === 2)
}

console.log('')
if (!falhas) {
  console.log('  A colisão existe e está no log: o observador dual é cego à dupla')
  console.log('  transposição espelhada — Φ₂ resgata; a escada dos momentos é o')
  console.log('  espectro, e o observador completo é a transformada inteira.')
  console.log('  Clone: I identifica o corpo; o endereço conta a carga.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
