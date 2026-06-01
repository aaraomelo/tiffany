import { ThemeProvider as MuiThemeProvider } from '@mui/material/styles'
import { createContext, useCallback, useContext, useEffect, useMemo, useState, type ReactNode } from 'react'
import { api, getToken } from '../api'
import { buildMuiTheme } from './muiTheme'
import { configToCssVars, defaultConfig, getPreset, PRESETS, type Preset, type ThemeConfig } from './palettes'

const STORAGE_KEY = 'erp_theme'

function loadFromStorage(): ThemeConfig {
  if (typeof window === 'undefined') return defaultConfig()
  try {
    const raw = window.localStorage.getItem(STORAGE_KEY)
    if (raw) return JSON.parse(raw) as ThemeConfig
  } catch { /* noop */ }
  return defaultConfig()
}

function applyVars(config: ThemeConfig) {
  const root = document.documentElement
  const vars = configToCssVars(config)
  for (const [k, v] of Object.entries(vars)) {
    root.style.setProperty(k, v)
  }
}

interface ThemeCtx {
  config: ThemeConfig
  applyPreset: (id: string) => void
  updateConfig: (patch: Partial<ThemeConfig>) => void
  saveToServer: () => Promise<void>
  resetServer: () => Promise<void>
  reloadFromServer: () => Promise<void>
  presets: Preset[]
  saving: boolean
  dirty: boolean
}

const ThemeContext = createContext<ThemeCtx>({
  config: defaultConfig(),
  applyPreset: () => {},
  updateConfig: () => {},
  saveToServer: async () => {},
  resetServer: async () => {},
  reloadFromServer: async () => {},
  presets: PRESETS,
  saving: false,
  dirty: false,
})

export function ThemeProvider({ children }: { children: ReactNode }) {
  const [config, setConfig] = useState<ThemeConfig>(() => loadFromStorage())
  const [savedSnapshot, setSavedSnapshot] = useState<ThemeConfig>(() => loadFromStorage())
  const [saving, setSaving] = useState(false)

  useEffect(() => {
    applyVars(config)
    try {
      window.localStorage?.setItem(STORAGE_KEY, JSON.stringify(config))
    } catch { /* noop */ }
  }, [config])

  const reloadFromServer = useCallback(async () => {
    if (!getToken()) return
    try {
      const res = await api<{ config: ThemeConfig | null }>('/api/theme')
      if (res.config) {
        const next = { ...defaultConfig(), ...res.config }
        setConfig(next)
        setSavedSnapshot(next)
      }
    } catch { /* ignora: usa localStorage */ }
  }, [])

  useEffect(() => {
    void reloadFromServer()
  }, [reloadFromServer])

  const applyPreset = useCallback((id: string) => {
    const preset = getPreset(id)
    setConfig({ ...preset.config })
  }, [])

  const updateConfig = useCallback((patch: Partial<ThemeConfig>) => {
    setConfig((c) => ({ ...c, ...patch, presetId: 'custom' }))
  }, [])

  const saveToServer = useCallback(async () => {
    setSaving(true)
    try {
      const res = await api<{ config: ThemeConfig }>('/api/theme', {
        method: 'PUT',
        body: JSON.stringify(config),
      })
      setSavedSnapshot(res.config ?? config)
    } finally {
      setSaving(false)
    }
  }, [config])

  const resetServer = useCallback(async () => {
    setSaving(true)
    try {
      await api('/api/theme', { method: 'DELETE' })
      const def = defaultConfig()
      setConfig(def)
      setSavedSnapshot(def)
    } finally {
      setSaving(false)
    }
  }, [])

  const dirty = useMemo(
    () => JSON.stringify(config) !== JSON.stringify(savedSnapshot),
    [config, savedSnapshot],
  )

  const value = useMemo<ThemeCtx>(
    () => ({
      config,
      applyPreset,
      updateConfig,
      saveToServer,
      resetServer,
      reloadFromServer,
      presets: PRESETS,
      saving,
      dirty,
    }),
    [config, applyPreset, updateConfig, saveToServer, resetServer, reloadFromServer, saving, dirty],
  )

  // Ponte com o MUI: recria o tema quando o config muda (cssVariables ligado).
  const muiTheme = useMemo(() => buildMuiTheme(config), [config])

  return (
    <ThemeContext.Provider value={value}>
      <MuiThemeProvider theme={muiTheme}>{children}</MuiThemeProvider>
    </ThemeContext.Provider>
  )
}

export function useTheme() {
  return useContext(ThemeContext)
}
