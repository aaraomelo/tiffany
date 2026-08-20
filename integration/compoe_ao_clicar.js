/* compoe_ao_clicar.js — O PDF SAI DO FONTE AO CLICAR, E NÃO HÁ NADA PRÉ-GRAVADO.
 *
 * O Aarão: «quando clica renderiza em tempo real, nada pré-gravado» · «lê no local dele e
 * converte pra pdf» · «não precisa mover os arquivos de lugar».
 *
 * Um PDF pré-compilado é uma GRAVAÇÃO — a mesma coisa que o GIF era ao lado do kernel, e
 * envelhece do mesmo modo. Esta semana dois artigos foram servidos de uma publicação anterior
 * enquanto o link devolvia 200: o workflow que corre a cada push não os compilava, e a
 * conferência era uma lista escrita à mão onde ninguém os tinha acrescentado.
 *
 * Aqui o pedido chega, o tradutor corre sobre o `.tex` ONDE ELE ESTÁ, e o PDF sai. Não se move
 * ficheiro nenhum e não fica cópia no dist.
 *
 * E A MEDIDA QUE IMPORTA NÃO É O 200 — é essa exactamente a que falhou. Um ficheiro velho
 * também devolve 200. A medida é: MUDA-SE O FONTE, e o que sai muda? E reverte-se, e volta?
 * Só isso separa «composto agora» de «servido de uma cópia».
 *
 *   §Z1  os cinco documentos saem, e saem com páginas — não um PDF truncado
 *   §Z2  A PROVA: muda-se o fonte e o que sai MUDA; reverte-se e VOLTA
 *   §Z3  e não há cópia guardada: o dist não tem os PDFs
 *   §Z4  o controlo: com o tradutor fora, falha RUIDOSAMENTE — não serve o antigo
 *
 *   node integration/compoe_ao_clicar.js    (precisa do servidor local em 8099)
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const URL_BASE = process.env.APP_URL || 'http://localhost:8099'
let falhas = 0
const ok = (msg, cond) => { console.log('      ' + (cond ? '✓' : '✗') + '  ' + msg); if (!cond) falhas++ }

function pede (doc, saida) {
  try {
    execSync(`curl -s -o ${JSON.stringify(saida)} -w "%{http_code}" --max-time 180 ` +
             `${JSON.stringify(URL_BASE + '/docs/' + doc + '.pdf')}`,
             { encoding: 'utf8', stdio: 'pipe' })
    return fs.existsSync(saida) ? fs.statSync(saida).size : 0
  } catch (e) { return 0 }
}
function paginas (f) {
  try { const s = execSync(`pdfinfo ${JSON.stringify(f)}`, { encoding: 'utf8', stdio: 'pipe' })
        const m = /^Pages:\s*(\d+)/m.exec(s); return m ? parseInt(m[1], 10) : 0 } catch (e) { return 0 }
}
function texto (f) {
  try { return execSync(`pdftotext ${JSON.stringify(f)} -`, { encoding: 'utf8', stdio: 'pipe', maxBuffer: 1 << 28 }) }
  catch (e) { return '' }
}

/* o servidor está de pé? sem ele isto não mede nada, e dizê-lo é melhor que dar verde */
let vivo = false
try { execSync(`curl -s -o /dev/null --max-time 5 ${JSON.stringify(URL_BASE)}`, { stdio: 'pipe' }); vivo = true } catch (e) {}
if (!vivo) {
  console.log('\n  o servidor local não responde em ' + URL_BASE + ' — este medidor precisa dele.')
  console.log('  (cd app && npx vite preview --port 8099)  NÃO MEDIU.\n')
  process.exit(2)                     /* 2, e não 0: não medir não é passar */
}

console.log('\n=== O PDF SAI DO FONTE AO CLICAR ===========================================\n')

console.log('§Z1  Os cinco documentos saem, e saem com páginas.\n')
const DOCS = ['teoria', 'catalogo', 'enredo', 'corpo_analitico', 'computacional']
{
  let vazios = 0, sem_pag = 0
  console.log('      documento         bytes        páginas')
  for (const d of DOCS) {
    const f = `/tmp/z_${d}.pdf`
    const n = pede(d, f)
    const p = paginas(f)
    if (n < 1000) vazios++
    if (p < 1) sem_pag++
    console.log('      ' + d.padEnd(17) + String(n).padEnd(12) + p)
  }
  ok('os cinco saem com corpo e com páginas — um PDF de zero páginas também devolveria 200, e' +
     ' é por isso que o código de estado não é a medida', vazios === 0 && sem_pag === 0)
}

