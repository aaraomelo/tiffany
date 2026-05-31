import { useEffect, useState } from 'react'
import { api } from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

type BudgetStatus = 'DRAFT' | 'SENT' | 'APPROVED' | 'REJECTED' | 'EXPIRED' | 'CONVERTED' | 'CANCELLED'
type BudgetTarget = 'ORDER' | 'SERVICE_ORDER'

interface BudgetRow {
  id: string
  number: number
  status: BudgetStatus
  target: BudgetTarget
  total: string
  createdAt: string
  customer: { id: string; name: string } | null
}
interface Customer { id: string; name: string }
interface ProductMini { id: string; sku: string; name: string; salePrice: string }

interface BudgetItem {
  productId?: string
  description: string
  quantity: number
  unitPrice: number
  isLabor: boolean
}

const ALLOWED: Record<BudgetStatus, BudgetStatus[]> = {
  DRAFT: ['SENT', 'CANCELLED'],
  SENT: ['APPROVED', 'REJECTED', 'EXPIRED', 'CANCELLED'],
  APPROVED: ['CONVERTED', 'CANCELLED'],
  REJECTED: [], EXPIRED: [], CONVERTED: [], CANCELLED: [],
}

const STATUS_STYLE: Record<BudgetStatus, React.CSSProperties> = {
  DRAFT: { color: 'var(--text-muted)' },
  SENT: { color: 'var(--primary)' },
  APPROVED: { color: 'var(--success)', fontWeight: 600 },
  REJECTED: { color: 'var(--danger)' },
  EXPIRED: { color: 'var(--text-muted)' },
  CONVERTED: { color: 'var(--success)', fontWeight: 600 },
  CANCELLED: { color: 'var(--text-muted)', textDecoration: 'line-through' },
}

