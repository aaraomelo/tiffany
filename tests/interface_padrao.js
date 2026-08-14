/* tests/interface_padrao.js — o LaTeX como interface padrão do sistema
 * (ordem do coordenador, 13/08).
 *
 * «Interface padrão» não é privilégio de língua no banco — as linguagens
 * continuam backends de porta única. É a PORTA DE ENTRADA E SAÍDA por
 * omissão: quando nada se especifica, o corpo entra e sai como .tex,
 * porque só o LaTeX tem o tradutor COMPLETO — compõe↔descompõe involutivo
 * (compor.wasm), o pleno .tex↔PDF (tex.wasm) e a volta byte a byte com a
 * fonte a viajar no PDF (tests/tex.c §X17, 62:0).
 *
 * §I0  o manifesto declara: interface_padrao = latex
 * §I1  o latex é o único backend com (p,q,r)=(1,1,1) E tradutor pleno
 * §I2  a involução da porta: compor† = descompor (id no corpo, wasm real)
 * §I3  a razão medida: a volta plena existe e está atestada (tex 62:0;
 *      a fonte viaja no PDF — thm:composicao)
 *
 *   node tests/interface_padrao.js
 */
'use strict'
const fs = require('fs')
const path = require('path')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const RAIZ = path.join(__dirname, '..')
const man = JSON.parse(fs.readFileSync(
  path.join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))

/* §I0 — a declaração */
ok('§I0 o manifesto declara interface_padrao = latex', man.interface_padrao === 'latex')

/* §I1 — a razão estrutural: (1,1,1) com tradutor pleno */
{
  const latex = man.linguagens.find(l => l.nome === 'latex')
  ok('§I1 latex existe e é (p,q,r)=(1,1,1)',
    latex && latex.p === 1 && latex.q === 1 && latex.r === 1)
  ok('§I1 latex tem o tradutor PLENO (tex.wasm) — nenhuma outra tem',
    latex && latex.pleno === 'tex.wasm' &&
    man.linguagens.filter(l => l.pleno).length === 1)
  ok('§I1 as outras continuam backends (nenhuma some do manifesto)',
    man.linguagens.length >= 12)
}

/* §I2 — a involução da porta, no wasm real */
{
  /* o protocolo do §W4 (backends_wasm.js): offsets relativos ao nulo_disco */
  const BASE = man.nulo_disco || 8
  const wasmPath = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'compor.wasm')
  const mod = new WebAssembly.Module(fs.readFileSync(wasmPath))
  const inst = new WebAssembly.Instance(mod, {})
  const E = inst.exports
  const mem = new Uint8Array(E.DISCO.buffer)
  const fonte = 'O \\emph{corpo} entra pela porta \\code{MOVE} e volta.'
  const src = Buffer.from(fonte, 'utf8')
  mem.set(src, BASE + 100)
  const n1 = E.latex_compor(100, src.length, 4096)
  const n2 = E.latex_descompor(4096, n1, 8192)
  const volta = Buffer.from(mem.slice(BASE + 8192, BASE + 8192 + n2)).toString('utf8')
  ok('§I2 compor† = descompor: a volta devolve o corpo (id na porta, wasm real)',
    n2 > 0 && volta === fonte)
}

/* §I3 — a volta plena está atestada (não se re-mede aqui; cita-se o atestado) */
{
  const at = fs.readFileSync(path.join(RAIZ, 'tools', 'atestados.txt'), 'utf8')
  const linha = at.split('\n').filter(l => /^tex /.test(l)).pop() || ''
  ok('§I3 o pleno está atestado verde na bateria (tex … 0)', / 0$/.test(linha.trim()))
}

console.log('')
if (!falhas) {
  console.log('  O LaTeX é a interface padrão: a única porta com (1,1,1) e tradutor')
  console.log('  pleno, involutiva no wasm real, com a volta byte a byte atestada.')
  console.log('  As linguagens continuam backends; o padrão é a porta, não o trono.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
