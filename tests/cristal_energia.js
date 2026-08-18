/* tests/cristal_energia.js — a verificação POR ENERGIA, não analítica (eval 13/08).
 *
 * A ordem do coordenador: trocar perturbação por INDUÇÃO; o dual META-INDUÇÃO
 * valida a CONSERVAÇÃO DE ENERGIA. A energia vem dos conceitos do repo:
 *
 *   energia  E(x) = Σ x²  — o segundo momento, a MASSA da cruz
 *            (corpo_topologico prop:massa; corpo_analitico: E=m pela régua ao
 *            quadrado; «a energia não se perde ao atravessar — muda de
 *            domínio»);
 *   anel     ℤ_65537, N=2^8, raiz 3^256 — a transformada dourada da Lei 8
 *            (o /AssinaturaOito do tradutor, tests/tex_core.c);
 *   lei      Parseval: Σ X_k·X_{−k} = N·Σ x² — a conservação da norma, o
 *            mesmo N(xy)=N(x)N(y) de Hurwitz do lado discreto e a soma
 *            reversível de Gentil do lado contínuo (teorema central).
 *
 * §E0  a volta do cristal POR ENERGIA: E(fonte) == E(reconstrução) — não se
 *      compara um byte com um byte; compara-se a norma (inteiro exato)
 * §E1  Parseval 1D no anel: digest A[256] (a acumulação do AssinaturaOito),
 *      espectro pela raiz 3^256, Σ X·X_− ≡ 256·Σ A²  (mod p)
 * §E2  Parseval 2D (16×16, raiz 3^4096 de ordem 16): fator 16·16 = 256
 * §E3  Parseval 4D (4×4×4×4, raiz 3^16384 de ordem 4 — o período do i):
 *      fator 4^4 = 256. Três fatorações da MESMA dimensão, a mesma energia:
 *      a conservação não vê a fatoração, só a dimensão total (Parseval
 *      multidimensão do teorema central — eixos compõem por produto, Fubini)
 * §E4  a raiz errada (3^128, ordem 512) NÃO fecha — a asserção pode falhar
 * §E5  MATRIZ DE INDUÇÃO por energia: a indução muda E; a meta-indução
 *      (o caminho espectral + a diferença de energia) valida a conservação
 *
 *   node tests/cristal_energia.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
/* limpeza da dupla árvore (ordem do diretor, 14/08): a forma embutida
 * saiu — a única implementação é a da infraestrutura (lib) */
const { Universal } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')
const U = Universal(sigmaPeano)

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const RAIZ = path.join(__dirname, '..')
const FONTE = path.join(RAIZ, 'cristal', 'cristal.jsonl')
/* os cristal_*.tex vivem em cristal/, junto ao cristal.jsonl de que são a face
 * embebida — papers/ ficou só com o fundo (universal, topológico, analítico). */
const PAPERS = path.join(RAIZ, 'cristal')
const P = 65537

/* ── energia exata (inteiro, sem mod): E = Σ byte² — a da lib ─────────────── */
const energia = linhas => U.energia(linhas)

const fonteLinhas = fs.readFileSync(FONTE, 'utf8').split('\n').filter(l => l.length)
const texs = fs.readdirSync(PAPERS).filter(f => /^cristal_.*\.tex$/.test(f)).sort()
const reconstruido = []
for (const f of texs) {
  for (const linha of fs.readFileSync(path.join(PAPERS, f), 'utf8').split('\n')) {
    if (linha.startsWith('%CRISTAL ')) reconstruido.push(linha.slice(9))
  }
}

/* §E0 — a volta por energia */
const E_fonte = energia(fonteLinhas)
const E_rec = energia(reconstruido)
console.log(`E(fonte) = ${E_fonte}   E(reconstrução) = ${E_rec}`)
ok('§E0 conservação: E(fonte) == E(reconstrução), inteiro exato',
  E_fonte === E_rec && E_fonte > 0)

/* ── o anel da Lei 8 ──────────────────────────────────────────────────────── */
function powmod (b, e) {
  let r = 1; b %= P
  while (e > 0) {
    if (e & 1) r = r * b % P
    b = b * b % P
    e >>= 1
  }
  return r
}

/* digest: a acumulação do AssinaturaOito — A[k mod 256] += byte (mod p) */
const A = new Array(256).fill(0)
{
  let k = 0
  for (const l of fonteLinhas) {
    const b = Buffer.from(l, 'utf8')
    for (let i = 0; i < b.length; i++) { A[k & 255] = (A[k & 255] + b[i]) % P; k++ }
  }
}
const EA = A.reduce((s, a) => (s + a * a) % P, 0)   /* Σ A² mod p */
const RHS = 256 * EA % P                             /* o fator é a dimensão total */

/* §E1 — Parseval 1D pela raiz de ordem 256 */
function ntt1d (v, g) {
  const N = v.length, X = new Array(N).fill(0)
  for (let k = 0; k < N; k++) {
    let acc = 0
    const gk = powmod(g, k)
    let w = 1
    for (let i = 0; i < N; i++) { acc = (acc + v[i] * w) % P; w = w * gk % P }
    X[k] = acc
  }
  return X
}
function parseval1d (X) {
  let s = 0
  const N = X.length
  for (let k = 0; k < N; k++) s = (s + X[k] * X[(N - k) % N]) % P
  return s
}
{
  const X = ntt1d(A, powmod(3, 256))
  const LHS = parseval1d(X)
  console.log(`1D: Σ X·X_− = ${LHS}   256·Σ A² = ${RHS}   (mod ${P})`)
  ok('§E1 Parseval 1D no anel: espectro == 256 vezes a energia', LHS === RHS)
}

