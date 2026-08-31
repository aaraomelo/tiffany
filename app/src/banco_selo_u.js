// banco_selo_u.js — uma chave do canal parte-se em N que se complementam.
// Algebra de tests/selo.c: partes + sistema somam 0; recupera-se por diferenças;
// sem o sistema a falta é livre; com ele é única. Uma forja derruba o conjunto
// (validacao_n_qubits). Não é o id do livro. Não vai no JSON S_ESTADO.

import { sha256 } from './banda.js'

/** Z_{2^256}: tamanho da banda do canal. */
export const MOD_BANDA = 1n << 256n

/** Primos do medidor C (selo.c §S3 / §S5). */
export const P_SOMA = 1000003n
export const P_VARREDURA = 97n

export function bytesParaZ (u8, mod = MOD_BANDA) {
  let z = 0n
  const b = u8 instanceof Uint8Array ? u8 : new Uint8Array(u8 || [])
  for (let i = 0; i < b.length; i++) z = (z << 8n) + BigInt(b[i])
  return z % mod
}

export function zParaHex (z, mod = MOD_BANDA) {
  const n = ((z % mod) + mod) % mod
  let bits = 0n
  for (let m = mod - 1n; m > 0n; m >>= 1n) bits++
  const larg = Math.max(1, Math.ceil(Number(bits) / 4))
  return n.toString(16).padStart(larg, '0')
}

export function hexParaZ (h, mod = MOD_BANDA) {
  const t = String(h || '').replace(/^0x/i, '')
  if (!t || !/^[0-9a-fA-F]+$/.test(t)) return 0n
  return BigInt('0x' + t) % mod
}

export function paraZ (v, mod = MOD_BANDA) {
  if (typeof v === 'bigint') return ((v % mod) + mod) % mod
  if (typeof v === 'number') return paraZ(BigInt(v | 0), mod)
  return hexParaZ(v, mod)
}

export function somaZ (xs, mod = MOD_BANDA) {
  let s = 0n
  for (const x of xs || []) s = (s + paraZ(x, mod)) % mod
  return s
}

/** LCG para testes reprodutíveis. Não é cifra. */
export function rngLcg (semente) {
  let x = (semente >>> 0) || 1
  return function () {
    x = (Math.imul(x, 1664525) + 1013904223) >>> 0
    return x & 255
  }
}

export function bytesDeRng (rng, n) {
  const u = new Uint8Array(n | 0)
  if (typeof rng === 'function') {
    for (let i = 0; i < u.length; i++) u[i] = rng() & 255
    return u
  }
  if (typeof crypto !== 'undefined' && crypto.getRandomValues) {
    crypto.getRandomValues(u)
    return u
  }
  throw new Error('sem rng')
}

/**
 * Uma chave K (canal / sistema) → N partes complementares.
 * Σ partes + K ≡ 0 (mod). A última fecha o círculo (selo.c §S3).
 */
export function separaChave (K, n, opts = {}) {
  const mod = opts.mod || MOD_BANDA
  const N = n | 0
  if (N <= 0) return []
  const k = paraZ(K, mod)
  const partes = []
  let acc = 0n
  const rng = opts.rng
  for (let i = 0; i < N - 1; i++) {
    const s = bytesParaZ(bytesDeRng(rng, 32), mod)
    partes.push(s)
    acc = (acc + s) % mod
  }
  const ultima = (mod - ((acc + k) % mod)) % mod
  partes.push(ultima)
  return partes
}

/** Junta: todas as N + canal. Falta uma → não fecha. */
export function fechaSelo (K, partes, mod = MOD_BANDA) {
  if (!partes || !partes.length) return false
  const total = (paraZ(K, mod) + somaZ(partes, mod)) % mod
  return total === 0n
}

/**
 * Qualquer parte sai das outras + o sistema (selo.c §S4).
 * Não se chama para fechar o contrato: o protocolo exige as N entregues.
 */
export function recuperaParte (K, partes, falta, mod = MOD_BANDA) {
  const N = (partes || []).length
  const j = falta | 0
  if (j < 0 || j >= N) return null
  let acc = paraZ(K, mod)
  for (let i = 0; i < N; i++) {
    if (i === j) continue
    if (partes[i] == null || partes[i] === '') return null
    acc = (acc + paraZ(partes[i], mod)) % mod
  }
  return (mod - acc) % mod
}

/**
 * §S5: sem K, a falta percorre o espaço; com K, é uma só.
 * Varrido em Z_p pequeno, como selo.c.
 */
export function contaFalta (partes, falta, K, p = P_VARREDURA) {
  const j = falta | 0
  const conhecidas = []
  for (let i = 0; i < partes.length; i++) {
    if (i !== j) conhecidas.push(paraZ(partes[i], p))
  }
  let sem = 0
  let com = 0
  const k = paraZ(K, p)
  const base = somaZ(conhecidas, p)
  for (let v = 0n; v < p; v++) {
    sem++
    if ((base + v + k) % p === 0n) com++
  }
  return { sem, com, p }
}

/** Compromisso público da parte. Forja → hash diferente → conjunto cai. */
export async function compromisso (parteHex) {
  const d = await sha256(new TextEncoder().encode(String(parteHex || '')))
  let h = ''
  for (let i = 0; i < d.length; i++) h += d[i].toString(16).padStart(2, '0')
  return h
}

export async function compromissosDe (partes, mod = MOD_BANDA) {
  const out = []
  for (const p of partes || []) out.push(await compromisso(zParaHex(paraZ(p, mod), mod)))
  return out
}

/**
 * Validação colectiva: uma forja reprova o conjunto (não a parte).
 * canal público verifica; as N partes têm de bater no compromisso e somar 0.
 */
export async function validaConjunto (canal, partes, comps, mod = MOD_BANDA) {
  const N = (comps || []).length
  if (!N || !partes || partes.length !== N) {
    return { fecha: 0, motivo: 'falta' }
  }
  for (let i = 0; i < N; i++) {
    if (partes[i] == null || partes[i] === '') {
      return { fecha: 0, motivo: 'falta' }
    }
    const c = await compromisso(typeof partes[i] === 'bigint'
      ? zParaHex(partes[i], mod)
      : String(partes[i]))
    if (c !== comps[i]) return { fecha: 0, motivo: 'forja' }
  }
  if (!fechaSelo(canal, partes, mod)) return { fecha: 0, motivo: 'nao-fecha' }
  return { fecha: 1, motivo: 'junção' }
}
