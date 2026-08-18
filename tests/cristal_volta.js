/* tests/cristal_volta.js — a volta do cristal recuperado, medidor v2 (eval 13/08).
 *
 * Regra de ouro: não converter o corpus antes de conseguir reconstruí-lo.
 * A fonte é cristal/cristal.jsonl (4234 + lotes: 4286 recuperados do
 * broca-so menos 52 fusões de curadoria — tools/cristal_cura.py; cada fusão
 * guarda as duas partes byte a byte, medidor tests/cristal_curadoria.js).
 * As projeções cristal/cristal_*.tex carregam cada registo em %CRISTAL —
 * o desenho do /Type/FonteTeX: a página é a leitura, a fonte viaja invisível.
 *
 * v2 (Teorema da Absorção, corpo_topologico.tex thm:absorcao): o resíduo lê-se
 * pelo ENDEREÇO (o id), não pela posição — R(a) = Back(a) − a. A posição é
 * derivada e não carrega informação; a régua v1 (por posição) fica só como
 * contraste na matriz de indução, para o teorema aparecer na tela.
 *
 * §V0  fonte existe e cada linha é JSON com id único
 * §V1  reconstrução pelo endereço == fonte, byte a byte por id (R=0)
 * §V2  dois caminhos: nº de \section == nº de %CRISTAL, por ficheiro e total
 * §V3  MATRIZ DE INDUÇÃO — v1 (posição) × v2 (endereço), na mesma cópia
 * §V4  portão: nenhum .tex nem a fonte carregam o IP privado
 *
 *   node tests/cristal_volta.js
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

/* §V0 — a fonte */
const fonteLinhas = fs.readFileSync(FONTE, 'utf8').split('\n').filter(l => l.length)
/* o pino MORREU quando o avanço virou fluxo (lote b, 14/08): a contagem
 * deriva dos lotes commitados em cristal/avanco_*.jsonl — a curadoria
 * vive neles, e o recuperado continua guardado pela âncora do jornal
 * ((c−t)+f == 4286, cristal_avanco §A1) */
const nascidos = fs.readdirSync(path.join(RAIZ, 'cristal'))
  .filter(f => /^avanco_.*\.jsonl$/.test(f))
  .reduce((n2, f) => n2 + fs.readFileSync(path.join(RAIZ, 'cristal', f), 'utf8').split('\n').filter(Boolean).length, 0)
ok('§V0 fonte com 4234 + nascidos conceitos (a contagem deriva dos lotes de avanço)',
  fonteLinhas.length === 4234 + nascidos)
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
ok('§V2 total de secções == fonte (as projeções acompanham os lotes)', seccoesTotal === fonteLinhas.length)

const idDe = (linha, i) => sigmaPeano.endereco(linha, i)

/* v1 — por POSIÇÃO na ordem derivada (o contraste; dissipa ~n² quando o
 * endereço morre — thm:absorcao). Fica só para a matriz. */
