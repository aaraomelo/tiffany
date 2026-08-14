/* tests/cristal_adversarial.js — a caçada ao falso positivo (eval 13/08).
 *
 * A pergunta do gerente: «conservar energia é suficiente para conservar o
 * corpo?» A ordem do diretor: gerar transformações T ao acaso e encontrar a
 * maldita — ΔE=0 com a identidade corrompida. A resposta do coordenador já
 * era a chave: «a energia carrega ASSINATURA; o primeiro bit decodificado
 * incorretamente já rejeita a iteração».
 *
 * Três réguas sobre a MESMA transformação:
 *   E escalar     ΔE = Σb²(depois) − Σb²(antes)      — um número só
 *   R_endereço    faltantes+alterados+excedentes+duplicados (medidor v2)
 *   assinatura    (E_a, fase_a) POR ENDEREÇO — fase = Σ i·b_i: a posição
 *                 interna; a transposição não engana a fase
 *
 * §A0  permutação: (ΔE, R_end) = (0, 0) — admissível pelas duas
 * §A1  A MALDITA 1: transposição interna — ΔE=0 ∧ R_end>0 (falso positivo
 *      da energia escalar); a FASE por endereço acusa
 * §A2  A MALDITA 2: reversão de trecho — idem
 * §A3  A MALDITA 3: troca de conteúdo entre dois endereços — ΔE=0 (o
 *      multiconjunto de bytes conserva) ∧ R_end=2; a energia POR ENDEREÇO acusa
 * §A4  flip de byte: a escalar apanha (controlo)
 * §A5  o veredito: R_total = (R_endereço, R_E) — os DOIS invariantes; e o
 *      portão do primeiro erro rejeita cedo (posição medida)
 *
 * Inteiro puro; LCG determinístico; T=200 por categoria sobre os 4286.
 *
 *   node tests/cristal_adversarial.js
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

const FONTE = path.join(__dirname, '..', 'cristal', 'cristal.jsonl')
const linhas = fs.readFileSync(FONTE, 'utf8').split('\n').filter(l => l.length)
const P = 65537

let SEED = 13
function lcg () {
  SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff
  return SEED >>> 4
}

function idDe (l, i) {
  const m = /"id":"((?:[^"\\]|\\.)*)"/.exec(l)
  return m ? m[1] : '￿ corrompido ' + i
}

/* energia e fase de UM registo (assinatura por endereço) — memoizado: E é
 * função pura do texto, a memoização não muda o valor */
const memo = new Map()
function ass (l) {
  let v = memo.get(l)
  if (v) return v
  const b = Buffer.from(l, 'utf8')
  let E = 0, fase = 0
  for (let i = 0; i < b.length; i++) { E += b[i] * b[i]; fase = (fase + (i + 1) * b[i]) % P }
  v = { E, fase }
  memo.set(l, v)
  return v
}
function energiaTotal (regs) {
  let E = 0
  for (const l of regs) E += ass(l).E
  return E
}

const E0 = energiaTotal(linhas)
const fonteMap = new Map()
for (let i = 0; i < linhas.length; i++) fonteMap.set(idDe(linhas[i], i), linhas[i])

function rEndereco (regs) {
  const visto = new Map(), vezes = new Map()
  for (let i = 0; i < regs.length; i++) {
    const a = idDe(regs[i], i)
    if (!visto.has(a)) visto.set(a, regs[i])
    vezes.set(a, (vezes.get(a) || 0) + 1)
  }
  let r = 0
  for (const [a, l] of fonteMap) {
    const v = visto.get(a)
    if (v === undefined) r++
    else if (v !== l) r++
  }
  for (const [a, n] of vezes) {
    if (!fonteMap.has(a)) r++
    if (n > 1) r += n - 1
  }
  return r
}

/* assinatura por endereço: quantos endereços têm (E, fase) diferente da fonte;
 * e a posição do PRIMEIRO byte errado no primeiro endereço acusado */
function rAssinatura (regs) {
  const visto = new Map()
  for (let i = 0; i < regs.length; i++) visto.set(idDe(regs[i], i), regs[i])
  let r = 0, primeiraPos = -1
  for (const [a, l] of fonteMap) {
    const v = visto.get(a)
    if (v === undefined) { r++; continue }
    const s0 = ass(l), s1 = ass(v)
    if (s0.E !== s1.E || s0.fase !== s1.fase) {
      r++
      if (primeiraPos < 0) {
        const n = Math.min(l.length, v.length)
        for (let i = 0; i < n; i++) {
          if (l.charCodeAt(i) !== v.charCodeAt(i)) { primeiraPos = i; break }
        }
        if (primeiraPos < 0) primeiraPos = n
      }
    }
  }
  return { r, primeiraPos }
}

/* ── as transformações ────────────────────────────────────────────────────── */
function span (l) {
  const p = l.indexOf('"descricao":"')
  if (p < 0) return null
  const ini = p + 13
  let fim = ini
  while (fim < l.length && !(l[fim] === '"' && l[fim - 1] !== '\\')) fim++
  return fim - ini > 8 ? [ini, fim] : null
}

