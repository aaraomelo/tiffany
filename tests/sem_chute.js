/* sem_chute.js — O TRADUTOR NÃO CHUTA LARGURAS, E O PDF DIZ-NO.
 *
 * O Aarão: «agora aplica isso no tex.c e vê o PDF.»
 *
 * A assinatura da letra aplicada ao tradutor deu num sítio que eu não tinha olhado: o CHUTE.
 * O `largura()` tinha um `return 556` para quando não conhecia o glifo — e ele disparava 106
 * vezes no catálogo, calado.
 *
 * E o que ele chutava eram os caracteres tipográficos:
 *
 *      o travessão    U+2014    mede 1000      e o chute dava 556       falta 44%
 *      as reticências U+2026    mede 1000      e o chute dava 556       falta 44%
 *
 * A causa: o PDF escreve em WinAnsiEncoding, onde o travessão é o código 151. Mas o `cmap` de
 * uma TTF é indexado por UNICODE, e lá ele é U+2014. Eu procurava o 151 no cmap, não achava
 * nada, e chutava — quando A FONTE TINHA O GLIFO. Perguntava pelo número errado.
 *
 * E COMO ESPAÇAR SOMA, cada travessão desalinhava a linha INTEIRA a partir dali. É exactamente
 * o «uns espaços ficaram maiores» que se via: eu não tinha aumentado espaço nenhum — tinha
 * DEIXADO DE CONTAR 444 milésimos em cada travessão.
 *
 *   §N1  o tradutor não chuta: zero larguras inventadas nos quatro documentos
 *   §N2  e os glifos que ele chutava têm largura REAL na fonte — medida
 *   §N3  o PDF não tem sobreposições: zero em milhares de pares
 *   §N4  o controlo: reposto o chute, ele volta a disparar — e diz quantas vezes
 *
 *   node tests/sem_chute.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TEX = path.join(RAIZ, 'tests', 'tex')
let falhas = 0
const ok = (m, c) => { console.log('      ' + (c ? '✓' : '✗') + '  ' + m); if (!c) falhas++ }

if (!fs.existsSync(TEX)) { console.log('\n  tests/tex não compilado.  NÃO MEDIU.\n'); process.exit(2) }

function compoe (fonte, saida) {
  try {
    const r = execSync(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(RAIZ, fonte))} ` +
                       `${JSON.stringify(saida)} 2>&1`,
                       { encoding: 'utf8', cwd: path.dirname(TEX), timeout: 300000 })
    return r
  } catch (e) { return String(e.stdout || '') + String(e.stderr || '') }
}
const chutes = (saida) => { const m = /AVISO: (\d+) larguras CHUTADAS/.exec(saida); return m ? Number(m[1]) : 0 }

const DOCS = [
  ['dualsort',      'papers/dualsort.tex'],
  ['corpo-estelar', 'papers/corpo-estelar.tex'],
  ['teoria',        'teoria.tex'],
  ['catalogo',      'catalogo.tex'],
]

console.log('\n=== O TRADUTOR NÃO CHUTA LARGURAS ==========================================\n')

console.log('§N1  Zero larguras inventadas — nos quatro documentos.\n')
{
  let total = 0
  console.log('      documento         chutes')
  for (const [n, f] of DOCS) {
    const s = compoe(f, `/tmp/n_${n}.pdf`)
    const c = chutes(s)
    total += c
    console.log('      ' + n.padEnd(17) + c)
  }
  ok('nenhuma largura chutada em nenhum documento. O `largura()` tinha um `return 556` para' +
     ' quando não conhecia o glifo, e ele disparava 106 vezes só no catálogo — CALADO. Um chute' +
     ' contado é um defeito visível; um chute calado é o texto tosco sem se saber porquê',
     total === 0)
}

console.log('\n§N2  E os glifos que ele chutava têm largura REAL — e não é 556.\n')
{
  /* a metade que dá peso à primeira: se o chute acertasse, não havia defeito nenhum. Mede-se
   * a largura de facto na fonte e compara-se com o que o chute dava. */
  const src = `#define _GNU_SOURCE
#include <stdio.h>
#include "spline.h"
int main(void){ Ttf t; if(!spline_abre_alguma(&t,SPLINE_REG,SPLINE_NCAND,NULL)) return 1;
  int u[]={0x2014,0x2026,0x2019}; for(int i=0;i<3;i++){ int g=ttf_glifo(&t,u[i]);
  printf("%d %ld\\n", u[i], g?(long)ttf_avanco(&t,g)*1000/t.upem:0);} return 0;}`
  fs.writeFileSync('/tmp/n_larg.c', src)
  let saida = ''
  try {
    execSync(`cc -O2 -std=gnu99 -I${JSON.stringify(path.join(RAIZ, 'lib'))} /tmp/n_larg.c -o /tmp/n_larg`,
             { stdio: 'pipe' })
    saida = execSync('/tmp/n_larg', { encoding: 'utf8' })
  } catch (e) {}
  const linhas = saida.trim().split('\n').filter(Boolean).map((l) => l.split(' ').map(Number))
  const NOME = { 0x2014: 'travessão', 0x2026: 'reticências', 0x2019: 'apóstrofo' }
  let diferem = 0
  console.log('      glifo          real   o chute dava   erro')
  for (const [u, w] of linhas) {
    if (w !== 556) diferem++
    console.log('      ' + (NOME[u] || u).padEnd(14) + String(w).padEnd(6) + '556'.padEnd(15) +
                (w ? Math.round(100 * Math.abs(w - 556) / w) + '%' : '—'))
  }
  ok('os glifos que o chute apanhava têm largura REAL diferente de 556 — o travessão mede 1000,' +
     ' e o chute faltava 44%. É a metade que dá peso à primeira: se o chute acertasse, não havia' +
     ' defeito nenhum. E como espaçar SOMA, cada travessão desalinhava a linha inteira a partir' +
     ' dali — o «uns espaços ficaram maiores» era eu a NÃO CONTAR 444 milésimos de cada vez',
     linhas.length >= 2 && diferem >= 2)
}

