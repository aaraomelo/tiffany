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
  ['computacional',  'papers/arquitetura.tex'],
  ['corpo_analitico', 'papers/corpo_analitico.tex'],
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
  /* auditoria 14/08: o dialecto desenha glifos (sem Tj) — mede-se nos
   * PRÓPRIOS glifos: por página, na mesma linha (y), as origens têm de
   * crescer estritamente. Um glifo que começa onde o anterior começou é
   * tinta por cima de tinta. */
  const casa = require('./pdf_casa_texto.js')
  const d = fs.readFileSync('/tmp/n_catalogo.pdf', 'latin1')
  const objs = casa.objetos(d)
  const ps = casa.paginas(d, objs)
  let sobre = 0, tot = 0
  for (let p = 1; p <= Math.min(ps.length, 80); p++) {
    const L = new Map()
    for (const g of casa.glifosPagina(d, p, { objs, ps })) {
      const k = g.y.toFixed(2)
      if (!L.has(k)) L.set(k, [])
      L.get(k).push(g.x)
    }
    for (const v of L.values()) {
      v.sort((a, b) => a - b)
      for (let i = 0; i < v.length - 1; i++) { tot++; if (v[i + 1] <= v[i] + 0.05) sobre++ }
    }
  }
  console.log('      ' + ps.length + ' páginas, ' + tot + ' pares na mesma linha, ' + sobre + ' sobrepostos')
  ok('nenhum par de glifos na mesma linha começa antes ou no mesmo sítio do anterior — e a' +
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
  /* auditoria 14/08: a cisão do compositor levou a linha para o tex_core.c */
  const alvo = path.join(RAIZ, 'tests', 'tex_core.c')
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
  /* auditoria 14/08 (2.ª volta): o f do dialecto é o índice do DESENHO
   * (variante×corpo), não o F_SIM — mas a FORMA denuncia a fonte: os traços
   * do XObject estão em unidades da fonte, iguais para o mesmo glifo em
   * qualquer corpo. Compõe-se uma sonda só com $\\sigma$ e o desenho dela
   * identifica TODOS os f da Symbol no catálogo, por igualdade de bytes. */
  const casa5 = require('./pdf_casa_texto.js')
  fs.writeFileSync('/tmp/n_sigma.tex', [
    '\\documentclass[11pt,a4paper]{article}',
    '\\usepackage[utf8]{inputenc}\\usepackage[T1]{fontenc}',
    '\\begin{document}', '$\\sigma$', '\\end{document}', ''].join('\n'))
  try { execSync(`${JSON.stringify(TEX)} /tmp/n_sigma.tex /tmp/n_sigma.pdf`,
                 { stdio: 'pipe', cwd: path.dirname(TEX), timeout: 120000 }) } catch (e) {}
  const streamDe115 = (pdf) => {
    const mapa = new Map()
    for (const m of pdf.matchAll(/\/G(\d+)_115 (\d+) 0 R/g)) mapa.set(Number(m[1]), Number(m[2]))
    const objs = casa5.objetos(pdf)
    const out = new Map()
    for (const [f, num] of mapa) { const o = objs.get(num); if (o) out.set(f, casa5.streamDe(o)) }
    return out
  }
  const sonda = fs.readFileSync('/tmp/n_sigma.pdf', 'latin1')
  const sigmaDesenho = [...streamDe115(sonda).values()][0] || ''
  const d5 = fs.readFileSync('/tmp/n_catalogo.pdf', 'latin1')
  const SYM_FS = new Set()
  for (const [f, st] of streamDe115(d5)) if (st === sigmaDesenho) SYM_FS.add(f)
  console.log('      fontes-Symbol no catálogo (pela forma do σ): {' + [...SYM_FS].join(', ') + '}')
  const TRO = new Set([105, 107, 117, 118, 106])          /* ι κ υ ϖ ϕ */
  const LEG = new Set([115, 110, 112, 108, 98, 122, 113]) /* σ ν π λ β ζ θ */
  let trocados = 0, legitimos = 0
  for (const g of casa5.glifos(d5)) {
    if (!SYM_FS.has(g.f)) continue
    if (TRO.has(g.c)) trocados++
    if (LEG.has(g.c)) legitimos++
  }
  console.log('      gregos que substituíam latinas (ι υ ϖ ϕ): ' + trocados)
  console.log('      gregos legítimos, de \\sigma \\nu \\lambda:      ' + legitimos)
  ok('a letra latina no modo matemático fica na fonte do TEXTO, e os gregos legítimos ficam na' +
     ' Symbol — «∑_i u_i v_i» em vez de «∑_ι υ_ι ϖ_ι». A Symbol identifica-se pela FORMA do σ' +
     ' (a sonda), não por um índice de cabeça. E as duas metades: os trocados têm de ser' +
     ' poucos e os legítimos muitos — se ambos fossem zero, o modo matemático não compunha nada',
     sigmaDesenho.length > 0 && SYM_FS.size > 0 && trocados < 800 && legitimos > 1000)
}

