#!/usr/bin/env node
/* mvp.mjs — liga MVP: traduz → erg → metal + canal + benchmark. */
import { spawn, execFileSync } from 'node:child_process'
import { existsSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import net from 'node:net'
import { garanteErg } from './banco_metal.mjs'

const __dir = dirname(fileURLToPath(import.meta.url))
const RAIZ = join(__dir, '..')
const SQL_MVP = join(RAIZ, '.torre', 'reino_mvp')
const SQL_BENCH = join(RAIZ, '.torre', 'reino_bench_mvp')

const args = process.argv.slice(2)
let PORT = 5173
let N = 50
let noServe = false
for (let i = 0; i < args.length; i++) {
  if (args[i] === '--port' && args[i + 1]) PORT = Number(args[++i])
  if (args[i] === '--n' && args[i + 1]) N = Number(args[++i])
  if (args[i] === '--no-serve') noServe = true
}

const URL = `http://127.0.0.1:${PORT}`

function esperaPorta (port, ms = 25000) {
  const t0 = Date.now()
  return new Promise((resolve, reject) => {
    function tenta () {
      const s = net.connect(port, '127.0.0.1', () => { s.end(); resolve() })
      s.on('error', () => {
        if (Date.now() - t0 > ms) reject(new Error('timeout porta ' + port))
        else setTimeout(tenta, 60)
      })
    }
    tenta()
  })
}

function run (label, script, extraEnv = {}) {
  console.log(`\n>>> ${label}\n`)
  return new Promise((resolve, reject) => {
    const child = spawn(process.execPath, script, {
      cwd: RAIZ,
      env: { ...process.env, ...extraEnv },
      stdio: 'inherit',
    })
    child.on('exit', (code) => (code === 0 ? resolve() : reject(new Error(`${label} exit ${code}`))))
  })
}

let srv = null
try {
  console.log('mvp: erg no metal (node→assembly→processador)')
  garanteErg()

  if (!noServe) {
    srv = spawn(process.execPath, [join(__dir, 'serve_banco.mjs')], {
      env: { ...process.env, PORT: String(PORT), TIFFANY_SQL_BASE: SQL_MVP },
      stdio: ['ignore', 'pipe', 'pipe'],
    })
    srv.stdout?.on('data', (c) => process.stderr.write(c))
    srv.stderr?.on('data', (c) => process.stderr.write(c))
    await esperaPorta(PORT)
    console.log(`mvp: ${URL}/banco/`)
  }

  await run('teste ponta a ponta', [join(RAIZ, 'tests', 'mvp_ponta.js')], {
    MVP_PORT: String(PORT),
    TIFFANY_SQL_BASE: SQL_MVP,
  })

  await run('benchmark', [join(__dir, 'bench_mvp.mjs'), '--url', URL, '--n', String(N)], {
    TIFFANY_SQL_BASE: SQL_BENCH,
  })

  console.log('\n=== MVP OK (erg+DISCO+canal) ===\n')
} catch (e) {
  console.error('\nmvp FALHOU:', e.message || e)
  process.exit(1)
} finally {
  if (srv) srv.kill()
}
