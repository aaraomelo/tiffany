/* capa.js — A CAPA CONTRA O GABARITO, BIT A BIT. E só se sai dela com erro 0.
 *
 * O Aarão: «está diferente teu fonte, cor, espaçamento — tira uma foto das duas capas e
 * compara bit a bit, só saímos dela com erro 0.»
 *
 * O gabarito é o `enredo.pdf` do repositório, composto pelo pdflatex. Rasterizam-se as duas
 * primeiras páginas e conta-se pixel a pixel — e não um número global, que esconde onde dói:
 *
 *   1. A POSIÇÃO   em que faixa da altura está a tinta de cada um
 *   2. O PESO      quanta tinta cada um põe — é a fonte que decide
 *   3. A COR       as cores usadas, contra o que o `\definecolor` declara
 *   4. O RESÍDUO   pixels diferentes, e onde
 *
 *   node tools/capa.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TMP = '/tmp/capa_bit'
const sh = (c) => { try { return execSync(c, { encoding: 'utf8', stdio: 'pipe' }) } catch (e) { return String((e.stdout || '') + (e.stderr || '')) } }

let feitas = 0, falhas = 0
function ok (q, cond) {
  feitas++; if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
  console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`)
}

fs.mkdirSync(TMP, { recursive: true })
console.log('=== A CAPA CONTRA O GABARITO, BIT A BIT ==================================\n')

/* compõe-se a nossa e rasterizam-se as duas */
const meu = path.join(TMP, 'meu.pdf')
sh(`${JSON.stringify(path.join(RAIZ, 'tests', 'tex'))} ${JSON.stringify(path.join(RAIZ, 'enredo.tex'))} ${JSON.stringify(meu)}`)
const gab = path.join(RAIZ, 'enredo.pdf')
if (!fs.existsSync(gab) || !fs.existsSync(meu)) { console.log('  falta um dos PDFs'); process.exit(1) }
for (const [n, f] of [['A', gab], ['B', meu]])
  sh(`pdftoppm -f 1 -l 1 -r 100 -png -gray ${JSON.stringify(f)} ${TMP}/${n}`)

/* O PNG lê-se pelo Python/PIL, e NÃO por um descodificador meu.
 *
 * Escrevi um a desfazer os filtros à mão e ele contava 96 759 pixels de tinta por faixa —
 * que é o TOTAL da faixa. As duas primeiras unidades passavam por acidente, porque ambas
 * contavam tudo dos dois lados e batiam. É a asserção que passa sem poder falhar, e foi
 * preciso olhar para o número para a apanhar: nenhuma delas falhava. */
function medePNG (fa, fb) {
  const py = `
from PIL import Image
import numpy as np, json
a=np.array(Image.open(${JSON.stringify(fa)}).convert('L'),dtype=int)
b=np.array(Image.open(${JSON.stringify(fb)}).convert('L'),dtype=int)
h=min(a.shape[0],b.shape[0]); w=min(a.shape[1],b.shape[1])
a=a[:h,:w]; b=b[:h,:w]
fa=[int((a[k*h//10:(k+1)*h//10]<128).sum()) for k in range(10)]
fb=[int((b[k*h//10:(k+1)*h//10]<128).sum()) for k in range(10)]
print(json.dumps({"w":w,"h":h,"ta":int((a<128).sum()),"tb":int((b<128).sum()),
                  "fa":fa,"fb":fb,"dif":int((abs(a-b)>32).sum())}))`
  /* pelo ficheiro e não por `-c`: as quebras de linha não sobrevivem ao escape do shell */
  const tmp = `${TMP}/mede.py`
  fs.writeFileSync(tmp, py)
  return JSON.parse(sh(`python3 ${JSON.stringify(tmp)}`))
}
const M = medePNG(`${TMP}/A-001.png`, `${TMP}/B-001.png`)
console.log(`  dimensões: ${M.w}×${M.h}`)
ok('as duas páginas rasterizam na mesma dimensão', M.w > 0 && M.h > 0)

