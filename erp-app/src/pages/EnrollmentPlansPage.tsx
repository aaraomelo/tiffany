import { useEffect, useState } from 'react'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'

type BillingCycle = 'MONTHLY' | 'QUARTERLY' | 'SEMIANNUAL' | 'ANNUAL'

interface EnrollmentPlan {
  id: string
  name: string
  description: string | null
  price: string
  billingCycle: BillingCycle
  weeklySessions: number | null
  enrollmentFee: string
  active: boolean
}

function money(v: string | number) {
  return Number(v).toLocaleString('pt-BR', { style: 'currency', currency: 'BRL' })
}

export function EnrollmentPlansPage() {
  const t = useT()
  const [items, setItems] = useState<EnrollmentPlan[] | null>(null)
  const [error, setError] = useState<string | null>(null)
  const [creating, setCreating] = useState(false)
  const [name, setName] = useState('')
  const [price, setPrice] = useState('')
  const [weekly, setWeekly] = useState('')
  const [cycle, setCycle] = useState<BillingCycle>('MONTHLY')

  async function load() {
    setError(null)
    try {
      setItems(await api<EnrollmentPlan[]>('/api/enrollment-plans'))
    } catch (e) {
      setError((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!name.trim() || !price) return
    setCreating(true)
    try {
      await api('/api/enrollment-plans', {
        method: 'POST',
        body: JSON.stringify({
          name,
          price: Number(price),
          billingCycle: cycle,
          weeklySessions: weekly ? Number(weekly) : undefined,
        }),
      })
      setName(''); setPrice(''); setWeekly('')
      void load()
    } catch (e) {
      setError((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  async function toggle(p: EnrollmentPlan) {
    try {
      await api(`/api/enrollment-plans/${p.id}`, {
        method: 'PATCH',
        body: JSON.stringify({ active: !p.active }),
      })
      void load()
    } catch (e) {
      setError((e as Error).message)
    }
  }

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('plans.title')}</h1>

      <section style={{ background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1.5rem' }}>
        <h2 style={{ marginTop: 0, fontSize: 18 }}>{t('plans.new')}</h2>
        <form onSubmit={handleCreate} style={{ display: 'grid', gridTemplateColumns: '2fr 1fr 1fr 1.2fr auto', gap: '0.6rem', alignItems: 'end' }}>
          <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
            {t('common.name')}
            <input value={name} onChange={(e) => setName(e.target.value)} required style={input} />
          </label>
          <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
            {t('plans.price')}
            <input type="number" step="0.01" min="0" value={price} onChange={(e) => setPrice(e.target.value)} required style={input} />
          </label>
          <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
            {t('plans.weekly_sessions')}
            <input type="number" min="1" value={weekly} onChange={(e) => setWeekly(e.target.value)} style={input} />
          </label>
          <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
            {t('plans.billing_cycle')}
            <select value={cycle} onChange={(e) => setCycle(e.target.value as BillingCycle)} style={input}>
              <option value="MONTHLY">{t('plans.cycle.MONTHLY')}</option>
              <option value="QUARTERLY">{t('plans.cycle.QUARTERLY')}</option>
              <option value="SEMIANNUAL">{t('plans.cycle.SEMIANNUAL')}</option>
              <option value="ANNUAL">{t('plans.cycle.ANNUAL')}</option>
            </select>
          </label>
          <button type="submit" disabled={creating} style={btn}>
            {creating ? '…' : t('common.create')}
          </button>
        </form>
      </section>

      {error && <p style={{ color: 'var(--danger)' }}>{error}</p>}
      {!items && !error && <p>{t('common.loading')}</p>}

      {items && (
        <table style={{ width: '100%', borderCollapse: 'collapse', fontSize: 14 }}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>{t('common.name')}</th>
              <th style={td}>{t('plans.price')}</th>
              <th style={td}>{t('plans.billing_cycle')}</th>
              <th style={td}>{t('plans.weekly_sessions')}</th>
              <th style={td}>{t('common.status')}</th>
              <th style={td}>{t('common.actions')}</th>
            </tr>
          </thead>
          <tbody>
            {items.length === 0 && (
              <tr><td colSpan={6} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('plans.empty')}</td></tr>
            )}
            {items.map((p) => (
              <tr key={p.id} style={{ borderBottom: '1px solid var(--border)', opacity: p.active ? 1 : 0.5 }}>
                <td style={td}>{p.name}</td>
                <td style={td}>{money(p.price)}</td>
                <td style={td}>{t(`plans.cycle.${p.billingCycle}`)}</td>
                <td style={td}>{p.weeklySessions ?? '—'}</td>
                <td style={td}>{p.active ? t('plans.active') : t('plans.inactive')}</td>
                <td style={td}>
                  <button onClick={() => void toggle(p)} style={linkBtn}>
                    {p.active ? t('plans.deactivate') : t('plans.activate')}
                  </button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </Layout>
  )
}

const input: React.CSSProperties = { padding: '0.55rem 0.7rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const linkBtn: React.CSSProperties = { background: 'transparent', border: 'none', color: 'var(--primary)', cursor: 'pointer', fontSize: 13, padding: 0 }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
