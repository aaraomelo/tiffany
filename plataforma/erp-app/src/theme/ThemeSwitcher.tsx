import { useT } from '../i18n/LangContext'
import { useTheme } from './ThemeContext'
import './ThemeSwitcher.css'

export function ThemeSwitcher() {
  const { config, applyPreset, presets } = useTheme()
  const t = useT()
  const activeId = config.presetId

  return (
    <div className="theme-switcher" role="group" aria-label={t('theme.aria_label')}>
      {presets.map((p) => (
        <button
          key={p.id}
          className={`theme-swatch ${activeId === p.id ? 'active' : ''}`}
          onClick={() => applyPreset(p.id)}
          aria-pressed={activeId === p.id}
          title={t(p.labelKey)}
          type="button"
          style={{
            background: p.config.primary,
            outline: activeId === p.id ? `2px solid ${p.config.textOnPrimary}` : 'none',
          }}
        />
      ))}
    </div>
  )
}
