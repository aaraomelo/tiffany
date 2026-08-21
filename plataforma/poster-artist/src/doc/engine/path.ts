/** Acessa `a.b.0.c` numa árvore de objetos/arrays. Portado do poster-artist. */
export function getNested(source: unknown, path: string): unknown {
  if (!path) return source
  const parts = path.split('.')
  let cursor: unknown = source
  for (const part of parts) {
    if (cursor == null) return undefined
    if (Array.isArray(cursor)) {
      const idx = Number(part)
      cursor = Number.isInteger(idx) ? cursor[idx] : undefined
    } else if (typeof cursor === 'object') {
      cursor = (cursor as Record<string, unknown>)[part]
    } else {
      return undefined
    }
  }
  return cursor
}
