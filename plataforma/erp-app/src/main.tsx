import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import './index.css'
import App from './App.tsx'
import { SnackbarProvider } from './components/Snackbar'
import { LangProvider } from './i18n/LangContext'
import { ThemeProvider } from './theme/ThemeContext'

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <LangProvider>
      <ThemeProvider>
        <SnackbarProvider>
          <App />
        </SnackbarProvider>
      </ThemeProvider>
    </LangProvider>
  </StrictMode>,
)
