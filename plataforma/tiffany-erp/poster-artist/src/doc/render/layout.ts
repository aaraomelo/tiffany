import type { Font } from 'opentype.js'
import {
  INK,
  MUTED,
  RULE,
  SECTION_BAR_BG,
  TITLE_BAR_BG,
  TOTAL_ROW_BG,
  type RGB,
} from '../engine/color'
import type { FontWeight } from '../engine/fonts'
import { fontMetrics, loadFamilyFont, measureWidth, wrapAtSize } from '../engine/text'
import { pageDimensionsPt } from '../engine/units'
import type {
  Align,
  DocBlock,
  DocumentSpec,
  TableBlock,
  TableCellValue,
  TableColumn,
} from '../schema/document'
import type { DrawOp, LaidOutDocument, LaidOutPage, TextOp } from './types'

// ---------------------------------------------------------------------------
// Motor de layout em fluxo. Compõe cada bloco em DrawOps numa origem local,
// empilha verticalmente e pagina (com repetição de cabeçalho de tabela ao
// quebrar). A medição de texto usa o MESMO fitter/opentype do PDF → paridade.
// ---------------------------------------------------------------------------

const SIZE = {
  body: 9,
  label: 8,
  sectionText: 12,
  titleText: 15,
  tableHeader: 8,
  cellPrimary: 9,
  cellSecondary: 8,
  footer: 8,
}

const STACK_GAP = 3 // espaçamento ao empilhar blocos dentro de uma coluna
const SECTION_GAP = 4 // espaçamento entre blocos de topo

type FontGetter = (family: string, weight: FontWeight) => Font
type Ctx = { font: FontGetter; family: string }

type Composed = { height: number; ops: DrawOp[] }

type FlowItem = {
  height: number
  gapBefore: number
  render: (x: number, yTop: number) => DrawOp[]
  group?: string
  isHeader?: boolean
}

// --- utilidades ------------------------------------------------------------

function lineMetrics(font: Font, size: number) {
  const m = fontMetrics(font)
  return { lineHeight: m.naturalLineHeight * size, ascender: m.ascender }
}

function translateOps(ops: DrawOp[], dx: number, dy: number): DrawOp[] {
  return ops.map((op) => {
    switch (op.kind) {
      case 'text':
        return { ...op, x: op.x + dx, yTop: op.yTop + dy }
      case 'rect':
        return { ...op, x: op.x + dx, y: op.y + dy }
      case 'line':
        return { ...op, x1: op.x1 + dx, y1: op.y1 + dy, x2: op.x2 + dx, y2: op.y2 + dy }
      case 'image':
        return { ...op, x: op.x + dx, y: op.y + dy }
    }
  })
}

type TextOpts = {
  family: string
  weight: FontWeight
  size: number
  color: RGB
  maxWidth: number
  align?: Align
  x?: number
  yTop?: number
}

/** Quebra `text` e produz uma TextOp por linha (origem local). */
function layoutText(ctx: Ctx, text: string, o: TextOpts): Composed {
  const font = ctx.font(o.family, o.weight)
  const { lines, lineHeight, ascender } = wrapAtSize(font, text, o.size, o.maxWidth)
  const x0 = o.x ?? 0
  let y = o.yTop ?? 0
  const ops: TextOp[] = []
  for (const line of lines) {
    const w = measureWidth(font, line, o.size)
    let lx = x0
    if (o.align === 'center') lx = x0 + (o.maxWidth - w) / 2
    else if (o.align === 'right') lx = x0 + o.maxWidth - w
    ops.push({
      kind: 'text',
      x: lx,
      yTop: y,
      text: line,
      family: o.family,
      weight: o.weight,
      size: o.size,
      color: o.color,
      ascender,
      lineHeight,
    })
    y += lineHeight
  }
  return { ops, height: lines.length * lineHeight }
}

// --- composição por bloco (origem local 0,0) -------------------------------

