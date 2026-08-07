/* design_no_pdf.js — O DESIGN ESTÁ NO PDF, E NÃO SÓ ESPECIFICADO.
 *
 * O Aarão: «então não adianta nada, tem que ligar no pdf.»
 *
 * Tinha razão e é o defeito na sua forma mais pura: eu tinha medido que as peças de design
 * existem, que as cores saem da fonte certa e que os operadores as desenham — e nada disso
 * aparecia no PDF que sai ao clicar. UMA ESPECIFICAÇÃO MEDIDA QUE NÃO ESTÁ LIGADA MEDE A
 * ESPECIFICAÇÃO, NÃO O PRODUTO. É o mesmo que três verdes a medir outra coisa.
 *
 * Aqui mede-se o PDF. Abre-se o ficheiro que o compositor escreveu e contam-se os operadores
 * de desenho que lá estão — não o que o código diz que faz.
 *
 *   §G1  os operadores de desenho ESTÃO no PDF de cada documento
 *   §G2  a cor é a do estilo.tex — lida do PDF e conferida contra a fonte
 *   §G3  e o texto não se perdeu: desenhar põe-se por baixo, não corta
 *   §G4  o controlo: apagadas as réguas do fonte, o PDF perde-as — e volta
 *
 *   node tests/design_no_pdf.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TEX = path.join(RAIZ, 'tests', 'tex')
let falhas = 0
const ok = (m, c) => { console.log('      ' + (c ? '✓' : '✗') + '  ' + m); if (!c) falhas++ }

const DOCS = [
  ['teoria',        'teoria.tex'],
  ['catalogo',      'catalogo.tex'],
  ['corpo-estelar', 'papers/corpo-estelar.tex'],
  ['dualsort',      'papers/dualsort.tex'],
]

function compoe (fonte, saida) {
  try { execSync(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(RAIZ, fonte))} ${JSON.stringify(saida)}`,
                 { stdio: 'pipe', cwd: path.dirname(TEX), timeout: 180000 }); return true }
  catch (e) { return false }
}
function bruto (f) { try { return fs.readFileSync(f, 'latin1') } catch (e) { return '' } }
function conta (s, op) { let n = 0, i = 0; while ((i = s.indexOf(op, i)) >= 0) { n++; i += op.length } return n }

if (!fs.existsSync(TEX)) {
  console.log('\n  o tradutor tests/tex não está compilado — este medidor precisa dele.')
  console.log('  (cd tests && cc -O2 -std=c99 -I../lib tex.c -lm -o tex)   NÃO MEDIU.\n')
  process.exit(2)                          /* 2, e não 0: não medir não é passar */
}

console.log('\n=== O DESIGN ESTÁ NO PDF ===================================================\n')

console.log('§G1  Os operadores de desenho ESTÃO no PDF — contados no ficheiro.\n')
const medido = {}
{
  let sem = 0
  console.log('      documento         réguas   barras   cor')
  for (const [nome, fonte] of DOCS) {
    const f = `/tmp/g_${nome}.pdf`
    if (!compoe(fonte, f)) { sem++; continue }
    const s = bruto(f)
    const r = conta(s, ' l S '), b = conta(s, ' l f '), c = conta(s, ' RG ') + conta(s, ' rg ')
    medido[nome] = { r, b, c }
    if (r === 0 && b === 0) sem++
    console.log('      ' + nome.padEnd(17) + String(r).padEnd(8) + String(b).padEnd(8) + c)
  }
  ok('os operadores de desenho estão no PDF de cada documento, contados NO FICHEIRO e não no' +
     ' código. É esta a diferença que faltava: eu tinha medido que as peças existem e que os' +
     ' operadores as desenham, e nada disso aparecia no que sai ao clicar. Uma especificação' +
     ' medida que não está ligada mede a especificação, não o produto', sem === 0)
}

console.log('\n§G2  A COR é a do estilo.tex — lida do PDF, conferida contra a fonte.\n')
{
  /* a cor no PDF vem em fracções de 0 a 1; no estilo.tex vem em hexadecimal. Converte-se a
   * FONTE e procura-se no PDF — e não o contrário, para que mudar a cor lá mude isto. */
  const estilo = fs.readFileSync(path.join(RAIZ, 'estilo.tex'), 'utf8')
  const mt = /\\definecolor\{tinta\}\{HTML\}\{([0-9A-Fa-f]{6})\}/.exec(estilo)
  const mo = /\\definecolor\{ouro\}\{HTML\}\{([0-9A-Fa-f]{6})\}/.exec(estilo)
  const frac = (hex) => [0, 2, 4].map((i) => (parseInt(hex.slice(i, i + 2), 16) / 255).toFixed(3)).join(' ')
  const s = bruto('/tmp/g_catalogo.pdf')
  const temTinta = mt && s.includes(frac(mt[1]))
  const temOuro  = mo && s.includes(frac(mo[1]))
  console.log('      tinta ' + (mt ? '#' + mt[1] + ' → ' + frac(mt[1]) : '?') + '   no PDF: ' + (temTinta ? 'sim' : 'NÃO'))
  console.log('      ouro  ' + (mo ? '#' + mo[1] + ' → ' + frac(mo[1]) : '?') + '   no PDF: ' + (temOuro ? 'sim' : 'NÃO'))
  ok('as cores que aparecem no PDF são as que o estilo.tex declara — convertidas da FONTE e' +
     ' procuradas no ficheiro, e não o contrário. Assim, mudar uma cor no estilo muda o que' +
     ' sai, e este medidor acompanha sozinho', !!temTinta && !!temOuro)
}

