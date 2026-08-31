// banco_cards_u.js — F3: card é capacidade, não estado.
// card ≠ S_ESTADO ≠ livro ≠ sessão. Descrição do manifesto; kernel lista-se, não executa.
// LaTeX/GLSL/WASM ficam refs (F4/F5). Card não usado não entra no paint (import() pós-quadro).

import { completa } from './banco_schema.js'

export const ID_CARDS_GK = 'gk'
export const CHAVE_CARD = 'gk:reino:card'
/** Peças sem kernel emitido no original (main.js MOTOR). */
export const PECAS_NOMEADAS = Object.freeze(['coracao_revela', 'captura'])
/** Primeiro card funcional: kernel listado, GLSL não executado. */
export const CARD_PRIMEIRO = Object.freeze({ nome: 'rainha_tiffany', secao: 'corte' })

export function indicePecasArt (art) {
  const pecas = (art && art.pecas) || {}
  const out = {}
  for (const k of Object.keys(pecas)) {
    const p = pecas[k] || {}
    out[k] = { kernel: String(p.kernel || ''), secao: String(p.secao || '') }
  }
  return out
}

/** Mesma chave que main.js: nome@secao primeiro, senão nome se art.secao casa. */
export function chaveKernel (p, secao, indice) {
  const nome = String((p && p.nome) || '')
  const sec = String(secao || '')
  if (!nome || !indice) return null
  const composta = nome + '@' + sec
  if (indice[composta]) return composta
  const a = indice[nome]
  return (a && a.secao === sec) ? nome : null
}

export function refsDoCard (card) {
  const kernel = !!(card && card.kernel)
  const txt = String((card && (card.desc || '')) + (card && card.op || ''))
  const latex = /\.tex\b|tex\.wasm|\\\\[a-zA-Z]|LaTeX/i.test(txt)
  return { glsl: kernel, wasm: false, latex }
}

export function catalogoCards (man, indice) {
  const idx = indice || {}
  const out = []
  for (const s of (man && man.secoes) || []) {
    const secao = String(s.id || '')
    for (const p of s.pecas || []) {
      const chave = chaveKernel(p, secao, idx)
      const kernel = chave && idx[chave] ? idx[chave].kernel : ''
      const card = {
        id: chave || String(p.nome || ''),
        nome: String(p.nome || ''),
        secao,
        titulo: String(p.titulo || ''),
        op: String(p.op || ''),
        desc: String(p.desc || ''),
        kernel,
        nomeado: !kernel,
      }
      card.refs = refsDoCard(card)
      out.push(card)
    }
  }
  return out
}

export function igualCard (a, b) {
  if (!a || !b) return a === b
  return a.id === b.id && a.nome === b.nome && a.secao === b.secao &&
    a.titulo === b.titulo && a.op === b.op && a.kernel === b.kernel
}

export function igualCatalogo (a, b) {
  if (!a || !b || a.length !== b.length) return false
  return a.every((c, i) => igualCard(c, b[i]))
}

/** card → acção → capacidade listada. Não executa GLSL/LaTeX/WASM. */
export function acaoDoCard (card) {
  const refs = (card && card.refs) || refsDoCard(card)
  const chama = []
  if (refs.glsl) chama.push('glsl')
  if (refs.wasm) chama.push('wasm')
  if (refs.latex) chama.push('latex')
  return {
    id: (card && card.id) || '',
    acao: 'selecionar',
    chama,
    executou: false,
  }
}

export function selecionaCard (storage, card) {
  const a = acaoDoCard(card)
  if (storage && a.id) {
    try { storage.setItem(CHAVE_CARD, JSON.stringify({ id: a.id, acao: a.acao })) } catch { /* quota */ }
  }
  return a
}

export function leCardSelecionado (storage) {
  if (!storage) return ''
  try {
    const o = JSON.parse(storage.getItem(CHAVE_CARD) || 'null')
    return (o && o.id) || ''
  } catch {
    return ''
  }
}

export function cardParaU (card) {
  const c = card || {}
  const refs = c.refs || refsDoCard(c)
  return completa({
    kind: 'ficheiro',
    id: c.id || 'card',
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    evidencia: 'manifesto.secoes.pecas; kernels_campo.pecas.kernel (indice, nao o GLSL); tests/cards_u.js',
    proibicao: 'card != S_ESTADO != livro != sessao; GLSL/LaTeX refs nao ingeridas; nao executar kernel',
    fonte: c.nome || '',
    nota: 'secao=' + (c.secao || '') + '; kernel=' + (c.kernel || 'nomeado') +
      '; glsl=' + (refs.glsl ? '1' : '0') + '; latex=' + (refs.latex ? '1' : '0'),
    slots: { glsl: refs.glsl ? 'ref' : 'N/A', latex: refs.latex ? 'ref' : 'N/A', wasm: refs.wasm ? 'ref' : 'N/A' },
  })
}

export function uParaCard (u) {
  const nota = (u && u.nota) || ''
  const sm = nota.match(/secao=([A-Za-z0-9_]+)/)
  const km = nota.match(/kernel=([A-Za-z0-9_]+)/)
  return {
    id: (u && u.id) || '',
    nome: (u && u.fonte) || '',
    secao: sm ? sm[1] : '',
    kernel: km && km[1] !== 'nomeado' ? km[1] : '',
    titulo: '',
    op: '',
  }
}

export function catalogoParaU (cards) {
  const lista = Array.isArray(cards) ? cards : []
  return completa({
    kind: 'pagina',
    id: ID_CARDS_GK,
    sentido: 0,
    formato: 'json',
    camada: 'capacidade',
    estatuto: 'realizado',
    evidencia: 'catalogo de cards; kernel lista-se; F3 nao executa GLSL; tests/cards_u.js',
    proibicao: 'card != S_ESTADO != livro != sessao; card nao usado nao pinta; LaTeX/GLSL fora de F3',
    nota: 'n=' + lista.length,
    filhos: lista.map(cardParaU),
  })
}
