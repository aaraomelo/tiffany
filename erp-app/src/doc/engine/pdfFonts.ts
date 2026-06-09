import { PDFDocument, StandardFonts, type PDFFont } from 'pdf-lib'
import fontkit from '@pdf-lib/fontkit'
import { resolveFontEntry, type FontWeight } from './fonts'

// Embarca as TTF no PDF via @pdf-lib/fontkit. Portado do poster-artist.
// Atenção: usar @pdf-lib/fontkit (não o fontkit puro — bug com algumas TTFs) e
// NÃO passar subset:true (bug do fontkit). TTF apenas, nunca WOFF2.
export type FontKit = {
  regular: PDFFont
  bold: PDFFont
}

const bufferCache = new Map<string, ArrayBuffer>()

async function loadFont(url: string): Promise<ArrayBuffer | null> {
  const cached = bufferCache.get(url)
  if (cached) return cached
  try {
    const res = await fetch(url)
    if (!res.ok) return null
    const buf = await res.arrayBuffer()
    bufferCache.set(url, buf)
    return buf
  } catch (err) {
    console.error('[loadFont] failed for', url, err)
    return null
  }
}

export async function embedFamily(doc: PDFDocument, family: string): Promise<FontKit> {
  doc.registerFontkit(fontkit)
  const entry = resolveFontEntry(family)

  const [regularBuf, boldBuf] = await Promise.all([
    loadFont(entry.regularUrl),
    loadFont(entry.boldUrl),
  ])

  const regular = regularBuf
    ? await doc.embedFont(regularBuf)
    : await doc.embedFont(StandardFonts.Helvetica)
  const bold = boldBuf
    ? await doc.embedFont(boldBuf)
    : await doc.embedFont(StandardFonts.HelveticaBold)

  return { regular, bold }
}

export function pickPdfFont(kit: FontKit, weight: FontWeight): PDFFont {
  return weight === 'bold' ? kit.bold : kit.regular
}
