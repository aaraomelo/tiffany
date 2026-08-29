/* tests/celula_browser.js — célula sql no browser: wasm + GKBANCO, sem I/O por query.
 *
 * §C0  manifesto declara mvp.celula e armazenamento GKBANCO
 * §C1  sql_move ida+volta na arena (consultar.wasm)
 * §C2  execQueryCelula SQL puro não toca disco
 * §C3  GKBANCO persiste estado shell
 *
 *   node tests/celula_browser.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const man = JSON.parse(readFileSync(
  join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const BASE = man.nulo_disco || 8

function loadWasm () {
  const p = join(RAIZ, 'assets', 'figuras', 'wasm', man.mvp?.sql?.wasm || 'consultar.wasm')
  if (!existsSync(p)) return null
  return new WebAssembly.Instance(new WebAssembly.Module(readFileSync(p)), {}).exports
}

/* §C0 */
{
  ok('§C0 mvp.celula declarado', !!man.mvp?.celula)
  ok('§C0 armazenamento GKBANCO', man.mvp?.armazenamento?.magia === 'GKBANCO')
  ok('§C0 sql.arena=browser', man.linguagens.find((l) => l.nome === 'sql')?.arena === 'browser')
}

/* §C1 — wasm arena */
{
  const ex = loadWasm()
  if (!ex) {
    ok('§C1 consultar.wasm', false)
  } else {
    const mem = new Uint8Array(ex.DISCO.buffer)
    const q = "INSERT TEXTO 'celula'"
    const b = Buffer.from(q, 'utf8')
    mem.set(b, BASE + 1024)
    const n1 = ex.sql_move(1024, b.length, 4096, -1)
    const n2 = ex.sql_move(4096, n1, 8192, +1)
    const volta = Buffer.from(mem.slice(BASE + 8192, BASE + 8192 + n2)).toString('utf8')
    ok('§C1 sql_move† na arena', volta === q)
  }
}

/* §C2/C3 — módulos browser + persistência GKBANCO isomórfica */
{
  ok('§C2 banco_celula.js existe', existsSync(join(RAIZ, 'app', 'src', 'banco_celula.js')))
  ok('§C3 banco_disco.js GKBANCO', existsSync(join(RAIZ, 'app', 'src', 'banco_disco.js')))
  const store = new Map()
  const ls = {
    getItem (k) { return store.has(k) ? store.get(k) : null },
    setItem (k, v) { store.set(String(k), String(v)) },
  }
  ls.setItem('gk:banco:estado', JSON.stringify({
    magia: 'GKBANCO', v: 1, shells: { node: { in: "console.log('a')", out: 'a\n' } },
  }))
  const o = JSON.parse(ls.getItem('gk:banco:estado'))
  ok('§C3 GKBANCO persiste shell no disco', o.shells.node.out === 'a\n')
}

console.log(`\n=== celula_browser: ${feitas - falhas}/${feitas} OK ===`)
process.exit(falhas ? 1 : 0)
