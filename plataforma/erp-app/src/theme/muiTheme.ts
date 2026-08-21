import { createTheme, type Theme } from '@mui/material/styles'
import { effectiveColors, resolveMode, type Mode, type ThemeConfig } from './palettes'

/**
 * Constrói o tema do MUI a partir do nosso ThemeConfig (customizável pelo
 * tenant em runtime) + o modo claro/escuro escolhido pelo usuário.
 * `cssVariables` gera as CSS vars do MUI — o tema continua mudando ao vivo ao
 * recriar este objeto quando config/mode mudam.
 */
export function buildMuiTheme(config: ThemeConfig, mode?: Mode): Theme {
  const c = effectiveColors(config, mode)
  return createTheme({
    cssVariables: true,
    palette: {
      mode: resolveMode(config, mode),
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
