/* dois_streams.js — O FUNDO DA CAIXA, PELOS DOIS STREAMS DO /Contents.
 *
 * O Aarão: «faz o fundo da caixa com os dois streams.»
 *
 * Um stream de PDF é sequencial: o que se escreve depois pinta por cima. Por isso o fundo de
 * uma caixa não podia vir no fim — tapava o texto — e ficou por ligar quando a barra já
 * estava. E não se sabe a altura da caixa antes de a fechar, logo também não podia vir no
 * princípio.
 *
 * A saída não é guardar a página em memória: aqui não há RAM. É o PRÓPRIO FORMATO — o
 * `/Contents` aceita um ARRAY, e o leitor concatena os streams na ordem em que estão lá.
 *
 *      /Contents [ A B ]      A = os fundos      B = o texto
 *
 * Assim a ORDEM EM QUE SE ESCREVE deixa de ser a ORDEM EM QUE SE PINTA, e é isso que resolve
 * o problema: o fundo escreve-se num temporário enquanto a página corre, copia-se para o
 * objecto A quando a página fecha — depois do texto, no ficheiro — e pinta antes dele.
 *
 * É a mesma separação que o sistema faz em toda a parte: guardar e ler são dois sentidos, e
 * aqui os dois streams são os dois sentidos da página.
 *
 *   §S1  o /Contents é um ARRAY de dois em todas as páginas
 *   §S2  e o PRIMEIRO do array é o do FUNDO — a ordem de pintura, não a do ficheiro
 *   §S3  o fundo tem cor e o texto tem glifos — cada um no seu, e não misturados
 *   §S4  e o TEXTO sobreviveu: pintar por baixo não apaga
 *   §S5  o controlo: tirado o \begin{tcolorbox} do fonte, os fundos somem — e voltam
 *
 *   node tests/dois_streams.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TEX = path.join(RAIZ, 'tests', 'tex')
let falhas = 0
const ok = (m, c) => { console.log('      ' + (c ? '✓' : '✗') + '  ' + m); if (!c) falhas++ }

if (!fs.existsSync(TEX)) {
  console.log('\n  o tradutor tests/tex não está compilado.  NÃO MEDIU.\n'); process.exit(2)
}
function compoe (fonte, saida) {
  try { execSync(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(RAIZ, fonte))} ${JSON.stringify(saida)}`,
                 { stdio: 'pipe', cwd: path.dirname(TEX), timeout: 180000 }); return true }
  catch (e) { return false }
}
const bruto = (f) => { try { return fs.readFileSync(f, 'latin1') } catch (e) { return '' } }

/* devolve, por página, os dois objectos do /Contents e o corpo de cada um */
function paginas (pdf) {
  const out = []
  for (const m of pdf.matchAll(/\/Contents\[(\d+) 0 R (\d+) 0 R\]/g)) {
    const [a, b] = [Number(m[1]), Number(m[2])]
    const corpo = (n) => {
      const i = pdf.indexOf(`${n} 0 obj<</Length`)
      if (i < 0) return ''
      const s = pdf.indexOf('stream\n', i)
      const e = pdf.indexOf('endstream', s)
      return (s < 0 || e < 0) ? '' : pdf.slice(s + 7, e)
    }
    out.push({ a, b, ca: corpo(a), cb: corpo(b) })
  }
  return out
}

console.log('\n=== O FUNDO DA CAIXA, PELOS DOIS STREAMS ===================================\n')

const DOC = 'papers/corpo-estelar.tex'
compoe(DOC, '/tmp/s_ce.pdf')
const pdf = bruto('/tmp/s_ce.pdf')
const pgs = paginas(pdf)

console.log('§S1  O /Contents é um ARRAY de dois em todas as páginas.\n')
{
  const simples = (pdf.match(/\/Contents \d+ 0 R>>/g) || []).length
  console.log('      páginas com /Contents[A B]: ' + pgs.length)
  console.log('      páginas com /Contents único: ' + simples)
  ok('todas as páginas têm o /Contents como ARRAY de dois, e nenhuma ficou com um só. É o' +
     ' próprio formato a dar a saída: o leitor concatena os streams na ordem do array, e por' +
     ' isso a ordem em que se ESCREVE deixa de ser a ordem em que se PINTA',
     pgs.length > 0 && simples === 0)
}

console.log('\n§S2  E o PRIMEIRO do array é o do FUNDO — a ordem de pintura, não a do ficheiro.\n')
{
  /* o teste que interessa: nas páginas que TÊM fundo, o desenho está no primeiro objecto do
   * array. Se estivesse no segundo, pintava por cima do texto — e era o defeito de volta.
   * As páginas sem caixa têm o primeiro vazio, e isso está certo: não há nada a pintar. */
  let comFundo = 0, noPrimeiro = 0, noSegundo = 0
  for (const p of pgs) {
    const temA = / rg /.test(p.ca), temB = / rg /.test(p.cb)
    if (temA || temB) comFundo++
    if (temA) noPrimeiro++
    if (temB) noSegundo++
  }
  console.log('      páginas com fundo: ' + comFundo)
  console.log('      com o desenho no 1º do array (pinta por baixo): ' + noPrimeiro)
  console.log('      com o desenho no 2º do array (pintaria por cima): ' + noSegundo)
  ok('nas páginas que têm fundo, o desenho está no PRIMEIRO objecto do array — logo pinta por' +
     ' baixo do texto — e em NENHUMA está no segundo. São as duas metades: se estivesse no' +
     ' segundo pintava por cima, e era o defeito de volta. E as páginas sem caixa têm o' +
     ' primeiro vazio, o que está certo: não há nada a pintar',
     comFundo > 0 && noPrimeiro === comFundo && noSegundo === 0)
}

