// estacao.js — volta contra o mundo (fundamento ob:residuo).
// Colunas: projectado, medido, R. Fecha só com leitura da estação (não do réu).

export function residualEstacao (projectado, medido) {
  return Math.abs((Number(projectado) || 0) - (Number(medido) || 0))
}

export function estacaoFecha (projectado, medido, external) {
  return !!external && residualEstacao(projectado, medido) === 0
}

/** Documento canónico para o banco (ob:banco). Fala + resposta — não pesos. */
export function documentoImplante ({ proj, meas, external = 1, id = '' } = {}) {
  const R = residualEstacao(proj, meas)
  const fonte = external ? 'MES' : 'reu'
  const resposta = 'implante P=' + proj + ' M=' + meas + ' R=' + R + ' fonte=' + fonte
  const fala = (id ? 'implante ' + id : resposta)
  return { fala, resposta, R }
}

/** «mostra o banco» / «o que e a volta no banco» — RAG, não pesos. */
export function pedeBanco (fala) {
  const t = String(fala || '').toLowerCase()
  return /(mostra o banco|volta no banco|o que e a volta no banco)/.test(t)
}

export function parseImplante (src) {
  const m = /implante P=(\d+) M=(\d+) R=(-?\d+) fonte=(MES|reu)/.exec(String(src || ''))
  if (!m) return null
  const proj = +m[1], meas = +m[2], R = +m[3]
  const external = m[4] === 'MES' ? 1 : 0
  if (residualEstacao(proj, meas) !== R) return null
  return { proj, meas, R, external }
}

/**
 * Fala: «mostra a estação», «lead:65 estacao:65», «proj:65 medido:40».
 * Sem dados: demo estação 04 (65% vs 65% MES).
 */
export function passoEstacao (fala) {
  const t = String(fala || '').toLowerCase()
  const expl = /(?:proj(?:ectado)?|lead)\s*:\s*(\d+).{0,24}(?:estac[aã]o|medido|mes)\s*:\s*(\d+)/i.exec(fala)
    || /(?:estac[aã]o|medido)\s*:\s*(\d+).{0,24}(?:proj(?:ectado)?|lead)\s*:\s*(\d+)/i.exec(fala)

  if (expl) {
    const a = +expl[1], b = +expl[2]
    const proj = /proj|lead/i.test(expl[0].slice(0, 12)) ? a : b
    const meas = proj === a ? b : a
    const R = residualEstacao(proj, meas)
    const ok = estacaoFecha(proj, meas, 1)
    return {
      tipo: 'estacao',
      nome: 'estação',
      proj, meas, R, ok, external: 1,
      motivo: ok ? 'volta MES: projectado = medido' : '|P−M|=' + R,
    }
  }

  if (!/(esta[cç][aã]o|lead\s*time|projectado\s+contra|medido na)/i.test(t)
      || /banco/i.test(t)) {
    return null
  }

  /* demo: caso 04 — 65% projectado, lido na estação */
  const R = residualEstacao(65, 65)
  return {
    tipo: 'estacao',
    nome: 'estação 04',
    proj: 65, meas: 65, R, ok: true, external: 1,
    motivo: 'demo: 65% projectado = 65% medido (MES)',
  }
}
