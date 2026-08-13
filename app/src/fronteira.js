// fronteira.js — resíduo ENTRE claims na UI (Lei 7: ligar sem fundir).
// Espelha conecthus/core/fronteira.c · tests/claim_fronteira.c

/** Inversões GUT↔5W2H: score[acao[i]] < score[acao[i+1]]. */
export function fronteiraGut5w2h (score, acao) {
  const n = Math.min(score?.length || 0, acao?.length || 0)
  if (n < 2) return 0
  let inv = 0
  for (let i = 0; i < n - 1; i++) {
    const a = acao[i], b = acao[i + 1]
    if (a < 0 || a >= score.length || b < 0 || b >= score.length) { inv++; continue }
    if (score[a] < score[b]) inv++
  }
  return inv
}

/** Why↔Ishikawa: 0 se root ∈ causes. */
export function fronteiraWhyIshikawa (root, causes) {
  if (!causes?.length) return 1
  return causes.includes(root) ? 0 : 1
}

/** PDCA↔VSM: |plan − futuro|. */
export function fronteiraPdcaVsm (plan, futuro) {
  return Math.abs((Number(plan) || 0) - (Number(futuro) || 0))
}

export const FRONT_NOME = Object.freeze({
  gut: 'GUT↔5W2H',
  why: 'Why↔Ishikawa',
  pdca: 'PDCA↔VSM',
})

/**
 * Detecta pedido de fronteira na fala e corre um par de demonstração
 * (ou dados embutidos tipo gut:60,48 ordem:0,1).
 */
export function passoFronteira (fala) {
  const t = String(fala || '').toLowerCase()
  if (!/(fronteira|gut\s*e\s*5w2h|5w2h.*gut|compara\s+gut|why.*ishikawa|ishikawa.*raiz|pdca.*vsm|vsm.*pdca)/i.test(t)
      && !/\bgut\s*:/.test(t) && !/\bpdca\s*:/.test(t)) {
    return null
  }

  // dados explícitos: gut:60,48,30,12 ordem:0,1,2,3
  const mg = /gut\s*:\s*([\d,\s]+)/i.exec(fala)
  const mo = /ordem\s*:\s*([\d,\s]+)/i.exec(fala)
  if (mg) {
    const score = mg[1].split(/[,\s]+/).filter(Boolean).map(Number)
    const acao = mo
      ? mo[1].split(/[,\s]+/).filter(Boolean).map(Number)
      : score.map((_, i) => i)
    const R = fronteiraGut5w2h(score, acao)
    return { tipo: 'gut', nome: FRONT_NOME.gut, R, ok: R === 0, motivo: R === 0 ? 'ranking e ordem concordam' : R + ' inversão(ões)' }
  }

  const mp = /pdca\s*:\s*(\d+)\s*(?:vsm|futuro)\s*:\s*(\d+)/i.exec(fala)
  if (mp) {
    const R = fronteiraPdcaVsm(+mp[1], +mp[2])
    return { tipo: 'pdca', nome: FRONT_NOME.pdca, R, ok: R === 0, motivo: R === 0 ? 'alvo = futuro' : '|alvo−futuro|=' + R }
  }

  if (/why|ishikawa|raiz/.test(t) && /ishikawa|raiz|why|fronteira/.test(t)) {
    const R = fronteiraWhyIshikawa(7, [3, 7, 1, 9])
    return { tipo: 'why', nome: FRONT_NOME.why, R, ok: R === 0, motivo: 'demo: raiz 7 no mapa' }
  }

  if (/pdca|vsm/.test(t)) {
    const R = fronteiraPdcaVsm(42, 42)
    return { tipo: 'pdca', nome: FRONT_NOME.pdca, R, ok: R === 0, motivo: 'demo: alvo = futuro = 42' }
  }

  // default GUT demo consistente
  const R = fronteiraGut5w2h([60, 48, 30, 12], [0, 1, 2, 3])
  return { tipo: 'gut', nome: FRONT_NOME.gut, R, ok: R === 0, motivo: 'demo: ranking consistente' }
}
