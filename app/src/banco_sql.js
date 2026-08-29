// banco_sql.js — face da máquina: sql na arena; shell absorvido (node_move + canal).

import { carregaManifesto } from './manifesto_loader.js'
import { initTradutor } from './banco_tradutor.js'
import { initCelula, execQueryCelula } from './banco_celula.js'
import { absorveBackend } from './banco_absorve.js'
import { discoBrowser } from './banco_disco.js'
import { escSql, shellPadrao, SHELLS, S_BASH_IN, S_BASH_OUT, S_PWSH_IN, S_PWSH_OUT, S_NODE_IN, S_NODE_OUT } from './banco_sql_interno.js'

let _pronto = null
let _disco = null

export async function initBancoSql (opts = {}) {
  if (_pronto) return _pronto
  const man = await carregaManifesto(opts.manifestoUrl)
  initTradutor(man)
  _disco = discoBrowser(opts)
  if (typeof document !== 'undefined' && opts.celula !== false) {
    await initCelula({ wasmBase: opts.wasmBase, storage: _disco })
  }
  _pronto = man
  return man
}

export function discoBanco () {
  return _disco
}

/** Query SQL — absorvida na arena (sql_move), não adaptador. Motor = banco/sql.c. */
export async function sqlQuery (q, ctx = {}) {
  await initBancoSql(ctx)
  const r = await execQueryCelula(q, ctx)
  return r.out
}

/** Script shell — node_move/bash_move na arena; canal sincroniza slots. */
export async function shellMove (script, canal = null, backend = shellPadrao(), ctx = {}) {
  await initBancoSql(ctx)
  const r = await absorveBackend(backend, script, { ...ctx, canal, storage: ctx.storage ?? _disco })
  return r.out
}

export { escSql, shellPadrao, SHELLS, S_BASH_IN, S_BASH_OUT, S_PWSH_IN, S_PWSH_OUT, S_NODE_IN, S_NODE_OUT }
