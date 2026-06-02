import { useEffect, useState } from 'react'
import {
  Alert,
  Box,
  Button,
  Checkbox,
  Chip,
  FormControlLabel,
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

interface SupplierAccount {
  id: string
  baseUrl: string
  label: string | null
  loginEmail: string
  hasSession: boolean
  defaultMarkupPct: string | null
  active: boolean
  syncing?: boolean
  lastSyncAt: string | null
  lastError: string | null
  productCount?: number
}

interface SupplierProduct {
  id: string
  name: string
  sku: string | null
  option0: string | null
  option1: string | null
  price: string
  pixPrice: string | null
  stock: number | null
  available: boolean
}

interface ProductsResponse {
  items: SupplierProduct[]
  total: number
  page: number
  pageSize: number
}

export function SuppliersPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [accounts, setAccounts] = useState<SupplierAccount[] | null>(null)
  const [busyId, setBusyId] = useState<string | null>(null)
  const [stub, setStub] = useState(false)

  // formulário de nova conta
  const [baseUrl, setBaseUrl] = useState('')
  const [loginEmail, setLoginEmail] = useState('')
  const [password, setPassword] = useState('')
  const [label, setLabel] = useState('')
  const [markup, setMarkup] = useState('')
  const [consent, setConsent] = useState(false)
  const [creating, setCreating] = useState(false)

  // produtos do fornecedor selecionado
  const [selected, setSelected] = useState<string | null>(null)
  const [products, setProducts] = useState<ProductsResponse | null>(null)
  const [q, setQ] = useState('')

  async function loadAccounts() {
    try {
      setAccounts(await api<SupplierAccount[]>('/api/suppliers/accounts'))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void loadAccounts() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!baseUrl.trim() || !loginEmail.trim() || !password) return
    if (!consent) { snackbar.error(t('suppliers.consent')); return }
    setCreating(true)
    try {
      await api('/api/suppliers/accounts', {
        method: 'POST',
        body: JSON.stringify({
          baseUrl,
          loginEmail,
          password,
          label: label || undefined,
          defaultMarkupPct: markup ? Number(markup) : undefined,
          consent,
        }),
      })
      setBaseUrl(''); setLoginEmail(''); setPassword(''); setLabel(''); setMarkup(''); setConsent(false)
      void loadAccounts()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  async function testLogin(id: string) {
    setBusyId(id)
    try {
      const res = await api<{ ok: boolean; message: string; stub?: boolean }>(
        `/api/suppliers/accounts/${id}/test-login`,
        { method: 'POST' },
      )
      if (res.stub) setStub(true)
      if (res.ok) snackbar.success(t('suppliers.login_ok'))
      else snackbar.error(`${t('suppliers.login_fail')}: ${res.message}`)
      void loadAccounts()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusyId(null)
    }
  }

  async function sync(id: string) {
    setBusyId(id)
    try {
      const res = await api<{ started: boolean; message: string }>(
        `/api/suppliers/accounts/${id}/sync`,
        { method: 'POST' },
      )
      snackbar.info(res.message || t('suppliers.sync_started'))
      void loadAccounts()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusyId(null)
    }
  }

  async function importProducts(id: string) {
    setBusyId(id)
    try {
      const res = await api<{ created: number; linked: number; skipped: number }>(
        `/api/suppliers/accounts/${id}/import`,
        { method: 'POST', body: JSON.stringify({}) },
      )
      snackbar.success(`${t('suppliers.import_done')}: +${res.created} / ↻${res.linked} / ⤼${res.skipped}`)
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusyId(null)
    }
  }

  async function loadProducts(id: string) {
    setSelected(id)
    try {
      const params = new URLSearchParams()
      if (q) params.set('q', q)
      params.set('pageSize', '100')
      setProducts(
        await api<ProductsResponse>(`/api/suppliers/accounts/${id}/products?${params.toString()}`),
      )
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 0.5 }}>{t('suppliers.title')}</Typography>
      <Typography variant="body2" color="text.secondary" sx={{ mb: 2 }}>{t('suppliers.subtitle')}</Typography>

      {stub && <Alert severity="info" sx={{ mb: 2 }}>{t('suppliers.stub_notice')}</Alert>}

      <Can I="create" a="Supplier">
        <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('suppliers.new_account')}</Typography>
          <Box component="form" onSubmit={handleCreate}>
            <Stack spacing={1.5}>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                <TextField label={t('suppliers.base_url')} helperText={t('suppliers.base_url_hint')} value={baseUrl} onChange={(e) => setBaseUrl(e.target.value)} required size="small" sx={{ flex: 2 }} fullWidth />
                <TextField label={t('suppliers.label')} value={label} onChange={(e) => setLabel(e.target.value)} size="small" sx={{ flex: 1 }} fullWidth />
              </Stack>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                <TextField label={t('suppliers.login_email')} type="email" value={loginEmail} onChange={(e) => setLoginEmail(e.target.value)} required size="small" sx={{ flex: 2 }} fullWidth />
                <TextField label={t('suppliers.password')} type="password" value={password} onChange={(e) => setPassword(e.target.value)} required size="small" sx={{ flex: 2 }} fullWidth />
                <TextField label={t('suppliers.markup')} type="number" value={markup} onChange={(e) => setMarkup(e.target.value)} size="small" sx={{ flex: 1, minWidth: 120 }} fullWidth />
              </Stack>
              <FormControlLabel
                control={<Checkbox checked={consent} onChange={(e) => setConsent(e.target.checked)} />}
                label={t('suppliers.consent')}
              />
              <Box>
                <Button type="submit" variant="contained" disabled={creating} sx={{ minWidth: 140 }}>
                  {creating ? '…' : t('suppliers.connect')}
                </Button>
              </Box>
            </Stack>
          </Box>
        </Paper>
      </Can>

      {!accounts ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : accounts.length === 0 ? (
        <Typography color="text.secondary">{t('suppliers.empty')}</Typography>
      ) : (
        <Stack spacing={2}>
          {accounts.map((a) => (
            <Paper key={a.id} variant="outlined" sx={{ p: 2 }}>
              <Stack direction={{ xs: 'column', md: 'row' }} spacing={1} sx={{ alignItems: { md: 'center' }, justifyContent: 'space-between' }}>
                <Box>
                  <Typography sx={{ fontWeight: 600 }}>
                    {a.label || a.baseUrl}
                    {!a.active && <Chip size="small" label={t('suppliers.inactive')} sx={{ ml: 1 }} />}
                    {a.syncing && <Chip size="small" color="warning" label={t('suppliers.syncing')} sx={{ ml: 1 }} />}
                    {a.hasSession && <Chip size="small" color="success" label="session" sx={{ ml: 1 }} />}
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    {a.loginEmail} · {a.baseUrl}
                  </Typography>
                  <Typography variant="caption" color="text.secondary" sx={{ display: 'block' }}>
                    {t('suppliers.last_sync')}: {a.lastSyncAt ? new Date(a.lastSyncAt).toLocaleString() : t('suppliers.never_synced')}
                    {typeof a.productCount === 'number' ? ` · ${a.productCount} ${t('suppliers.products').toLowerCase()}` : ''}
                  </Typography>
                  {a.lastError && <Alert severity="warning" sx={{ mt: 1, py: 0 }}>{a.lastError}</Alert>}
                </Box>
                <Stack direction="row" spacing={1} sx={{ flexWrap: 'wrap' }}>
                  <Can I="update" a="Supplier">
                    <Button size="small" variant="outlined" disabled={busyId === a.id} onClick={() => void testLogin(a.id)}>{t('suppliers.test_login')}</Button>
                    <Button size="small" variant="contained" disabled={busyId === a.id} onClick={() => void sync(a.id)}>{t('suppliers.sync')}</Button>
                  </Can>
                  <Button size="small" variant="text" onClick={() => void loadProducts(a.id)}>{t('suppliers.products')}</Button>
                  <Can I="create" a="Product">
                    <Button size="small" variant="outlined" color="secondary" disabled={busyId === a.id} onClick={() => void importProducts(a.id)}>{t('suppliers.import')}</Button>
                  </Can>
                </Stack>
              </Stack>
            </Paper>
          ))}
        </Stack>
      )}

      {selected && products && (
        <Box sx={{ mt: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1 }}>{t('suppliers.products')}</Typography>
          <Stack direction="row" spacing={1} sx={{ mb: 1 }}>
            <TextField placeholder={t('common.search')} value={q} onChange={(e) => setQ(e.target.value)} size="small" sx={{ flex: 1 }} />
            <Button variant="outlined" onClick={() => void loadProducts(selected)}>{t('common.filter')}</Button>
          </Stack>
          {products.items.length === 0 ? (
            <Typography color="text.secondary">{t('suppliers.products_empty')}</Typography>
          ) : (
            <TableContainer component={Paper} variant="outlined">
              <Table size="small">
                <TableHead>
                  <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                    <TableCell>{t('suppliers.col_name')}</TableCell>
                    <TableCell>{t('suppliers.col_variant')}</TableCell>
                    <TableCell>{t('suppliers.col_sku')}</TableCell>
                    <TableCell align="right">{t('suppliers.col_price')}</TableCell>
                    <TableCell align="right">{t('suppliers.col_pix')}</TableCell>
                    <TableCell align="right">{t('suppliers.col_stock')}</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {products.items.map((p) => (
                    <TableRow key={p.id} hover sx={{ opacity: p.available ? 1 : 0.5 }}>
                      <TableCell>{p.name}</TableCell>
                      <TableCell>{[p.option0, p.option1].filter(Boolean).join(' / ') || '—'}</TableCell>
                      <TableCell>{p.sku ?? '—'}</TableCell>
                      <TableCell align="right">{Number(p.price).toFixed(2)}</TableCell>
                      <TableCell align="right">{p.pixPrice ? Number(p.pixPrice).toFixed(2) : '—'}</TableCell>
                      <TableCell align="right">{p.stock ?? '—'}</TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </TableContainer>
          )}
          <Typography variant="caption" color="text.secondary" sx={{ display: 'block', mt: 1 }}>
            {products.total} · {t('suppliers.dry_run_notice')}
          </Typography>
        </Box>
      )}
    </Layout>
  )
}
