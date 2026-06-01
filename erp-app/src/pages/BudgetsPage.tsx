import { useEffect, useState } from 'react'
import {
  Box,
  Button,
  MenuItem,
  Paper,
  Stack,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  TextField,
  Typography,
} from '@mui/material'
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

const STATUS_STYLE: Record<BudgetStatus, object> = {
  DRAFT: { color: 'text.secondary' },
  SENT: { color: 'primary.main' },
  APPROVED: { color: 'success.main', fontWeight: 600 },
  REJECTED: { color: 'error.main' },
  EXPIRED: { color: 'text.secondary' },
  CONVERTED: { color: 'success.main', fontWeight: 600 },
  CANCELLED: { color: 'text.secondary', textDecoration: 'line-through' },
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
      <Stack direction="row" spacing={1} sx={{ alignItems: 'center', justifyContent: 'space-between', mb: 2 }}>
        <Typography variant="h5" sx={{ fontWeight: 700 }}>{t('budgets.title')}</Typography>
        <Can I="create" a="Budget">
          <Button variant="contained" onClick={() => setShowForm(!showForm)}>{showForm ? t('common.cancel') : t('budgets.new_btn')}</Button>
        </Can>
      </Stack>

      {showForm && (
        <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('budgets.new')}</Typography>
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ mb: 2 }}>
            <TextField select label={t('orders.customer')} value={customerId} onChange={(e) => setCustomerId(e.target.value)} size="small" sx={{ flex: 1 }} fullWidth>
              <MenuItem value="">{t('budgets.form_customer_none')}</MenuItem>
              {customers.map((c) => <MenuItem key={c.id} value={c.id}>{c.name}</MenuItem>)}
            </TextField>
            <TextField select label={t('customers.type')} value={target} onChange={(e) => setTarget(e.target.value as BudgetTarget)} size="small" sx={{ flex: 1 }} fullWidth>
              <MenuItem value="SERVICE_ORDER">{t('budgets.target_so_label')}</MenuItem>
              <MenuItem value="ORDER">{t('budgets.target_order_label')}</MenuItem>
            </TextField>
          </Stack>

          <Typography variant="subtitle2" sx={{ mb: 1 }}>{t('budgets.items_section')}</Typography>
          <Box component="form" onSubmit={addItem} sx={{ mb: 1.5 }}>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} sx={{ alignItems: { sm: 'flex-start' } }}>
              <TextField select label={t('budgets.type_product')} value={draft.isLabor ? '' : draft.productId ?? ''} onChange={(e) => {
                if (e.target.value === '') {
                  setDraft({ ...draft, isLabor: true, productId: undefined, description: '' })
                } else {
                  applyProduct(e.target.value)
                }
              }} size="small" sx={{ flex: 2, minWidth: 160 }} fullWidth>
                <MenuItem value="">{t('budgets.labor_option')}</MenuItem>
                {products.map((p) => <MenuItem key={p.id} value={p.id}>{p.sku} — {p.name}</MenuItem>)}
              </TextField>
              <TextField label={t('budgets.placeholder_description')} value={draft.description} onChange={(e) => setDraft({ ...draft, description: e.target.value })} size="small" sx={{ flex: 2 }} fullWidth />
              <TextField type="number" label={t('budgets.placeholder_qty')} value={draft.quantity} onChange={(e) => setDraft({ ...draft, quantity: Number(e.target.value) })} size="small" slotProps={{ htmlInput: { step: 0.0001 } }} sx={{ flex: 1 }} fullWidth />
              <TextField type="number" label={t('budgets.placeholder_amount')} value={draft.unitPrice} onChange={(e) => setDraft({ ...draft, unitPrice: Number(e.target.value) })} size="small" slotProps={{ htmlInput: { step: 0.01 } }} sx={{ flex: 1 }} fullWidth />
              <Button type="submit" variant="contained" sx={{ minWidth: 110, height: 40 }}>{t('budgets.add_item_btn')}</Button>
            </Stack>
          </Box>

          {items.length > 0 && (
            <Stack spacing={0.5} sx={{ mb: 1.5 }}>
              {items.map((i, ix) => (
                <Stack key={ix} direction="row" spacing={1} sx={{ alignItems: 'center', justifyContent: 'space-between', py: 0.5, borderBottom: '1px solid', borderColor: 'divider' }}>
                  <Typography variant="body2">{i.isLabor ? '🛠️' : '📦'} {i.description} × {i.quantity} @ R$ {i.unitPrice.toFixed(2)}</Typography>
                  <Stack direction="row" spacing={1} sx={{ alignItems: 'center' }}>
                    <Typography variant="body2">R$ {(i.quantity * i.unitPrice).toFixed(2)}</Typography>
                    <Button size="small" variant="text" color="error" onClick={() => setItems(items.filter((_, j) => j !== ix))}>×</Button>
                  </Stack>
                </Stack>
              ))}
            </Stack>
          )}
          <Typography variant="subtitle1" sx={{ textAlign: 'right', fontWeight: 600, mt: 1 }}>{t('common.total')}: R$ {total.toFixed(2)}</Typography>
          <Box sx={{ display: 'flex', justifyContent: 'flex-end', mt: 1 }}>
            <Button variant="contained" onClick={handleCreate} disabled={busy || items.length === 0}>{busy ? '…' : t('budgets.save_btn')}</Button>
          </Box>
        </Paper>
      )}

      <TableContainer component={Paper} variant="outlined">
        <Table size="small">
          <TableHead>
            <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
              <TableCell>#</TableCell>
              <TableCell>{t('budgets.col_date')}</TableCell>
              <TableCell>{t('budgets.col_customer')}</TableCell>
              <TableCell>{t('budgets.col_type')}</TableCell>
              <TableCell align="right">{t('common.total')}</TableCell>
              <TableCell>{t('common.status')}</TableCell>
              <TableCell>{t('common.actions')}</TableCell>
            </TableRow>
          </TableHead>
          <TableBody>
            {data.length === 0 && (
              <TableRow><TableCell colSpan={7} sx={{ color: 'text.secondary' }}>{t('budgets.empty')}</TableCell></TableRow>
            )}
            {data.map((b) => (
              <TableRow key={b.id} hover>
                <TableCell><strong>{b.number}</strong></TableCell>
                <TableCell>{new Date(b.createdAt).toLocaleDateString()}</TableCell>
                <TableCell>{b.customer?.name ?? '—'}</TableCell>
                <TableCell>{t(`budgets.target.${b.target}`)}</TableCell>
                <TableCell align="right">R$ {Number(b.total).toFixed(2)}</TableCell>
                <TableCell sx={STATUS_STYLE[b.status]}>{t(`budgets.status.${b.status}`)}</TableCell>
                <TableCell>
                  <Can I="update" a="Budget">
                    <Stack direction="row" spacing={0.5} sx={{ flexWrap: 'wrap' }}>
                      {ALLOWED[b.status].map((s) => (
                        <Button key={s} size="small" variant="text" onClick={() => setStatus(b.id, s)}>{t(`budgets.status.${s}`)}</Button>
                      ))}
                      {b.status === 'APPROVED' && (
                        <Button size="small" variant="text" color="success" onClick={() => convert(b.id)}>{t('budgets.convert')}</Button>
                      )}
                    </Stack>
                  </Can>
                </TableCell>
              </TableRow>
            ))}
          </TableBody>
        </Table>
      </TableContainer>
    </Layout>
  )
}
