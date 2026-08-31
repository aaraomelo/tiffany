// banco_cena_u.js — F7: um host de cena no pulso F6. Não o motor_campo inteiro.
// host conhecido ≠ host carregado ≠ host registado ≠ host avançado pelo tique.
// F7 não compila GLSL, não busca WASM, não puxa trailer/cards_campo.
// Não toca banco_front / banco_disco / S_ESTADO. Não reabre F0–F6. Não mexe em relogio.js.

import { completa } from './banco_schema.js'
import {
  DT0, ESCALA_FASE,
  faseDoPulso, faseAposTique, umTique, registaConsumidor, tiquesRelogio,
} from './banco_relogio_u.js'

export const ID_CENA_GK = 'gk'
export const CHAVE_CENA = 'gk:reino:cena'
export const HOST_CANONICO = 'hero'
export const SELETOR_HOST = '.heroart'
export const PECA_HOST = 'coracao_revela'
export const FONTE_CENA = './motor_campo.js'
/** Assinatura do hero no original (main.js). */
export const C200_HERO = 9 / 200
export const ANTIFASE_HERO = Math.PI

const CADEIA = Object.freeze([
  './motor_campo.js', './motor_wasm.js', './cards_campo.js', './trailer_campo.js',
  './cards_kernel.js', './kernels_campo.json', './substratos.js', './pontes.js',
])

