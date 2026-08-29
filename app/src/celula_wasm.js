// celula_wasm.js — wasm do backend + celula.erg + erg.fita embutidos (cadeia física).

import { manifestoAtual } from './manifesto_loader.js'
import { celulaDeWasm, secErgNome, SEC_FITA } from './wasm_sec_browser.js'

export { secErgNome, SEC_FITA, celulaDeWasm, extraiSecaoWasm } from './wasm_sec_browser.js'
export { correBackendMetal, loadIsaWasm, parseConstsErg, arenaSeeds } from './corre_metal_browser.js'

/**
 * Carrega wasm com secções custom (gera_nucleo + wasm_sec cadeia).
 * @returns {{ exports, celula, wasm, L }}
 */
export async function loadWasmCelula (nome, wasmBase = '/wasm/') {
  const L = manifestoAtual().linguagens.find((l) => l.nome === nome)
  if (!L) throw new Error('backend «' + nome + '» ausente')
  const r = await fetch(wasmBase + L.wasm)
  if (!r.ok) throw new Error('wasm ' + L.wasm + ': ' + r.status)
  const wasm = new Uint8Array(await r.arrayBuffer())
  const celula = celulaDeWasm(wasm, nome)
  const { instance } = await WebAssembly.instantiate(wasm)
  return { exports: instance.exports, celula, fita: celula.fita, wasm, L }
}

/** Verifica cadeia asm embutida (não bloqueia se ausente — wasm limpo de dev). */
export function validaCelula (nome, celula) {
  const move = nome + '_move'
  if (!celula?.erg) return { ok: false, falta: 'erg' }
  if (!celula.erg.includes(move)) return { ok: false, falta: move }
  if (!celula.temFita) return { ok: false, falta: SEC_FITA }
  return { ok: true, fitaLen: celula.fitaLen, secErg: celula.secErg }
}
