import { useEffect, useState } from 'react'
import {
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
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'

interface OrderRow {
  id: string
  number: number
  total: string
  status: string
  channel: string
  createdAt: string
  customer: { id: string; name: string } | null
  items: Array<{ id: string }>
}
interface ListResponse { items: OrderRow[]; total: number; page: number; pageSize: number }

const STATUS_STYLE: Record<string, object> = {
  COMPLETED: { color: 'success.main', fontWeight: 600 },
  PAID: { color: 'primary.main', fontWeight: 600 },
  PARTIALLY_PAID: { color: 'warning.main' },
  AWAITING_PAYMENT: { color: 'text.secondary' },
  CANCELLED: { color: 'text.secondary', textDecoration: 'line-through' },
}

const STATUS_KEYS = ['AWAITING_PAYMENT', 'PARTIALLY_PAID', 'PAID', 'COMPLETED', 'CANCELLED']

export function OrdersPage() {
  const t = useT()
  const [data, setData] = useState<ListResponse | null>(null)
  const [status, setStatus] = useState('')

  async function load() {
    const params = new URLSearchParams()
    if (status) params.set('status', status)
    setData(await api<ListResponse>(`/api/orders?${params.toString()}`))
  }
  useEffect(() => { void load() }, [status])

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 2 }}>{t('orders.title')}</Typography>

      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} sx={{ mb: 2 }}>
        <TextField select value={status} onChange={(e) => setStatus(e.target.value)} size="small" sx={{ minWidth: 200 }}>
          <MenuItem value="">{t('common.all')}</MenuItem>
          {STATUS_KEYS.map((s) => <MenuItem key={s} value={s}>{t(`orders.status.${s}`)}</MenuItem>)}
        </TextField>
      </Stack>

      {!data ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <TableContainer component={Paper} variant="outlined">
          <Table size="small">
            <TableHead>
              <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                <TableCell>{t('orders.col_number')}</TableCell>
                <TableCell>{t('orders.col_date')}</TableCell>
                <TableCell>{t('orders.customer')}</TableCell>
                <TableCell>{t('orders.items')}</TableCell>
                <TableCell>{t('orders.channel')}</TableCell>
                <TableCell align="right">{t('common.total')}</TableCell>
                <TableCell>{t('common.status')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {data.items.length === 0 && (
                <TableRow><TableCell colSpan={7} sx={{ color: 'text.secondary' }}>{t('orders.empty')}</TableCell></TableRow>
              )}
              {data.items.map((o) => (
                <TableRow key={o.id} hover>
                  <TableCell><strong>{o.number}</strong></TableCell>
                  <TableCell>{new Date(o.createdAt).toLocaleString()}</TableCell>
                  <TableCell>{o.customer?.name ?? '—'}</TableCell>
                  <TableCell>{o.items.length}</TableCell>
                  <TableCell>{o.channel}</TableCell>
                  <TableCell align="right">R$ {Number(o.total).toFixed(2)}</TableCell>
                  <TableCell sx={STATUS_STYLE[o.status] ?? {}}>{t(`orders.status.${o.status}`, {})}</TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}
    </Layout>
  )
}
