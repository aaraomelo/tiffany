import { resolveNoCorpo, tipoDe, FICHEIROS } from './src/corpo.js'
import { defineConfig } from 'vite'
import { fileURLToPath, pathToFileURL, URL } from 'node:url'
import { execSync } from 'node:child_process'
import { statSync, existsSync, readFileSync, writeFileSync, copyFileSync, mkdirSync } from 'node:fs'
import { resolve, dirname } from 'node:path'
import net from 'node:net'
import dgram from 'node:dgram'
import crypto from 'node:crypto'
import { Buffer } from 'node:buffer'

// A data da última atualização do enredo: sai do commit do FONTE (.tex), não da hora do build —
// rebuildar sem mexer no livro não deve mudar a data. Se não houver git, cai no mtime do arquivo.
const fonteEnredo = fileURLToPath(new URL('../enredo.tex', import.meta.url))
function dataEnredo() {
  try {
    const iso = execSync(`git log -1 --format=%cI -- "${fonteEnredo}"`, { encoding: 'utf8' }).trim()
    if (iso) return iso
  } catch (e) { /* sem git: usa o arquivo */ }
  return statSync(fonteEnredo).mtime.toISOString()
}

function erro (res, msg) {
  res.statusCode = 500
  res.setHeader('Content-Type', 'text/plain; charset=utf-8')
  res.end(msg + '\n')
}

// ── O CORPO NO FRONT: /corpo/<caminho> sai do BANCO (.torre/reino_corpo) ──
// Fonte de verdade: IMPORT CORPO no sql.c. Disco do repo só entra no import.
function serveOCorpo () {
  const raiz = fileURLToPath(new URL('..', import.meta.url))
  const sqlBin = resolve(raiz, 'app/.cache/tiffany_sql')
  const sqlBase = resolve(raiz, '.torre/reino')
  const corpoRoot = resolve(raiz, '.torre/reino_corpo')
  function ensureSql () {
    if (!existsSync(sqlBin)) {
      mkdirSync(dirname(sqlBin), { recursive: true })
      execSync(
        `cc -O2 -std=c99 -I${resolve(raiz, 'lib')} -I${resolve(raiz, 'banco')} ` +
        `${resolve(raiz, 'banco/sql.c')} -lm -o ${sqlBin}`,
        { stdio: 'inherit' }
      )
    }
    mkdirSync(sqlBase, { recursive: true })
  }
  function ensureCorpoImportado () {
    ensureSql()
    const marcador = resolve(corpoRoot, '.importado')
    if (existsSync(marcador)) return
    mkdirSync(corpoRoot, { recursive: true })
    execSync(
      `"${sqlBin}" "${sqlBase}" ${JSON.stringify(
        `IMPORT CORPO '${resolve(raiz, 'app/src/corpo.json')}' '${raiz}'`
      )}`,
      { stdio: ['ignore', 'inherit', 'inherit'], cwd: resolve(raiz, 'banco') }
    )
    writeFileSync(marcador, new Date().toISOString() + '\n')
  }
  return {
    name: 'serve-o-corpo',
    configureServer (server) { server.middlewares.use(mw) },
    configurePreviewServer (server) { server.middlewares.use(mw) },
    closeBundle () {
      ensureCorpoImportado()
      const dest = resolve(raiz, 'app/dist/corpo')
      let n = 0
      for (const rel of FICHEIROS) {
        const de = resolve(corpoRoot, rel)
        if (!existsSync(de)) throw new Error(`corpo: ${rel} falta no banco — IMPORT CORPO`)
        const para = resolve(dest, rel)
        mkdirSync(dirname(para), { recursive: true })
        copyFileSync(de, para)
        n++
      }
      console.log(`  corpo: ${n} ficheiros do banco → dist/corpo/`)
      const wasm = resolve(raiz, 'app/dist/wasm/tex.wasm')
      if (!existsSync(wasm))
        throw new Error('dist/wasm/tex.wasm em falta — corre tools/sobe_tex_wasm.sh antes do build')
      console.log(`  wasm: tex.wasm (${statSync(wasm).size} bytes)`)
    },
  }
  function mw (req, res, next) {
    const url = (req.url || '').split('?')[0]
    if (url === '/conecthus/backends/manifesto.json') {
      const p = resolve(raiz, 'conecthus/backends/manifesto.json')
      res.setHeader('Content-Type', 'application/json; charset=utf-8')
      return res.end(readFileSync(p))
    }
    if (url.startsWith('/conecthus/backends/')) {
      const rel = url.slice('/conecthus/backends/'.length).replace(/\?.*$/, '')
      if (rel && !rel.includes('..')) {
        const p = resolve(raiz, 'conecthus/backends', rel)
        if (existsSync(p) && statSync(p).isFile()) {
          const ext = rel.endsWith('.erg') ? 'text/plain; charset=utf-8' : 'application/octet-stream'
          res.setHeader('Content-Type', ext)
          return res.end(readFileSync(p))
        }
      }
    }
    if (!(req.url || '').startsWith('/corpo/')) return next()
    const rel = resolveNoCorpo(req.url)
    if (!rel) {
      res.statusCode = 404
      res.setHeader('Content-Type', 'text/plain; charset=utf-8')
      return res.end('não está no corpo: só sai o que tools/corpo.sh mediu que o tradutor abre.\n')
    }
    try { ensureCorpoImportado() } catch (e) {
      return erro(res, 'IMPORT CORPO falhou: ' + (e.message || e))
    }
    const cam = resolve(corpoRoot, rel)
    if (!existsSync(cam))
      return erro(res, `${rel} no manifesto mas ausente no banco (.torre/reino_corpo)`)
    const b = readFileSync(cam)
    res.setHeader('Content-Type', tipoDe(rel))
    res.setHeader('Content-Length', b.length)
    res.setHeader('X-Tiffany-Source', 'banco/sql IMPORT CORPO')
    res.end(b)
  }
}

