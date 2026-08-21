import type { RGB } from '../engine/color'
import type { FontWeight } from '../engine/fonts'

// Primitivas de desenho posicionadas (coords em pt, origem no topo-esquerdo da
// página, y cresce pra baixo). Os DOIS renderers (PDF e HTML) consomem ISTO —
// a quebra de linha e o dimensionamento já foram resolvidos no layout, então
// preview e PDF ficam idênticos.

export type TextOp = {
  kind: 'text'
  /** Topo da caixa-em da linha. */
  x: number
  yTop: number
  text: string
  family: string
  weight: FontWeight
  size: number
  color: RGB
  /** Razão do ascender (baseline = yTop + size*ascender). */
  ascender: number
  /** Entrelinha absoluta em pt (= altura da caixa da linha). */
  lineHeight: number
}

export type RectOp = {
  kind: 'rect'
  x: number
  y: number
  w: number
  h: number
  color: RGB
}

export type LineOp = {
  kind: 'line'
  x1: number
  y1: number
  x2: number
  y2: number
  color: RGB
  width: number
}

export type ImageOp = {
  kind: 'image'
  x: number
  y: number
  w: number
  h: number
  src: string
}

export type DrawOp = TextOp | RectOp | LineOp | ImageOp

export type LaidOutPage = {
  width: number
  height: number
  ops: DrawOp[]
}

export type LaidOutDocument = {
  pages: LaidOutPage[]
  pageWidth: number
  pageHeight: number
}
