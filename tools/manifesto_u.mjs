#!/usr/bin/env node
/* tools/manifesto_u.mjs — CLI da ponte Manifesto ↔ U (o motor chama via node ingerido).
 *   node tools/manifesto_u.mjs para-u [manifesto.json]
 *   node tools/manifesto_u.mjs para-m [u.json]
 */
import { readFileSync } from 'node:fs'
import { manifestoParaU, uParaMatriz } from '../app/src/banco_manifesto_u.js'

const modo = process.argv[2]
const cam = process.argv[3]
if (!modo || !cam || (modo !== 'para-u' && modo !== 'para-m')) {
  process.stderr.write('uso: manifesto_u.mjs para-u|para-m <ficheiro>\n')
  process.exit(1)
}
const doc = JSON.parse(readFileSync(cam, 'utf8'))
const out = modo === 'para-u' ? manifestoParaU(doc) : uParaMatriz(doc)
process.stdout.write(JSON.stringify(out) + '\n')