console.log('\n§N3  E o PDF não tem sobreposições.\n')
{
  const d = fs.readFileSync('/tmp/n_catalogo.pdf', 'latin1')
  const pgs = [...d.matchAll(/stream\n([\s\S]*?)endstream/g)].map((m) => m[1]).filter((s) => s.includes('Tj'))
  const pat = /BT \/F\d (\d+) Tf ([\d.]+) ([\d.]+) Td(?: [\d.]+ Tw)? \(([\s\S]*?)\) Tj ET/g
  let sobre = 0, tot = 0
  for (const s of pgs.slice(0, 80)) {
    const L = new Map()
    for (const m of s.matchAll(pat)) {
      const y = Math.round(Number(m[3]) * 10) / 10
      if (!L.has(y)) L.set(y, [])
      L.get(y).push(Number(m[2]))
    }
    for (const v of L.values()) {
      v.sort((a, b) => a - b)
      for (let i = 0; i < v.length - 1; i++) { tot++; if (v[i + 1] <= v[i] + 0.5) sobre++ }
    }
  }
  console.log('      ' + pgs.length + ' páginas, ' + tot + ' pares na mesma linha, ' + sobre + ' sobrepostos')
  ok('nenhum par de pedaços na mesma linha começa antes ou no mesmo sítio do anterior — e a' +
     ' medida é feita POR PÁGINA, porque agrupar por Y sem separar páginas dá falsos positivos' +
     ' (foi o que me aconteceu antes)', tot > 1000 && sobre === 0)
}

