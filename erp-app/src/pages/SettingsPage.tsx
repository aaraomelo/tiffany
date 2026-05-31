import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { api, toggleModule, type TenantModule } from '../api'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'
import { useModules } from '../modules/ModulesContext'

interface AssistantConfig {
  llmProvider: string
  model: string
  hasApiKey: boolean
  apiKeyMasked: string | null
  soulPrompt: string | null
  active: boolean
}

const MODELS = [
  { id: 'claude-haiku-4-5', label: 'Claude Haiku 4.5 (rápido, barato)' },
  { id: 'claude-sonnet-4-6', label: 'Claude Sonnet 4.6 (qualidade alta)' },
  { id: 'claude-opus-4-7', label: 'Claude Opus 4.7 (premium)' },
]

export function SettingsPage() {
  const t = useT()
  const [cfg, setCfg] = useState<AssistantConfig | null>(null)
  const [error, setError] = useState<string | null>(null)
  const [busy, setBusy] = useState(false)
  const [info, setInfo] = useState<string | null>(null)
  const [form, setForm] = useState({ model: '', apiKey: '', soulPrompt: '' })
  const [showKey, setShowKey] = useState(false)

  async function load() {
    setError(null)
    try {
      const c = await api<AssistantConfig>('/api/assistant/config')
      setCfg(c)
      setForm({
        model: c.model,
        apiKey: '',
        soulPrompt: c.soulPrompt ?? '',
      })
    } catch (e) {
      setError((e as Error).message)
    }
  }
  useEffect(() => { void load() }, [])

  async function save() {
    setBusy(true); setError(null); setInfo(null)
    try {
      const body: Record<string, string | null> = {
        model: form.model,
      }
      if (form.apiKey.trim()) body.apiKey = form.apiKey.trim()
      if (form.soulPrompt.trim()) body.soulPrompt = form.soulPrompt.trim()
      const updated = await api<AssistantConfig>('/api/assistant/config', {
        method: 'PUT',
        body: JSON.stringify(body),
      })
      setCfg(updated)
      setForm({ ...form, apiKey: '' })
      setInfo('Configurações salvas')
      setTimeout(() => setInfo(null), 2500)
    } catch (e) {
      setError((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  async function clearKey() {
    if (!confirm('Remover a chave de API?')) return
    setBusy(true); setError(null)
    try {
      await api('/api/assistant/config', {
        method: 'PUT',
        body: JSON.stringify({ apiKey: null }),
      })
      await load()
      setInfo('Chave removida')
      setTimeout(() => setInfo(null), 2500)
    } catch (e) {
      setError((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('settings.title')}</h1>
      <p style={{ color: 'var(--text-muted)', marginTop: 0 }}>{t('settings.subtitle')}</p>

      {error && <div style={errBox}>{error}</div>}
      {info && <div style={okBox}>{info}</div>}

      <section style={card}>
        <header style={cardHeader}>
          <div>
            <h2 style={{ margin: 0, fontSize: 16 }}>{t('settings.assistant_title')}</h2>
            <p style={{ margin: '0.2rem 0 0', color: 'var(--text-muted)', fontSize: 13 }}>
              {t('settings.assistant_subtitle')}
            </p>
          </div>
          {cfg?.hasApiKey && (
            <span style={badge('success')}>{t('settings.configured')}</span>
          )}
        </header>

        {!cfg && <p>{t('common.loading')}</p>}
        {cfg && (
          <div style={{ display: 'grid', gap: '0.8rem', marginTop: '1rem' }}>
            <label style={lbl}>
              <span>{t('settings.provider')}</span>
              <select disabled style={input}>
                <option>Anthropic (Claude)</option>
              </select>
            </label>

            <label style={lbl}>
              <span>{t('settings.model')}</span>
              <select
                value={form.model}
                onChange={(e) => setForm({ ...form, model: e.target.value })}
                style={input}
              >
                {MODELS.map((m) => <option key={m.id} value={m.id}>{m.label}</option>)}
              </select>
            </label>

            <label style={lbl}>
              <span>
                {t('settings.api_key')}
                {cfg.apiKeyMasked && (
                  <span style={{ marginLeft: 8, color: 'var(--text-muted)', fontSize: 12 }}>
                    {t('settings.current')}: <code>{cfg.apiKeyMasked}</code>
                  </span>
                )}
              </span>
              <div style={{ display: 'flex', gap: '0.4rem' }}>
                <input
                  type={showKey ? 'text' : 'password'}
                  value={form.apiKey}
                  onChange={(e) => setForm({ ...form, apiKey: e.target.value })}
                  placeholder={cfg.hasApiKey ? t('settings.replace_key_placeholder') : 'sk-ant-...'}
                  style={{ ...input, flex: 1, fontFamily: 'monospace' }}
                />
                <button type="button" onClick={() => setShowKey(!showKey)} style={btnSecondary}>
                  {showKey ? '🙈' : '👁'}
                </button>
                {cfg.hasApiKey && (
                  <button type="button" onClick={clearKey} disabled={busy} style={{ ...btnSecondary, color: 'var(--danger)' }}>
                    {t('settings.clear')}
                  </button>
                )}
              </div>
              <small style={{ color: 'var(--text-muted)', fontSize: 12 }}>
                {t('settings.api_key_hint')}
              </small>
            </label>

            <label style={lbl}>
              <span>{t('settings.soul_prompt')}</span>
              <textarea
                value={form.soulPrompt}
                onChange={(e) => setForm({ ...form, soulPrompt: e.target.value })}
                placeholder={t('settings.soul_prompt_placeholder')}
                style={{ ...input, minHeight: 80, fontFamily: 'monospace', fontSize: 13 }}
              />
              <small style={{ color: 'var(--text-muted)', fontSize: 12 }}>
                {t('settings.soul_prompt_hint')}
              </small>
            </label>

            <div style={{ display: 'flex', gap: '0.5rem', justifyContent: 'flex-end' }}>
              <button onClick={save} disabled={busy} style={btnPrimary}>
                {busy ? '…' : t('common.save')}
              </button>
            </div>
          </div>
        )}
      </section>

      <ModulesCard />

      <section style={card}>
        <header style={cardHeader}>
          <h2 style={{ margin: 0, fontSize: 16 }}>{t('settings.theme_title')}</h2>
          <Link to="/theme" style={{ color: 'var(--primary)', fontSize: 14 }}>{t('settings.go_to')}</Link>
        </header>
        <p style={{ margin: '0.5rem 0 0', color: 'var(--text-muted)', fontSize: 14 }}>
          {t('settings.theme_subtitle')}
        </p>
      </section>
    </Layout>
  )
}

function ModulesCard() {
  const t = useT()
  const { packSlug, modules, loading, refresh } = useModules()
  const [busy, setBusy] = useState<string | null>(null)
  const [err, setErr] = useState<string | null>(null)

  async function onToggle(m: TenantModule, next: boolean) {
    setBusy(m.slug)
    setErr(null)
    try {
      await toggleModule(m.slug, next)
      await refresh()
    } catch (e) {
      setErr((e as Error).message)
    } finally {
      setBusy(null)
    }
  }

  // Agrupar por categoria, ocultar módulos sem routePath (invisíveis ao usuário)
  const visible = modules.filter((m) => m.routePath || m.isCore)
  const byCategory = visible.reduce<Record<string, TenantModule[]>>((acc, m) => {
    ;(acc[m.category] ??= []).push(m)
    return acc
  }, {})
  const order: TenantModule['category'][] = ['CORE', 'SALES', 'SERVICE', 'FOOD', 'EVENT', 'SCHOOL', 'FISCAL', 'AI']

  return (
    <section style={card}>
      <header style={cardHeader}>
        <div>
          <h2 style={{ margin: 0, fontSize: 16 }}>{t('settings.modules_title')}</h2>
          <p style={{ margin: '0.2rem 0 0', color: 'var(--text-muted)', fontSize: 13 }}>
            {t('settings.modules_subtitle')}
          </p>
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: '0.6rem' }}>
          {packSlug && (
            <span style={{ background: 'var(--surface-alt)', border: '1px solid var(--border)', padding: '0.25rem 0.6rem', borderRadius: 4, fontSize: 12 }}>
              {t('settings.modules_current_pack')}: <strong>{packSlug}</strong>
            </span>
          )}
          <Link to="/modules" style={{ color: 'var(--primary)', fontSize: 14 }}>
            {t('settings.manage_modules')} →
          </Link>
        </div>
      </header>

      {err && <div style={{ ...errBox, marginTop: '0.8rem' }}>{err}</div>}
      {loading && <p style={{ marginTop: '1rem' }}>{t('common.loading')}</p>}

      {!loading && (
        <div style={{ marginTop: '1rem', display: 'grid', gap: '0.8rem' }}>
          {order
            .filter((cat) => byCategory[cat]?.length)
            .map((cat) => (
              <div key={cat}>
                <div style={{ fontSize: 12, fontWeight: 600, color: 'var(--text-muted)', textTransform: 'uppercase', letterSpacing: 0.5, marginBottom: '0.4rem' }}>
                  {cat}
                </div>
                <div style={{ display: 'grid', gap: '0.3rem' }}>
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
                          opacity: m.isCore ? 0.75 : 1,
                          cursor: m.isCore ? 'not-allowed' : 'pointer',
                        }}
                      >
                        <input
                          type="checkbox"
                          checked={m.enabled}
                          disabled={m.isCore || busy === m.slug}
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
  )
}

const card: React.CSSProperties = {
  background: 'var(--surface)',
  border: '1px solid var(--border)',
  padding: '1.2rem',
  borderRadius: 'var(--radius-lg, 8px)',
  marginBottom: '1rem',
}
const cardHeader: React.CSSProperties = {
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
}
const lbl: React.CSSProperties = { display: 'grid', gap: 4, fontSize: 14 }
const input: React.CSSProperties = { padding: '0.55rem 0.7rem', fontSize: 14, borderRadius: 6 }
const btnPrimary: React.CSSProperties = {
  padding: '0.55rem 1.2rem', fontSize: 14,
  background: 'var(--primary)', color: 'var(--text-on-primary)',
  border: 'none', borderRadius: 6, cursor: 'pointer', fontWeight: 600,
}
const btnSecondary: React.CSSProperties = {
  padding: '0.55rem 0.8rem', fontSize: 14,
  background: 'var(--surface-alt)', color: 'var(--text)',
  border: '1px solid var(--border)', borderRadius: 6, cursor: 'pointer',
}
const errBox: React.CSSProperties = {
  background: '#fee', border: '1px solid var(--danger)',
  padding: '0.7rem', borderRadius: 6, marginBottom: '1rem',
}
const okBox: React.CSSProperties = {
  background: '#e8f5e9', border: '1px solid var(--success)',
  padding: '0.7rem', borderRadius: 6, marginBottom: '1rem',
}
function badge(kind: 'success' | 'warn'): React.CSSProperties {
  const colors = { success: 'var(--success)', warn: 'var(--warn)' }
  return {
    background: colors[kind], color: 'white',
    padding: '0.2rem 0.6rem', borderRadius: 4, fontSize: 12, fontWeight: 600,
  }
}
