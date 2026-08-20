/* escala.js — A ESCALA TIPOGRÁFICA SAI DO estilo.tex, E É φ^(1/3).
 *
 * O Aarão: «sim, e o design — aqui não mudou nada ainda, mesmo texto grosso tosco.»
 *
 * Tinha razão, e o que faltava não eram as réguas nem as caixas: era a TIPOGRAFIA. O tradutor
 * usava `CORPO 10` e `ENTRE 14` — dois números escritos à mão — e o design tem uma ESCALA,
 * declarada no `estilo.tex` em sete degraus:
 *
 *      7,62 · 8,94 · 10,50 · 12,33 · 14,47 · 16,99 · 23,42
 *
 * E ela não é uma lista de tamanhos escolhidos. As razões entre degraus consecutivos são
 *
 *      1,1732 · 1,1745 · 1,1743 · 1,1736 · 1,1742          e   φ^(1/3) = 1,1740
 *
 * A escala é geométrica na RAIZ CÚBICA DO ÁUREO. E a entrelinha é 1,4497 do corpo em todos os
 * sete — enquanto o tradutor usava 14/10 = 1,4, que é perto e é outra coisa. Daí o texto sair
 * grosso: 10 onde o design manda 10,50, sem hierarquia e com menos ar entre linhas.
 *
 * E a correção é a mesma das cores: LER da fonte. Escrever «10,50» no tradutor seria a
 * referência à mão outra vez — mudava-se a escala no estilo e o PDF não mudava.
 *
 *   §T1  a escala do estilo.tex é geométrica, e a razão é φ^(1/3)
 *   §T2  a entrelinha é uma razão CONSTANTE do corpo — a mesma nos sete
 *   §T3  e o PDF usa os degraus da escala, não 10 e 14
 *   §T4  o controlo: mudada a escala no estilo.tex, o PDF muda — e volta
 *
 *   node tests/escala.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TEX = path.join(RAIZ, 'tests', 'tex')
const ESTILO = path.join(RAIZ, 'estilo.tex')
let falhas = 0
const ok = (m, c) => {
  console.log('      ' + (c ? '✓' : '✗') + '  ' + m)
  /* a UNIDADE conta-se: sem esta linha a bateria soma UMA unidade grossa (o exit)
   * por este ficheiro inteiro, e as asserções finas desaparecem do total. */
  console.log('#UNIT ' + (c ? 'ok' : 'falha') + ' ' + m)
  if (!c) falhas++
}

if (!fs.existsSync(TEX)) { console.log('\n  tests/tex não compilado.  NÃO MEDIU.\n'); process.exit(2) }

/* O estilo.tex escreve DECIMAIS, e um decimal escrito com e casas é um INTEIRO
 * sobre 10^e — não um double. Lê-se assim: `n` é o inteiro e `e` as casas que a
 * FONTE escreveu. A meia-unidade da última casa (o que o autor do estilo podia
 * ter querido e arredondou) é a única folga que existe, e sai da fonte: não é
 * um número escolhido por mim. Guarda-se em escala DUPLA — 2n ± 1 sobre 2·10^e
 * — para que a meia-unidade seja inteira. */
function lerDecimal (txt) {
  const [ip, fp = ''] = txt.split('.')
  return { n: BigInt(ip + fp), e: fp.length }
}
/* a caixa do valor v = n/10^e, em vigésimos-de-última-casa: [lo, hi] / (2·10^e) */
const caixa = (d) => ({ lo: 2n * d.n - 1n, hi: 2n * d.n + 1n, den: 2n * 10n ** BigInt(d.e) })
/* p/q < φ^(1/3)  ⟺  2p³ < (1+√5)q³   (p, q > 0) — inteiro, sem raiz calculada */
const menorQueRaiz3Phi = (p, q) => { const d = 2n * p ** 3n - q ** 3n; return d <= 0n || d * d < 5n * q ** 6n }
const maiorQueRaiz3Phi = (p, q) => { const d = 2n * p ** 3n - q ** 3n; return d > 0n && d * d > 5n * q ** 6n }
/* a/b < c/d com b,d > 0 */
const fracMenor = (a, b, c, d) => a * d < c * b

