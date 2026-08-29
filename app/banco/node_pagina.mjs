// node_pagina.mjs — lê pagina.{html,css,js} e emite JSON (stdout).
// Invocado pelo banco: NODE MOVE '…node_pagina…' (uma operação)
import { readFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const dir = dirname(fileURLToPath(import.meta.url))
const payload = {
  html: readFileSync(join(dir, 'pagina.html'), 'utf8'),
  css: readFileSync(join(dir, 'pagina.css'), 'utf8'),
  js: readFileSync(join(dir, 'pagina.js'), 'utf8'),
  node: process.version,
  via: 'node MOVE',
}
process.stdout.write(JSON.stringify(payload))
