// banco_disco.js — disco da máquina (GKBANCO, mesmo padrão GKCORPO).
// Realização wasm: localStorage = disco (linha de base). Remota: gk/banco/estado.json.
// IDB: o mesmo contrato (getItem/setItem/…) atrás de opts.disco='idb'. Não substitui LS no boot.
// Quota do LS = tecto do contexto; não muda a semântica GKBANCO. O fio S_ESTADO cabe em 16 bits.

import { memoriaLS, deflateU8, inflateU8, u8ParaLatin1, latin1ParaU8 } from './corpo_disco.js'

export const MAGIA = 'GKBANCO'
export const CHAVE_ESTADO = 'gk:banco:estado'
export const CHAVE_MANIFESTO = 'gk:banco:manifesto'
export const chaveWasm = (nome) => 'gk:banco:wasm:' + nome
export const PREFIXO_WASM = 'gk:banco:wasm:'
export const STORE_ESTADO = CHAVE_ESTADO
export const STORE_MANIFESTO = CHAVE_MANIFESTO
export const STORE_WASM = 'gk:banco:wasm'
export const STORE_KV = 'gk:banco'
export const IDB_NOME = MAGIA
export const IDB_VER = 1
/** Tecto do fio (Word.total|e). A quota do LS é o tecto real: corta-se ao QuotaExceeded. */
export const LIMITE_WIRE = 65535
export const LIMITE_OMISSAO = Infinity

const STORES_IDB = [STORE_ESTADO, STORE_MANIFESTO, STORE_WASM, STORE_KV]

/** Literal: chave LS → object store + chave no store. */
export function parteChave (k) {
  const s = String(k)
  if (s === CHAVE_ESTADO) return { store: STORE_ESTADO, key: CHAVE_ESTADO }
  if (s === CHAVE_MANIFESTO) return { store: STORE_MANIFESTO, key: CHAVE_MANIFESTO }
  if (s.indexOf(PREFIXO_WASM) === 0) return { store: STORE_WASM, key: s.slice(PREFIXO_WASM.length) }
  return { store: STORE_KV, key: s }
}

export function chaveDeParte (store, key) {
  if (store === STORE_WASM) return PREFIXO_WASM + key
  return String(key)
}

function storageSobreMapa (mapa, persist) {
  return {
    getItem (k) { return mapa.has(k) ? mapa.get(k) : null },
    setItem (k, v) {
      const ks = String(k)
      const vs = String(v)
      const tinha = mapa.has(ks)
      const prev = tinha ? mapa.get(ks) : undefined
      mapa.set(ks, vs)
      if (persist) {
        try { persist('set', ks, vs) } catch (e) {
          if (tinha) mapa.set(ks, prev)
          else mapa.delete(ks)
          throw e
        }
      }
    },
    removeItem (k) {
      const ks = String(k)
      const tinha = mapa.has(ks)
      const prev = tinha ? mapa.get(ks) : undefined
      mapa.delete(ks)
      if (persist) {
        try { persist('del', ks) } catch (e) {
          if (tinha) mapa.set(ks, prev)
          throw e
        }
      }
    },
    key (i) { return [...mapa.keys()][i] ?? null },
    get length () { return mapa.size },
  }
}

/** Shim Node/testes: Map + as mesmas lojas. API isomorfa ao storage.
 *  abortar() — próxima persist lança AbortError (tx abortada); write-through faz rollback. */
export function memoriaIDB () {
  const stores = {
    [STORE_ESTADO]: new Map(),
    [STORE_MANIFESTO]: new Map(),
    [STORE_WASM]: new Map(),
    [STORE_KV]: new Map(),
  }
  const ctx = { abortar: false }
  const mapa = new Map()
  function persist (op, k, v) {
    if (ctx.abortar) {
      ctx.abortar = false
      const err = new Error('IDB abort')
      err.name = 'AbortError'
      throw err
    }
    const { store, key } = parteChave(k)
    if (op === 'set') stores[store].set(key, v)
    else stores[store].delete(key)
  }
  const st = storageSobreMapa(mapa, persist)
  st._stores = stores
  st.abortar = () => { ctx.abortar = true }
  return st
}

function abreIDBNativo () {
  return new Promise((resolve, reject) => {
    const req = globalThis.indexedDB.open(IDB_NOME, IDB_VER)
    req.onupgradeneeded = () => {
      const db = req.result
      for (const name of STORES_IDB) {
        if (!db.objectStoreNames.contains(name)) db.createObjectStore(name)
      }
    }
    req.onsuccess = () => resolve(req.result)
    req.onerror = () => reject(req.error)
  })
}