// ── Antena local: WS /antena ↔ TCP 127.0.0.1:47314 (protocolo TFAL / banda) ──
// O browser fala o protocolo próprio; o Vite só é o fio até ao daemon `banco/fala`.
function antenaFala () {
  const port = Number(process.env.FALA_PORT || 47314)
  const host = process.env.FALA_HOST || '127.0.0.1'
  return {
    name: 'antena-fala',
    configureServer (server) {
      server.httpServer?.on('upgrade', (req, socket, head) => {
        const url = req.url || ''
        if (!url.startsWith('/antena')) return
        // Handshake WebSocket mínimo (RFC6455) — sem dependência `ws`.
        const key = req.headers['sec-websocket-key']
        if (!key) { socket.destroy(); return }
        const accept = crypto
          .createHash('sha1')
          .update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11')
          .digest('base64')
        socket.write(
          'HTTP/1.1 101 Switching Protocols\r\n' +
          'Upgrade: websocket\r\nConnection: Upgrade\r\n' +
          `Sec-WebSocket-Accept: ${accept}\r\n\r\n`
        )
        const tcp = net.connect({ host, port }, () => {
          let acc = Buffer.alloc(0)
          socket.on('data', (chunk) => {
            acc = Buffer.concat([acc, chunk])
            while (acc.length >= 2) {
              const b0 = acc[0], b1 = acc[1]
              const masked = (b1 & 0x80) !== 0
              let len = b1 & 0x7f
              let o = 2
              if (len === 126) {
                if (acc.length < 4) return
                len = acc.readUInt16BE(2); o = 4
              } else if (len === 127) {
                if (acc.length < 10) return
                len = Number(acc.readBigUInt64BE(2)); o = 10
              }
              const need = o + (masked ? 4 : 0) + len
              if (acc.length < need) return
              let payload = acc.subarray(o + (masked ? 4 : 0), need)
              if (masked) {
                const m = acc.subarray(o, o + 4)
                payload = Buffer.from(payload)
                for (let i = 0; i < payload.length; i++) payload[i] ^= m[i & 3]
              }
              acc = acc.subarray(need)
              const opcode = b0 & 0x0f
              if (opcode === 0x8) { tcp.end(); socket.end(); return }
              if (opcode === 0x9) {
                const pong = Buffer.alloc(2 + payload.length)
                pong[0] = 0x8a; pong[1] = payload.length
                payload.copy(pong, 2)
                socket.write(pong)
                continue
              }
              if (opcode === 0x2 || opcode === 0x1) tcp.write(payload)
            }
          })
          // TCP → WS: reúne frames TFAL (hdr 14 + len) antes de enviar
          let tcpAcc = Buffer.alloc(0)
          function wsBin (data) {
            let hdr
            if (data.length < 126) {
              hdr = Buffer.from([0x82, data.length])
            } else if (data.length < 65536) {
              hdr = Buffer.alloc(4)
              hdr[0] = 0x82; hdr[1] = 126; hdr.writeUInt16BE(data.length, 2)
            } else {
              hdr = Buffer.alloc(10)
              hdr[0] = 0x82; hdr[1] = 127; hdr.writeBigUInt64BE(BigInt(data.length), 2)
            }
            socket.write(Buffer.concat([hdr, data]))
          }
          tcp.on('data', (buf) => {
            tcpAcc = Buffer.concat([tcpAcc, buf])
            while (tcpAcc.length >= 14) {
              if (tcpAcc[0] !== 0x54 || tcpAcc[1] !== 0x46 ||
                  tcpAcc[2] !== 0x41 || tcpAcc[3] !== 0x4c) {
                tcpAcc = tcpAcc.subarray(1)
                continue
              }
              const plen = tcpAcc.readUInt32BE(10)
              const need = 14 + plen
              if (tcpAcc.length < need) return
              wsBin(tcpAcc.subarray(0, need))
              tcpAcc = tcpAcc.subarray(need)
            }
          })
          tcp.on('close', () => socket.end())
          tcp.on('error', () => socket.destroy())
          socket.on('close', () => tcp.destroy())
          socket.on('error', () => tcp.destroy())
        })
        tcp.on('error', (e) => {
          console.error('antena: daemon fala em', host + ':' + port, '—', e.message,
            '(cd banco && cc -O2 -std=c99 -Wall -I../lib -I. fala.c -o fala && ./fala)')
          socket.destroy()
        })
        if (head && head.length) socket.unshift(head)
      })
    },
  }
}

