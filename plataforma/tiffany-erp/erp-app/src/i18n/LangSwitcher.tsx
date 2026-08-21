import { useEffect, useRef, useState } from 'react'
import { useLang, type Lang } from './LangContext'
import './LangSwitcher.css'

interface LangOption {
  code: Lang
  label: string
  flag: string
}

const LANGS: LangOption[] = [
  { code: 'pt', label: 'Português', flag: '🇧🇷' },
  { code: 'es', label: 'Español', flag: '🇻🇪' },
  { code: 'en', label: 'English', flag: '🇺🇸' },
]

export function LangSwitcher() {
  const { lang, setLang } = useLang()
  const [open, setOpen] = useState(false)
  const wrapRef = useRef<HTMLDivElement | null>(null)

  const current = LANGS.find((l) => l.code === lang) ?? LANGS[0]

  useEffect(() => {
    if (!open) return
    function handleClickOutside(e: MouseEvent) {
      if (wrapRef.current && !wrapRef.current.contains(e.target as Node)) {
        setOpen(false)
      }
    }
    function handleEsc(e: KeyboardEvent) {
      if (e.key === 'Escape') setOpen(false)
    }
    document.addEventListener('mousedown', handleClickOutside)
    document.addEventListener('keydown', handleEsc)
    return () => {
      document.removeEventListener('mousedown', handleClickOutside)
      document.removeEventListener('keydown', handleEsc)
    }
  }, [open])

  function pick(code: Lang) {
    setLang(code)
    setOpen(false)
  }

  return (
    <div className="lang-switcher" ref={wrapRef}>
      <button
        className="lang-trigger"
        onClick={() => setOpen((o) => !o)}
        aria-haspopup="listbox"
        aria-expanded={open}
        type="button"
      >
        <span className="lang-flag" aria-hidden="true">{current.flag}</span>
        <span className="lang-code">{current.code.toUpperCase()}</span>
        <span className="lang-caret" aria-hidden="true">▾</span>
      </button>
      {open && (
        <ul className="lang-menu" role="listbox">
          {LANGS.map((l) => (
            <li key={l.code}>
              <button
                role="option"
                aria-selected={lang === l.code}
                className={`lang-option ${lang === l.code ? 'active' : ''}`}
                onClick={() => pick(l.code)}
                type="button"
              >
                <span className="lang-flag" aria-hidden="true">{l.flag}</span>
                <span className="lang-name">{l.label}</span>
                <span className="lang-code-small">{l.code.toUpperCase()}</span>
              </button>
            </li>
          ))}
        </ul>
      )}
    </div>
  )
}
