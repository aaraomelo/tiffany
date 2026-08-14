/* avalia_macros.js — A AVALIAÇÃO NAS RAÍZES, E QUE ELA É O QUE TRAZ A CAPA.
 *
 * O Aarão: «em corpo estelar tem o código do universal, isso deve servir para fazer um
 * tradutor latex→pdf».
 *
 * E serve. O `universal.c` diz o que é a transformada universal: **a que leva CONVOLUÇÃO
 * em PRODUTO, e quem faz isso é a AVALIAÇÃO NAS RAÍZES** — avaliar num zero é um
 * homomorfismo de anéis, e é por ser homomorfismo que não precisa de saber o que traduz.
 *
 * `\gkcapa{A}{B}{C}` é isso: o comando é o polinómio, os argumentos são o ponto, traduzir
 * é AVALIAR. E a definição já existe, escrita pelo autor:
 *
 *     \providecommand{\gkcapa}[3]{ \title{... #1 ... #2 ... #3 ...} }
 *
 * O tradutor fazia o contrário — reconhecer por nome, um `strcmp` por comando. Este ficheiro
 * mede as duas coisas que isso custava, e a segunda é a que interessa:
 *
 *   §M1  quantas macros a fonte define, e que a avaliação não precisa de as conhecer
 *   §M2  A CAPA SAI — e sai com as palavras do estilo.tex, não com as deste ficheiro
 *   §M3  os degraus da dourada atravessam: o tamanho no PDF é o que o estilo manda
 *   §M4  MUTAÇÃO — sem a avaliação, a capa desaparece
 *   §M5  A NUMERAÇÃO: a sequência dos capítulos é a do pdflatex, reposições incluídas
 *
 * §M4 é o que dá valor aos outros: sem ele, §M2 podia passar por a capa vir de outro sítio.
 *
 *   node tests/avalia_macros.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TEX = path.join(__dirname, 'tex')
const TMP = '/tmp/avalia_macros'

let feitas = 0, falhas = 0
function ok (q, cond) {
  feitas++; if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
  console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`)
}
const sh = (c, o) => { try { return execSync(c, Object.assign({ encoding: 'utf8', stdio: 'pipe' }, o || {})) } catch (e) { return String((e.stdout || '') + (e.stderr || '')) } }

fs.mkdirSync(TMP, { recursive: true })
const estilo = fs.readFileSync(path.join(RAIZ, 'estilo.tex'), 'utf8')
const fonteC = fs.readFileSync(path.join(__dirname, 'tex.c'), 'utf8')

console.log('=== A AVALIAÇÃO NAS RAÍZES: expandir, não reconhecer ======================\n')

/* ─── §M1 quantas a fonte define, e quantas o tradutor nomeia ──────────────────────── */
const defs = [...estilo.matchAll(/\\(?:new|provide|renew)command\{\\([A-Za-z]+)\}/g)].map((m) => m[1])
const distintas = [...new Set(defs)]
const nomeadas = distintas.filter((m) => fonteC.includes(`"${m}"`))
const usos = ['enredo.tex', 'teoria.tex', 'catalogo.tex']
  .map((f) => (fs.readFileSync(path.join(RAIZ, f), 'utf8').match(
    new RegExp('\\\\(?:' + distintas.join('|') + ')(?![A-Za-z])', 'g')) || []).length)
  .reduce((a, b) => a + b, 0)

console.log(`§M1  ${distintas.length} macros definidas no estilo.tex`)
console.log(`     ${nomeadas.length} têm o nome escrito no tex.c`)
console.log(`     ${usos} usos nos três documentos`)
/* a asserção não é «são muitas» — é que a MAIORIA não é nomeada, que é exactamente o que
 * torna a avaliação necessária. Se alguém as nomeasse todas à mão, isto falharia — e devia,
 * porque seria a estrutura errada a voltar. */
ok('a maioria das macros da fonte NÃO é nomeada pelo tradutor', nomeadas.length * 2 < distintas.length)
ok('e elas são de facto usadas — mais de mil vezes nos três documentos', usos > 1000)

