// banco_schema.js — nodo U autossimilar (cruz Alonzo: completa = duas metades).
// JSON canónico; claim/erg/sql/tex são emissões. U ≠ Alonzo ≠ Parte.

export const FORMATOS = ['json', 'claim', 'erg', 'sql', 'tex', 'wasm', 'fita', 'manifesto', 'implante', 'html', 'css', 'js', 'sh', 'ps1']

export function detectaFormato (path, buf) {
  const u8 = buf instanceof Uint8Array ? buf : new TextEncoder().encode(String(buf || ''))
  const txt = typeof buf === 'string' ? buf : new TextDecoder('utf-8', { fatal: false }).decode(u8)
  if (u8.length >= 4 && u8[0] === 0 && u8[1] === 0x61 && u8[2] === 0x73 && u8[3] === 0x6d) return 'wasm'
  const t = txt.replace(/^\s+/, '')
  if (t.startsWith('implante')) return 'implante'
  if (/^claim(\s|$)/.test(t)) return 'claim'
  if (t.startsWith('{')) {
    if (/"linguagens"/.test(txt) || /"corpos"/.test(txt)) return 'manifesto'
    return 'json'
  }
  const p = String(path || '')
  if (p.endsWith('.html')) return 'html'
  if (p.endsWith('.css')) return 'css'
  if (p.endsWith('.js') && !p.endsWith('.json')) return 'js'
  if (p.endsWith('.claim')) return 'claim'
  if (p.endsWith('.erg')) return 'erg'
  if (p.endsWith('.fita') || p.endsWith('.fita.bin')) return 'fita'
  if (p.endsWith('.wasm')) return 'wasm'
  if (p.endsWith('.tex')) return 'tex'
  if (p.endsWith('.json')) return 'json'
  if (/\nLOAD |\n; wasm/.test(txt)) return 'erg'
  return 'json'
}

function zera () {
  return {
    kind: 'ficheiro',
    id: 'ficheiro',
    sentido: 0,
    formato: 'json',
    estatuto: 'nao localizada',
  }
}

export function parseClaim (src) {
  const n = zera()
  n.kind = 'claim'
  n.formato = 'claim'
  n.estatuto = 'realizado'
  n.law = -1
  let viu = false
  let end = false
  for (const raw of String(src).split(/\r?\n/)) {
    const linha = raw.trim()
    if (!linha || linha.startsWith('#')) continue
    const [k, ...rest] = linha.split(/\s+/)
    const v = rest.join(' ')
    if (k === 'claim') { n.id = v; viu = true; continue }
    if (k === 'end') { end = true; break }
    if (k === 'residual') throw new Error('residual nao entra no Claim')
    if (k === 'law') n.law = parseInt(v, 10)
    else if (k === 'object') n.object = v
    else if (k === 'step') n.step = v
    else if (k === 'back') n.back = v
    else if (k === 'measure') n.measure = v
    else if (k === 'invariant') n.invariant = v
    else if (k === 'mutate') n.mutate = v
    else if (k === 'classify') n.classify = v
  }
  if (!viu || !end || !n.id || n.law < 0 || !n.step || !n.back || !n.measure) {
    throw new Error('claim incompleto')
  }
  return completa(n)
}

export function nodoU () {
  return completa({
    kind: 'U',
    id: 'U',
    sentido: 0,
    formato: 'json',
    star: 'D',
    estatuto: 'gramatica',
    X: {
      dim: 8,
      base: 'e_k=2^k',
      dobras: 3,
      leis: 8,
      coord: '<b,e_k>=(b>>k) AND 1',
      gram: 'Id',
      endereco: '(x,k) horizontal/vertical',
    },
    Xstar: { emparelhamento: 'paridade(a AND b)', nao: 'X* != V* != Duo' },
    Mor: ['Hom', 'Iso', 'Diff', 'Isom_mu', 'Duo'],
    Aut: { estado: 'G in V' },
    proibicao: 'U != catalogo != INGEST; nao e Parte; Star(U)=D',
    evidencia: 'corpo_universal.tex univ:def:U',
  })
}

