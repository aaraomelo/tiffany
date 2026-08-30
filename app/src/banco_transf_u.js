// banco_transf_u.js — F já fechada na Álgebra (fis:def:transf, fis:thm:H).
// Mesmo chi / F que tests/transformada.c e tests/aranha_n.c (ta_chi, ta_F).
// Não é FFT nova. dim X = 8 (fis:thm:base); sem Lei 8.
// O corte χ_k lê o saldo; id selecciona B(id) entre esses cortes.

export const TRANSF_M = 8
export const TRANSF_N = 1 << TRANSF_M

/** χ_k(x) = (−1)^{⟨k,x⟩}, ⟨k,x⟩ = paridade(k ∧ x). Inteiro, ±1. */
export function chi (k, x) {
  let b = (k & x) >>> 0
  let p = 0
  while (b) {
    p ^= b & 1
    b >>>= 1
  }
  return p ? -1 : 1
}

/** F(f)_k = Σ_x f(x) χ_k(x). Matriz H_kx = χ_k(x). Não normalizada. */
export function F (f) {
  const n = f.length
  const hat = new Array(n)
  for (let k = 0; k < n; k++) {
    let s = 0
    for (let x = 0; x < n; x++) s += f[x] * chi(k, x)
    hat[k] = s
  }
  return hat
}

export function norma2 (f) {
  let s = 0
  for (let i = 0; i < f.length; i++) s += f[i] * f[i]
  return s
}

/** Parseval: ‖F(f)‖² = 2^m ‖f‖². Resíduo 0 ou 1. */
export function residuoParseval (f, hat) {
  const n = f.length
  return norma2(hat) === n * norma2(f) ? 0 : 1
}

/** F∘F = 2^m · id. Resíduo 0 ou 1. */
export function residuoVolta (f, hat) {
  const n = f.length
  const volta = F(hat)
  for (let i = 0; i < n; i++) {
    if (volta[i] !== n * f[i]) return 1
  }
  return 0
}

export function supp (hat) {
  const out = []
  for (let k = 0; k < hat.length; k++) {
    if (hat[k] !== 0) out.push(k)
  }
  return out
}

/** Campo G em X: I como vector de comprimento 2^m, ou histograma (ta_campo). */
export function campoDeI (I) {
  const n = TRANSF_N
  if (Array.isArray(I) && I.length === n) return I.slice()
  const f = new Array(n).fill(0)
  if (Array.isArray(I)) {
    for (let i = 0; i < I.length; i++) f[i & (n - 1)] += I[i]
    return f
  }
  if (typeof I === 'string') {
    const bytes = Array.from(new TextEncoder().encode(I))
    return campoDeI(bytes)
  }
  throw new Error('I pede campo em X')
}

export function caractere (k) {
  const f = new Array(TRANSF_N)
  for (let x = 0; x < TRANSF_N; x++) f[x] = chi(k, x)
  return f
}

/**
 * Face que endereça: 8 bits (dim X) a partir de id = hex(sha256).
 * Soma em (Z/2)^8 dos 32 bytes — o ⊕ do grupo, não um operador novo.
 * B(id) = {s}: um carácter, como o canal de tests/banda.c.
 */
export function selectorDoId (idHex) {
  const h = String(idHex || '').replace(/[\s:]+/g, '').toLowerCase()
  if (!/^[0-9a-f]+$/.test(h) || h.length < 2 || h.length % 2 !== 0) {
    throw new Error('id pede hex da banda')
  }
  let s = 0
  for (let i = 0; i < h.length; i += 2) s ^= parseInt(h.slice(i, i + 2), 16)
  return s
}

export function bandaDeId (idHex) {
  return Object.freeze([selectorDoId(idHex)])
}

export function bandasDisjuntas (idA, idB) {
  const a = bandaDeId(idA)
  const b = bandaDeId(idB)
  const setB = new Set(b)
  return a.every((k) => !setB.has(k))
}

export function suppContidoEmB (hat, B) {
  const set = B instanceof Set ? B : new Set(B)
  const s = supp(hat)
  return s.every((k) => set.has(k))
}

/** Valida_C do filtro: supp(F(I)) ⊆ B(id). Pullback dos contratos fica no caller. */
export function validaBanda (I, idHex) {
  const f = campoDeI(I)
  const hat = F(f)
  return suppContidoEmB(hat, bandaDeId(idHex))
}

export function medeTransformada () {
  const f = caractere(1)
  const hat = F(f)
  const parseval = residuoParseval(f, hat)
  const volta = residuoVolta(f, hat)
  return {
    transformada_no_motor: true,
    fonte: 'fis:def:transf',
    teorema: 'fis:thm:H',
    fis: 'fis:obs:U-consome',
    cat: 'cat:nucleo-u',
    m: TRANSF_M,
    n: TRANSF_N,
    parseval,
    volta,
    residuo: parseval === 0 && volta === 0 ? 0 : 1,
    corte: 'chi_k',
    nota: 'F e o chi de transformada.c / aranha_n.c; id selecciona B, nao e F',
  }
}
