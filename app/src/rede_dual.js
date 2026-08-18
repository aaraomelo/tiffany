// rede_dual.js — Realização na UI do Teorema da Rede Neural Dual
// (papers/corpo_topologico.tex §rede-dual; Dual Sort: W=-I, b=c).
//
//   P (estaca/perceptron)  ↔  ℋ (banda)  ↔  H (memória associativa)
//
// Hopfield = memória da volta; λ⁻ vem da dualidade reversível, não do atrator sozinho.
// Metrónomo atesta λ⁺+λ⁻≈0 quando a volta fecha.

import { BATUTA } from './maestro.js'

const DELTA_DEFAULT = 0.18
const N_BITS = 32

/** Feature escalar em [-1,1] a partir do texto (estaca 1-D). */
export function featScalar (texto) {
  const s = String(texto || '')
  let h = 2166136261 >>> 0
  for (let i = 0; i < s.length; i++) {
    h ^= s.charCodeAt(i)
    h = Math.imul(h, 16777619) >>> 0
  }
  // mapa uniforme → [-1,1]
  return (h / 0xffffffff) * 2 - 1
}

/** Padrão bipolar ±1 de comprimento N_BITS (Hopfield). */
export function featBits (texto) {
  const s = String(texto || '')
  const bits = new Int8Array(N_BITS)
  let h = 2166136261 >>> 0
  for (let i = 0; i < s.length; i++) {
    h ^= s.charCodeAt(i)
    h = Math.imul(h, 16777619) >>> 0
  }
  for (let i = 0; i < N_BITS; i++) {
    const b = (h >>> (i % 32)) & 1
    bits[i] = b ? 1 : -1
    h = Math.imul(h ^ (i * 2654435761), 1597334677) >>> 0
  }
  return bits
}

/**
 * Perceptron da estaca: u = c - x  (W=-1, b=c).
 * Dois limiares: banda [c-Δ, c+Δ] ⇔ |u|≤Δ.
 */
export function potencialEstaca (x, c = 0) {
  return c - x
}

/**
 * Actualização da memória de estado h (Teorema da Retenção Neural).
 * |u|≤Δ ⇒ h'=h.
 */
export function atualizaH (u, h, delta = DELTA_DEFAULT) {
  if (u > delta) return 1
  if (u < -delta) return -1
  return h
}

/** Hebb: W = Σ ξξᵀ / N, diagonal 0. */
export function hebbW (padroes) {
  const W = Array.from({ length: N_BITS }, () => new Float64Array(N_BITS))
  if (!padroes.length) return W
  const n = padroes.length
  for (const xi of padroes) {
    for (let i = 0; i < N_BITS; i++) {
      for (let j = 0; j < N_BITS; j++) {
        if (i === j) continue
        W[i][j] += (xi.bits[i] * xi.bits[j]) / n
      }
    }
  }
  return W
}

/** Um passo assíncrono tipo Hopfield → atrator. */
export function hopfieldRecall (bits, W, passos = 8) {
  const x = Int8Array.from(bits)
  for (let t = 0; t < passos; t++) {
    for (let i = 0; i < N_BITS; i++) {
      let s = 0
      for (let j = 0; j < N_BITS; j++) s += W[i][j] * x[j]
      x[i] = s >= 0 ? 1 : -1
    }
  }
  return x
}

function overlap (a, b) {
  let s = 0
  for (let i = 0; i < N_BITS; i++) s += a[i] * b[i]
  return s / N_BITS
}

/**
 * Estado persistente da rede dual na sessão.
 * X=(x,h): x = feature; h = memória de retenção (Def. sistema híbrido).
 * c = centro da estaca; padroes = ξ Hopfield (memória associativa ≠ inversão).
 */
export function criarEstadoRede ({ c = 0, delta = DELTA_DEFAULT } = {}) {
  return {
    c,
    delta,
    x: 0,
    h: 0,
    padroes: [], // { bits, fala, Y }
    ultimoY: '',
    ultimaFala: '',
    // ganhos locais aproximados (assinatura; conjugação medida pelo Metrónomo)
    lambdaP: 0,
    lambdaH: 0,
  }
}

/**
 * Dinâmica de decisão F_P e dual conjugado F_H = D∘F_P⁻¹∘D⁻¹
 * (D = estaca Dual Sort). α=|DF_P|; Hopfield NÃO entra aqui.
 */
export function mapaFrente (x, c, alpha = 1.7) {
  return c + alpha * (x - c)
}

export function mapaVoltaConjugada (x, c, alpha = 1.7) {
  const D = (z) => 2 * c - z
  const FpInv = (y) => c + (y - c) / alpha
  return D(FpInv(D(x)))
}