function entradasIDB (db, store) {
  return new Promise((resolve, reject) => {
    const tx = db.transaction(store, 'readonly')
    const req = tx.objectStore(store).openCursor()
    const out = []
    req.onsuccess = () => {
      const c = req.result
      if (c) {
        out.push([c.key, c.value])
        c.continue()
      } else resolve(out)
    }
    req.onerror = () => reject(req.error)
  })
}

/** Browser: IndexedDB real hidratado num storage síncrono (write-through). Node: shim. */
export async function abreDiscoIDB (opts = {}) {
  if (opts.storage != null) return opts.storage
  if (typeof globalThis.indexedDB === 'undefined') return memoriaIDB()
  const db = await abreIDBNativo()
  const mapa = new Map()
  for (const store of STORES_IDB) {
    const pares = await entradasIDB(db, store)
    for (const [key, val] of pares) {
      if (val == null) continue
      mapa.set(chaveDeParte(store, key), String(val))
    }
  }
  const persist = (op, k, v) => {
    try {
      const { store, key } = parteChave(k)
      const tx = db.transaction(store, 'readwrite')
      const os = tx.objectStore(store)
      if (op === 'set') os.put(v, key)
      else os.delete(key)
    } catch { /* quota / transacção */ }
  }
  const st = storageSobreMapa(mapa, persist)
  st._idb = db
  return st
}

/** Disco da realização wasm — localStorage por omissão; Map isomórfico em Node/testes.
 *  IDB só se opts.storage já for a ponte, ou opts.idb pré-aberto. Não escolhe IDB sozinho. */
export function discoBrowser (opts = {}) {
  if (opts.storage != null) return opts.storage
  if (opts.disco === 'idb' && opts.idb) return opts.idb
  if (typeof globalThis.localStorage !== 'undefined') return globalThis.localStorage
  return memoriaLS()
}

/** Flag opts.disco==='idb' → ponte IDB (nativo ou shim). Falha → LS. Boot default = LS. */
export async function escolheDisco (opts = {}) {
  if (opts.storage != null) return opts.storage
  if (opts.disco === 'idb') {
    try { return await abreDiscoIDB(opts) } catch { /* cai no LS */ }
  }
  return discoBrowser(opts)
}

export function estadoVazio () {
  return {
    magia: MAGIA,
    v: 2,
    shells: {},
    atualizado: null,
    pendente: [],
  }
}

export function normalizaEstado (o) {
  if (!o || o.magia !== MAGIA || !o.shells || typeof o.shells !== 'object') return estadoVazio()
  return {
    magia: MAGIA,
    v: 2,
    shells: o.shells,
    atualizado: o.atualizado || null,
    pendente: Array.isArray(o.pendente) ? o.pendente : [],
    pagina: o.pagina && typeof o.pagina === 'object' ? o.pagina : undefined,
  }
}

export function gravaPagina (estado, fontes) {
  estado.pagina = {
    html: (fontes && fontes.html) || '',
    css: (fontes && fontes.css) || '',
    js: (fontes && fontes.js) || '',
  }
}

export function lePagina (estado) {
  const p = estado && estado.pagina
  if (!p || typeof p !== 'object') return null
  if (!p.html && !p.css && !p.js) return null
  return { html: p.html || '', css: p.css || '', js: p.js || '' }
}

export function gravaManifestoLS (storage, man) {
  if (!storage || !man) return
  try { storage.setItem(CHAVE_MANIFESTO, JSON.stringify(man)) } catch { /* quota */ }
}

export function leManifestoLS (storage) {
  if (!storage) return null
  try {
    const s = storage.getItem(CHAVE_MANIFESTO)
    return s ? JSON.parse(s) : null
  } catch {
    return null
  }
}

/** Wasm no LS (deflate+latin1, como GKCORPO). Não vai no S_ESTADO — a realização remota tem o C. */
export async function gravaWasmLS (storage, nome, u8) {
  if (!storage || !nome || !u8) return
  try {
    const z = await deflateU8(u8)
    storage.setItem(chaveWasm(nome), u8ParaLatin1(z))
  } catch {
    try { storage.setItem(chaveWasm(nome), u8ParaLatin1(u8)) } catch { /* quota */ }
  }
}

export async function leWasmLS (storage, nome) {
  if (!storage || !nome) return null
  const s = storage.getItem(chaveWasm(nome))
  if (s == null) return null
  const raw = latin1ParaU8(s)
  if (raw.length >= 2 && raw[0] === 0x78) {
    try { return await inflateU8(raw) } catch { return raw }
  }
  return raw
}

export function leEstado (storage = globalThis.localStorage) {
  if (!storage) return estadoVazio()
  try {
    const s = storage.getItem(CHAVE_ESTADO)
    if (!s) return estadoVazio()
    return normalizaEstado(JSON.parse(s))
  } catch {
    return estadoVazio()
  }
}

