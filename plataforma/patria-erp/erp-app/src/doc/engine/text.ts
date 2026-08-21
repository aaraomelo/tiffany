import opentype, { type Font } from 'opentype.js'
import { resolveFontEntry, urlForFamily, type FontWeight } from './fonts'

// ---------------------------------------------------------------------------
// Medição de fonte compartilhada (opentype.js). É a JOIA da paridade: tanto o
// preview HTML quanto o PDF medem texto com a MESMA fonte e o MESMO retângulo,
// então quebram linhas e dimensionam igual. Portado do easysync-poster-artist.
// ---------------------------------------------------------------------------

const fontCache = new Map<string, Promise<Font>>()

export function loadOpentypeFont(url: string): Promise<Font> {
  let pending = fontCache.get(url)
  if (!pending) {
    pending = fetch(url)
      .then((res) => {
        if (!res.ok) throw new Error(`Failed to fetch ${url}: ${res.status}`)
        return res.arrayBuffer()
      })
      .then((buf) => opentype.parse(buf))
    fontCache.set(url, pending)
  }
  return pending
}

export function loadFamilyFont(family: string, weight: FontWeight): Promise<Font> {
  return loadOpentypeFont(urlForFamily(resolveFontEntry(family).family, weight))
}

const MIN_PT = 1
const MAX_PT = 800

export type FitOptions = {
  allowWrap?: boolean
  /** Sobrescreve a razão natural de entrelinha (senão (asc-desc)/upem). */
  lineHeight?: number
  /** Limita o resultado a este tamanho (não amplia além dele). */
  maxFontSize?: number
}

export type FitResult = {
  fontSize: number
  lines: string[]
  /** Razão de entrelinha efetiva usada no fit. */
  lineHeight: number
  /** Razão do ascender (em unidades de fontSize) acima da baseline. */
  ascender: number
  /** Razão do descender (em unidades de fontSize) abaixo da baseline (positivo). */
  descender: number
}

export function fontMetrics(font: Font): {
  ascender: number
  descender: number
  naturalLineHeight: number
} {
  const upem = font.unitsPerEm
  const ascender = font.ascender / upem
  const descender = -font.descender / upem
  return { ascender, descender, naturalLineHeight: ascender + descender }
}

export function measureWidth(font: Font, text: string, fontSize: number): number {
  return font.getAdvanceWidth(text, fontSize, { kerning: true })
}

/** Word-wrap guloso. Palavras isoladas maiores que maxWidth vão como estão. */
export function breakIntoLines(
  font: Font,
  text: string,
  fontSize: number,
  maxWidth: number,
): string[] {
  // Respeita quebras explícitas (\n) primeiro, depois quebra cada parágrafo.
  const paragraphs = text.split('\n')
  const out: string[] = []
  for (const para of paragraphs) {
    if (para === '') {
      out.push('')
      continue
    }
    const tokens = para.split(/(\s+)/).filter((t) => t.length > 0)
    let current = ''
    for (const token of tokens) {
      const isSpace = /^\s+$/.test(token)
      const candidate = current + token
      if (measureWidth(font, candidate, fontSize) <= maxWidth) {
        current = candidate
        continue
      }
      if (isSpace) {
        if (current.trim()) {
          out.push(current.trim())
          current = ''
        }
        continue
      }
      if (current.trim()) out.push(current.trim())
      current = token
    }
    if (current.trim()) out.push(current.trim())
  }
  return out.length > 0 ? out : [text]
}

/**
 * Maior tamanho de fonte que faz `text` caber em `boxWidth × boxHeight`.
 * Com `allowWrap`, o texto é quebrado; altura total = linhas × fontSize × lineHeight.
 * Considera ascender + descender — essencial pra glyphs com descender (g, p, ,)
 * não vazarem do retângulo no PDF.
 */
export function fitFontSize(
  font: Font,
  text: string,
  boxWidth: number,
  boxHeight: number,
  opts: FitOptions = {},
): FitResult {
  const allowWrap = opts.allowWrap ?? true
  const metrics = fontMetrics(font)
  const lineHeight = opts.lineHeight ?? metrics.naturalLineHeight

  if (!text || boxWidth <= 0 || boxHeight <= 0) {
    return {
      fontSize: MIN_PT,
      lines: [text],
      lineHeight,
      ascender: metrics.ascender,
      descender: metrics.descender,
    }
  }

  let lo = MIN_PT
  let hi = Math.min(opts.maxFontSize ?? MAX_PT, Math.floor(boxHeight / lineHeight))
  let best = lo
  let bestLines: string[] = [text]

  while (lo <= hi) {
    const mid = (lo + hi) >> 1
    const lines = allowWrap ? breakIntoLines(font, text, mid, boxWidth) : [text]
    const maxLineWidth = lines.reduce(
      (m, line) => Math.max(m, measureWidth(font, line, mid)),
      0,
    )
    const totalHeight = lines.length * mid * lineHeight
    if (maxLineWidth <= boxWidth && totalHeight <= boxHeight) {
      best = mid
      bestLines = lines
      lo = mid + 1
    } else {
      hi = mid - 1
    }
  }

  return {
    fontSize: best,
    lines: bestLines,
    lineHeight,
    ascender: metrics.ascender,
    descender: metrics.descender,
  }
}

/**
 * Quebra `text` num tamanho de fonte FIXO (não dimensiona). Retorna as linhas e
 * a altura total ocupada. Usado pelos documentos em fluxo, onde o tamanho da
 * fonte é dado e o que varia é a altura (que empilha).
 */
export function wrapAtSize(
  font: Font,
  text: string,
  fontSize: number,
  maxWidth: number,
  lineHeightRatio?: number,
): { lines: string[]; lineHeight: number; height: number; ascender: number } {
  const metrics = fontMetrics(font)
  const lh = (lineHeightRatio ?? metrics.naturalLineHeight) * fontSize
  const lines = breakIntoLines(font, text, fontSize, maxWidth)
  return {
    lines,
    lineHeight: lh,
    height: lines.length * lh,
    ascender: metrics.ascender,
  }
}
