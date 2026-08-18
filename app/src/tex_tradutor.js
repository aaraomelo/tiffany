// ── O TRADUTOR .tex→PDF NO CLIENTE: tex.wasm + disco LS, sem TeX Live ──────────
//
// O Aarão: «isso tem que rodar no front do cliente, PDF gerado no front via WASM,
// sem servidor». Localmente o middleware Node era um atalho — em produção o nginx
// só serve ficheiros, e a composição É no browser.
//
// Porta: MOVE(slot, ±1) no DISCO — inicia prende só o banco (0–2); o rascunho
// (fonte/PDF) nasce após MARCO e recua com volta_compila. Host lê slot 14.
// Corpo: mapa GKCORPO no localStorage; inflate no Map JS; fopen miss → poe 1
// no DISCO (import __fich_miss). A estrela não leva o subset inteiro à partida.
import manifesto from './corpo.json'
import docsTradutor from './docs_tradutor.json'
import {
  ficheirosPara, gravaCorpo, leMapa, leFicheiro, mapaBate, apagaCorpo, bytesLS,
  resolveCorpoNome,
} from './corpo_disco.js'
import { absorve } from './estrela_porta.js'

const DOCS = { ...docsTradutor.docs }

let motor = null // { exports, poe: Set, cache: Map, miss: { n, bytes } }
let corpo = null // { origem, mapa, ms, msFetch, msGrava }

function num (x) {
  // long → i64 no módulo: o motor devolve BigInt; os índices do ArrayBuffer pedem Number
  return typeof x === 'bigint' ? Number(x) : x
}

function parseRegua (v) {
  if (v == null) return { reguaL: null, reguaC: null }
  return { reguaL: Math.floor(v / 100), reguaC: v % 100 }
}

/** Lê /SementeEstrela do PDF — a torre viaja no documento. */
function parseSemente (latin) {
  const i = latin.indexOf('/Type/SementeEstrela')
  if (i < 0) return null
  const chunk = latin.slice(i, i + 400)
  const numF = (k) => {
    const m = chunk.match(new RegExp('/' + k + '\\s+(\\d+)'))
    return m ? Number(m[1]) : null
  }
  const piN = numF('PiN')
  const lado = numF('Lado')
  const { reguaL, reguaC } = parseRegua(numF('Regua'))
  return {
    alcance: numF('Alcance'),
    dim: numF('Dim'),
    induc: numF('Induc'),
    lado,
    norma: numF('Norma'),
    iface: numF('Interface'),
    reguaL,
    reguaC,
    pi: piN != null ? piN / 1e9 : null,
    gentil: lado === 1,
  }
}

/** Lê /AssinaturaOito — selo Lei 8; TorreDim/TorreN sobem com a torre. */
function parseAssinatura (latin) {
  const i = latin.indexOf('/Type/AssinaturaOito')
  if (i < 0) return null
  const chunk = latin.slice(i, i + 120)
  const numF = (k) => {
    const m = chunk.match(new RegExp('/' + k + '\\s+(\\d+)'))
    return m ? Number(m[1]) : null
  }
  return { n: numF('N'), torreDim: numF('TorreDim'), torreN: numF('TorreN') }
}

/** Lê /AssinaturaTorre — selo Gentil (dim≥16); NTT min(512, dim·32). */
function parseAssinaturaTorre (latin) {
  const i = latin.indexOf('/Type/AssinaturaTorre')
  if (i < 0) return null
  const chunk = latin.slice(i, i + 120)
  const numF = (k) => {
    const m = chunk.match(new RegExp('/' + k + '\\s+(\\d+)'))
    return m ? Number(m[1]) : null
  }
  const a = latin.indexOf('[', i)
  if (a < 0) return { n: numF('N'), torreDim: numF('TorreDim'), sel: null }
  const sel = []
  let w = a + 1
  const dig = (w) => latin.charCodeAt(w) - 48
  while (w < latin.length && latin[w] !== ']' && sel.length < 4096) {
    while (w < latin.length && latin[w] === ' ') w++
    if (latin[w] === ']') break
    let u = 0
    while (w < latin.length && latin.charCodeAt(w) >= 48 && latin.charCodeAt(w) <= 57) {
      u = u * 10 + dig(w)
      w++
    }
    sel.push(u)
  }
  return { n: numF('N'), torreDim: numF('TorreDim'), sel }
}

