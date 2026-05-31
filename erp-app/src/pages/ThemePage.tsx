import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { useTheme } from '../theme/ThemeContext'
import type { ThemeConfig } from '../theme/palettes'

type ColorKey =
  | 'primary' | 'primaryHover' | 'primaryLight'
  | 'secondary' | 'accent'
  | 'bg' | 'surface' | 'surfaceAlt' | 'border'
  | 'text' | 'textMuted'
  | 'success' | 'warn' | 'danger'
  | 'textOnPrimary'

const COLOR_GROUPS: { label: string; keys: ColorKey[] }[] = [
  { label: 'theme.group.brand', keys: ['primary', 'primaryHover', 'primaryLight', 'secondary', 'accent', 'textOnPrimary'] },
  { label: 'theme.group.surfaces', keys: ['bg', 'surface', 'surfaceAlt', 'border'] },
  { label: 'theme.group.text', keys: ['text', 'textMuted'] },
  { label: 'theme.group.feedback', keys: ['success', 'warn', 'danger'] },
]

export function ThemePage() {
  const { config, applyPreset, updateConfig, saveToServer, resetServer, presets, saving, dirty } = useTheme()
  const t = useT()
  const snackbar = useSnackbar()

  async function handleSave() {
    try {
      await saveToServer()
      snackbar.success(t('theme.saved'))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  async function handleReset() {
    try {
      await resetServer()
      snackbar.success(t('theme.reset_ok'))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  return (
    <Layout>
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', marginBottom: '1rem', flexWrap: 'wrap', gap: '0.6rem' }}>
        <div>
          <h1 style={{ margin: 0 }}>{t('theme.title')}</h1>
          <p style={{ margin: '0.3rem 0 0', color: 'var(--text-muted)', fontSize: 14 }}>{t('theme.subtitle')}</p>
        </div>
        <div style={{ display: 'flex', gap: '0.5rem', alignItems: 'center' }}>
          <button onClick={handleReset} disabled={saving} style={btnSecondary}>{t('theme.reset')}</button>
          <button onClick={handleSave} disabled={saving || !dirty} style={btnPrimary}>
            {saving ? '…' : (dirty ? t('theme.save') : t('theme.saved_clean'))}
          </button>
        </div>
      </header>

      <div style={{ display: 'grid', gridTemplateColumns: '1.4fr 1fr', gap: '1.2rem' }}>
        <div>
          <section style={card}>
            <h2 style={h2}>{t('theme.presets')}</h2>
            <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(160px, 1fr))', gap: '0.6rem' }}>
              {presets.map((p) => (
                <button key={p.id} onClick={() => applyPreset(p.id)} style={presetCard(p.config.primary, p.config.surface, p.config.text, config.presetId === p.id)}>
                  <div style={{ display: 'flex', gap: 4, marginBottom: 8 }}>
                    {[p.config.primary, p.config.secondary, p.config.accent, p.config.surface].map((c, i) => (
                      <span key={i} style={{ width: 20, height: 20, borderRadius: 4, background: c, border: '1px solid rgba(0,0,0,0.1)' }} />
                    ))}
                  </div>
                  <div style={{ fontSize: 13, fontWeight: 600 }}>{t(p.labelKey)}</div>
                </button>
              ))}
            </div>
          </section>

          {COLOR_GROUPS.map((g) => (
            <section key={g.label} style={card}>
              <h2 style={h2}>{t(g.label)}</h2>
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(220px, 1fr))', gap: '0.6rem' }}>
                {g.keys.map((k) => (
                  <ColorField key={k} k={k} config={config} onChange={updateConfig} t={t} />
                ))}
              </div>
            </section>
          ))}

          <section style={card}>
            <h2 style={h2}>{t('theme.group.geometry')}</h2>
            <RangeField
              label={t('theme.radius')} value={config.radius} min={0} max={20}
              onChange={(v) => updateConfig({ radius: v })} unit="px"
            />
            <RangeField
              label={t('theme.radius_lg')} value={config.radiusLg} min={0} max={32}
              onChange={(v) => updateConfig({ radiusLg: v })} unit="px"
            />
            <RangeField
              label={t('theme.border_width')} value={config.borderWidth} min={0} max={4}
              onChange={(v) => updateConfig({ borderWidth: v })} unit="px"
            />
            <RangeField
              label={t('theme.spacing_base')} value={config.spacingBase} min={20} max={150}
              onChange={(v) => updateConfig({ spacingBase: v })} unit="/100rem"
            />
          </section>
        </div>

        <aside style={{ position: 'sticky', top: '1rem', alignSelf: 'flex-start' }}>
          <section style={card}>
            <h2 style={h2}>{t('theme.preview')}</h2>
            <Preview t={t} />
          </section>
        </aside>
      </div>
    </Layout>
  )
}

function ColorField({ k, config, onChange, t }: { k: ColorKey; config: ThemeConfig; onChange: (p: Partial<ThemeConfig>) => void; t: (k: string) => string }) {
  const value = config[k]
  return (
    <label style={{ display: 'flex', gap: '0.5rem', alignItems: 'center', fontSize: 13 }}>
      <input type="color" value={value} onChange={(e) => onChange({ [k]: e.target.value } as Partial<ThemeConfig>)}
        style={{ width: 36, height: 30, padding: 0, border: '1px solid var(--border)', borderRadius: 4, cursor: 'pointer' }} />
      <input type="text" value={value} onChange={(e) => onChange({ [k]: e.target.value } as Partial<ThemeConfig>)}
        style={{ width: 80, fontFamily: 'monospace', fontSize: 12, padding: '0.3rem 0.4rem', borderRadius: 4 }} />
      <span style={{ flex: 1 }}>{t(`theme.color.${k}`)}</span>
    </label>
  )
}

function RangeField({ label, value, min, max, onChange, unit }: { label: string; value: number; min: number; max: number; onChange: (v: number) => void; unit: string }) {
  return (
    <div style={{ display: 'grid', gridTemplateColumns: '160px 1fr 80px', gap: '0.6rem', alignItems: 'center', marginBottom: '0.5rem', fontSize: 13 }}>
      <span>{label}</span>
      <input type="range" min={min} max={max} value={value} onChange={(e) => onChange(Number(e.target.value))} />
      <span style={{ fontFamily: 'monospace', fontSize: 12, color: 'var(--text-muted)' }}>{value}{unit}</span>
    </div>
  )
}

function Preview({ t }: { t: (k: string) => string }) {
  return (
    <div>
      <div style={{ background: 'var(--primary)', color: 'var(--text-on-primary)', padding: 'var(--spacing-base, 0.5rem) calc(var(--spacing-base, 0.5rem) * 2)', borderRadius: 'var(--radius-lg, 8px)', marginBottom: '0.8rem', fontWeight: 600 }}>
        {t('theme.preview_header')}
      </div>
      <p style={{ color: 'var(--text)', fontSize: 14, margin: '0.5rem 0' }}>{t('theme.preview_text')}</p>
      <p style={{ color: 'var(--text-muted)', fontSize: 13, margin: '0.3rem 0' }}>{t('theme.preview_muted')}</p>
      <div style={{ display: 'flex', gap: '0.5rem', flexWrap: 'wrap', margin: '0.8rem 0' }}>
        <button style={btnPrimary}>{t('theme.preview_btn_primary')}</button>
        <button style={btnSecondary}>{t('theme.preview_btn_secondary')}</button>
      </div>
      <div style={{ background: 'var(--surface)', border: `var(--border-width, 1px) solid var(--border)`, padding: '0.8rem', borderRadius: 'var(--radius-lg, 8px)', marginBottom: '0.5rem' }}>
        <strong>{t('theme.preview_card_title')}</strong>
        <div style={{ fontSize: 13, color: 'var(--text-muted)', marginTop: 4 }}>{t('theme.preview_card_body')}</div>
      </div>
      <div style={{ display: 'flex', gap: '0.4rem' }}>
        <Pill bg="var(--success)">{t('theme.feedback.success')}</Pill>
        <Pill bg="var(--warn)">{t('theme.feedback.warn')}</Pill>
        <Pill bg="var(--danger)">{t('theme.feedback.danger')}</Pill>
        <Pill bg="var(--accent)">{t('theme.feedback.accent')}</Pill>
      </div>
    </div>
  )
}

function Pill({ bg, children }: { bg: string; children: React.ReactNode }) {
  return (
    <span style={{ background: bg, color: '#fff', padding: '0.2rem 0.55rem', borderRadius: 'var(--radius, 6px)', fontSize: 12, fontWeight: 600 }}>{children}</span>
  )
}

const card: React.CSSProperties = {
  background: 'var(--surface)',
  border: `var(--border-width, 1px) solid var(--border)`,
  padding: '1rem',
  borderRadius: 'var(--radius-lg, 8px)',
  marginBottom: '1rem',
}
const h2: React.CSSProperties = { marginTop: 0, fontSize: 14, textTransform: 'uppercase', letterSpacing: '0.05em', color: 'var(--text-muted)', marginBottom: '0.8rem' }
const btnPrimary: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 'var(--radius, 6px)', cursor: 'pointer', fontWeight: 600 }
const btnSecondary: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--surface-alt)', color: 'var(--text)', border: `var(--border-width, 1px) solid var(--border)`, borderRadius: 'var(--radius, 6px)', cursor: 'pointer' }

function presetCard(primary: string, surface: string, text: string, active: boolean): React.CSSProperties {
  return {
    padding: '0.6rem',
    borderRadius: 'var(--radius-lg, 8px)',
    background: surface,
    color: text,
    border: active ? `2px solid ${primary}` : `var(--border-width, 1px) solid var(--border)`,
    cursor: 'pointer',
    textAlign: 'left',
  }
}