function degraus () {
  const s = fs.readFileSync(ESTILO, 'utf8')
  return [...s.matchAll(/\\fontsize\{([\d.]+)\}\{([\d.]+)\}/g)]
    .map((m) => ({
      corpo: parseFloat(m[1]), entre: parseFloat(m[2]),
      corpoD: lerDecimal(m[1]), entreD: lerDecimal(m[2]),
    }))
    .sort((a, b) => a.corpo - b.corpo)
}
function compoe (fonte, saida) {
  try { execSync(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(RAIZ, fonte))} ${JSON.stringify(saida)}`,
                 { stdio: 'pipe', cwd: path.dirname(TEX), timeout: 180000 }); return true }
  catch (e) { return false }
}
const tamanhos = (f) => {
  /* auditoria 14/08: o dialecto não tem Tf — o corpo é a ESCALA do cm × upem
   * (s·1000), exata a 2 casas */
  const d = fs.readFileSync(f, 'latin1')
  return [...new Set([...d.matchAll(/([\d.]+) 0 0 [\d.]+ [\-\d.]+ [\-\d.]+ cm\s*\/G/g)]
    .map((m) => Math.round(+m[1] * 1000 * 100) / 100))].sort((a, b) => a - b)
}

console.log('\n=== A ESCALA TIPOGRÁFICA SAI DO estilo.tex, E É φ^(1/3) ====================\n')

const D = degraus()
const PHI = (1 + Math.sqrt(5)) / 2
const RAIZ3 = Math.pow(PHI, 1 / 3)

console.log('§T1  A escala é GEOMÉTRICA, e a razão é φ^(1/3).\n')
{
  /* auditoria 20/08: o `|r − φ^(1/3)| > 0,002` era uma régua ESCRITA À MÃO, e em
   * doubles. A régua do objecto é outra e já cá estava: o estilo escreve os
   * degraus com duas casas, logo cada degrau é um inteiro ± meia centésima. A
   * razão de dois degraus vive então na caixa [ (2b−1)/(2a+1), (2b+1)/(2a−1) ],
   * e a asserção é que φ^(1/3) CAI DENTRO dela — decidida em BigInt, sem raiz
   * cúbica calculada e sem uma vírgula em lado nenhum. É mais apertada que o
   * 0,002 nos degraus grandes e mais larga nos pequenos, porque é a folga que
   * a FONTE tem, e não a que eu escolhi. */
  /* a lei, como função dos degraus: dá para reaplicar a degraus PERTURBADOS, e é
   * assim que o gume deixa de ser um sed meu e passa a ser medida */
  const leiT1 = (cs) => {
    let fora = 0
    for (let i = 1; i < cs.length - 1; i++) {      /* o último salta um degrau: 23,42 = φ^(2/3)·16,99 */
      const a = caixa(cs[i - 1]), b = caixa(cs[i])
      if (!(menorQueRaiz3Phi(b.lo * a.den, a.hi * b.den) &&
            maiorQueRaiz3Phi(b.hi * a.den, a.lo * b.den))) fora++
    }
    return fora
  }
  const cs = D.map((d) => d.corpoD)
  const pares = D.length - 2
  console.log('      degrau   corpo    razão ao anterior   caixa da razão contém φ^(1/3)')
  for (let i = 0; i < D.length; i++) {
    const r = i ? D[i].corpo / D[i - 1].corpo : 0
    const um = (i && i < D.length - 1)
      ? (leiT1([cs[i - 1], cs[i], cs[i]]) === 0 ? 'sim' : 'NÃO') : ''
    console.log('      ' + String(i).padEnd(8) + String(D[i].corpo).padEnd(8) +
                (i ? r.toFixed(4) : '—').padEnd(20) + um)
  }
  const fora = leiT1(cs)
  /* O GUME, medido e não afirmado: de quantas unidades da ÚLTIMA CASA é preciso
   * mexer um degrau para a lei cair? Se fosse ∞ a lei não podia falhar. */
  let gumeT1 = Infinity, ondeT1 = -1
  for (let i = 0; i < cs.length; i++) {
    for (let k = 1; k <= 20 && k < gumeT1; k++) {
      for (const sg of [1n, -1n]) {
        const p = cs.map((d, j) => (j === i ? { n: d.n + sg * BigInt(k), e: d.e } : d))
        if (leiT1(p) > 0) { gumeT1 = k; ondeT1 = i }
      }
    }
  }
  console.log('\n      φ^(1/3) ≈ ' + RAIZ3.toFixed(4) + '   —  e cai na caixa dos ' + pares +
              ' pares, ' + (fora === 0 ? 'em todos' : 'fora em ' + fora))
  console.log('      GUME: bastam ' + gumeT1 + ' centésimas no degrau ' + ondeT1 + ' para a lei cair')
  /* NÃO se afirma que a escala é bonita: afirma-se que a razão é CONSTANTE e igual a um número
   * que o sistema já tem. Se fosse constante e igual a outra coisa, isto falhava — e falhava
   * bem, porque aí não seria φ. */
  ok('a escala é geométrica e a razão é φ^(1/3) em ' + pares + ' pares consecutivos: a razão de' +
     ' cada par, com a folga das duas casas que o estilo.tex escreve, contém φ^(1/3) — e a' +
     ' decisão é em INTEIROS (2p³ vs (1+√5)q³), sem raiz cúbica e sem limiar meu. E a lei PODE' +
     ' cair: ' + gumeT1 + ' centésimas no degrau ' + ondeT1 + ' chegam. Não se afirma que a escala' +
     ' é bonita: afirma-se que a razão é CONSTANTE e igual a um número que o sistema já tem',
     fora === 0 && pares >= 4 && gumeT1 < Infinity)
}

console.log('\n§T2  A entrelinha é uma razão CONSTANTE do corpo — a mesma nos sete.\n')
{
  /* auditoria 20/08: «dispersão < 0,002» e «|média − 1,4| > 0,04» eram duas
   * réguas minhas sobre uma média de doubles. O que se quer dizer — «é a MESMA
   * razão nos sete» — diz-se sem régua nenhuma: cada degrau dá uma CAIXA para
   * entre/corpo (o inteiro escrito ± meia centésima), e a razão é a mesma sse
   * as oito caixas se INTERSECTAM. É um facto sobre racionais, e é ou não é.
   * E «não é 1,4» passa a ser: 7/5 está FORA dessa intersecção. */
  const rs = D.map((d) => d.entre / d.corpo)
  /* a intersecção das caixas, como função dos pares (corpo, entre) */
  const leiT2 = (ds) => {
    let LN = 0n, LD = 1n, HN = 1n, HD = 0n          /* [LN/LD, HN/HD], começa em [0, ∞) */
    for (const d of ds) {
      const c = caixa(d.corpoD), e = caixa(d.entreD)
      const ln = e.lo * c.den, ld = c.hi * e.den    /* mínimo possível de entre/corpo */
      const hn = e.hi * c.den, hd = c.lo * e.den    /* máximo possível */
      if (fracMenor(LN, LD, ln, ld)) { LN = ln; LD = ld }
      if (HD === 0n || fracMenor(hn, hd, HN, HD)) { HN = hn; HD = hd }
    }
    return { LN, LD, HN, HD, cruzam: fracMenor(LN, LD, HN, HD) }
  }
  const I = leiT2(D)
  const cruzam = I.cruzam
  const foraDoSete = fracMenor(7n, 5n, I.LN, I.LD) || fracMenor(I.HN, I.HD, 7n, 5n)
  /* o GUME, medido: quantas centésimas de entrelinha esvaziam a intersecção */
  let gumeT2 = Infinity, ondeT2 = -1
  for (let i = 0; i < D.length; i++) {
    for (let k = 1; k <= 20 && k < gumeT2; k++) {
      for (const sg of [1n, -1n]) {
        const p = D.map((d, j) => (j === i
          ? { ...d, entreD: { n: d.entreD.n + sg * BigInt(k), e: d.entreD.e } } : d))
        if (!leiT2(p).cruzam) { gumeT2 = k; ondeT2 = i }
      }
    }
  }
  console.log('      entrelinha/corpo: ' + rs.map((r) => r.toFixed(4)).join('  '))
  console.log('      as ' + D.length + ' caixas intersectam-se: ' + cruzam + ' — e a intersecção é' +
              ' [' + (Number(I.LN) / Number(I.LD)).toFixed(6) + ', ' + (Number(I.HN) / Number(I.HD)).toFixed(6) + ']')
  console.log('      e o tradutor usava 14/10 = 1,4 — que está ' + (foraDoSete ? 'FORA' : 'dentro') +
              ' dessa intersecção')
  console.log('      GUME: ' + gumeT2 + ' centésimas na entrelinha do degrau ' + ondeT2 + ' esvaziam-na')
  ok('a entrelinha é a MESMA razão do corpo nos ' + D.length + ' degraus — as caixas que as duas' +
     ' casas do estilo.tex dão a entre/corpo têm intersecção NÃO VAZIA, logo existe uma razão' +
     ' única compatível com os oito. E 7/5 está fora dela: o 1,4 do tradutor é perto e é outra' +
     ' coisa. Tudo em racionais exactos, sem média e sem limiar — e a lei PODE cair: ' + gumeT2 +
     ' centésimas na entrelinha do degrau ' + ondeT2 + ' chegam',
     cruzam && foraDoSete && gumeT2 < Infinity)
}

console.log('\n§T3  E o PDF usa os degraus da escala, não 10 e 14.\n')
{
  // ARREDONDA-SE COMO O C ARREDONDA, e não de outra maneira. Eu tinha escrito
  // Math.round(corpo + 0.5) — que arredonda DUAS vezes: 12,33 vira 12,83 e depois 13, quando
  // o (int)(x + 0.5) do tradutor dá 12. A asserção acusou e o defeito era do medidor, não do
  // código: comparar contra uma régua que não é a que o objecto usa acusa sempre.
  /* auditoria 14/08: o dialecto emite o corpo EXATO na escala do cm (o Tf
   * inteiro morreu) — compara-se contra os degraus VERDADEIROS e contra os
   * corpos derivados que o compositor constrói por produto cruzado de
   * degraus (corpo_exp_m: corpo·d_a/d_b, pares (0,4) e (1,3)) — a régua do
   * objeto, lida do estilo, sem um número de mão */
  const mil = D.map((d) => Math.round(d.corpo * 1000))
  /* o corpo do TEXTO não é degrau: é o \normalsize da CLASSE — lê-se de
   * size{N}.clo + latex.ltx (a lição do avalia_macros), não se afirma */
  function corpoDaClasse () {
    try {
      const mestre = fs.readFileSync(path.join(RAIZ, 'livro.tex'), 'utf8')
      const sz = (/(\d+)pt/.exec((/\\documentclass\[([^\]]*)\]/.exec(mestre) || [])[1] || '') || [])[1]
      if (!sz) return null
      const sh2 = (c) => require('child_process').execSync(c, { encoding: 'utf8', stdio: 'pipe' })
      const clo = sh2(`kpsewhich size${sz}.clo`).trim()
      const cmd = (/\\@setfontsize\s*\\normalsize\s*(\\@[a-z]+)/.exec(fs.readFileSync(clo, 'latin1')) || [])[1]
      const ltx = sh2('kpsewhich latex.ltx').trim()
      const rx = new RegExp('\\\\def' + cmd.replace(/\\/g, '\\\\') + '\\{([\\d.]+)\\}')
      const v = (rx.exec(fs.readFileSync(ltx, 'latin1')) || [])[1]
      return v ? Math.round(parseFloat(v) * 1000) : null
    } catch (e) { return null }
  }
  const daClasse = corpoDaClasse()
  if (daClasse) mil.push(daClasse)
  const pares = [[mil[0], mil[4]], [mil[1], mil[3]]]
  const admissiveis = []
  for (const b0 of mil) {
    admissiveis.push(b0)
    for (const [a, b] of pares) admissiveis.push(Math.floor(b0 * a / b))
  }
  /* auditoria 20/08: aqui estava `Math.abs(t - m/1000) <= 0.02` — mais uma régua
   * escrita à mão, e larga: MEDIDO, seis dos oito tamanhos batem EXACTAMENTE e os
   * outros dois falham por 1 e por 4 milésimos. Vinte milésimos de folga é cinco
   * vezes o que o objecto pede, e uma folga assim não mede: absorve.
   *
   * A comparação é INTEIRA. O PDF escreve a escala do cm com seis casas
   * (`0.023420`), logo o corpo em pontos é exacto ao MILÉSIMO, e os degraus do
   * estilo.tex já entram aqui em milésimos (`mil`). Não há vírgula a comparar.
   *
   * E a folga sai da FONTE, como no §T1: o estilo escreve DUAS casas, logo cada
   * degrau é exacto a ±5 milésimos. Um tamanho DERIVADO — o produto cruzado de
   * dois degraus que o compositor faz — herda essa meia-casa, e é essa a única
   * folga que existe. Um degrau NÃO derivado não tem folga nenhuma: bate ou não. */
  /* e a folga DERIVA-SE das casas que o estilo escreve, em vez de ser escrita: com
   * `e` decimais, a granularidade em milésimos é 1000/10^e e a meia casa é metade.
   * Assim, se o estilo passar a três decimais, a folga aperta sozinha — e a frase
   * da asserção continua verdadeira sem ninguém lhe tocar. */
  const CASAS = Math.max(...D.map((d) => d.corpoD.e))
  const FOLGA_DERIVADO = 1000 / Math.pow(10, CASAS) / 2   /* 2 casas → 5 milésimos */
  const base = new Set(mil)                        /* os degraus, sem derivação */
  const milDe = (t) => Math.round(t * 1000)        /* t tem 2 casas: exacto */
  const exacto = (t) => base.has(milDe(t))
  const perto = (t) => exacto(t) ||
    admissiveis.some((m) => Math.abs(milDe(t) - m) <= FOLGA_DERIVADO)
  let fora = 0, docs = 0, exactos = 0, derivados = 0, piorDerivado = 0
  console.log('      documento         tamanhos no PDF')
  for (const [n, f] of [['computacional', 'papers/arquitetura.tex'], ['teoria', 'teoria.tex']]) {
    if (!compoe(f, `/tmp/t_${n}.pdf`)) continue
    docs++
    const ts = tamanhos(`/tmp/t_${n}.pdf`)
    for (const t of ts) {
      if (!perto(t)) fora++
      if (exacto(t)) exactos++; else {
        derivados++
        let d = Infinity
        for (const m of admissiveis) d = Math.min(d, Math.abs(milDe(t) - m))
        if (d > piorDerivado) piorDerivado = d
      }
    }
    console.log('      ' + n.padEnd(17) + ts.join(', ') + ' pt')
  }
  console.log('      degraus (exatos): ' + D.map((d) => d.corpo).join(', ') +
              ' — e os derivados por produto cruzado dos pares (0,4) e (1,3)')
  console.log('      ' + exactos + ' tamanhos batem um degrau EXACTAMENTE (0 milésimos) · ' +
              derivados + ' são derivados, pior desvio ' + piorDerivado + ' milésimos' +
              ' (a folga é ' + FOLGA_DERIVADO + ' = meia casa das ' + CASAS +
              ' que o estilo.tex escreve, derivada e não escrita)')
  ok('todos os tamanhos que aparecem no PDF são degraus da escala do estilo.tex, e nenhum vem' +
     ' de fora dela. Antes eram 10, 12 e 15 — três números escritos à mão, e o do meio nem' +
     ' sequer da mesma família. E a comparação é INTEIRA, em milésimos: ' + exactos + ' batem um' +
     ' degrau com desvio ZERO e ' + derivados + ' são derivados (produto cruzado de dois degraus),' +
     ' dentro da meia-casa que as duas decimais do estilo dão. O `<= 0,02` que aqui estava era' +
     ' vinte milésimos — cinco vezes o pior desvio real, e uma folga que absorve em vez de medir',
     docs === 2 && fora === 0 && exactos > 0 && piorDerivado <= FOLGA_DERIVADO)
}

console.log('\n§T4  O CONTROLO: mudada a escala no estilo.tex, o PDF muda — e volta.\n')
{
  const original = fs.readFileSync(ESTILO, 'utf8')
  let antes = [], depois = [], voltou = [], mordeu = false
  try {
    compoe('papers/arquitetura.tex', '/tmp/t_a.pdf'); antes = tamanhos('/tmp/t_a.pdf')
    const alvo = '\\fontsize{10.50}{15.22}'
    mordeu = original.includes(alvo)
    if (mordeu) {
      fs.writeFileSync(ESTILO, original.replace(alvo, '\\fontsize{19.00}{27.55}'))
      compoe('papers/arquitetura.tex', '/tmp/t_b.pdf'); depois = tamanhos('/tmp/t_b.pdf')
    }
  } finally {
    fs.writeFileSync(ESTILO, original)                 /* devolve-se SEMPRE */
  }
  compoe('papers/arquitetura.tex', '/tmp/t_c.pdf'); voltou = tamanhos('/tmp/t_c.pdf')
  const devolvido = fs.readFileSync(ESTILO, 'utf8') === original
  const mudou = JSON.stringify(antes) !== JSON.stringify(depois)
  const igual = JSON.stringify(antes) === JSON.stringify(voltou)
  console.log('      com a escala original:  ' + antes.join(', '))
  console.log('      com o corpo em 19pt:    ' + depois.join(', '))
  console.log('      revertido o estilo:     ' + voltou.join(', '))
  ok('mudado o corpo NO estilo.tex, os tamanhos do PDF mudam; revertido, voltam aos mesmos — e' +
     ' o ficheiro voltou ao original. É o que dá valor ao resto: sem esta metade, «o PDF usa a' +
     ' escala» passava com tamanhos que viessem de qualquer lado. Mexe-se na causa e vê-se a' +
     ' consequência', mordeu && mudou && igual && devolvido)
}

console.log('\n=== A ESCALA ================================================================')
console.log('  Não faltavam as réguas nem as caixas: faltava a TIPOGRAFIA. O tradutor usava')
console.log('  CORPO 10 e ENTRE 14 — dois números escritos à mão — e o design tem uma escala de')
console.log('  sete degraus, declarada no estilo.tex.')
console.log('')
console.log('    7,62 · 8,94 · 10,50 · 12,33 · 14,47 · 16,99 · 23,42')
console.log('')
console.log('  E ela é GEOMÉTRICA na raiz cúbica do áureo: as razões consecutivas são 1,1732 ·')
console.log('  1,1745 · 1,1743 · 1,1736 · 1,1742, e φ^(1/3) = 1,1740. A entrelinha é 1,4497 do')
console.log('  corpo nos sete — e o tradutor usava 1,4, que é perto e é outra coisa.')
console.log('')
console.log('  A correção é a mesma das cores: LER da fonte. Escrever «10,50» no tradutor era a')
console.log('  referência à mão outra vez — mudava-se a escala no estilo e o PDF não mudava.')
if (falhas) { console.log('\n  FALHAS: ' + falhas + '\n'); process.exit(1) }
console.log('\n  RESÍDUO 0 — a escala sai da fonte, e chega ao PDF.\n')