/** Álgebra linear 2×2 mínima (conjugação em dim>1). */
function matVec2 (A, v) {
  return [
    A[0][0] * v[0] + A[0][1] * v[1],
    A[1][0] * v[0] + A[1][1] * v[1],
  ]
}
function matMul2 (A, B) {
  return [
    [A[0][0] * B[0][0] + A[0][1] * B[1][0], A[0][0] * B[0][1] + A[0][1] * B[1][1]],
    [A[1][0] * B[0][0] + A[1][1] * B[1][0], A[1][0] * B[0][1] + A[1][1] * B[1][1]],
  ]
}
function matInv2 (A) {
  const det = A[0][0] * A[1][1] - A[0][1] * A[1][0]
  return [
    [A[1][1] / det, -A[0][1] / det],
    [-A[1][0] / det, A[0][0] / det],
  ]
}
function matDet2 (A) {
  return A[0][0] * A[1][1] - A[0][1] * A[1][0]
}
function matId2 () {
  return [[1, 0], [0, 1]]
}
function matSubFro2 (A, B) {
  let s = 0
  for (let i = 0; i < 2; i++) {
    for (let j = 0; j < 2; j++) s += (A[i][j] - B[i][j]) ** 2
  }
  return Math.sqrt(s)
}

/** F_P(x)=c+A(x−c) em ℝ²; F_H=D∘F_P⁻¹∘D⁻¹ com D(x)=2c−x. */
export function mapaFrente2 (x, c, A) {
  const d = [x[0] - c[0], x[1] - c[1]]
  const Ad = matVec2(A, d)
  return [c[0] + Ad[0], c[1] + Ad[1]]
}

export function mapaVoltaConjugada2 (x, c, A) {
  const D = (z) => [2 * c[0] - z[0], 2 * c[1] - z[1]]
  const Ainv = matInv2(A)
  const FpInv = (y) => {
    const d = [y[0] - c[0], y[1] - c[1]]
    const Ad = matVec2(Ainv, d)
    return [c[0] + Ad[0], c[1] + Ad[1]]
  }
  return D(FpInv(D(x)))
}

/**
 * Jacobiano numérico 2×2 de F:ℝ²→ℝ² por diferenças centrais.
 * Mede DF_H DF_P ≈ I (experimento do Teorema rede-dual em dim 2).
 */
export function jacobiano2 (F, x, eps = 1e-6) {
  const J = [[0, 0], [0, 0]]
  for (let j = 0; j < 2; j++) {
    const xp = [x[0], x[1]]
    const xm = [x[0], x[1]]
    xp[j] += eps
    xm[j] -= eps
    const fp = F(xp)
    const fm = F(xm)
    J[0][j] = (fp[0] - fm[0]) / (2 * eps)
    J[1][j] = (fp[1] - fm[1]) / (2 * eps)
  }
  return J
}

export function medeConjugacao2 ({
  c = [0.1, -0.2],
  A = [[1.7, 0.3], [-0.2, 1.4]],
  pontos = null,
  eps = 1e-6,
  tol = 1e-5,
} = {}) {
  const xs = pontos || [
    [0.4, -0.5], [-0.6, 0.3], [0.2, 0.7], [-0.3, -0.4], [0.8, 0.1],
  ]
  let mauJac = 0
  let mauId = 0
  let mauLam = 0
  const I = matId2()
  for (const x of xs) {
    const Fp = (z) => mapaFrente2(z, c, A)
    const Fh = (z) => mapaVoltaConjugada2(z, c, A)
    const Jp = jacobiano2(Fp, x, eps)
    const Jh = jacobiano2(Fh, x, eps)
    const prod = matMul2(Jh, Jp)
    if (matSubFro2(prod, I) > tol) mauJac++
    const y = Fp(x)
    const back = Fh(y)
    if (Math.hypot(back[0] - x[0], back[1] - x[1]) > tol) mauId++
    const lamP = Math.log(Math.abs(matDet2(Jp)))
    const lamH = Math.log(Math.abs(matDet2(Jh)))
    if (Math.abs(lamP + lamH) > 1e-6) mauLam++
  }
  return { mauJac, mauId, mauLam, n: xs.length, ok: mauJac + mauId + mauLam === 0 }
}

/**
 * ℱ:(x,h)↦(x′,h′) — objecto do Teorema da Rede Dual.
 *   |u|≤Δ → retenção: (x,h)↦(x,h)
 *   u>Δ  → frente P:  x′=F_P(x), h′=+1
 *   u<−Δ → volta H:   x′=F_H(x) (conjugação), h′=−1
 * Maestro escolhe o ramo via batuta; Hopfield só fornece memória Y, não x′.
 */
export function aplicacaoHibrida (x, h, {
  c = 0,
  delta = DELTA_DEFAULT,
  alpha = 1.7,
  ramo = null, // 'frente'|'volta'|null (=pelo sinal de u)
} = {}) {
  const u = potencialEstaca(x, c)
  const hNovo = atualizaH(u, h, delta)
  if (Math.abs(u) <= delta) {
    return {
      x: x,
      h: hNovo,
      u,
      ramo: 'reter',
      I: BATUTA.NEUTRO,
    }
  }
  let lado = ramo
  if (!lado) lado = u > delta ? 'frente' : 'volta'
  if (lado === 'volta') {
    return {
      x: mapaVoltaConjugada(x, c, alpha),
      h: hNovo,
      u,
      ramo: 'volta',
      I: BATUTA.AVANCAR,
    }
  }
  return {
    x: mapaFrente(x, c, alpha),
    h: hNovo,
    u,
    ramo: 'frente',
    I: BATUTA.AVANCAR,
  }
}

