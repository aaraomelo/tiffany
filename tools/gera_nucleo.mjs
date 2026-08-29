#!/usr/bin/env node
/* gera_nucleo.mjs — pré-monta fitas do núcleo (interpretar.c → wasm → ERG → fita).
 * Ferramenta de build; semântica corre no erg, não aqui.
 *   node tools/gera_nucleo.mjs [node|bash|powershell|all] */
import { garanteErg } from './banco_metal.mjs'
import { garanteFitaCorre } from '../lib/nucleo_metal.mjs'

const alvo = process.argv[2] || 'all'
const lista = alvo === 'all' ? ['node', 'bash', 'powershell'] : [alvo]
const erg = garanteErg()

for (const b of lista) {
  try {
    const { fitaPath, ergPath } = garanteFitaCorre(b, erg, { force: true })
    console.log(`${b}: ${ergPath}`)
    console.log(`      ${fitaPath}`)
  } catch (e) {
    console.error(`${b}: ${e.message || e}`)
    process.exitCode = 1
  }
}