console.log('\n§Z2  A PROVA: muda-se o FONTE e o que sai MUDA; reverte-se e VOLTA.\n')
{
  /* É AQUI QUE SE SEPARA «composto agora» de «servido de uma cópia», e o 200 não o separa.
   * Escolhe-se uma palavra que não existe em lado nenhum, põe-se no fonte, pede-se, e depois
   * tira-se e pede-se outra vez. O fonte é devolvido SEMPRE, mesmo se algo rebentar. */
  const alvo = path.join(RAIZ, 'papers', 'arquitetura.tex')
  const original = fs.readFileSync(alvo, 'utf8')
  const marca = 'SEMPREVIVAZZ'
  const ancora = 'A referência são os naturais'
  let apareceu = false, sumiu = false, mordeu = false
  try {
    mordeu = original.includes(ancora)
    if (mordeu) {
      fs.writeFileSync(alvo, original.replace(ancora, ancora + ' ' + marca))
      pede('computacional', '/tmp/z_mud.pdf')
      apareceu = texto('/tmp/z_mud.pdf').includes(marca)
    }
  } finally {
    fs.writeFileSync(alvo, original)                 /* devolve-se SEMPRE */
  }
  pede('computacional', '/tmp/z_rev.pdf')
  sumiu = !texto('/tmp/z_rev.pdf').includes(marca)
  const devolvido = fs.readFileSync(alvo, 'utf8') === original
  console.log('      com a palavra no fonte, o PDF servido tem-na:      ' + (apareceu ? 'sim' : 'NÃO'))
  console.log('      revertido o fonte, o PDF servido deixa de a ter:   ' + (sumiu ? 'sim' : 'NÃO'))
  console.log('      e o ficheiro voltou ao original:                   ' + (devolvido ? 'sim' : 'NÃO'))
  ok('mudado o FONTE, o que sai muda; revertido, volta — SEM rebuild e SEM reiniciar nada.' +
     ' São as duas metades, e nenhuma sozinha chega: se só aparecesse, podia ser um cache a' +
     ' encher-se; se só sumisse, podia nunca lá ter estado. É isto que o código 200 não diz —' +
     ' um ficheiro velho também devolve 200, e foi assim que dois artigos ficaram a ser' +
     ' servidos de uma publicação anterior', mordeu && apareceu && sumiu && devolvido)
}

console.log('\n§Z3  E não há cópia guardada: o dist não tem os PDFs.\n')
{
  /* se ficasse uma cópia no dist, ela seria servida em vez da composição — e voltávamos ao
   * princípio. A ausência é o que se mede, e mede-se olhando. */
  const dist = path.join(RAIZ, 'app', 'dist', 'docs')
  let copias = []
  if (fs.existsSync(dist)) copias = fs.readdirSync(dist).filter((f) => f.endsWith('.pdf'))
  console.log('      PDFs em app/dist/docs: ' + copias.length + (copias.length ? ' — ' + copias.join(', ') : ''))
  ok('não há PDF guardado no dist — se houvesse, era ele o servido, e a composição não passava' +
     ' de enfeite. A ausência é que faz o caminho ser um só', copias.length === 0)
}

console.log('\n§Z4  O CONTROLO: sem o tradutor, falha RUIDOSAMENTE — não serve o antigo.\n')
{
  /* tira-se o tradutor do sítio e pede-se. O que NÃO pode acontecer é vir um PDF: cair num
   * ficheiro antigo seria servir a gravação outra vez, calado — o defeito que isto desfaz. */
  const bin = path.join(RAIZ, 'tests', 'tex')
  const guardado = bin + '.guardado'
  let calou = true, codigo = ''
  if (fs.existsSync(bin)) {
    fs.renameSync(bin, guardado)
    try {
      codigo = execSync(`curl -s -o /tmp/z_sem.pdf -w "%{http_code}" --max-time 60 ` +
                        `${JSON.stringify(URL_BASE + '/docs/computacional.pdf')}`,
                        { encoding: 'utf8', stdio: 'pipe' }).trim()
      const n = fs.existsSync('/tmp/z_sem.pdf') ? fs.statSync('/tmp/z_sem.pdf').size : 0
      /* «calou-se» = devolveu um PDF na mesma. É isso que não pode ser. */
      calou = (codigo === '200' && n > 1000)
    } catch (e) { calou = false } finally {
      fs.renameSync(guardado, bin)
    }
  }
  console.log('      sem o tradutor, o pedido devolve: ' + codigo + ' (200 com PDF seria o antigo)')
  ok('sem o tradutor a composição FALHA e diz que falhou — não cai num ficheiro antigo. Um' +
     ' fallback silencioso aqui seria reconstruir exactamente o defeito: serve-se a gravação e' +
     ' ninguém sabe. Falhar alto é a única forma de o 200 voltar a querer dizer alguma coisa',
     !calou)
}

console.log('\n=== COMPOR AO CLICAR ========================================================')
console.log('  O pedido chega, o tradutor corre sobre o .tex ONDE ELE ESTÁ, e o PDF sai. Nada se')
console.log('  move de lugar e nada fica guardado — nem no dist, nem em cache.')
console.log('')
console.log('  E a medida não é o 200: um ficheiro velho também devolve 200, e foi assim que dois')
console.log('  artigos ficaram a ser servidos de uma publicação anterior sem ninguém dar por isso.')
console.log('  A medida é MUDAR O FONTE e ver o que sai mudar — e revertê-lo e ver voltar.')
if (falhas) { console.log('\n  FALHAS: ' + falhas + '\n'); process.exit(1) }
console.log('\n  RESÍDUO 0 — o que se serve é o que o fonte diz, agora.\n')
