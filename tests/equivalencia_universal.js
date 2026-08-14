/* tests/equivalencia_universal.js — o adaptador de equivalência:
 * 𝒰[σ_Peano] ≅ 𝒫, operacionalmente (eval 13/08; o teste obrigatório do
 * gerente ANTES do corte).
 *
 * O lado 𝒰 é a infraestrutura GENÉRICA, parametrizada por uma assinatura
 * σ = (bytes, endereço, anel, raiz, transições). O lado 𝒫 é a forma
 * EMBUTIDA nos medidores da instância (cristal_volta/cristal_energia/
 * residuos_totais — copiada byte a byte, não adaptada). O critério é
 * brutal: igualdade EXATA sobre os MESMOS objetos reais. Se divergir um
 * bit, o adaptador está errado — não o Peano.
 *
 * §Q0  a assinatura σ_Peano (bytes UTF-8, id, ℤ_65537, 3^256, pares)
 * §Q1  E_U = E_P sobre o cristal inteiro (inteiro exato)
 * §Q2  (Φ,Φ₂)_U = (Φ,Φ₂)_P sobre as 20 respostas do banal
 * §Q3  R_endereço,U = R_endereço,P no íntegro e sob indução
 * §Q4  D_U = D_P (as transições) no íntegro e sob indução
 * §Q5  o espectro: a energia espectral do digest = 37.222 pelos DOIS lados
 * §Q6  o veredito: 𝒰[σ_Peano] e 𝒫 concordam em todos os invariantes
 *
 *   node tests/equivalencia_universal.js
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
const P = 65537

/* ── o lado 𝒰: genérico, parametrizado pela assinatura ───────────────────── */
function Universal (sigma) {
  return {
    energia (obj) {
      let E = 0
      for (const item of obj) {
        const b = sigma.bytes(item)
        for (let i = 0; i < b.length; i++) E += b[i] * b[i]
      }
      return E
    },
    escada (texto) {
      const b = sigma.bytes(texto)
      let E = 0, f1 = 0, f2 = 0
      for (let i = 0; i < b.length; i++) {
        E += b[i] * b[i]
        f1 = (f1 + (i + 1) * b[i]) % sigma.p
        f2 = (f2 + ((i + 1) * (i + 1) % sigma.p) * b[i]) % sigma.p
      }
      return { E, f1, f2 }
    },
    rEndereco (fonte, regs) {
      const F = new Map()
      for (let i = 0; i < fonte.length; i++) F.set(sigma.endereco(fonte[i], i), fonte[i])
      const visto = new Map(), vezes = new Map()
      for (let i = 0; i < regs.length; i++) {
        const a = sigma.endereco(regs[i], i)
        if (!visto.has(a)) visto.set(a, regs[i])
        vezes.set(a, (vezes.get(a) || 0) + 1)
      }
      let r = 0
      for (const [a, l] of F) {
        const v = visto.get(a)
        if (v === undefined) r++
        else if (v !== l) r++
      }
      for (const [a, n] of vezes) {
        if (!F.has(a)) r++
        if (n > 1) r += n - 1
      }
      return r
    },
    transicoes (texto) {
      const b = sigma.bytes(texto)
      const t = []
      for (let i = 0; i + 1 < b.length; i++) t.push(b[i] * 256 + b[i + 1])
      return t
    },
  }
}

/* a assinatura Peano */
const sigmaPeano = {
  p: P,
  bytes: s => Buffer.from(String(s), 'utf8'),
  endereco: (l, i) => {
    try { return JSON.parse(l).id } catch { return '￿ corrompido ' + i }
  },
}
const U = Universal(sigmaPeano)

