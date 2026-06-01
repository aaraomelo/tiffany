import { WhatsApp } from '@mui/icons-material'
import { useEffect, useState } from 'react'
import {
  Box,
  Button,
  IconButton,
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
  Tooltip,
  Typography,
} from '@mui/material'
import { api } from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

type TuitionStatus = 'PENDING' | 'PAID' | 'OVERDUE' | 'CANCELLED'

interface Tuition {
  id: string
  referenceMonth: number
  referenceYear: number
  dueDate: string
  amount: string
  status: TuitionStatus
  paidAt: string | null
  student: { id: string; name: string; phone: string | null; whatsapp: string | null }
}

// telefone -> formato wa.me (só dígitos, DDI BR padrão)
function waNumber(phone?: string | null): string | null {
  if (!phone) return null
  let d = phone.replace(/\D/g, '')
  if (!d) return null
  if (!d.startsWith('55') && d.length <= 11) d = '55' + d
  return d
}

interface ListResponse {
  items: Tuition[]
  total: number
  page: number
  pageSize: number
}

const STATUSES: TuitionStatus[] = ['PENDING', 'PAID', 'OVERDUE', 'CANCELLED']

function money(v: string | number) {
  return Number(v).toLocaleString('pt-BR', { style: 'currency', currency: 'BRL' })
}

function ref(m: number, y: number) {
  return `${String(m).padStart(2, '0')}/${y}`
}

const now = new Date()

export function TuitionsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<ListResponse | null>(null)
  const [status, setStatus] = useState<TuitionStatus | ''>('')
  const [generating, setGenerating] = useState(false)

  async function load() {
    const params = new URLSearchParams()
    if (status) params.set('status', status)
    try {
      setData(await api<ListResponse>(`/api/tuitions?${params.toString()}`))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [status])

  async function pay(tu: Tuition) {
    try {
      await api(`/api/tuitions/${tu.id}/pay`, { method: 'POST', body: JSON.stringify({}) })
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  async function cancel(tu: Tuition) {
    try {
      await api(`/api/tuitions/${tu.id}/cancel`, { method: 'POST', body: JSON.stringify({}) })
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  function charge(tu: Tuition) {
    const num = waNumber(tu.student.whatsapp || tu.student.phone)
    if (!num) { snackbar.error(t('tuitions.no_phone')); return }
    const msg = t('tuitions.charge_msg', {
      name: tu.student.name,
      ref: ref(tu.referenceMonth, tu.referenceYear),
      amount: money(tu.amount),
      due: new Date(tu.dueDate).toLocaleDateString('pt-BR'),
    })
    window.open(`https://wa.me/${num}?text=${encodeURIComponent(msg)}`, '_blank', 'noopener')
  }

  async function generateBatch() {
    setGenerating(true)
    try {
      const res = await api<{ created: number; skipped: number }>('/api/enrollments/generate-tuitions', {
        method: 'POST',
        body: JSON.stringify({
          referenceMonth: now.getMonth() + 1,
          referenceYear: now.getFullYear(),
        }),
      })
      snackbar.success(t('tuitions.batch_done', { created: res.created, skipped: res.skipped }))
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setGenerating(false)
    }
  }

  return (
    <Layout>
      <Stack direction="row" spacing={1} sx={{ alignItems: 'center', mb: 2 }}>
        <Typography variant="h5" sx={{ fontWeight: 700, flex: 1 }}>{t('tuitions.title')}</Typography>
        <Can I="create" a="Tuition">
          <Button variant="contained" onClick={() => void generateBatch()} disabled={generating}>
            {generating ? '…' : t('tuitions.generate_batch')}
          </Button>
        </Can>
      </Stack>

      <Box sx={{ mb: 2 }}>
        <TextField select value={status} onChange={(e) => setStatus(e.target.value as TuitionStatus | '')} size="small" sx={{ minWidth: 160 }}>
          <MenuItem value="">{t('common.all')}</MenuItem>
          {STATUSES.map((s) => <MenuItem key={s} value={s}>{t(`tuitions.status.${s}`)}</MenuItem>)}
        </TextField>
      </Box>

      {!data ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <TableContainer component={Paper} variant="outlined">
          <Table size="small">
            <TableHead>
              <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                <TableCell>{t('enrollments.student')}</TableCell>
                <TableCell>{t('tuitions.reference')}</TableCell>
                <TableCell>{t('tuitions.due_date')}</TableCell>
                <TableCell>{t('common.amount')}</TableCell>
                <TableCell>{t('common.status')}</TableCell>
                <TableCell>{t('common.actions')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {data.items.length === 0 && (
                <TableRow><TableCell colSpan={6} sx={{ color: 'text.secondary' }}>{t('tuitions.empty')}</TableCell></TableRow>
              )}
              {data.items.map((tu) => (
                <TableRow key={tu.id} hover>
                  <TableCell>{tu.student.name}</TableCell>
                  <TableCell>{ref(tu.referenceMonth, tu.referenceYear)}</TableCell>
                  <TableCell>{new Date(tu.dueDate).toLocaleDateString('pt-BR')}</TableCell>
                  <TableCell>{money(tu.amount)}</TableCell>
                  <TableCell>{t(`tuitions.status.${tu.status}`)}</TableCell>
                  <TableCell>
                    {(tu.status === 'PENDING' || tu.status === 'OVERDUE') && (
                      <Stack direction="row" spacing={0.5} sx={{ alignItems: 'center' }}>
                        <Tooltip title={t('tuitions.charge')}>
                          <IconButton size="small" sx={{ color: '#25D366' }} onClick={() => charge(tu)}>
                            <WhatsApp fontSize="small" />
                          </IconButton>
                        </Tooltip>
                        <Can I="update" a="Tuition">
                          <Button size="small" variant="text" onClick={() => void pay(tu)}>{t('tuitions.pay')}</Button>
                          <Button size="small" variant="text" color="error" onClick={() => void cancel(tu)}>{t('common.cancel')}</Button>
                        </Can>
                      </Stack>
                    )}
                  </TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}

      {data && (
        <Box sx={{ mt: 1.5 }}>
          <Typography variant="caption" color="text.secondary">
            {t('students.total_page', { total: data.total, page: data.page })}
          </Typography>
        </Box>
      )}
    </Layout>
  )
}
