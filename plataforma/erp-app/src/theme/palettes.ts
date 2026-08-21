export interface ThemeConfig {
  presetId?: string

  // cores
  primary: string
  primaryHover: string
  primaryLight: string
  secondary: string
  accent: string
  bg: string
  surface: string
  surfaceAlt: string
  border: string
  text: string
  textMuted: string
  success: string
  warn: string
  danger: string
  textOnPrimary: string

  // geometria
  radius: number       // border-radius pequeno (px)
  radiusLg: number     // border-radius grande (px)
  borderWidth: number  // px
  spacingBase: number  // rem*100 (50 = 0.5rem)
}

export interface Preset {
  id: string
  labelKey: string
  config: ThemeConfig
}

const baseGeo = { radius: 6, radiusLg: 8, borderWidth: 1, spacingBase: 50 }

export const PRESETS: Preset[] = [
  {
    id: 'patria-blue',
    labelKey: 'theme.palette.patria-blue',
    config: {
      ...baseGeo,
      presetId: 'patria-blue',
      primary: '#1F4E79', primaryHover: '#173a5b', primaryLight: '#e8eff7',
      secondary: '#0277bd', accent: '#ff9800',
      bg: '#fafbfc', surface: '#ffffff', surfaceAlt: '#f6f8fa', border: '#e5e9ee',
      text: '#222222', textMuted: '#666666',
      success: '#2E7D32', warn: '#B45309', danger: '#c0392b',
      textOnPrimary: '#ffffff',
    },
  },
  {
    id: 'nco-red',
    labelKey: 'theme.palette.nco-red',
    config: {
      ...baseGeo,
      presetId: 'nco-red',
      primary: '#cc1144', primaryHover: '#a30e36', primaryLight: '#fde8ee',
      secondary: '#161b27', accent: '#f59e0b',
      bg: '#0c1019', surface: '#161b27', surfaceAlt: '#1d2330', border: '#2a3142',
      text: '#e6e8f0', textMuted: '#8a91a8',
      success: '#22c55e', warn: '#f59e0b', danger: '#ef4444',
      textOnPrimary: '#ffffff',
    },
  },
  {
    id: 'forest-green',
    labelKey: 'theme.palette.forest-green',
    config: {
      ...baseGeo,
      presetId: 'forest-green',
      primary: '#2E7D32', primaryHover: '#1f5d23', primaryLight: '#e8f5e9',
      secondary: '#558b2f', accent: '#ff8f00',
      bg: '#f7faf7', surface: '#ffffff', surfaceAlt: '#eef5ef', border: '#d8e6da',
      text: '#222222', textMuted: '#5f6d61',
      success: '#2E7D32', warn: '#B45309', danger: '#c0392b',
      textOnPrimary: '#ffffff',
    },
  },
  {
    id: 'sunset-orange',
    labelKey: 'theme.palette.sunset-orange',
    config: {
      ...baseGeo,
      presetId: 'sunset-orange',
      primary: '#d97706', primaryHover: '#a85d05', primaryLight: '#fef3e2',
      secondary: '#92400e', accent: '#dc2626',
      bg: '#fffaf3', surface: '#ffffff', surfaceAlt: '#fdf6ec', border: '#e9dfd0',
      text: '#222222', textMuted: '#7a6b56',
      success: '#2E7D32', warn: '#B45309', danger: '#c0392b',
      textOnPrimary: '#ffffff',
    },
  },
  {
    id: 'graphite',
    labelKey: 'theme.palette.graphite',
    config: {
      ...baseGeo,
      presetId: 'graphite',
      primary: '#37474f', primaryHover: '#263238', primaryLight: '#eceff1',
      secondary: '#546e7a', accent: '#26a69a',
      bg: '#f4f5f7', surface: '#ffffff', surfaceAlt: '#eef0f3', border: '#d7dbe0',
      text: '#222222', textMuted: '#5b6066',
      success: '#2E7D32', warn: '#B45309', danger: '#c0392b',
      textOnPrimary: '#ffffff',
    },
  },
]

export const DEFAULT_PRESET_ID = 'patria-blue'

export function getPreset(id: string | undefined): Preset {
  return PRESETS.find((p) => p.id === id) ?? PRESETS[0]
}

export function defaultConfig(): ThemeConfig {
  return { ...PRESETS[0].config }
}

export type Mode = 'light' | 'dark'

// Neutros (fundo/superfície/texto) genéricos por modo. As cores de marca
// (primary/secondary/accent/feedback) vêm sempre do preset/config.
type Neutrals = Pick<
  ThemeConfig,
  'bg' | 'surface' | 'surfaceAlt' | 'border' | 'text' | 'textMuted'
>

const LIGHT_NEUTRALS: Neutrals = {
  bg: '#fafbfc', surface: '#ffffff', surfaceAlt: '#f6f8fa',
  border: '#e5e9ee', text: '#222222', textMuted: '#666666',
}
const DARK_NEUTRALS: Neutrals = {
  bg: '#0e1117', surface: '#161b22', surfaceAlt: '#1c2230',
  border: '#2a3140', text: '#e6e8ee', textMuted: '#9aa3b2',
}

// Luminância do fundo → modo "natural" do preset.
export function isDarkColor(hex: string): boolean {
  const m = hex.replace('#', '')
  if (m.length < 6) return false
  const r = parseInt(m.slice(0, 2), 16)
  const g = parseInt(m.slice(2, 4), 16)
  const b = parseInt(m.slice(4, 6), 16)
  return (0.299 * r + 0.587 * g + 0.114 * b) / 255 < 0.5
}

export function resolveMode(c: ThemeConfig, mode?: Mode): Mode {
  return mode ?? (isDarkColor(c.bg) ? 'dark' : 'light')
}

/**
 * Cores efetivas: se o modo pedido bate com o modo natural do preset, usa as
 * cores do próprio config; senão troca só os neutros pelo conjunto do modo
 * (marca preservada). Assim o toggle escurece/clareia qualquer tema.
 */
export function effectiveColors(c: ThemeConfig, mode?: Mode): ThemeConfig {
  const want = resolveMode(c, mode)
  const natural: Mode = isDarkColor(c.bg) ? 'dark' : 'light'
  if (want === natural) return c
  return { ...c, ...(want === 'dark' ? DARK_NEUTRALS : LIGHT_NEUTRALS) }
}

/// Mapeia ThemeConfig → CSS variables (já no modo efetivo).
export function configToCssVars(config: ThemeConfig, mode?: Mode): Record<string, string> {
  const c = effectiveColors(config, mode)
  return {
    '--primary': c.primary,
    '--primary-hover': c.primaryHover,
    '--primary-light': c.primaryLight,
    '--secondary': c.secondary,
    '--accent': c.accent,
    '--bg': c.bg,
    '--surface': c.surface,
    '--surface-alt': c.surfaceAlt,
    '--border': c.border,
    '--text': c.text,
    '--text-muted': c.textMuted,
    '--success': c.success,
    '--warn': c.warn,
    '--danger': c.danger,
    '--text-on-primary': c.textOnPrimary,
    '--text-on-primary-muted': 'rgba(255,255,255,0.7)',
    '--border-on-primary': 'rgba(255,255,255,0.3)',
    '--hover-on-primary': 'rgba(255,255,255,0.12)',
    '--radius': `${c.radius}px`,
    '--radius-lg': `${c.radiusLg}px`,
    '--border-width': `${c.borderWidth}px`,
    '--spacing-base': `${c.spacingBase / 100}rem`,
  }
}