/** UTF-16 do LS: 2 × (chave+valor), só chaves gk:banco. */
export function bytesBanco (storage) {
  if (!storage) return 0
  if (typeof storage.key === 'function' && typeof storage.length === 'number') {
    let n = 0
    for (let i = 0; i < storage.length; i++) {
      const k = storage.key(i)
      if (!k || k.indexOf('gk:banco') !== 0) continue
      const v = storage.getItem(k) || ''
      n += 2 * (k.length + v.length)
    }
    return n
  }
  const v = storage.getItem(CHAVE_ESTADO) || ''
  return 2 * (CHAVE_ESTADO.length + v.length)
}

export function bytesJson (estado) {
  const enc = typeof TextEncoder !== 'undefined'
    ? new TextEncoder().encode(JSON.stringify(estado)).length
    : Buffer.byteLength(JSON.stringify(estado), 'utf8')
  return enc
}

function tamUtf16 (estado) {
  return 2 * (CHAVE_ESTADO.length + JSON.stringify(estado).length)
}

function maisAntiga (estado) {
  let oldest = null
  let oldestT = null
  for (const [n, s] of Object.entries(estado.shells || {})) {
    const t = (s && s.t) || ''
    if (oldest === null || t < oldestT) {
      oldest = n
      oldestT = t
    }
  }
  return oldest
}

/** Corta shells mais antigas até o estado caber no limite (contexto local ou fio). */
export function cortaContexto (estado, limite, como = 'utf16') {
  if (!(limite < Infinity)) return estado
  const tam = () => como === 'utf8' ? bytesJson(estado) : tamUtf16(estado)
  while (tam() > limite && Object.keys(estado.shells || {}).length > 0) {
    const n = maisAntiga(estado)
    if (!n) break
    delete estado.shells[n]
  }
  if (tam() > limite && Array.isArray(estado.pendente) && estado.pendente.length) {
    estado.pendente = []
  }
  return estado
}

function eQuota (e) {
  return e && (e.name === 'QuotaExceededError' || e.code === 22 || e.code === 1014)
}

export function gravaEstado (estado, storage = globalThis.localStorage, opts = {}) {
  if (!storage) return
  const lim = opts.limite ?? LIMITE_OMISSAO
  estado.atualizado = new Date().toISOString()
  cortaContexto(estado, lim, 'utf16')
  const escreve = () => storage.setItem(CHAVE_ESTADO, JSON.stringify(estado))
  try {
    escreve()
  } catch (e) {
    if (!eQuota(e)) throw e
    while (Object.keys(estado.shells || {}).length > 0) {
      const n = maisAntiga(estado)
      if (!n) break
      delete estado.shells[n]
      try {
        escreve()
        return
      } catch (e2) {
        if (!eQuota(e2)) throw e2
      }
    }
    estado.pendente = []
    try { escreve() } catch { /* quota ainda cheia: GKCORPO comeu o disco */ }
  }
}

export function gravaShell (estado, nome, entrada, saida) {
  estado.shells[nome] = {
    in: entrada ?? estado.shells[nome]?.in ?? '',
    out: saida ?? estado.shells[nome]?.out ?? '',
    t: new Date().toISOString(),
  }
}

export function leShell (estado, nome) {
  return estado.shells[nome] || { in: '', out: '' }
}

/** Mais recente por shell (t ISO). pendente = união. */
export function mergeEstado (a, b) {
  const A = normalizaEstado(a)
  const B = normalizaEstado(b)
  const out = estadoVazio()
  const names = new Set([...Object.keys(A.shells), ...Object.keys(B.shells)])
  for (const n of names) {
    const sa = A.shells[n]
    const sb = B.shells[n]
    if (!sa) out.shells[n] = { ...sb }
    else if (!sb) out.shells[n] = { ...sa }
    else out.shells[n] = (sa.t || '') >= (sb.t || '') ? { ...sa } : { ...sb }
  }
  out.atualizado = (A.atualizado || '') >= (B.atualizado || '') ? A.atualizado : B.atualizado
  if (A.pagina || B.pagina) {
    out.pagina = (A.atualizado || '') >= (B.atualizado || '')
      ? (A.pagina || B.pagina)
      : (B.pagina || A.pagina)
  }
  const seen = new Set()
  for (const p of [...A.pendente, ...B.pendente]) {
    const id = (p.t || '') + '\0' + (p.nome || '') + '\0' + (p.in || '')
    if (seen.has(id)) continue
    seen.add(id)
    out.pendente.push(p)
  }
  return out
}

export function enfileiraPendente (estado, item) {
  if (!estado.pendente) estado.pendente = []
  estado.pendente.push({
    nome: item.nome,
    in: item.in || '',
    t: item.t || new Date().toISOString(),
  })
}

export function limpaPendente (estado) {
  estado.pendente = []
}
