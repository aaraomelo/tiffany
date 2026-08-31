// banco_celula.js — célula sql: consultar.wasm na arena + GKBANCO (LS = disco; IDB só com flag).

import { loadWasm, moveWasm } from './banco_move.js'
import { manifestoAtual } from './manifesto_loader.js'
import { escolheDisco, leEstado, gravaEstado } from './banco_disco.js'

let exSql = null
let estado = null
let disco = null
let wasmBase = '/wasm/'

export async function initCelula (opts = {}) {
  wasmBase = opts.wasmBase || wasmBase
  disco = await escolheDisco(opts)
  const man = manifestoAtual()
  const wasmNome = man.mvp?.sql?.wasm || man.linguagens.find((l) => l.nome === 'sql')?.wasm || 'consultar.wasm'
  if (!exSql) exSql = await loadWasm(wasmBase, wasmNome, disco)
  estado = leEstado(disco)
  return exSql
}

export function discoCelula () {
  return disco
}

export function celulaPronta () {
  return !!exSql
}

/** Query SQL — MOVE na arena (bit-level, sem I/O). */
export function execSqlWasm (q) {
  if (!exSql) throw new Error('célula não inicializada')
  const tags = moveWasm(exSql, 'sql_move', q, -1)
  const volta = moveWasm(exSql, 'sql_move', tags, +1)
  gravaEstado(estado, disco)
  return { out: volta, meta: { via: 'arena+GKBANCO', bytesIn: q.length, bytesOut: volta.length } }
}

/** Face da célula — só SQL; shell é semântica→sql noutro módulo. */
export async function execQueryCelula (q, opts = {}) {
  if (!exSql) await initCelula(opts)
  return execSqlWasm(String(q || '').trim())
}
