// patria.js — portão da publicação (local contra o ar).
// Fecha só com fetch real: 0∧0 não fecha; réu (sem GET) não fecha.

export const PATRIA_PIPELINE = '/corpo/conecthus/pipeline.tex'

export function residualPatria (local, live) {
  return (local && live) ? 0 : 1
}

export function patriaFecha (local, live, external) {
  return !!external && !!local && !!live
}

export function pedePatria (fala) {
  const t = String(fala || '').toLowerCase()
  return /(mostra o deploy|mostra a p[aá]tria|no ar\??|o que e o deploy)/.test(t)
}

/**
 * Fala: «mostra o deploy», «mostra a pátria», «no ar».
 * Sem medição live: demo local=1 live=0 → REOPEN (ainda não no ar).
 */
export function passoPatria (fala, { local = 1, live = 0, external = 1 } = {}) {
  if (!pedePatria(fala)) return null
  const R = residualPatria(local, live)
  const ok = patriaFecha(local, live, external)
  return {
    tipo: 'patria',
    nome: 'Pátria',
    local: local ? 1 : 0,
    live: live ? 1 : 0,
    R, ok, external: external ? 1 : 0,
    motivo: ok
      ? 'pipeline no ar (corpo + fetch)'
      : 'local=' + (local ? 1 : 0) + ' live=' + (live ? 1 : 0),
  }
}

export async function medePatriaLive (fetchFn = fetch) {
  try {
    const r = await fetchFn(PATRIA_PIPELINE)
    return r && r.ok ? 1 : 0
  } catch {
    return 0
  }
}
