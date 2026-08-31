// banco_latex_u.js — F4: LaTeX é capacidade sob demanda, não paint.
// existência ≠ carga ≠ execução. tex.wasm ∉ G_paint.
// Card (F3) dispara; tex_tradutor só no import(); tex.wasm só na execução.
// Não toca banco_front / banco_disco / S_ESTADO.

import { completa } from './banco_schema.js'
import { acaoDoCard } from './banco_cards_u.js'

export const ID_LATEX_GK = 'gk'
export const CHAVE_LATEX = 'gk:reino:latex'
export const FONTE_LATEX = './tex_tradutor.js'
export const WASM_LATEX = '/wasm/tex.wasm'
/** Expressão conhecida — a boxed de tests/tex_wasm.js §T5 (Alonzo). */
export const EXPR_CONHECIDA = '\\boxed{\\ \\sigma\\,\\sigma = -1\\ }'

let modulo = null
let nWasm = 0

export function resetLatex () {
  modulo = null
  nWasm = 0
}

export function faseLatex () {
  if (nWasm) return 'execucao'
  if (modulo) return 'carga'
  return 'existencia'
}

export function fetchesTexWasm () { return nWasm }
export function moduloLatexCarregado () { return !!modulo }

/** Mesma régua que tex_tradutor.js idDeArquivo. */
export function idDeArquivo (href) {
  const m = /\/docs\/([A-Za-z0-9_-]+)\.pdf/.exec(href || '')
  return m ? m[1] : null
}

export function catalogoLatex (manDocs, docsMap) {
  const map = docsMap || {}
  const out = []
  for (const d of manDocs || []) {
    const id = idDeArquivo(d.arquivo) || ''
    const fonte = (id && map[id]) ? String(map[id]) : ''
    out.push({
      id,
      nome: String((d && d.nome) || ''),
      arquivo: String((d && d.arquivo) || ''),
      fonte,
      compoe: !!fonte,
    })
  }
  return out
}

export function igualDoc (a, b) {
  if (!a || !b) return a === b
  return a.id === b.id && a.fonte === b.fonte && a.compoe === b.compoe
}

export function igualCatalogoLatex (a, b) {
  if (!a || !b || a.length !== b.length) return false
  return a.every((d, i) => igualDoc(d, b[i]))
}

/** Card ou doc catalogado. Sem latex no gatilho → não carrega. */
export function querLatex (gatilho) {
  if (!gatilho) return false
  if (typeof gatilho === 'string') return !!gatilho
  if (gatilho.arquivo) return !!idDeArquivo(gatilho.arquivo)
  if (gatilho.fonte && gatilho.compoe) return true
  if (Array.isArray(gatilho.chama) && gatilho.chama.includes('latex')) return true
  if (gatilho.refs && gatilho.refs.latex) return true
  if (gatilho.nome || gatilho.secao) return acaoDoCard(gatilho).chama.includes('latex')
  return false
}

export function selecionaLatex (storage, id) {
  const s = String(id || '')
  if (storage && s) {
    try { storage.setItem(CHAVE_LATEX, JSON.stringify({ id: s })) } catch { /* quota */ }
  }
  return s
}

export function leLatexSelecionado (storage) {
  if (!storage) return ''
  try {
    const o = JSON.parse(storage.getItem(CHAVE_LATEX) || 'null')
    return (o && o.id) || ''
  } catch {
    return ''
  }
}

export function resultadoLatex (bytes) {
  const u8 = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes || [])
  const latin = typeof bytes === 'string' ? bytes : new TextDecoder('latin1').decode(u8)
  return {
    pdf: u8.length >= 2 && u8[0] === 0x25 && u8[1] === 0x50,
    eof: latin.includes('%%EOF'),
    semente: latin.includes('/Type/SementeEstrela'),
    oito: latin.includes('/Type/AssinaturaOito'),
    n: u8.length,
  }
}

export function igualResultado (a, b) {
  if (!a || !b) return a === b
  return a.pdf === b.pdf && a.eof === b.eof && a.semente === b.semente && a.oito === b.oito
}

