import CssBaseline from '@mui/material/CssBaseline'
import { ThemeProvider as MuiThemeProvider } from '@mui/material/styles'
import { createContext, useCallback, useContext, useEffect, useMemo, useState, type ReactNode } from 'react'
import { api, getToken } from '../api'
import { buildMuiTheme } from './muiTheme'
import { configToCssVars, defaultConfig, getPreset, isDarkColor, PRESETS, type Mode, type Preset, type ThemeConfig } from './palettes'

const STORAGE_KEY = 'erp_theme'
const MODE_KEY = 'erp_mode'

function loadFromStorage(): ThemeConfig {
  if (typeof window === 'undefined') return defaultConfig()
  try {
    const raw = window.localStorage.getItem(STORAGE_KEY)
    if (raw) return JSON.parse(raw) as ThemeConfig
  } catch { /* noop */ }
  return defaultConfig()
}

function loadMode(): Mode {
  if (typeof window === 'undefined') return 'light'
  const saved = window.localStorage?.getItem(MODE_KEY)
  if (saved === 'light' || saved === 'dark') return saved
  // primeira visita: segue o modo natural do tema salvo
  return isDarkColor(loadFromStorage().bg) ? 'dark' : 'light'
}

function applyVars(config: ThemeConfig, mode: Mode) {
  const root = document.documentElement
  const vars = configToCssVars(config, mode)
  for (const [k, v] of Object.entries(vars)) {
    root.style.setProperty(k, v)
  }
}

interface ThemeCtx {
  config: ThemeConfig
  mode: Mode
  toggleMode: () => void
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
  mode: 'light',
  toggleMode: () => {},
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
  const [mode, setMode] = useState<Mode>(() => loadMode())

  useEffect(() => {
    applyVars(config, mode)
    try {
      window.localStorage?.setItem(STORAGE_KEY, JSON.stringify(config))
      window.localStorage?.setItem(MODE_KEY, mode)
    } catch { /* noop */ }
  }, [config, mode])

  const toggleMode = useCallback(() => {
    setMode((m) => (m === 'dark' ? 'light' : 'dark'))
  }, [])

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
    // ao escolher um preset, segue o modo natural dele (claro/escuro)
    setMode(isDarkColor(preset.config.bg) ? 'dark' : 'light')
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
      mode,
      toggleMode,
      applyPreset,
      updateConfig,
      saveToServer,
      resetServer,
      reloadFromServer,
      presets: PRESETS,
      saving,
      dirty,
    }),
    [config, mode, toggleMode, applyPreset, updateConfig, saveToServer, resetServer, reloadFromServer, saving, dirty],
  )

  // Ponte com o MUI: recria o tema quando config/modo mudam (cssVariables ligado).
  const muiTheme = useMemo(() => buildMuiTheme(config, mode), [config, mode])

  return (
    <ThemeContext.Provider value={value}>
      <MuiThemeProvider theme={muiTheme}>
        <CssBaseline />
        {children}
      </MuiThemeProvider>
    </ThemeContext.Provider>
  )
}

export function useTheme() {
  return useContext(ThemeContext)
}