function residuoV1 (regs) {
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

/* v2 — pelo ENDEREÇO: R(a) = Back(a) − a. Conta faltantes, alterados,
 * excedentes E duplicados (cada cópia a mais carrega o nó). A ordem física
 * não participa: é derivada do endereço. */
const fontePorId = new Map()
for (const l of fonteLinhas) fontePorId.set(idDe(l, 0), l)

function residuoV2 (regs) { return U.rEndereco(fonteLinhas, regs) }

ok('§V1 reconstrução pelo endereço, R=0', residuoV2(reconstruido) === 0)
ok('§V1 dois caminhos: a régua de posição concorda no íntegro',
  residuoV1(reconstruido) === 0)

/* §V3 — MATRIZ DE INDUÇÃO (a ordem do diretor, 13/08): as duas réguas
 * sobre a MESMA cópia induzida; o v2 (endereço) decide, o v1 (posição) fica
 * de contraste — o teorema tem de aparecer na tela: R(a) = Back(a) − a. */
console.log('')
console.log('=== MATRIZ DE INDUÇÃO — v1 (posição) × v2 (endereço) ===')

function induz (nome, transforma, esperaR) {
  if (reconstruido.length === 0) { ok('§V3 ' + nome, false); return }
  const copia = transforma([...reconstruido])
  const R1 = residuoV1(copia)
  const R2 = residuoV2(copia)
  const veredito = esperaR === 0
    ? (R2 === 0 ? 'ABSORVIDA (R=0)' : 'REOPEN (DEFEITO: devia absorver)')
    : (R2 > 0 ? 'REOPEN' : 'passou (DEFEITO)')
  console.log(`#IND ${nome.padEnd(44)} v1 R=${String(R1).padStart(4)}   v2 R=${String(R2).padStart(2)} -> ${veredito}`)
  ok('§V3 ' + nome + (esperaR === 0 ? ' absorvida (R=0)' : ' acusa (R>0)'),
    esperaR === 0 ? R2 === 0 : R2 > 0)
}

const meio = () => Math.floor(reconstruido.length / 2)

induz('permutação física (endereços intactos)', m => m.reverse(), 0)
/* a ordem é derivada do endereço e não carrega informação — indução
 * equivalente (o precedente: «trocar empatados é mutação equivalente»,
 * pipeline.tex §estação) */

induz('conteúdo no endereço (1 byte na descricao)', m => {
  const alvo = m[meio()]
  const i = alvo.indexOf('"descricao":"') + 13
  m[meio()] = alvo.slice(0, i) +
    String.fromCharCode(alvo.charCodeAt(i) ^ 1) + alvo.slice(i + 1)
  return m
}, 1)

induz('categoria no endereço (meta.dominio)', m => {
  for (let i = 0; i < m.length; i++) {
    const t = m[i].replace(/"dominio":"[^"]*"/, '"dominio":"induzido"')
    if (t !== m[i]) { m[i] = t; break }
  }
  return m
}, 1)

induz('remoção de endereço (o par colapsa)', m => {
  m.splice(meio(), 1)
  return m
}, 1)

induz('duplicação de endereço (excesso no nó)', m => {
  m.push(m[meio()])
  return m
}, 1)

induz('endereço destruído (registo ilegível)', m => {
  m[meio()] = m[meio()].slice(0, 20) + '<<<corrompido>>>'
  return m
}, 1)
/* aqui o teorema aparece: v1 dissipa ~n/2 (o ilegível perde o lugar e empurra
 * a lista); v2 cobra o PAR exato — faltante ⊕ excedente, a Lei 0 */

/* mudança admissível: retocar a PROJEÇÃO visível sem tocar o registo — a
 * página é a leitura, a fonte viaja no %CRISTAL; a régua absorve */
{
  const corpo = fs.readFileSync(path.join(PAPERS, texs[0]), 'utf8')
    .replace('\\maketitle', '\\maketitle\n\nRetoque visível da projeção --- texto novo, registo intacto.\n')
  const regs = []
  for (const linha of corpo.split('\n')) {
    if (linha.startsWith('%CRISTAL ')) regs.push(linha.slice(9))
  }
  for (let f = 1; f < texs.length; f++) {
    for (const linha of fs.readFileSync(path.join(PAPERS, texs[f]), 'utf8').split('\n')) {
      if (linha.startsWith('%CRISTAL ')) regs.push(linha.slice(9))
    }
  }
  const R1 = residuoV1(regs), R2 = residuoV2(regs)
  console.log(`#IND ${'mudança admissível (retoque da projeção)'.padEnd(44)} v1 R=${String(R1).padStart(4)}   v2 R=${String(R2).padStart(2)} -> ${R2 === 0 ? 'ABSORVIDA (R=0)' : 'REOPEN (DEFEITO)'}`)
  ok('§V3 mudança admissível absorvida (R=0)', R2 === 0 && R1 === 0)
}

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
  console.log(`  O cristal voltou: ${fonteLinhas.length} conceitos (52 fusões + os lotes), fonte ==`)
  console.log('  projeções, a indução acusa.')
  console.log('  LaTeX = projeção verificável; a fonte é cristal/cristal.jsonl.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
