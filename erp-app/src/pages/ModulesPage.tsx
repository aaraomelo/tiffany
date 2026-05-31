import { useEffect, useState } from 'react'
import { applyPack, fetchPacks, toggleModule, type ModulePack, type TenantModule } from '../api'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'
import { useModules } from '../modules/ModulesContext'

const CATEGORY_ORDER: TenantModule['category'][] = [
  'CORE', 'SALES', 'SERVICE', 'FOOD', 'EVENT', 'SCHOOL', 'FISCAL', 'AI',
]

export function ModulesPage() {
  const t = useT()
  const { packSlug, modules, loading, refresh } = useModules()
  const [packs, setPacks] = useState<ModulePack[]>([])
  const [busy, setBusy] = useState<string | null>(null)
  const [error, setError] = useState<string | null>(null)
  const [info, setInfo] = useState<string | null>(null)

  useEffect(() => {
    void (async () => {
      try {
        setPacks(await fetchPacks())
      } catch (e) {
        setError((e as Error).message)
      }
    })()
  }, [])

  function flash(msg: string) {
    setInfo(msg)
    setTimeout(() => setInfo(null), 2500)
  }

  async function onApplyPack(slug: string) {
    if (slug === packSlug) return
    if (!confirm(t('modules.apply_confirm'))) return
    setBusy(`pack:${slug}`)
    setError(null)
    try {
      await applyPack(slug)
      await refresh()
      flash(t('modules.applied'))
    } catch (e) {
      setError((e as Error).message)
    } finally {
      setBusy(null)
    }
  }

  async function onToggle(m: TenantModule, next: boolean) {
    setBusy(`mod:${m.slug}`)
    setError(null)
    try {
      await toggleModule(m.slug, next)
      await refresh()
      flash(t('modules.saved'))
    } catch (e) {
      setError((e as Error).message)
    } finally {
      setBusy(null)
    }
  }

  // Módulos visíveis ao usuário (têm rota ou são núcleo), agrupados por categoria
  const visible = modules.filter((m) => m.routePath || m.isCore)
  const byCategory = visible.reduce<Record<string, TenantModule[]>>((acc, m) => {
    ;(acc[m.category] ??= []).push(m)
    return acc
  }, {})

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('modules.title')}</h1>
      <p style={{ color: 'var(--text-muted)', marginTop: 0 }}>{t('modules.subtitle')}</p>

      {error && <div style={errBox}>{error}</div>}
      {info && <div style={okBox}>{info}</div>}

      {/* ---------- Segmentos (packs) ---------- */}
      <section style={card}>
        <header>
          <h2 style={{ margin: 0, fontSize: 16 }}>{t('modules.segments_title')}</h2>
          <p style={{ margin: '0.2rem 0 0', color: 'var(--text-muted)', fontSize: 13 }}>
            {t('modules.segments_subtitle')}
          </p>
        </header>

        <div style={{ display: 'grid', gap: '0.8rem', gridTemplateColumns: 'repeat(auto-fit, minmax(240px, 1fr))', marginTop: '1rem' }}>
          {packs.map((p) => {
            const active = p.slug === packSlug
            return (
              <div
                key={p.slug}
                style={{
                  border: `2px solid ${active ? 'var(--primary)' : 'var(--border)'}`,
                  background: active ? 'var(--surface-alt)' : 'var(--surface)',
                  borderRadius: 10,
                  padding: '0.9rem 1rem',
                  display: 'flex',
                  flexDirection: 'column',
                  gap: '0.5rem',
                }}
              >
                <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <strong style={{ flex: 1 }}>{p.name}</strong>
                  {active && <span style={badge}>{t('modules.current_badge')}</span>}
                </div>
                <p style={{ margin: 0, fontSize: 13, color: 'var(--text-muted)', flex: 1 }}>{p.description}</p>
                <div style={{ fontSize: 12, color: 'var(--text-muted)' }}>
                  {t('modules.count', { count: p.items.length })}
                </div>
                <button
                  onClick={() => void onApplyPack(p.slug)}
                  disabled={active || busy === `pack:${p.slug}`}
                  style={active ? btnDisabled : btnPrimary}
                >
                  {busy === `pack:${p.slug}` ? '…' : active ? t('modules.current_badge') : t('modules.apply')}
                </button>
              </div>
            )
          })}
        </div>
      </section>

      {/* ---------- Módulos individuais ---------- */}
      <section style={card}>
        <header>
          <h2 style={{ margin: 0, fontSize: 16 }}>{t('modules.modules_title')}</h2>
          <p style={{ margin: '0.2rem 0 0', color: 'var(--text-muted)', fontSize: 13 }}>
            {t('modules.modules_subtitle')}
          </p>
        </header>

        {loading && <p style={{ marginTop: '1rem' }}>{t('common.loading')}</p>}

        {!loading && (
          <div style={{ marginTop: '1rem', display: 'grid', gap: '0.9rem' }}>
            {CATEGORY_ORDER.filter((cat) => byCategory[cat]?.length).map((cat) => (
              <div key={cat}>
                <div style={catLabel}>{t(`modules.cat.${cat}`)}</div>
                <div style={{ display: 'grid', gap: '0.3rem', gridTemplateColumns: 'repeat(auto-fill, minmax(220px, 1fr))' }}>
                  {byCategory[cat]
                    .sort((a, b) => a.sortOrder - b.sortOrder)
                    .map((m) => (
                      <label
                        key={m.slug}
                        title={m.isCore ? t('settings.modules_core_hint') : undefined}
                        style={{
                          display: 'flex',
                          alignItems: 'center',
                          gap: '0.6rem',
                          padding: '0.5rem 0.7rem',
                          background: 'var(--surface-alt)',
                          border: '1px solid var(--border)',
                          borderRadius: 6,
                          opacity: m.isCore ? 0.7 : 1,
                          cursor: m.isCore ? 'not-allowed' : 'pointer',
                        }}
                      >
                        <input
                          type="checkbox"
                          checked={m.enabled}
                          disabled={m.isCore || busy === `mod:${m.slug}`}
                          onChange={(e) => onToggle(m, e.target.checked)}
                        />
                        <span style={{ flex: 1, fontSize: 14 }}>{m.name}</span>
                        {m.isCore && (
                          <span style={{ fontSize: 11, color: 'var(--text-muted)' }}>{t('settings.modules_core_badge')}</span>
                        )}
                      </label>
                    ))}
                </div>
              </div>
            ))}
          </div>
        )}
      </section>
    </Layout>
  )
}