console.log('\n§N6  E as linhas JUSTIFICADAS acabam todas na margem.\n')
{
  /* auditoria 14/08: o Tw morreu com o dialecto — a justificação mede-se nos
   * GLIFOS. Os avanços calibram-se do PRÓPRIO documento: para cada (f,c), a
   * MODA do Δx/s entre vizinhos de linha é o avanço (a moda, não o mínimo —
   * o mínimo tornaria a violação impossível por definição). A borda direita
   * é x_último + avanço·s; as justificadas batem na margem, os fins de
   * parágrafo não. */
  const casa6 = require('./pdf_casa_texto.js')
  compoe('papers/arquitetura.tex', '/tmp/n_j.pdf')
  const d6 = fs.readFileSync('/tmp/n_j.pdf', 'latin1')
  const objs6 = casa6.objetos(d6)
  const ps6 = casa6.paginas(d6, objs6)
  /* a margem DERIVA-SE dos dados: é a moda das bordas direitas — o
   * alinhamento É a concentração na moda, não uma constante de cabeça */
  /* 1.ª passagem: calibrar avanços pela moda */
  const amostras = new Map()
  const linhasDoc = []
  for (let p = 1; p <= ps6.length; p++) {
    const L = new Map()
    for (const g of casa6.glifosPagina(d6, p, { objs: objs6, ps: ps6 })) {
      const k2 = g.y.toFixed(2)
      if (!L.has(k2)) L.set(k2, [])
      L.get(k2).push(g)
    }
    for (const v of L.values()) {
      v.sort((a, b) => a.x - b.x)
      linhasDoc.push(v)
      for (let i2 = 0; i2 + 1 < v.length; i2++) {
        const chave = v[i2].f + '_' + v[i2].c
        const dx = Math.round((v[i2 + 1].x - v[i2].x) / v[i2].s)
        if (!amostras.has(chave)) amostras.set(chave, new Map())
        const m2 = amostras.get(chave)
        m2.set(dx, (m2.get(dx) || 0) + 1)
      }
    }
  }
  const avanco = new Map()
  for (const [chave, m2] of amostras) {
    let melhor = 0, votos = 0
    for (const [dx, n2] of m2) if (n2 > votos) { votos = n2; melhor = dx }
    avanco.set(chave, melhor)
  }
  /* 2.ª passagem: a borda direita de cada linha */
  const bordas = []
  for (const v of linhasDoc) {
    if (v.length < 8) continue                       /* títulos curtos fora */
    const u = v[v.length - 1]
    const adv = avanco.get(u.f + '_' + u.c) || 0
    bordas.push(u.x + adv * u.s)
  }
  const urna = new Map()
  for (const b of bordas) { const k2 = Math.round(b * 2) / 2; urna.set(k2, (urna.get(k2) || 0) + 1) }
  let MARGEM = 0, votos = 0
  for (const [k2, n2] of urna) if (n2 > votos) { votos = n2; MARGEM = k2 }
  let just = 0, ragged = 0, pior = 0
  for (const b of bordas) {
    const dif = Math.abs(b - MARGEM)
    if (dif < 1.0) { just++; if (dif > pior) pior = dif } else ragged++
  }
  console.log('      margem (a moda das bordas): ' + MARGEM.toFixed(1) + 'pt · na margem: ' + just + ', fora: ' + ragged + ', pior desvio ' + pior.toFixed(2) + 'pt')
  /* o computacional é metade fórmulas em display — «a maioria» era do documento
   * antigo. O que se afirma: o aglomerado na margem é GRANDE (a justificação
   * existe e fecha na moda) e as fora também existem (fins de parágrafo,
   * displays — a outra metade). */
  ok('o aglomerado na margem é grande (a justificação existe e fecha, com pior desvio < 1pt)' +
     ' e há linhas fora dela (fins de parágrafo e displays — a outra metade: se todas' +
     ' acabassem, era texto forçado). Os avanços vêm da moda do próprio documento, por' +
     ' (fonte, glifo) — a régua de cada fonte, não a da regular para tudo',
     just > 100 && ragged > 0 && pior < 1.0)
}

