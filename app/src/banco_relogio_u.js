// banco_relogio_u.js — F6: contrato do pulso, não o ficheiro, não a cena.
// relógio conhecido ≠ módulo carregado ≠ consumidor registado ≠ tique.
// F6 não executa cena, card ou GLSL. rAF único fica no original; o motor tique é discreto.
// Não toca banco_front / banco_disco / S_ESTADO. Não reabre F0–F5.

import { completa } from './banco_schema.js'

export const ID_RELOGIO_GK = 'gk'
export const CHAVE_RELOGIO = 'gk:reino:relogio'
export const FONTE_RELOGIO = './relogio.js'
export const FONTE_AVANCA = './motor_wasm.js'
/** Ponto fixo do original (motor_wasm.js). */
export const ESCALA_FASE = 1 << 20
/** voltas/s com slider 1× — original. */
export const VEL_BASE = 0.15
/** Primeiro quadro do original (last=0). */
export const DT0 = 0.016
export const DT_MAX_MS = 100

const CADEIA = Object.freeze([
  './motor_campo.js', './motor_wasm.js', './cards_campo.js', './trailer_campo.js',
  './cards_kernel.js', './kernels_campo.json', './substratos.js', './tex_tradutor.js',
])

const MASK = BigInt(ESCALA_FASE - 1)
const MEIO = 1n << 19n
const VEL_OMISSAO = Object.freeze({ v: 2, tocando: true })

function estadoNovo () {
  return {
    modulo: null,
    faseFixo: 0n,
    nTique: 0,
    consumidores: [],
    nChama: 0,
  }
}

const porDisco = new Map()
let isolado = estadoNovo()

function estadoDe (storage) {
  if (!storage) return isolado
  if (!porDisco.has(storage)) porDisco.set(storage, estadoNovo())
  return porDisco.get(storage)
}

export function cadeiaProibida () { return CADEIA }

export function resetRelogio () {
  porDisco.clear()
  isolado = estadoNovo()
}

export function faseRelogio (storage) {
  const st = estadoDe(storage)
  if (st.nTique) return 'tique'
  if (st.consumidores.length) return 'registro'
  if (st.modulo) return 'carga'
  return 'existencia'
}

export function moduloRelogioCarregado (storage) { return !!estadoDe(storage).modulo }
export function tiquesRelogio (storage) { return estadoDe(storage).nTique }
export function consumidoresRelogio (storage) { return estadoDe(storage).consumidores.length }
export function chamadasConsumidor (storage) { return estadoDe(storage).nChama }
export function faseDoPulso (storage) {
  return Number(estadoDe(storage).faseFixo) / ESCALA_FASE
}

export function catalogoRelogio () {
  return {
    fonte: FONTE_RELOGIO,
    avanca: FONTE_AVANCA,
    escala: ESCALA_FASE,
    velBase: VEL_BASE,
    dt0: DT0,
    dtMaxMs: DT_MAX_MS,
    fontesRaf: 1,
    ordem: 'avanca-depois-consumidores',
    cena: false,
    glsl: false,
  }
}

export function igualCatalogoRelogio (a, b) {
  if (!a || !b) return a === b
  return a.fonte === b.fonte && a.escala === b.escala && a.velBase === b.velBase &&
    a.dt0 === b.dt0 && a.fontesRaf === b.fontesRaf && a.ordem === b.ordem &&
    a.cena === b.cena && a.glsl === b.glsl
}

/** Fallback JS de avancaFase (painel ausente) — o wrap é AND. */
export function faseAposTique (faseFixo, dt, vel) {
  const v = vel || VEL_OMISSAO
  let f = BigInt(faseFixo)
  if (!v.tocando) return f
  const omega = Number(v.v) * VEL_BASE
  const of = Math.round(omega * ESCALA_FASE)
  const df = Math.round(Number(dt) * ESCALA_FASE)
  return (f + ((BigInt(of) * BigInt(df) + MEIO) >> 20n)) & MASK
}

export function querRelogio (gatilho) {
  if (!gatilho) return false
  if (gatilho.relogio === true || gatilho.pulso === true) return true
  const id = String(gatilho.id || '')
  return id === 'relogio' || id === 'pulso'
}

export function selecionaRelogio (storage, id) {
  const s = String(id || '')
  if (storage && s) {
    try { storage.setItem(CHAVE_RELOGIO, JSON.stringify({ id: s })) } catch { /* quota */ }
  }
  return s
}

export function leRelogioSelecionado (storage) {
  if (!storage) return ''
  try {
    const o = JSON.parse(storage.getItem(CHAVE_RELOGIO) || 'null')
    return (o && o.id) || ''
  } catch {
    return ''
  }
}

export function resultadoRelogio (storage, opts = {}) {
  const st = estadoDe(storage)
  return {
    fase: faseDoPulso(storage),
    tiques: st.nTique,
    consumidores: st.consumidores.length,
    chamadas: st.nChama,
    escala: ESCALA_FASE,
    wasm: 0,
    cena: false,
    glsl: false,
    compilou: !!opts.compilou,
  }
}

