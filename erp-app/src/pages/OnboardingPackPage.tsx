import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { applyPack, fetchPacks, type ModulePack } from '../api'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'
import { useModules } from '../modules/ModulesContext'
import { ThemeSwitcher } from '../theme/ThemeSwitcher'

export function OnboardingPackPage() {
  const t = useT()
  const navigate = useNavigate()
  const { packSlug, refresh } = useModules()
  const [packs, setPacks] = useState<ModulePack[]>([])
  const [selected, setSelected] = useState<string | null>(null)
  const [loading, setLoading] = useState(true)
  const [saving, setSaving] = useState(false)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    void (async () => {
      try {
        const list = await fetchPacks()
        setPacks(list)
        const initial = packSlug ?? list.find((p) => p.isDefault)?.slug ?? list[0]?.slug ?? null
        setSelected(initial)
      } catch (e) {
        setError((e as Error).message)
      } finally {
        setLoading(false)
      }
    })()
  }, [packSlug])

  async function confirm() {
    if (!selected) return
    setSaving(true)
    setError(null)
    try {
      await applyPack(selected)
      await refresh()
      navigate('/pos')
    } catch (e) {
      setError((e as Error).message)
    } finally {
      setSaving(false)
    }
  }

  return (
    <div style={{ minHeight: '100vh', background: 'var(--bg)', color: 'var(--text)', fontFamily: 'system-ui' }}>
      <header style={{ background: 'var(--primary)', color: 'var(--text-on-primary)', padding: '0.6rem 1.5rem', display: 'flex', alignItems: 'center', gap: '1rem' }}>
        <strong style={{ fontSize: 18, flex: 1 }}>{t('app.name')}</strong>
        <ThemeSwitcher />
        <LangSwitcher />
      </header>

      <main style={{ maxWidth: 880, margin: '2.5rem auto', padding: '0 1.5rem' }}>
        <h1 style={{ marginTop: 0, color: 'var(--primary)' }}>{t('onboarding.pack.title')}</h1>
        <p style={{ color: 'var(--text-muted)', marginTop: 0, marginBottom: '2rem' }}>
          {t('onboarding.pack.subtitle')}
        </p>

        {error && <div style={errBox}>{error}</div>}
        {loading && <p>{t('common.loading')}</p>}

        {!loading && (
          <>
            <div style={{ display: 'grid', gap: '1rem', gridTemplateColumns: 'repeat(auto-fit, minmax(260px, 1fr))' }}>
              {packs.map((p) => {
                const active = selected === p.slug
                return (
                  <button
                    key={p.slug}
                    type="button"
                    onClick={() => setSelected(p.slug)}
                    style={{
                      textAlign: 'left',
                      background: active ? 'var(--surface-alt)' : 'var(--surface)',
                      border: `2px solid ${active ? 'var(--primary)' : 'var(--border)'}`,
                      borderRadius: 10,
                      padding: '1.1rem',
                      cursor: 'pointer',
                      color: 'inherit',
                      fontFamily: 'inherit',
                    }}
                  >
                    <div style={{ display: 'flex', alignItems: 'center', gap: '0.6rem', marginBottom: '0.4rem' }}>
                      <h2 style={{ margin: 0, fontSize: 17 }}>{p.name}</h2>
                      {p.isDefault && (
                        <span style={{ background: 'var(--primary)', color: 'var(--text-on-primary)', padding: '0.1rem 0.5rem', borderRadius: 4, fontSize: 11, fontWeight: 600 }}>
                          {t('onboarding.pack.recommended')}
                        </span>
                      )}
                    </div>
                    <p style={{ margin: '0 0 0.6rem', color: 'var(--text-muted)', fontSize: 13, lineHeight: 1.4 }}>
                      {p.description}
                    </p>
                    <small style={{ color: 'var(--text-muted)', fontSize: 12 }}>
                      {t('onboarding.pack.modules_count', { count: p.items.length })}
                    </small>
                  </button>
                )
              })}
            </div>

            <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '0.6rem', marginTop: '2rem' }}>
              <button
                type="button"
                onClick={() => navigate('/settings')}
                style={btnSecondary}
                disabled={saving}
              >
                {t('onboarding.pack.customize_later')}
              </button>
              <button
                type="button"
                onClick={confirm}
                disabled={!selected || saving}
                style={btnPrimary}
              >
                {saving ? '…' : t('onboarding.pack.confirm')}
              </button>
            </div>
          </>
        )}
      </main>
    </div>
  )
}

const errBox: React.CSSProperties = {
  background: '#fee', border: '1px solid var(--danger)',
  padding: '0.7rem', borderRadius: 6, marginBottom: '1rem',
}
const btnPrimary: React.CSSProperties = {
  padding: '0.6rem 1.3rem', fontSize: 14,
  background: 'var(--primary)', color: 'var(--text-on-primary)',
  border: 'none', borderRadius: 6, cursor: 'pointer', fontWeight: 600,
}
const btnSecondary: React.CSSProperties = {
  padding: '0.6rem 1.3rem', fontSize: 14,
  background: 'var(--surface-alt)', color: 'var(--text)',
  border: '1px solid var(--border)', borderRadius: 6, cursor: 'pointer',
}
