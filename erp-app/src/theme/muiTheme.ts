import { createTheme, type Theme } from '@mui/material/styles'
import type { ThemeConfig } from './palettes'

// Luminância simples pra decidir modo claro/escuro a partir do fundo.
function isDark(hex: string): boolean {
  const m = hex.replace('#', '')
  if (m.length < 6) return false
  const r = parseInt(m.slice(0, 2), 16)
  const g = parseInt(m.slice(2, 4), 16)
  const b = parseInt(m.slice(4, 6), 16)
  const lum = (0.299 * r + 0.587 * g + 0.114 * b) / 255
  return lum < 0.5
}

/**
 * Constrói o tema do MUI a partir do nosso ThemeConfig (customizável pelo
 * tenant em runtime). `cssVariables` gera as CSS vars do MUI — o tema continua
 * mudando ao vivo ao recriar este objeto quando o config muda.
 */
export function buildMuiTheme(c: ThemeConfig): Theme {
  return createTheme({
    cssVariables: true,
    palette: {
      mode: isDark(c.bg) ? 'dark' : 'light',
      primary: {
        main: c.primary,
        dark: c.primaryHover,
        light: c.primaryLight,
        contrastText: c.textOnPrimary,
      },
      secondary: { main: c.secondary },
      error: { main: c.danger },
      warning: { main: c.warn },
      success: { main: c.success },
      background: { default: c.bg, paper: c.surface },
      text: { primary: c.text, secondary: c.textMuted },
      divider: c.border,
    },
    shape: { borderRadius: c.radius },
    typography: {
      fontFamily: "system-ui, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif",
    },
    components: {
      MuiButton: {
        defaultProps: { disableElevation: true },
        styleOverrides: { root: { textTransform: 'none', fontWeight: 600 } },
      },
    },
  })
}
