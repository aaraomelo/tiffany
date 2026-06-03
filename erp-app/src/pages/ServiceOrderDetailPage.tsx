import { useEffect, useState } from 'react'
import { Link, useParams } from 'react-router-dom'
import {
  Box,
  Button,
  Chip,
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

type Status =
  | 'OPEN' | 'IN_PROGRESS' | 'WAITING_PARTS' | 'WAITING_CUSTOMER'
  | 'FINISHED' | 'DELIVERED' | 'CANCELLED'

interface Part {
  id: string
  productId: string
  quantity: string
  unitPrice: string
  total: string
  product: { sku: string; name: string; unit?: { code: string } }
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

  if (!so) return <Layout><Typography color="text.secondary">{t('common.loading')}</Typography></Layout>

  const transitions = ALLOWED[so.status] ?? []
  const editable = !['DELIVERED', 'CANCELLED', 'FINISHED'].includes(so.status)

  return (
    <Layout>
      <Typography sx={{ mb: 1 }}><Link to="/service-orders">{t('service_orders.back')}</Link></Typography>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', mb: 2 }}>
        <Box>
          <Typography variant="h5" sx={{ fontWeight: 700 }}>{t('nav.service_orders')} #{so.number}</Typography>
          <Typography variant="body2" color="text.secondary" sx={{ my: 0.5 }}>
            {so.customer.name}
            {so.vehicle && ` · ${so.vehicle.plate ?? '—'} ${so.vehicle.brand ?? ''} ${so.vehicle.model ?? ''}`}
            {so.warrantyTerm && ` · ${t('service_orders.detail_warranty', { name: so.warrantyTerm.name, days: so.warrantyTerm.days })}`}
          </Typography>
          <Typography variant="body2" sx={{ my: 0.5 }}>{so.description}</Typography>
          {so.diagnosis && <Typography variant="body2" sx={{ my: 0.5, fontStyle: 'italic' }}>{t('service_orders.detail_diagnosis', { value: so.diagnosis })}</Typography>}
        </Box>
        <Box sx={{ textAlign: 'right' }}>
          <Typography variant="h4" sx={{ fontWeight: 600 }}>R$ {Number(so.total).toFixed(2)}</Typography>
          <Typography variant="body2" color="text.secondary">
            {t('service_orders.parts_total_inline', { value: Number(so.partsTotal).toFixed(2) })} · {t('service_orders.labor_total_inline', { value: Number(so.laborTotal).toFixed(2) })}
          </Typography>
          <Chip label={t(`service_orders.status.${so.status}`)} color="primary" size="small" sx={{ mt: 0.5, fontWeight: 600 }} />
        </Box>
      </Box>

      {transitions.length > 0 && (
        <Can I="update" a="ServiceOrder">
          <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
            <Typography variant="subtitle2" sx={{ fontWeight: 600, mb: 1 }}>{t('service_orders.next_transitions')}</Typography>
            <Stack direction="row" spacing={1} sx={{ flexWrap: 'wrap' }}>
              {transitions.map((s) => (
                <Button key={s} variant="contained" onClick={() => changeStatus(s)} disabled={busy}>{t(`service_orders.status.${s}`)}</Button>
              ))}
            </Stack>
          </Paper>
        </Can>
      )}

      <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
        <Typography variant="subtitle2" sx={{ fontWeight: 600, mb: 1 }}>{t('service_orders.parts_count', { n: so.parts.length })}</Typography>
        {so.parts.length === 0 && <Typography variant="body2" color="text.secondary">{t('service_orders.no_part')}</Typography>}
        {so.parts.map((p) => (
          <Box key={p.id}>
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', py: 0.5 }}>
              <Typography variant="body2"><code>{p.product.sku}</code> {p.product.name} × {Number(p.quantity)}{p.product.unit ? ` ${p.product.unit.code}` : ''} @ R$ {Number(p.unitPrice).toFixed(2)}</Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 0.5 }}>
                <Typography variant="body2">R$ {Number(p.total).toFixed(2)}</Typography>
                {editable && <Can I="delete" a="ServiceOrder"><IconButton size="small" color="inherit" onClick={() => removePart(p.id)}>×</IconButton></Can>}
              </Box>
            </Box>
            <Divider />
          </Box>
        ))}
        {editable && (
          <Can I="update" a="ServiceOrder">
            <Box component="form" onSubmit={addPart} sx={{ mt: 1.5 }}>
              <Stack direction="row" spacing={0.5} sx={{ alignItems: 'flex-end' }}>
                <TextField
                  select
                  required
                  size="small"
                  value={partProductId}
                  onChange={(e) => setPartProductId(e.target.value)}
                  sx={{ flex: 2 }}
                >
                  <MenuItem value="">{t('service_orders.select_part')}</MenuItem>
                  {products.map((p) => <MenuItem key={p.id} value={p.id}>{p.sku} — {p.name}</MenuItem>)}
                </TextField>
                <TextField
                  type="number"
                  size="small"
                  value={partQty}
                  onChange={(e) => setPartQty(e.target.value)}
                  slotProps={{ htmlInput: { step: 0.0001 } }}
                  sx={{ width: 90 }}
                />
                <Button type="submit" variant="contained" disabled={busy}>{t('service_orders.add_part_btn')}</Button>
              </Stack>
            </Box>
          </Can>
        )}
      </Paper>

      <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
        <Typography variant="subtitle2" sx={{ fontWeight: 600, mb: 1 }}>{t('service_orders.labors_count', { n: so.labors.length })}</Typography>
        {so.labors.length === 0 && <Typography variant="body2" color="text.secondary">{t('service_orders.no_part')}</Typography>}
        {so.labors.map((l) => (
          <Box key={l.id}>
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', py: 0.5 }}>
              <Typography variant="body2">{l.description} × {Number(l.quantity)} @ R$ {Number(l.unitPrice).toFixed(2)}</Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 0.5 }}>
                <Typography variant="body2">R$ {Number(l.total).toFixed(2)}</Typography>
                {editable && <Can I="delete" a="ServiceOrder"><IconButton size="small" color="inherit" onClick={() => removeLabor(l.id)}>×</IconButton></Can>}
              </Box>
            </Box>
            <Divider />
          </Box>
        ))}
        {editable && (
          <Can I="update" a="ServiceOrder">
            <Box component="form" onSubmit={addLabor} sx={{ mt: 1.5 }}>
              <Stack direction="row" spacing={0.5} sx={{ alignItems: 'flex-end' }}>
                <TextField
                  required
                  size="small"
                  value={laborDesc}
                  onChange={(e) => setLaborDesc(e.target.value)}
                  placeholder={t('service_orders.labor_description_placeholder')}
                  sx={{ flex: 2 }}
                />
                <TextField
                  required
                  type="number"
                  size="small"
                  value={laborPrice}
                  onChange={(e) => setLaborPrice(e.target.value)}
                  placeholder={t('service_orders.cash_placeholder')}
                  slotProps={{ htmlInput: { step: 0.01, min: 0 } }}
                  sx={{ width: 120 }}
                />
                <Button type="submit" variant="contained" disabled={busy}>{t('service_orders.add_labor_btn')}</Button>
              </Stack>
            </Box>
          </Can>
        )}
      </Paper>
    </Layout>
  )
}
