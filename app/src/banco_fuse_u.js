// banco_fuse_u.js — N chaves independentes → compromisso colectivo.
// NÃO é separaChave (1 canal → N pedaços). NÃO é Ed25519 como Z_{2^256}.
// Cada K_i determina id_i = hex(sha256(bytes)) — o mesmo mapa da identidade.
// Fuse = sha256(marca || N || id||C ordenados). Sem soma de chaves.
// cripto:obs:coord-fuse

import { parseChavePublica, bandaDeChavePublica, sha256 } from './banda.js'
import { hexBanda, idEstavelDaChave } from './banco_identidade_u.js'

export const FUSE_MARCA = 'fuse:v1'

function concatBytes (parts) {
  let n = 0
  for (const p of parts) n += p.length
  const out = new Uint8Array(n)
  let o = 0
  for (const p of parts) {
    out.set(p, o)
    o += p.length
  }
  return out
}

function enc (s) {
  return new TextEncoder().encode(String(s))
}

function chaveNorm (k) {
  const t = String(k || '').replace(/[\s:]+/g, '').toLowerCase()
  return parseChavePublica(t) ? t : ''
}

/**
 * Compromisso da chave (bytes), domínio fuse:v1:k.
 * ≠ compromisso(parte) do selo, que hasheia o hex UTF-8.
 */
export async function compromissoDeChave (chaveHex) {
  const raw = parseChavePublica(chaveHex)
  if (!raw || !raw.length) throw new Error('chave')
  return hexBanda(await sha256(concatBytes([enc(FUSE_MARCA + ':k'), raw])))
}

/**
 * Fuse(K_0,…,K_{N-1}): composição criptográfica, ordem canónica por id.
 * Não trata a chave como elemento de Ed25519 nem como soma em Z_{2^256}.
 */
export async function fuse (chaves) {
  const itens = []
  const vistos = new Set()
  for (const k of chaves || []) {
    const n = chaveNorm(k)
    if (!n) return { ok: 0, sigma: '', ids: [], comps: [], motivo: 'chave' }
    const id = await idEstavelDaChave(n)
    if (vistos.has(id)) return { ok: 0, sigma: '', ids: [], comps: [], motivo: 'clone' }
    vistos.add(id)
    itens.push({ id, C: await compromissoDeChave(n), chave: n })
  }
  if (!itens.length) return { ok: 0, sigma: '', ids: [], comps: [], motivo: 'falta' }
  itens.sort((a, b) => (a.id < b.id ? -1 : a.id > b.id ? 1 : 0))
  const chunks = [enc(FUSE_MARCA + ':N' + itens.length)]
  for (const it of itens) chunks.push(enc('|' + it.id + '|' + it.C))
  const sigma = hexBanda(await sha256(concatBytes(chunks)))
  return {
    ok: 1,
    sigma,
    ids: itens.map((x) => x.id),
    comps: itens.map((x) => x.C),
    motivo: 'fuse',
  }
}

/** Fecha só com as N chaves independentes; uma falta ou forja derruba o conjunto. */
export async function validaFuse (chaves, sigma) {
  const N = (chaves || []).length
  if (!N) return { fecha: 0, motivo: 'falta', sigma: '', ids: [] }
  for (const k of chaves) {
    if (!chaveNorm(k)) return { fecha: 0, motivo: 'falta', sigma: '', ids: [] }
  }
  const r = await fuse(chaves)
  if (!r.ok) return { fecha: 0, motivo: r.motivo, sigma: '', ids: [] }
  if (sigma && r.sigma !== String(sigma)) {
    return { fecha: 0, motivo: 'forja', sigma: r.sigma, ids: r.ids }
  }
  return { fecha: 1, motivo: 'junção-fuse', sigma: r.sigma, ids: r.ids }
}

export { bandaDeChavePublica, idEstavelDaChave }