function tPermuta (regs) {
  for (let i = regs.length - 1; i > 0; i--) {
    const j = lcg() % (i + 1)
    const t = regs[i]; regs[i] = regs[j]; regs[j] = t
  }
  return true
}
function tTranspoe (regs) {
  for (let t = 0; t < 64; t++) {
    const i = lcg() % regs.length
    const sp = span(regs[i])
    if (!sp) continue
    const q = sp[0] + lcg() % (sp[1] - sp[0] - 1)
    const a = regs[i][q], b = regs[i][q + 1]
    if (a === b || a === '\\' || b === '\\' || a === '"' || b === '"') continue
    regs[i] = regs[i].slice(0, q) + b + a + regs[i].slice(q + 2)
    return true
  }
  return false
}
function tReverte (regs) {
  for (let t = 0; t < 64; t++) {
    const i = lcg() % regs.length
    const sp = span(regs[i])
    if (!sp) continue
    const ini = sp[0], fim = Math.min(sp[1], ini + 8 + lcg() % 12)
    const tre = regs[i].slice(ini, fim)
    if (tre.includes('\\') || tre.includes('"')) continue
    const rev = [...tre].reverse().join('')
    if (rev === tre) continue
    regs[i] = regs[i].slice(0, ini) + rev + regs[i].slice(fim)
    return true
  }
  return false
}
function tTrocaConteudo (regs) {
  for (let t = 0; t < 64; t++) {
    const i = lcg() % regs.length
    const j = lcg() % regs.length
    if (i === j) continue
    const si = span(regs[i]), sj = span(regs[j])
    if (!si || !sj) continue
    const di = regs[i].slice(si[0], si[1]), dj = regs[j].slice(sj[0], sj[1])
    if (di === dj) continue
    regs[i] = regs[i].slice(0, si[0]) + dj + regs[i].slice(si[1])
    regs[j] = regs[j].slice(0, sj[0]) + di + regs[j].slice(sj[1])
    return true
  }
  return false
}
function tFlip (regs) {
  for (let t = 0; t < 64; t++) {
    const i = lcg() % regs.length
    const sp = span(regs[i])
    if (!sp) continue
    const q = sp[0] + lcg() % (sp[1] - sp[0])
    const c = regs[i][q]
    if (c === '\\' || c === '"') continue
    regs[i] = regs[i].slice(0, q) +
      String.fromCharCode(regs[i].charCodeAt(q) ^ 1) + regs[i].slice(q + 1)
    return true
  }
  return false
}

const T = 200
const CATS = [
  ['§A0 permutação (admissível)', tPermuta, { dE0: true, rEnd0: true }],
  ['§A1 transposição interna', tTranspoe, { dE0: true, rEnd0: false }],
  ['§A2 reversão de trecho', tReverte, { dE0: true, rEnd0: false }],
  ['§A3 troca de conteúdo entre endereços', tTrocaConteudo, { dE0: true, rEnd0: false }],
  ['§A4 flip de byte (controlo)', tFlip, { dE0: false, rEnd0: false }],
]

console.log('=== CAÇADA AO FALSO POSITIVO — T=%d por categoria, n=%d ===\n', T, linhas.length)
console.log('categoria                                ΔE=0    R_end>0  assin.acusa  1ºerro médio')

let malditasEncontradas = 0
let assinaturaFalhou = 0
for (const [nome, transforma, espera] of CATS) {
  SEED = 13
  let nDE0 = 0, nREnd = 0, nAss = 0, somaPos = 0, nPos = 0, feitos = 0
  for (let t = 0; t < T; t++) {
    const regs = [...linhas]
    if (!transforma(regs)) continue
    feitos++
    const dE = energiaTotal(regs) - E0
    const rE = rEndereco(regs)
    const ra = rAssinatura(regs)
    if (dE === 0) nDE0++
    if (rE > 0) nREnd++
    if (ra.r > 0) { nAss++; if (ra.primeiraPos >= 0) { somaPos += ra.primeiraPos; nPos++ } }
    if (dE === 0 && rE > 0) malditasEncontradas++
    if (rE > 0 && ra.r === 0) assinaturaFalhou++
  }
  const posMedia = nPos ? Math.round(somaPos / nPos) : -1
  console.log(`${nome.padEnd(40)} ${String(nDE0).padStart(4)}/${feitos}  ${String(nREnd).padStart(4)}/${feitos}  ${String(nAss).padStart(6)}/${feitos}  ${posMedia >= 0 ? 'pos ' + posMedia : '—'}`)
  if (espera.dE0 && espera.rEnd0) {
    ok(nome + ': (ΔE, R_end) = (0,0) — admissível', nDE0 === feitos && nREnd === 0 && feitos > 0)
  } else if (espera.dE0) {
    ok(nome + ': MALDITA — ΔE=0 e R_end>0 (a escalar não vê)', nDE0 === feitos && nREnd === feitos && feitos > 0)
    ok(nome + ': a assinatura por endereço acusa TODAS', nAss === feitos)
  } else {
    ok(nome + ': a escalar apanha (ΔE≠0)', nDE0 === 0 && feitos > 0)
  }
}

console.log('')
console.log(`malditas encontradas (ΔE=0 ∧ R_end>0): ${malditasEncontradas}`)
console.log(`corrupções que escaparam à assinatura por endereço: ${assinaturaFalhou}`)
ok('§A5 o falso positivo da energia escalar EXISTE (a pergunta do gerente: não basta)',
  malditasEncontradas > 0)
ok('§A5 a assinatura por endereço (energia+fase) não deixou escapar nenhuma',
  assinaturaFalhou === 0)

console.log('')
if (!falhas) {
  console.log('  Conservar energia NÃO é suficiente para conservar o corpo:')
  console.log('  a maldita existe (ΔE=0, identidade corrompida) e foi encontrada.')
  console.log('  O fecho é o par: R_total = (R_endereço, R_E) = 0 — e a assinatura')
  console.log('  (energia+fase POR ENDEREÇO) rejeita no primeiro byte errado.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
