/* tests/cristal_volta.js — a volta do cristal recuperado (eval 13/08).
 *
 * Regra de ouro: não converter o corpus antes de conseguir reconstruí-lo.
 * A fonte é cristal/cristal.jsonl (4286 conceitos, última versão, broca-so).
 * As projeções papers/cristal_*.tex carregam cada registo em %CRISTAL —
 * o desenho do /Type/FonteTeX: a página é a leitura, a fonte viaja invisível.
 *
 * §V0  fonte existe e cada linha é JSON com id único
 * §V1  reconstrução: %CRISTAL de todos os .tex == fonte, byte a byte (R=0)
 * §V2  dois caminhos: nº de \section == nº de %CRISTAL, por ficheiro e total
 * §V3  mutação: um byte trocado num registo embutido acusa (R>0)
 * §V4  portão: nenhum .tex nem a fonte carregam o IP privado
 *
 *   node tests/cristal_volta.js
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
const FONTE = path.join(RAIZ, 'cristal', 'cristal.jsonl')
const PAPERS = path.join(RAIZ, 'papers')

/* §V0 — a fonte */
const fonteLinhas = fs.readFileSync(FONTE, 'utf8').split('\n').filter(l => l.length)
ok('§V0 fonte com 4286 conceitos', fonteLinhas.length === 4286)
const fonteIds = new Set()
let jsonOk = true
for (const l of fonteLinhas) {
  try { fonteIds.add(JSON.parse(l).id) } catch { jsonOk = false }
}
ok('§V0 cada linha é JSON', jsonOk)
ok('§V0 ids únicos', fonteIds.size === fonteLinhas.length)

/* §V1/§V2 — reconstrução a partir das projeções */
const texs = fs.readdirSync(PAPERS).filter(f => /^cristal_.*\.tex$/.test(f)).sort()
ok('§V2 dez projeções no disco', texs.length === 10)

const reconstruido = []
let seccoesTotal = 0, casaOk = true
for (const f of texs) {
  const corpo = fs.readFileSync(path.join(PAPERS, f), 'utf8')
  const regs = []
  let seccoes = 0
  for (const linha of corpo.split('\n')) {
    if (linha.startsWith('%CRISTAL ')) regs.push(linha.slice(9))
    else if (linha.startsWith('\\section{')) seccoes++
  }
  if (regs.length !== seccoes) casaOk = false
  reconstruido.push(...regs)
  seccoesTotal += seccoes
}
ok('§V2 secções == registos, ficheiro a ficheiro', casaOk)
ok('§V2 total de secções == 4286', seccoesTotal === 4286)

function idDe (linha, i) {
  try { return JSON.parse(linha).id } catch { return '￿ corrompido ' + i }
}

function residuo (regs) {
  const orden = [...regs].sort((a, b) => {
    const ia = idDe(a, 0), ib = idDe(b, 1)
    return ia < ib ? -1 : ia > ib ? 1 : 0
  })
  if (orden.length !== fonteLinhas.length) {
    return Math.abs(orden.length - fonteLinhas.length) || 1
  }
  let r = 0
  for (let i = 0; i < orden.length; i++) if (orden[i] !== fonteLinhas[i]) r++
  return r
}

const R = residuo(reconstruido)
ok('§V1 reconstrução byte a byte, R=0', R === 0)

/* §V3 — mutações (a ordem do gerente, 13/08): o sistema não só volta quando
 * está íntegro — DENUNCIA quando é alterado. Cada mutação corre a volta e
 * tem de dar R≠0 → REOPEN, à vista. */
function muta (nome, transforma, esperaReopen) {
  if (reconstruido.length === 0) { ok('§V3 ' + nome, false); return }
  const R = residuo(transforma([...reconstruido]))
  if (esperaReopen) {
    console.log(`#MUT ${nome}: R=${R} -> ${R > 0 ? 'REOPEN' : 'passou (DEFEITO)'}`)
    ok('§V3 ' + nome + ' acusa (R>0 -> REOPEN)', R > 0)
  } else {
    console.log(`#MUT ${nome}: R=${R} -> ${R === 0 ? 'SURVIVED (por desenho)' : 'REOPEN'}`)
    ok('§V3 ' + nome + ' sobrevive por desenho (R=0)', R === 0)
  }
}

const meio = () => Math.floor(reconstruido.length / 2)

muta('apagar um conceito', m => { m.splice(meio(), 1); return m }, true)

muta('trocar uma resposta (1 byte na descricao)', m => {
  const alvo = m[meio()]
  const i = alvo.indexOf('"descricao":"') + 13
  m[meio()] = alvo.slice(0, i) +
    String.fromCharCode(alvo.charCodeAt(i) ^ 1) + alvo.slice(i + 1)
  return m
}, true)

muta('alterar uma categoria (meta.dominio)', m => {
  for (let i = 0; i < m.length; i++) {
    const t = m[i].replace(/"dominio":"[^"]*"/, '"dominio":"mutado"')
    if (t !== m[i]) { m[i] = t; break }
  }
  return m
}, true)

muta('corromper uma projeção (registo ilegível)', m => {
  m[meio()] = m[meio()].slice(0, 20) + '<<<corrompido>>>'
  return m
}, true)

muta('alterar a ordenação dos registos', m => m.reverse(), false)
/* a ordem canónica é DERIVADA (sort por id) — a ordenação não carrega
 * informação, e por isso esta mutação é equivalente (o precedente da casa:
 * «trocar empatados é mutação equivalente», pipeline.tex §estação). O
 * metadado, esse, acusa — é a mutação da categoria acima. */

/* §V4 — portão do privado (tools/corpo.sh: «IP privado — não publicar») */
{
  let limpo = !fs.readFileSync(FONTE, 'utf8').includes('78.46.19.151')
  for (const f of texs) {
    if (fs.readFileSync(path.join(PAPERS, f), 'utf8').includes('78.46.19.151')) limpo = false
  }
  ok('§V4 IP privado ausente da fonte e das projeções', limpo)
}

console.log('')
if (!falhas) {
  console.log('  O cristal voltou: 4286 conceitos, fonte == projeções, mutação acusa.')
  console.log('  LaTeX = projeção verificável; a fonte é cristal/cristal.jsonl.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
