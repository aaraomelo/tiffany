// banco_coord_u.js — coordenação de trabalho no banco, usando o livro por id.
// NÃO é consenso. NÃO estende Liquidacao. NÃO é a Cadeia de Cristais.
//
// rede → banco → livro (recibo de atribuição)
// O job numérico vive no banco; R_i continua a ser liquidação de termos de grau 2.
// B não distribui. cripto:obs:coord

import { CONTRATOS, CHAVE_CADEIA, liquidaNaCadeia, leLivro } from './banco_cristalchain_u.js'
import {
  MOD_BANDA, bytesParaZ, zParaHex, separaChave, validaConjunto, compromisso,
} from './banco_selo_u.js'
import { validaFuse } from './banco_fuse_u.js'

export const CHAVE_COORD = 'gk:banco:coord:'

/** Testemunha de atribuição: termos que já liquidam. A faixa NÃO entra aqui. */
export const TERMOS_TESTEMUNHA = CONTRATOS[0].termos

export function chaveJob (jobId) {
  return CHAVE_COORD + String(jobId || '')
}

/** J = {0..n-1} em w faixas contíguas. */
export function parteFaixas (n, w) {
  const N = n | 0
  const W = w | 0
  const out = []
  if (N < 0 || W <= 0) return out
  for (let i = 0; i < W; i++) {
    const a = Math.floor(i * N / W)
    const b = Math.floor((i + 1) * N / W)
    out.push({ i, a, b })
  }
  return out
}

export function somaFaixa (a, b) {
  let s = 0
  for (let k = a | 0; k < (b | 0); k++) s += k
  return s
}

export function somaMonolito (n) {
  const N = n | 0
  if (N <= 0) return 0
  return (N * (N - 1)) / 2
}

export function chaveParte (jobId, i) {
  return CHAVE_COORD + String(jobId || '') + ':parte:' + (i | 0)
}

export function leParte (storage, jobId, i) {
  if (!storage || !jobId) return null
  return storage.getItem(chaveParte(jobId, i))
}

export function gravaParte (storage, jobId, i, hex) {
  if (!storage || !jobId || hex == null) return
  storage.setItem(chaveParte(jobId, i), String(hex))
}

function seloDo (o) {
  return {
    canal: o.canal ? String(o.canal) : '',
    compromissos: Array.isArray(o.compromissos) ? o.compromissos.map(String) : [],
    modo: o.modo === 'fuse' ? 'fuse' : 'parte',
    sigma: o.sigma ? String(o.sigma) : '',
  }
}

function copiaSelo (dst, src) {
  if (!dst || !src) return dst
  if (src.canal && !dst.canal) dst.canal = String(src.canal)
  if (src.compromissos && src.compromissos.length &&
      !(dst.compromissos && dst.compromissos.length)) {
    dst.compromissos = src.compromissos.map(String)
  }
  if (src.modo === 'fuse') dst.modo = 'fuse'
  if (src.sigma && !dst.sigma) dst.sigma = String(src.sigma)
  return dst
}

export function leJob (storage, jobId) {
  if (!storage || !jobId) return null
  try {
    const s = storage.getItem(chaveJob(jobId))
    if (!s) return null
    const o = JSON.parse(s)
    const selo = seloDo(o)
    return {
      id: String(o.id || jobId),
      n: o.n | 0,
      workers: o.workers | 0,
      pecas: o.pecas && typeof o.pecas === 'object' ? { ...o.pecas } : {},
      canal: selo.canal,
      compromissos: selo.compromissos,
      modo: selo.modo,
      sigma: selo.sigma,
    }
  } catch {
    return null
  }
}

export function gravaJob (storage, job) {
  if (!storage || !job || !job.id) return
  const selo = seloDo(job)
  const out = {
    id: job.id,
    n: job.n | 0,
    workers: job.workers | 0,
    pecas: job.pecas || {},
  }
  if (selo.canal) out.canal = selo.canal
  if (selo.compromissos.length) out.compromissos = selo.compromissos
  if (selo.modo === 'fuse') out.modo = 'fuse'
  if (selo.sigma) out.sigma = selo.sigma
  storage.setItem(chaveJob(job.id), JSON.stringify(out))
}

export function jobVazio (jobId, n, workers) {
  return {
    id: String(jobId || ''),
    n: n | 0,
    workers: workers | 0,
    pecas: {},
    canal: '',
    compromissos: [],
    modo: 'parte',
    sigma: '',
  }
}

