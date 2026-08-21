import { useEffect, useMemo, useRef, useState } from 'react'
import { Link } from 'react-router-dom'
import {
  Alert,
  Box,
  Button,
  Divider,
  IconButton,
  MenuItem,
  Paper,
  Stack,
  TextField,
  Typography,
} from '@mui/material'
import { Can } from '../access/AbilityContext'
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
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 2 }}>{t('pos.title')}</Typography>

      {!activeSession && (
        <Alert severity="warning" sx={{ mb: 2 }}>
          {t('pos.no_session_alert').split('<link>')[0]}
          <Link to="/cash">{t('pos.no_session_alert').match(/<link>(.*?)<\/link>/)?.[1] ?? 'abrir um caixa'}</Link>
          {t('pos.no_session_alert').split('</link>')[1]}
        </Alert>
      )}

      {pix && (
        <Alert severity="info" sx={{ mb: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600 }}>
            {t('pos.pix_waiting')} {pix.stub && <Typography component="span" variant="caption" color="text.secondary">{t('pos.pix_stub_notice')}</Typography>}
          </Typography>
          <Typography variant="body2" sx={{ my: 1 }}>{t('pos.pix_amount_line', {
            amount: Number(pix.amount).toFixed(2),
            fee: Number(pix.feeAmount).toFixed(2),
            net: Number(pix.netAmount).toFixed(2),
          })}</Typography>
          <TextField
            multiline
            minRows={3}
            fullWidth
            size="small"
            value={pix.qrText}
            slotProps={{ htmlInput: { readOnly: true, style: { fontFamily: 'monospace', fontSize: 11 } } }}
          />
          <Typography variant="body2" color="text.secondary" sx={{ my: 1 }}>{t('pos.pix_expires_at', { time: new Date(pix.expiresAt).toLocaleTimeString() })}</Typography>
          <Stack direction="row" spacing={1}>
            {pix.stub && (
              <Button variant="contained" onClick={simulatePix} disabled={busy}>
                {busy ? '…' : t('pos.simulate_confirm')}
              </Button>
            )}
            <Button variant="outlined" color="inherit" onClick={() => setPix(null)}>{t('common.cancel')}</Button>
          </Stack>
        </Alert>
      )}

      <Box sx={{ display: 'grid', gridTemplateColumns: { xs: '1fr', md: '2fr 1fr' }, gap: 2 }}>
        <Box>
          <TextField
            inputRef={queryRef}
            autoFocus
            fullWidth
            size="small"
            placeholder={t('pos.scan_placeholder')}
            value={query}
            onChange={(e) => setQuery(e.target.value)}
          />
          <Stack spacing={0.5} sx={{ mt: 1 }}>
            {filtered.map((p) => (
              <Paper
                key={p.id}
                variant="outlined"
                onClick={() => add(p)}
                sx={{ p: 1.2, cursor: 'pointer', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}
              >
                <Typography variant="body2"><code>{p.sku}</code> — {p.name}</Typography>
                <Typography variant="body2" sx={{ fontWeight: 700 }}>R$ {Number(p.salePrice).toFixed(2)} / {p.unit.code}</Typography>
              </Paper>
            ))}
          </Stack>
        </Box>

        <Paper variant="outlined" sx={{ p: 2, position: 'sticky', top: '1rem', alignSelf: 'flex-start' }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1 }}>{t('pos.cart_count', { n: cart.length })}</Typography>
          {cart.length === 0 && <Typography variant="body2" color="text.secondary">{t('pos.cart_empty')}</Typography>}
          {cart.map((i, ix) => (
            <Box key={i.productId} sx={{ mb: 1 }}>
              <Typography variant="body2"><strong>{i.sku}</strong> {i.name}</Typography>
              <Stack direction="row" spacing={0.5} sx={{ alignItems: 'center' }}>
                <IconButton size="small" onClick={() => setQty(ix, i.quantity - 1)}>−</IconButton>
                <TextField
                  type="number"
                  size="small"
                  value={i.quantity}
                  onChange={(e) => setQty(ix, Number(e.target.value))}
                  slotProps={{ htmlInput: { step: 0.0001 } }}
                  sx={{ width: 90 }}
                />
                <IconButton size="small" onClick={() => setQty(ix, i.quantity + 1)}>+</IconButton>
                <Typography variant="body2">× R$ {i.unitPrice.toFixed(2)} = <strong>R$ {(i.unitPrice * i.quantity).toFixed(2)}</strong></Typography>
              </Stack>
            </Box>
          ))}
          <Divider sx={{ my: 1.5 }} />
          <Typography variant="h5" sx={{ fontWeight: 600, textAlign: 'right' }}>R$ {total.toFixed(2)}</Typography>
          <Can I="create" a="Order" fallback={<Typography variant="body2" color="text.secondary" sx={{ mt: 1 }}>{t('pos.no_permission')}</Typography>}>
            <TextField
              select
              label={t('pos.payment_method')}
              value={method}
              onChange={(e) => setMethod(e.target.value as Method)}
              size="small"
              fullWidth
              sx={{ mt: 1.5 }}
            >
              {METHODS.map((m) => <MenuItem key={m} value={m}>{t(`pos.method.${m}`)}</MenuItem>)}
            </TextField>
            <Button
              variant="contained"
              fullWidth
              disabled={busy || cart.length === 0 || !activeSession}
              onClick={checkout}
              sx={{ mt: 1.5, py: 1.2 }}
            >
              {busy ? '…' : t('pos.finish_sale')}
            </Button>
          </Can>
        </Paper>
      </Box>
    </Layout>
  )
}
