// banco_sql_interno.js — slots e shellRemoto (sem tradutor; evita ciclo).

import {
  S_BASH_IN, S_BASH_OUT, S_CHUNK,
  S_PWSH_IN, S_PWSH_OUT, S_NODE_IN, S_NODE_OUT,
} from './canal_browser.js'

export function escSql (s) {
  return String(s).replace(/'/g, "''")
}

export const SHELLS = {
  bash: { label: 'BASH', slotIn: S_BASH_IN, slotOut: S_BASH_OUT },
  powershell: { label: 'POWERSHELL', slotIn: S_PWSH_IN, slotOut: S_PWSH_OUT },
  node: { label: 'NODE', slotIn: S_NODE_IN, slotOut: S_NODE_OUT },
}

export function shellPadrao () {
  return 'node'
}

export async function shellRemoto (script, canal, backend = 'bash', timeoutMs = 20000) {
  const sh = SHELLS[backend] || SHELLS.bash
  const body = backend === 'bash' && !script.endsWith('\n') ? script + '\n' : script
  const b = new TextEncoder().encode(body)
  const parts = []
  const off = canal.on(S_CHUNK, ({ total, e }) => { parts.push(total, e) })
  try {
    for (let i = 0; i < b.length; i += 2) {
      await canal.grava(S_CHUNK, b[i], i + 1 < b.length ? b[i + 1] : 0)
    }
    const outWait = canal.le(sh.slotOut, timeoutMs)
    await canal.grava(sh.slotIn, b.length & 255, (b.length >> 8) & 255)
    const rsp = await outWait
    const len = rsp.total + (rsp.e << 8)
    const buf = new Uint8Array(len)
    let bi = 0
    for (let pi = 0; bi < len && pi < parts.length; pi++) buf[bi++] = parts[pi] & 255
    return new TextDecoder().decode(buf.subarray(0, bi))
  } finally {
    off()
  }
}

export { S_BASH_IN, S_BASH_OUT, S_PWSH_IN, S_PWSH_OUT, S_NODE_IN, S_NODE_OUT }
