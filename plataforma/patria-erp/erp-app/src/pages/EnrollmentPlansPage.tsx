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

type BillingCycle = 'MONTHLY' | 'QUARTERLY' | 'SEMIANNUAL' | 'ANNUAL'

interface EnrollmentPlan {
  id: string
  name: string
  description: string | null
  price: string
  billingCycle: BillingCycle
  weeklySessions: number | null
  enrollmentFee: string
  active: boolean
}

const CYCLES: BillingCycle[] = ['MONTHLY', 'QUARTERLY', 'SEMIANNUAL', 'ANNUAL']

function money(v: string | number) {
  return Number(v).toLocaleString('pt-BR', { style: 'currency', currency: 'BRL' })
}

export function EnrollmentPlansPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [items, setItems] = useState<EnrollmentPlan[] | null>(null)
  const [creating, setCreating] = useState(false)
  const [name, setName] = useState('')
  const [price, setPrice] = useState('')
  const [weekly, setWeekly] = useState('')
  const [cycle, setCycle] = useState<BillingCycle>('MONTHLY')

  async function load() {
    try {
      setItems(await api<EnrollmentPlan[]>('/api/enrollment-plans'))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!name.trim() || !price) return
    setCreating(true)
    try {
      await api('/api/enrollment-plans', {
        method: 'POST',
        body: JSON.stringify({
          name,
          price: Number(price),
          billingCycle: cycle,
          weeklySessions: weekly ? Number(weekly) : undefined,
        }),
      })
      setName(''); setPrice(''); setWeekly('')
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  async function toggle(p: EnrollmentPlan) {
    try {
      await api(`/api/enrollment-plans/${p.id}`, {
        method: 'PATCH',
        body: JSON.stringify({ active: !p.active }),
      })
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 2 }}>{t('plans.title')}</Typography>

      <Can I="create" a="EnrollmentPlan">
        <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('plans.new')}</Typography>
          <Box component="form" onSubmit={handleCreate}>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ alignItems: { sm: 'flex-start' } }}>
              <TextField label={t('common.name')} value={name} onChange={(e) => setName(e.target.value)} required size="small" sx={{ flex: 2 }} fullWidth />
              <TextField type="number" label={t('plans.price')} value={price} onChange={(e) => setPrice(e.target.value)} required size="small" slotProps={{ htmlInput: { step: '0.01', min: 0 } }} sx={{ flex: 1 }} fullWidth />
              <TextField type="number" label={t('plans.weekly_sessions')} value={weekly} onChange={(e) => setWeekly(e.target.value)} size="small" slotProps={{ htmlInput: { min: 1 } }} sx={{ flex: 1 }} fullWidth />
              <TextField select label={t('plans.billing_cycle')} value={cycle} onChange={(e) => setCycle(e.target.value as BillingCycle)} size="small" sx={{ flex: 1, minWidth: 140 }} fullWidth>
                {CYCLES.map((c) => <MenuItem key={c} value={c}>{t(`plans.cycle.${c}`)}</MenuItem>)}
              </TextField>
              <Button type="submit" variant="contained" disabled={creating} sx={{ minWidth: 110, height: 40 }}>
                {creating ? '…' : t('common.create')}
              </Button>
            </Stack>
          </Box>
        </Paper>
      </Can>

      {!items ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <TableContainer component={Paper} variant="outlined">
          <Table size="small">
            <TableHead>
              <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                <TableCell>{t('common.name')}</TableCell>
                <TableCell>{t('plans.price')}</TableCell>
                <TableCell>{t('plans.billing_cycle')}</TableCell>
                <TableCell>{t('plans.weekly_sessions')}</TableCell>
                <TableCell>{t('common.status')}</TableCell>
                <TableCell>{t('common.actions')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {items.length === 0 && (
                <TableRow><TableCell colSpan={6} sx={{ color: 'text.secondary' }}>{t('plans.empty')}</TableCell></TableRow>
              )}
              {items.map((p) => (
                <TableRow key={p.id} hover sx={{ opacity: p.active ? 1 : 0.5 }}>
                  <TableCell>{p.name}</TableCell>
                  <TableCell>{money(p.price)}</TableCell>
                  <TableCell>{t(`plans.cycle.${p.billingCycle}`)}</TableCell>
                  <TableCell>{p.weeklySessions ?? '—'}</TableCell>
                  <TableCell>{p.active ? t('plans.active') : t('plans.inactive')}</TableCell>
                  <TableCell>
                    <Can I="update" a="EnrollmentPlan">
                      <Button size="small" variant="text" onClick={() => void toggle(p)}>
                        {p.active ? t('plans.deactivate') : t('plans.activate')}
                      </Button>
                    </Can>
                  </TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}
    </Layout>
  )
}