export function igualResultadoRelogio (a, b) {
  if (!a || !b) return a === b
  return a.fase === b.fase && a.tiques === b.tiques && a.escala === b.escala &&
    a.wasm === b.wasm && a.cena === b.cena && a.glsl === b.glsl
}

/** Carga: import() do orquestrador. Não inicia rAF. Não puxa WASM. */
export async function carregaRelogio (opts = {}) {
  const st = estadoDe(opts.storage)
  if (!st.modulo) {
    const imp = opts.importar || (() => import('./relogio.js'))
    st.modulo = await imp()
  }
  return st.modulo
}

/** Registo: consumidor sem executar capacidade. */
export function registaConsumidor (fn, opts = {}) {
  const st = estadoDe(opts.storage)
  if (typeof fn === 'function') st.consumidores.push(fn)
  return st.consumidores.length
}

/** Um avanço = um evento. Fase como o fallback de avancaFase. Consumidores depois. */
export function umTique (opts = {}) {
  const st = estadoDe(opts.storage)
  const dt = opts.dt == null ? DT0 : Number(opts.dt)
  const vel = opts.vel || VEL_OMISSAO
  st.faseFixo = faseAposTique(st.faseFixo, dt, vel)
  st.nTique++
  const lista = st.consumidores.slice()
  for (let i = 0; i < lista.length; i++) {
    lista[i]()
    st.nChama++
  }
  return resultadoRelogio(opts.storage)
}

/**
 * Relógio → carga → registo → tique, em passos.
 * Sem opts: só selecciona. Card/GLSL/LaTeX não disparam o pulso.
 * opts.carregar / opts.registar / opts.tique são booleanos; aoConsumir é callback.
 */
export async function disparaRelogio (storage, gatilho, opts = {}) {
  const id = (gatilho && gatilho.id) || 'relogio'
  if (id) selecionaRelogio(storage, id)
  const o = { ...opts, storage }
  if (!querRelogio(gatilho)) {
    return { id, fase: faseRelogio(storage), tiques: tiquesRelogio(storage), wasm: 0 }
  }
  if (opts.carregar === true) await carregaRelogio(o)
  if (opts.registar === true) {
    const fn = typeof opts.aoConsumir === 'function' ? opts.aoConsumir : function () {}
    registaConsumidor(fn, o)
  }
  if (opts.tique === true) {
    const r = umTique(o)
    return {
      id,
      fase: faseRelogio(storage),
      tiques: tiquesRelogio(storage),
      wasm: 0,
      resultado: r,
    }
  }
  return {
    id,
    fase: faseRelogio(storage),
    tiques: tiquesRelogio(storage),
    wasm: 0,
    modulo: moduloRelogioCarregado(storage),
    consumidores: consumidoresRelogio(storage),
  }
}

export function relogioParaU (cat) {
  const c = cat || catalogoRelogio()
  return completa({
    kind: 'pagina',
    id: ID_RELOGIO_GK,
    sentido: 0,
    formato: 'json',
    camada: 'capacidade',
    estatuto: 'realizado',
    evidencia: 'relogio.js iniciaRelogio; avancaFase wrap AND 2^20; tests/relogio_u.js',
    proibicao: 'relogio conhecido != modulo != consumidor != tique; F6 nao executa cena card ou GLSL; rAF unico no original; motor tique discreto; nao tocar banco_front/disco/S_ESTADO',
    nota: 'fonte=' + c.fonte + '; escala=' + c.escala + '; velBase=' + c.velBase +
      '; dt0=' + c.dt0 + '; fase=' + faseRelogio() +
      '; tiques=' + tiquesRelogio() + '; cons=' + consumidoresRelogio(),
    slots: {
      existencia: 'realizado',
      carga: moduloRelogioCarregado() ? 'realizado' : 'tardia',
      registro: consumidoresRelogio() ? 'realizado' : 'tardia',
      tique: tiquesRelogio() ? 'realizado' : 'tardia',
    },
    filhos: [
      completa({
        kind: 'ficheiro',
        id: 'pulso',
        sentido: 0,
        formato: 'json',
        estatuto: 'realizado',
        fonte: FONTE_RELOGIO,
        evidencia: 'um rAF no original; avancaFase depois consumidores',
        proibicao: 'nao ingerir relogio.js como ficheiro; nao puxar motor_campo/cards_campo/trailer',
      }),
    ],
  })
}

export function uParaRelogio (u) {
  const nota = (u && u.nota) || ''
  const em = nota.match(/escala=([0-9]+)/)
  const tm = nota.match(/tiques=([0-9]+)/)
  return {
    escala: em ? Number(em[1]) : ESCALA_FASE,
    tiques: tm ? Number(tm[1]) : 0,
    fase: (nota.match(/fase=([a-z]+)/) || [])[1] || '',
    fonte: (nota.match(/fonte=([^;]+)/) || [])[1] || '',
  }
}
