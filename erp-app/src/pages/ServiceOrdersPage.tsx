import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
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

const STATUS_STYLE: Record<Status, object> = {
  OPEN: { color: 'primary.main' },
  IN_PROGRESS: { color: 'secondary.main', fontWeight: 600 },
  WAITING_PARTS: { color: 'warning.main' },
  WAITING_CUSTOMER: { color: 'warning.main' },
  FINISHED: { color: 'success.main', fontWeight: 600 },
  DELIVERED: { color: 'success.main' },
  CANCELLED: { color: 'text.secondary', textDecoration: 'line-through' },
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
      <Stack direction="row" spacing={1} sx={{ alignItems: 'center', justifyContent: 'space-between', mb: 2 }}>
        <Typography variant="h5" sx={{ fontWeight: 700 }}>{t('service_orders.title')}</Typography>
        <Can I="create" a="ServiceOrder">
          <Button variant="contained" onClick={() => setShowForm(!showForm)}>
            {showForm ? t('common.cancel') : t('service_orders.new_btn')}
          </Button>
        </Can>
      </Stack>

      {showForm && (
        <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('service_orders.new')}</Typography>
          <Box component="form" onSubmit={handleCreate}>
            <Stack spacing={1.5}>
              <TextField select required label={t('service_orders.field_customer')} value={form.customerId} onChange={(e) => setForm({ ...form, customerId: e.target.value })} size="small" fullWidth>
                <MenuItem value="">{t('service_orders.placeholder_select')}</MenuItem>
                {customers.map((c) => <MenuItem key={c.id} value={c.id}>{c.name}</MenuItem>)}
              </TextField>
              <TextField required label={t('service_orders.description')} value={form.description} onChange={(e) => setForm({ ...form, description: e.target.value })} placeholder={t('service_orders.placeholder_description')} size="small" fullWidth />
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                <TextField select label={t('service_orders.field_part_optional')} value={form.productId} onChange={(e) => setForm({ ...form, productId: e.target.value })} size="small" sx={{ flex: 2 }} fullWidth>
                  <MenuItem value="">{t('service_orders.no_part')}</MenuItem>
                  {products.map((p) => <MenuItem key={p.id} value={p.id}>{p.sku} — {p.name} (R$ {Number(p.salePrice).toFixed(2)})</MenuItem>)}
                </TextField>
                <TextField type="number" label={t('service_orders.field_qty')} value={form.productQty} onChange={(e) => setForm({ ...form, productQty: e.target.value })} size="small" slotProps={{ htmlInput: { step: 0.0001, min: 0.0001 } }} sx={{ flex: 1 }} fullWidth />
              </Stack>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                <TextField label={t('service_orders.field_labor_desc')} value={form.laborDesc} onChange={(e) => setForm({ ...form, laborDesc: e.target.value })} size="small" sx={{ flex: 2 }} fullWidth />
                <TextField type="number" label={t('service_orders.field_labor_value')} value={form.laborPrice} onChange={(e) => setForm({ ...form, laborPrice: e.target.value })} placeholder={t('service_orders.placeholder_labor_value')} size="small" slotProps={{ htmlInput: { step: 0.01, min: 0 } }} sx={{ flex: 1 }} fullWidth />
              </Stack>
              <Box sx={{ display: 'flex', justifyContent: 'flex-end' }}>
                <Button type="submit" variant="contained" disabled={busy}>{busy ? '…' : t('service_orders.create_btn')}</Button>
              </Box>
            </Stack>
          </Box>
        </Paper>
      )}

      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} sx={{ mb: 2 }}>
        <TextField select value={status} onChange={(e) => setStatus(e.target.value as Status | '')} size="small" sx={{ minWidth: 200 }}>
          <MenuItem value="">{t('service_orders.all_status')}</MenuItem>
          {ALL_STATUSES.map((s) => <MenuItem key={s} value={s}>{s}</MenuItem>)}
        </TextField>
      </Stack>

      {!data ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <TableContainer component={Paper} variant="outlined">
          <Table size="small">
            <TableHead>
              <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                <TableCell>#</TableCell>
                <TableCell>{t('service_orders.col_opened')}</TableCell>
                <TableCell>{t('orders.customer')}</TableCell>
                <TableCell>{t('service_orders.col_vehicle')}</TableCell>
                <TableCell>{t('service_orders.description')}</TableCell>
                <TableCell align="right">{t('common.total')}</TableCell>
                <TableCell>{t('common.status')}</TableCell>
                <TableCell></TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {data.items.length === 0 && (
                <TableRow><TableCell colSpan={8} sx={{ color: 'text.secondary' }}>{t('service_orders.empty')}</TableCell></TableRow>
              )}
              {data.items.map((s) => (
                <TableRow key={s.id} hover>
                  <TableCell><strong>{s.number}</strong></TableCell>
                  <TableCell>{new Date(s.openedAt).toLocaleDateString()}</TableCell>
                  <TableCell>{s.customer.name}</TableCell>
                  <TableCell>{s.vehicle ? `${s.vehicle.plate ?? '—'} ${s.vehicle.brand ?? ''} ${s.vehicle.model ?? ''}` : '—'}</TableCell>
                  <TableCell>{s.description}</TableCell>
                  <TableCell align="right">R$ {Number(s.total).toFixed(2)}</TableCell>
                  <TableCell sx={STATUS_STYLE[s.status]}>{s.status}</TableCell>
                  <TableCell><Button size="small" component={Link} to={`/service-orders/${s.id}`}>{t('service_orders.open')}</Button></TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}
    </Layout>
  )
}