function memView (E) {
  // memory.grow invalida o buffer — sempre se reata
  return new Uint8Array(E.DISCO.buffer)
}

function reserva (E, n) {
  const p = typeof E.vfs_reserva === 'function'
    ? num(E.vfs_reserva(n))
    : num(E.malloc(BigInt(n)))
  if (!p) throw new Error('tex.wasm: disco cheio')
  return p
}

function poeBytes (E, nome, bytes) {
  const enc = new TextEncoder()
  const nb = enc.encode(nome)
  const pNome = reserva(E, nb.length + 1)
  const pDados = reserva(E, (bytes.length || 0) + 1)
  const v = memView(E)
  v.set(nb, pNome)
  v[pNome + nb.length] = 0
  if (bytes.length) v.set(bytes, pDados)
  v[pDados + bytes.length] = 0
  if (!E.poe_ficheiro(pNome, pDados, bytes.length))
    throw new Error('tex.wasm: poe_ficheiro recusou ' + nome)
}

function cstr (E, ptr) {
  const v = memView(E)
  let s = ''
  for (let i = ptr; i < v.length && v[i]; i++) s += String.fromCharCode(v[i])
  return s
}

function bytesCache (nome) {
  const can = resolveCorpoNome(nome, motor.cache)
  if (!can) return null
  return { nome: can, u8: motor.cache.get(can) }
}

/** síncrono: o Map já tem o inflate; só copia 1 ficheiro para o DISCO. */
function fichMiss (ptr) {
  if (!motor) return 0
  const E = motor.exports
  const pedido = cstr(E, ptr)
  const hit = bytesCache(pedido)
  if (!hit) return 0
  if (motor.poe.has(hit.nome)) return 1
  poeBytes(E, hit.nome, hit.u8)
  motor.poe.add(hit.nome)
  motor.miss.n++
  motor.miss.bytes += hit.u8.length
  return 1
}

function storage () {
  try {
    if (typeof localStorage === 'undefined') return null
    return localStorage
  } catch {
    return null
  }
}

async function fetchCorpo (caminho) {
  const r = await fetch('/corpo/' + caminho)
  if (!r.ok) throw new Error(`corpo/${caminho} → ${r.status}`)
  return new Uint8Array(await r.arrayBuffer())
}

async function garanteCorpo () {
  if (corpo) return corpo
  const t0 = performance.now()
  const ls = storage()
  const lista = manifesto.ficheiros
  const mapaLS = ls && leMapa(ls)
  if (mapaLS && mapaBate(mapaLS, manifesto.soma, lista)) {
    corpo = { origem: 'localStorage', mapa: mapaLS, ms: Math.round(performance.now() - t0) }
    return corpo
  }
  const pares = new Array(lista.length)
  const tFetch = performance.now()
  await Promise.all(lista.map(async (f, i) => {
    pares[i] = { nome: f, bytes: await fetchCorpo(f) }
  }))
  const msFetch = Math.round(performance.now() - tFetch)
  let msGrava = 0
  let mapa = { soma: manifesto.soma, slots: pares.map((p) => ({ nome: p.nome, tam: p.bytes.length })) }
  if (ls) {
    const t1 = performance.now()
    try {
      await gravaCorpo(ls, pares, manifesto.soma)
      mapa = leMapa(ls) || mapa
      msGrava = Math.round(performance.now() - t1)
    } catch {
      try { apagaCorpo(ls) } catch { /* quota: o fetch já tem os bytes desta sessão */ }
      corpo = {
        origem: 'fetch', mapa, pares, ms: Math.round(performance.now() - t0), msFetch, msGrava: 0,
      }
      return corpo
    }
  }
  // bytes ficam no LS; não se guarda a segunda cópia JS
  corpo = { origem: ls ? 'fetch+LS' : 'fetch', mapa, ms: Math.round(performance.now() - t0), msFetch, msGrava }
  return corpo
}

async function bytesDe (nome) {
  if (corpo.pares) {
    const p = corpo.pares.find((x) => x.nome === nome)
    if (p) return p.bytes
  }
  const ls = storage()
  if (ls && corpo.mapa) {
    const u8 = await leFicheiro(ls, corpo.mapa, nome)
    if (u8) return u8
  }
  return fetchCorpo(nome)
}

