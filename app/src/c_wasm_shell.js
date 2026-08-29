// c_wasm_shell.js — cadeia C → wasm → shell (duomorfismo, fisica.tex §fis:def:duomorf).
// Reexporta node; generaliza para bash/node/powershell.

import { manifestoAtual } from './manifesto_loader.js'
import { paridade, assinaturaParidade } from './banco_tradutor.js'
import { absorveBackend } from './banco_absorve.js'

const SHELLS = ['bash', 'node', 'powershell']

export function cadeiaShell (nome, man = manifestoAtual()) {
  const L = man.linguagens.find((l) => l.nome === nome)
  return L?.cadeia || null
}

export function capasCadeia (nome = 'node', man = manifestoAtual()) {
  const c = cadeiaShell(nome, man)
  if (!c) {
    const L = man.linguagens.find((l) => l.nome === nome)
    const celula = { nome: nome + '_celula', p: L?.p ?? 1, q: L?.q ?? 1, r: L?.r ?? 0 }
    const metal = { nome: nome + '_pleno', p: L?.p ?? 1, q: L?.q ?? 1, r: 1 }
    return { celula, metal, fonte: L?.fonte, wasm: L?.wasm, pleno: L?.pleno }
  }
  const celula = c.celula || { p: 1, q: 1, r: 0, nome: nome + '_celula' }
  const metal = c.metal || { p: 1, q: 1, r: 1, nome: nome + '_pleno' }
  return { celula, metal, traduz: c.traduz, fonte: c.c, wasm: c.wasm, pleno: c.pleno }
}

export function pi (L) {
  return assinaturaParidade(L)
}

/** Paridade duomórfica célula wasm ↔ pleno metal (bit b = cruzado r). */
export function paridadeWasmMetal (nome = 'node', man = manifestoAtual()) {
  const { celula, metal } = capasCadeia(nome, man) || {}
  if (!celula || !metal) throw new Error('cadeia «' + nome + '» ausente')
  const a = pi(celula) ^ pi(metal)
  const b = (celula.r | 0) !== (metal.r | 0) ? 1 : 0
  return { a, b, quadrante: 'Q' + a + b, de: celula.nome, para: metal.nome }
}

export function paridadeCanalShell (nome, man = manifestoAtual()) {
  return paridade('canal', nome)
}

/** Absorção na cadeia: wasm MOVE(±1) + canal (b=1) + pleno. */
export async function traduzCadeia (backend, script, ctx = {}) {
  if (!SHELLS.includes(backend)) throw new Error('shell «' + backend + '»')
  const Q = paridadeWasmMetal(backend)
  const r = await absorveBackend(backend, script, ctx)
  const motor = r.meta?.motor ?? 'wasm'
  const seq = ['c', 'wasm']
  if (motor === 'fita') seq.push('asm', 'isa', backend + '_pleno')
  else seq.push('asm', ctx.canal && ctx.remoto ? 'canal' : null, backend + '_pleno')
  return {
    texto: r.out,
    par: Q,
    cadeia: seq.filter(Boolean),
    meta: r.meta,
  }
}

export async function traduzCadeiaNode (script, ctx = {}) {
  return traduzCadeia('node', script, ctx)
}

export { absorveBackend as moveShellArena, paridadeWasmMetal as paridadeWasmMetalNode, cadeiaShell as cadeiaNode }
