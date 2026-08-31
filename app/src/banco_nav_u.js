// banco_nav_u.js — F1: navegação original do GK no hospedeiro (iframe = oráculo).
// Host → tenant → gk → #ancora. Não reinterpreta main.js; não executa GLSL/LaTeX.
// ?tenant= não entra no URL do Reino. nav(foo) ⊥ nav(bar); id(K) pode coincidir.

import { completa } from './banco_schema.js'

export const ID_NAV_GK = 'gk'

export const CHAVE_NAV = 'gk:banco:nav'
export const ANCORAS_FIXAS = Object.freeze([
  'top', 'trailer', 'substratos', 'pontes', 'assistente', 'autor', 'docs',
])

export function ancoraDeHash (hash) {
  const a = String(hash || '').replace(/^#/, '').split(/[?&]/)[0]
  if (!a || !/^[a-z][a-z0-9_]*$/i.test(a)) return ''
  return a
}

/** Âncoras do app original: #top + manifesto.secoes + extras do nav() em main.js. */
export function ancorasDoManifesto (man) {
  const ids = (man && man.secoes || []).map((s) => String(s.id || '')).filter(Boolean)
  const seen = new Set(['top', ...ids])
  const extra = ANCORAS_FIXAS.filter((id) => !seen.has(id))
  return ['top', ...ids, ...extra]
}

/** Oracle: href="#id" em main.js (não executa). */
export function hrefsDoMain (mainJs) {
  const out = []
  const re = /href\s*=\s*["']#([A-Za-z][A-Za-z0-9_]*)["']/g
  let m
  while ((m = re.exec(String(mainJs || '')))) {
    if (!out.includes(m[1])) out.push(m[1])
  }
  return out
}

/** iframe src: o Reino original. Nunca ?tenant=. */
export function urlHospedadoGk (opts = {}) {
  const a = ancoraDeHash(opts.ancora || opts.hash || '')
  return a ? '/#' + a : '/'
}

/** Entrada no banco: /banco/?gk=1#ancora. Tenant vem do Host, não da query. */
export function urlBancoGk (opts = {}) {
  const a = ancoraDeHash(opts.ancora || opts.hash || '')
  return '/banco/?gk=1' + (a ? '#' + a : '')
}

export function navPerp (a, b) {
  return String((a && a.tenant) || '') !== String((b && b.tenant) || '')
}

export function leAncora (storage) {
  if (!storage) return ''
  try {
    const o = JSON.parse(storage.getItem(CHAVE_NAV) || 'null')
    return ancoraDeHash(o && o.ancora)
  } catch {
    return ''
  }
}

export function gravaAncora (storage, ancora) {
  if (!storage) return
  try {
    storage.setItem(CHAVE_NAV, JSON.stringify({ ancora: ancoraDeHash(ancora) }))
  } catch { /* quota */ }
}

export function navParaU (n) {
  const tenant = (n && n.tenant) || ''
  const ancora = ancoraDeHash(n && n.ancora)
  return completa({
    kind: 'pagina',
    id: ID_NAV_GK,
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    evidencia: 'main.js href=#id; manifesto.secoes; iframe oraculo; tests/nav_u.js',
    proibicao: 'nav != S_ESTADO; iframe andaime; nao reinterpretar main.js; ?tenant= so localhost; X-Tenant deriva do Host; tenant != id != K_i',
    tenant,
    fonte: urlHospedadoGk({ ancora }),
    nota: 'ancora=' + (ancora || 'top') + '; hospedado',
  })
}

export function uParaNav (u) {
  const nota = (u && u.nota) || ''
  const am = nota.match(/ancora=([A-Za-z0-9_]+)/)
  return {
    tenant: (u && u.tenant) || '',
    ancora: ancoraDeHash(am ? am[1] : ''),
    fonte: (u && u.fonte) || '/',
  }
}

export function igualNav (a, b) {
  return !!(a && b && a.tenant === b.tenant && ancoraDeHash(a.ancora) === ancoraDeHash(b.ancora))
}
