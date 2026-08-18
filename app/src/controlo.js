// controlo.js — Controlo de Histerese acima da IR (UI).
// papers/corpo_topologico.tex thm:controle-histerese; conecthus/core/control.c
// Eixos 𝒱: L1, |R|, |n|, mut, caixa, teclado, fonético, forma
// Dados reais: ABNT2/QWERTY + mapa φ PT (espelha eixos_texto.c)

export const CTRL = Object.freeze({
  RETAIN: 0,
  MOVE: 1,
  RETRACT: 2,
})

export const CTRL_NOME = Object.freeze(['RETAIN', 'MOVE', 'RETRACT'])

/** Θ default — 8 eixos. */
export const THETA_DEFAULT = Object.freeze([5, 0, 0, 1, 2, 6, 2, 5])

const TECLAS = [
  'qwertyuiop',
  'asdfghjklç',
  'zxcvbnm',
]

function baseLat (ch) {
  const map = {
    á: 'a', à: 'a', â: 'a', ã: 'a', ä: 'a',
    é: 'e', è: 'e', ê: 'e', ë: 'e',
    í: 'i', ì: 'i', î: 'i', ï: 'i',
    ó: 'o', ò: 'o', ô: 'o', õ: 'o', ö: 'o',
    ú: 'u', ù: 'u', û: 'u', ü: 'u',
    ç: 'c', ñ: 'n',
  }
  const c = ch.toLowerCase()
  if (map[c]) return { b: map[c], acento: 1 }
  if (c >= 'a' && c <= 'z') return { b: c, acento: 0 }
  return { b: c, acento: 0 }
}

function normAscii (s) {
  let out = '', nac = 0
  for (const ch of String(s || '')) {
    const { b, acento } = baseLat(ch)
    if (acento) nac++
    if (b.length === 1 && b.charCodeAt(0) < 128) out += b
  }
  return { s: out, nac }
}

function lev (a, b) {
  const na = a.length, nb = b.length
  if (!na) return nb
  if (!nb) return na
  let prev = Array.from({ length: nb + 1 }, (_, j) => j)
  for (let i = 1; i <= na; i++) {
    const cur = [i]
    for (let j = 1; j <= nb; j++) {
      const c = a[i - 1] === b[j - 1] ? 0 : 1
      cur[j] = Math.min(prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + c)
    }
    prev = cur
  }
  return prev[nb]
}

export function eixosCaixa (a, b) {
  const A = normAscii(a), B = normAscii(b)
  return lev(A.s, B.s) + Math.abs(A.nac - B.nac)
}

function teclaXY (ch) {
  const raw = String(ch).toLowerCase()
  if (raw === 'ç') return { r: 1, c: 9 }
  for (let r = 0; r < TECLAS.length; r++) {
    const c = TECLAS[r].indexOf(raw)
    if (c >= 0) return { r, c }
  }
  const { b } = baseLat(raw)
  for (let r = 0; r < TECLAS.length; r++) {
    const c = TECLAS[r].indexOf(b)
    if (c >= 0) return { r, c }
  }
  return null
}

function distTecla (ca, cb) {
  const pa = teclaXY(ca), pb = teclaXY(cb)
  if (!pa || !pb) return 4
  return Math.abs(pa.r - pb.r) + Math.abs(pa.c - pb.c)
}

export function eixosTeclado (a, b) {
  const A = [...String(a || '')].filter((c) => c.trim())
  const B = [...String(b || '')].filter((c) => c.trim())
  const na = A.length, nb = B.length
  if (!na) return nb ? nb * 4 : 0
  if (!nb) return na * 4
  let prev = Array.from({ length: nb + 1 }, (_, j) => j * 4)
  for (let i = 1; i <= na; i++) {
    const cur = [i * 4]
    for (let j = 1; j <= nb; j++) {
      const sub = prev[j - 1] + distTecla(A[i - 1], B[j - 1])
      cur[j] = Math.min(sub, prev[j] + 4, cur[j - 1] + 4)
    }
    prev = cur
  }
  return prev[nb]
}