console.log('\n§G3  E o TEXTO não se perdeu: desenhar põe-se por baixo, não corta.\n')
{
  /* a segunda metade: acrescentar desenho não pode tirar palavras. Se tirasse, o desenho
   * estaria a cortar o fluxo — e nenhuma peça de design corta (q=0). */
  let texto = ''
  try { texto = execSync(`pdftotext ${JSON.stringify('/tmp/g_catalogo.pdf')} -`,
                         { encoding: 'utf8', stdio: 'pipe', maxBuffer: 1 << 28 }) } catch (e) {}
  const pal = new Set((texto.match(/[A-Za-zÀ-ÿ]{8,}/g) || []))
  console.log('      palavras longas no PDF do catálogo: ' + pal.size)
  console.log('      réguas desenhadas no mesmo PDF:     ' + (medido.catalogo ? medido.catalogo.r : 0))
  ok('o PDF tem desenho E tem texto — milhares de palavras longas ao lado de centenas de' +
     ' réguas. São as duas metades: o desenho apareceu (o que não podia ser zero) e o texto' +
     ' ficou (o que não podia diminuir). É a assinatura das peças de design a confirmar-se no' +
     ' produto: nenhuma corta (q=0), porque desenhar põe-se por baixo do texto e não o parte',
     pal.size > 1000 && medido.catalogo && medido.catalogo.r > 100)
}

console.log('\n§G4  O CONTROLO: apagadas as réguas do fonte, o PDF perde-as — e volta.\n')
{
  /* sem isto, «há réguas no PDF» passava com réguas que viessem de qualquer lado. Tira-se a
   * CAUSA no fonte e a consequência tem de desaparecer. O fonte é devolvido sempre. */
  const alvo = path.join(RAIZ, 'papers', 'dualsort.tex')
  const original = fs.readFileSync(alvo, 'utf8')
  let antes = 0, depois = 0, voltou = 0
  try {
    compoe('papers/dualsort.tex', '/tmp/g_ctrl1.pdf')
    antes = conta(bruto('/tmp/g_ctrl1.pdf'), ' l S ')
    fs.writeFileSync(alvo, original.replace(/\\(top|mid|bottom)rule/g, ''))
    compoe('papers/dualsort.tex', '/tmp/g_ctrl2.pdf')
    depois = conta(bruto('/tmp/g_ctrl2.pdf'), ' l S ')
  } finally {
    fs.writeFileSync(alvo, original)                 /* devolve-se SEMPRE */
  }
  compoe('papers/dualsort.tex', '/tmp/g_ctrl3.pdf')
  voltou = conta(bruto('/tmp/g_ctrl3.pdf'), ' l S ')
  console.log('      com as réguas no fonte:   ' + antes)
  console.log('      apagadas do fonte:        ' + depois)
  console.log('      revertido o fonte:        ' + voltou)
  ok('tiradas as réguas do FONTE, o PDF perde-as; revertido, voltam ao mesmo número. Sem esta' +
     ' metade, «há réguas no PDF» passava com réguas vindas de qualquer lado — e é a mesma' +
     ' mutação que separou «composto agora» de «servido de uma cópia»: mexe-se na causa e' +
     ' vê-se a consequência', antes > 0 && depois < antes && voltou === antes)
}

console.log('\n=== O DESIGN NO PDF =========================================================')
console.log('  Estava especificado e medido, e não estava LIGADO. Uma especificação medida que')
console.log('  não chega ao produto mede a especificação — e é o mesmo defeito de sempre:')
console.log('  três verdes a medir outra coisa.')
console.log('')
console.log('  Agora conta-se no FICHEIRO: as réguas do booktabs saem como caminho de grau 1')
console.log('  (dois pontos, traçado) e a barra da caixa como retângulo preenchido — os mesmos')
console.log('  operadores do glifo, com outro grau. E a cor vem do estilo.tex, não daqui.')
console.log('')
console.log('  O que fica por ligar está nomeado: o FUNDO da caixa. Um stream de PDF é')
console.log('  sequencial, e um fundo escrito depois do texto TAPA-O — pede dois streams (o')
console.log('  /Contents aceita um array). A barra vive na margem e por isso já pode vir.')
if (falhas) { console.log('\n  FALHAS: ' + falhas + '\n'); process.exit(1) }
console.log('\n  RESÍDUO 0 — o design está no PDF, contado no ficheiro.\n')
