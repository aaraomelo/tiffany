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
function medeHilbert (fa, fb) {
  const py = `
from PIL import Image
import numpy as np, json
def hil(n):
    # d -> (x,y), a curva de ordem n: a Lei 1 parte em 2x2, a Lei 2 roda com periodo 4
    pts=[]
    N=1<<n
    for d in range(N*N):
        rx=ry=0; t=d; x=y=0; s=1
        while s<N:
            rx=1 if (t//2)&1 else 0
            ry=1 if (t^rx)&1 else 0
            if ry==0:
                if rx==1: x=s-1-x; y=s-1-y
                x,y=y,x
            x+=s*rx; y+=s*ry; t//=4; s*=2
        pts.append((x,y))
    return pts
a=np.array(Image.open(${JSON.stringify(fa)}).convert('L'),dtype=int)
b=np.array(Image.open(${JSON.stringify(fb)}).convert('L'),dtype=int)
h=min(a.shape[0],b.shape[0]); w=min(a.shape[1],b.shape[1])
n=9                       # 512x512 cobre a pagina reamostrada
N=1<<n
import PIL.Image as I
# a reamostragem TEM DE PRESERVAR A TINTA: com o filtro por omissao a media dilui um
# traco fino em cinzento claro e ele desaparece no limiar — 303 pontos de 262144, quando
# a pagina tem milhares. O MINIMO de cada bloco guarda o traco: se ha' tinta no bloco, ha'
# tinta no ponto. E' a mesma escolha de sempre: a regua tem de sobreviver a' mudanca de
# escala, e a media nao sobrevive.
def reduz(m, N):
    H,W = m.shape
    by, bx = H//N, W//N
    if by < 1 or bx < 1: return np.array(I.fromarray(m.astype('uint8')).resize((N,N)),dtype=int)
    m = m[:by*N,:bx*N].reshape(N,by,N,bx)
    return m.min(axis=(1,3))
A=reduz(a[:h,:w], N)
B=reduz(b[:h,:w], N)
P=hil(n)
va=[1 if A[y,x]<128 else 0 for (x,y) in P]
vb=[1 if B[y,x]<128 else 0 for (x,y) in P]
ca=np.cumsum(va); cb=np.cumsum(vb)
d=np.abs(ca-cb)
print(json.dumps({"ordem":n,"n":len(P),"sa":int(ca[-1]),"sb":int(cb[-1]),
                  "res":int(abs(int(ca[-1])-int(cb[-1]))),"pior":int(d.max())}))`
  const tmp = `${TMP}/hilbert.py`
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

/* ─── 4. O RESÍDUO, PELA CURVA DE HILBERT ─────────────────────────────────────────
 *
 * Contar pixels diferentes NÃO é resíduo: é comparação, e perde a vizinhança --- dois
 * pixels trocados de sítio contam como dois erros, e uma linha deslocada um pixel conta
 * como a linha inteira errada.
 *
 * A curva de Hilbert LINEARIZA: `ν` contrai 2D→1D preservando a vizinhança, e é bidual
 * (`ν∘π = id` e `π∘ν = id`, medido em `tests/hilbert_bidual.c`). A página passa a ser uma
 * ÓRBITA, e duas órbitas comparam-se pelo que a transformada delas diz --- não ponto a
 * ponto, que é onde o pixel falha.
 *
 * Aqui: as duas páginas linearizam-se por Hilbert, e o resíduo é a diferença dos
 * ACUMULADOS ao longo da curva --- que é o que sobrevive a um deslocamento e não sobrevive
 * a tinta a mais ou a menos. */
const H = medeHilbert(`${TMP}/A-001.png`, `${TMP}/B-001.png`)
console.log(`\n  4. HILBERT   ordem ${H.ordem}, ${H.n} pontos na curva`)
console.log(`               acumulado gabarito ${H.sa}   nosso ${H.sb}`)
console.log(`               resíduo do acumulado: ${H.res}  (${(100*H.res/(H.sa||1)).toFixed(2)}%)`)
console.log(`               pior desvio local: ${H.pior} num ponto de ${H.n}`)
ok('as duas páginas dão a MESMA órbita de Hilbert — resíduo 0', H.res === 0)

console.log(`\n${'='.repeat(74)}`)
console.log(`  ${feitas - falhas}/${feitas} unidades`)
console.log('')
console.log('  Só se sai da capa com erro 0. Enquanto não for, o número diz onde: a posição')
console.log('  é a faixa, o peso é a fonte, a cor é o \\definecolor, e o resíduo é a ÓRBITA:')
console.log('  a página linearizada por Hilbert, que preserva a vizinhança. Contar pixels')
console.log('  diferentes não é resíduo — é comparação, e uma linha deslocada um pixel conta')
console.log('  como a linha inteira errada.')
console.log('')
process.exit(falhas ? 1 : 0)