function paraFonemas (s) {
  const { s: t } = normAscii(s)
  let out = '', i = 0
  while (i < t.length) {
    const c = t[i], n1 = t[i + 1] || ''
    if (c === 'l' && n1 === 'h') { out += 'L'; i += 2; continue }
    if (c === 'n' && n1 === 'h') { out += 'N'; i += 2; continue }
    if (c === 'c' && n1 === 'h') { out += 'S'; i += 2; continue }
    if (c === 'r' && n1 === 'r') { out += 'R'; i += 2; continue }
    if (c === 's' && n1 === 's') { out += 's'; i += 2; continue }
    if (c === 'q' && n1 === 'u') { out += 'k'; i += 2; continue }
    if (c === 'g' && n1 === 'u' && (t[i + 2] === 'e' || t[i + 2] === 'i')) {
      out += 'g'; i += 2; continue
    }
    if (c === 'c' && (n1 === 'e' || n1 === 'i')) { out += 's'; i++; continue }
    if (c === 'g' && (n1 === 'e' || n1 === 'i')) { out += 'Z'; i++; continue }
    if (c === 'c') { out += 'k'; i++; continue }
    if (c === 'x') { out += 'S'; i++; continue }
    if (c === 'w') { out += 'v'; i++; continue }
    if (c === 'y') { out += 'i'; i++; continue }
    out += c; i++
  }
  return out
}

export function eixosFonetico (a, b) {
  return lev(paraFonemas(a), paraFonemas(b))
}

export function eixosForma (a, b) {
  return lev(normAscii(a).s, normAscii(b).s)
}

/**
 * Distância 8 eixos. Se a.texto/b.texto existirem, usa caixa/teclado/φ/forma reais.
 * Caso contrário: L1 de x escalar (legado UI).
 */
export function distControlo (a, b) {
  const ta = a?.texto, tb = b?.texto
  if (ta != null && tb != null && String(ta).length && String(tb).length) {
    const xa = Number(a?.x) || 0
    const xb = Number(b?.x) || 0
    const l1 = Math.abs(xa - xb) * 100
    const dR = Math.abs((a?.residual ?? 0) - (b?.residual ?? 0))
    const dn = Math.abs((a?.n ?? 1) - (b?.n ?? 1))
    const dm = Math.abs((a?.mut ?? 0) - (b?.mut ?? 0))
    return [
      l1, dR, dn, dm,
      eixosCaixa(ta, tb),
      eixosTeclado(ta, tb),
      eixosFonetico(ta, tb),
      eixosForma(ta, tb),
    ]
  }
  const xa = Number(a?.x) || 0
  const xb = Number(b?.x) || 0
  const l1 = Math.abs(xa - xb) * 100
  const dR = Math.abs((a?.residual ?? 0) - (b?.residual ?? 0))
  const dn = Math.abs((a?.n ?? 1) - (b?.n ?? 1))
  const dm = Math.abs((a?.mut ?? 0) - (b?.mut ?? 0))
  return [l1, dR, dn, dm, l1, 0, 0, l1]
}

export function admissivel (D, theta = THETA_DEFAULT) {
  for (let i = 0; i < Math.min(D.length, theta.length); i++) {
    if (D[i] > theta[i]) return false
  }
  return true
}

export function decideControlo ({ r = 0, D = [0, 0, 0, 0, 0, 0, 0, 0], theta = THETA_DEFAULT, exigeFecho = true } = {}) {
  if (exigeFecho && r !== 0) {
    return { act: CTRL.RETRACT, nome: 'RETRACT', D, motivo: 'R≠0 — REOPEN / retração' }
  }
  if (admissivel(D, theta)) {
    return { act: CTRL.RETAIN, nome: 'RETAIN', D, motivo: 'R=0 ∧ D⪯Θ — estado preservado' }
  }
  return { act: CTRL.MOVE, nome: 'MOVE', D, motivo: 'R=0 ∧ D≰Θ — fora da borda / muda andar' }
}

export function passoControlo (retido, candidato, residual) {
  const r = residual?.ok ? 0 : (residual?.r ?? 1)
  const D = distControlo(retido || { x: 0 }, candidato || { x: 0 })
  D[1] = Math.abs(r)
  return decideControlo({ r, D })
}

export function cenarioAcaso ({ nSorteios = 40, empatesDegen = 0, discordaL1 = 0 } = {}) {
  const degen = empatesDegen >= nSorteios
  return {
    degen,
    motivo: degen
      ? 'acaso empata — métrica degenerada (cega à escolha)'
      : 'L1 discorda do acaso — métrica informativa',
    nSorteios,
    empatesDegen,
    discordaL1,
  }
}
