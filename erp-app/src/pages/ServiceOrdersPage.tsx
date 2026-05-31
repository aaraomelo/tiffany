import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

type Status =
  | 'OPEN' | 'IN_PROGRESS' | 'WAITING_PARTS' | 'WAITING_CUSTOMER'
  | 'FINISHED' | 'DELIVERED' | 'CANCELLED'

interface SORow {
  id: string
  number: number
  status: Status
  description: string
  total: string
  openedAt: string
  customer: { id: string; name: string }
  vehicle: { plate: string | null; brand: string | null; model: string | null } | null
}
interface ListResp { items: SORow[]; total: number }
interface Customer { id: string; name: string }
interface ProductMini { id: string; sku: string; name: string; salePrice: string }

const STATUS_STYLE: Record<Status, React.CSSProperties> = {
  OPEN: { color: 'var(--primary)' },
  IN_PROGRESS: { color: 'var(--secondary)', fontWeight: 600 },
  WAITING_PARTS: { color: 'var(--warn)' },
  WAITING_CUSTOMER: { color: 'var(--warn)' },
  FINISHED: { color: 'var(--success)', fontWeight: 600 },
  DELIVERED: { color: 'var(--success)' },
  CANCELLED: { color: 'var(--text-muted)', textDecoration: 'line-through' },
}

const ALL_STATUSES: Status[] = ['OPEN', 'IN_PROGRESS', 'WAITING_PARTS', 'WAITING_CUSTOMER', 'FINISHED', 'DELIVERED', 'CANCELLED']