/** Envia frame WebSocket binário (RFC6455, sem máscara no servidor). */
function wsEnviaBin (socket, data) {
  const buf = Buffer.isBuffer(data) ? data : Buffer.from(data)
  let hdr
  if (buf.length < 126) {
    hdr = Buffer.from([0x82, buf.length])
  } else if (buf.length < 65536) {
    hdr = Buffer.alloc(4)
    hdr[0] = 0x82; hdr[1] = 126; hdr.writeUInt16BE(buf.length, 2)
  } else {
    hdr = Buffer.alloc(10)
    hdr[0] = 0x82; hdr[1] = 127; hdr.writeBigUInt64BE(BigInt(buf.length), 2)
  }
  socket.write(Buffer.concat([hdr, buf]))
}

// ── Canal S_CANAL: WS /canal ↔ UDP multicast (6 bytes bump-ados, sem traduzir) ──
// O browser fala bump+banda; o Vite é só o fio até o mesmo grupo que banco/sql.c.
function antenaCanal () {
  const grupo = process.env.TIFFANY_CANAL_GRUPO || '239.7.31.27'
  const porta = Number(process.env.TIFFANY_CANAL_PORTA || 47313)
  let udp = null
  const clientes = new Set()

  function ensureUdp () {
    if (udp) return udp
    udp = dgram.createSocket({ type: 'udp4', reuseAddr: true })
    udp.on('message', (msg) => {
      if (msg.length !== 6) return
      for (const s of clientes) {
        if (!s.destroyed) wsEnviaBin(s, msg)
      }
    })
    udp.on('error', (e) => console.error('canal udp:', e.message))
    udp.bind(porta, () => {
      try {
        udp.addMembership(grupo)
        console.log(`  canal: UDP ${grupo}:${porta} ↔ WS /canal (6 bytes bump)`)
      } catch (e) {
        console.warn('canal: multicast indisponível —', e.message)
      }
    })
    return udp
  }

  return {
    name: 'antena-canal',
    configureServer (server) {
      server.httpServer?.on('upgrade', (req, socket, head) => {
        const url = req.url || ''
        if (!url.startsWith('/canal')) return
        const key = req.headers['sec-websocket-key']
        if (!key) { socket.destroy(); return }
        const accept = crypto
          .createHash('sha1')
          .update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11')
          .digest('base64')
        socket.write(
          'HTTP/1.1 101 Switching Protocols\r\n' +
          'Upgrade: websocket\r\nConnection: Upgrade\r\n' +
          `Sec-WebSocket-Accept: ${accept}\r\n\r\n`
        )
        ensureUdp()
        clientes.add(socket)
        let acc = Buffer.alloc(0)
        socket.on('data', (chunk) => {
          acc = Buffer.concat([acc, chunk])
          while (acc.length >= 2) {
            const b0 = acc[0], b1 = acc[1]
            const masked = (b1 & 0x80) !== 0
            let len = b1 & 0x7f
            let o = 2
            if (len === 126) {
              if (acc.length < 4) return
              len = acc.readUInt16BE(2); o = 4
            } else if (len === 127) {
              if (acc.length < 10) return
              len = Number(acc.readBigUInt64BE(2)); o = 10
            }
            const need = o + (masked ? 4 : 0) + len
            if (acc.length < need) return
            let payload = acc.subarray(o + (masked ? 4 : 0), need)
            if (masked) {
              const m = acc.subarray(o, o + 4)
              payload = Buffer.from(payload)
              for (let i = 0; i < payload.length; i++) payload[i] ^= m[i & 3]
            }
            acc = acc.subarray(need)
            const opcode = b0 & 0x0f
            if (opcode === 0x8) { socket.end(); return }
            if (opcode === 0x9) {
              const pong = Buffer.alloc(2 + payload.length)
              pong[0] = 0x8a; pong[1] = payload.length
              payload.copy(pong, 2)
              socket.write(pong)
              continue
            }
            if ((opcode === 0x2 || opcode === 0x1) && payload.length === 6) {
              udp.send(payload, porta, grupo)
            }
          }
        })
        socket.on('close', () => clientes.delete(socket))
        socket.on('error', () => { clientes.delete(socket); socket.destroy() })
        if (head && head.length) socket.unshift(head)
      })
    },
  }
}

