// banco_glsl_u.js — F5: GLSL/WASM sob demanda, não paint.
// GLSL catalogado ≠ WASM carregado ≠ kernel compilado ≠ kernel executado.
// Card (F3) dispara. Não puxa substratos → motor_campo → motor_wasm → kernels.
// Não toca banco_front / banco_disco / S_ESTADO.

import { completa } from './banco_schema.js'
import { acaoDoCard } from './banco_cards_u.js'

export const ID_GLSL_GK = 'gk'
export const CHAVE_GLSL = 'gk:reino:glsl'
export const FONTE_GLSL = './motor_campo.js'
export const FONTE_WASM = './motor_wasm.js'
export const WASM_PAINEL = '/wasm/painel_motor.wasm'
export const KERNEL_PRIMEIRO = 'aura'
/** Vertex canónico — o mesmo nos três operadores do original. */
export const VS_CANONICO = '#version 300 es\nin vec2 pos; void main(){ gl_Position = vec4(pos, 0.0, 1.0); }'
/** Wrap bit a bit do relógio (motor_wasm.js). */
export const ESCALA_FASE = 1 << 20

const CADEIA = Object.freeze([
  './substratos.js', './motor_campo.js', './motor_wasm.js',
  './cards_kernel.js', './cards_campo.js', './kernels_campo.json', './trailer_campo.js',
])

let modulo = null
let nWasm = 0
let nCompile = 0
let nExec = 0
const compilados = new Map()

export function cadeiaProibida () { return CADEIA }

export function resetGlsl () {
  modulo = null
  nWasm = 0
  nCompile = 0
  nExec = 0
  compilados.clear()
}

export function faseGlsl () {
  if (nExec) return 'execucao'
  if (nCompile) return 'compilacao'
  if (nWasm) return 'carga'
  return 'existencia'
}

export function fetchesPainelWasm () { return nWasm }
export function kernelsCompilados () { return nCompile }
export function kernelsExecutados () { return nExec }
export function moduloGlslCarregado () { return !!modulo }

export function catalogoGlsl (manKernels, indice) {
  const usados = kernelsEmitidos(indice)
  const set = new Set(usados)
  return (manKernels || []).map((k) => {
    const nome = String((k && k.nome) || k || '')
    return {
      nome,
      op: String((k && k.op) || ''),
      triade: String((k && k.triade) || ''),
      emitido: set.has(nome),
    }
  })
}

export function kernelsEmitidos (indice) {
  const s = new Set()
  for (const p of Object.values(indice || {})) {
    if (p && p.kernel) s.add(String(p.kernel))
  }
  return [...s].sort()
}

export function igualKernel (a, b) {
  if (!a || !b) return a === b
  return a.nome === b.nome && a.emitido === b.emitido
}

export function igualCatalogoGlsl (a, b) {
  if (!a || !b || a.length !== b.length) return false
  return a.every((k, i) => igualKernel(k, b[i]))
}

export function querGlsl (gatilho) {
  if (!gatilho) return false
  if (typeof gatilho === 'string') return !!gatilho
  if (gatilho.kernel) return true
  if (Array.isArray(gatilho.chama) && gatilho.chama.includes('glsl')) return true
  if (gatilho.refs && gatilho.refs.glsl) return true
  if (gatilho.nome || gatilho.secao) return acaoDoCard(gatilho).chama.includes('glsl')
  return false
}

export function selecionaGlsl (storage, id) {
  const s = String(id || '')
  if (storage && s) {
    try { storage.setItem(CHAVE_GLSL, JSON.stringify({ id: s })) } catch { /* quota */ }
  }
  return s
}

export function leGlslSelecionado (storage) {
  if (!storage) return ''
  try {
    const o = JSON.parse(storage.getItem(CHAVE_GLSL) || 'null')
    return (o && o.id) || ''
  } catch {
    return ''
  }
}

export function resultadoGlsl (opts = {}) {
  return {
    kernel: String(opts.kernel || ''),
    compilou: !!opts.compilou,
    executou: !!opts.executou,
    vs: VS_CANONICO.startsWith('#version 300 es'),
    escala: ESCALA_FASE,
    wasm: nWasm,
  }
}

export function igualResultadoGlsl (a, b) {
  if (!a || !b) return a === b
  return a.kernel === b.kernel && a.compilou === b.compilou && a.executou === b.executou &&
    a.vs === b.vs && a.escala === b.escala
}