const card: React.CSSProperties = {
  background: 'var(--surface)',
  border: '1px solid var(--border)',
  padding: '1.2rem',
  borderRadius: 8,
  marginBottom: '1rem',
}
const catLabel: React.CSSProperties = {
  fontSize: 12, fontWeight: 600, color: 'var(--text-muted)',
  textTransform: 'uppercase', letterSpacing: 0.5, marginBottom: '0.4rem',
}
const badge: React.CSSProperties = {
  background: 'var(--primary)', color: 'var(--text-on-primary)',
  padding: '0.15rem 0.5rem', borderRadius: 4, fontSize: 11, fontWeight: 600,
}
const btnPrimary: React.CSSProperties = {
  padding: '0.5rem 1rem', fontSize: 14, background: 'var(--primary)',
  color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer', fontWeight: 600,
}
const btnDisabled: React.CSSProperties = {
  ...btnPrimary, background: 'var(--surface-alt)', color: 'var(--text-muted)',
  border: '1px solid var(--border)', cursor: 'default', fontWeight: 400,
}
const errBox: React.CSSProperties = {
  background: '#fee', border: '1px solid var(--danger)', padding: '0.7rem', borderRadius: 6, marginBottom: '1rem',
}
const okBox: React.CSSProperties = {
  background: '#e8f5e9', border: '1px solid var(--success)', padding: '0.7rem', borderRadius: 6, marginBottom: '1rem',
}