export function nomeFaixa (jobId, i) {
  return 'coord:' + String(jobId || '') + ':faixa:' + (i | 0)
}

/**
 * Abre o job: a banda do canal (todos a carregam) parte-se em N.
 * Devolve as partes para distribuir; o disco do coord fica só com
 * canal + compromissos (a origem pública, não as partes).
 */
export async function abreJob (opts = {}) {
  const storage = opts.storage
  const jobId = String(opts.jobId || '')
  const n = opts.n | 0
  const workers = opts.workers | 0
  const canalBytes = opts.canal instanceof Uint8Array
    ? opts.canal
    : new Uint8Array(opts.canal || [])
  if (!storage || !jobId || workers <= 0 || canalBytes.length === 0) {
    return { job: null, partes: [] }
  }
  const K = bytesParaZ(canalBytes)
  const partesZ = separaChave(K, workers, { rng: opts.rng, mod: MOD_BANDA })
  const partes = partesZ.map((z) => zParaHex(z))
  const comps = []
  for (const p of partes) comps.push(await compromisso(p))
  const job = {
    id: jobId,
    n,
    workers,
    pecas: {},
    canal: zParaHex(K),
    compromissos: comps,
  }
  gravaJob(storage, job)
  return { job, partes }
}

/**
 * Abre o job em modo fuse: N chaves independentes, sem partir o canal.
 * A banda do transporte não é K_contrato. cripto:obs:coord-fuse
 */
export async function abreJobFuse (opts = {}) {
  const storage = opts.storage
  const jobId = String(opts.jobId || '')
  const n = opts.n | 0
  const workers = opts.workers | 0
  if (!storage || !jobId || workers <= 0) return { job: null }
  const job = {
    id: jobId,
    n,
    workers,
    pecas: {},
    canal: '',
    compromissos: [],
    modo: 'fuse',
    sigma: '',
  }
  gravaJob(storage, job)
  return { job }
}

/** Todos carregam a chave do canal + os compromissos. Sem as partes. */
export function entregaCanal (de, para, jobId) {
  const src = leJob(de, jobId)
  if (!src || !src.canal) return { entregou: 0, job: leJob(para, jobId) }
  let dst = leJob(para, jobId) || jobVazio(jobId, src.n, src.workers)
  dst.n = src.n || dst.n
  dst.workers = src.workers || dst.workers
  const tinha = dst.canal === src.canal &&
    dst.compromissos.length === src.compromissos.length
  dst.canal = src.canal
  dst.compromissos = src.compromissos.slice()
  gravaJob(para, dst)
  return { entregou: tinha ? 0 : 1, job: dst }
}

/** Entrega a parte i (KV, não S_ESTADO). */
export function entregaParte (de, para, jobId, i) {
  const p = leParte(de, jobId, i)
  if (!p) return { entregou: 0 }
  if (leParte(para, jobId, i) === p) return { entregou: 0 }
  gravaParte(para, jobId, i, p)
  return { entregou: 1 }
}

/**
 * Worker id_i recebe a faixa J_i.
 * 1) liquida testemunha nomeada no livro do dono (idempotente);
 * 2) o resultado numérico grava-se no banco, fora de Liquidacao.
 */
export function correFaixa (opts = {}) {
  const storage = opts.storage
  const dono = opts.dono
  const jobId = opts.jobId
  const i = opts.i | 0
  const a = opts.a | 0
  const b = opts.b | 0
  const n = opts.n | 0
  const workers = opts.workers | 0
  const nome = nomeFaixa(jobId, i)
  const liq = liquidaNaCadeia(TERMOS_TESTEMUNHA, {
    dono,
    camada: opts.camada,
    storage,
    nome,
    q: opts.q || 12,
  })
  if (!liq.liquidado) {
    return { liq, peca: null, job: leJob(storage, jobId) }
  }
  let job = leJob(storage, jobId) || jobVazio(jobId, n, workers)
  job.n = n || job.n
  job.workers = workers || job.workers
  if (!job.pecas[i]) {
    const parte = opts.parte || leParte(storage, jobId, i) || ''
    job.pecas[i] = {
      dono: String(dono),
      i,
      a,
      b,
      soma: somaFaixa(a, b),
    }
    if (parte) job.pecas[i].parte = String(parte)
    if (opts.chave) job.pecas[i].chave = String(opts.chave)
    if (opts.idFuse) job.pecas[i].idFuse = String(opts.idFuse)
    gravaJob(storage, job)
  }
  return { liq, peca: job.pecas[i], job }
}

