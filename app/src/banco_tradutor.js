// banco_tradutor.js — máquina: MOVE na arena + arestas do manifesto. Sem adaptador SQL.

import { moveWasm, enc, dec, loadWasm } from './banco_move.js'
import { manifestoAtual } from './manifesto_loader.js'
import { execQueryCelula, initCelula } from './banco_celula.js'
import { absorveBackend } from './banco_absorve.js'

let HUB = 'sql'
let PIPE = []

export function initTradutor (man) {
  HUB = man.interface_padrao || 'sql'
  PIPE = man.pipe_linguagens || []
}

function man () {
  return manifestoAtual()
}

/** Backend wasm, protocolo canal, ou transporte HTTP não absorvido. */
export function entradaManifesto (nome) {
  const m = man()
  const L = m.linguagens.find((l) => l.nome === nome)
  if (L) return L
  if (m.protocolo?.nome === nome) return m.protocolo
  const o = m.orbitas?.find((x) => x.nome === nome)
  if (o) return o
  const f = m.fios?.find((x) => x.nome === nome)
  if (f) return f
  return null
}

/** Órbita Hopfield declarada (sql/latex/node). Não é fio HTTP. */
export function orbitaManifesto (nome) {
  return man().orbitas?.find((o) => o.nome === nome) || null
}

export function assinaturaParidade (lang) {
  return ((lang.p | 0) + (lang.q | 0) + (lang.r | 0)) & 1
}

export function langManifesto (nome) {
  const L = entradaManifesto(nome)
  if (!L) throw new Error('entrada «' + nome + '» ausente no manifesto')
  return L
}

export function paridade (de, para) {
  const A = typeof de === 'string' ? langManifesto(de) : de
  const B = typeof para === 'string' ? langManifesto(para) : para
  const a = assinaturaParidade(A) ^ assinaturaParidade(B)
  const b = (A.r | 0) !== (B.r | 0) ? 1 : 0
  return { a, b, quadrante: 'Q' + a + b, de: A.nome, para: B.nome }
}

export function arestaCanonica (de, para, hub = HUB) {
  const nomeDe = typeof de === 'string' ? de : de.nome
  const nomePara = typeof para === 'string' ? para : para.nome
  if (nomeDe === nomePara) {
    return { de: nomeDe, para: nomePara, a: 0, b: 0, rota: [nomeDe], ponte: null }
  }
  const { a, b } = paridade(nomeDe, nomePara)
  let rota
  if (a === 0) rota = [nomeDe, nomePara]
  else if (nomeDe === hub || nomePara === hub) rota = [nomeDe, nomePara]
  else rota = [nomeDe, hub, nomePara]
  const ponte = a === 0 ? null : (rota.length === 3 ? 'sql_tags' : 'sql_face')
  return { de: nomeDe, para: nomePara, a, b, rota, ponte }
}

export function aresta (de, para) {
  const nomeDe = typeof de === 'string' ? de : de.nome
  const nomePara = typeof para === 'string' ? para : para.nome
  const found = man().arestas?.find((e) => e.de === nomeDe && e.para === nomePara)
  if (!found) throw new Error('aresta «' + nomeDe + '→' + nomePara + '» ausente no manifesto')
  return found
}

export function validaArestas (hub = HUB) {
  const errs = []
  for (const found of man().arestas || []) {
    if (!entradaManifesto(found.de) || !entradaManifesto(found.para)) {
      errs.push('aresta desconhecida: ' + found.de + '→' + found.para)
      continue
    }
    const canon = arestaCanonica(found.de, found.para, hub)
    if (found.a !== canon.a) errs.push(found.de + '→' + found.para + ': a diverge')
    if (found.b !== canon.b) errs.push(found.de + '→' + found.para + ': b diverge')
    if (found.rota.join(',') !== canon.rota.join(',')) errs.push(found.de + '→' + found.para + ': rota')
    if (found.ponte !== canon.ponte) errs.push(found.de + '→' + found.para + ': ponte')
  }
  for (const [de, para] of [['canal', 'node'], ['node', 'canal'], ['fetch', 'html']]) {
    if (!man().arestas?.find((e) => e.de === de && e.para === para)) {
      errs.push('falta aresta ' + de + '→' + para)
    }
  }
  return errs
}

export function caminho (de, para, hub = HUB) {
  return aresta(de, para).rota
}

export function moveExport (lang) {
  const L = typeof lang === 'string' ? langManifesto(lang) : lang
  if (L.exports) {
    const m = L.exports.find((e) => e.endsWith('_move'))
    if (m) return m
  }
  return L.nome + '_move'
}

export function ponteSql (exSql, corpo, sentido) {
  const fn = sentido < 0 ? exSql.sql_compilar : exSql.sql_descompilar
  if (typeof fn !== 'function') throw new Error('sql_compilar/descompilar em falta')
  const inOff = 1024
  const midOff = 4096
  const n = enc(exSql, inOff, corpo)
  const outLen = fn(inOff, n, midOff)
  return dec(exSql, midOff, outLen)
}

