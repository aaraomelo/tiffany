// banco_manifesto_u.js — ponte bidirecional Manifesto ↔ Schema U.
// U = nodo canónico. M = projecção matricial (corpos × órbitas).
// Não adivinha Hebb; não relê I0; não promove o manifesto a autoridade teórica.

import { completa, nodoU } from './banco_schema.js'

function copia (x) {
  return x == null ? x : JSON.parse(JSON.stringify(x))
}

export function chaveCorpo (c, i) {
  if (c && c.parte) return String(c.parte)
  if (c && c.canonico) return String(c.canonico)
  return '#' + i
}

function hopfieldDoCorpo (c, man) {
  const hp = man && man.hopfield
  if (!hp || !c) return false
  return hp.canonico != null && hp.canonico === c.canonico
}

/** Projecção M: índice corpos×órbitas. Célula copia o que o manifesto já declara. */
export function matrizDoManifesto (man) {
  const lista = (man && man.corpos && man.corpos.lista) || []
  const orbitas = copia((man && man.orbitas) || [])
  const hp = copia((man && man.hopfield) || {})
  const linhas = lista.map((c, i) => chaveCorpo(c, i))
  const colunas = orbitas.map((o) => o.nome)
  const celulas = []
  for (let i = 0; i < lista.length; i++) {
    const c = lista[i]
    for (let j = 0; j < orbitas.length; j++) {
      const o = orbitas[j]
      const hop = hopfieldDoCorpo(c, man) &&
        Array.isArray(hp.orbitas) && hp.orbitas.includes(o.nome)
      celulas.push({
        i,
        j,
        corpo: linhas[i],
        orbita: o.nome,
        ocupada: c.orbita != null && c.orbita === o.nome,
        estado: c.estatuto,
        evidencias: (c.medidores || []).slice(),
        matriz: c.matriz,
        relacoes: hop,
      })
    }
  }
  const corpos = man && man.corpos ? man.corpos : {}
  return {
    linhas,
    colunas,
    celulas,
    corpos: {
      palcos: corpos.palcos,
      nota: corpos.nota,
      proibicao: corpos.proibicao,
      lista: copia(lista),
    },
    orbitas,
    hopfield: hp,
  }
}

function nodoCorpo (c, i) {
  const n = {
    kind: 'realizacao',
    id: chaveCorpo(c, i),
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    estado: c.estatuto,
    parte: Object.prototype.hasOwnProperty.call(c, 'parte') ? c.parte : null,
    canonico: Object.prototype.hasOwnProperty.call(c, 'canonico') ? c.canonico : null,
    camada: c.camada,
    matriz: c.matriz,
    orbita: Object.prototype.hasOwnProperty.call(c, 'orbita') ? c.orbita : null,
    medidores: (c.medidores || []).slice(),
    evidencia: (c.medidores || []).join('|'),
    posicao: { i },
  }
  if (c.homonimos) n.homonimos = c.homonimos.slice()
  return completa(n)
}

function nodoOrbita (o, j) {
  const n = {
    kind: 'orbita',
    id: o.nome,
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    lingua: o.lingua,
    suporte: o.suporte,
    xi: o.xi,
    involucao: o.involucao,
    posicao: { j },
  }
  if (o.corpus != null) n.corpus = o.corpus
  if (o.chave != null) n.chave = o.chave
  if (o.latex != null) n.latex = o.latex
  if (o.nota != null) n.nota = o.nota
  if (o.slots != null) n.slots = copia(o.slots)
  return completa(n)
}

function nodoCelula (cel) {
  return completa({
    kind: 'celula',
    id: cel.i + ',' + cel.j,
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    i: cel.i,
    j: cel.j,
    corpo: cel.corpo,
    orbita: cel.orbita,
    ocupada: !!cel.ocupada,
    estado: cel.estado,
    medidores: (cel.evidencias || []).slice(),
    evidencia: (cel.evidencias || []).join('|'),
    matriz: cel.matriz,
    relacoes: !!cel.relacoes,
    posicao: { i: cel.i, j: cel.j },
  })
}

function corpoDeNodo (n) {
  const c = {
    parte: Object.prototype.hasOwnProperty.call(n, 'parte') ? n.parte : null,
    canonico: Object.prototype.hasOwnProperty.call(n, 'canonico') ? n.canonico : null,
    camada: n.camada,
    estatuto: n.estado,
    medidores: (n.medidores || []).slice(),
    matriz: n.matriz,
    orbita: Object.prototype.hasOwnProperty.call(n, 'orbita') ? n.orbita : null,
  }
  if (n.homonimos) c.homonimos = n.homonimos.slice()
  return c
}

