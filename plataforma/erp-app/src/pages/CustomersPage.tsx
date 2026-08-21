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

type PartyRole = 'CUSTOMER' | 'SUPPLIER' | 'BOTH' | 'EMPLOYEE'
const ROLES: PartyRole[] = ['CUSTOMER', 'SUPPLIER', 'BOTH', 'EMPLOYEE']

interface CustomerSupplier {
  id: string
  name: string
  tradeName: string | null
  document: string | null
  email: string | null
  phone: string | null
  role: PartyRole
  personType: 'INDIVIDUAL' | 'COMPANY'
  active: boolean
  createdAt: string
}

interface ListResponse {
  items: CustomerSupplier[]
  total: number
  page: number
  pageSize: number
}

export function CustomersPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<ListResponse | null>(null)
  const [q, setQ] = useState('')
  const [role, setRole] = useState<PartyRole | ''>('')
  const [creating, setCreating] = useState(false)
  const [newName, setNewName] = useState('')
  const [newDoc, setNewDoc] = useState('')
  const [newRole, setNewRole] = useState<PartyRole>('CUSTOMER')

  async function load() {
    const params = new URLSearchParams()
    if (q) params.set('q', q)
    if (role) params.set('role', role)
    try {
      const res = await api<ListResponse>(`/api/customer-suppliers?${params.toString()}`)
      setData(res)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!newName.trim()) return
    setCreating(true)
    try {
      await api('/api/customer-suppliers', {
        method: 'POST',
        body: JSON.stringify({ name: newName, document: newDoc || undefined, role: newRole }),
      })
      setNewName(''); setNewDoc('')
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 2 }}>{t('customers.title')}</Typography>

      <Can I="create" a="Customer">
        <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('customers.new')}</Typography>
          <Box component="form" onSubmit={handleCreate}>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ alignItems: { sm: 'flex-start' } }}>
              <TextField label={t('common.name')} value={newName} onChange={(e) => setNewName(e.target.value)} required size="small" sx={{ flex: 2 }} fullWidth />
              <TextField label={t('common.document')} value={newDoc} onChange={(e) => setNewDoc(e.target.value)} size="small" sx={{ flex: 1 }} fullWidth />
              <TextField select label={t('customers.type')} value={newRole} onChange={(e) => setNewRole(e.target.value as PartyRole)} size="small" sx={{ flex: 1, minWidth: 140 }} fullWidth>
                {ROLES.map((r) => <MenuItem key={r} value={r}>{t(`customers.role.${r}`)}</MenuItem>)}
              </TextField>
              <Button type="submit" variant="contained" disabled={creating} sx={{ minWidth: 110, height: 40 }}>
                {creating ? '…' : t('common.create')}
              </Button>
            </Stack>
          </Box>
        </Paper>
      </Can>

      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} sx={{ mb: 2 }}>
        <TextField placeholder={t('customers.search_placeholder')} value={q} onChange={(e) => setQ(e.target.value)} size="small" sx={{ flex: 1 }} />
        <TextField select value={role} onChange={(e) => setRole(e.target.value as PartyRole | '')} size="small" sx={{ minWidth: 160 }}>
          <MenuItem value="">{t('common.all')}</MenuItem>
          {ROLES.map((r) => <MenuItem key={r} value={r}>{t(`customers.role.${r}`)}</MenuItem>)}
        </TextField>
        <Button variant="outlined" onClick={() => void load()}>{t('common.filter')}</Button>
      </Stack>

      {!data ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <TableContainer component={Paper} variant="outlined">
          <Table size="small">
            <TableHead>
              <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                <TableCell>{t('customers.col_name')}</TableCell>
                <TableCell>{t('customers.col_doc')}</TableCell>
                <TableCell>{t('customers.col_type')}</TableCell>
                <TableCell>{t('customers.col_email')}</TableCell>
                <TableCell>{t('customers.col_phone')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {data.items.length === 0 && (
                <TableRow><TableCell colSpan={5} sx={{ color: 'text.secondary' }}>{t('customers.empty')}</TableCell></TableRow>
              )}
              {data.items.map((cs) => (
                <TableRow key={cs.id} hover>
                  <TableCell>{cs.name}</TableCell>
                  <TableCell>{cs.document ?? '—'}</TableCell>
                  <TableCell>{t(`customers.role.${cs.role}`)}</TableCell>
                  <TableCell>{cs.email ?? '—'}</TableCell>
                  <TableCell>{cs.phone ?? '—'}</TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}

      {data && (
        <Box sx={{ mt: 1.5 }}>
          <Typography variant="caption" color="text.secondary">
            {t('customers.total_page', { total: data.total, page: data.page })}
          </Typography>
        </Box>
      )}
    </Layout>
  )
}