// ── SQL NO METAL: /sql executa banco/sql.c (Unix) ou MOVE no disco (Win) ──
function serveSql () {
  const raiz = fileURLToPath(new URL('..', import.meta.url))
  const sqlBin = resolve(raiz, 'app/.cache/tiffany_sql')
  const sqlBase = resolve(raiz, '.torre/reino')
  const discoMod = resolve(raiz, 'tools/banco_sql_disco.mjs')
  const isWin = process.platform === 'win32'
  function ensureSql () {
    if (isWin) {
      mkdirSync(sqlBase, { recursive: true })
      return
    }
    if (!existsSync(sqlBin)) {
      mkdirSync(dirname(sqlBin), { recursive: true })
      execSync(
        `cc -O2 -std=c99 -I${resolve(raiz, 'lib')} -I${resolve(raiz, 'banco')} ` +
        `${resolve(raiz, 'banco/sql.c')} -lm -o ${sqlBin}`,
        { stdio: 'inherit' }
      )
    }
    mkdirSync(sqlBase, { recursive: true })
  }
  async function runQuery (q, res) {
    try {
      ensureSql()
      if (isWin) {
        const { execSqlDisco } = await import(pathToFileURL(discoMod).href)
        const r = execSqlDisco(sqlBase, q)
        res.statusCode = 200
        res.setHeader('Content-Type', 'text/plain; charset=utf-8')
        res.setHeader('X-Tiffany-Shell', 'disco')
        res.end(r.out)
        return
      }
      const out = execSync(
        `"${sqlBin}" "${sqlBase}" ${JSON.stringify(q)}`,
        { encoding: 'utf8', maxBuffer: 16 * 1024 * 1024 }
      )
      res.statusCode = 200
      res.setHeader('Content-Type', 'text/plain; charset=utf-8')
      res.setHeader('X-Tiffany-Shell', 'sql.c')
      res.end(out)
    } catch (e) {
      const msg = (e.stdout || '') + (e.stderr || '') + (e.message || '')
      erro(res, msg.trim() || 'sql falhou')
    }
  }
  return {
    name: 'serve-sql',
    configureServer (server) { server.middlewares.use(mw) },
    configurePreviewServer (server) { server.middlewares.use(mw) },
  }
  function mw (req, res, next) {
    const url = req.url || ''
    if (url === '/sql' || url.startsWith('/sql?')) {
      if (req.method === 'GET') {
        const q = new URL(url, 'http://local').searchParams.get('q')
        if (!q) return erro(res, 'falta ?q= — a query SQL')
        return runQuery(q, res)
      }
      if (req.method === 'POST') {
        const chunks = []
        req.on('data', c => chunks.push(c))
        req.on('end', () => {
          const body = Buffer.concat(chunks).toString('utf8')
          let q = body
          try {
            const j = JSON.parse(body)
            if (j && typeof j.q === 'string') q = j.q
          } catch (e) { /* corpo cru = query */ }
          if (!q.trim()) return erro(res, 'corpo vazio — envie a query SQL')
          runQuery(q.trim(), res)
        })
        return
      }
      res.statusCode = 405
      res.end('use GET ?q= ou POST com a query\n')
      return
    }
    next()
  }
}

