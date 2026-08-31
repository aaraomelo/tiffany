/* canal_watcher.mjs — lado branco local: bump → MOVE na arena DISCO (pleno C). */

import { readFileSync, writeFileSync, mkdirSync } from 'node:fs'

import { join, dirname } from 'node:path'

import { tramaBump, tramaClara } from './banco_banda.mjs'

import { execMoveMetal } from './banco_metal.mjs'

import {

  S_CHUNK, S_BASH_IN, S_BASH_OUT, S_PWSH_IN, S_PWSH_OUT,

  S_NODE_IN, S_NODE_OUT, S_FRONT_REQ, S_FRONT_RSP,

  S_ESTADO_REQ, S_ESTADO_RSP,
  S_DEPOSITO_REQ, S_DEPOSITO_RSP,

} from './canal_slots.mjs'



function escSql (s) {

  return String(s).replace(/'/g, "''")

}



function enviaCorpo (broadcast, banda, slotFim, buf) {

  const b = Buffer.isBuffer(buf) ? buf : Buffer.from(buf, 'utf8')

  for (let i = 0; i < b.length; i += 2) {

    broadcast(tramaBump(S_CHUNK, b[i], i + 1 < b.length ? b[i + 1] : 0, banda))

  }

  broadcast(tramaBump(slotFim, b.length & 255, (b.length >> 8) & 255, banda))

}



export function createCanalWatcher ({ sqlBase, banda, broadcast, bancoDir }) {

  const chunk = []

  const frontPaths = ['pagina.html', 'pagina.css', 'pagina.js']

  if (!sqlBase) throw new Error('canal_watcher: sqlBase DISCO em falta (.torre/reino)')



  function shellMove (backend, script) {
    const r = execMoveMetal(sqlBase, backend, script)
    return r.stdout
  }



  function onSlotIn (backend, outSlot, nin) {

    const n = Math.min(nin, chunk.length)

    const body = Buffer.from(chunk.splice(0, n)).toString('utf8')

    chunk.length = 0

    try {

      const out = shellMove(backend, body)

      enviaCorpo(broadcast, banda, outSlot, out)

    } catch (e) {

      enviaCorpo(broadcast, banda, outSlot, String(e.message || e))

    }

  }



  function serveFront (kind) {

    const rel = frontPaths[kind]

    if (!rel) {

      enviaCorpo(broadcast, banda, S_FRONT_RSP, '')

      return

    }

    try {

      const body = readFileSync(join(bancoDir, rel), 'utf8')

      enviaCorpo(broadcast, banda, S_FRONT_RSP, body)

    } catch {

      enviaCorpo(broadcast, banda, S_FRONT_RSP, '')

    }

  }

  const ESTADO_VAZIO = '{"magia":"GKBANCO","v":2,"shells":{},"atualizado":null,"pendente":[]}'

  function estadoPath () {

    return join(sqlBase, 'gk', 'banco', 'estado.json')

  }

  function serveEstado (nin) {

    const path = estadoPath()

    mkdirSync(dirname(path), { recursive: true })

    if (nin <= 0) {

      chunk.length = 0

      let body = ESTADO_VAZIO

      try { body = readFileSync(path, 'utf8') } catch { /* disco vazio */ }

      enviaCorpo(broadcast, banda, S_ESTADO_RSP, body)

      return

    }

    const n = Math.min(nin, chunk.length)

    const body = Buffer.from(chunk.splice(0, n)).toString('utf8')

    chunk.length = 0

    try { writeFileSync(path, body) } catch { /* quota do host */ }

    enviaCorpo(broadcast, banda, S_ESTADO_RSP, body)

  }

  function depositoPath () {

    return join(sqlBase, 'gk', 'banco', 'deposito.bin')

  }

  /* D_patria: bytes opacos. Não parseia JSON. Sem a banda ≠ GKBANCO. */
  function serveDeposito (nin) {

    const path = depositoPath()

    mkdirSync(dirname(path), { recursive: true })

    if (nin <= 0) {

      chunk.length = 0

      let body = Buffer.alloc(0)

      try { body = readFileSync(path) } catch { /* disco vazio */ }

      enviaCorpo(broadcast, banda, S_DEPOSITO_RSP, body)

      return

    }

    const n = Math.min(nin, chunk.length)

    const body = Buffer.from(chunk.splice(0, n))

    chunk.length = 0

    try { writeFileSync(path, body) } catch { /* quota do host */ }

    enviaCorpo(broadcast, banda, S_DEPOSITO_RSP, body)

  }



  return function handleBump (bumped) {

    let t

    try {
      t = tramaClara(bumped, banda)
    } catch {

      return

    }

    if (t.slot === S_CHUNK) {

      chunk.push(t.total, t.e)

      return

    }

    if (t.slot === S_FRONT_REQ) {

      chunk.length = 0

      serveFront(t.total)

      return

    }

    if (t.slot === S_ESTADO_REQ) {

      const nin = t.total + (t.e << 8)

      serveEstado(nin)

      return

    }

    if (t.slot === S_DEPOSITO_REQ) {

      const nin = t.total + (t.e << 8)

      serveDeposito(nin)

      return

    }

    if (t.slot === S_BASH_IN) {

      const nin = t.total + (t.e << 8)

      onSlotIn('bash', S_BASH_OUT, nin)

      return

    }

    if (t.slot === S_NODE_IN) {

      const nin = t.total + (t.e << 8)

      onSlotIn('node', S_NODE_OUT, nin)

      return

    }

    if (t.slot === S_PWSH_IN) {

      const nin = t.total + (t.e << 8)

      onSlotIn('powershell', S_PWSH_OUT, nin)

    }

  }

}



/** Tramas de um corpo: pares S_CHUNK + 1 sinal no slot final. */

export function framesDeCorpo (n) {

  return Math.ceil(Math.max(0, n) / 2) + 1

}

/** §N1 negro.c: S_n = r^s, S_b = r^{-s}. r é a medida da ida, não o quociente tautológico. */

export function medeParN1 (r, s = 1) {

  if (!(r > 0) || !(s > 0)) return false

  const sNeg = Math.pow(r, s)

  const sBran = Math.pow(1 / r, s)

  return Math.abs(sNeg * sBran - 1) < 1e-9

}

/** Ida (ou volta): o que o negro emitiu o branco absorveu — as duas pontas da mesma trama. */

export function medeIdaCanal (nEmit, nAbsorve) {

  return nEmit > 0 && nEmit === nAbsorve

}

