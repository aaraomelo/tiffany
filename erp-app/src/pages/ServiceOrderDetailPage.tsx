import { useEffect, useState } from 'react'
import { Link, useParams } from 'react-router-dom'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

type Status =
  | 'OPEN' | 'IN_PROGRESS' | 'WAITING_PARTS' | 'WAITING_CUSTOMER'
  | 'FINISHED' | 'DELIVERED' | 'CANCELLED'

interface Part {
  id: string
  productId: string
  quantity: string
  unitPrice: string
  total: string
  product: { sku: string; name: string }
}
interface Labor {
  id: string
  description: string
  quantity: string
  unitPrice: string
  total: string
}
interface SODetail {
  id: string
  number: number
  status: Status
  description: string
  diagnosis: string | null
  partsTotal: string
  laborTotal: string
  discount: string
  total: string
  openedAt: string
  finishedAt: string | null
  deliveredAt: string | null
  customer: { id: string; name: string; email: string | null; phone: string | null }
  vehicle: { id: string; plate: string | null; brand: string | null; model: string | null } | null
  mechanic: { id: string; name: string } | null
  warrantyTerm: { id: string; name: string; days: number } | null
  parts: Part[]
  labors: Labor[]
}
interface ProductMini { id: string; sku: string; name: string; salePrice: string }

const ALLOWED: Record<Status, Status[]> = {
  OPEN: ['IN_PROGRESS', 'WAITING_PARTS', 'WAITING_CUSTOMER', 'CANCELLED'],
  IN_PROGRESS: ['WAITING_PARTS', 'WAITING_CUSTOMER', 'FINISHED', 'CANCELLED'],
  WAITING_PARTS: ['IN_PROGRESS', 'CANCELLED'],
  WAITING_CUSTOMER: ['IN_PROGRESS', 'FINISHED', 'CANCELLED'],
  FINISHED: ['DELIVERED'],
  DELIVERED: [],
  CANCELLED: [],
}