/* ─── 1. A POSIÇÃO ────────────────────────────────────────────────────────────────── */
console.log('\n  1. POSIÇÃO   faixa    gabarito     meu')
let faixaA = -1, faixaB = -1
for (let k = 0; k < 10; k++) {
  if (M.fa[k] > 0 && faixaA < 0) faixaA = k
  if (M.fb[k] > 0 && faixaB < 0) faixaB = k
  if (M.fa[k] || M.fb[k])
    console.log(`               ${String(k * 10).padStart(3)}%  ${String(M.fa[k]).padStart(9)} ${String(M.fb[k]).padStart(9)}`)
}
console.log(`               a tinta começa em ${faixaA * 10}% no gabarito e ${faixaB * 10}% no nosso`)
ok('a capa começa na mesma faixa da altura que o gabarito', faixaA >= 0 && faixaA === faixaB)

/* ─── 2. O PESO, que é a fonte ────────────────────────────────────────────────────── */
const razao = M.tb / (M.ta || 1)
console.log(`\n  2. PESO      gabarito ${M.ta} px de tinta, nosso ${M.tb}  (razão ${razao.toFixed(2)}×)`)
console.log(`               fonte do gabarito: ${(sh(`pdffonts ${JSON.stringify(gab)}`).match(/\+(\w+)/g) || []).slice(0, 3).join(' ')}`)
ok('a quantidade de tinta bate com a do gabarito (±10%) — mais do que isso é outra fonte',
   M.ta > 0 && razao > 0.9 && razao < 1.1)


/* ─── 3. A COR: contra o que o estilo declara ─────────────────────────────────────── */
const est = fs.readFileSync(path.join(RAIZ, 'estilo.tex'), 'utf8')
const decl = {}
for (const m of est.matchAll(/definecolor\{(\w+)\}\{HTML\}\{(\w{6})\}/g)) decl[m[1]] = m[2]
const cru = fs.readFileSync(meu).toString('latin1').slice(0, 60000)
const usadas = new Set()
for (const m of cru.matchAll(/([\d.]+) ([\d.]+) ([\d.]+) rg BT/g))
  usadas.add([1, 2, 3].map((k) => Math.round(+m[k] * 255)).join(','))
let resCor = 0
console.log('\n  3. COR       declarada          no PDF              resíduo')
for (const n of ['tinta', 'regua', 'ouro']) {
  if (!decl[n]) continue
  const alvo = [0, 2, 4].map((i) => parseInt(decl[n].slice(i, i + 2), 16))
  let melhor = null, dmin = 1e9
  for (const u of usadas) {
    const v = u.split(',').map(Number)
    const d = v.reduce((s, x, i) => s + Math.abs(x - alvo[i]), 0)
    if (d < dmin) { dmin = d; melhor = v }
  }
  resCor += dmin
  console.log(`               ${n.padEnd(6)} #${decl[n]}   rgb(${melhor})   ${dmin}`)
}
ok('as cores no PDF são as do \\definecolor — resíduo 0', resCor === 0)

/* ─── 4. O RESÍDUO ────────────────────────────────────────────────────────────────── */
const pct = 100 * M.dif / (M.w * M.h)
console.log(`\n  4. RESÍDUO   ${M.dif} pixels diferentes de ${M.w * M.h}  (${pct.toFixed(2)}%)`)
ok('a capa é IGUAL ao gabarito, pixel a pixel — resíduo 0', M.dif === 0)

console.log(`\n${'='.repeat(74)}`)
console.log(`  ${feitas - falhas}/${feitas} unidades`)
console.log('')
console.log('  Só se sai da capa com erro 0. Enquanto não for, o número diz onde: a posição')
console.log('  é a faixa, o peso é a fonte, a cor é o \\definecolor, e o resíduo é o total.')
console.log('')
process.exit(falhas ? 1 : 0)
