import { useEffect, useState } from 'react'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'

interface OrderRow {
  id: string
  number: number
  total: string
  status: string
  channel: string
  createdAt: string
  customer: { id: string; name: string } | null
  items: Array<{ id: string }>
}
interface ListResponse { items: OrderRow[]; total: number; page: number; pageSize: number }

const STATUS_STYLE: Record<string, React.CSSProperties> = {
  COMPLETED: { color: 'var(--success)', fontWeight: 600 },
  PAID: { color: 'var(--primary)', fontWeight: 600 },
  PARTIALLY_PAID: { color: 'var(--warn)' },
  AWAITING_PAYMENT: { color: 'var(--text-muted)' },
  CANCELLED: { color: 'var(--text-muted)', textDecoration: 'line-through' },
}

const STATUS_KEYS = ['AWAITING_PAYMENT', 'PARTIALLY_PAID', 'PAID', 'COMPLETED', 'CANCELLED']

export function OrdersPage() {
  const t = useT()
  const [data, setData] = useState<ListResponse | null>(null)
  const [status, setStatus] = useState('')

  async function load() {
    const params = new URLSearchParams()
    if (status) params.set('status', status)
    setData(await api<ListResponse>(`/api/orders?${params.toString()}`))
  }
  useEffect(() => { void load() }, [status])

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('orders.title')}</h1>
      <div style={{ marginBottom: '1rem' }}>
        <select value={status} onChange={(e) => setStatus(e.target.value)} style={input}>
          <option value="">{t('common.all')}</option>
          {STATUS_KEYS.map((s) => <option key={s} value={s}>{t(`orders.status.${s}`)}</option>)}
        </select>
      </div>
      {!data && <p>{t('common.loading')}</p>}
      {data && (
        <table style={tbl}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>{t('orders.col_number')}</th>
              <th style={td}>{t('orders.col_date')}</th>
              <th style={td}>{t('orders.customer')}</th>
              <th style={td}>{t('orders.items')}</th>
              <th style={td}>{t('orders.channel')}</th>
              <th style={{ ...td, textAlign: 'right' }}>{t('common.total')}</th>
              <th style={td}>{t('common.status')}</th>
            </tr>
          </thead>
          <tbody>
            {data.items.length === 0 && <tr><td colSpan={7} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('orders.empty')}</td></tr>}
            {data.items.map((o) => (
              <tr key={o.id} style={{ borderBottom: '1px solid var(--border)' }}>
                <td style={td}><strong>{o.number}</strong></td>
                <td style={td}>{new Date(o.createdAt).toLocaleString()}</td>
                <td style={td}>{o.customer?.name ?? '—'}</td>
                <td style={td}>{o.items.length}</td>
                <td style={td}>{o.channel}</td>
                <td style={{ ...td, textAlign: 'right' }}>R$ {Number(o.total).toFixed(2)}</td>
                <td style={{ ...td, ...(STATUS_STYLE[o.status] ?? {}) }}>{t(`orders.status.${o.status}`, {})}</td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </Layout>
  )
}

const input: React.CSSProperties = { padding: '0.5rem 0.6rem', fontSize: 14, borderRadius: 6 }
const tbl: React.CSSProperties = { width: '100%', borderCollapse: 'collapse', fontSize: 14, background: 'var(--surface)', borderRadius: 6, overflow: 'hidden', border: '1px solid var(--border)' }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
