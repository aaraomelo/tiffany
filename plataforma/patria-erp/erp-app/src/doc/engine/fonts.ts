// Catálogo de fontes — as MESMAS TTF são usadas no preview HTML (@font-face em
// index.css) e embarcadas no PDF (pdf-lib + fontkit). O cliente troca os TTF em
// public/fonts/ pela identidade visual dele sem rebuild.
//
// pdf-lib não lida bem com WOFF/WOFF2 (corrupção silenciosa do fontkit), por
// isso só TTF reais aqui.
export type FontWeight = 'normal' | 'bold'

export type FontFamilyEntry = {
  family: string
  category: 'display' | 'sans'
  regularUrl: string
  boldUrl: string
}

const T = (file: string) => `/fonts/${file}`

export const FONT_CATALOG: Record<string, FontFamilyEntry> = {
  Inter: {
    family: 'Inter',
    category: 'sans',
    regularUrl: T('Inter-Regular.ttf'),
    boldUrl: T('Inter-Regular.ttf'),
  },
  Roboto: {
    family: 'Roboto',
    category: 'sans',
    regularUrl: T('Roboto-Regular.ttf'),
    boldUrl: T('Roboto-Regular.ttf'),
  },
  Montserrat: {
    family: 'Montserrat',
    category: 'sans',
    regularUrl: T('Montserrat-Regular.ttf'),
    boldUrl: T('Montserrat-Regular.ttf'),
  },
  Oswald: {
    family: 'Oswald',
    category: 'display',
    regularUrl: T('Oswald-Regular.ttf'),
    boldUrl: T('Oswald-Regular.ttf'),
  },
  Anton: {
    family: 'Anton',
    category: 'display',
    regularUrl: T('Anton-Regular.ttf'),
    boldUrl: T('Anton-Regular.ttf'),
  },
  'Bebas Neue': {
    family: 'Bebas Neue',
    category: 'display',
    regularUrl: T('BebasNeue-Regular.ttf'),
    boldUrl: T('BebasNeue-Regular.ttf'),
  },
  'Archivo Black': {
    family: 'Archivo Black',
    category: 'display',
    regularUrl: T('ArchivoBlack-Regular.ttf'),
    boldUrl: T('ArchivoBlack-Regular.ttf'),
  },
}

export const FONT_FAMILIES = Object.keys(FONT_CATALOG)

// Documentos usam um sans neutro por padrão (o PDF modelo é Helvetica/Arial).
export const DEFAULT_FONT_FAMILY = 'Inter'

export function resolveFontEntry(family: string | undefined): FontFamilyEntry {
  if (family && FONT_CATALOG[family]) return FONT_CATALOG[family]
  return FONT_CATALOG[DEFAULT_FONT_FAMILY]
}

export function urlForFamily(family: string, weight: FontWeight = 'normal'): string {
  const entry = resolveFontEntry(family)
  return weight === 'bold' ? entry.boldUrl : entry.regularUrl
}
