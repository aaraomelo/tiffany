#!/usr/bin/env node
/* banco_shell_win.mjs — BASH / POWERSHELL / NODE no Windows (sem sql.c).
 * Uso: node banco_shell_win.mjs <base> "<query>"
 * Contrato: apenas MOVE (Lei 1). Uma invocação = um ciclo no disco. */
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'
import { createShell } from './banco_shell_core.mjs'

const base = process.argv[2] || join(dirname(fileURLToPath(import.meta.url)), '..', '.torre', 'reino')
const q = (process.argv[3] || '').trim()
const shell = createShell(base)

try {
  const r = shell.execQuery(q)
  for (const line of r.logs) console.error(line)
  process.stdout.write(r.stdout)
  process.exit(0)
} catch (e) {
  console.error(String(e.message || e))
  process.exit(1)
}