function recallHopfield (estado, fala) {
  if (!estado.padroes.length) return null
  // match exacto: padrão já aprendido (volta sem depender do sinal de u)
  const exact = estado.padroes.find(p => p.fala === fala)
  if (exact) {
    return { Y: exact.Y, fonte: 'hopfield', fala: exact.fala, overlap: 1 }
  }
  const bits = featBits(fala)
  const W = hebbW(estado.padroes)
  const atr = hopfieldRecall(bits, W)
  let best = null
  let bestOv = -2
  for (const p of estado.padroes) {
    const ov = overlap(atr, p.bits)
    if (ov > bestOv) {
      bestOv = ov
      best = p
    }
  }
  if (best && bestOv > 0.35) {
    return { Y: best.Y, fonte: 'hopfield', fala: best.fala, overlap: bestOv }
  }
  return null
}

export function passoRedeDual (estado, fala) {
  const x = featScalar(fala)
  const u = potencialEstaca(x, estado.c)
  const hAnt = estado.h
  const banda = Math.abs(u) <= estado.delta

  let acao = 'frente'
  let I = BATUTA.AVANCAR
  let motivo = 'P: fora da banda — decisão/frente (estaca)'
  let recall = null
  let ramoF = 'frente'

  if (banda) {
    acao = 'reter'
    ramoF = 'reter'
    I = BATUTA.NEUTRO
    motivo = 'ℋ: |u|≤Δ — retenção (h′=h); ℱ:(x,h)↦(x,h)'
    if (estado.ultimoY && estado.ultimaFala) {
      recall = { Y: estado.ultimoY, fonte: 'retenção', fala: estado.ultimaFala }
    } else {
      recall = recallHopfield(estado, fala)
      if (recall) motivo = 'ℋ+H: banda com recall Hopfield (memória Y, não inversão)'
    }
  } else {
    recall = recallHopfield(estado, fala)
    if (recall) {
      acao = 'volta'
      ramoF = 'volta'
      I = BATUTA.AVANCAR
      motivo = 'H: memória Y (Hopfield) + x′ por conjugação F_H=D∘F_P⁻¹∘D⁻¹'
    } else {
      ramoF = 'frente'
      motivo = estado.padroes.length
        ? 'P: fora da banda, overlap baixo — frente ℱ_P'
        : 'P: fora da banda — decisão/frente (estaca)'
    }
  }

  // ℱ:(x,h)↦(x′,h′) — objecto do teorema (Hopfield não define x′)
  const Ffinal = aplicacaoHibrida(x, hAnt, {
    c: estado.c,
    delta: estado.delta,
    ramo: ramoF === 'reter' ? null : ramoF,
  })
  const h = Ffinal.h
  const xNovo = Ffinal.x

  const lambdaP = Math.log(Math.max(1e-6, Math.abs(u) + estado.delta))
  const lambdaH = recall && acao !== 'frente' ? -lambdaP : 0

  return {
    acao,
    I,
    u,
    x,
    xNovo,
    X: { x, h: hAnt },
    Xn: { x: xNovo, h },
    h,
    hAnt,
    banda,
    delta: estado.delta,
    c: estado.c,
    motivo,
    recall,
    lambdaP,
    lambdaH,
    lambdaSoma: lambdaP + lambdaH,
    F: Ffinal,
  }
}

/** Após resposta bem-sucedida: actualiza X=(x,h), padrões, último.
 *  Centra a estaca em c=feat(fala) — repetir a mesma fala cai na banda (retenção). */
export function aprendeRedeDual (estado, fala, Y, passo) {
  estado.h = passo.h
  estado.x = passo.xNovo != null ? passo.xNovo : passo.x
  estado.lambdaP = passo.lambdaP
  estado.lambdaH = passo.lambdaH
  if (!Y) return estado
  estado.ultimoY = String(Y)
  estado.ultimaFala = String(fala)
  // estaca acompanha a última fala aceite (Dual Sort: c é o centro)
  estado.c = featScalar(fala)
  if (passo.acao === 'frente' || passo.acao === 'volta') {
    const bits = featBits(fala)
    const ja = estado.padroes.findIndex(p => p.fala === fala)
    const entry = { bits, fala: String(fala), Y: String(Y) }
    if (ja >= 0) estado.padroes[ja] = entry
    else {
      estado.padroes.push(entry)
      if (estado.padroes.length > 24) estado.padroes.shift()
    }
  }
  return estado
}

export const REDE_DELTA = DELTA_DEFAULT
