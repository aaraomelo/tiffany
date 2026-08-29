/* tests/interface_padrao.js — o SQL como interface padrão do sistema
 * (ordem do coordenador, 20/08).
 *
 * «Interface padrão» não é privilégio de língua no banco — as linguagens
 * continuam backends de porta única. É a PORTA DE ENTRADA E SAÍDA por
 * omissão: quando nada se especifica, o corpo entra e sai como query SQL,
 * porque o banco/sql.c compila para a ISA, traduz programas, e o LaTeX
 * (e as outras) continuam realizacoes — com tex.wasm como tradutor pleno
 * do latex, nao da interface.
 *
 * §I0  o manifesto declara: interface_padrao = sql
 * §I1  o sql e (1,1,1) — analisa, compila, atravessa
 * §I2  o latex continua backend com tradutor pleno (tex.wasm)
 * §I3  a involucao da porta SQL: compilar† = descompilar (consultar.wasm)
 * §I4  o pleno sql.c esta atestado verde na bateria
 *
 *   node tests/interface_padrao.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const man = JSON.parse(readFileSync(
  join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))

/* §I0 — a declaração */
ok('§I0 o manifesto declara interface_padrao = sql', man.interface_padrao === 'sql')

/* §I1 — a razão estrutural: (1,1,1) na porta SQL */
{
  const sql = man.linguagens.find(l => l.nome === 'sql')
  ok('§I1 sql existe e é (p,q,r)=(1,1,1)',
    sql && sql.p === 1 && sql.q === 1 && sql.r === 1)
  ok('§I1 sql aponta o pleno nativo (banco/sql.c)',
    sql && sql.pleno === 'banco/sql.c')
  ok('§I1 as linguagens-backend continuam no manifesto (>=12)',
    man.linguagens.length >= 12)
}

/* §I2 — LaTeX continua backend com tradutor pleno, não interface */
{
  const latex = man.linguagens.find(l => l.nome === 'latex')
  ok('§I2 latex continua (1,1,1) como realizacao',
    latex && latex.p === 1 && latex.q === 1 && latex.r === 1)
  ok('§I2 latex tem tradutor pleno tex.wasm — nenhuma outra tem',
    latex && latex.pleno === 'tex.wasm' &&
    man.linguagens.filter(l => l.pleno === 'tex.wasm').length === 1)
}

/* §I3 — a involução da porta SQL, no wasm real */
{
  const BASE = man.nulo_disco || 8
  const wasmPath = join(RAIZ, 'assets', 'figuras', 'wasm', 'consultar.wasm')
  if (!existsSync(wasmPath)) {
    ok('§I3 consultar.wasm existe (corra tools/sobe_backends_wasm.sh)', false)
  } else {
    const mod = new WebAssembly.Module(readFileSync(wasmPath))
    const inst = new WebAssembly.Instance(mod, {})
    const E = inst.exports
    const mem = new Uint8Array(E.DISCO.buffer)
    const fonte = "INSERT TEXTO 'ouro'"
    const src = Buffer.from(fonte, 'utf8')
    mem.set(src, BASE + 100)
    const n1 = E.sql_compilar(100, src.length, 4096)
    const n2 = E.sql_descompilar(4096, n1, 8192)
    const volta = Buffer.from(mem.slice(BASE + 8192, BASE + 8192 + n2)).toString('utf8')
    ok('§I3 compilar† = descompilar: a volta devolve a query (wasm real)',
      n2 > 0 && volta === fonte)
  }
}

/* §I4 — o pleno nativo está atestado */
{
  const at = readFileSync(join(RAIZ, 'tools', 'atestados.txt'), 'utf8')
  const linha = at.split('\n').filter(l => /^sql /.test(l)).pop() || ''
  ok('§I4 o pleno banco/sql.c está verde na bateria (sql … 0)', / 0$/.test(linha.trim()))
}

/* §I5 — entrada das apps = FEBE Tiffany (não muda interface_padrao) */
ok('§I5 manifesto declara entrada_apps = pgwire', man.entrada_apps === 'pgwire')
ok('§I5 interface_padrao continua sql (pgwire é face, não motor)',
  man.interface_padrao === 'sql')

console.log('')
if (!falhas) {
  console.log('  O SQL é a interface padrão: query compila para a ISA, traduz programas,')
  console.log('  e as linguagens (LaTeX incluído) continuam backends de porta única.')
  console.log('  Apps ligam por pgwire (FEBE); o motor continua sql.c.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