/* ─── §M2 a capa sai, com as palavras do estilo ────────────────────────────────────── */
const pdf = path.join(TMP, 'enredo.pdf')
sh(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(RAIZ, 'enredo.tex'))} ${JSON.stringify(pdf)}`,
   { cwd: __dirname, timeout: 900000 })
/* auditoria 14/08: o dialecto desenha glifos — pdftotext lê zero. O leitor
 * é o da casa; espaços são lacunas, logo as comparações tiram os espaços. */
const casa = require('./pdf_casa_texto.js')
const p1 = casa.textoPagina(fs.readFileSync(pdf, 'latin1'), 1).replace(/\s+/g, '')

/* as palavras não se escrevem aqui: LÊEM-SE do corpo do \gkcapa no estilo.tex, senão eram
 * uma referência escrita à mão e mudar o estilo não mudaria este medidor */
/* o corpo é até à chaveta que FECHA — uma janela de tamanho fixo apanhava o comando
 * seguinte, e o medidor exigia palavras que nunca foram da capa */
function corpo_de (nome) {
  const i = estilo.indexOf(`\\providecommand{\\${nome}}`)
  if (i < 0) return ''
  let d = 1, k = estilo.indexOf('{', i + nome.length + 17) + 1
  const a = k
  while (k < estilo.length && d) { if (estilo[k] === '{') d++; else if (estilo[k] === '}') d--; k++ }
  return estilo.slice(a, k - 1)
}
const corpo = corpo_de('gkcapa')
const doEstilo = [...corpo.matchAll(/\b([A-ZÀ-Ú][a-zà-ú]{3,})\b/g)].map((m) => m[1])
const naCapa = [...new Set(doEstilo)].filter((w) => p1.includes(w))
console.log(`\n§M2  palavras próprias do \\gkcapa no estilo: ${[...new Set(doEstilo)].join(', ')}`)
console.log(`     na página 1 do PDF: ${naCapa.join(', ') || '(nenhuma)'}`)
ok('as palavras que só existem no corpo do \\gkcapa aparecem na página 1',
   naCapa.length >= 2 && naCapa.length === [...new Set(doEstilo)].length)

/* e os ARGUMENTOS — o título, que vive no enredo.tex e não no estilo */
const arg = /\\gkcapa\{([^}]*)\}/.exec(fs.readFileSync(path.join(RAIZ, 'enredo.tex'), 'utf8'))
const tit = arg ? arg[1].replace(/---/g, '—') : null
console.log(`     o 1.º argumento, lido do enredo.tex: «${tit}»`)
ok('o argumento do \\gkcapa sai na página 1 — a avaliação substitui #1',
   !!tit && p1.includes(tit.replace(/\s+/g, '')))

/* ─── §M3 os degraus da dourada atravessam ─────────────────────────────────────────── */
/* o estilo declara os tamanhos; o PDF tem de os usar. Compara-se o CONJUNTO, não um valor. */
const degraus = [...new Set([...estilo.matchAll(/\\fontsize\{([\d.]+)\}/g)].map((m) => +m[1]))]
  .sort((a, b) => a - b)
/* os streams vêm comprimidos: descomprimem-se aqui, sem depender de o qpdf existir —
 * com ele em falta a lista saía VAZIA e a asserção `every` passava sobre nada */
const zlib = require('zlib')
const cru = fs.readFileSync(pdf)
let bruto = ''
for (const m of cru.toString('latin1').matchAll(/stream\r?\n/g)) {
  const a = m.index + m[0].length
  const b = cru.toString('latin1').indexOf('endstream', a)
  if (b < 0) break
  /* e quando NÃO comprime — que é o caso deste tradutor — o stream lê-se cru. Sem isto o
   * inflate falhava em todos, `bruto` ficava vazio, e a asserção passava sobre nada. */
  try { bruto += zlib.inflateSync(cru.subarray(a, b)).toString('latin1') }
  catch (e) { bruto += cru.subarray(a, b).toString('latin1') }
  if (bruto.length > 2e6) break
}
/* auditoria 14/08: sem Tf no dialecto — o corpo é a ESCALA do cm × upem
 * (s·1000), exata a 2 casas como o \\fontsize declara */
const noPdf = [...new Set([...bruto.matchAll(/([\d.]+) 0 0 [\d.]+ [\-\d.]+ [\-\d.]+ cm\s*\/G/g)]
  .map((m) => Math.round(+m[1] * 1000 * 100) / 100))]
  .sort((a, b) => a - b)
/* RESÍDUO ZERO, não «perto»: o `Tf` do PDF aceita fracções, logo o corpo pode ser o degrau
 * EXACTO. Uma tolerância aqui deixaria passar o arredondamento a inteiro que havia — e era
 * ele o defeito: as razões passavam de 1,1740 constante para 1,1111 … 1,2143. */
/* O CORPO do texto NÃO é um degrau do estilo: é o `\normalsize` da CLASSE (tex_core.c:929, e
 * compor a 10,5 era o bug). Lê-se da classe, não se afirma --- como o §M5 lê os nomes do babel:
 * livro.tex dá o tamanho (11pt), size{N}.clo diz que `\normalsize` usa `\@Nipt`, latex.ltx dá o
 * número (10,95). Sem isto, o §M4 exigia que o corpo fosse um degrau, e a asserção contradizia o
 * desenho: media-se um só corpo fora da escala, e era exactamente o `\normalsize` da classe. */
const escapeRe = (s) => s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')
function corpo_da_classe () {
  try {
    const mestre = fs.readFileSync(path.join(RAIZ, 'livro.tex'), 'utf8')
    const sz = (/(\d+)pt/.exec((/\\documentclass\[([^\]]*)\]/.exec(mestre) || [])[1] || '') || [])[1]
    if (!sz) return null
    const clo = sh(`kpsewhich size${sz}.clo`).trim()
    if (!clo || !fs.existsSync(clo)) return null
    const cmd = (/\\@setfontsize\s*\\normalsize\s*(\\@[a-z]+)/.exec(fs.readFileSync(clo, 'latin1')) || [])[1]
    if (!cmd) return null
    const ltx = sh('kpsewhich latex.ltx').trim()
    const m = new RegExp(escapeRe('\\def' + cmd) + '\\s*\\{([\\d.]+)\\}').exec(fs.readFileSync(ltx, 'latin1'))
    return m ? +m[1] : null
  } catch (e) { return null }
}
const corpoClasse = corpo_da_classe()
/* um corpo é EXACTO se é um degrau do estilo (os títulos) OU o `\normalsize` da classe (o texto) */
const perto = (v) => degraus.some((d) => Math.abs(d - v) < 1e-9) ||
                     (corpoClasse != null && Math.abs(corpoClasse - v) < 1e-9)
console.log(`\n§M3  degraus no estilo.tex: ${degraus.join(' ')}`)
console.log(`     corpo da classe (\\normalsize): ${corpoClasse}`)
console.log(`     corpos usados no PDF:  ${noPdf.join(' ')}`)
/* `every` sobre lista vazia é verdadeiro: sem o `length > 0` isto passava sem medir nada. E exige-se
 * que o corpo da classe se LEIA (corpoClasse != null), senão a asserção passaria sem a régua. */
ok('todo corpo usado no PDF é um degrau do estilo, ou o \\normalsize da classe — resíduo 0',
   noPdf.length > 0 && corpoClasse != null && noPdf.every(perto))
ok('e usa-se MAIS DE UM — a hierarquia atravessou, não é tudo o corpo do texto',
   noPdf.length >= 3)

/* ─── §M5 A NUMERAÇÃO: contra o pdflatex, não contra uma regra minha ───────────────── */
/* Os números derivam-se — contadores com reposição. As PALAVRAS não: «Capítulo», «Parte»
 * são do idioma, e escrevê-las aqui era a referência escrita à mão. Lêem-se do `.ldf` do
 * babel instalado, que é de onde o pdflatex as tira. */
const opt = /\\usepackage\[([^\]]*)\]\{babel\}/.exec(estilo)
const idioma = opt ? opt[1].split(',')[0].trim() : null
const ldf = idioma ? sh(`kpsewhich ${idioma}.ldf`).trim() : ''
const nomes = {}
if (ldf && fs.existsSync(ldf)) {
  const d = fs.readFileSync(ldf, 'latin1')
  for (const chave of ['chaptername', 'partname']) {
    const m = new RegExp(chave + '\\{((?:[^{}]|\\{[^{}]*\\})*)\\}').exec(d)
    if (m) nomes[chave] = m[1].replace(/\\'\{?\\?([aeiouAEIOU]|i)\}?/g,
      (x, v) => ({ a: 'á', e: 'é', i: 'í', o: 'ó', u: 'ú', A: 'Á', E: 'É', I: 'Í', O: 'Ó', U: 'Ú' })[v] || v)
      .replace(/[\\{}]/g, '')
  }
}
console.log(`\n§M5  babel: [${idioma}] → ${ldf || '(não encontrado)'}`)
console.log(`     nomes lidos do .ldf: ${JSON.stringify(nomes)}`)
ok('o nome do capítulo vem do babel instalado, não deste ficheiro',
   !!nomes.chaptername && nomes.chaptername.length > 3)

/* a SEQUÊNCIA dos capítulos, dos dois lados — inclusive as REPOSIÇÕES, que não são regra
 * nenhuma: o enredo.tex tem um `\setcounter{chapter}{0}` escrito à mão a meio, e sem essa
 * primitiva a minha numeração ia 1..148 onde o original vai 1..54 e depois 1..94 */
const seqDe = (f) => (sh(`pdftotext ${JSON.stringify(f)} -`).match(/[Cc]ap[íi]tulo\s+(\d+)/g) || [])
  .map((t) => +t.replace(/\D+/g, ''))
/* o nosso PDF lê-se pela casa (o pdftotext dá vazio no dialecto) */
const seqCasa = (f) => (casa.texto(fs.readFileSync(f, 'latin1')).match(/[Cc]ap[íi]tulo\s*(\d+)/g) || [])
  .map((t) => +t.replace(/\D+/g, ''))
const orig = path.join('/tmp/compara', 'enredo_orig.pdf')
/* auditoria 14/08: o oráculo constrói-se quando falta — o pdflatex está na
 * máquina, e é ELE a referência deste medidor */
if (!fs.existsSync(orig)) {
  try { sh(`node ${JSON.stringify(path.join(RAIZ, 'tools', 'compara.js'))} enredo`, { timeout: 900000 }) } catch (e) {}
}
if (fs.existsSync(orig)) {
  const sA = seqDe(orig), sB = seqCasa(pdf)
  const quebras = (v) => v.map((x, i) => (i && x !== v[i - 1] + 1 ? `${v[i - 1]}→${x}` : null)).filter(Boolean)
  const qA = quebras(sA), qB = quebras(sB)
  console.log(`     capítulos: pdflatex ${sA.length} (último ${sA[sA.length - 1]}), nosso ${sB.length} (último ${sB[sB.length - 1]})`)
  console.log(`     reposições: pdflatex [${qA}]  nosso [${qB}]`)
  ok('a sequência de capítulos é IGUAL à do pdflatex, reposições incluídas',
     sA.length > 0 && sA.length === sB.length && String(qA) === String(qB) &&
     sA.every((x, i) => x === sB[i]))
} else {
  ok('a sequência de capítulos é IGUAL à do pdflatex, reposições incluídas', false)
  console.log('     (sem o PDF do pdflatex em /tmp/compara — corre `node tools/compara.js enredo`)')
}

/* ─── §M4 A MUTAÇÃO: sem a avaliação, a capa desaparece ────────────────────────────── */
/* corta-se o \providecommand{\gkcapa} do estilo. Se a capa continuar a sair, ela não vinha
 * da avaliação — vinha de algum sítio escrito à mão, e §M2 estava a medir isso. */
const mut = path.join(TMP, 'mut')
fs.mkdirSync(mut, { recursive: true })
for (const f of ['estilo.tex', 'enredo.tex', 'livro.tex'])
  try { fs.copyFileSync(path.join(RAIZ, f), path.join(mut, f)) } catch (e) {}
const iC = estilo.indexOf('\\providecommand{\\gkcapa}')
let d = 1, k = estilo.indexOf('{', iC + 23) + 1
while (k < estilo.length && d) { if (estilo[k] === '{') d++; else if (estilo[k] === '}') d--; k++ }
fs.writeFileSync(path.join(mut, 'estilo.tex'), estilo.slice(0, iC) + estilo.slice(k))
const pdfM = path.join(TMP, 'mut.pdf')
sh(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(mut, 'enredo.tex'))} ${JSON.stringify(pdfM)}`,
   { cwd: __dirname, timeout: 900000 })