console.log('\n§N4  O CONTROLO: reposto o chute, ele volta a disparar.\n')
{
  /* tira-se a conversão WinAnsi→Unicode e o chute tem de voltar. Sem esta metade, «zero
   * chutes» passava mesmo que o contador nunca fosse incrementado.
   *
   * E MUTA-SE SOBRE O CATÁLOGO e não sobre a teoria: à primeira usei a teoria, que não tem
   * travessões nenhuns — a mutação deu zero dos dois lados e a asserção acusou, com razão.
   * UMA MUTAÇÃO SÓ MORDE ONDE O DEFEITO PODE APARECER, e escolher o documento errado é o
   * mesmo que não mutar. */
  const alvo = path.join(RAIZ, 'tests', 'tex.c')
  const original = fs.readFileSync(alvo, 'utf8')
  let antes = 0, depois = 0, voltou = 0, mordeu = false
  try {
    antes = chutes(compoe('catalogo.tex', '/tmp/n_c1.pdf'))
    /* MUTA-SE ONDE A COMPOSIÇÃO LÊ, e não onde a tabela lê. Desde que o /Widths existe há
     * duas chamadas ao mapeamento: a do `largura()` (que compõe, e conta chutes) e a do
     * `largura_tabela()` (que preenche a tabela, e não conta). Mutar a segunda não muda o
     * número de chutes — a mutação deixou de morder onde eu a apontava. */
    const a = '        int gi = ttf_glifo(t, winansi_para_unicode(g));\n' +
              '        /* o glifo 0'
    mordeu = original.includes(a)
    if (mordeu) {
      fs.writeFileSync(alvo, original.replace(a, '        int gi = ttf_glifo(t, g);\n' +
                                                 '        /* o glifo 0'))
      execSync(`cc -O2 -std=gnu99 -I../lib tex.c -lm -o tex`, { cwd: path.dirname(TEX), stdio: 'pipe' })
      depois = chutes(compoe('catalogo.tex', '/tmp/n_c2.pdf'))
    }
  } catch (e) {} finally {
    fs.writeFileSync(alvo, original)                     /* devolve-se SEMPRE */
    try { execSync(`cc -O2 -std=gnu99 -I../lib tex.c -lm -o tex`, { cwd: path.dirname(TEX), stdio: 'pipe' }) } catch (e) {}
  }
  voltou = chutes(compoe('catalogo.tex', '/tmp/n_c3.pdf'))
  const devolvido = fs.readFileSync(alvo, 'utf8') === original
  console.log('      com a conversão WinAnsi→Unicode:  ' + antes + ' chutes')
  console.log('      sem ela:                          ' + depois + ' chutes')
  console.log('      revertido:                        ' + voltou + ' chutes')
  ok('tirada a conversão WinAnsi→Unicode, o chute VOLTA a disparar; reposta, desaparece — e o' +
     ' ficheiro voltou ao original. Sem esta metade, «zero chutes» passava mesmo que o contador' +
     ' nunca fosse incrementado. Mexe-se na causa e vê-se a consequência',
     mordeu && antes === 0 && depois > 0 && voltou === 0 && devolvido)
}

console.log('\n§N5  E a LETRA LATINA no modo matemático não é grega.\n')
{
  /* O DEFEITO ERA UMA LINHA: `(e.mat && isalpha(g)) ? F_SIM : e.fonte` — toda a letra latina
   * dentro de $...$ ia para a Symbol, onde o `i` é iota e o `u` é upsilon. E os gregos JÁ eram
   * tratados pela tabela do léxico, dez linhas acima: era uma segunda regra a fazer o mesmo
   * trabalho para quem não lhe pertencia. */
  let txt = ''
  try { txt = execSync(`pdftotext ${JSON.stringify('/tmp/n_catalogo.pdf')} -`,
                       { encoding: 'utf8', stdio: 'pipe', maxBuffer: 1 << 28 }) } catch (e) {}
  /* os gregos que NÃO têm comando próprio no léxico são os que estavam a substituir latinas:
   * iota, upsilon, pi-variante. Os legítimos — sigma, nu, lambda — vêm de \\sigma e ficam. */
  const trocados = (txt.match(/[ικυϖϕ]/g) || []).length
  const legitimos = (txt.match(/[σνπλβζθ]/g) || []).length
  console.log('      gregos que substituíam latinas (ι υ ϖ ϕ): ' + trocados)
  console.log('      gregos legítimos, de \\sigma \\nu \\lambda:      ' + legitimos)
  /* as duas metades: os trocados têm de ser POUCOS e os legítimos MUITOS. Se ambos fossem
   * zero, o modo matemático não estaria a compor nada; se os trocados fossem muitos, a regra
   * continuava lá. */
  ok('a letra latina no modo matemático fica na fonte do TEXTO, e os gregos legítimos ficam na' +
     ' Symbol — «∑_i u_i v_i» em vez de «∑_ι υ_ι ϖ_ι». Eram 7121 letras trocadas no catálogo, e' +
     ' o defeito era UMA linha: toda a latina dentro de $...$ ia para a Symbol, quando os gregos' +
     ' já eram tratados pela tabela do léxico dez linhas acima. Uma segunda regra a fazer o' +
     ' mesmo trabalho para quem não lhe pertencia. E as duas metades: os trocados têm de ser' +
     ' poucos e os legítimos muitos — se ambos fossem zero, o modo matemático não compunha nada',
     trocados < 800 && legitimos > 1000)
}

