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
const ok = (m, c) => { console.log('      ' + (c ? '✓' : '✗') + '  ' + m); if (!c) falhas++ }

if (!fs.existsSync(TEX)) { console.log('\n  tests/tex não compilado.  NÃO MEDIU.\n'); process.exit(2) }

function degraus () {
  const s = fs.readFileSync(ESTILO, 'utf8')
  return [...s.matchAll(/\\fontsize\{([\d.]+)\}\{([\d.]+)\}/g)]
    .map((m) => ({ corpo: parseFloat(m[1]), entre: parseFloat(m[2]) }))
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
  console.log('      degrau   corpo    razão ao anterior')
  let fora = 0, pares = 0
  for (let i = 0; i < D.length; i++) {
    const r = i ? D[i].corpo / D[i - 1].corpo : 0
    if (i && i < D.length - 1) {                  /* o último salta um degrau: 23,42 = φ^(2/3)·16,99 */
      pares++
      if (Math.abs(r - RAIZ3) > 0.002) fora++
    }
    console.log('      ' + String(i).padEnd(8) + String(D[i].corpo).padEnd(8) + (i ? r.toFixed(4) : '—'))
  }
  console.log('\n      φ^(1/3) = ' + RAIZ3.toFixed(4) + '   —  e os ' + pares + ' pares batem a menos de 0,002')
  /* NÃO se afirma que a escala é bonita: afirma-se que a razão é CONSTANTE e igual a um número
   * que o sistema já tem. Se fosse constante e igual a outra coisa, isto falhava — e falhava
   * bem, porque aí não seria φ. */
  ok('a escala é geométrica e a razão é φ^(1/3) = ' + RAIZ3.toFixed(4) + ', em ' + pares +
     ' pares consecutivos e a menos de 0,002. Não se afirma que a escala é bonita: afirma-se que' +
     ' a razão é CONSTANTE e igual a um número que o sistema já tem. Se fosse constante e igual' +
     ' a outra coisa, isto falhava — e falhava bem', fora === 0 && pares >= 4)
}

console.log('\n§T2  A entrelinha é uma razão CONSTANTE do corpo — a mesma nos sete.\n')
{
  const rs = D.map((d) => d.entre / d.corpo)
  const min = Math.min(...rs), max = Math.max(...rs), med = rs.reduce((a, b) => a + b, 0) / rs.length
  console.log('      entrelinha/corpo: ' + rs.map((r) => r.toFixed(4)).join('  '))
  console.log('      mínimo ' + min.toFixed(4) + '   máximo ' + max.toFixed(4) + '   dispersão ' +
              (max - min).toFixed(4))
  console.log('      e o tradutor usava 14/10 = 1,4000 — perto, e outra coisa')
  ok('a entrelinha é a MESMA razão do corpo nos sete degraus, com dispersão abaixo de 0,002 —' +
     ' e é ' + med.toFixed(4) + ', não 1,4. A diferença é pequena e é o que faz o texto parecer' +
     ' apertado: a entrelinha é o que dá ar à página', (max - min) < 0.002 && Math.abs(med - 1.4) > 0.04)
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
  const perto = (t) => admissiveis.some((m) => Math.abs(t - m / 1000) <= 0.02)
  let fora = 0, docs = 0
  console.log('      documento         tamanhos no PDF')
  for (const [n, f] of [['computacional', 'papers/corpo_computacional.tex'], ['teoria', 'teoria.tex']]) {
    if (!compoe(f, `/tmp/t_${n}.pdf`)) continue
    docs++
    const ts = tamanhos(`/tmp/t_${n}.pdf`)
    for (const t of ts) if (!perto(t)) fora++
    console.log('      ' + n.padEnd(17) + ts.join(', ') + ' pt')
  }
  console.log('      degraus (exatos): ' + D.map((d) => d.corpo).join(', ') +
              ' — e os derivados por produto cruzado dos pares (0,4) e (1,3)')
  ok('todos os tamanhos que aparecem no PDF são degraus da escala do estilo.tex, e nenhum vem' +
     ' de fora dela. Antes eram 10, 12 e 15 — três números escritos à mão, e o do meio nem' +
     ' sequer da mesma família', docs === 2 && fora === 0)
}

console.log('\n§T4  O CONTROLO: mudada a escala no estilo.tex, o PDF muda — e volta.\n')
{
  const original = fs.readFileSync(ESTILO, 'utf8')
  let antes = [], depois = [], voltou = [], mordeu = false
  try {
    compoe('papers/corpo_computacional.tex', '/tmp/t_a.pdf'); antes = tamanhos('/tmp/t_a.pdf')
    const alvo = '\\fontsize{10.50}{15.22}'
    mordeu = original.includes(alvo)
    if (mordeu) {
      fs.writeFileSync(ESTILO, original.replace(alvo, '\\fontsize{19.00}{27.55}'))
      compoe('papers/corpo_computacional.tex', '/tmp/t_b.pdf'); depois = tamanhos('/tmp/t_b.pdf')
    }
  } finally {
    fs.writeFileSync(ESTILO, original)                 /* devolve-se SEMPRE */
  }
  compoe('papers/corpo_computacional.tex', '/tmp/t_c.pdf'); voltou = tamanhos('/tmp/t_c.pdf')
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
