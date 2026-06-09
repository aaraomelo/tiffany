import type { RGB } from '../engine/color'
import type { FontWeight } from '../engine/fonts'
import type { Orientation, PageSize } from '../engine/units'

// ---------------------------------------------------------------------------
// Modelo declarativo de DOCUMENTO em fluxo.
//
// Diferente do poster (layout absoluto de página única), um documento é uma
// sequência de BLOCOS que empilham verticalmente e quebram em páginas A4. É a
// extensão necessária pra Ordem de Serviço: header, barras de seção, tabelas
// com N linhas, subtotais, assinaturas.
// ---------------------------------------------------------------------------

export type Align = 'left' | 'center' | 'right'

export type TextStyle = {
  size?: number
  weight?: FontWeight
  color?: RGB
  family?: string
  align?: Align
}

/** Espaço vertical fixo (pt). */
export type SpacerBlock = { kind: 'spacer'; height: number }

/** Barra cinza de seção, largura total. `title` = barra do cabeçalho (maior). */
export type SectionBarBlock = {
  kind: 'sectionBar'
  text: string
  variant?: 'title' | 'section'
}

/** Parágrafo de texto que quebra dentro da largura disponível. */
export type ParagraphBlock = { kind: 'paragraph'; text: string } & TextStyle

/** Rótulo pequeno (cinza, bold) com o valor logo abaixo. */
export type FieldBlock = {
  kind: 'field'
  label: string
  value: string
  valueStyle?: TextStyle
}

/** Colunas lado a lado; altura = maior coluna. */
export type ColumnsBlock = {
  kind: 'columns'
  columns: Array<{ flex?: number; blocks: DocBlock[] }>
  gap?: number
}

export type TableColumn = {
  header: string
  /** Peso relativo da largura da coluna. */
  flex: number
  align?: Align
}

export type TableCellValue =
  | string
  | { primary: string; secondary?: string; align?: Align; weight?: FontWeight }

export type TableBlock = {
  kind: 'table'
  columns: TableColumn[]
  rows: TableCellValue[][]
}

/** Bloco de subtotais alinhado à direita (Serviços / Peças / Total). */
export type SummaryBlock = {
  kind: 'summary'
  rows: Array<{ label: string; value: string; strong?: boolean }>
  /** Largura do bloco como fração da largura de conteúdo (default 0.5). */
  widthFraction?: number
}

/** Linhas de assinatura lado a lado (linha + rótulo centralizado). */
export type SignaturesBlock = {
  kind: 'signatures'
  items: Array<{ label: string }>
}

/** Imagem (logo). src = data URL ou URL. */
export type ImageBlock = {
  kind: 'image'
  src: string
  width: number
  height: number
  align?: Align
}

export type DocBlock =
  | SpacerBlock
  | SectionBarBlock
  | ParagraphBlock
  | FieldBlock
  | ColumnsBlock
  | TableBlock
  | SummaryBlock
  | SignaturesBlock
  | ImageBlock

export type PageMargin = { top: number; right: number; bottom: number; left: number }

export type DocumentSpec = {
  size: PageSize
  orientation: Orientation
  margin: PageMargin
  fontFamily: string
  /** Texto do rodapé; `{page}` e `{pages}` são substituídos. Ex: "Página {page}/{pages}". */
  footer?: string
  blocks: DocBlock[]
}
