import { PDFDocument, rgb } from 'pdf-lib'
import { pdfColor } from '../engine/color'
import { embedFamily, pickPdfFont, type FontKit } from '../engine/pdfFonts'
import type { LaidOutDocument } from './types'

// Remove glyphs que a fonte embarcada não codifica (emoji etc.) pra não
// estourar o pdf-lib. (O PDF modelo tinha 🌵 que virava caixinha.)
function sanitize(text: string): string {
  return text.replace(/[\u{1F000}-\u{1FFFF}\u{2600}-\u{27BF}\u{FE0F}]/gu, '')
}

async function embedImage(doc: PDFDocument, src: string) {
  try {
    let bytes: Uint8Array
    let isJpg = false
    if (src.startsWith('data:')) {
      const [meta, b64] = src.split(',')
      isJpg = meta.includes('jpeg') || meta.includes('jpg')
      const bin = atob(b64)
      bytes = Uint8Array.from(bin, (c) => c.charCodeAt(0))
    } else {
      const res = await fetch(src)
      const ct = res.headers.get('content-type') ?? ''
      isJpg = ct.includes('jpeg') || src.endsWith('.jpg') || src.endsWith('.jpeg')
      bytes = new Uint8Array(await res.arrayBuffer())
    }
    return isJpg ? await doc.embedJpg(bytes) : await doc.embedPng(bytes)
  } catch {
    return null
  }
}

export async function renderDocumentPdf(laid: LaidOutDocument): Promise<Uint8Array> {
  const doc = await PDFDocument.create()

  const families = new Set<string>()
  for (const page of laid.pages)
    for (const op of page.ops) if (op.kind === 'text') families.add(op.family)

  const kits = new Map<string, FontKit>()
  for (const fam of families) kits.set(fam, await embedFamily(doc, fam))
  const anyKit = kits.values().next().value ?? (await embedFamily(doc, 'Inter'))

  for (const page of laid.pages) {
    const p = doc.addPage([page.width, page.height])
    const H = page.height

    for (const op of page.ops) {
      switch (op.kind) {
        case 'rect': {
          const [r, g, b] = pdfColor(op.color)
          p.drawRectangle({
            x: op.x,
            y: H - (op.y + op.h),
            width: op.w,
            height: op.h,
            color: rgb(r, g, b),
          })
          break
        }
        case 'line': {
          const [r, g, b] = pdfColor(op.color)
          p.drawLine({
            start: { x: op.x1, y: H - op.y1 },
            end: { x: op.x2, y: H - op.y2 },
            thickness: op.width,
            color: rgb(r, g, b),
          })
          break
        }
        case 'text': {
          const clean = sanitize(op.text)
          if (!clean) break
          const kit = kits.get(op.family) ?? anyKit
          const font = pickPdfFont(kit, op.weight)
          const [r, g, b] = pdfColor(op.color)
          const baseline = H - (op.yTop + op.size * op.ascender)
          try {
            p.drawText(clean, {
              x: op.x,
              y: baseline,
              size: op.size,
              font,
              color: rgb(r, g, b),
            })
          } catch {
            // glyph não codificável — ignora silenciosamente
          }
          break
        }
        case 'image': {
          const img = await embedImage(doc, op.src)
          if (img) {
            p.drawImage(img, { x: op.x, y: H - (op.y + op.h), width: op.w, height: op.h })
          }
          break
        }
      }
    }
  }

  return doc.save()
}