/** Carga: WASM do painel. Não compila GLSL. */
export async function carregaWasm (opts = {}) {
  if (!modulo) {
    const imp = opts.importar || (() => import('./motor_wasm.js'))
    modulo = await imp()
  }
  if (!nWasm) {
    nWasm++
    const fetchWasm = opts.fetchWasm || (async () => {
      const r = await fetch(WASM_PAINEL)
      return new Uint8Array(await r.arrayBuffer())
    })
    await fetchWasm()
  }
  return modulo
}

/** Compila o kernel pelo nome. O source GLSL fica no original (cards_kernel). */
export async function compilaKernel (nome, opts = {}) {
  await carregaWasm(opts)
  const k = String(nome || KERNEL_PRIMEIRO)
  if (!compilados.has(k)) {
    nCompile++
    const fn = opts.aoCompilar || ((n) => ({ kernel: n, ok: true }))
    compilados.set(k, await fn(k))
  }
  return compilados.get(k)
}

/** Executa o kernel já compilado. */
export async function executaKernel (nome, opts = {}) {
  const c = await compilaKernel(nome, opts)
  nExec++
  if (typeof opts.aoExecutar === 'function') await opts.aoExecutar(nome || KERNEL_PRIMEIRO, c)
  return resultadoGlsl({ kernel: nome || KERNEL_PRIMEIRO, compilou: !!c, executou: true })
}

/**
 * Card → GLSL → WASM → compile → execute, em passos.
 * Sem opts: só selecciona. Sem latex/glsl no gatilho → não carrega.
 */
export async function disparaGlsl (storage, gatilho, opts = {}) {
  const kernel = (gatilho && gatilho.kernel) || KERNEL_PRIMEIRO
  const id = (gatilho && gatilho.id) || kernel
  if (id) selecionaGlsl(storage, id)
  if (!querGlsl(gatilho)) {
    return { id, fase: faseGlsl(), wasm: nWasm, compilou: nCompile, executou: nExec }
  }
  if (opts.executar === true) {
    const r = await executaKernel(kernel, opts)
    return { id, fase: faseGlsl(), wasm: nWasm, compilou: nCompile, executou: nExec, resultado: r }
  }
  if (opts.compilar === true) {
    await compilaKernel(kernel, opts)
    return { id, fase: faseGlsl(), wasm: nWasm, compilou: nCompile, executou: nExec }
  }
  if (opts.carregar === true) {
    await carregaWasm(opts)
    return { id, fase: faseGlsl(), wasm: nWasm, compilou: nCompile, executou: nExec }
  }
  return { id, fase: faseGlsl(), wasm: nWasm, compilou: nCompile, executou: nExec }
}

export function glslParaU (cat) {
  const lista = Array.isArray(cat) ? cat : []
  const n = lista.filter((k) => k.emitido).length
  return completa({
    kind: 'pagina',
    id: ID_GLSL_GK,
    sentido: 0,
    formato: 'json',
    camada: 'capacidade',
    estatuto: 'realizado',
    evidencia: 'manifesto.kernels; indice pecas.kernel (nao o GLSL); painel_motor.wasm; tests/glsl_u.js',
    proibicao: 'GLSL catalogado != WASM carregado != kernel compilado != kernel executado; nao puxar substratos/motor_campo/kernels; nao tocar banco_front/disco/S_ESTADO',
    nota: 'n=' + lista.length + '; emitido=' + n + '; fase=' + faseGlsl() +
      '; wasm=' + nWasm + '; compile=' + nCompile + '; exec=' + nExec +
      '; vs=300; escala=' + ESCALA_FASE,
    slots: {
      existencia: 'realizado',
      carga: nWasm ? 'realizado' : 'tardia',
      compilacao: nCompile ? 'realizado' : 'tardia',
      execucao: nExec ? 'realizado' : 'tardia',
    },
    filhos: lista.map((k) => completa({
      kind: 'ficheiro',
      id: k.nome || 'kernel',
      sentido: 0,
      formato: 'json',
      estatuto: k.emitido ? 'realizado' : 'nao localizada',
      fonte: k.nome || '',
      evidencia: 'manifesto.kernels; cards_kernel.programa(nome) no original',
      proibicao: 'nao ingerir source GLSL; compile e pelo nome',
    })),
  })
}

export function uParaGlsl (u) {
  const nota = (u && u.nota) || ''
  const nm = nota.match(/n=([0-9]+)/)
  const em = nota.match(/emitido=([0-9]+)/)
  return {
    n: nm ? Number(nm[1]) : ((u && u.filhos) || []).length,
    emitido: em ? Number(em[1]) : 0,
    fase: (nota.match(/fase=([a-z]+)/) || [])[1] || '',
    wasm: Number((nota.match(/wasm=([0-9]+)/) || [])[1] || 0),
  }
}