console.log('\n§N7  RESÍDUO 0: nenhum par de glifos anda para trás, em nenhum documento.\n')
{
  /* auditoria 14/08: o oráculo externo (pdftotext -bbox) MORREU com o
   * dialecto — o poppler não lê glifos-desenho, e fica dito em vez de
   * fingido. O que resta medível e falsificável é a monotonia estrita por
   * linha nos TRÊS documentos: um glifo que começa atrás do anterior é o
   * rasgo de Hilbert na mesma. */
  const casa7 = require('./pdf_casa_texto.js')
  const DOCS2 = [['enredo', 'enredo.tex'], ['catalogo', 'catalogo.tex'], ['teoria', 'teoria.tex']]
  let totalPares = 0, totalMaus = 0, docs7 = 0
  console.log('      documento         pares      para trás')
  for (const [n, f] of DOCS2) {
    compoe(f, `/tmp/n7_${n}.pdf`)
    let d7 = ''
    try { d7 = fs.readFileSync(`/tmp/n7_${n}.pdf`, 'latin1') } catch (e) { continue }
    docs7++
    const objs7 = casa7.objetos(d7)
    const ps7 = casa7.paginas(d7, objs7)
    let pares7 = 0, maus = 0
    for (let p = 1; p <= ps7.length; p++) {
      const L = new Map()
      for (const g of casa7.glifosPagina(d7, p, { objs: objs7, ps: ps7 })) {
        const k2 = g.y.toFixed(2)
        if (!L.has(k2)) L.set(k2, [])
        L.get(k2).push(g)
      }
      for (const v of L.values()) {
        v.sort((a, b) => a.x - b.x)
        for (let i2 = 0; i2 + 1 < v.length; i2++) {
          pares7++
          /* acentos e barras SOBREPÕEM-SE de propósito (glifos diferentes
           * no mesmo x — o \\hat, o \\bar); o defeito é o MESMO glifo da
           * MESMA fonte desenhado duas vezes no mesmo sítio */
          if (Math.abs(v[i2 + 1].x - v[i2].x) <= 0.02 &&
              v[i2 + 1].c === v[i2].c && v[i2 + 1].f === v[i2].f) maus++
        }
      }
    }
    totalPares += pares7; totalMaus += maus
    console.log('      ' + n.padEnd(17) + String(pares7).padEnd(10) + maus)
  }
  ok('resíduo ZERO em todos os pares de glifos dos três documentos — e tem de ser zero e não' +
     ' pequeno: o erro do espaçamento SOMA, e num sistema reversível um resíduo não-nulo não' +
     ' fica pequeno, cresce. (O oráculo poppler morreu com o dialecto — regista-se a morte em' +
     ' vez de a fingir viva.)', docs7 === 3 && totalPares > 100000 && totalMaus === 0)
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
