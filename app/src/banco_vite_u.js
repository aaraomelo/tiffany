// banco_vite_u.js — esqueleto Vite do Golden Kingdom ↔ Schema U (kind=pagina, id=gk).
// Vite é hospedeiro, não kind novo. GLSL / LaTeX são capacidades tardias: listam-se, não executam.
// ≠ página-banco (id=pagina, tríade pagina.html/css/js). Não copiar main.js para o DOM do banco.

import { completa } from './banco_schema.js'
import { nodoFacePagina, TRIADE } from './banco_pagina_u.js'

export const ID_GK = 'gk'
export const MODULO_GK = '/src/main.js'
export const CSS_GK = './style.css'
export const PUBLIC_DIR = 'assets/figuras'
export const ENTRADA_HTML = 'app/index.html'
export const ENTRADA_MODULO = 'app/src/main.js'

/** Entrada HTML: #app + script type=module. */
export function parseEntradaHtml (html) {
  const src = String(html || '')
  const temApp = /id\s*=\s*["']app["']/.test(src)
  let modulo = ''
  let eModulo = false
  const re = /<script\b([^>]*)>/gi
  let m
  while ((m = re.exec(src))) {
    const attrs = m[1]
    if (!/type\s*=\s*["']module["']/i.test(attrs)) continue
    eModulo = true
    const sm = attrs.match(/\bsrc\s*=\s*["']([^"']+)["']/i)
    if (sm) {
      modulo = sm[1]
      break
    }
  }
  return { temApp, modulo, eModulo }
}

/** Imports estáticos de 1º nível (sem executar, sem import()). */
export function parseImportsPrimeiroNivel (jsSrc) {
  const out = []
  const seen = new Set()
  for (const raw of String(jsSrc || '').split(/\n/)) {
    const t = raw.trim()
    if (!t || t.startsWith('//') || t.startsWith('/*') || t.startsWith('*')) continue
    if (!t.startsWith('import ')) break
    const m = t.match(/from\s+['"](\.[^'"]+)['"]/) || t.match(/^import\s+['"](\.[^'"]+)['"]/)
    if (!m) continue
    const spec = m[1]
    if (seen.has(spec)) continue
    seen.add(spec)
    out.push(spec)
  }
  return out
}

export function cssEntrada (imports) {
  const css = (imports || []).find((s) => s.endsWith('.css'))
  return css || CSS_GK
}

function formatoDeSpec (spec) {
  if (spec.endsWith('.css')) return 'css'
  if (spec.endsWith('.html')) return 'html'
  if (spec.endsWith('.js')) return 'js'
  return 'json'
}

function idDeSpec (spec) {
  const base = String(spec || '').replace(/^\.\//, '').split('/').pop() || 'ficheiro'
  return base.replace(/\.[^.]+$/, '') || base
}

function nodoImport (spec) {
  const formato = formatoDeSpec(spec)
  return completa({
    kind: 'ficheiro',
    id: idDeSpec(spec),
    sentido: 0,
    formato,
    estatuto: 'realizado',
    fonte: spec,
    evidencia: 'import 1o nivel de main.js; nao executa',
    proibicao: 'GLSL/LaTeX capacidades tardias; != bundle',
  })
}

/** Esqueleto a partir dos dois ficheiros (paths, não o bundle). */
export function leEsqueleto (html, mainJs) {
  const ent = parseEntradaHtml(html)
  const imports = parseImportsPrimeiroNivel(mainJs)
  return {
    html: String(html || ''),
    modulo: ent.modulo || MODULO_GK,
    css: cssEntrada(imports),
    imports,
    publicDir: PUBLIC_DIR,
  }
}

/** P → U. id=gk; filhos tríade = html + paths css/js; imports = ficheiros. */
export function gkParaU (g, man) {
  const html = (g && g.html) != null ? String(g.html) : ''
  const css = (g && g.css) || CSS_GK
  const modulo = (g && g.modulo) || MODULO_GK
  const imports = Array.isArray(g && g.imports) ? g.imports.slice() : []
  const filhos = [
    nodoFacePagina('html', html, man),
    nodoFacePagina('css', css, man),
    nodoFacePagina('js', modulo, man),
    ...imports.map(nodoImport),
  ]
  return completa({
    kind: 'pagina',
    id: ID_GK,
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    evidencia: ENTRADA_HTML + '; type=module ' + MODULO_GK + '; Vite hospedeiro',
    proibicao: 'Vite != pagina-banco; GLSL/LaTeX capacidades tardias; nao kind novo',
    fonte: ENTRADA_HTML,
    nota: 'Vite = hospedeiro; esqueleto realizado; app original || app ingerido',
    triade: TRIADE.slice(),
    filhos,
  })
}

/** U → P. Projecção do esqueleto; faces regeneram-se. */
export function uParaGk (u) {
  const filhos = (u && u.filhos) || []
  const htmlN = filhos.find((x) => x.id === 'html')
  const cssN = filhos.find((x) => x.id === 'css')
  const jsN = filhos.find((x) => x.id === 'js')
  return {
    html: htmlN && htmlN.texto != null ? String(htmlN.texto) : '',
    css: cssN && cssN.texto != null ? String(cssN.texto) : '',
    modulo: jsN && jsN.texto != null ? String(jsN.texto) : '',
    imports: filhos.filter((x) => x.kind === 'ficheiro').map((x) => String(x.fonte || '')),
    publicDir: PUBLIC_DIR,
  }
}

export function igualGk (a, b) {
  if (!a || !b) return a === b
  if (a.html !== b.html || a.css !== b.css || a.modulo !== b.modulo) return false
  if (a.publicDir !== b.publicDir) return false
  const ia = a.imports || []
  const ib = b.imports || []
  if (ia.length !== ib.length) return false
  return ia.every((s, i) => s === ib[i])
}
