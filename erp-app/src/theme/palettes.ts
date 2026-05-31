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

/// Mapeia ThemeConfig → CSS variables.
export function configToCssVars(c: ThemeConfig): Record<string, string> {
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
