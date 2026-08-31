#!/usr/bin/env node

/* serve_banco.mjs — dev local: /banco + /sql (MOVE DISCO) + WS /canal (protocolo bidirecional) */

import http from 'node:http'

import fs from 'node:fs'

import path from 'node:path'

import { fileURLToPath } from 'node:url'

import { execSqlDisco } from './banco_sql_disco.mjs'

import { attachCanalLoopback } from './canal_loopback.mjs'

import { createCanalWatcher } from './canal_watcher.mjs'

import { bandaDeTecido, bandaDeChave, publicaMeta } from './banco_banda.mjs'



const __dir = path.dirname(fileURLToPath(import.meta.url))

const RAIZ = path.join(__dir, '..')

const APP = path.join(RAIZ, 'app')

const WASM = path.join(RAIZ, 'assets', 'figuras', 'wasm')

/* Fonte única ISA: linguagens, orbitas, hopfield, corpos. Sem segundo URL. */
const MANIFESTO = path.join(RAIZ, 'conecthus', 'backends', 'manifesto.json')

const BANCO = path.join(APP, 'banco')

const SQL_BASE = process.env.TIFFANY_SQL_BASE || path.join(RAIZ, '.torre', 'reino')

const PORT = Number(process.env.PORT || 5173)

fs.mkdirSync(SQL_BASE, { recursive: true })



const MIME = {

  '.html': 'text/html; charset=utf-8',

  '.js': 'text/javascript; charset=utf-8',

  '.mjs': 'text/javascript; charset=utf-8',

  '.css': 'text/css; charset=utf-8',

  '.wasm': 'application/wasm',

  '.json': 'application/json; charset=utf-8',

}



const bandaTecido = bandaDeTecido()
let bandaPatria = null
try {
  const j = JSON.parse(fs.readFileSync(path.join(BANCO, 'patria.json'), 'utf8'))
  if (j.pub) bandaPatria = bandaDeChave(j.pub)
} catch { /* sem patria.json */ }
const banda = bandaTecido

let canal = null



function send (res, code, type, body) {

  res.writeHead(code, { 'Content-Type': type, 'Access-Control-Allow-Origin': '*' })

  res.end(body)

}



function readSafe (base, rel) {

  const p = path.normalize(path.join(base, rel))

  if (!p.startsWith(base)) return null

  if (!fs.existsSync(p) || fs.statSync(p).isDirectory()) return null

  return p

}



function runSql (q) {

  const r = execSqlDisco(SQL_BASE, q)

  if (r.meta) publicaMeta(canal?.broadcast, r.meta, banda)

  return r.out

}



const server = http.createServer((req, res) => {

  const url = (req.url || '/').split('?')[0]



  if (req.method === 'POST' && url === '/sql') {

    const chunks = []

    req.on('data', (c) => chunks.push(c))

    req.on('end', () => {

      try {

        const q = Buffer.concat(chunks).toString('utf8').trim()

        const out = runSql(q)

        send(res, 200, 'text/plain; charset=utf-8', out)

      } catch (e) {

        send(res, 500, 'text/plain; charset=utf-8', String(e.message || e))

      }

    })

    return

  }



  if (url === '/conecthus/backends/manifesto.json') {
    return send(res, 200, MIME['.json'], fs.readFileSync(MANIFESTO))
  }

  if (url === '/banco/node_path') {

    return send(res, 200, MIME['.json'], JSON.stringify({

      path: path.join(BANCO, 'node_pagina.mjs'),

      dir: BANCO,

      node: process.execPath,

    }))

  }



  if (url === '/banco' || url === '/banco/') {

    const f = path.join(BANCO, 'index.html')

    return send(res, 200, MIME['.html'], fs.readFileSync(f))

  }



  let file = null

  if (url.startsWith('/src/')) file = readSafe(APP, url.slice(1))

  else if (url.startsWith('/wasm/')) file = readSafe(WASM, url.slice(6))

  else if (url.startsWith('/banco/')) file = readSafe(BANCO, url.slice(7))



  if (!file) {

    res.writeHead(404, { 'Content-Type': 'text/plain' })

    return res.end('404 ' + url + '\n')

  }

  const ext = path.extname(file)

  send(res, 200, MIME[ext] || 'application/octet-stream', fs.readFileSync(file))

})



let broadcastFn = null

const watchers = [

  createCanalWatcher({

    sqlBase: SQL_BASE,

    banda: bandaTecido,

    broadcast: (b) => broadcastFn?.(b),

    bancoDir: BANCO,

  }),

]

if (bandaPatria) {

  watchers.push(createCanalWatcher({

    sqlBase: SQL_BASE,

    banda: bandaPatria,

    broadcast: (b) => broadcastFn?.(b),

    bancoDir: BANCO,

  }))

}

function handleBump (bumped) {

  for (const h of watchers) h(bumped)

}

canal = attachCanalLoopback(server, handleBump)

broadcastFn = canal.broadcast



server.listen(PORT, '127.0.0.1', () => {

  const u = `http://127.0.0.1:${PORT}/banco/`

  console.log(`banco local: ${u}`)

  console.log(`  sql: MOVE erg+DISCO (.torre/reino) — node→assembly→metal`)

  console.log(`  canal: WS /canal fan-out` + (bandaPatria ? ' (tecido + patria)' : ''))

  console.log(`  wasm: ${WASM}`)

})

