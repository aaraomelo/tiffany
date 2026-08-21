import { getNested } from './path'
import { NAMED_FORMATTERS } from './format'

// Substitui `{path.x.y}` por valores do data, com formatador opcional via
// `{path | brl}` ou `{path | date}`. Portado/estendido do poster-artist.
const PLACEHOLDER = /\{([^}]+)\}/g

export function interpolate(template: string | undefined, data: unknown): string {
  if (template == null) return ''
  return template.replace(PLACEHOLDER, (_match, expr: string) => {
    const [rawPath, rawFn] = expr.split('|').map((s) => s.trim())
    const value = getNested(data, rawPath)
    if (rawFn && NAMED_FORMATTERS[rawFn]) {
      return NAMED_FORMATTERS[rawFn](value)
    }
    return value == null ? '' : String(value)
  })
}