function orbitaDeNodo (n) {
  const o = {
    nome: n.id,
    lingua: n.lingua,
    suporte: n.suporte,
    xi: n.xi,
    involucao: n.involucao,
  }
  if (n.corpus != null) o.corpus = n.corpus
  if (n.chave != null) o.chave = n.chave
  if (n.latex != null) o.latex = n.latex
  if (n.nota != null) o.nota = n.nota
  if (n.slots != null) o.slots = copia(n.slots)
  return o
}

function celulaDeNodo (n) {
  return {
    i: n.i,
    j: n.j,
    corpo: n.corpo,
    orbita: n.orbita,
    ocupada: !!n.ocupada,
    estado: n.estado,
    evidencias: (n.medidores || []).slice(),
    matriz: n.matriz,
    relacoes: !!n.relacoes,
  }
}

/** M → U. Gramática de U re-injectada; coordenadas = células do manifesto. */
export function uDeMatriz (M) {
  const u = nodoU()
  const lista = (M && M.corpos && M.corpos.lista) || []
  const orbitas = (M && M.orbitas) || []
  const celulas = (M && M.celulas) || []
  u.evidencia = 'manifesto.corpos x manifesto.orbitas'
  u.proibicao = 'U != catalogo != INGEST; M e projeccao, nao segundo schema; Star(U)=D'
  u.indice = {
    linhas: (M && M.linhas) || lista.map((c, i) => chaveCorpo(c, i)),
    colunas: (M && M.colunas) || orbitas.map((o) => o.nome),
  }
  u.hopfield = copia((M && M.hopfield) || {})
  u.corpos = {
    palcos: M && M.corpos ? M.corpos.palcos : undefined,
    nota: M && M.corpos ? M.corpos.nota : undefined,
    proibicao: M && M.corpos ? M.corpos.proibicao : undefined,
  }
  u.filhos = [
    ...lista.map((c, i) => nodoCorpo(c, i)),
    ...orbitas.map((o, j) => nodoOrbita(o, j)),
    ...celulas.map(nodoCelula),
  ]
  return u
}

export function manifestoParaU (man) {
  return uDeMatriz(matrizDoManifesto(man))
}

/** U → M. Faces descartadas (regeneram-se). Hopfield copiado, não recomputado. */
export function uParaMatriz (u) {
  const filhos = (u && u.filhos) || []
  const lista = filhos.filter((n) => n.kind === 'realizacao').map(corpoDeNodo)
  const orbitas = filhos.filter((n) => n.kind === 'orbita').map(orbitaDeNodo)
  const celulas = filhos.filter((n) => n.kind === 'celula').map(celulaDeNodo)
  const cab = (u && u.corpos) || {}
  return {
    linhas: (u && u.indice && u.indice.linhas) || lista.map((c, i) => chaveCorpo(c, i)),
    colunas: (u && u.indice && u.indice.colunas) || orbitas.map((o) => o.nome),
    celulas,
    corpos: {
      palcos: cab.palcos,
      nota: cab.nota,
      proibicao: cab.proibicao,
      lista,
    },
    orbitas,
    hopfield: copia((u && u.hopfield) || {}),
  }
}

export function uParaManifesto (u) {
  const M = uParaMatriz(u)
  return {
    orbitas: M.orbitas,
    hopfield: M.hopfield,
    corpos: M.corpos,
  }
}

export function igual (a, b) {
  if (a === b) return true
  if (a == null || b == null) return a === b
  if (typeof a !== typeof b) return false
  if (Array.isArray(a)) {
    if (!Array.isArray(b) || a.length !== b.length) return false
    return a.every((x, i) => igual(x, b[i]))
  }
  if (typeof a === 'object') {
    const ka = Object.keys(a).sort()
    const kb = Object.keys(b).sort()
    if (ka.length !== kb.length) return false
    return ka.every((k, i) => k === kb[i] && igual(a[k], b[k]))
  }
  return false
}

export function diffMatriz (a, b) {
  const d = { corpos: [], orbitas: [], celulas: [] }
  const la = (a && a.corpos && a.corpos.lista) || []
  const lb = (b && b.corpos && b.corpos.lista) || []
  const n = Math.max(la.length, lb.length)
  for (let i = 0; i < n; i++) {
    if (!igual(la[i], lb[i])) d.corpos.push(chaveCorpo(la[i] || lb[i], i))
  }
  const oa = (a && a.orbitas) || []
  const ob = (b && b.orbitas) || []
  const m = Math.max(oa.length, ob.length)
  for (let j = 0; j < m; j++) {
    if (!igual(oa[j], ob[j])) d.orbitas.push((oa[j] || ob[j] || {}).nome)
  }
  const ca = (a && a.celulas) || []
  const cb = (b && b.celulas) || []
  const k = Math.max(ca.length, cb.length)
  for (let t = 0; t < k; t++) {
    if (!igual(ca[t], cb[t])) {
      const c = ca[t] || cb[t] || {}
      d.celulas.push({ i: c.i, j: c.j })
    }
  }
  return d
}