/** Infla o subset no Map JS — ainda não toca no DISCO. */
async function encheCache (fonte) {
  const nomes = ficheirosPara(fonte, manifesto.ficheiros)
  let n = 0
  let bytes = 0
  const t0 = performance.now()
  for (const nome of nomes) {
    if (motor.cache.has(nome)) continue
    const u8 = await bytesDe(nome)
    motor.cache.set(nome, u8)
    n++
    bytes += u8.length
  }
  return { n, bytes, ms: Math.round(performance.now() - t0), lista: nomes.length }
}

async function carregaMotor () {
  if (motor) return motor
  const buf = await (await fetch('/wasm/tex.wasm')).arrayBuffer()
  const mod = await WebAssembly.compile(buf)
  const needs = WebAssembly.Module.imports(mod).some(
    (i) => i.module === 'env' && i.name === '__fich_miss')
  const instance = needs
    ? await WebAssembly.instantiate(mod, { env: { __fich_miss: (ptr) => fichMiss(ptr) } })
    : await WebAssembly.instantiate(mod)
  const E = instance.exports
  if (typeof E.inicia_wasm !== 'function')
    throw new Error('tex.wasm sem inicia_wasm — reconstrói com tools/sobe_tex_wasm.sh')
  if (typeof E.compila_ficheiro !== 'function')
    throw new Error('tex.wasm sem compila_ficheiro')
  E.inicia_wasm()
  motor = { exports: E, poe: new Set(), cache: new Map(), miss: { n: 0, bytes: 0 } }
  return motor
}

/** Compõe `id` (chave de DOCS) no browser e devolve { bytes, ms, disco }. */
export async function comporDoc (id) {
  const fonte = DOCS[id]
  if (!fonte) throw new Error(`documento desconhecido: ${id}`)
  const t0 = performance.now()
  const { exports: E } = await carregaMotor()
  const c = await garanteCorpo()
  /* 1 bit: recua o monte; o banco LS/Map fica. Cache no JS; DISCO só no fopen. */
  if (typeof E.volta_compila === 'function') E.volta_compila()
  motor.poe.clear()
  motor.miss = { n: 0, bytes: 0 }
  const cache = await encheCache(fonte)
  if (typeof E.marca_vfs === 'function') E.marca_vfs()
  E.limpa_saida()
  const enc = new TextEncoder()
  const nEnt = enc.encode(fonte)
  const nSai = enc.encode('saida.pdf')
  const pEnt = reserva(E, nEnt.length + 1)
  const pSai = reserva(E, nSai.length + 1)
  const v = memView(E)
  v.set(nEnt, pEnt); v[pEnt + nEnt.length] = 0
  v.set(nSai, pSai); v[pSai + nSai.length] = 0
  const tComp = performance.now()
  const rc = E.compila_ficheiro(pEnt, pSai)
  const msCompila = Math.round(performance.now() - tComp)
  if (rc !== 0) throw new Error(`compila_ficheiro(${fonte}) → ${rc}`)
  const n = num(E.tam_saida())
  // Lei 1: +1 absorve o PDF no slot 14 — mesma porta que o painel (estrela_porta)
  const addr = typeof E.MOVE === 'function' ? absorve(E, 14) : num(E.end_saida())
  if (n < 100 || !addr) throw new Error(`saída vazia (${n} bytes)`)
  const out = memView(E).slice(addr, addr + n)
  if (out[0] !== 0x25 || out[1] !== 0x50) // %P
    throw new Error('a saída não começa por %PDF')
  const latin = new TextDecoder('latin1').decode(out)
  if (!latin.includes('%%EOF'))
    throw new Error('a saída não fecha com %%EOF')
  // Lei 7 = circuito tex↔pdf (octonião dual). Lei 8 = selo Caelum abaixo.
  if (!latin.includes('/Type/SementeEstrela'))
    throw new Error('Alonzo: falta /SementeEstrela — a composição não viajou')
  if (!latin.includes('/Type/AssinaturaOito'))
    throw new Error('Caelum (Lei 8): falta /AssinaturaOito — o esqueleto não assinou')
  const estrela = parseSemente(latin)
  const oito = parseAssinatura(latin)
  const torre = parseAssinaturaTorre(latin)
  // Gentil (dim≥16): o selo de torre viaja no PDF quando a composição sobe
  if (estrela && estrela.dim != null && estrela.dim >= 16 && !torre)
    throw new Error('Gentil: falta /AssinaturaTorre — a torre não assinou')
  const poe = {
    n: motor.miss.n,
    bytes: motor.miss.bytes,
    ms: cache.ms,
    lista: cache.lista,
  }
  const discoPagAntes = E.DISCO.buffer.byteLength
  /* 1 bit: CURSOR ← MARCO. Páginas WASM ficam (banco de páginas) — não é dissipação do passo. */
  const marco = typeof E.volta_compila === 'function' ? num(E.volta_compila()) : 0
  motor.poe.clear()
  const pdfApos = typeof E.MOVE === 'function' ? absorve(E, 14) : 0
  const ls = storage()
  return {
    bytes: out,
    ms: Math.round(performance.now() - t0),
    fonte,
    estrela,
    oito,
    torre,
    disco: {
      origem: c.origem,
      msCorpo: c.ms,
      msFetch: c.msFetch || 0,
      msGrava: c.msGrava || 0,
      msPoe: poe.ms,
      msCompila,
      poeN: poe.n,
      poeBytes: poe.bytes,
      poeLista: poe.lista,
      lsBytes: ls ? bytesLS(ls) : 0,
      discoPag: E.DISCO.buffer.byteLength,
      discoPagAntes,
      marco,
      /* true = rascunho sumiu; páginas iguais = banco de páginas, não λ>0 */
      bit1: pdfApos === 0 && E.DISCO.buffer.byteLength === discoPagAntes,
    },
  }
}