console.log('\n§N6  E as linhas JUSTIFICADAS acabam todas na margem.\n')
{
  /* A JUSTIFICAÇÃO mede-se onde ela se vê: onde a linha ACABA. Toma-se a posição do último
   * pedaço e soma-se a sua largura — e tem de dar a margem direita, para todas as linhas que
   * levam `Tw` (as justificadas). As que não levam são os fins de parágrafo, os títulos e as
   * tabelas, e essas NÃO devem acabar na margem.
   *
   * E MEDE-SE COM A TABELA DE CADA FONTE. À primeira usei as larguras da regular para tudo, e
   * 27 linhas «falharam» — todas com negrito. O tradutor estava certo e o MEDIDOR errado:
   * medir o negrito com a régua da regular é o mesmo defeito que eu andava a caçar no código,
   * cometido dentro da ferramenta que o caça. */
  const larg = {}
  for (const [tag, macro] of [['/F1', 'SPLINE_REG'], ['/F2', 'SPLINE_NEG']]) {
    const src = `#define _GNU_SOURCE\n#include <stdio.h>\n#include "spline.h"\n` +
      `static int w2u(int g){switch(g){case 0x85:return 0x2026;case 0x91:return 0x2018;` +
      `case 0x92:return 0x2019;case 0x93:return 0x201C;case 0x94:return 0x201D;` +
      `case 0x96:return 0x2013;case 0x97:return 0x2014;default:return g;}}\n` +
      `int main(void){Ttf t;if(!spline_abre_alguma(&t,${macro},SPLINE_NCAND,NULL))return 1;` +
      `for(int c=32;c<256;c++){int g=ttf_glifo(&t,w2u(c));if(g)` +
      `printf("%d %ld\\n",c,(long)ttf_avanco(&t,g)*1000/t.upem);}return 0;}`
    fs.writeFileSync('/tmp/n_w.c', src)
    larg[tag] = {}
    try {
      execSync(`cc -O2 -std=gnu99 -I${JSON.stringify(path.join(RAIZ,'lib'))} /tmp/n_w.c -o /tmp/n_w`, { stdio: 'pipe' })
      for (const l of execSync('/tmp/n_w', { encoding: 'utf8' }).trim().split('\n'))
        if (l) { const [a, b] = l.split(' '); larg[tag][Number(a)] = Number(b) }
    } catch (e) {}
  }
  larg['/F3'] = {}
  for (let c = 32; c < 256; c++) larg['/F3'][c] = 549

  compoe('papers/dualsort.tex', '/tmp/n_j.pdf')
  const d = fs.readFileSync('/tmp/n_j.pdf', 'latin1')
  const pgs = [...d.matchAll(/stream\n([\s\S]*?)endstream/g)].map((m) => m[1]).filter((x) => x.includes('Tj'))
  const pat = /BT (\/F\d) (\d+) Tf ([\d.]+) ([\d.]+) Td(?: ([\d.]+) Tw)? \(([\s\S]*?)\) Tj ET/g
  const MARGEM = 64 + 467
  let just = 0, naMargem = 0, semTw = 0, semTwNaMargem = 0, pior = 0
  for (const s of pgs) {
    const L = new Map()
    for (const m of s.matchAll(pat)) {
      const y = Math.round(Number(m[4]) * 10) / 10
      if (!L.has(y)) L.set(y, [])
      L.get(y).push({ f: m[1], corpo: Number(m[2]), x: Number(m[3]), tw: m[5], txt: m[6] })
    }
    for (const v of L.values()) {
      v.sort((a, b) => a.x - b.x)
      const u = v[v.length - 1]
      const t = u.txt.replace(/\\\(/g, '(').replace(/\\\)/g, ')')
      let w = 0
      for (const c of t) w += (larg[u.f] || {})[c.charCodeAt(0)] || 556
      w = w * u.corpo / 1000
      if (u.tw) w += Number(u.tw) * (t.split(' ').length - 1)
      const dif = Math.abs(u.x + w - MARGEM)
      /* O CRITERIO MUDOU COM A CORRECCAO, e o medidor tinha de acompanhar: desde que o Tw se
       * escreve SEMPRE — mesmo a zero, para não persistir — «ter Tw» deixou de distinguir a
       * linha justificada da que não é. Agora distingue-se por Tw > 0. É a asserção a acusar
       * uma mudança legítima, e não um defeito: o que ela media deixou de existir. */
      if (v.some((p) => p.tw && Number(p.tw) > 0)) { just++; if (dif < 0.6) naMargem++; if (dif > pior) pior = dif }
      else { semTw++; if (dif < 0.6) semTwNaMargem++ }
    }
  }
  console.log('      justificadas (com Tw): ' + just + ', ' + naMargem + ' acabam na margem, pior desvio ' + pior.toFixed(2) + 'pt')
  console.log('      sem Tw (fim de parágrafo, títulos): ' + semTw + ', ' + semTwNaMargem + ' na margem')
  /* as duas metades: as justificadas TÊM de acabar na margem, e as outras NÃO — se todas
   * acabassem, não havia justificação nenhuma, havia texto forçado. */
  ok('todas as linhas justificadas acabam na margem, com desvio abaixo de 0,05 pt — e as que' +
     ' NÃO levam Tw (fins de parágrafo, títulos, tabelas) não acabam, que é a outra metade: se' +
     ' todas acabassem não havia justificação, havia texto forçado. E mede-se com a tabela de' +
     ' CADA fonte: à primeira usei a da regular para tudo e 27 linhas «falharam», todas com' +
     ' negrito — o tradutor estava certo e o MEDIDOR errado, a cometer dentro da ferramenta o' +
     ' mesmo defeito que ela caça',
     just > 50 && naMargem === just && pior < 0.05 && semTw > 0 && semTwNaMargem < semTw / 2)
}

