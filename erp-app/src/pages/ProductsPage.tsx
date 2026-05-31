import { useEffect, useState } from 'react'
import { api } from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

interface Brand { id: string; name: string }
interface Unit { id: string; code: string; description: string }
interface Product {
  id: string
  sku: string
  name: string
  gtin: string | null
  costPrice: string
  salePrice: string
  active: boolean
  brand: Brand | null
  unit: Unit
}
interface ListResponse { items: Product[]; total: number; page: number; pageSize: number }

export function ProductsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<ListResponse | null>(null)
  const [brands, setBrands] = useState<Brand[]>([])
  const [units, setUnits] = useState<Unit[]>([])
  const [q, setQ] = useState('')
  const [showForm, setShowForm] = useState(false)
  const [creating, setCreating] = useState(false)
  const [form, setForm] = useState({
    sku: '', name: '', unitId: '', brandId: '',
    costPrice: '', salePrice: '', gtin: '',
  })

  async function load() {
    const params = new URLSearchParams()
    if (q) params.set('q', q)
    try {
      const [list, brandList, unitList] = await Promise.all([
        api<ListResponse>(`/api/products?${params.toString()}`),
        api<Brand[]>('/api/catalog/brands'),
        api<Unit[]>('/api/catalog/units'),
      ])
      setData(list); setBrands(brandList); setUnits(unitList)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!form.unitId) { snackbar.error(t('products.error_select_unit')); return }
    setCreating(true)
    try {
      await api('/api/products', {
        method: 'POST',
        body: JSON.stringify({
          sku: form.sku,
          name: form.name,
          unitId: form.unitId,
          brandId: form.brandId || undefined,
          costPrice: form.costPrice ? Number(form.costPrice) : 0,
          salePrice: form.salePrice ? Number(form.salePrice) : 0,
          gtin: form.gtin || undefined,
        }),
      })
      setForm({ sku: '', name: '', unitId: '', brandId: '', costPrice: '', salePrice: '', gtin: '' })
      setShowForm(false)
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  return (
    <Layout>
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1rem' }}>
        <h1 style={{ margin: 0 }}>{t('products.title')}</h1>
        <Can I="create" a="Product">
          <button onClick={() => setShowForm(!showForm)} style={btn}>
            {showForm ? t('common.cancel') : t('products.new_btn')}
          </button>
        </Can>
      </header>

      <Can I="create" a="Product">
        {showForm && (
          <section style={card}>
            <h2 style={{ marginTop: 0, fontSize: 16 }}>{t('products.new')}</h2>
            <form onSubmit={handleCreate} style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '0.6rem' }}>
              <Field label={t('products.sku')}><input required value={form.sku} onChange={(e) => setForm({ ...form, sku: e.target.value })} style={input} /></Field>
              <Field label={t('products.gtin')}><input value={form.gtin} onChange={(e) => setForm({ ...form, gtin: e.target.value })} style={input} /></Field>
              <Field label={t('common.name')} span={2}><input required value={form.name} onChange={(e) => setForm({ ...form, name: e.target.value })} style={input} /></Field>
              <Field label={t('products.unit')}>
                <select required value={form.unitId} onChange={(e) => setForm({ ...form, unitId: e.target.value })} style={input}>
                  <option value="">{t('products.select_placeholder')}</option>
                  {units.map((u) => <option key={u.id} value={u.id}>{u.code} — {u.description}</option>)}
                </select>
              </Field>
              <Field label={t('products.brand')}>
                <select value={form.brandId} onChange={(e) => setForm({ ...form, brandId: e.target.value })} style={input}>
                  <option value="">{t('products.brand_none')}</option>
                  {brands.map((b) => <option key={b.id} value={b.id}>{b.name}</option>)}
                </select>
              </Field>
              <Field label={t('products.cost_price')}><input type="number" step="0.01" min="0" value={form.costPrice} onChange={(e) => setForm({ ...form, costPrice: e.target.value })} style={input} /></Field>
              <Field label={t('products.sale_price')}><input type="number" step="0.01" min="0" value={form.salePrice} onChange={(e) => setForm({ ...form, salePrice: e.target.value })} style={input} /></Field>
              <div style={{ gridColumn: 'span 4', display: 'flex', justifyContent: 'flex-end' }}>
                <button type="submit" disabled={creating} style={btn}>{creating ? '…' : t('common.create')}</button>
              </div>
            </form>
          </section>
        )}
      </Can>

      <section style={{ display: 'flex', gap: '0.6rem', marginBottom: '1rem' }}>
        <input placeholder={t('products.search_placeholder')} value={q} onChange={(e) => setQ(e.target.value)} style={{ ...input, flex: 1 }} />
        <button onClick={() => void load()} style={btn}>{t('common.filter')}</button>
      </section>

      {!data && <p>{t('common.loading')}</p>}

      {data && (
        <table style={tbl}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>{t('products.col_sku')}</th>
              <th style={td}>{t('products.col_name')}</th>
              <th style={td}>{t('products.col_brand')}</th>
              <th style={td}>{t('products.col_unit')}</th>
              <th style={td}>{t('products.col_cost')}</th>
              <th style={td}>{t('products.col_sale')}</th>
            </tr>
          </thead>
          <tbody>
            {data.items.length === 0 && (
              <tr><td colSpan={6} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('products.empty')}</td></tr>
            )}
            {data.items.map((p) => (
              <tr key={p.id} style={{ borderBottom: '1px solid var(--border)' }}>
                <td style={td}><code>{p.sku}</code></td>
                <td style={td}>{p.name}</td>
                <td style={td}>{p.brand?.name ?? '—'}</td>
                <td style={td}>{p.unit.code}</td>
                <td style={td}>R$ {Number(p.costPrice).toFixed(2)}</td>
                <td style={td}>R$ {Number(p.salePrice).toFixed(2)}</td>
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
