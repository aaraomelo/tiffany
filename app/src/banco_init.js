// banco_init.js — arranque: manifesto ISA + tradutor + célula.
// O motor é banco/sql.c (interface_padrao=sql). Assistente e terminal são clientes.

import { carregaManifesto } from './manifesto_loader.js'
import { initTradutor } from './banco_tradutor.js'
import { initBancoSql } from './banco_sql.js'
import { escolheDisco } from './banco_disco.js'

export async function initBanco (opts = {}) {
  const disco = await escolheDisco(opts)
  const man = await carregaManifesto(opts.manifestoUrl, disco)
  initTradutor(man)
  await initBancoSql({ ...opts, storage: disco })
  return man
}
