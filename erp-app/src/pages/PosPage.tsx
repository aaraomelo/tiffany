import { useEffect, useMemo, useRef, useState } from 'react'
import { Link } from 'react-router-dom'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { getActiveSession } from './CashPage'

interface ProductMini {
  id: string
  sku: string
  name: string
  salePrice: string
  unit: { code: string }
}
interface CartItem {
  productId: string
  sku: string
  name: string
  unit: string
  unitPrice: number
  quantity: number
}
interface OrderResponse {
  id: string
  number: number
  total: string
  status: string
}
interface PixResponse {
  paymentId: string
  stub: boolean
  qrText: string
  amount: string
  feeAmount: string
  netAmount: string
  expiresAt: string
}

type Method = 'CASH' | 'PIX' | 'CREDIT_CARD' | 'DEBIT_CARD' | 'CREDIT_NOTE'

const METHODS: Method[] = ['CASH', 'PIX', 'CREDIT_CARD', 'DEBIT_CARD', 'CREDIT_NOTE']

export function PosPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [products, setProducts] = useState<ProductMini[]>([])
  const [query, setQuery] = useState('')
  const [cart, setCart] = useState<CartItem[]>([])
  const [method, setMethod] = useState<Method>('CASH')
  const [order, setOrder] = useState<OrderResponse | null>(null)
  const [pix, setPix] = useState<PixResponse | null>(null)
  const [busy, setBusy] = useState(false)
  const queryRef = useRef<HTMLInputElement>(null)
  const activeSession = getActiveSession()

  useEffect(() => {
    api<{ items: ProductMini[] }>('/api/products?pageSize=500')
      .then((r) => setProducts(r.items))
      .catch((e) => snackbar.error((e as Error).message))
  }, [])

  const total = useMemo(
    () => cart.reduce((acc, i) => acc + i.unitPrice * i.quantity, 0),
    [cart],
  )

  const filtered = useMemo(() => {
    if (!query) return products.slice(0, 8)
    const q = query.toLowerCase()
    return products.filter((p) =>
      p.sku.toLowerCase().includes(q) || p.name.toLowerCase().includes(q),
    ).slice(0, 8)
  }, [products, query])

  function add(p: ProductMini) {
    setCart((c) => {
      const ix = c.findIndex((i) => i.productId === p.id)
      if (ix >= 0) {
        const next = [...c]
        next[ix] = { ...next[ix], quantity: next[ix].quantity + 1 }
        return next
      }
      return [
        ...c,
        { productId: p.id, sku: p.sku, name: p.name, unit: p.unit.code, unitPrice: Number(p.salePrice), quantity: 1 },
      ]
    })
    setQuery('')
    queryRef.current?.focus()
  }

  function setQty(ix: number, qty: number) {
    if (qty <= 0) setCart((c) => c.filter((_, i) => i !== ix))
    else setCart((c) => c.map((it, i) => i === ix ? { ...it, quantity: qty } : it))
  }

  function reset() {
    setCart([]); setOrder(null); setPix(null)
  }

  async function checkout() {
    if (!activeSession) { snackbar.error(t('pos.error_no_session')); return }
    if (cart.length === 0) return
    setBusy(true)
    try {
      const created = await api<OrderResponse>('/api/orders', {
        method: 'POST',
        body: JSON.stringify({
          cashSessionId: activeSession.sessionId,
          items: cart.map((i) => ({
            productId: i.productId,
            quantity: i.quantity,
            unitPrice: i.unitPrice,
          })),
        }),
      })
      setOrder(created)
      const amount = Number(created.total)
      if (method === 'PIX') {
        const px = await api<PixResponse>('/api/checkout/pix', {
          method: 'POST',
          body: JSON.stringify({ orderId: created.id, amount, description: `Pedido ${created.number}` }),
        })
        setPix(px)
      } else {
        await api('/api/payments', {
          method: 'POST',
          body: JSON.stringify({ orderId: created.id, method, amount }),
        })
        await api(`/api/orders/${created.id}/fulfill`, { method: 'POST' })
        snackbar.success(t('pos.sale_complete', { number: created.number, amount: amount.toFixed(2) }))
        reset()
      }
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  async function simulatePix() {
    if (!pix || !order) return
    setBusy(true)
    try {
      await api(`/api/checkout/simulate-confirm/${pix.paymentId}`, { method: 'POST' })
      await api(`/api/orders/${order.id}/fulfill`, { method: 'POST' })
      snackbar.success(t('pos.pix_confirmed', {
        number: order.number,
        amount: Number(pix.amount).toFixed(2),
        net: Number(pix.netAmount).toFixed(2),
      }))
      reset()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('pos.title')}</h1>

      {!activeSession && (
        <div style={{ background: '#fdf6e3', border: '1px solid var(--warn)', padding: '1rem', borderRadius: 6, marginBottom: '1rem' }}>
          {t('pos.no_session_alert').split('<link>')[0]}
          <Link to="/cash">{t('pos.no_session_alert').match(/<link>(.*?)<\/link>/)?.[1] ?? 'abrir um caixa'}</Link>
          {t('pos.no_session_alert').split('</link>')[1]}
        </div>
      )}

      {pix && (
        <section style={{ padding: '1rem', borderRadius: 8, marginBottom: '1.5rem', background: '#fff3e0', border: '1px solid #ff9800' }}>
          <h2 style={{ marginTop: 0, fontSize: 16 }}>
            {t('pos.pix_waiting')} {pix.stub && <span style={{ fontSize: 12, color: 'var(--text-muted)' }}>{t('pos.pix_stub_notice')}</span>}
          </h2>
          <p>{t('pos.pix_amount_line', {
            amount: Number(pix.amount).toFixed(2),
            fee: Number(pix.feeAmount).toFixed(2),
            net: Number(pix.netAmount).toFixed(2),
          })}</p>
          <textarea
            readOnly
            value={pix.qrText}
            style={{ width: '100%', height: 80, fontFamily: 'monospace', fontSize: 11, padding: '0.5rem', resize: 'none' }}
          />
          <p style={{ fontSize: 13, color: 'var(--text-muted)' }}>{t('pos.pix_expires_at', { time: new Date(pix.expiresAt).toLocaleTimeString() })}</p>
          <div style={{ display: 'flex', gap: '0.5rem' }}>
            {pix.stub && (
              <button onClick={simulatePix} disabled={busy} style={btn}>
                {busy ? '…' : t('pos.simulate_confirm')}
              </button>
            )}
            <button onClick={() => setPix(null)} style={{ ...btn, background: 'var(--text-muted)' }}>{t('common.cancel')}</button>
          </div>
        </section>
      )}

      <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: '1.5rem' }}>
        <section>
          <input
            ref={queryRef}
            autoFocus
            placeholder={t('pos.scan_placeholder')}
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            style={{ ...input, fontSize: 16, padding: '0.7rem', width: '100%' }}
          />
          <ul style={{ listStyle: 'none', padding: 0, marginTop: '0.5rem' }}>
            {filtered.map((p) => (
              <li key={p.id} style={{ border: '1px solid var(--border)', borderRadius: 6, padding: '0.6rem 0.8rem', marginBottom: '0.3rem', cursor: 'pointer', background: 'var(--surface)', display: 'flex', justifyContent: 'space-between' }} onClick={() => add(p)}>
                <span><code>{p.sku}</code> — {p.name}</span>
                <strong>R$ {Number(p.salePrice).toFixed(2)} / {p.unit.code}</strong>
              </li>
            ))}
          </ul>
        </section>

        <aside style={{ background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, position: 'sticky', top: '1rem', alignSelf: 'flex-start' }}>
          <h2 style={{ marginTop: 0, fontSize: 16 }}>{t('pos.cart_count', { n: cart.length })}</h2>
          {cart.length === 0 && <p style={{ color: 'var(--text-muted)' }}>{t('pos.cart_empty')}</p>}
          {cart.map((i, ix) => (
            <div key={i.productId} style={{ marginBottom: '0.6rem', fontSize: 14 }}>
              <div><strong>{i.sku}</strong> {i.name}</div>
              <div style={{ display: 'flex', gap: '0.3rem', alignItems: 'center' }}>
                <button onClick={() => setQty(ix, i.quantity - 1)} style={qtyBtn}>−</button>
                <input
                  type="number"
                  step="0.0001"
                  value={i.quantity}
                  onChange={(e) => setQty(ix, Number(e.target.value))}
                  style={{ width: 70, ...input, padding: '0.3rem' }}
                />
                <button onClick={() => setQty(ix, i.quantity + 1)} style={qtyBtn}>+</button>
                <span>× R$ {i.unitPrice.toFixed(2)} = <strong>R$ {(i.unitPrice * i.quantity).toFixed(2)}</strong></span>
              </div>
            </div>
          ))}
          <hr style={{ border: 'none', borderTop: '1px solid var(--border)', margin: '0.8rem 0' }} />
          <div style={{ fontSize: 22, fontWeight: 600, textAlign: 'right' }}>R$ {total.toFixed(2)}</div>
          <label style={{ ...lbl, marginTop: '0.8rem' }}>
            {t('pos.payment_method')}
            <select value={method} onChange={(e) => setMethod(e.target.value as Method)} style={input}>
              {METHODS.map((m) => <option key={m} value={m}>{t(`pos.method.${m}`)}</option>)}
            </select>
          </label>
          <button
            disabled={busy || cart.length === 0 || !activeSession}
            onClick={checkout}
            style={{ ...btn, width: '100%', marginTop: '0.8rem', padding: '0.8rem', fontSize: 16 }}
          >
            {busy ? '…' : t('pos.finish_sale')}
          </button>
        </aside>
      </div>
    </Layout>
  )
}

const lbl: React.CSSProperties = { display: 'grid', gap: 4, fontSize: 13 }
const input: React.CSSProperties = { padding: '0.5rem 0.6rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const qtyBtn: React.CSSProperties = { padding: '0.2rem 0.55rem', background: 'var(--border)', border: 'none', borderRadius: 4, cursor: 'pointer' }