/* §E2 — Parseval 2D: 16×16, raiz de ordem 16; fator 16·16 = 256 */
{
  const g = powmod(3, 4096)
  const X = []
  for (let k = 0; k < 16; k++) {
    X.push(new Array(16).fill(0))
    for (let l = 0; l < 16; l++) {
      let acc = 0
      for (let r = 0; r < 16; r++) {
        for (let c = 0; c < 16; c++) {
          acc = (acc + A[16 * r + c] * powmod(g, (r * k + c * l) % 16)) % P
        }
      }
      X[k][l] = acc
    }
  }
  let LHS = 0
  for (let k = 0; k < 16; k++) {
    for (let l = 0; l < 16; l++) {
      LHS = (LHS + X[k][l] * X[(16 - k) % 16][(16 - l) % 16]) % P
    }
  }
  console.log(`2D: Σ X·X_− = ${LHS}   16·16·Σ A² = ${RHS}`)
  ok('§E2 Parseval 2D: dois eixos compõem por produto (Fubini)', LHS === RHS)
}

/* §E3 — Parseval 4D: 4×4×4×4, raiz de ordem 4 (o período do i); fator 4^4 */
{
  const g = powmod(3, 16384)
  const idx = (a, b, c, d) => 64 * a + 16 * b + 4 * c + d
  const X = new Array(256).fill(0)
  for (let ka = 0; ka < 4; ka++) {
    for (let kb = 0; kb < 4; kb++) {
      for (let kc = 0; kc < 4; kc++) {
        for (let kd = 0; kd < 4; kd++) {
          let acc = 0
          for (let a = 0; a < 4; a++) {
            for (let b = 0; b < 4; b++) {
              for (let c = 0; c < 4; c++) {
                for (let d = 0; d < 4; d++) {
                  const e = (a * ka + b * kb + c * kc + d * kd) % 4
                  acc = (acc + A[idx(a, b, c, d)] * powmod(g, e)) % P
                }
              }
            }
          }
          X[idx(ka, kb, kc, kd)] = acc
        }
      }
    }
  }
  let LHS = 0
  for (let ka = 0; ka < 4; ka++) {
    for (let kb = 0; kb < 4; kb++) {
      for (let kc = 0; kc < 4; kc++) {
        for (let kd = 0; kd < 4; kd++) {
          LHS = (LHS + X[idx(ka, kb, kc, kd)] *
            X[idx((4 - ka) % 4, (4 - kb) % 4, (4 - kc) % 4, (4 - kd) % 4)]) % P
        }
      }
    }
  }
  console.log(`4D: Σ X·X_− = ${LHS}   4^4·Σ A² = ${RHS}`)
  ok('§E3 Parseval 4D: quatro eixos de período 4, o mesmo fator 256', LHS === RHS)
}

/* §E4 — a asserção pode falhar: a raiz de ordem errada NÃO fecha */
{
  const X = ntt1d(A, powmod(3, 128))   /* ordem 512: não divide 256 */
  ok('§E4 raiz errada (ordem 512) quebra o Parseval — a régua tem gume',
    parseval1d(X) !== RHS)
}

/* §E5 — MATRIZ DE INDUÇÃO por energia: ΔE = E(induzido) − E(fonte) */
console.log('')
console.log('=== MATRIZ DE INDUÇÃO — verificação por energia (ΔE), não analítica ===')
function induz (nome, transforma, esperaConserva) {
  const copia = transforma([...reconstruido])
  const dE = energia(copia) - E_fonte
  const veredito = esperaConserva
    ? (dE === 0 ? 'CONSERVA (ΔE=0)' : 'REOPEN (DEFEITO: devia conservar)')
    : (dE !== 0 ? 'REOPEN (ΔE≠0)' : 'passou (DEFEITO)')
  console.log(`#IND ${nome.padEnd(44)} ΔE=${String(dE).padStart(9)} -> ${veredito}`)
  ok('§E5 ' + nome + (esperaConserva ? ' conserva (ΔE=0)' : ' acusa (ΔE≠0)'),
    esperaConserva ? dE === 0 : dE !== 0)
}
const meio = () => Math.floor(reconstruido.length / 2)

induz('permutação física (endereços intactos)', m => m.reverse(), true)
induz('conteúdo no endereço (1 byte)', m => {
  const alvo = m[meio()]
  const i = alvo.indexOf('"descricao":"') + 13
  m[meio()] = alvo.slice(0, i) +
    String.fromCharCode(alvo.charCodeAt(i) ^ 1) + alvo.slice(i + 1)
  return m
}, false)
induz('remoção de endereço', m => { m.splice(meio(), 1); return m }, false)
induz('duplicação de endereço', m => { m.push(m[meio()]); return m }, false)
induz('endereço destruído (registo ilegível)', m => {
  m[meio()] = m[meio()].slice(0, 20) + '<<<corrompido>>>'
  return m
}, false)

console.log('')
if (!falhas) {
  console.log('  A meta-indução valida a conservação: E(fonte) == E(reconstrução),')
  console.log('  e o Parseval dourado fecha em 1D, 2D e 4D com o MESMO fator 256 —')
  console.log('  a conservação não vê a fatoração, só a dimensão total.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