export function BudgetsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<BudgetRow[]>([])
  const [showForm, setShowForm] = useState(false)
  const [customers, setCustomers] = useState<Customer[]>([])
  const [products, setProducts] = useState<ProductMini[]>([])
  const [target, setTarget] = useState<BudgetTarget>('SERVICE_ORDER')
  const [customerId, setCustomerId] = useState('')
  const [items, setItems] = useState<BudgetItem[]>([])
  const [draft, setDraft] = useState<BudgetItem>({ description: '', quantity: 1, unitPrice: 0, isLabor: true })
  const [busy, setBusy] = useState(false)

  async function load() {
    setData(await api<BudgetRow[]>('/api/budgets'))
  }
  useEffect(() => { void load() }, [])
  useEffect(() => {
    if (showForm) {
      Promise.all([
        api<{ items: Customer[] }>('/api/customer-suppliers?role=CUSTOMER&pageSize=200'),
        api<{ items: ProductMini[] }>('/api/products?pageSize=200'),
      ]).then(([cs, ps]) => {
        setCustomers(cs.items); setProducts(ps.items)
      })
    }
  }, [showForm])

  function addItem(e: React.FormEvent) {
    e.preventDefault()
    if (!draft.description || draft.unitPrice <= 0 || draft.quantity <= 0) return
    setItems([...items, draft])
    setDraft({ description: '', quantity: 1, unitPrice: 0, isLabor: true })
  }

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (items.length === 0) return
    setBusy(true)
    try {
      await api('/api/budgets', {
        method: 'POST',
        body: JSON.stringify({
          customerId: customerId || undefined,
          target,
          items: items.map((i) => ({
            productId: i.productId,
            description: i.description,
            quantity: i.quantity,
            unitPrice: i.unitPrice,
            isLabor: i.isLabor,
          })),
        }),
      })
      setShowForm(false)
      setCustomerId(''); setItems([])
      void load()
    } catch (e) { snackbar.error((e as Error).message) } finally { setBusy(false) }
  }

  async function setStatus(id: string, status: BudgetStatus) {
    try {
      await api(`/api/budgets/${id}/status`, {
        method: 'POST',
        body: JSON.stringify({ status }),
      })
      void load()
    } catch (e) { snackbar.error((e as Error).message) }
  }

  async function convert(id: string) {
    try {
      const r = await api<{ type: string; number: number }>(`/api/budgets/${id}/convert`, {
        method: 'POST',
        body: JSON.stringify({}),
      })
      snackbar.success(t('budgets.converted_alert', { type: r.type, number: r.number }))
      void load()
    } catch (e) { snackbar.error((e as Error).message) }
  }

  function applyProduct(productId: string) {
    const p = products.find((x) => x.id === productId)
    if (!p) return
    setDraft({
      productId: p.id,
      description: `${p.sku} — ${p.name}`,
      quantity: 1,
      unitPrice: Number(p.salePrice),
      isLabor: false,
    })
  }

  const total = items.reduce((acc, i) => acc + i.unitPrice * i.quantity, 0)

  return (
    <Layout>
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1rem' }}>
        <h1 style={{ margin: 0 }}>{t('budgets.title')}</h1>
        <Can I="create" a="Budget">
          <button onClick={() => setShowForm(!showForm)} style={btn}>{showForm ? t('common.cancel') : t('budgets.new_btn')}</button>
        </Can>
      </header>

      {showForm && (
        <section style={card}>
          <h2 style={{ marginTop: 0, fontSize: 16 }}>{t('budgets.new')}</h2>
          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '0.6rem', marginBottom: '1rem' }}>
            <label style={lbl}>
              {t('orders.customer')}
              <select value={customerId} onChange={(e) => setCustomerId(e.target.value)} style={input}>
                <option value="">{t('budgets.form_customer_none')}</option>
                {customers.map((c) => <option key={c.id} value={c.id}>{c.name}</option>)}
              </select>
            </label>
            <label style={lbl}>
              {t('customers.type')}
              <select value={target} onChange={(e) => setTarget(e.target.value as BudgetTarget)} style={input}>
                <option value="SERVICE_ORDER">{t('budgets.target_so_label')}</option>
                <option value="ORDER">{t('budgets.target_order_label')}</option>
              </select>
            </label>
          </div>

          <h3 style={{ fontSize: 14 }}>{t('budgets.items_section')}</h3>
          <form onSubmit={addItem} style={{ display: 'grid', gridTemplateColumns: '2fr 1fr 1fr auto auto', gap: '0.4rem', alignItems: 'end', marginBottom: '0.8rem' }}>
            <div style={lbl}>
              {t('budgets.type_product')}
              <select value={draft.isLabor ? '' : draft.productId ?? ''} onChange={(e) => {
                if (e.target.value === '') {
                  setDraft({ ...draft, isLabor: true, productId: undefined, description: '' })
                } else {
                  applyProduct(e.target.value)
                }
              }} style={input}>
                <option value="">{t('budgets.labor_option')}</option>
                {products.map((p) => <option key={p.id} value={p.id}>{p.sku} — {p.name}</option>)}
              </select>
            </div>
            <input placeholder={t('budgets.placeholder_description')} value={draft.description} onChange={(e) => setDraft({ ...draft, description: e.target.value })} style={input} />
            <input type="number" step="0.0001" placeholder={t('budgets.placeholder_qty')} value={draft.quantity} onChange={(e) => setDraft({ ...draft, quantity: Number(e.target.value) })} style={input} />
            <input type="number" step="0.01" placeholder={t('budgets.placeholder_amount')} value={draft.unitPrice} onChange={(e) => setDraft({ ...draft, unitPrice: Number(e.target.value) })} style={input} />
            <button type="submit" style={btn}>{t('budgets.add_item_btn')}</button>
          </form>

          {items.length > 0 && (
            <ul style={{ listStyle: 'none', padding: 0, fontSize: 14 }}>
              {items.map((i, ix) => (
                <li key={ix} style={{ display: 'flex', justifyContent: 'space-between', padding: '0.3rem 0', borderBottom: '1px solid var(--border)' }}>
                  <span>{i.isLabor ? '🛠️' : '📦'} {i.description} × {i.quantity} @ R$ {i.unitPrice.toFixed(2)}</span>
                  <span>
                    R$ {(i.quantity * i.unitPrice).toFixed(2)}
                    <button onClick={() => setItems(items.filter((_, j) => j !== ix))} style={{ ...btnSmall, marginLeft: '0.5rem', background: 'var(--text-muted)' }}>×</button>
                  </span>
                </li>
              ))}
            </ul>
          )}
          <div style={{ textAlign: 'right', fontSize: 18, fontWeight: 600, marginTop: '0.8rem' }}>{t('common.total')}: R$ {total.toFixed(2)}</div>
          <div style={{ display: 'flex', justifyContent: 'flex-end', marginTop: '0.8rem' }}>
            <button onClick={handleCreate} disabled={busy || items.length === 0} style={btn}>{busy ? '…' : t('budgets.save_btn')}</button>
          </div>
        </section>
      )}

      <table style={tbl}>
        <thead>
          <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
            <th style={td}>#</th>
            <th style={td}>{t('budgets.col_date')}</th>
            <th style={td}>{t('budgets.col_customer')}</th>
            <th style={td}>{t('budgets.col_type')}</th>
            <th style={{ ...td, textAlign: 'right' }}>{t('common.total')}</th>
            <th style={td}>{t('common.status')}</th>
            <th style={td}>{t('common.actions')}</th>
          </tr>
        </thead>
        <tbody>
          {data.length === 0 && <tr><td colSpan={7} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('budgets.empty')}</td></tr>}
          {data.map((b) => (
            <tr key={b.id} style={{ borderBottom: '1px solid var(--border)' }}>
              <td style={td}><strong>{b.number}</strong></td>
              <td style={td}>{new Date(b.createdAt).toLocaleDateString()}</td>
              <td style={td}>{b.customer?.name ?? '—'}</td>
              <td style={td}>{t(`budgets.target.${b.target}`)}</td>
              <td style={{ ...td, textAlign: 'right' }}>R$ {Number(b.total).toFixed(2)}</td>
              <td style={{ ...td, ...STATUS_STYLE[b.status] }}>{t(`budgets.status.${b.status}`)}</td>
              <td style={td}>
                <Can I="update" a="Budget">
                  <div style={{ display: 'flex', gap: '0.3rem', flexWrap: 'wrap' }}>
                    {ALLOWED[b.status].map((s) => (
                      <button key={s} onClick={() => setStatus(b.id, s)} style={btnSmall}>{t(`budgets.status.${s}`)}</button>
                    ))}
                    {b.status === 'APPROVED' && (
                      <button onClick={() => convert(b.id)} style={{ ...btnSmall, background: 'var(--success)' }}>{t('budgets.convert')}</button>
                    )}
                  </div>
                </Can>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </Layout>
  )
}

const card: React.CSSProperties = { background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1.5rem' }
const lbl: React.CSSProperties = { display: 'grid', gap: 4, fontSize: 13 }
const input: React.CSSProperties = { padding: '0.5rem 0.6rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const btnSmall: React.CSSProperties = { padding: '0.25rem 0.55rem', fontSize: 12, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 4, cursor: 'pointer' }
const tbl: React.CSSProperties = { width: '100%', borderCollapse: 'collapse', fontSize: 14, background: 'var(--surface)', borderRadius: 6, overflow: 'hidden', border: '1px solid var(--border)' }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
