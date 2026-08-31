// banco_pagina_u.js — ponte página web ↔ Schema U (mesmo $id tiffany://u).
// Página = nodo autossimilar: tríade html/css/js. Cliente/servidor = faces MOVE ±1 no canal.
// js ≠ node. fetch/http não são órbita Hopfield. Não é segundo schema.

import { completa, nodoLingua } from './banco_schema.js'

export const TRIADE = ['html', 'css', 'js']
/** Offsets a partir de S_CANAL (sql.c / canal_browser.js). */
export const SLOTS_FRONT = { in: 9200, out: 9201, base: 'S_CANAL' }

function linguaDoManifesto (man, nome) {
  const L = (man && man.linguagens || []).find((l) => l.nome === nome)
  if (L) return L
  return { nome, faz: nome, p: 0, q: 0, r: 0, absorcao: { move: nome + '_move' } }
}

export function nodoFacePagina (nome, texto, man) {
  const n = nodoLingua(linguaDoManifesto(man, nome))
  n.formato = nome
  n.texto = texto == null ? '' : String(texto)
  n.proibicao = 'js != node; html/css/js != Parte; fetch nao e orbita'
  return n
}

/** P → U. Tríade vira filhos; faces = cruz MOVE (cliente emite / servidor absorve). */
export function paginaParaU (fontes, man, id = 'pagina') {
  const n = completa({
    kind: 'pagina',
    id: id || 'pagina',
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    evidencia: 'app/banco/pagina.{html,css,js}; canal S_FRONT_REQ/RSP',
    proibicao: 'js != node; html/css/js != Parte; fetch/http nao e orbita Hopfield',
    triade: TRIADE.slice(),
    slots: { ...SLOTS_FRONT },
    filhos: TRIADE.map((nome) => nodoFacePagina(nome, fontes && fontes[nome], man)),
  })
  return n
}

/** U → P. Projecção da tríade; faces regeneram-se. */
export function uParaPagina (u) {
  const filhos = (u && u.filhos) || []
  const out = { id: (u && u.id) || 'pagina', html: '', css: '', js: '' }
  for (const nome of TRIADE) {
    const f = filhos.find((x) => x.id === nome)
    out[nome] = f && f.texto != null ? String(f.texto) : ''
  }
  return out
}

export function igualPagina (a, b) {
  return (a && a.html) === (b && b.html) &&
    (a && a.css) === (b && b.css) &&
    (a && a.js) === (b && b.js)
}
