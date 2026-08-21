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

interface StockItem {
  id: string
  quantity: string
  reserved: string
  avgCost: string
  product: { id: string; sku: string; name: string; unit?: { code: string } }
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
      <Stack direction="row" sx={{ justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
        <Typography variant="h5" sx={{ fontWeight: 700 }}>{t('stock.title')}</Typography>
        <Can I="update" a="Stock">
          <Button variant={showForm ? 'outlined' : 'contained'} onClick={() => setShowForm(!showForm)}>
            {showForm ? t('common.cancel') : t('stock.new_btn')}
          </Button>
        </Can>
      </Stack>

      {showForm && (
        <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('stock.modal_title')}</Typography>
          <Box component="form" onSubmit={handleMovement}>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ flexWrap: 'wrap', alignItems: { sm: 'flex-start' } }}>
              <TextField select required label={t('stock.product')} value={form.productId} onChange={(e) => setForm({ ...form, productId: e.target.value })} size="small" sx={{ flex: 2, minWidth: 220 }} fullWidth>
                <MenuItem value="">{t('stock.placeholder_select')}</MenuItem>
                {products.map((p) => <MenuItem key={p.id} value={p.id}>{p.sku} — {p.name}</MenuItem>)}
              </TextField>
              <TextField select required label={t('stock.warehouse')} value={form.warehouseId} onChange={(e) => setForm({ ...form, warehouseId: e.target.value })} size="small" sx={{ flex: 1, minWidth: 160 }} fullWidth>
                <MenuItem value="">{t('stock.placeholder_select')}</MenuItem>
                {warehouses.map((w) => <MenuItem key={w.id} value={w.id}>{w.code} — {w.name}</MenuItem>)}
              </TextField>
              <TextField select label={t('stock.movement_type')} value={form.type} onChange={(e) => setForm({ ...form, type: e.target.value })} size="small" sx={{ flex: 1, minWidth: 160 }} fullWidth>
                {MOVEMENT_TYPES.map((m) => <MenuItem key={m.code} value={m.code}>{m.dir > 0 ? '↑' : '↓'} {t(`stock.movement.${m.code}`)}</MenuItem>)}
              </TextField>
              <TextField required type="number" label={t('common.quantity')} value={form.quantity} onChange={(e) => setForm({ ...form, quantity: e.target.value })} size="small" sx={{ flex: 1, minWidth: 140 }} fullWidth slotProps={{ htmlInput: { step: '0.0001', min: '0.0001' } }} />
              <TextField type="number" label={t('stock.unit_cost')} value={form.unitCost} onChange={(e) => setForm({ ...form, unitCost: e.target.value })} disabled={form.type !== 'PURCHASE_IN'} size="small" sx={{ flex: 1, minWidth: 140 }} fullWidth slotProps={{ htmlInput: { step: '0.01', min: '0' } }} />
              <TextField label={t('stock.notes')} value={form.notes} onChange={(e) => setForm({ ...form, notes: e.target.value })} size="small" sx={{ flex: 2, minWidth: 220 }} fullWidth />
              <Button type="submit" variant="contained" disabled={saving} sx={{ minWidth: 110, height: 40 }}>{saving ? '…' : t('stock.apply')}</Button>
            </Stack>
          </Box>
        </Paper>
      )}

      {!data ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <TableContainer component={Paper} variant="outlined">
          <Table size="small">
            <TableHead>
              <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                <TableCell>{t('stock.col_sku')}</TableCell>
                <TableCell>{t('stock.col_product')}</TableCell>
                <TableCell>{t('stock.col_warehouse')}</TableCell>
                <TableCell align="right">{t('stock.balance')}</TableCell>
                <TableCell align="right">{t('stock.reserved')}</TableCell>
                <TableCell align="right">{t('stock.avg_cost')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {data.items.length === 0 && (
                <TableRow><TableCell colSpan={6} sx={{ color: 'text.secondary' }}>{t('stock.empty')}</TableCell></TableRow>
              )}
              {data.items.map((s) => (
                <TableRow key={s.id} hover>
                  <TableCell><code>{s.product.sku}</code></TableCell>
                  <TableCell>{s.product.name}</TableCell>
                  <TableCell>{s.warehouse.code}</TableCell>
                  <TableCell align="right">{Number(s.quantity).toLocaleString()}{s.product.unit ? ` ${s.product.unit.code}` : ''}</TableCell>
                  <TableCell align="right">{Number(s.reserved).toLocaleString()}{s.product.unit ? ` ${s.product.unit.code}` : ''}</TableCell>
                  <TableCell align="right">R$ {Number(s.avgCost).toFixed(2)}</TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}
    </Layout>
  )
}
