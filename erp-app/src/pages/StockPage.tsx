import { useEffect, useState } from 'react'
import { api } from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

interface StockItem {
  id: string
  quantity: string
  reserved: string
  avgCost: string
  product: { id: string; sku: string; name: string }
  warehouse: { id: string; code: string; name: string }
}
interface ListResponse { items: StockItem[]; total: number }
interface Warehouse { id: string; code: string; name: string }
interface ProductMini { id: string; sku: string; name: string }

const MOVEMENT_TYPES: { code: string; dir: 1 | -1 }[] = [
  { code: 'PURCHASE_IN', dir: 1 },
  { code: 'ADJUST_IN', dir: 1 },
  { code: 'RETURN_IN', dir: 1 },
  { code: 'TRANSFER_IN', dir: 1 },
  { code: 'PRODUCTION_IN', dir: 1 },
  { code: 'SALE_OUT', dir: -1 },
  { code: 'ADJUST_OUT', dir: -1 },
  { code: 'RETURN_OUT', dir: -1 },
  { code: 'TRANSFER_OUT', dir: -1 },
  { code: 'SERVICE_OUT', dir: -1 },
  { code: 'LOSS', dir: -1 },
]

export function StockPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<ListResponse | null>(null)
  const [warehouses, setWarehouses] = useState<Warehouse[]>([])
  const [products, setProducts] = useState<ProductMini[]>([])
  const [showForm, setShowForm] = useState(false)
  const [saving, setSaving] = useState(false)
  const [form, setForm] = useState({
    productId: '', warehouseId: '',
    type: 'PURCHASE_IN', quantity: '', unitCost: '', notes: '',
  })

  async function load() {
    try {
      const [list, whs, prodResp] = await Promise.all([
        api<ListResponse>('/api/stock'),
        api<Warehouse[]>('/api/warehouses'),
        api<{ items: ProductMini[] }>('/api/products?pageSize=200'),
      ])
      setData(list); setWarehouses(whs); setProducts(prodResp.items)
      if (whs.length === 1 && !form.warehouseId) {
        setForm((f) => ({ ...f, warehouseId: whs[0].id }))
      }
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleMovement(e: React.FormEvent) {
    e.preventDefault()
    setSaving(true)
    try {
      await api('/api/stock-movements', {
        method: 'POST',
        body: JSON.stringify({
          productId: form.productId,
          warehouseId: form.warehouseId,
          type: form.type,
          quantity: Number(form.quantity),
          unitCost: form.unitCost ? Number(form.unitCost) : undefined,
          notes: form.notes || undefined,
        }),
      })
      setForm({ ...form, quantity: '', unitCost: '', notes: '' })
      setShowForm(false)
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setSaving(false)
    }
  }

  return (
    <Layout>
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1rem' }}>
        <h1 style={{ margin: 0 }}>{t('stock.title')}</h1>
        <Can I="update" a="Stock">
          <button onClick={() => setShowForm(!showForm)} style={btn}>
            {showForm ? t('common.cancel') : t('stock.new_btn')}
          </button>
        </Can>
      </header>

      {showForm && (
        <section style={card}>
          <h2 style={{ marginTop: 0, fontSize: 16 }}>{t('stock.modal_title')}</h2>
          <form onSubmit={handleMovement} style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '0.6rem' }}>
            <label style={{ ...lbl, gridColumn: 'span 2' }}>
              {t('stock.product')}
              <select required value={form.productId} onChange={(e) => setForm({ ...form, productId: e.target.value })} style={input}>
                <option value="">{t('stock.placeholder_select')}</option>
                {products.map((p) => <option key={p.id} value={p.id}>{p.sku} — {p.name}</option>)}
              </select>
            </label>
            <label style={lbl}>
              {t('stock.warehouse')}
              <select required value={form.warehouseId} onChange={(e) => setForm({ ...form, warehouseId: e.target.value })} style={input}>
                <option value="">{t('stock.placeholder_select')}</option>
                {warehouses.map((w) => <option key={w.id} value={w.id}>{w.code} — {w.name}</option>)}
              </select>
            </label>
            <label style={lbl}>
              {t('stock.movement_type')}
              <select value={form.type} onChange={(e) => setForm({ ...form, type: e.target.value })} style={input}>
                {MOVEMENT_TYPES.map((m) => <option key={m.code} value={m.code}>{m.dir > 0 ? '↑' : '↓'} {t(`stock.movement.${m.code}`)}</option>)}
              </select>
            </label>
            <label style={lbl}>
              {t('common.quantity')}
              <input required type="number" step="0.0001" min="0.0001" value={form.quantity} onChange={(e) => setForm({ ...form, quantity: e.target.value })} style={input} />
            </label>
            <label style={lbl}>
              {t('stock.unit_cost')}
              <input type="number" step="0.01" min="0" value={form.unitCost} onChange={(e) => setForm({ ...form, unitCost: e.target.value })} style={input} disabled={form.type !== 'PURCHASE_IN'} />
            </label>
            <label style={{ ...lbl, gridColumn: 'span 2' }}>
              {t('stock.notes')}
              <input value={form.notes} onChange={(e) => setForm({ ...form, notes: e.target.value })} style={input} />
            </label>
            <div style={{ gridColumn: 'span 4', display: 'flex', justifyContent: 'flex-end' }}>
              <button type="submit" disabled={saving} style={btn}>{saving ? '…' : t('stock.apply')}</button>
            </div>
          </form>
        </section>
      )}

      {!data && <p>{t('common.loading')}</p>}

      {data && (
        <table style={tbl}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>{t('stock.col_sku')}</th>
              <th style={td}>{t('stock.col_product')}</th>
              <th style={td}>{t('stock.col_warehouse')}</th>
              <th style={{ ...td, textAlign: 'right' }}>{t('stock.balance')}</th>
              <th style={{ ...td, textAlign: 'right' }}>{t('stock.reserved')}</th>
              <th style={{ ...td, textAlign: 'right' }}>{t('stock.avg_cost')}</th>
            </tr>
          </thead>
          <tbody>
            {data.items.length === 0 && (
              <tr><td colSpan={6} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('stock.empty')}</td></tr>
            )}
            {data.items.map((s) => (
              <tr key={s.id} style={{ borderBottom: '1px solid var(--border)' }}>
                <td style={td}><code>{s.product.sku}</code></td>
                <td style={td}>{s.product.name}</td>
                <td style={td}>{s.warehouse.code}</td>
                <td style={{ ...td, textAlign: 'right' }}>{Number(s.quantity).toLocaleString()}</td>
                <td style={{ ...td, textAlign: 'right' }}>{Number(s.reserved).toLocaleString()}</td>
                <td style={{ ...td, textAlign: 'right' }}>R$ {Number(s.avgCost).toFixed(2)}</td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </Layout>
  )
}

const card: React.CSSProperties = { background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1.5rem' }
const lbl: React.CSSProperties = { display: 'grid', gap: 4, fontSize: 13 }
const input: React.CSSProperties = { padding: '0.5rem 0.6rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const tbl: React.CSSProperties = { width: '100%', borderCollapse: 'collapse', fontSize: 14, background: 'var(--surface)', borderRadius: 6, overflow: 'hidden', border: '1px solid var(--border)' }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