function compose(block: DocBlock, width: number, ctx: Ctx): Composed {
  switch (block.kind) {
    case 'spacer':
      return { height: block.height, ops: [] }

    case 'sectionBar': {
      const isTitle = block.variant === 'title'
      const size = isTitle ? SIZE.titleText : SIZE.sectionText
      const vpad = isTitle ? 6 : 5
      const lpad = isTitle ? 10 : 8
      const font = ctx.font(ctx.family, 'bold')
      const lm = lineMetrics(font, size)
      const barH = lm.lineHeight + 2 * vpad
      const bg = isTitle ? TITLE_BAR_BG : SECTION_BAR_BG
      const text = layoutText(ctx, block.text, {
        family: ctx.family,
        weight: 'bold',
        size,
        color: INK,
        maxWidth: width - lpad * 2,
        align: 'left',
        x: lpad,
        yTop: (barH - lm.lineHeight) / 2,
      })
      return {
        height: barH,
        ops: [{ kind: 'rect', x: 0, y: 0, w: width, h: barH, color: bg }, ...text.ops],
      }
    }

    case 'paragraph': {
      return layoutText(ctx, block.text, {
        family: block.family ?? ctx.family,
        weight: block.weight ?? 'normal',
        size: block.size ?? SIZE.body,
        color: block.color ?? INK,
        maxWidth: width,
        align: block.align ?? 'left',
      })
    }

    case 'field': {
      const label = layoutText(ctx, block.label, {
        family: ctx.family,
        weight: 'bold',
        size: SIZE.label,
        color: MUTED,
        maxWidth: width,
      })
      const vs = block.valueStyle
      const value = layoutText(ctx, block.value, {
        family: vs?.family ?? ctx.family,
        weight: vs?.weight ?? 'normal',
        size: vs?.size ?? SIZE.body,
        color: vs?.color ?? INK,
        maxWidth: width,
        align: vs?.align,
        yTop: label.height + 2,
      })
      return { height: label.height + 2 + value.height, ops: [...label.ops, ...value.ops] }
    }

    case 'columns': {
      const gap = block.gap ?? 16
      const n = block.columns.length
      const totalFlex = block.columns.reduce((s, c) => s + (c.flex ?? 1), 0)
      const innerWidth = width - gap * (n - 1)
      let x = 0
      let maxH = 0
      const ops: DrawOp[] = []
      for (const col of block.columns) {
        const colW = (innerWidth * (col.flex ?? 1)) / totalFlex
        const stacked = composeStack(col.blocks, colW, ctx)
        ops.push(...translateOps(stacked.ops, x, 0))
        maxH = Math.max(maxH, stacked.height)
        x += colW + gap
      }
      return { height: maxH, ops }
    }

    case 'table':
      return composeTable(block, width, ctx)

    case 'summary': {
      const w = width * (block.widthFraction ?? 0.5)
      const xOff = width - w
      const pad = 8
      const rowH = 16
      const ops: DrawOp[] = []
      let y = 0
      for (const r of block.rows) {
        if (r.strong) ops.push({ kind: 'rect', x: xOff, y, w, h: rowH, color: TOTAL_ROW_BG })
        const weight: FontWeight = r.strong ? 'bold' : 'normal'
        const lm = lineMetrics(ctx.font(ctx.family, weight), SIZE.body)
        const yText = y + (rowH - lm.lineHeight) / 2
        ops.push(
          ...layoutText(ctx, r.label, {
            family: ctx.family,
            weight,
            size: SIZE.body,
            color: INK,
            maxWidth: w - pad * 2,
            x: xOff + pad,
            yTop: yText,
          }).ops,
        )
        ops.push(
          ...layoutText(ctx, r.value, {
            family: ctx.family,
            weight,
            size: SIZE.body,
            color: INK,
            maxWidth: w - pad * 2,
            align: 'right',
            x: xOff + pad,
            yTop: yText,
          }).ops,
        )
        y += rowH
      }
      return { height: y, ops }
    }

    case 'signatures': {
      const n = block.items.length
      const blockH = 42
      const lineY = 26
      const slotW = width / n
      const ops: DrawOp[] = []
      block.items.forEach((item, i) => {
        const slotX = i * slotW
        const inset = slotW * 0.12
        ops.push({
          kind: 'line',
          x1: slotX + inset,
          y1: lineY,
          x2: slotX + slotW - inset,
          y2: lineY,
          color: INK,
          width: 0.8,
        })
        ops.push(
          ...layoutText(ctx, item.label, {
            family: ctx.family,
            weight: 'bold',
            size: SIZE.body,
            color: INK,
            maxWidth: slotW,
            align: 'center',
            x: slotX,
            yTop: lineY + 5,
          }).ops,
        )
      })
      return { height: blockH, ops }
    }

    case 'image': {
      let x = 0
      if (block.align === 'center') x = (width - block.width) / 2
      else if (block.align === 'right') x = width - block.width
      return {
        height: block.height,
        ops: [{ kind: 'image', x, y: 0, w: block.width, h: block.height, src: block.src }],
      }
    }
  }
}