function estadoNovo () {
  return {
    carregado: false,
    registado: false,
    modulo: null,
    uTime: 0,
    quadros: 0,
    peca: PECA_HOST,
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

export function resetCena () {
  porDisco.clear()
  isolado = estadoNovo()
}

export function faseCena (storage) {
  const st = estadoDe(storage)
  if (st.quadros) return 'avanco'
  if (st.registado) return 'registro'
  if (st.carregado) return 'carga'
  return 'existencia'
}

export function hostCarregado (storage) { return estadoDe(storage).carregado }
export function hostRegistado (storage) { return estadoDe(storage).registado }
export function quadrosHost (storage) { return estadoDe(storage).quadros }
export function uTimeHost (storage) { return estadoDe(storage).uTime }

export function catalogoCena () {
  return {
    id: HOST_CANONICO,
    seletor: SELETOR_HOST,
    peca: PECA_HOST,
    fonte: FONTE_CENA,
    c200: C200_HERO,
    antifase: ANTIFASE_HERO,
    n: 1,
    trailer: false,
    cards: false,
    glsl: false,
    wasm: false,
  }
}

export function igualCatalogoCena (a, b) {
  if (!a || !b) return a === b
  return a.id === b.id && a.seletor === b.seletor && a.peca === b.peca &&
    a.c200 === b.c200 && a.antifase === b.antifase && a.n === b.n &&
    a.trailer === b.trailer && a.cards === b.cards && a.glsl === b.glsl && a.wasm === b.wasm
}

export function querCena (gatilho) {
  if (!gatilho) return false
  if (gatilho.cena === true) return true
  if (gatilho.host === HOST_CANONICO) return true
  const id = String(gatilho.id || '')
  return id === HOST_CANONICO || id === 'cena'
}

export function selecionaCena (storage, id) {
  const s = String(id || HOST_CANONICO)
  if (storage && s) {
    try { storage.setItem(CHAVE_CENA, JSON.stringify({ id: s })) } catch { /* quota */ }
  }
  return s
}

export function leCenaSelecionada (storage) {
  if (!storage) return ''
  try {
    const o = JSON.parse(storage.getItem(CHAVE_CENA) || 'null')
    return (o && o.id) || ''
  } catch {
    return ''
  }
}

export function resultadoCena (storage) {
  const st = estadoDe(storage)
  return {
    id: HOST_CANONICO,
    peca: st.peca,
    uTime: st.uTime,
    quadros: st.quadros,
    tiques: tiquesRelogio(storage),
    escala: ESCALA_FASE,
    wasm: 0,
    compilou: false,
    trailer: false,
    cards: false,
  }
}

export function igualResultadoCena (a, b) {
  if (!a || !b) return a === b
  return a.uTime === b.uTime && a.quadros === b.quadros && a.tiques === b.tiques &&
    a.wasm === b.wasm && a.compilou === b.compilou && a.peca === b.peca
}

/** Carga: instancia o host. Não importa motor_campo. Não compila GLSL. */
export async function carregaHost (opts = {}) {
  const st = estadoDe(opts.storage)
  if (!st.carregado) {
    st.carregado = true
    if (typeof opts.importar === 'function') st.modulo = await opts.importar()
  }
  return st
}

/** Registo: o host lê a fase depois do tique F6. Sem draw, sem compile. */
export function registaHost (opts = {}) {
  const storage = opts.storage
  const st = estadoDe(storage)
  st.carregado = true
  if (st.registado) return st
  st.registado = true
  registaConsumidor(() => {
    const h = estadoDe(storage)
    h.uTime = faseDoPulso(storage)
    h.quadros++
  }, { storage })
  return st
}

/** Um tique F6: se o host está registado, u_time acompanha a fase. */
export function avancaHost (opts = {}) {
  const r = umTique({ ...opts, storage: opts.storage })
  return { relogio: r, host: resultadoCena(opts.storage) }
}

/**
 * Cena → carga → registo → tique, em passos.
 * Sem opts: só selecciona. Pulso/card/LaTeX não disparam a cena.
 */
export async function disparaCena (storage, gatilho, opts = {}) {
  const id = (gatilho && (gatilho.id || gatilho.host)) || HOST_CANONICO
  if (id) selecionaCena(storage, id)
  const o = { ...opts, storage }
  if (!querCena(gatilho)) {
    return { id, fase: faseCena(storage), quadros: quadrosHost(storage), wasm: 0 }
  }
  if (opts.carregar === true) await carregaHost(o)
  if (opts.registar === true) registaHost(o)
  if (opts.tique === true) {
    const r = avancaHost(o)
    return {
      id,
      fase: faseCena(storage),
      quadros: quadrosHost(storage),
      wasm: 0,
      resultado: r.host,
    }
  }
  return {
    id,
    fase: faseCena(storage),
    quadros: quadrosHost(storage),
    wasm: 0,
    carregado: hostCarregado(storage),
    registado: hostRegistado(storage),
  }
}

export function cenaParaU (cat) {
  const c = cat || catalogoCena()
  return completa({
    kind: 'pagina',
    id: ID_CENA_GK,
    sentido: 0,
    formato: 'json',
    camada: 'capacidade',
    estatuto: 'realizado',
    evidencia: 'main.js .heroart + coracao_revela; initMotorCampo; u_time = faseDoMotor; tests/cena_u.js',
    proibicao: 'host conhecido != carregado != registado != avancado pelo tique; F7 = F6 + um host; nao motor_campo inteiro; nao compile GLSL; nao WASM novo; nao trailer; nao cards_campo; nao tocar banco_front/disco/S_ESTADO; nao mexer relogio.js',
    nota: 'id=' + c.id + '; peca=' + c.peca + '; seletor=' + c.seletor +
      '; c200=' + c.c200 + '; fase=' + faseCena() +
      '; quadros=' + quadrosHost() + '; uTime=' + uTimeHost(),
    slots: {
      existencia: 'realizado',
      carga: hostCarregado() ? 'realizado' : 'tardia',
      registro: hostRegistado() ? 'realizado' : 'tardia',
      avanco: quadrosHost() ? 'realizado' : 'tardia',
    },
    filhos: [
      completa({
        kind: 'ficheiro',
        id: c.id,
        sentido: 0,
        formato: 'json',
        estatuto: 'realizado',
        fonte: c.seletor,
        evidencia: 'um host; tique → u_time; original querySelector(.heroart)',
        proibicao: 'nao ingerir FS GLSL; nao puxar trailer nem cards em cena',
      }),
    ],
  })
}

export function uParaCena (u) {
  const nota = (u && u.nota) || ''
  const qm = nota.match(/quadros=([0-9]+)/)
  return {
    id: (nota.match(/id=([^;]+)/) || [])[1] || '',
    peca: (nota.match(/peca=([^;]+)/) || [])[1] || '',
    fase: (nota.match(/fase=([a-z]+)/) || [])[1] || '',
    quadros: qm ? Number(qm[1]) : 0,
  }
}

export { faseAposTique, DT0, ESCALA_FASE }