/** Completa = cruz das duas metades (mesmo schema). */
export function completa (nodo) {
  const n = { ...nodo, sentido: nodo.sentido ?? 0 }
  if (n.sentido === 0) {
    n.faces = {
      menos: metade(n, -1),
      mais: metade(n, +1),
    }
  }
  return n
}

export function metade (nodo, sentido) {
  const s = sentido < 0 ? -1 : 1
  return {
    kind: 'metade',
    id: nodo.id,
    sentido: s,
    formato: 'json',
    estatuto: nodo.estatuto || 'gramatica',
    evidencia: s < 0 ? 'MOVE emite' : 'MOVE absorve',
  }
}

export function nodoLingua (L) {
  return completa({
    kind: 'lingua',
    id: L.nome,
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    faz: L.faz,
    move: L.absorcao?.move || L.exports?.find((e) => e.endsWith('_move')) || '',
    p: L.p | 0,
    q: L.q | 0,
    r: L.r | 0,
    fonte: L.fonte,
    proibicao: 'lingua != Parte; js != node',
  })
}

export function parseFicheiro (path, buf, opts = {}) {
  const formato = detectaFormato(path, buf)
  const sentido = opts.sentido ?? 0
  const as = opts.as || 'json'
  const txt = typeof buf === 'string' ? buf : new TextDecoder('utf-8', { fatal: false }).decode(buf)
  let n
  if (formato === 'claim') n = parseClaim(txt)
  else if (formato === 'manifesto') {
    /* Roupas ISA: linguagens[] → filhos lingua. A matriz corpos×órbitas é a ponte
     * banco_manifesto_u.js (MANIFESTO → U / U → MANIFESTO), não este PARSE. */
    const man = JSON.parse(txt)
    n = nodoU()
    n.fonte = path
    n.evidencia = 'manifesto.linguagens realizam U'
    n.filhos = (man.linguagens || []).map(nodoLingua)
  } else if (formato === 'json') {
    n = JSON.parse(txt)
    if (!n.kind) n.kind = 'ficheiro'
  } else if (formato === 'html' || formato === 'css' || formato === 'js') {
    n = zera()
    n.kind = 'lingua'
    n.id = formato
    n.formato = formato
    n.texto = txt
    n.fonte = path
    n.estatuto = 'realizado'
    n.proibicao = 'js != node; fetch nao e orbita'
    n.evidencia = 'pagina.' + formato
  } else {
    n = zera()
    n.kind = formato === 'tex' ? 'ficha' : 'ficheiro'
    n.id = formato
    n.formato = formato
    n.fonte = path
    n.estatuto = formato === 'tex' ? 'nao localizada' : 'realizado'
    n.evidencia = formato === 'tex' ? 'fichaingestao no catalogo; nao se rele I0' : formato
  }
  n.sentido = sentido
  if (sentido !== 0) n = metade(n, sentido)
  return emit(n, as)
}

export function emit (nodo, formato = 'json') {
  const as = formato || 'json'
  if (as === 'json') return JSON.stringify(nodo)
  if (as === 'claim') {
    if (nodo.kind !== 'claim') throw new Error('metade: claim so de kind=claim')
    return ['claim ' + nodo.id, 'law ' + nodo.law, 'object ' + (nodo.object || ''),
      'step ' + nodo.step, 'back ' + nodo.back, 'measure ' + nodo.measure,
      'invariant ' + (nodo.invariant || ''), 'mutate ' + (nodo.mutate || ''),
      'classify ' + (nodo.classify || ''), 'end', ''].join('\n')
  }
  if (as === 'sql') {
    return "INSERT TEXTO 'schema/" + nodo.id + '|' + nodo.kind + '|' + nodo.sentido + '|' + nodo.formato + "'\n"
  }
  if (as === 'tex') {
    return '\\ficharow{' + nodo.id + '}{' + nodo.kind + ' sentido ' + nodo.sentido + ' formato ' + nodo.formato + '}\n'
  }
  throw new Error('formato «' + as + '» nao localizada')
}