/** Carga: import() do tradutor. tex.wasm ainda 0. */
export async function carregaLatex (opts = {}) {
  if (!modulo) {
    const imp = opts.importar || (() => import('./tex_tradutor.js'))
    modulo = await imp()
  }
  return modulo
}

/** Execução: aí sim tex.wasm. */
export async function executaLatex (tex, opts = {}) {
  const m = await carregaLatex(opts)
  nWasm++
  const fetchWasm = opts.fetchWasm || (async () => {
    const r = await fetch(WASM_LATEX)
    return new Uint8Array(await r.arrayBuffer())
  })
  const wasm = await fetchWasm()
  if (typeof opts.compor === 'function') {
    return resultadoLatex(await opts.compor(tex || EXPR_CONHECIDA, wasm, m))
  }
  if (m && typeof m.comporTexto === 'function') {
    const r = await m.comporTexto(opts.nome || 'f4.tex', tex || EXPR_CONHECIDA)
    return resultadoLatex(r && r.bytes)
  }
  return resultadoLatex(wasm)
}

/**
 * Card → LaTeX → tex.wasm, em passos.
 * Sem opts.executar: só selecciona (existência). Sem opts.carregar: não importa o tradutor.
 */
export async function disparaLatex (storage, gatilho, opts = {}) {
  const doc = typeof gatilho === 'string'
    ? gatilho
    : (gatilho && gatilho.arquivo ? idDeArquivo(gatilho.arquivo) : '')
  const id = doc || (gatilho && gatilho.id) || ''
  if (id) selecionaLatex(storage, id)
  if (!querLatex(gatilho)) {
    return { id, fase: faseLatex(), carregou: !!modulo, executou: false, wasm: nWasm }
  }
  if (opts.executar) {
    const r = await executaLatex(opts.expr || EXPR_CONHECIDA, opts)
    return { id, fase: faseLatex(), carregou: true, executou: true, wasm: nWasm, resultado: r }
  }
  if (opts.carregar) {
    await carregaLatex(opts)
    return { id, fase: faseLatex(), carregou: true, executou: false, wasm: nWasm }
  }
  return { id, fase: faseLatex(), carregou: !!modulo, executou: false, wasm: nWasm }
}

export function latexParaU (cat) {
  const lista = Array.isArray(cat) ? cat : []
  const n = lista.filter((d) => d.compoe).length
  return completa({
    kind: 'pagina',
    id: ID_LATEX_GK,
    sentido: 0,
    formato: 'json',
    camada: 'capacidade',
    estatuto: 'realizado',
    evidencia: 'docs_tradutor.json + manifesto.docs; idDeArquivo = tex_tradutor; tests/latex_u.js',
    proibicao: 'existencia != carga != execucao; tex.wasm notin G_paint; nao tocar banco_front/disco/S_ESTADO',
    nota: 'n=' + lista.length + '; compoe=' + n + '; fase=' + faseLatex() +
      '; wasm=' + nWasm + '; expr=' + EXPR_CONHECIDA,
    slots: { existencia: 'realizado', carga: 'tardia', execucao: 'tardia', wasm: nWasm ? 'ref' : 'N/A' },
    filhos: lista.filter((d) => d.compoe).map((d) => completa({
      kind: 'ficheiro',
      id: d.id || 'doc',
      sentido: 0,
      formato: 'tex',
      estatuto: 'realizado',
      fonte: d.fonte || '',
      evidencia: 'docs_tradutor.docs; clique original /docs/*.pdf',
      proibicao: 'doc estatico (cv, fisica-araniana) nao e tex.wasm',
    })),
  })
}

export function uParaLatex (u) {
  const nota = (u && u.nota) || ''
  const nm = nota.match(/n=([0-9]+)/)
  const cm = nota.match(/compoe=([0-9]+)/)
  return {
    n: nm ? Number(nm[1]) : ((u && u.filhos) || []).length,
    compoe: cm ? Number(cm[1]) : ((u && u.filhos) || []).length,
    fase: (nota.match(/fase=([a-z]+)/) || [])[1] || '',
    wasm: Number((nota.match(/wasm=([0-9]+)/) || [])[1] || 0),
  }
}
