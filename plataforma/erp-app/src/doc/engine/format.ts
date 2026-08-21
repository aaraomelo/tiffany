// Formatadores ptBR usados por documentos. Centralizados pra que template e
// interpolador compartilhem a mesma saída.

export function brl(value: number | string | null | undefined): string {
  const n = typeof value === 'string' ? Number(value) : (value ?? 0)
  if (!Number.isFinite(n)) return 'R$ 0,00'
  return n.toLocaleString('pt-BR', {
    style: 'currency',
    currency: 'BRL',
    minimumFractionDigits: 2,
    maximumFractionDigits: 2,
  })
}

export function decimal(value: number | string | null | undefined, places = 0): string {
  const n = typeof value === 'string' ? Number(value) : (value ?? 0)
  if (!Number.isFinite(n)) return '0'
  return n.toLocaleString('pt-BR', {
    minimumFractionDigits: 0,
    maximumFractionDigits: places,
  })
}

/** Aceita Date, ISO string ou timestamp → dd/MM/yyyy. */
export function dateBR(value: string | number | Date | null | undefined): string {
  if (value == null || value === '') return ''
  // Data pura (yyyy-mm-dd): parseia como local pra não escorregar 1 dia por
  // causa do fuso (new Date('2026-06-08') é UTC meia-noite → 07/06 no Brasil).
  if (typeof value === 'string') {
    const m = /^(\d{4})-(\d{2})-(\d{2})$/.exec(value)
    if (m) return `${m[3]}/${m[2]}/${m[1]}`
  }
  const d = value instanceof Date ? value : new Date(value)
  if (Number.isNaN(d.getTime())) return String(value)
  const dd = String(d.getDate()).padStart(2, '0')
  const mm = String(d.getMonth() + 1).padStart(2, '0')
  return `${dd}/${mm}/${d.getFullYear()}`
}

export const NAMED_FORMATTERS: Record<string, (...args: unknown[]) => string> = {
  brl: (v) => brl(v as number),
  date: (v) => dateBR(v as string),
  upper: (v) => String(v ?? '').toUpperCase(),
  lower: (v) => String(v ?? '').toLowerCase(),
}
