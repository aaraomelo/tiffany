/* tests/pdf_casa_texto.js — O LEITOR DO DIALECTO DA CASA (auditoria 14/08).
 *
 * O compositor deixou de emitir texto Tj: cada glifo é um Form XObject
 * desenhado por «q a 0 0 a x y cm /Gf_c Do Q», onde f é o índice da fonte
 * e c É O CÓDIGO DO CARÁCTER (WinAnsi). O pdftotext do sistema lê zero —
 * e os medidores de texto liam a camada errada. Este leitor lê a certa.
 *
 * Limitação DECLARADA: não há glifos de espaço — as palavras separam-se
 * por lacunas geométricas. Quem precisa de «palavras» usa sequências de
 * letras (os medidores dizem-no no rótulo); quem precisa de posições usa
 * os glifos crus (x, y, s, f, c). O tamanho do corpo é s·1000 (upem).
 *
 * Uma leitura, muitos usos — a lição do spline.h.
 */
'use strict'

const WINANSI = {
  128: '€', 130: '‚', 131: 'ƒ', 132: '„', 133: '…', 134: '†', 135: '‡',
  136: 'ˆ', 137: '‰', 138: 'Š', 139: '‹', 140: 'Œ', 145: '‘',
  146: '’', 147: '“', 148: '”', 149: '•', 150: '–', 151: '—',
  152: '˜', 153: '™', 154: 'š', 155: '›', 156: 'œ', 159: 'Ÿ',
}

function objetos (pdf) {
  const m = new Map()
  const re = /(\d+) 0 obj/g
  let x
  while ((x = re.exec(pdf))) {
    const fim = pdf.indexOf('endobj', x.index)
    m.set(Number(x[1]), pdf.slice(x.index, fim < 0 ? pdf.length : fim))
  }
  return m
}

function streamDe (corpo) {
  const i = corpo.indexOf('stream')
  if (i < 0) return ''
  let j = i + 6
  if (corpo[j] === '\r') j++
  if (corpo[j] === '\n') j++
  const f = corpo.indexOf('endstream', j)
  return corpo.slice(j, f < 0 ? corpo.length : f)
}

/* as páginas na ordem do ficheiro (o compositor emite-as em ordem) */
function paginas (pdf, objs) {
  objs = objs || objetos(pdf)
  const ps = []
  for (const [n, corpo] of objs) {
    const cab = corpo.slice(0, Math.max(corpo.indexOf('stream'), 0) || corpo.length)
    if (!/\/Type\s*\/Page(?!s)/.test(cab)) continue
    let nums = []
    const arr = cab.match(/\/Contents\s*\[([^\]]*)\]/)
    if (arr) nums = [...arr[1].matchAll(/(\d+) 0 R/g)].map(m => Number(m[1]))
    else {
      const um = cab.match(/\/Contents\s+(\d+) 0 R/)
      if (um) nums = [Number(um[1])]
    }
    ps.push({ obj: n, nums })
  }
  return ps
}

/* os glifos de um stream: {s, x, y, f, c} — s·1000 é o corpo em pontos */
function glifos (stream) {
  const g = []
  const re = /([\d.]+) 0 0 ([\d.]+) ([\-\d.]+) ([\-\d.]+) cm\s*\/G(\d+)_(\d+) Do/g
  let m
  while ((m = re.exec(stream))) {
    g.push({ s: +m[1], x: +m[3], y: +m[4], f: +m[5], c: +m[6] })
  }
  return g
}

function charDe (c) {
  if (WINANSI[c]) return WINANSI[c]
  return String.fromCharCode(c)          /* ASCII e latin1 (à-ÿ inclusive) */
}

/* linhas por y (topo primeiro), glifos por x — SEM inventar espaços */
function textoDeGlifos (gs) {
  const porY = new Map()
  for (const g of gs) {
    const k = g.y.toFixed(2)
    if (!porY.has(k)) porY.set(k, [])
    porY.get(k).push(g)
  }
  const linhas = [...porY.entries()].sort((a, b) => +b[0] - +a[0])
  return linhas.map(([, l]) =>
    l.sort((a, b) => a.x - b.x).map(g => charDe(g.c)).join('')).join('\n')
}

function glifosPagina (pdf, n, ctx) {
  const objs = (ctx && ctx.objs) || objetos(pdf)
  const ps = (ctx && ctx.ps) || paginas(pdf, objs)
  const p = ps[n - 1]
  if (!p) return []
  let g = []
  for (const num of p.nums) {
    const o = objs.get(num)
    if (o) g = g.concat(glifos(streamDe(o)))
  }
  return g
}

function textoPagina (pdf, n, ctx) { return textoDeGlifos(glifosPagina(pdf, n, ctx)) }

function texto (pdf) {
  const objs = objetos(pdf)
  const ps = paginas(pdf, objs)
  const partes = []
  for (let i = 1; i <= ps.length; i++) partes.push(textoPagina(pdf, i, { objs, ps }))
  return partes.join('\n')
}

/* sequências de letras (o proxy honesto de «palavra» num dialecto sem espaços) */
function letrasLongas (txt, n) {
  return new Set(txt.match(new RegExp(`[A-Za-zÀ-ÿ]{${n || 8},}`, 'g')) || [])
}

module.exports = {
  objetos, streamDe, paginas, glifos, glifosPagina, textoDeGlifos,
  textoPagina, texto, letrasLongas, charDe,
}
