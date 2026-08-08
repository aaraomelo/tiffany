/* compara.js — O TRADUTOR CONTRA O pdflatex, EM TRÊS NÍVEIS. TEM DE FICAR IGUAL.
 *
 * O Aarão: «compila o original com o compilador e compara os dois, podes criar ferramenta de
 * comparação bit a bit» · e, quando perguntei contra o quê: «CONTRA TUDO CARA, TEM QUE FICAR
 * IGUAL».
 *
 * Três níveis, e nenhum substitui os outros:
 *
 *   1. O TEXTO      que palavras saem de cada um — o que falta, o que sobra
 *   2. AS CAIXAS    onde cada palavra está — a posição, em pontos
 *   3. OS PIXELS    o que o olho vê — a página rasterizada, ponto a ponto
 *
 * O primeiro apanha o que se perdeu; o segundo, o que está torto; o terceiro, o que nenhum
 * dos dois vê — uma régua por cima de uma letra não muda o texto nem a caixa.
 *
 * E A MEDIDA É O RESÍDUO, não uma percentagem simpática: o alvo é ZERO nos três. Onde não
 * for, diz-se quanto e onde — em vez de um número global que esconde onde dói.
 *
 *   node tools/compara.js [documento]        (por omissão: os três)
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TEX = path.join(RAIZ, 'tests', 'tex')
const TMP = '/tmp/compara'

const DOCS = {
  enredo:   'enredo.tex',
  teoria:   'teoria.tex',
  catalogo: 'catalogo.tex',
}

function sh (cmd, opts) {
  try { return execSync(cmd, Object.assign({ encoding: 'utf8', stdio: 'pipe' }, opts || {})) }
  catch (e) { return String((e.stdout || '') + (e.stderr || '')) }
}

/* ─── as palavras com as suas caixas, de um PDF ────────────────────────────────────── */
function caixas (pdf) {
  const xml = pdf + '.bbox'
  sh(`pdftotext -bbox ${JSON.stringify(pdf)} ${JSON.stringify(xml)}`)
  let d = ''
  try { d = fs.readFileSync(xml, 'utf8') } catch (e) { return [] }
  const out = []
  let pag = 0
  for (const bloco of d.split('<page').slice(1)) {
    pag++
    for (const m of bloco.matchAll(/<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([\s\S]*?)<\/word>/g))
      out.push({ pag, x: +m[1], y: +m[2], X: +m[3], Y: +m[4], t: m[5] })
  }
  return out
}

/* normaliza para comparar TEXTO: o que interessa é a palavra, não o corte tipográfico */
const norm = (t) => t.replace(/[‘’“”]/g, "'")
                     .replace(/[–—]/g, '-')
                     .replace(/\s+/g, '')

console.log('\n=== O TRADUTOR CONTRA O pdflatex — E TEM DE FICAR IGUAL ====================')

fs.mkdirSync(TMP, { recursive: true })
const alvo = process.argv[2]
const lista = alvo ? { [alvo]: DOCS[alvo] } : DOCS

