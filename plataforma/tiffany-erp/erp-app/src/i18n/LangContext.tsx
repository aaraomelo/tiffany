import { createContext, useContext, useEffect, useMemo, useState, type ReactNode } from 'react'
import en from './strings/en.json'
import es from './strings/es.json'
import pt from './strings/pt.json'

export type Lang = 'pt' | 'en' | 'es'
type Dict = Record<string, string>

const STORAGE_KEY = 'erp_lang'
const BUNDLES: Record<Lang, Dict> = { pt, en, es }
const FALLBACK_LANG: Lang = 'pt'

function detectInitialLang(): Lang {
  if (typeof window === 'undefined') return FALLBACK_LANG
  const saved = window.localStorage?.getItem(STORAGE_KEY) as Lang | null
  if (saved && (saved === 'pt' || saved === 'en' || saved === 'es')) return saved
  const nav = (navigator?.language || 'pt').toLowerCase()
  if (nav.startsWith('en')) return 'en'
  if (nav.startsWith('es')) return 'es'
  return 'pt'
}

interface LangCtx {
  lang: Lang
  setLang: (l: Lang) => void
  t: (key: string, vars?: Record<string, string | number>) => string
}

const LangContext = createContext<LangCtx>({
  lang: FALLBACK_LANG,
  setLang: () => {},
  t: (key: string) => key,
})

export function LangProvider({ children }: { children: ReactNode }) {
  const [lang, setLangState] = useState<Lang>(() => detectInitialLang())

  useEffect(() => {
    try {
      window.localStorage?.setItem(STORAGE_KEY, lang)
      document.documentElement.lang = lang
    } catch { /* noop */ }
  }, [lang])

  const value = useMemo<LangCtx>(() => {
    const dict = BUNDLES[lang] ?? {}
    const fallback = BUNDLES[FALLBACK_LANG] ?? {}
    const t = (key: string, vars?: Record<string, string | number>): string => {
      const raw = dict[key] ?? fallback[key] ?? key
      if (!vars) return raw
      return raw.replace(/\{(\w+)\}/g, (_, k) =>
        vars[k] !== undefined ? String(vars[k]) : `{${k}}`,
      )
    }
    return { lang, setLang: setLangState, t }
  }, [lang])

  return <LangContext.Provider value={value}>{children}</LangContext.Provider>
}

export function useT() {
  return useContext(LangContext).t
}

export function useLang() {
  const { lang, setLang } = useContext(LangContext)
  return { lang, setLang }
}
