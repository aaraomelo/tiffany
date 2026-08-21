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

type EnrollmentStatus = 'ACTIVE' | 'SUSPENDED' | 'CANCELLED' | 'COMPLETED'

interface Enrollment {
  id: string
  status: EnrollmentStatus
  startDate: string
  dueDay: number
  monthlyPrice: string
  student: { id: string; name: string }
  plan: { id: string; name: string }
}

interface StudentLite { id: string; name: string }
interface PlanLite { id: string; name: string; price: string }

function money(v: string | number) {
  return Number(v).toLocaleString('pt-BR', { style: 'currency', currency: 'BRL' })
}

const now = new Date()

export function EnrollmentsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [items, setItems] = useState<Enrollment[] | null>(null)
  const [students, setStudents] = useState<StudentLite[]>([])
  const [plans, setPlans] = useState<PlanLite[]>([])
  const [studentId, setStudentId] = useState('')
  const [planId, setPlanId] = useState('')
  const [dueDay, setDueDay] = useState('10')
  const [creating, setCreating] = useState(false)

  async function load() {
    try {
      const [list, st, pl] = await Promise.all([
        api<Enrollment[]>('/api/enrollments'),
        api<{ items: StudentLite[] }>('/api/students?status=ACTIVE&pageSize=200'),
        api<PlanLite[]>('/api/enrollment-plans?active=true'),
      ])
      setItems(list)
      setStudents(st.items)
      setPlans(pl)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!studentId || !planId) return
    setCreating(true)
    try {
      await api('/api/enrollments', {
        method: 'POST',
        body: JSON.stringify({ studentId, planId, dueDay: Number(dueDay) }),
      })
      setStudentId(''); setPlanId('')
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  async function generateTuition(en: Enrollment) {
    try {
      await api(`/api/enrollments/${en.id}/generate-tuition`, {
        method: 'POST',
        body: JSON.stringify({
          referenceMonth: now.getMonth() + 1,
          referenceYear: now.getFullYear(),
        }),
      })
      snackbar.success(t('enrollments.tuition_generated', { name: en.student.name }))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 2 }}>{t('enrollments.title')}</Typography>

      <Can I="create" a="Enrollment">
        <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('enrollments.new')}</Typography>
          <Box component="form" onSubmit={handleCreate}>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ alignItems: { sm: 'flex-start' } }}>
              <TextField select label={t('enrollments.student')} value={studentId} onChange={(e) => setStudentId(e.target.value)} required size="small" sx={{ flex: 2, minWidth: 160 }} fullWidth>
                <MenuItem value="">—</MenuItem>
                {students.map((s) => <MenuItem key={s.id} value={s.id}>{s.name}</MenuItem>)}
              </TextField>
              <TextField select label={t('enrollments.plan')} value={planId} onChange={(e) => setPlanId(e.target.value)} required size="small" sx={{ flex: 2, minWidth: 160 }} fullWidth>
                <MenuItem value="">—</MenuItem>
                {plans.map((p) => <MenuItem key={p.id} value={p.id}>{p.name} · {money(p.price)}</MenuItem>)}
              </TextField>
              <TextField type="number" label={t('enrollments.due_day')} value={dueDay} onChange={(e) => setDueDay(e.target.value)} size="small" slotProps={{ htmlInput: { min: 1, max: 28 } }} sx={{ flex: 1 }} fullWidth />
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
                <TableCell>{t('enrollments.student')}</TableCell>
                <TableCell>{t('enrollments.plan')}</TableCell>
                <TableCell>{t('enrollments.monthly')}</TableCell>
                <TableCell>{t('enrollments.due_day')}</TableCell>
                <TableCell>{t('common.status')}</TableCell>
                <TableCell>{t('common.actions')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {items.length === 0 && (
                <TableRow><TableCell colSpan={6} sx={{ color: 'text.secondary' }}>{t('enrollments.empty')}</TableCell></TableRow>
              )}
              {items.map((en) => (
                <TableRow key={en.id} hover>
                  <TableCell>{en.student.name}</TableCell>
                  <TableCell>{en.plan.name}</TableCell>
                  <TableCell>{money(en.monthlyPrice)}</TableCell>
                  <TableCell>{en.dueDay}</TableCell>
                  <TableCell>{t(`enrollments.status.${en.status}`)}</TableCell>
                  <TableCell>
                    {en.status === 'ACTIVE' && (
                      <Can I="create" a="Tuition">
                        <Button size="small" variant="text" onClick={() => void generateTuition(en)}>
                          {t('enrollments.generate_tuition')}
                        </Button>
                      </Can>
                    )}
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