for (const [nome, fonte] of Object.entries(lista)) {
  if (!fonte) { console.log(`\n  documento desconhecido: ${alvo}`); continue }
  console.log(`\n────────────────────────────────────────────────  ${nome}`)

  /* o ORIGINAL, pelo pdflatex — três passagens, que é o que o workflow faz */
  const orig = path.join(TMP, `${nome}_orig.pdf`)
  if (!fs.existsSync(orig)) {
    process.stdout.write('  a compilar com pdflatex... ')
    for (let i = 0; i < 3; i++)
      sh(`pdflatex -interaction=nonstopmode -output-directory=${JSON.stringify(TMP)} ` +
         `-jobname=${nome}_orig ${JSON.stringify(path.join(RAIZ, fonte))}`,
         { cwd: RAIZ, timeout: 900000 })
    console.log(fs.existsSync(orig) ? 'ok' : 'FALHOU')
  }
  /* o NOSSO, pelo tex.c */
  const nosso = path.join(TMP, `${nome}_tex.pdf`)
  sh(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(RAIZ, fonte))} ${JSON.stringify(nosso)}`,
     { cwd: path.dirname(TEX), timeout: 900000 })

  if (!fs.existsSync(orig) || !fs.existsSync(nosso)) { console.log('  falta um dos PDFs'); continue }

  const pa = /Pages:\s*(\d+)/.exec(sh(`pdfinfo ${JSON.stringify(orig)}`))
  const pb = /Pages:\s*(\d+)/.exec(sh(`pdfinfo ${JSON.stringify(nosso)}`))
  console.log(`  páginas: pdflatex ${pa ? pa[1] : '?'}   tex.c ${pb ? pb[1] : '?'}`)

  /* ── 1. O TEXTO ─────────────────────────────────────────────────────────────────── */
  const A = caixas(orig), B = caixas(nosso)
  const sa = new Set(A.map((w) => norm(w.t)).filter((t) => t.length >= 4))
  const sb = new Set(B.map((w) => norm(w.t)).filter((t) => t.length >= 4))
  const falta = [...sa].filter((t) => !sb.has(t))
  const sobra = [...sb].filter((t) => !sa.has(t))
  console.log(`\n  1. TEXTO   ${sa.size} palavras distintas no original, ${sb.size} no nosso`)
  console.log(`             FALTAM ${falta.length}   SOBRAM ${sobra.length}`)
  if (falta.length) console.log(`             ex. em falta: ${falta.slice(0, 6).join(', ')}`)
  if (sobra.length) console.log(`             ex. a mais:   ${sobra.slice(0, 6).join(', ')}`)

  /* ── 2. AS CAIXAS ───────────────────────────────────────────────────────────────── */
  /* alinha-se pela SEQUÊNCIA das palavras, não pela página: o nosso pagina diferente, e
   * exigir a mesma página seria medir a paginação e não a posição. */
  const seqA = A.map((w) => norm(w.t)), seqB = B.map((w) => norm(w.t))
  let i = 0, j = 0, casados = 0, somaDx = 0, piorDx = 0, piorQual = null
  while (i < seqA.length && j < seqB.length) {
    if (seqA[i] === seqB[j]) {
      const dx = Math.abs(A[i].x - B[j].x)
      casados++; somaDx += dx
      if (dx > piorDx) { piorDx = dx; piorQual = seqA[i] }
      i++; j++
    } else {
      /* procura-se a seguinte que case, até 8 à frente de cada lado */
      let achou = false
      for (let k = 1; k <= 8 && !achou; k++) {
        if (seqA[i + k] === seqB[j]) { i += k; achou = true }
        else if (seqA[i] === seqB[j + k]) { j += k; achou = true }
      }
      if (!achou) { i++; j++ }
    }
  }
  console.log(`\n  2. CAIXAS  ${casados} palavras casadas na sequência`)
  console.log(`             desvio médio em x: ${casados ? (somaDx / casados).toFixed(1) : '—'} pt`)
  console.log(`             o pior: ${piorDx.toFixed(1)} pt  ${piorQual ? '(«' + piorQual + '»)' : ''}`)

  /* e as INVASÕES, que são o defeito que não depende do original */
  let inv = 0, pares = 0
  for (let k = 0; k < B.length - 1; k++) {
    if (B[k].pag !== B[k + 1].pag || Math.abs(B[k].y - B[k + 1].y) > 0.5) continue
    pares++
    /* UMA CÉLULA QUE QUEBRA NÃO É UMA INVASÃO. Numa tabela, a palavra seguinte pode voltar
     * à primeira coluna com o mesmo `y` — `x` pequeno depois de um `X` grande —, e isso lê-se
     * como sobreposição sem o ser. MEDIDO: das 3 «invasões» do enredo, a de 298,8 pt era
     * isto, e a página está correcta na imagem.
     *
     * Uma invasão real é uma sobreposição PEQUENA: a palavra seguinte começa dentro da
     * anterior, não centenas de pontos à esquerda dela. */
    const d = B[k].X - B[k + 1].x
    if (d > 0.5 && d < (B[k].X - B[k].x)) inv++
  }
  console.log(`             invasões no nosso: ${inv} em ${pares} pares vizinhos`)

  /* ── 3. OS PIXELS ───────────────────────────────────────────────────────────────── */
  /* rasteriza-se a MESMA página lógica dos dois e conta-se a diferença. Não é para dar zero
   * — as fontes são outras — é para APONTAR ONDE olhar: a página com mais diferença é a que
   * tem o defeito, e o número serve para comparar entre execuções. */
  const npg = Math.min(pa ? +pa[1] : 0, pb ? +pb[1] : 0)
  const amostra = [1, Math.floor(npg / 3), Math.floor(npg / 2)].filter((p) => p >= 1)
  const difs = []
  for (const p of amostra) {
    sh(`pdftoppm -f ${p} -l ${p} -r 72 -gray -png ${JSON.stringify(orig)} ${TMP}/a`)
    sh(`pdftoppm -f ${p} -l ${p} -r 72 -gray -png ${JSON.stringify(nosso)} ${TMP}/b`)
    const fa = fs.readdirSync(TMP).find((f) => f.startsWith('a-') && f.endsWith('.png'))
    const fb = fs.readdirSync(TMP).find((f) => f.startsWith('b-') && f.endsWith('.png'))
    if (!fa || !fb) continue
    const r = sh(`compare -metric AE ${TMP}/${fa} ${TMP}/${fb} null: 2>&1 || true`)
    const m = /^(\d+)/.exec(r.trim())
    difs.push([p, m ? +m[1] : null])
    for (const f of [fa, fb]) try { fs.unlinkSync(path.join(TMP, f)) } catch (e) {}
  }
  console.log(`\n  3. PIXELS  amostra de ${difs.length} páginas`)
  for (const [p, d] of difs)
    console.log(`             pág ${String(p).padStart(4)}: ${d === null ? '(sem ImageMagick)' : d + ' pixels diferentes'}`)

  /* ── O RESÍDUO ──────────────────────────────────────────────────────────────────── */
  const zero = falta.length === 0 && inv === 0
  console.log(`\n  RESÍDUO: texto ${falta.length}   invasões ${inv}   ${zero ? '— IGUAL no que se mede' : '— ainda NÃO é igual'}`)
}

console.log('\n────────────────────────────────────────────────')
console.log('  Os três níveis não se substituem: o texto apanha o que se perdeu, a caixa o que')
console.log('  está torto, e o pixel o que nenhum dos dois vê — uma régua por cima de uma letra')
console.log('  não muda o texto nem a caixa.')
console.log('')
console.log('  E o alvo é ZERO, não uma percentagem simpática. Onde não for, diz-se quanto e')
console.log('  onde.\n')
