import { useEffect, useState } from 'react'
import { fetchPacks, toggleModule, togglePack, type ModulePack, type TenantModule } from '../api'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { useModules } from '../modules/ModulesContext'

const CATEGORY_ORDER: TenantModule['category'][] = [
  'REGISTRY', 'INVENTORY', 'SALES', 'SERVICE', 'FOOD', 'EVENT', 'SCHOOL', 'SYSTEM', 'FISCAL', 'AI', 'CORE',
]

export function ModulesPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const { modules, activePacks, loading, applyResponse, patchModule } = useModules()
  const [packs, setPacks] = useState<ModulePack[]>([])
  const [busy, setBusy] = useState<string | null>(null)

  useEffect(() => {
    void (async () => {
      try {
        setPacks(await fetchPacks())
      } catch (e) {
        snackbar.error((e as Error).message)
      }
    })()
  }, [snackbar])

  // Liga (aditivo) ou desliga o segmento inteiro. Desligar não apaga dados.
  async function onTogglePack(slug: string, enabled: boolean) {
    setBusy(`pack:${slug}`)
    try {
      const data = await togglePack(slug, enabled)
      applyResponse(data)
      snackbar.success(enabled ? t('modules.applied') : t('modules.removed'))
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(null)
    }
  }

  // Otimista: reflete na hora e reverte se o servidor recusar.
  async function onToggle(m: TenantModule, next: boolean) {
    patchModule(m.slug, next)
    try {
      await toggleModule(m.slug, next)
    } catch (e) {
      patchModule(m.slug, !next)
      snackbar.error((e as Error).message)
    }
  }

  const activeSet = new Set(activePacks)

  // Só módulos com tela própria (oculta infra de plataforma sem rota),
  // agrupados por categoria
  const visible = modules.filter((m) => m.routePath)
  const byCategory = visible.reduce<Record<string, TenantModule[]>>((acc, m) => {
    ;(acc[m.category] ??= []).push(m)
    return acc
  }, {})

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('modules.title')}</h1>
      <p style={{ color: 'var(--text-muted)', marginTop: 0 }}>{t('modules.subtitle')}</p>

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
            // Segmento "ativo" = foi explicitamente ativado pelo tenant.
            const active = activeSet.has(p.slug)
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
                  {active && <span style={badge}>{t('modules.active_badge')}</span>}
                </div>
                <p style={{ margin: 0, fontSize: 13, color: 'var(--text-muted)', flex: 1 }}>{p.description}</p>
                <div style={{ fontSize: 12, color: 'var(--text-muted)' }}>
                  {t('modules.count', { count: p.items.length })}
                </div>
                <button
                  onClick={() => void onTogglePack(p.slug, !active)}
                  disabled={busy === `pack:${p.slug}`}
                  style={active ? btnGhost : btnPrimary}
                >
                  {busy === `pack:${p.slug}` ? '…' : active ? t('modules.deactivate') : t('modules.activate')}
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
const btnGhost: React.CSSProperties = {
  ...btnPrimary, background: 'transparent', color: 'var(--danger)',
  border: '1px solid var(--danger)', fontWeight: 600,
}