export function traduzWasm ({ de, para, texto, instancias, hub = HUB }) {
  const edge = aresta(de, para)
  const rota = edge.rota
  const Q = paridade(de, para)
  const nomeDe = rota[0]
  const nomePara = rota[rota.length - 1]
  const exDe = instancias.get(nomeDe)
  const exPara = instancias.get(nomePara)
  const exSql = instancias.get(hub)
  if (!exDe || !exPara) throw new Error('wasm de/para não carregado')

  let corpo = moveWasm(exDe, moveExport(nomeDe), texto, -1)

  if (edge.ponte === 'sql_tags' && exSql) {
    corpo = ponteSql(exSql, corpo, -1)
    corpo = ponteSql(exSql, corpo, +1)
  } else if (edge.ponte === 'sql_face' && exSql) {
    if (nomePara === hub) corpo = ponteSql(exSql, corpo, -1)
    else if (nomeDe === hub) corpo = ponteSql(exSql, corpo, +1)
  }

  return {
    texto: moveWasm(exPara, moveExport(nomePara), corpo, +1),
    rota,
    par: Q,
    aresta: edge,
  }
}

const SHELLS_LIST = ['bash', 'node', 'powershell']

async function traduzPasso (de, para, texto, ctx) {
  const edge = aresta(de, para)
  const Q = paridade(de, para)

  if (de === 'canal' && SHELLS_LIST.includes(para)) {
    const r = await absorveBackend(para, texto, ctx)
    return { texto: r.out, rota: edge.rota, par: Q, aresta: edge, meta: r.meta }
  }

  if (SHELLS_LIST.includes(de) && para === 'canal') {
    const r = await absorveBackend(de, texto, ctx)
    return { texto: r.out, rota: edge.rota, par: Q, aresta: edge, meta: r.meta }
  }

  if (de === 'sql' && para === 'sql') {
    await initCelula({ wasmBase: ctx.wasmBase, storage: ctx.storage })
    const r = await execQueryCelula(texto, ctx)
    return { texto: r.out, rota: ['sql'], par: { a: 0, b: 0, quadrante: 'Q00' }, aresta: edge, meta: r.meta }
  }

  if (de === 'fetch' && ['html', 'css', 'js'].includes(para)) {
    const base = ctx.bancoBase || '/banco/'
    const url = base + 'pagina.' + para
    const r = await fetch(url)
    if (!r.ok) throw new Error(url + ': ' + r.status)
    return { texto: await r.text(), rota: edge.rota, par: Q, aresta: edge }
  }

  if (ctx.instancias?.get(de) && ctx.instancias?.get(para)) {
    return traduzWasm({ de, para, texto, instancias: ctx.instancias, hub: HUB })
  }

  throw new Error('passo «' + de + '→' + para + '» sem absorção')
}

export async function traduz ({ de, para, texto, ctx = {} }) {
  const edge = aresta(de, para)
  const Q = paridade(de, para)
  let corpo = texto
  const rota = edge.rota

  if (rota.length === 1) {
    return { texto: corpo, rota, par: Q, aresta: edge }
  }

  if (rota.length === 2) {
    return traduzPasso(rota[0], rota[1], corpo, ctx)
  }

  for (let i = 0; i < rota.length - 1; i++) {
    const passo = await traduzPasso(rota[i], rota[i + 1], corpo, ctx)
    corpo = passo.texto
  }

  return { texto: corpo, rota, par: Q, aresta: edge }
}

/** Shell absorvido na arena — mesmo caminho que §W10 node. */
export async function traduzShell (script, ctx, backend = 'node') {
  if (ctx.canal) {
    return traduz({ de: 'canal', para: backend, texto: script, ctx })
  }
  const r = await absorveBackend(backend, script, ctx)
  return { texto: r.out, rota: [backend], par: { a: 0, b: 0, quadrante: 'Q00', de: backend, para: backend }, aresta: null, meta: r.meta }
}

export async function carregaInstanciasPipe (base = '/wasm/') {
  const instancias = new Map()
  await Promise.all(PIPE.map(async (nome) => {
    const L = langManifesto(nome)
    instancias.set(nome, await loadWasm(base, L.wasm))
  }))
  return instancias
}

export function tabelaParidade (nomes = PIPE) {
  const rows = []
  for (const a of nomes) {
    for (const b of nomes) {
      if (a === b) continue
      try {
        const p = paridade(a, b)
        rows.push({ de: a, para: b, a: p.a, b: p.b, Q: p.quadrante, caminho: caminho(a, b).join(' → ') })
      } catch { /* */ }
    }
  }
  return rows
}

export { HUB, PIPE }