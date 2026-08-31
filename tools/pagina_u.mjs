#!/usr/bin/env node
/* tools/pagina_u.mjs — CLI da ponte página ↔ U (motor chama via node ingerido).
 *   node tools/pagina_u.mjs para-u [app/banco]
 *   node tools/pagina_u.mjs para-p [u.json]
 */
import { readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { paginaParaU, uParaPagina } from '../app/src/banco_pagina_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const modo = process.argv[2]
const cam = process.argv[3]
if (!modo || (modo !== 'para-u' && modo !== 'para-p')) {
  process.stderr.write('uso: pagina_u.mjs para-u|para-p <dir|u.json>\n')
  process.exit(1)
}

if (modo === 'para-u') {
  const dir = cam || join(RAIZ, 'app', 'banco')
  const man = JSON.parse(readFileSync(
    join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))
  const fontes = {
    html: readFileSync(join(dir, 'pagina.html'), 'utf8'),
    css: readFileSync(join(dir, 'pagina.css'), 'utf8'),
    js: readFileSync(join(dir, 'pagina.js'), 'utf8'),
  }
  process.stdout.write(JSON.stringify(paginaParaU(fontes, man)) + '\n')
} else {
  if (!cam) {
    process.stderr.write('para-p pede o nodo U\n')
    process.exit(1)
  }
  const u = JSON.parse(readFileSync(cam, 'utf8'))
  process.stdout.write(JSON.stringify(uParaPagina(u)) + '\n')
}
