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
      <div style={containerStyle} aria-live="polite" role="status">
        {snacks.map((s) => (
          <button
            key={s.id}
            type="button"
            onClick={() => dismiss(s.id)}
            style={snackStyle(s.kind)}
            title="Fechar"
          >
            <span aria-hidden style={{ fontSize: 15, lineHeight: 1 }}>{ICON[s.kind]}</span>
            <span style={{ flex: 1, textAlign: 'left' }}>{s.message}</span>
          </button>
        ))}
      </div>
    </SnackbarContext.Provider>
  )
}

export function useSnackbar() {
  return useContext(SnackbarContext)
}

const ICON: Record<SnackKind, string> = {
  success: '✓',
  error: '!',
  info: 'i',
}

const ACCENT: Record<SnackKind, string> = {
  success: 'var(--success, #2e7d32)',
  error: 'var(--danger, #cc1144)',
  info: 'var(--primary, #1F4E79)',
}

const containerStyle: React.CSSProperties = {
  position: 'fixed',
  bottom: '1.25rem',
  right: '1.25rem',
  display: 'flex',
  flexDirection: 'column',
  gap: '0.5rem',
  zIndex: 9999,
  maxWidth: 'min(92vw, 420px)',
}

function snackStyle(kind: SnackKind): React.CSSProperties {
  return {
    display: 'flex',
    alignItems: 'center',
    gap: '0.6rem',
    background: 'var(--surface, #fff)',
    color: 'var(--text, #222)',
    border: '1px solid var(--border, #ddd)',
    borderLeft: `4px solid ${ACCENT[kind]}`,
    borderRadius: 8,
    padding: '0.7rem 0.9rem',
    fontSize: 14,
    boxShadow: '0 6px 24px rgba(0,0,0,0.16)',
    cursor: 'pointer',
    textAlign: 'left',
    fontFamily: 'inherit',
    animation: 'snackIn 160ms ease-out',
  }
}
