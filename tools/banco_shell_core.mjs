/* banco_shell_core.mjs — MOVE: traduz → assembly ERG → metal (DISCO). */
import { parseMove, parseLiteral, SLOTS, S_CANAL_BASE } from './banco_shell_parse.mjs'
import { createShell, execMoveDisco } from './banco_metal.mjs'

export { parseMove, parseLiteral, SLOTS, S_CANAL_BASE, createShell, execMoveDisco }