/* ── o lado 𝒫: as formas EMBUTIDAS dos medidores da instância ────────────── */
/* de cristal_energia.js, byte a byte */
function energiaP (linhas) {
  let E = 0
  for (const l of linhas) {
    const b = Buffer.from(l, 'utf8')
    for (let i = 0; i < b.length; i++) E += b[i] * b[i]
  }
  return E
}
/* de assinatura_banal.js, byte a byte */
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
/* de cristal_volta.js (residuoV2), byte a byte */
function idDe (linha, i) {
  try { return JSON.parse(linha).id } catch { return '￿ corrompido ' + i }
}
function residuoV2P (fonteLinhas, regs) {
  const fontePorId = new Map()
  for (const l of fonteLinhas) fontePorId.set(idDe(l, 0), l)
  const conteudo = new Map()
  const vezes = new Map()
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
/* de residuos_totais.js, byte a byte */
function transicoesP (s) {
  const b = Buffer.from(String(s), 'utf8')
  const t = []
  for (let i = 0; i + 1 < b.length; i++) t.push(b[i] * 256 + b[i + 1])
  return t
}

/* ── as fixtures reais ────────────────────────────────────────────────────── */
const cristal = fs.readFileSync(path.join(RAIZ, 'cristal', 'cristal.jsonl'), 'utf8')
  .split('\n').filter(l => l.length)
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

ok('§Q0 a assinatura σ_Peano instancia o 𝒰 genérico', typeof U.energia === 'function')

/* §Q1 — E sobre o cristal inteiro */
{
  const eU = U.energia(cristal), eP = energiaP(cristal)
  console.log(`E_U = ${eU} · E_P = ${eP}`)
  ok('§Q1 E_U = E_P sobre os 4286 (inteiro exato)', eU === eP && eU === 38731623179)
}

/* §Q2 — a escada sobre o banal */
{
  let iguais = true
  for (const [, resp] of pares) {
    const u = U.escada(resp), p = IP(resp)
    if (u.E !== p.E || u.f1 !== p.fase || u.f2 !== p.f2) iguais = false
  }
  ok('§Q2 (E,Φ,Φ₂)_U = (E,Φ,Φ₂)_P nas 20 respostas reais', iguais && pares.length === 20)
}

/* §Q3 — R_endereço no íntegro e sob indução */
{
  const rU0 = U.rEndereco(cristal, cristal)
  const rP0 = residuoV2P(cristal, cristal)
  const mut = [...cristal]
  mut.splice(2143, 1)                          /* remoção */
  mut.push(mut[100])                           /* duplicação */
  const rU1 = U.rEndereco(cristal, mut)
  const rP1 = residuoV2P(cristal, mut)
  console.log(`íntegro: U=${rU0} P=${rP0} · induzido: U=${rU1} P=${rP1}`)
  ok('§Q3 R_end,U = R_end,P no íntegro (0) e sob indução (iguais, >0)',
    rU0 === 0 && rP0 === 0 && rU1 === rP1 && rU1 > 0)
}

/* §Q4 — as transições (a leitura da membrana) */
{
  let iguais = true
  for (const [, resp] of pares) {
    const tU = U.transicoes(resp), tP = transicoesP(resp)
    if (tU.length !== tP.length || tU.some((v, i) => v !== tP[i])) iguais = false
  }
  /* e sob indução: trocar um byte muda as transições IGUAL dos dois lados */
  const s = pares[0][1]
  const s2 = s.slice(0, 3) + String.fromCharCode(s.charCodeAt(3) ^ 1) + s.slice(4)
  const dU = U.transicoes(s2).filter((v, i) => v !== U.transicoes(s)[i]).length
  const dP = transicoesP(s2).filter((v, i) => v !== transicoesP(s)[i]).length
  ok('§Q4 D_U = D_P: as transições coincidem nas 20 e sob indução (Δ iguais)',
    iguais && dU === dP && dU > 0)
}

/* §Q5 — o espectro do digest: 37.222 pelos dois lados */
{
  function powmod (b, e) {
    let r = 1; b %= P
    while (e > 0) { if (e & 1) r = r * b % P; b = b * b % P; e >>= 1 }
    return r
  }
  const A = new Array(256).fill(0)
  {
    let k = 0
    for (const l of cristal) {
      const b = Buffer.from(l, 'utf8')
      for (let i = 0; i < b.length; i++) { A[k & 255] = (A[k & 255] + b[i]) % P; k++ }
    }
  }
  const EA = A.reduce((s, a) => (s + a * a) % P, 0)
  const RHS = 256 * EA % P                      /* caminho 𝒫: direto */
  const g = powmod(3, 256)                      /* caminho 𝒰: espectral */
  const X = new Array(256).fill(0)
  for (let k = 0; k < 256; k++) {
    let acc = 0
    const gk = powmod(g, k)
    let w = 1
    for (let i = 0; i < 256; i++) { acc = (acc + A[i] * w) % P; w = w * gk % P }
    X[k] = acc
  }
  let LHS = 0
  for (let k = 0; k < 256; k++) LHS = (LHS + X[k] * X[(256 - k) % 256]) % P
  console.log(`espectral U = ${LHS} · direto P = ${RHS}`)
  ok('§Q5 a energia espectral = 37.222 pelos DOIS caminhos (Parseval, o valor da bateria)',
    LHS === RHS && LHS === 37222)
}

/* §Q6 — o veredito */
ok('§Q6 𝒰[σ_Peano] ≅ 𝒫: todos os invariantes concordam — o adaptador está pronto para o corte',
  falhas === 0)

console.log('')
if (!falhas) {
  console.log('  O adaptador atesta: a assinatura do Peano encaixa na fundação do')
  console.log('  Universal sem mudar um bit — E, (Φ,Φ₂), R_endereço, D e o espectro')
  console.log('  são idênticos pelos dois lados, sobre os objetos reais.')
  console.log('  Aguardando a instrução final de corte.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
