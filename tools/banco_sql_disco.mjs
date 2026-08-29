/* banco_sql_disco.mjs — face SQL no disco: MOVE bidirecional, sem daemon nem fila.
 * Cada query é atómica: grava in no .torre, corre, lê out. Estado no disco. */
import { mkdirSync } from 'node:fs'
import { createShell } from './banco_shell_core.mjs'

/** Executa uma query Tiffany (ex. `NODE MOVE '…'`) directamente no disco. */
export function execSqlDisco (base, q) {
  mkdirSync(base, { recursive: true })
  const r = createShell(base).execQuery(String(q || '').trim())
  return { out: r.stdout, meta: r.meta, logs: r.logs }
}