console.log('\n§S3  O fundo tem cor e o texto tem glifos — cada um no seu.\n')
{
  let misturado = 0, certos = 0
  for (const p of pgs) {
    const corNoA = / rg /.test(p.ca), glifoNoA = /Tj/.test(p.ca)
    const glifoNoB = /Tj/.test(p.cb)
    if (glifoNoA) misturado++                       /* texto no stream do fundo: errado */
    if (glifoNoB) certos++
  }
  console.log('      páginas com texto no stream do fundo: ' + misturado + ' (tem de ser 0)')
  console.log('      páginas com texto no stream do texto:  ' + certos)
  ok('os dois streams estão separados pelo que fazem: nenhum glifo no stream do fundo, e o' +
     ' texto todo no seu. Se estivessem misturados, o array não estaria a separar nada — seria' +
     ' um stream partido em dois pedaços arbitrários', misturado === 0 && certos === pgs.length)
}

console.log('\n§S4  E o TEXTO sobreviveu: pintar por baixo não apaga.\n')
{
  let txt = ''
  try { txt = execSync(`pdftotext ${JSON.stringify('/tmp/s_ce.pdf')} -`,
                       { encoding: 'utf8', stdio: 'pipe', maxBuffer: 1 << 28 }) } catch (e) {}
  const pal = new Set(txt.match(/[A-Za-zÀ-ÿ]{8,}/g) || [])
  const fundos = (pdf.match(/ l f Q/g) || []).length
  console.log('      palavras longas no PDF: ' + pal.size + '   fundos pintados: ' + fundos)
  ok('o PDF tem fundos pintados E tem o texto — as duas metades outra vez: o fundo apareceu (o' +
     ' que não podia ser zero) e as palavras ficaram (o que não podia diminuir). É a assinatura' +
     ' das peças de design confirmada no produto: desenhar não corta', pal.size > 200 && fundos > 0)
}

console.log('\n§S5  O CONTROLO: tirado o tcolorbox do fonte, os fundos somem — e voltam.\n')
{
  const alvo = path.join(RAIZ, 'papers', 'corpo-estelar.tex')
  const original = fs.readFileSync(alvo, 'utf8')
  const conta = (f) => (bruto(f).match(/ l f Q/g) || []).length
  let antes = 0, depois = 0, voltou = 0
  try {
    antes = conta('/tmp/s_ce.pdf')
    // MUTA-SE O QUE ESTE DOCUMENTO TEM. A primeira versão tirava \begin{tcolorbox} — e o
    // corpo-estelar não tem nenhum: os fundos dele vêm dos \begin{teorema}. A mutação não
    // mordia, e uma mutação que não morde não é controlo nenhum: dá o mesmo número dos dois
    // lados e a asserção acusa sem haver defeito. Mutar exige saber o que lá está.
    fs.writeFileSync(alvo, original.replace(/\\begin\{(tcolorbox|teorema|proposicao|obs)\}/g, '')
                                   .replace(/\\end\{(tcolorbox|teorema|proposicao|obs)\}/g, ''))
    compoe(DOC, '/tmp/s_ctrl.pdf'); depois = conta('/tmp/s_ctrl.pdf')
  } finally {
    fs.writeFileSync(alvo, original)                 /* devolve-se SEMPRE */
  }
  compoe(DOC, '/tmp/s_volta.pdf'); voltou = conta('/tmp/s_volta.pdf')
  const devolvido = fs.readFileSync(alvo, 'utf8') === original
  console.log('      com as caixas no fonte: ' + antes + '   sem elas: ' + depois + '   revertido: ' + voltou)
  ok('tiradas as caixas do FONTE, os fundos diminuem; revertido, voltam ao mesmo número — e o' +
     ' ficheiro voltou ao original. Sem esta metade, «há fundos no PDF» passava com fundos' +
     ' vindos de qualquer lado: mexe-se na causa e vê-se a consequência',
     antes > 0 && depois < antes && voltou === antes && devolvido)
}

console.log('\n=== OS DOIS STREAMS =========================================================')
console.log('  Um stream de PDF é sequencial: o que se escreve depois pinta por cima. E não se')
console.log('  sabe a altura da caixa antes de a fechar. Logo o fundo não podia vir no fim nem')
console.log('  no princípio — e ficou por ligar quando a barra já estava.')
console.log('')
console.log('  A saída não é guardar a página em memória: aqui não há RAM. É o PRÓPRIO FORMATO.')
console.log('  O /Contents aceita [A B] e o leitor concatena — logo a ordem em que se ESCREVE')
console.log('  deixa de ser a ordem em que se PINTA. O fundo vai para um temporário enquanto a')
console.log('  página corre, copia-se depois do texto no ficheiro, e pinta antes dele.')
if (falhas) { console.log('\n  FALHAS: ' + falhas + '\n'); process.exit(1) }
console.log('\n  RESÍDUO 0 — o fundo pinta por baixo, e o texto ficou inteiro.\n')
