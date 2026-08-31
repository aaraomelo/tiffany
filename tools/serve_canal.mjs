#!/usr/bin/env node
/* serve_canal.mjs — só o barramento: WS /canal com fan-out (sem SPA, sem SQL).
 * Produção: nginx location = /canal → 127.0.0.1:47314
 * Alias genérico: canal.patriatechnology.com
 */
import http from 'node:http'
import { attachCanalLoopback } from './canal_loopback.mjs'

const PORT = Number(process.env.TIFFANY_CANAL_WS || 47314)
const HOST = process.env.TIFFANY_CANAL_BIND || '127.0.0.1'

const server = http.createServer((req, res) => {
  res.writeHead(200, { 'Content-Type': 'text/plain; charset=utf-8' })
  res.end('canal\n')
})

attachCanalLoopback(server)

server.listen(PORT, HOST, () => {
  console.log(`canal ws ${HOST}:${PORT}/canal (fan-out)`)
})
