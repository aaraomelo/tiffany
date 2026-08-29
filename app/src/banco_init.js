// banco_init.js — arranque Araano: manifesto + tradutor.

import { carregaManifesto } from './manifesto_loader.js'
import { initTradutor } from './banco_tradutor.js'
import { initBancoSql } from './banco_sql.js'

export async function initBanco (opts = {}) {
  const man = await carregaManifesto(opts.manifestoUrl)
  initTradutor(man)
  await initBancoSql({ manifestoUrl: opts.manifestoUrl })
  return man
}