export function ServiceOrdersPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<ListResp | null>(null)
  const [status, setStatus] = useState<Status | ''>('')
  const [showForm, setShowForm] = useState(false)
  const [customers, setCustomers] = useState<Customer[]>([])
  const [products, setProducts] = useState<ProductMini[]>([])
  const [form, setForm] = useState({
    customerId: '', description: '',
    laborDesc: '', laborPrice: '',
    productId: '', productQty: '1',
  })
  const [busy, setBusy] = useState(false)

  useEffect(() => {
    setForm((f) => ({ ...f, laborDesc: f.laborDesc || t('service_orders.labors') }))
  }, [t])

  async function load() {
    const params = new URLSearchParams()
    if (status) params.set('status', status)
    setData(await api<ListResp>(`/api/service-orders?${params.toString()}`))
  }
  useEffect(() => { void load() }, [status])

  async function loadOptions() {
    const [cs, ps] = await Promise.all([
      api<{ items: Customer[] }>('/api/customer-suppliers?role=CUSTOMER&pageSize=200'),
      api<{ items: ProductMini[] }>('/api/products?pageSize=200'),
    ])
    setCustomers(cs.items)
    setProducts(ps.items)
  }
  useEffect(() => { if (showForm) void loadOptions() }, [showForm])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    setBusy(true)
    try {
      const labors = form.laborPrice ? [{
        description: form.laborDesc,
        quantity: 1,
        unitPrice: Number(form.laborPrice),
      }] : []
      const parts = form.productId ? [{
        productId: form.productId,
        quantity: Number(form.productQty),
      }] : []
      await api('/api/service-orders', {
        method: 'POST',
        body: JSON.stringify({
          customerId: form.customerId,
          description: form.description,
          parts,
          labors,
        }),
      })
      setShowForm(false)
      setForm({ customerId: '', description: '', laborDesc: t('service_orders.labors'), laborPrice: '', productId: '', productQty: '1' })
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  return (
    <Layout>
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1rem' }}>
        <h1 style={{ margin: 0 }}>{t('service_orders.title')}</h1>
        <button onClick={() => setShowForm(!showForm)} style={btn}>
          {showForm ? t('common.cancel') : t('service_orders.new_btn')}
        </button>
      </header>

      {showForm && (
        <section style={card}>
          <h2 style={{ marginTop: 0, fontSize: 16 }}>{t('service_orders.new')}</h2>
          <form onSubmit={handleCreate} style={{ display: 'grid', gridTemplateColumns: 'repeat(2, 1fr)', gap: '0.6rem' }}>
            <Field label={t('service_orders.field_customer')} span={2}>
              <select required value={form.customerId} onChange={(e) => setForm({ ...form, customerId: e.target.value })} style={input}>
                <option value="">{t('service_orders.placeholder_select')}</option>
                {customers.map((c) => <option key={c.id} value={c.id}>{c.name}</option>)}
              </select>
            </Field>
            <Field label={t('service_orders.description')} span={2}>
              <input required value={form.description} onChange={(e) => setForm({ ...form, description: e.target.value })} placeholder={t('service_orders.placeholder_description')} style={input} />
            </Field>
            <Field label={t('service_orders.field_part_optional')}>
              <select value={form.productId} onChange={(e) => setForm({ ...form, productId: e.target.value })} style={input}>
                <option value="">{t('service_orders.no_part')}</option>
                {products.map((p) => <option key={p.id} value={p.id}>{p.sku} — {p.name} (R$ {Number(p.salePrice).toFixed(2)})</option>)}
              </select>
            </Field>
            <Field label={t('service_orders.field_qty')}>
              <input type="number" step="0.0001" min="0.0001" value={form.productQty} onChange={(e) => setForm({ ...form, productQty: e.target.value })} style={input} />
            </Field>
            <Field label={t('service_orders.field_labor_desc')}>
              <input value={form.laborDesc} onChange={(e) => setForm({ ...form, laborDesc: e.target.value })} style={input} />
            </Field>
            <Field label={t('service_orders.field_labor_value')}>
              <input type="number" step="0.01" min="0" value={form.laborPrice} onChange={(e) => setForm({ ...form, laborPrice: e.target.value })} placeholder={t('service_orders.placeholder_labor_value')} style={input} />
            </Field>
            <div style={{ gridColumn: 'span 2', display: 'flex', justifyContent: 'flex-end' }}>
              <button type="submit" disabled={busy} style={btn}>{busy ? '…' : t('service_orders.create_btn')}</button>
            </div>
          </form>
        </section>
      )}

      <div style={{ marginBottom: '1rem' }}>
        <select value={status} onChange={(e) => setStatus(e.target.value as Status | '')} style={input}>
          <option value="">{t('service_orders.all_status')}</option>
          {ALL_STATUSES.map((s) => <option key={s} value={s}>{s}</option>)}
        </select>
      </div>

      {!data && <p>{t('common.loading')}</p>}
      {data && (
        <table style={tbl}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>#</th>
              <th style={td}>{t('service_orders.col_opened')}</th>
              <th style={td}>{t('orders.customer')}</th>
              <th style={td}>{t('service_orders.col_vehicle')}</th>
              <th style={td}>{t('service_orders.description')}</th>
              <th style={{ ...td, textAlign: 'right' }}>{t('common.total')}</th>
              <th style={td}>{t('common.status')}</th>
              <th style={td}></th>
            </tr>
          </thead>
          <tbody>
            {data.items.length === 0 && <tr><td colSpan={8} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('service_orders.empty')}</td></tr>}
            {data.items.map((s) => (
              <tr key={s.id} style={{ borderBottom: '1px solid var(--border)' }}>
                <td style={td}><strong>{s.number}</strong></td>
                <td style={td}>{new Date(s.openedAt).toLocaleDateString()}</td>
                <td style={td}>{s.customer.name}</td>
                <td style={td}>{s.vehicle ? `${s.vehicle.plate ?? '—'} ${s.vehicle.brand ?? ''} ${s.vehicle.model ?? ''}` : '—'}</td>
                <td style={td}>{s.description}</td>
                <td style={{ ...td, textAlign: 'right' }}>R$ {Number(s.total).toFixed(2)}</td>
                <td style={{ ...td, ...STATUS_STYLE[s.status] }}>{s.status}</td>
                <td style={td}><Link to={`/service-orders/${s.id}`}>{t('service_orders.open')}</Link></td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </Layout>
  )
}

function Field({ label, children, span }: { label: string; children: React.ReactNode; span?: number }) {
  return (
    <label style={{ display: 'grid', gap: 4, fontSize: 13, gridColumn: span ? `span ${span}` : undefined }}>
      {label}
      {children}
    </label>
  )
}

const card: React.CSSProperties = { background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1.5rem' }
const input: React.CSSProperties = { padding: '0.5rem 0.6rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const tbl: React.CSSProperties = { width: '100%', borderCollapse: 'collapse', fontSize: 14, background: 'var(--surface)', borderRadius: 6, overflow: 'hidden', border: '1px solid var(--border)' }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
