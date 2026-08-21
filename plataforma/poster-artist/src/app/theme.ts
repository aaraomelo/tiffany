import { createTheme } from '@mui/material/styles'

// Tema enxuto alinhado ao erp-app (MUI 9 + emotion). Mantém o app
// independente, mas com a mesma linguagem visual da plataforma.
export const theme = createTheme({
  palette: {
    mode: 'light',
    primary: { main: '#1f6feb' },
    background: { default: '#f4f5f7' },
  },
  shape: { borderRadius: 8 },
  typography: {
    fontFamily: 'Inter, Roboto, system-ui, sans-serif',
  },
})
