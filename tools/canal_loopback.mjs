/* canal_loopback.mjs — WS /canal loopback local (6 bytes bump, sem UDP). */
import crypto from 'node:crypto'

function wsEnviaBin (socket, buf) {
  const payload = Buffer.isBuffer(buf) ? buf : Buffer.from(buf)
  const frame = Buffer.alloc(2 + payload.length)
  frame[0] = 0x82
  frame[1] = payload.length
  payload.copy(frame, 2)
  socket.write(frame)
}

export function attachCanalLoopback (httpServer, onFrame) {
  const clientes = new Set()

  function broadcast (buf) {
    for (const s of clientes) {
      if (!s.destroyed) wsEnviaBin(s, buf)
    }
  }

  httpServer.on('upgrade', (req, socket, head) => {
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
    clientes.add(socket)
    let acc = Buffer.alloc(0)
    socket.on('data', (chunk) => {
      acc = Buffer.concat([acc, chunk])
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
          const pong = Buffer.alloc(2 + payload.length)
          pong[0] = 0x8a
          pong[1] = payload.length
          payload.copy(pong, 2)
          socket.write(pong)
          continue
        }
        if ((opcode === 0x2 || opcode === 0x1) && payload.length === 6) {
          if (onFrame) onFrame(payload)
          broadcast(payload)
        }
      }
    })
    socket.on('close', () => clientes.delete(socket))
    socket.on('error', () => { clientes.delete(socket); socket.destroy() })
    if (head && head.length) socket.unshift(head)
  })

  return { broadcast }
}