const p1M = fs.existsSync(pdfM) ? casa.textoPagina(fs.readFileSync(pdfM, 'latin1'), 1).replace(/\s+/g, '') : ''
/* auditoria 14/08: palavras da capa que TAMBÉM vivem no texto do enredo
 * (Reino, Dourado…) reaparecem na página 1 quando a capa sai — o conteúdo
 * não prova nada sobre a avaliação. Só as palavras exclusivas do estilo
 * têm de sumir, e essas somem EXATAS. */
const enredoTxt = fs.readFileSync(path.join(RAIZ, 'enredo.tex'), 'utf8')
const soDoEstilo = naCapa.filter((w) => !enredoTxt.includes(w))
const sobrou = soDoEstilo.filter((w) => p1M.includes(w))
console.log(`\n§M4  MUTAÇÃO: cortado o \\providecommand{\\gkcapa} do estilo`)
console.log(`     das ${soDoEstilo.length} palavras exclusivas do estilo, sobraram ${sobrou.length}: ${sobrou.join(', ') || '(nenhuma)'}`)
ok('sem a definição no estilo, as palavras EXCLUSIVAS do estilo somem — logo vinham da avaliação',
   soDoEstilo.length > 0 && sobrou.length === 0)

/* ─── fecho ───────────────────────────────────────────────────────────────────────── */
console.log(`\n${'='.repeat(74)}`)
console.log(`  ${feitas - falhas}/${feitas} unidades`)
console.log('')
console.log('  A avaliação não sabe o que traduz, e é por isso que traduz tudo: cada macro')
console.log('  nova que o autor escreva no estilo.tex passa a sair no PDF sem se tocar no')
console.log('  tradutor. Era o contrário — 74 definidas, 2 nomeadas, 4664 usos.')
console.log('')
process.exit(falhas ? 1 : 0)
