/* canal_cliente.mjs — cliente WS /canal no Node (mesmo protocolo que canal_browser.js). */
import net from 'node:net'
import crypto from 'node:crypto'
import { bandaDeTecido, tramaBump, tramaClara } from './banco_banda.mjs'
import { S_CHUNK, S_NODE_IN, S_NODE_OUT, S_BASH_IN, S_BASH_OUT, S_PWSH_IN, S_PWSH_OUT } from './canal_slots.mjs'

const SLOTS_SHELL = {
  node: { in: S_NODE_IN, out: S_NODE_OUT },
  bash: { in: S_BASH_IN, out: S_BASH_OUT },
  powershell: { in: S_PWSH_IN, out: S_PWSH_OUT },
}

function wsFrame (opcode, payload) {
  const buf = Buffer.isBuffer(payload) ? payload : Buffer.from(payload)
  const frame = Buffer.alloc(2 + buf.length)
  frame[0] = opcode
  frame[1] = buf.length
  buf.copy(frame, 2)
  return frame
}

/** Abre sessão WS em /canal; banda = sha256(tecido) igual ao servidor. */
export function abrirCanalNode (port, opts = {}) {
  const host = opts.host || '127.0.0.1'
  const banda = opts.banda || bandaDeTecido(opts.tecido)
  const ouvintes = new Map()
  let socket = null
  let acc = Buffer.alloc(0)
  let aberto = false
  let abrePendente = null

  function emit (slot, total, e) {
    const set = ouvintes.get(slot)
    if (!set) return
    for (const fn of set) fn({ slot, total, e })
  }

  function grava (slot, total, e) {
    if (!aberto) return Promise.reject(new Error('canal fechado'))
    const frame = tramaBump(slot, total, e, banda)
    socket.write(wsFrame(0x82, frame))
    return Promise.resolve()
  }

  function on (slot, fn) {
    if (!ouvintes.has(slot)) ouvintes.set(slot, new Set())
    ouvintes.get(slot).add(fn)
    return () => { ouvintes.get(slot)?.delete(fn) }
  }

  function le (slot, timeoutMs = 20000) {
    return new Promise((resolve, reject) => {
      const to = setTimeout(() => {
        ouvintes.get(slot)?.delete(handler)
        reject(new Error('canal: timeout slot ' + slot))
      }, timeoutMs)
      function handler (t) {
        clearTimeout(to)
        ouvintes.get(slot)?.delete(handler)
        resolve(t)
      }
      on(slot, handler)
    })
  }

  function processaAcc () {
    while (acc.length >= 2) {
      const b0 = acc[0]
      const b1 = acc[1]
      const masked = (b1 & 0x80) !== 0
      let len = b1 & 0x7f
      let o = 2
      if (len === 126) {
        if (acc.length < 4) return
        len = acc.readUInt16BE(2)
        o = 4
      } else if (len === 127) {
        if (acc.length < 10) return
        len = Number(acc.readBigUInt64BE(2))
        o = 10
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
        socket.write(wsFrame(0x8a, payload))
        continue
      }
      if ((opcode === 0x2 || opcode === 0x1) && payload.length === 6) {
        try {
          const t = tramaClara(payload, banda)
          emit(t.slot, t.total, t.e)
        } catch { /* banda errada */ }
      }
    }
  }

  const canal = {
    grava,
    on,
    le,
    banda,
    bandaHex () { return banda.slice(0, 16).toString('hex') },
    fechar () { if (socket) socket.end(); socket = null; aberto = false },
    pronto () {
      if (aberto) return Promise.resolve()
      if (abrePendente) return abrePendente
      abrePendente = new Promise((resolve, reject) => {
        const key = crypto.randomBytes(16).toString('base64')
        socket = net.connect(port, host)
        socket.on('error', reject)
        socket.on('data', (chunk) => {
          acc = Buffer.concat([acc, chunk])
          if (!aberto) {
            const hdr = acc.toString('utf8')
            if (!hdr.includes('\r\n\r\n')) return
            if (!hdr.startsWith('HTTP/1.1 101')) {
              reject(new Error('upgrade WS falhou'))
              return
            }
            const rest = acc.subarray(hdr.indexOf('\r\n\r\n') + 4)
            acc = rest
            aberto = true
            resolve()
            processaAcc()
            return
          }
          processaAcc()
        })
        socket.write(
          `GET /canal HTTP/1.1\r\n` +
          `Host: ${host}:${port}\r\n` +
          `Upgrade: websocket\r\n` +
          `Connection: Upgrade\r\n` +
          `Sec-WebSocket-Key: ${key}\r\n` +
          `Sec-WebSocket-Version: 13\r\n\r\n`
        )
      })
      return abrePendente
    },
  }

  return canal
}

/** shellRemoto via canal — igual a banco_sql_interno.shellRemoto. */
export async function shellRemotoCanal (canal, script, backend = 'node', timeoutMs = 20000) {
  const sh = SLOTS_SHELL[backend]
  if (!sh) throw new Error('backend «' + backend + '»')
  await canal.pronto()
  const body = backend === 'bash' && script && !script.endsWith('\n') ? script + '\n' : script
  const b = Buffer.from(body, 'utf8')
  const parts = []
  const off = canal.on(S_CHUNK, ({ total, e }) => { parts.push(total, e) })
  try {
    for (let i = 0; i < b.length; i += 2) {
      await canal.grava(S_CHUNK, b[i], i + 1 < b.length ? b[i + 1] : 0)
    }
    const outWait = canal.le(sh.out, timeoutMs)
    await canal.grava(sh.in, b.length & 255, (b.length >> 8) & 255)
    const rsp = await outWait
    const len = rsp.total + (rsp.e << 8)
    const buf = Buffer.alloc(len)
    let bi = 0
    for (let pi = 0; bi < len && pi < parts.length; pi++) buf[bi++] = parts[pi] & 255
    return buf.toString('utf8', 0, bi)
  } finally {
    off()
  }
}
