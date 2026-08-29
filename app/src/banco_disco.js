// banco_disco.js — GKBANCO no localStorage (mesmo padrão GKCORPO do Reino Dourado).
// Browser: localStorage = DISCO. Servidor: .torre/reino_*/disco.bin (banco_wasm.mjs).
// Isomorfismo dos dois lados — sem DRAM como persistência.

import { memoriaLS } from './corpo_disco.js'

export const MAGIA = 'GKBANCO'
export const CHAVE_ESTADO = 'gk:banco:estado'

/** DISCO do lado browser — localStorage por omissão; Map isomórfico em Node/testes. */
export function discoBrowser (opts = {}) {
  if (opts.storage != null) return opts.storage
  if (typeof globalThis.localStorage !== 'undefined') return globalThis.localStorage
  return memoriaLS()
}

export function estadoVazio () {
  return {
    magia: MAGIA,
    v: 1,
    shells: {},
    atualizado: null,
  }
}

export function leEstado (storage = globalThis.localStorage) {
  if (!storage) return estadoVazio()
  try {
    const s = storage.getItem(CHAVE_ESTADO)
    if (!s) return estadoVazio()
    const o = JSON.parse(s)
    if (o.magia !== MAGIA || !o.shells) return estadoVazio()
    return o
  } catch {
    return estadoVazio()
  }
}

export function gravaEstado (estado, storage = globalThis.localStorage) {
  if (!storage) return
  estado.atualizado = new Date().toISOString()
  storage.setItem(CHAVE_ESTADO, JSON.stringify(estado))
}

export function gravaShell (estado, nome, entrada, saida) {
  estado.shells[nome] = {
    in: entrada ?? estado.shells[nome]?.in ?? '',
    out: saida ?? estado.shells[nome]?.out ?? '',
  }
}

export function leShell (estado, nome) {
  return estado.shells[nome] || { in: '', out: '' }
}