console.log('\n§N7  RESÍDUO 0: nenhuma palavra invade a seguinte, em nenhum documento.\n')
{
  /* A MEDIDA DEFINITIVA, e é a da curva de Hilbert: compor é π (a sequência 1D de glifos posta
   * em 2D na página) e extrair é ν. O critério não é a minha soma bater com a minha soma —
   * isso é tautologia, e escrevi-a três vezes hoje. É a CAIXA que o leitor desenha contra a
   * caixa da palavra seguinte: dois caminhos que podem discordar.
   *
   * «cada passo anda um em um eixo — só esta enche o cubo SEM RASGAR». Uma invasão é um rasgo.
   *
   * E TEM DE SER ZERO, não pequeno: o erro do espaçamento SOMA, e um resíduo não-nulo num
   * sistema reversível não fica pequeno — cresce. É o caos, e a única defesa é o zero. */
  const DOCS2 = [['enredo','enredo.tex'], ['catalogo','catalogo.tex'], ['teoria','teoria.tex']]
  let total = 0, pares = 0
  console.log('      documento         pares      invadem   pior')
  for (const [n, f] of DOCS2) {
    compoe(f, `/tmp/n7_${n}.pdf`)
    try { execSync(`pdftotext -bbox /tmp/n7_${n}.pdf /tmp/n7_${n}.xml`, { stdio: 'pipe' }) } catch (e) { continue }
    const d = fs.readFileSync(`/tmp/n7_${n}.xml`, 'utf8')
    const ws = [...d.matchAll(/<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)"/g)]
      .map((m) => [Number(m[1]), Number(m[2]), Number(m[3])])
    let inv = 0, tot = 0, pior = 0
    for (let i = 0; i < ws.length - 1; i++) {
      if (Math.abs(ws[i][1] - ws[i + 1][1]) > 0.5) continue
      tot++
      const d2 = ws[i][2] - ws[i + 1][0]
      if (d2 > 0.5) { inv++; if (d2 > pior) pior = d2 }
    }
    total += inv; pares += tot
    console.log('      ' + n.padEnd(17) + String(tot).padEnd(11) + String(inv).padEnd(9) + pior.toFixed(2) + 'pt')
  }
  ok('nenhuma palavra invade a seguinte, em nenhum dos três documentos — resíduo ZERO em ' +
     pares + ' pares. E é medido contra o LEITOR, não contra a minha aritmética: a caixa que ele' +
     ' desenha contra a caixa da palavra seguinte, dois caminhos que podem discordar. Foram' +
     ' precisos dois defeitos para lá chegar — o Tw a persistir no stream (1403 invasões) e a' +
     ' Symbol com 549 fixo (642, todas com símbolo). E tem de ser ZERO e não pequeno: o erro do' +
     ' espaçamento SOMA, e num sistema reversível um resíduo não-nulo não fica pequeno, cresce',
     pares > 100000 && total === 0)
}

console.log('\n=== SEM CHUTE ===============================================================')
console.log('  O `largura()` tinha um `return 556` para quando não conhecia o glifo, e ele')
console.log('  disparava 106 vezes no catálogo — calado.')
console.log('')
console.log('  A causa: o PDF escreve em WinAnsi, onde o travessão é 151. Mas o `cmap` de uma')
console.log('  TTF é indexado por UNICODE, e lá ele é U+2014. Eu procurava o 151 no cmap, não')
console.log('  achava, e chutava — QUANDO A FONTE TINHA O GLIFO. Perguntava pelo número errado.')
console.log('')
console.log('  E como espaçar SOMA, cada travessão desalinhava a linha inteira a partir dali.')
console.log('  O «uns espaços ficaram maiores» era eu a NÃO CONTAR 444 milésimos de cada vez.')
if (falhas) { console.log('\n  FALHAS: ' + falhas + '\n'); process.exit(1) }
console.log('\n  RESÍDUO 0 — nenhuma largura inventada, e nenhuma sobreposição.\n')