export function ServiceOrderDetailPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const { id } = useParams<{ id: string }>()
  const [so, setSo] = useState<SODetail | null>(null)
  const [products, setProducts] = useState<ProductMini[]>([])
  const [partProductId, setPartProductId] = useState('')
  const [partQty, setPartQty] = useState('1')
  const [laborDesc, setLaborDesc] = useState('')
  const [laborPrice, setLaborPrice] = useState('')
  const [busy, setBusy] = useState(false)

  async function load() {
    if (!id) return
    try {
      const data = await api<SODetail>(`/api/service-orders/${id}`)
      setSo(data)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }
  useEffect(() => {
    void load()
    api<{ items: ProductMini[] }>('/api/products?pageSize=200').then((r) => setProducts(r.items))
  }, [id])

  async function addPart(e: React.FormEvent) {
    e.preventDefault()
    if (!so) return
    setBusy(true)
    try {
      await api(`/api/service-orders/${so.id}/parts`, {
        method: 'POST',
        body: JSON.stringify({ productId: partProductId, quantity: Number(partQty) }),
      })
      setPartProductId(''); setPartQty('1')
      void load()
    } catch (e) { snackbar.error((e as Error).message) } finally { setBusy(false) }
  }

  async function removePart(partId: string) {
    if (!so) return
    try {
      await api(`/api/service-orders/${so.id}/parts/${partId}`, { method: 'DELETE' })
      void load()
    } catch (e) { snackbar.error((e as Error).message) }
  }

  async function addLabor(e: React.FormEvent) {
    e.preventDefault()
    if (!so) return
    setBusy(true)
    try {
      await api(`/api/service-orders/${so.id}/labors`, {
        method: 'POST',
        body: JSON.stringify({ description: laborDesc, quantity: 1, unitPrice: Number(laborPrice) }),
      })
      setLaborDesc(''); setLaborPrice('')
      void load()
    } catch (e) { snackbar.error((e as Error).message) } finally { setBusy(false) }
  }

  async function removeLabor(laborId: string) {
    if (!so) return
    try {
      await api(`/api/service-orders/${so.id}/labors/${laborId}`, { method: 'DELETE' })
      void load()
    } catch (e) { snackbar.error((e as Error).message) }
  }

  async function changeStatus(status: Status) {
    if (!so) return
    setBusy(true)
    try {
      await api(`/api/service-orders/${so.id}/status`, {
        method: 'POST',
        body: JSON.stringify({ status }),
      })
      void load()
    } catch (e) { snackbar.error((e as Error).message) } finally { setBusy(false) }
  }

  if (!so) return <Layout><p>{t('common.loading')}</p></Layout>

  const transitions = ALLOWED[so.status] ?? []
  const editable = !['DELIVERED', 'CANCELLED', 'FINISHED'].includes(so.status)

  return (
    <Layout>
      <p style={{ marginTop: 0 }}><Link to="/service-orders">{t('service_orders.back')}</Link></p>
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', marginBottom: '1rem' }}>
        <div>
          <h1 style={{ margin: 0 }}>{t('nav.service_orders')} #{so.number}</h1>
          <p style={{ margin: '0.3rem 0', color: 'var(--text-muted)', fontSize: 14 }}>
            {so.customer.name}
            {so.vehicle && ` · ${so.vehicle.plate ?? '—'} ${so.vehicle.brand ?? ''} ${so.vehicle.model ?? ''}`}
            {so.warrantyTerm && ` · ${t('service_orders.detail_warranty', { name: so.warrantyTerm.name, days: so.warrantyTerm.days })}`}
          </p>
          <p style={{ margin: '0.3rem 0', fontSize: 14 }}>{so.description}</p>
          {so.diagnosis && <p style={{ margin: '0.3rem 0', fontSize: 14, fontStyle: 'italic' }}>{t('service_orders.detail_diagnosis', { value: so.diagnosis })}</p>}
        </div>
        <div style={{ textAlign: 'right' }}>
          <div style={{ fontSize: 24, fontWeight: 600 }}>R$ {Number(so.total).toFixed(2)}</div>
          <div style={{ fontSize: 13, color: 'var(--text-muted)' }}>
            {t('service_orders.parts_total_inline', { value: Number(so.partsTotal).toFixed(2) })} · {t('service_orders.labor_total_inline', { value: Number(so.laborTotal).toFixed(2) })}
          </div>
          <div style={{ marginTop: '0.4rem', display: 'inline-block', padding: '0.2rem 0.6rem', borderRadius: 4, background: 'var(--primary)', color: 'var(--text-on-primary)', fontSize: 12, fontWeight: 600 }}>{so.status}</div>
        </div>
      </header>

      {transitions.length > 0 && (
        <section style={card}>
          <h3 style={{ marginTop: 0, fontSize: 14 }}>{t('service_orders.next_transitions')}</h3>
          <div style={{ display: 'flex', gap: '0.5rem', flexWrap: 'wrap' }}>
            {transitions.map((s) => (
              <button key={s} onClick={() => changeStatus(s)} disabled={busy} style={btn}>{s}</button>
            ))}
          </div>
        </section>
      )}

      <section style={card}>
        <h3 style={{ marginTop: 0, fontSize: 14 }}>{t('service_orders.parts_count', { n: so.parts.length })}</h3>
        {so.parts.length === 0 && <p style={{ color: 'var(--text-muted)', fontSize: 13 }}>{t('service_orders.no_part')}</p>}
        {so.parts.map((p) => (
          <div key={p.id} style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', padding: '0.4rem 0', borderBottom: '1px solid var(--border)', fontSize: 14 }}>
            <span><code>{p.product.sku}</code> {p.product.name} × {Number(p.quantity)} @ R$ {Number(p.unitPrice).toFixed(2)}</span>
            <span>
              R$ {Number(p.total).toFixed(2)}
              {editable && <button onClick={() => removePart(p.id)} style={{ ...btnSmall, marginLeft: '0.5rem', background: 'var(--text-muted)' }}>×</button>}
            </span>
          </div>
        ))}
        {editable && (
          <form onSubmit={addPart} style={{ display: 'flex', gap: '0.4rem', marginTop: '0.8rem', alignItems: 'end' }}>
            <select required value={partProductId} onChange={(e) => setPartProductId(e.target.value)} style={{ ...input, flex: 2 }}>
              <option value="">{t('service_orders.select_part')}</option>
              {products.map((p) => <option key={p.id} value={p.id}>{p.sku} — {p.name}</option>)}
            </select>
            <input type="number" step="0.0001" value={partQty} onChange={(e) => setPartQty(e.target.value)} style={{ ...input, width: 90 }} />
            <button type="submit" disabled={busy} style={btn}>{t('service_orders.add_part_btn')}</button>
          </form>
        )}
      </section>

      <section style={card}>
        <h3 style={{ marginTop: 0, fontSize: 14 }}>{t('service_orders.labors_count', { n: so.labors.length })}</h3>
        {so.labors.length === 0 && <p style={{ color: 'var(--text-muted)', fontSize: 13 }}>{t('service_orders.no_part')}</p>}
        {so.labors.map((l) => (
          <div key={l.id} style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', padding: '0.4rem 0', borderBottom: '1px solid var(--border)', fontSize: 14 }}>
            <span>{l.description} × {Number(l.quantity)} @ R$ {Number(l.unitPrice).toFixed(2)}</span>
            <span>
              R$ {Number(l.total).toFixed(2)}
              {editable && <button onClick={() => removeLabor(l.id)} style={{ ...btnSmall, marginLeft: '0.5rem', background: 'var(--text-muted)' }}>×</button>}
            </span>
          </div>
        ))}
        {editable && (
          <form onSubmit={addLabor} style={{ display: 'flex', gap: '0.4rem', marginTop: '0.8rem', alignItems: 'end' }}>
            <input required value={laborDesc} onChange={(e) => setLaborDesc(e.target.value)} placeholder={t('service_orders.labor_description_placeholder')} style={{ ...input, flex: 2 }} />
            <input required type="number" step="0.01" min="0" value={laborPrice} onChange={(e) => setLaborPrice(e.target.value)} placeholder={t('service_orders.cash_placeholder')} style={{ ...input, width: 120 }} />
            <button type="submit" disabled={busy} style={btn}>{t('service_orders.add_labor_btn')}</button>
          </form>
        )}
      </section>
    </Layout>
  )
}

const card: React.CSSProperties = { background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1rem' }
const input: React.CSSProperties = { padding: '0.5rem 0.6rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const btnSmall: React.CSSProperties = { padding: '0.2rem 0.55rem', fontSize: 12, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 4, cursor: 'pointer' }