// ── Front via banco: /banco/ (html+css+js wasm) ──
function serveBancoFront () {
  const bancoDir = fileURLToPath(new URL('./banco', import.meta.url))
  const tipos = { html: 'text/html', css: 'text/css', js: 'text/javascript' }
  return {
    name: 'serve-banco-front',
    configureServer (server) { server.middlewares.use(mw) },
    configurePreviewServer (server) { server.middlewares.use(mw) },
    closeBundle () {
      const raiz = fileURLToPath(new URL('..', import.meta.url))
      const dest = resolve(raiz, 'app/dist/banco')
      for (const nome of ['pagina.html', 'pagina.css', 'pagina.js']) {
        const de = resolve(bancoDir, nome)
        if (!existsSync(de)) throw new Error(`banco front: falta ${nome}`)
        const para = resolve(dest, nome)
        mkdirSync(dirname(para), { recursive: true })
        copyFileSync(de, para)
      }
      console.log('  banco: pagina.html/css/js → dist/banco/')
    },
  }
  function mw (req, res, next) {
    const url = (req.url || '').split('?')[0]
    if (url === '/banco') {
      res.statusCode = 302
      res.setHeader('Location', '/banco/')
      return res.end()
    }
    if (url === '/banco/node_path') {
      const p = resolve(bancoDir, 'node_pagina.mjs')
      res.setHeader('Content-Type', 'application/json; charset=utf-8')
      return res.end(JSON.stringify({ path: p, dir: bancoDir, node: process.execPath }))
    }
    const m = url.match(/^\/banco\/(pagina\.(html|css|js))$/)
    if (!m) return next()
    const cam = resolve(bancoDir, m[1])
    if (!existsSync(cam)) {
      res.statusCode = 404
      return res.end('não encontrado\n')
    }
    res.setHeader('Content-Type', `${tipos[m[2]]}; charset=utf-8`)
    res.end(readFileSync(cam))
  }
}

export default defineConfig({
  plugins: [serveOCorpo(), serveSql(), antenaFala(), antenaCanal(), serveBancoFront()],
  base: './',
  build: {
    rollupOptions: {
      input: {
        main: fileURLToPath(new URL('./index.html', import.meta.url)),
        banco: fileURLToPath(new URL('./banco/index.html', import.meta.url)),
      },
    },
  },
  // publicDir: /reino/*.png, /wasm/tex.wasm — o tradutor no cliente, não PDFs pré-gravados
  publicDir: fileURLToPath(new URL('../assets/figuras', import.meta.url)),
  server: { open: true },
  define: { __ENREDO_ATUALIZADO__: JSON.stringify(dataEnredo()) },
})