/** Empilha blocos atomicamente (usado dentro de colunas). */
function composeStack(blocks: DocBlock[], width: number, ctx: Ctx): Composed {
  let y = 0
  const ops: DrawOp[] = []
  blocks.forEach((b, i) => {
    if (i > 0 && b.kind !== 'spacer') y += STACK_GAP
    const c = compose(b, width, ctx)
    ops.push(...translateOps(c.ops, 0, y))
    y += c.height
  })
  return { height: y, ops }
}

// --- tabela ----------------------------------------------------------------

function columnXs(columns: TableColumn[], width: number): { xs: number[]; ws: number[] } {
  const totalFlex = columns.reduce((s, c) => s + c.flex, 0)
  const ws = columns.map((c) => (width * c.flex) / totalFlex)
  const xs: number[] = []
  let acc = 0
  for (const w of ws) {
    xs.push(acc)
    acc += w
  }
  return { xs, ws }
}

const CELL_PAD_X = 2
const CELL_PAD_Y = 4

function composeTableHeader(block: TableBlock, width: number, ctx: Ctx): Composed {
  const { xs, ws } = columnXs(block.columns, width)
  const font = ctx.font(ctx.family, 'normal')
  const lm = lineMetrics(font, SIZE.tableHeader)
  const headerH = lm.lineHeight + CELL_PAD_Y * 2
  const ops: DrawOp[] = []
  block.columns.forEach((col, i) => {
    ops.push(
      ...layoutText(ctx, col.header, {
        family: ctx.family,
        weight: 'normal',
        size: SIZE.tableHeader,
        color: MUTED,
        maxWidth: ws[i] - CELL_PAD_X * 2,
        align: col.align ?? 'left',
        x: xs[i] + CELL_PAD_X,
        yTop: CELL_PAD_Y,
      }).ops,
    )
  })
  ops.push({ kind: 'line', x1: 0, y1: headerH, x2: width, y2: headerH, color: RULE, width: 0.7 })
  return { height: headerH, ops }
}

function normalizeCell(cell: TableCellValue): {
  primary: string
  secondary?: string
  align?: Align
  weight?: FontWeight
} {
  if (typeof cell === 'string') return { primary: cell }
  return cell
}

function composeTableRow(
  block: TableBlock,
  row: TableCellValue[],
  width: number,
  ctx: Ctx,
): Composed {
  const { xs, ws } = columnXs(block.columns, width)
  const cellOps: DrawOp[][] = []
  let maxContentH = 0
  block.columns.forEach((col, i) => {
    const cell = normalizeCell(row[i] ?? '')
    const align = cell.align ?? col.align ?? 'left'
    const maxW = ws[i] - CELL_PAD_X * 2
    const x = xs[i] + CELL_PAD_X
    const primary = layoutText(ctx, cell.primary, {
      family: ctx.family,
      weight: cell.weight ?? 'normal',
      size: SIZE.cellPrimary,
      color: INK,
      maxWidth: maxW,
      align,
      x,
    })
    const ops = [...primary.ops]
    let h = primary.height
    if (cell.secondary) {
      const secondary = layoutText(ctx, cell.secondary, {
        family: ctx.family,
        weight: 'normal',
        size: SIZE.cellSecondary,
        color: MUTED,
        maxWidth: maxW,
        align,
        x,
        yTop: primary.height + 1,
      })
      ops.push(...secondary.ops)
      h += 1 + secondary.height
    }
    cellOps.push(ops)
    maxContentH = Math.max(maxContentH, h)
  })
  const rowH = maxContentH + CELL_PAD_Y * 2
  const ops = cellOps.flatMap((c) => translateOps(c, 0, CELL_PAD_Y))
  return { height: rowH, ops }
}

function composeTable(block: TableBlock, width: number, ctx: Ctx): Composed {
  const header = composeTableHeader(block, width, ctx)
  const ops: DrawOp[] = [...header.ops]
  let y = header.height
  for (const row of block.rows) {
    const r = composeTableRow(block, row, width, ctx)
    ops.push(...translateOps(r.ops, 0, y))
    y += r.height
  }
  return { height: y, ops }
}

