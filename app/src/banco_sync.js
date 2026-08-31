// banco_sync.js — disco da M_wasm (LS) ↔ disco da M_remota (gk/banco/estado.json) via canal S_ESTADO.
// Mesmo JSON, duas realizações. Sem addr+pub = solo (só M_wasm). Não é fallback offline.

import { enviaChunks, recebeCorpo, S_ESTADO_REQ, S_ESTADO_RSP } from './canal_browser.js'
import {
  leEstado, gravaEstado, mergeEstado, cortaContexto, estadoVazio,
  normalizaEstado, limpaPendente, enfileiraPendente, LIMITE_WIRE, LIMITE_OMISSAO,
} from './banco_disco.js'

export { S_ESTADO_REQ, S_ESTADO_RSP }

export async function estadoPuxa (canal) {
  const waiter = recebeCorpo(canal, S_ESTADO_RSP)
  await canal.grava(S_ESTADO_REQ, 0, 0)
  const txt = await waiter
  if (!txt) return estadoVazio()
  try {
    return normalizaEstado(JSON.parse(txt))
  } catch {
    return estadoVazio()
  }
}

export async function estadoEmpurra (canal, estado) {
  const body = normalizaEstado(JSON.parse(JSON.stringify(estado)))
  cortaContexto(body, LIMITE_WIRE, 'utf8')
  const json = JSON.stringify(body)
  const waiter = recebeCorpo(canal, S_ESTADO_RSP)
  await enviaChunks(canal, S_ESTADO_REQ, json)
  const txt = await waiter
  try {
    return normalizaEstado(JSON.parse(txt))
  } catch {
    return body
  }
}

/**
 * estado remoto é eventual; DOM não depende dele.
 * Timeout 20 s em recebeCorpo fica como protecção — fora do paint.
 */
export function sincronizaEmFundo (canal, storage, opts = {}) {
  if (!canal) return Promise.resolve({ estado: leEstado(storage), via: 'solo' })
  return Promise.resolve()
    .then(() => sincroniza(canal, storage, opts))
    .catch(() => ({ estado: leEstado(storage), via: 'wasm' }))
}

/**
 * Puxa a realização remota, funde com o disco local (t por shell), grava LS, empurra.
 * Sem canal: só M_wasm. Canal sem resposta: disco local fica; pendente marca.
 * Não chamar no caminho crítico do paint — usar sincronizaEmFundo.
 */
export async function sincroniza (canal, storage, opts = {}) {
  const local = leEstado(storage)
  if (!canal) {
    gravaEstado(local, storage, opts)
    return { estado: local, via: 'solo' }
  }
  let remoto
  try {
    remoto = await estadoPuxa(canal)
  } catch {
    gravaEstado(local, storage, opts)
    return { estado: local, via: 'wasm' }
  }
  const merged = mergeEstado(local, remoto)
  cortaContexto(merged, opts.limite ?? LIMITE_OMISSAO, 'utf16')
  cortaContexto(merged, LIMITE_WIRE, 'utf8')
  gravaEstado(merged, storage, opts)
  try {
    const echo = await estadoEmpurra(canal, merged)
    const final = mergeEstado(merged, echo)
    limpaPendente(final)
    gravaEstado(final, storage, opts)
    return { estado: final, via: 'sync' }
  } catch (e) {
    enfileiraPendente(merged, { nome: '_sync', in: String(e.message || e) })
    gravaEstado(merged, storage, opts)
    return { estado: merged, via: 'local-pendente' }
  }
}
