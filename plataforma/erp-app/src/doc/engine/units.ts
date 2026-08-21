export type PageSize = 'A3' | 'A4' | 'A5' | 'A6'
export type Orientation = 'portrait' | 'landscape'

export const MM_TO_PT = 72 / 25.4

export const PAGE_SIZE_MM: Record<PageSize, readonly [number, number]> = {
  A3: [297, 420],
  A4: [210, 297],
  A5: [148, 210],
  A6: [105, 148],
}

/** Dimensões da página em pontos PDF (1pt = 1/72"). */
export function pageDimensionsPt(
  size: PageSize,
  orientation: Orientation,
): [number, number] {
  const [shortSide, longSide] = PAGE_SIZE_MM[size]
  const w = orientation === 'portrait' ? shortSide : longSide
  const h = orientation === 'portrait' ? longSide : shortSide
  return [w * MM_TO_PT, h * MM_TO_PT]
}

export function mm(v: number): number {
  return v * MM_TO_PT
}
