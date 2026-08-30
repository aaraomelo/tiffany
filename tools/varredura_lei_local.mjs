#!/usr/bin/env node
/* tools/varredura_lei_local.mjs — candidatos de Lei Local (nao promove).
 *   node tools/varredura_lei_local.mjs
 * Lê manifesto + realizações com resíduo 0. Emite JSON. 0 promoções.
 */
import { readFileSync } from 'node:fs'
import { spawnSync } from 'node:child_process'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { varrerLeiLocal, semPromocoes } from '../app/src/banco_lei_local_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const rel = varrerLeiLocal(man, { spawnSync })
if (!semPromocoes(rel)) {
  process.stderr.write('varredura_lei_local: promocao ilegal\n')
  process.exit(2)
}
process.stdout.write(JSON.stringify(rel, null, 2) + '\n')
