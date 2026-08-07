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
    const a = 'ttf_glifo(t, winansi_para_unicode(g))'
    mordeu = original.includes(a)
    if (mordeu) {
      fs.writeFileSync(alvo, original.replace(a, 'ttf_glifo(t, g)'))
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
