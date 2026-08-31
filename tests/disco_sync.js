/* tests/disco_sync.js — disco da M_wasm (LS) + ponte IDB + merge + quota + canal S_ESTADO.
 *
 *   node tests/disco_sync.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import {
  MAGIA, CHAVE_ESTADO, CHAVE_MANIFESTO, chaveWasm,
  STORE_ESTADO, STORE_MANIFESTO, STORE_WASM,
  estadoVazio, leEstado, gravaEstado, gravaShell,
  mergeEstado, cortaContexto, bytesBanco, bytesJson, LIMITE_WIRE,
  gravaPagina, lePagina, gravaManifestoLS, leManifestoLS,
  memoriaIDB, escolheDisco,
} from '../app/src/banco_disco.js'
import { modoDaSessao } from '../app/src/banco_sessao_u.js'
import { sincronizaEmFundo } from '../app/src/banco_sync.js'
import { S_CANAL, S_ESTADO_REQ, S_ESTADO_RSP } from '../tools/canal_slots.mjs'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const sql = readFileSync(join(RAIZ, 'banco', 'sql.c'), 'utf8')

ok('§D0 ponte_disco',
  man.corpos?.motor?.ponte_disco === 'app/src/banco_disco.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_disco)))
ok('§D0 ponte_sync',
  man.corpos?.motor?.ponte_sync === 'app/src/banco_sync.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_sync)))
ok('§D0 S_ESTADO no sql.c',
  sql.includes('#define S_ESTADO_REQ (S_CANAL + 9210u)') &&
  sql.includes('#define S_ESTADO_RSP (S_CANAL + 9211u)'))
ok('§D0 S_ESTADO JS', S_ESTADO_REQ === S_CANAL + 9210 && S_ESTADO_RSP === S_CANAL + 9211)
ok('§D0 LIMITE_WIRE 16 bits', LIMITE_WIRE === 65535)
ok('§D0 solo sem addr+pub', modoDaSessao({}) === 'solo')
{
  const front = readFileSync(join(RAIZ, 'app', 'src', 'banco_front.js'), 'utf8')
  const iPaint = front.indexOf('montaDom(')
  const iFundo = front.indexOf('sincronizaEmFundo(')
  ok('§D0 estado remoto é eventual; DOM não depende dele',
    front.includes('estado remoto é eventual') &&
    iPaint >= 0 && iFundo > iPaint &&
    !/await sincroniza\(/.test(front))
  const sqlSrc = readFileSync(join(RAIZ, 'app', 'src', 'banco_sql.js'), 'utf8')
  const tradSrc = readFileSync(join(RAIZ, 'app', 'src', 'banco_tradutor.js'), 'utf8')
  ok('§D0 primeiro paint não depende das capacidades não usadas',
    front.includes('primeiro paint não depende') &&
    !/from ['"]\.\/banco_coord_u\.js['"]/.test(front) &&
    !/from ['"]\.\/banco_coord_canal\.js['"]/.test(front) &&
    !/from ['"]\.\/c_wasm_shell\.js['"]/.test(front) &&
    front.includes("import('./banco_coord_u.js')") &&
    !/from ['"]\.\/banco_absorve\.js['"]/.test(sqlSrc) &&
    !/from ['"]\.\/banco_absorve\.js['"]/.test(tradSrc) &&
    sqlSrc.includes("import('./banco_absorve.js')") &&
    tradSrc.includes("import('./banco_absorve.js')"))
  const fundo = await sincronizaEmFundo(null, memoriaLS())
  ok('§D0 sincronizaEmFundo sem canal não bloqueia', fundo.via === 'solo')
}

{
  const ls = memoriaLS()
  const e = estadoVazio()
  gravaShell(e, 'bash', 'echo a', 'a\n')
  gravaEstado(e, ls)
  const o = leEstado(ls)
  ok('§D1 persiste GKBANCO no LS', o.magia === MAGIA && o.shells.bash.out === 'a\n' && o.shells.bash.t)
  ok('§D1 bytesBanco conta UTF-16', bytesBanco(ls) === 2 * (CHAVE_ESTADO.length + ls.getItem(CHAVE_ESTADO).length))
}

{
  const v1 = { magia: MAGIA, v: 1, shells: { node: { in: '1', out: '1\n' } } }
  const ls = memoriaLS()
  ls.setItem(CHAVE_ESTADO, JSON.stringify(v1))
  const o = leEstado(ls)
  ok('§D1 v1 sem t ainda lê', o.shells.node.out === '1\n' && Array.isArray(o.pendente) && o.v === 2)
}

{
  const a = estadoVazio()
  a.shells.bash = { in: 'old', out: 'old', t: '2020-01-01T00:00:00.000Z' }
  a.shells.node = { in: 'a', out: 'a', t: '2024-01-01T00:00:00.000Z' }
  const b = estadoVazio()
  b.shells.bash = { in: 'new', out: 'new', t: '2025-01-01T00:00:00.000Z' }
  b.shells.pwsh = { in: 'p', out: 'p', t: '2023-01-01T00:00:00.000Z' }
  const m = mergeEstado(a, b)
  ok('§D2 merge: t mais recente ganha', m.shells.bash.out === 'new')
  ok('§D2 merge: shells dos dois lados', m.shells.node.out === 'a' && m.shells.pwsh.out === 'p')
}

{
  const e = estadoVazio()
  e.shells.velha = { in: 'x'.repeat(200), out: 'y'.repeat(200), t: '2020-01-01T00:00:00.000Z' }
  e.shells.nova = { in: 'ok', out: 'ok', t: '2026-01-01T00:00:00.000Z' }
  cortaContexto(e, 400, 'utf16')
  ok('§D3 corta a mais antiga', !e.shells.velha && e.shells.nova)
}

{
  function quotaLS (limite) {
    const m = new Map()
    return {
      getItem (k) { return m.has(k) ? m.get(k) : null },
      setItem (k, v) {
        const n = 2 * (String(k).length + String(v).length)
        if (n > limite) {
          const err = new Error('quota')
          err.name = 'QuotaExceededError'
          throw err
        }
        m.set(String(k), String(v))
      },
      removeItem (k) { m.delete(String(k)) },
      key (i) { return [...m.keys()][i] ?? null },
      get length () { return m.size },
    }
  }
  const ls = quotaLS(400)
  const e = estadoVazio()
  e.shells.a = { in: 'a'.repeat(80), out: 'a'.repeat(80), t: '2020-01-01T00:00:00.000Z' }
  e.shells.b = { in: 'ok', out: 'ok', t: '2026-01-01T00:00:00.000Z' }
  gravaEstado(e, ls)
  const o = leEstado(ls)
  ok('§D3 quota do browser corta contexto', Object.keys(o.shells).length <= 1)
  ok('§D3 bytesJson <= LIMITE_WIRE', bytesJson(o) <= LIMITE_WIRE)
}

{
  const e = estadoVazio()
  e.shells.bash = { in: 'echo', out: 'ok', t: '2026-01-01T00:00:00.000Z' }
  ok('§D4 json de estado cabe no fio típico', bytesJson(e) < LIMITE_WIRE)
}

ok('§D4 mvp.armazenamento local+remoto',
  man.mvp?.armazenamento?.magia === MAGIA &&
  /S_ESTADO/.test(man.mvp?.armazenamento?.canal || man.mvp?.armazenamento?.remoto || '') &&
  /eventual/.test(man.mvp?.armazenamento?.canal || ''))

{
  const ls = memoriaLS()
  const e = estadoVazio()
  gravaPagina(e, { html: '<p>x</p>', css: 'p{}', js: '1' })
  gravaEstado(e, ls)
  const p = lePagina(leEstado(ls))
  ok('§D5 pagina no LS', p && p.html === '<p>x</p>' && p.css === 'p{}' && p.js === '1')
  gravaManifestoLS(ls, { linguagens: [{ nome: 'sql' }] })
  ok('§D5 manifesto no LS', leManifestoLS(ls).linguagens[0].nome === 'sql' &&
    ls.getItem(CHAVE_MANIFESTO).includes('sql'))
}

{
  const idb = memoriaIDB()
  const e = estadoVazio()
  gravaShell(e, 'bash', 'echo a', 'a\n')
  gravaPagina(e, { html: '<p>x</p>', css: 'p{}', js: '1' })
  gravaEstado(e, idb)
  const o = leEstado(idb)
  ok('§D6 persiste GKBANCO no IDB', o.magia === MAGIA && o.shells.bash.out === 'a\n')
  ok('§D6 estado vai ao store gk:banco:estado',
    idb._stores[STORE_ESTADO].has(CHAVE_ESTADO) &&
    JSON.parse(idb._stores[STORE_ESTADO].get(CHAVE_ESTADO)).magia === MAGIA)
  ok('§D6 pagina no IDB', lePagina(o).html === '<p>x</p>')
  gravaManifestoLS(idb, { linguagens: [{ nome: 'sql' }] })
  ok('§D6 manifesto no store gk:banco:manifesto',
    leManifestoLS(idb).linguagens[0].nome === 'sql' &&
    idb._stores[STORE_MANIFESTO].get(CHAVE_MANIFESTO).includes('sql'))
  idb.setItem(chaveWasm('sql'), 'z')
  ok('§D6 wasm no store gk:banco:wasm',
    idb._stores[STORE_WASM].get('sql') === 'z' &&
    idb.getItem(chaveWasm('sql')) === 'z')
}

{
  const ls = memoriaLS()
  const idb = memoriaIDB()
  const e = estadoVazio()
  e.shells.bash = { in: 'echo', out: 'ok', t: '2026-01-01T00:00:00.000Z' }
  gravaEstado(e, ls)
  gravaEstado(JSON.parse(JSON.stringify(e)), idb)
  ok('§D6 LS e IDB leem o mesmo JSON',
    leEstado(ls).shells.bash.out === leEstado(idb).shells.bash.out &&
    leEstado(ls).magia === leEstado(idb).magia)
}

ok('§D6 mvp.armazenamento idb/docker/mongo candidatos',
  /nao localizada/.test(man.mvp?.armazenamento?.docker || '') &&
  /nao localizada/.test(man.mvp?.armazenamento?.mongo || '') &&
  /candidato 1/.test(man.mvp?.armazenamento?.idb || ''))
ok('§D6 volume = estado.json (D neutro, nao M_Docker)',
  /estado\.json/.test(man.mvp?.armazenamento?.volume || '') &&
  /nao localizada/.test(man.mvp?.armazenamento?.docker || '') &&
  /paridade de contentor/.test(man.mvp?.armazenamento?.docker || ''))

{
  const def = await escolheDisco({})
  const flag = await escolheDisco({ disco: 'idb' })
  ok('§D6 escolheDisco default = LS (sem stores IDB)', typeof def._stores === 'undefined')
  ok('§D6 escolheDisco flag = ponte IDB', !!flag._stores)
}

console.log('')
if (!falhas) {
  console.log('  Disco: LS = contexto local (quota); S_ESTADO = disco remoto; IDB = ponte; merge por t.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
