import { Alert, Box } from '@mui/material'
import {
  createContext,
  useCallback,
  useContext,
  useMemo,
  useRef,
  useState,
  type ReactNode,
} from 'react'

export type SnackKind = 'success' | 'error' | 'info'

interface Snack {
  id: number
  message: string
  kind: SnackKind
}

interface SnackbarCtx {
  notify: (message: string, kind?: SnackKind) => void
  success: (message: string) => void
  error: (message: string) => void
  info: (message: string) => void
}

const SnackbarContext = createContext<SnackbarCtx>({
  notify: () => {},
  success: () => {},
  error: () => {},
  info: () => {},
})

const DURATION = 4000

export function SnackbarProvider({ children }: { children: ReactNode }) {
  const [snacks, setSnacks] = useState<Snack[]>([])
  const seq = useRef(0)

  const dismiss = useCallback((id: number) => {
    setSnacks((prev) => prev.filter((s) => s.id !== id))
  }, [])

  const notify = useCallback(
    (message: string, kind: SnackKind = 'info') => {
      if (!message) return
      const id = ++seq.current
      setSnacks((prev) => [...prev, { id, message, kind }])
      setTimeout(() => dismiss(id), DURATION)
    },
    [dismiss],
  )

  const value = useMemo<SnackbarCtx>(
    () => ({
      notify,
      success: (m: string) => notify(m, 'success'),
      error: (m: string) => notify(m, 'error'),
      info: (m: string) => notify(m, 'info'),
    }),
    [notify],
  )

  return (
    <SnackbarContext.Provider value={value}>
      {children}
      <Box
        aria-live="polite"
        sx={{
          position: 'fixed',
          bottom: 20,
          right: 20,
          display: 'flex',
          flexDirection: 'column',
          gap: 1,
          zIndex: (theme) => theme.zIndex.snackbar,
          maxWidth: 'min(92vw, 420px)',
        }}
      >
        {snacks.map((s) => (
          <Alert
            key={s.id}
            severity={s.kind}
            variant="filled"
            onClose={() => dismiss(s.id)}
            sx={{ boxShadow: 6, animation: 'snackIn 160ms ease-out' }}
          >
            {s.message}
          </Alert>
        ))}
      </Box>
    </SnackbarContext.Provider>
  )
}

export function useSnackbar() {
  return useContext(SnackbarContext)
}
