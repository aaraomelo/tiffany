// corre_metal_browser.js — node_corre (erg.fita) no browser via isa.wasm + arena DISCO.

import { correFita } from './isa_fita.js'
import { loadWasmCelula } from './celula_wasm.js'

export const NULO_DISCO = 8
export const OFF_NIN = 24576
export const OFF_NOUT = 24578
export const OFF_IN = 256
export const OFF_OUT = 16384
export const RODATA_TAG = 65408
export const CAP = 8192

const RODATA = {
  node: 'console.log(',
  bash: 'echo ',
  powershell: 'Write-Output ',
}

let cacheIsa = null

export function parseConstsErg (ergText) {
  const seeds = []
  if (!ergText) return seeds
  for (const line of ergText.split('\n')) {
    const m = line.match(/^; CONST (\d+) (\d+)/)
    if (m) seeds.push([Number(m[1]), Number(m[2]), 0])
  }
  return seeds
}

export function arenaSeeds (script, backend) {
  const seeds = []
  const b = new TextEncoder().encode(script || '')
  const n = Math.min(b.length, CAP)
  for (let i = 0; i < n; i++) seeds.push([NULO_DISCO + OFF_IN + i, b[i], 0])
  seeds.push([NULO_DISCO + OFF_NIN, n & 255, 0])
  seeds.push([NULO_DISCO + OFF_NIN + 1, (n >> 8) & 255, 0])
  const tag = RODATA[backend] || ''
  for (let i = 0; i < tag.length; i++)
    seeds.push([NULO_DISCO + RODATA_TAG + i, tag.charCodeAt(i), 0])
  return seeds
}

export async function loadIsaWasm (wasmBase = '/wasm/') {
  if (cacheIsa) return cacheIsa
  const r = await fetch(wasmBase + 'isa.wasm')
  if (!r.ok) throw new Error('isa.wasm: ' + r.status)
  const { instance } = await WebAssembly.instantiate(await r.arrayBuffer())
  if (typeof instance.exports.MOVE !== 'function') throw new Error('isa: export MOVE em falta')
  cacheIsa = instance.exports
  return cacheIsa
}

export function limpaCacheIsa () {
  cacheIsa = null
}

export function leStdoutArena (runner) {
  const lo = Number(runner.leSlot(NULO_DISCO + OFF_NOUT)[0])
  const hi = Number(runner.leSlot(NULO_DISCO + OFF_NOUT + 1)[0])
  const len = lo + hi * 256
  if (len <= 0 || len > CAP) return ''
  const bytes = new Uint8Array(len)
  for (let i = 0; i < len; i++) bytes[i] = Number(runner.leSlot(NULO_DISCO + OFF_OUT + i)[0])
  return new TextDecoder().decode(bytes)
}

export async function fetchCorreErg (backend, base = '/conecthus/backends/') {
  const url = base + backend + '/' + backend + '_corre.erg'
  const r = await fetch(url)
  if (!r.ok) return ''
  return r.text()
}

/** Executa erg.fita embutida (paralelo a erg corre no metal). */
export async function correBackendMetal (backend, script, opts = {}) {
  const wasmBase = opts.wasmBase ?? '/wasm/'
  const pack = await (opts.wasmCelula ?? loadWasmCelula(backend, wasmBase))
  const fita = pack.celula?.fita ?? pack.fita
  if (!fita?.length) throw new Error('erg.fita em falta — corra gera_nucleo')

  const isa = await loadIsaWasm(wasmBase)
  const correErg = opts.correErg ?? await fetchCorreErg(backend, opts.ergBase)
  const seeds = [
    ...parseConstsErg(correErg),
    ...arenaSeeds(script, backend),
  ]

  const runner = correFita(isa.MOVE, fita, seeds, opts.maxSteps ?? 2_000_000)
  const out = leStdoutArena(runner)

  return {
    out,
    meta: {
      via: 'isa+fita+arena',
      backend,
      fitaLen: fita.length,
      passos: runner.passos,
      temAsm: !!pack.celula?.erg,
      secErg: pack.celula?.secErg,
    },
  }
}
