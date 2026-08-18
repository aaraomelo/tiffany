// maestro.js — Realização computacional do Teorema do Maestro (corpus/docs/computacional.tex §assistente).
//
//   (Π, X, tick, I) → Y     depois    (X, Y, Π) → r
//
// O motor (cristal TFAL / tradutor) REALIZA a projeção — não É o Maestro.
// Projectar ≠ medir. Metrónomo atesta a volta: a realização ainda pertence a Π?
// Massa fornece o corpo; partitura determina; Maestro projecta; Metrónomo atesta.

/** Batuta / trial: direcção do fluxo (Lei 3). */
export const BATUTA = Object.freeze({
  AVANCAR: 1,
  NEUTRO: 0,
  RETRAIR: -1,
})

/**
 * Corte da massa (max-cut operacional): meia volta nos itens.
 * Mesmo procedimento em 4 ou 4000 — isomorfo ao Maestro do computacional/max-cut.
 * suporte = A, borda = B; |A|≈|B| quando n par.
 */
export function corteMassa (itens) {
  const lista = (itens || []).map(String).filter(Boolean)
  const n = lista.length
  if (n === 0) return { suporte: [], borda: [] }
  const suporte = []
  const borda = []
  for (let i = 0; i < n; i++) {
    const r = (i + Math.floor(n / 2)) % n
    if (r < i) suporte.push(lista[i])
    else borda.push(lista[i])
  }
  // a fala actual (último item) entra sempre no suporte activo
  const ultimo = lista[n - 1]
  if (!suporte.includes(ultimo)) {
    suporte.push(ultimo)
    const j = borda.indexOf(ultimo)
    if (j >= 0) borda.splice(j, 1)
  }
  return { suporte, borda }
}

/** Partitura Π — especificação da realização (não inventa estrutura). */
export function partituraFala (objetivo) {
  return {
    objetivo: String(objetivo || ''),
    esquema: 'fala-tfal',
    autorizaTransformacao: false,
    rMax: 0,
  }
}

/**
 * Metrónomo: leitura do resíduo — atesta a volta.
 * Não é «entrada − saída» textual grosseira nem detector de verdade.
 * Pergunta: a realização ainda pertence à projeção determinada por Π neste tick?
 */
export function medirResiduo (X, Y, pi, meta = {}) {
  const y = (Y == null ? '' : String(Y)).trim()
  const suporte = (X && X.suporte) || []
  const op = meta.op

  // falta de suporte: cristal não sabe — massa insuficiente
  if (meta.naoSei || op === 'nao_sei') {
    return {
      r: 1,
      ok: false,
      suporteFaltante: true,
      motivo: 'massa insuficiente — aumentar X (ensinar / ferramenta) ou retrair',
    }
  }
  if (meta.erro) {
    return {
      r: 1,
      ok: false,
      suporteFaltante: false,
      motivo: meta.erro,
    }
  }
  if (!y) {
    return {
      r: 1,
      ok: false,
      suporteFaltante: false,
      motivo: 'saída vazia — não pertence a Π',
    }
  }

  // adesão mínima a Π: esquema fala-tfal exige texto não-vazio após FALA
  if (pi && pi.esquema === 'fala-tfal' && op !== 'resposta' && op !== undefined) {
    // op opcional quando motor só devolve texto
  }

  // invenção grosseira: se Π não autoriza transformação e Y não toca no suporte
  // (conservação estrita) — palavras do suporte vs Y
  if (pi && !pi.autorizaTransformacao && suporte.length) {
    const blob = suporte.join(' ').toLowerCase()
    const yLow = y.toLowerCase()
    // resposta do cristal sobre a banda: se cita .tex, ok; senão basta ter vindo do motor
    // com op resposta — a volta fechou no canal
    if (meta.doCristal) {
      return { r: 0, ok: true, suporteFaltante: false, motivo: 'volta fechou no cristal (TFAL)' }
    }
    // motor local sem cristal: exige eco mínimo do objectivo/suporte
    const obj = (pi.objetivo || '').toLowerCase()
    const toca = (obj && yLow.includes(obj.slice(0, Math.min(12, obj.length)))) ||
      suporte.some(s => s.length > 2 && yLow.includes(String(s).toLowerCase().slice(0, 16)))
    if (!toca && blob.length > 0) {
      return {
        r: 1,
        ok: false,
        suporteFaltante: false,
        motivo: 'realização fora do suporte de Π — batuta pode retrair',
      }
    }
  }

  const rMax = (pi && pi.rMax != null) ? pi.rMax : 0
  return {
    r: 0,
    ok: true,
    suporteFaltante: false,
    motivo: rMax === 0 ? 'r=0 — atestação da volta' : 'dentro de rMax',
  }
}

/**
 * Um ciclo do Maestro: P_k = tick ∘ batuta ∘ Π.
 * projectar(X, pi) — motor externo (antena TFAL, etc.); não é o Maestro.
 */
export async function maestroProject ({ pi, X, tick, I, projectar }) {
  const batuta = I === BATUTA.RETRAIR || I === BATUTA.NEUTRO || I === BATUTA.AVANCAR
    ? I
    : BATUTA.AVANCAR

  if (batuta === BATUTA.NEUTRO) {
    return {
      Y: '',
      residuo: { r: 0, ok: true, suporteFaltante: false, motivo: 'neutro — sem projeção' },
      tick,
      batuta,
      X,
      pi,
    }
  }

  if (batuta === BATUTA.RETRAIR) {
    return {
      Y: '',
      residuo: {
        r: 0,
        ok: true,
        suporteFaltante: false,
        motivo: 'retração — sem emitir; ajustar Π ou X',
      },
      tick,
      batuta,
      X,
      pi,
      retraido: true,
    }
  }

  // I = +1: projectar
  if (typeof projectar !== 'function') {
    throw new Error('maestroProject: falta motor projectar(X, pi)')
  }
  const out = await projectar(X, pi)
  const Y = out && out.Y != null ? String(out.Y) : ''
  const residuo = medirResiduo(X, Y, pi, out || {})

  return {
    Y,
    residuo,
    tick: (tick | 0) + 1,
    batuta,
    X,
    pi,
    meta: out,
  }
}

/** Monta massa bruta a partir da fala e do histórico curto. */
export function massaDeTurno (fala, historico = []) {
  const itens = []
  for (const h of historico.slice(-6)) {
    if (h) itens.push(String(h))
  }
  itens.push(String(fala || ''))
  return corteMassa(itens)
}