/** Compõe um TEXTO solto (a resposta do daemon) pelo MESMO esquema do PDF:
 *  fonte virtual no Map, fich_miss serve, compila, MOVE(14,+1) absorve.
 *  O embrulho mínimo é do chamador; as fontes/estilo vêm do subset da casa. */
export async function comporTexto (nomeVirtual, tex) {
  const { exports: E } = await carregaMotor()
  await garanteCorpo()
  if (typeof E.volta_compila === 'function') E.volta_compila()
  motor.poe.clear()
  motor.miss = { n: 0, bytes: 0 }
  await encheCache('papers/corpo_computacional.tex')     /* o subset com as fontes da casa */
  motor.cache.set(nomeVirtual, new TextEncoder().encode(tex))
  if (typeof E.marca_vfs === 'function') E.marca_vfs()
  E.limpa_saida()
  const enc = new TextEncoder()
  const nEnt = enc.encode(nomeVirtual)
  const nSai = enc.encode('saida.pdf')
  const pEnt = reserva(E, nEnt.length + 1)
  const pSai = reserva(E, nSai.length + 1)
  const v = memView(E)
  v.set(nEnt, pEnt); v[pEnt + nEnt.length] = 0
  v.set(nSai, pSai); v[pSai + nSai.length] = 0
  const rc = E.compila_ficheiro(pEnt, pSai)
  if (rc !== 0) throw new Error(`comporTexto(${nomeVirtual}) → ${rc}`)
  const n = num(E.tam_saida())
  const addr = typeof E.MOVE === 'function' ? absorve(E, 14) : num(E.end_saida())
  if (n < 100 || !addr) throw new Error(`saída vazia (${n} bytes)`)
  const out = memView(E).slice(addr, addr + n)
  const latin = new TextDecoder('latin1').decode(out)
  if (out[0] !== 0x25 || !latin.includes('%%EOF')) throw new Error('PDF inválido')
  const estrela = parseSemente(latin)
  const oito = parseAssinatura(latin)
  motor.cache.delete(nomeVirtual)
  if (typeof E.volta_compila === 'function') E.volta_compila()
  motor.poe.clear()
  return { bytes: out, estrela, oito }
}

/** Compõe e abre o PDF. `janela` (opcional) é um tab já aberto no click
 *  síncrono — senão o browser bloqueia o popup depois do await. */
export async function abrirDoc (id, janela) {
  const { bytes, ms, disco, estrela, oito, torre } = await comporDoc(id)
  const url = URL.createObjectURL(new Blob([bytes], { type: 'application/pdf' }))
  if (janela && !janela.closed) {
    janela.location = url
  } else {
    window.open(url, '_blank', 'noopener')
  }
  setTimeout(() => URL.revokeObjectURL(url), 60_000)
  return { ms, disco, estrela, oito, torre }
}

export function idDeArquivo (href) {
  const m = /\/docs\/([A-Za-z0-9_-]+)\.pdf/.exec(href || '')
  return m ? m[1] : null
}

export { DOCS }
