// RGB como tupla 0..255 (mesma representação do poster-artist, fácil de
// serializar em config declarativa e de converter pra pdf-lib/CSS).
export type RGB = [number, number, number]

export function cssColor(c: RGB): string {
  return `rgb(${c[0]}, ${c[1]}, ${c[2]})`
}

/** Normaliza pra 0..1 (formato que o pdf-lib `rgb()` espera). */
export function pdfColor(c: RGB): [number, number, number] {
  return [c[0] / 255, c[1] / 255, c[2] / 255]
}

// Paleta padrão do documento (calibrada pelo PDF modelo da Ordem de Serviço).
export const INK: RGB = [33, 33, 33] // texto principal
export const MUTED: RGB = [120, 120, 120] // labels / cabeçalho de tabela
export const TITLE_BAR_BG: RGB = [201, 201, 201] // barra do título (cinza médio)
export const SECTION_BAR_BG: RGB = [233, 233, 233] // barras de seção (cinza claro)
export const TOTAL_ROW_BG: RGB = [239, 239, 239] // linha de total
export const RULE: RGB = [204, 204, 204] // linhas/divisórias
export const WHITE: RGB = [255, 255, 255]
