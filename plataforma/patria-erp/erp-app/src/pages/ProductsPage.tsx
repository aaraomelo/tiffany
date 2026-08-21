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
  hasVariants?: boolean
  brand: Brand | null
  unit: Unit
  _count?: { variants: number }
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
      // Pré-seleciona UN — Unidade (fallback: primeira da lista) quando ainda vazio.
      setForm((f) => {
        if (f.unitId) return f
        const def = unitList.find((u) => u.code === 'UN') ?? unitList[0]
        return def ? { ...f, unitId: def.id } : f
      })
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
      const defUnit = units.find((u) => u.code === 'UN') ?? units[0]
      setForm({ sku: '', name: '', unitId: defUnit?.id ?? '', brandId: '', costPrice: '', salePrice: '', gtin: '' })
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
      <Stack direction="row" spacing={1} sx={{ alignItems: 'center', justifyContent: 'space-between', mb: 2 }}>
        <Typography variant="h5" sx={{ fontWeight: 700 }}>{t('products.title')}</Typography>
        <Can I="create" a="Product">
          <Button variant="outlined" onClick={() => setShowForm(!showForm)}>
            {showForm ? t('common.cancel') : t('products.new_btn')}
          </Button>
        </Can>
      </Stack>

      <Can I="create" a="Product">
        {showForm && (
          <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
            <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('products.new')}</Typography>
            <Box component="form" onSubmit={handleCreate}>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ alignItems: { sm: 'flex-start' }, flexWrap: 'wrap' }}>
                <TextField label={t('products.sku')} required value={form.sku} onChange={(e) => setForm({ ...form, sku: e.target.value })} size="small" sx={{ flex: 1, minWidth: 140 }} fullWidth />
                <TextField label={t('products.gtin')} value={form.gtin} onChange={(e) => setForm({ ...form, gtin: e.target.value })} size="small" sx={{ flex: 1, minWidth: 140 }} fullWidth />
                <TextField label={t('common.name')} required value={form.name} onChange={(e) => setForm({ ...form, name: e.target.value })} size="small" sx={{ flex: 2, minWidth: 200 }} fullWidth />
                <TextField select label={t('products.unit')} required value={form.unitId} onChange={(e) => setForm({ ...form, unitId: e.target.value })} size="small" sx={{ flex: 1, minWidth: 160 }} fullWidth>
                  <MenuItem value="">{t('products.select_placeholder')}</MenuItem>
                  {units.map((u) => <MenuItem key={u.id} value={u.id}>{u.code} — {u.description}</MenuItem>)}
                </TextField>
                <TextField select label={t('products.brand')} value={form.brandId} onChange={(e) => setForm({ ...form, brandId: e.target.value })} size="small" sx={{ flex: 1, minWidth: 160 }} fullWidth>
                  <MenuItem value="">{t('products.brand_none')}</MenuItem>
                  {brands.map((b) => <MenuItem key={b.id} value={b.id}>{b.name}</MenuItem>)}
                </TextField>
                <TextField label={t('products.cost_price')} type="number" slotProps={{ htmlInput: { step: '0.01', min: '0' } }} value={form.costPrice} onChange={(e) => setForm({ ...form, costPrice: e.target.value })} size="small" sx={{ flex: 1, minWidth: 140 }} fullWidth />
                <TextField label={t('products.sale_price')} type="number" slotProps={{ htmlInput: { step: '0.01', min: '0' } }} value={form.salePrice} onChange={(e) => setForm({ ...form, salePrice: e.target.value })} size="small" sx={{ flex: 1, minWidth: 140 }} fullWidth />
                <Button type="submit" variant="contained" disabled={creating} sx={{ minWidth: 110, height: 40 }}>
                  {creating ? '…' : t('common.create')}
                </Button>
              </Stack>
            </Box>
          </Paper>
        )}
      </Can>

      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} sx={{ mb: 2 }}>
        <TextField placeholder={t('products.search_placeholder')} value={q} onChange={(e) => setQ(e.target.value)} size="small" sx={{ flex: 1 }} />
        <Button variant="outlined" onClick={() => void load()}>{t('common.filter')}</Button>
      </Stack>

      {!data ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <TableContainer component={Paper} variant="outlined">
          <Table size="small">
            <TableHead>
              <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                <TableCell>{t('products.col_sku')}</TableCell>
                <TableCell>{t('products.col_name')}</TableCell>
                <TableCell>{t('products.col_brand')}</TableCell>
                <TableCell>{t('products.col_unit')}</TableCell>
                <TableCell>{t('products.col_cost')}</TableCell>
                <TableCell>{t('products.col_sale')}</TableCell>
                <TableCell align="right">{t('products.col_variants')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {data.items.length === 0 && (
                <TableRow><TableCell colSpan={7} sx={{ color: 'text.secondary' }}>{t('products.empty')}</TableCell></TableRow>
              )}
              {data.items.map((p) => (
                <TableRow key={p.id} hover>
                  <TableCell><code>{p.sku}</code></TableCell>
                  <TableCell>{p.name}</TableCell>
                  <TableCell>{p.brand?.name ?? '—'}</TableCell>
                  <TableCell>{p.unit.code}</TableCell>
                  <TableCell>R$ {Number(p.costPrice).toFixed(2)}</TableCell>
                  <TableCell>R$ {Number(p.salePrice).toFixed(2)}</TableCell>
                  <TableCell align="right">{p._count?.variants ? p._count.variants : '—'}</TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}
    </Layout>
  )
}