// --- flatten + paginação ---------------------------------------------------

function flatten(blocks: DocBlock[], width: number, ctx: Ctx): FlowItem[] {
  const items: FlowItem[] = []
  let tableSeq = 0
  blocks.forEach((block, idx) => {
    const gapBefore = idx === 0 || block.kind === 'spacer' ? 0 : SECTION_GAP
    if (block.kind === 'table') {
      const group = `t${tableSeq++}`
      const header = composeTableHeader(block, width, ctx)
      items.push({
        height: header.height,
        gapBefore,
        group,
        isHeader: true,
        render: (x, y) => translateOps(header.ops, x, y),
      })
      for (const row of block.rows) {
        const r = composeTableRow(block, row, width, ctx)
        items.push({
          height: r.height,
          gapBefore: 0,
          group,
          render: (x, y) => translateOps(r.ops, x, y),
        })
      }
    } else {
      const c = compose(block, width, ctx)
      items.push({
        height: c.height,
        gapBefore,
        render: (x, y) => translateOps(c.ops, x, y),
      })
    }
  })
  return items
}

function collectFamilies(blocks: DocBlock[], acc: Set<string>) {
  for (const b of blocks) {
    if (b.kind === 'paragraph' && b.family) acc.add(b.family)
    if (b.kind === 'field' && b.valueStyle?.family) acc.add(b.valueStyle.family)
    if (b.kind === 'columns') b.columns.forEach((c) => collectFamilies(c.blocks, acc))
  }
}

export async function layoutDocument(spec: DocumentSpec): Promise<LaidOutDocument> {
  const [pageW, pageH] = pageDimensionsPt(spec.size, spec.orientation)

  // Pré-carrega as fontes (regular + bold) de todas as famílias usadas.
  const families = new Set<string>([spec.fontFamily])
  collectFamilies(spec.blocks, families)
  const loaded = new Map<string, Font>()
  await Promise.all(
    Array.from(families).flatMap((fam) =>
      (['normal', 'bold'] as FontWeight[]).map(async (w) => {
        loaded.set(`${fam}:${w}`, await loadFamilyFont(fam, w))
      }),
    ),
  )
  const font: FontGetter = (fam, w) =>
    loaded.get(`${fam}:${w}`) ?? loaded.get(`${spec.fontFamily}:${w}`)!
  const ctx: Ctx = { font, family: spec.fontFamily }

  const left = spec.margin.left
  const contentWidth = pageW - spec.margin.left - spec.margin.right
  const maxY = pageH - spec.margin.bottom

  const items = flatten(spec.blocks, contentWidth, ctx)

  const pages: LaidOutPage[] = []
  let cur: DrawOp[] = []
  let y = spec.margin.top
  const headers = new Map<string, FlowItem>()

  const newPage = () => {
    pages.push({ width: pageW, height: pageH, ops: cur })
    cur = []
    y = spec.margin.top
  }

  for (const item of items) {
    const atTop = y === spec.margin.top
    const gap = atTop ? 0 : item.gapBefore
    const fits = y + gap + item.height <= maxY
    if (!fits && !atTop) {
      newPage()
      // Repete o cabeçalho da tabela ao quebrar no meio dela.
      if (item.group && !item.isHeader && headers.has(item.group)) {
        const h = headers.get(item.group)!
        cur.push(...h.render(left, y))
        y += h.height
      }
    } else {
      y += gap
    }
    cur.push(...item.render(left, y))
    if (item.isHeader && item.group) headers.set(item.group, item)
    y += item.height
  }
  pages.push({ width: pageW, height: pageH, ops: cur })

  // Rodapé por página.
  if (spec.footer) {
    const total = pages.length
    pages.forEach((page, i) => {
      const text = spec.footer!.replace('{page}', String(i + 1)).replace('{pages}', String(total))
      const footer = layoutText(ctx, text, {
        family: spec.fontFamily,
        weight: 'normal',
        size: SIZE.footer,
        color: MUTED,
        maxWidth: contentWidth,
        align: 'right',
        x: left,
        yTop: pageH - spec.margin.bottom + 8,
      })
      page.ops.push(...footer.ops)
    })
  }

  return { pages, pageWidth: pageW, pageHeight: pageH }
}