/** Junta pecas; incompleto → null. Não toca em B. */
export function juntaJob (job) {
  if (!job || !job.pecas) return null
  const w = job.workers | 0
  if (w <= 0) return null
  let soma = 0
  for (let i = 0; i < w; i++) {
    const p = job.pecas[i]
    if (!p || typeof p.soma !== 'number') return null
    soma += p.soma
  }
  return soma
}

/**
 * O contrato fecha só com as N partes + a chave do canal.
 * Não recupera a que falta (S4 é álgebra, não atalho).
 * Uma forja derruba o conjunto. Junta numérica sem selo continua juntaJob.
 */
export async function fechaContrato (job) {
  const junta = juntaJob(job)
  if (job && job.modo === 'fuse') {
    const chaves = []
    for (let i = 0; i < (job.workers | 0); i++) {
      chaves.push(job.pecas[i] && job.pecas[i].chave ? job.pecas[i].chave : '')
    }
    const v = await validaFuse(chaves)
    if (!v.fecha) return { fecha: 0, junta: null, motivo: v.motivo, sigma: v.sigma || '', ids: v.ids || [] }
    if (junta === null) return { fecha: 0, junta: null, motivo: 'junta', sigma: v.sigma, ids: v.ids }
    return { fecha: 1, junta, motivo: v.motivo, sigma: v.sigma, ids: v.ids }
  }
  if (!job || !job.canal || !job.compromissos || !job.compromissos.length) {
    return { fecha: 0, junta, motivo: 'sem-selo' }
  }
  const partes = []
  for (let i = 0; i < (job.workers | 0); i++) {
    partes.push(job.pecas[i] && job.pecas[i].parte ? job.pecas[i].parte : '')
  }
  const v = await validaConjunto(job.canal, partes, job.compromissos)
  if (!v.fecha) return { fecha: 0, junta: null, motivo: v.motivo }
  if (junta === null) return { fecha: 0, junta: null, motivo: 'junta' }
  return { fecha: 1, junta, motivo: v.motivo }
}

/**
 * Copia pecas[i] do disco do worker para o disco do coordenador.
 * Não é S_ESTADO: o documento de estado não leva o recibo.
 * Idempotente: a mesma peca não duplica.
 */
export function entregaPeca (de, para, jobId, i) {
  const src = leJob(de, jobId)
  const idx = i | 0
  if (!src || !src.pecas[idx]) return { entregou: 0, peca: null, job: leJob(para, jobId) }
  let dst = leJob(para, jobId) || jobVazio(jobId, src.n, src.workers)
  dst.n = src.n || dst.n
  dst.workers = src.workers || dst.workers
  copiaSelo(dst, src)
  if (!dst.pecas[idx]) {
    dst.pecas[idx] = { ...src.pecas[idx] }
    gravaJob(para, dst)
    return { entregou: 1, peca: dst.pecas[idx], job: dst }
  }
  return { entregou: 0, peca: dst.pecas[idx], job: dst }
}

/** Copia o livro do dono (chave KV). Fora do JSON S_ESTADO. */
export function entregaLivro (de, para, dono) {
  if (!de || !para || !dono) return { entregou: 0 }
  const k = CHAVE_CADEIA + String(dono)
  const s = de.getItem(k)
  if (!s) return { entregou: 0 }
  if (para.getItem(k) === s) return { entregou: 0 }
  para.setItem(k, s)
  return { entregou: 1 }
}

/** O documento GKBANCO (shells/página) não contém o recibo nem o job. */
export function estadoCegoAoRecibo (estado) {
  const j = JSON.stringify(estado || {})
  return j.indexOf('cristalchain') < 0 && j.indexOf('gk:banco:coord') < 0
}

export function clonaStorage (src) {
  const dst = {
    _m: new Map(),
    getItem (k) { return this._m.has(k) ? this._m.get(k) : null },
    setItem (k, v) { this._m.set(String(k), String(v)) },
    removeItem (k) { this._m.delete(String(k)) },
    key (i) { return [...this._m.keys()][i] ?? null },
    get length () { return this._m.size },
  }
  if (!src) return dst
  for (let i = 0; i < src.length; i++) {
    const k = src.key(i)
    if (k) dst.setItem(k, src.getItem(k))
  }
  return dst
}

export { leLivro }
export {
  MOD_BANDA, separaChave, fechaSelo, recuperaParte, validaConjunto,
} from './banco_selo_u.js'
export { fuse, validaFuse } from './banco_fuse_u.js'
