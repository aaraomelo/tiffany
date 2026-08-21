import { CheckCircleOutlined, WarningAmberOutlined } from '@mui/icons-material'
import {
  Button,
  Chip,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  FormControlLabel,
  Paper,
  Stack,
  Switch,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  TextField,
  Typography,
} from '@mui/material'
import { useEffect, useState } from 'react'
import {
  fetchHealthPeople,
  fetchHealthRecord,
  saveHealthRecord,
  type HealthPerson,
  type HealthRecord,
} from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

export function HealthPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [people, setPeople] = useState<HealthPerson[] | null>(null)
  const [q, setQ] = useState('')
  const [editing, setEditing] = useState<{ id: string; name: string } | null>(null)
  const [form, setForm] = useState<HealthRecord>({})
  const [saving, setSaving] = useState(false)

  async function load() {
    try {
      setPeople(await fetchHealthPeople(q))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }
  useEffect(() => { void load() }, [])

  async function openEditor(p: HealthPerson) {
    try {
      const data = await fetchHealthRecord(p.id)
      setForm(data.record ?? {})
      setEditing({ id: p.id, name: data.student.name })
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  function set<K extends keyof HealthRecord>(k: K, v: HealthRecord[K]) {
    setForm((f) => ({ ...f, [k]: v }))
  }

  async function save() {
    if (!editing) return
    setSaving(true)
    try {
      await saveHealthRecord(editing.id, form)
      snackbar.success(t('health.saved'))
      setEditing(null)
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setSaving(false)
    }
  }

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 0.5 }}>{t('health.title')}</Typography>
      <Typography color="text.secondary" sx={{ mb: 2 }}>{t('health.subtitle')}</Typography>

      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} sx={{ mb: 2 }}>
        <TextField placeholder={t('health.search')} value={q} onChange={(e) => setQ(e.target.value)} size="small" sx={{ flex: 1 }} />
        <Button variant="outlined" onClick={() => void load()}>{t('common.filter')}</Button>
      </Stack>

      {!people ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <TableContainer component={Paper} variant="outlined">
          <Table size="small">
            <TableHead>
              <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                <TableCell>{t('common.name')}</TableCell>
                <TableCell>{t('common.phone')}</TableCell>
                <TableCell>{t('health.blood_type')}</TableCell>
                <TableCell>{t('health.clearance')}</TableCell>
                <TableCell>{t('health.alerts')}</TableCell>
                <TableCell>{t('common.actions')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {people.length === 0 && (
                <TableRow><TableCell colSpan={6} sx={{ color: 'text.secondary' }}>{t('health.empty')}</TableCell></TableRow>
              )}
              {people.map((p) => (
                <TableRow key={p.id} hover>
                  <TableCell>{p.name}</TableCell>
                  <TableCell>{p.phone ?? '—'}</TableCell>
                  <TableCell>{p.bloodType ?? '—'}</TableCell>
                  <TableCell>
                    {p.medicalClearance
                      ? <Chip size="small" color="success" icon={<CheckCircleOutlined />} label={t('health.cleared')} />
                      : <Chip size="small" variant="outlined" label={t('health.not_cleared')} />}
                  </TableCell>
                  <TableCell>
                    {p.hasAlerts && <Chip size="small" color="warning" icon={<WarningAmberOutlined />} label={t('health.has_alerts')} />}
                  </TableCell>
                  <TableCell>
                    <Button size="small" onClick={() => void openEditor(p)}>
                      {p.hasRecord ? t('health.edit_record') : t('health.add_record')}
                    </Button>
                  </TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}

      <Dialog open={!!editing} onClose={() => setEditing(null)} maxWidth="sm" fullWidth>
        <DialogTitle>{t('health.record_of', { name: editing?.name ?? '' })}</DialogTitle>
        <DialogContent>
          <Stack spacing={1.5} sx={{ mt: 1 }}>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
              <TextField label={t('health.blood_type')} value={form.bloodType ?? ''} onChange={(e) => set('bloodType', e.target.value)} size="small" sx={{ width: { sm: 140 } }} />
              <FormControlLabel control={<Switch checked={form.medicalClearance ?? false} onChange={(e) => set('medicalClearance', e.target.checked)} />} label={t('health.clearance')} />
            </Stack>
            <TextField label={t('health.allergies')} value={form.allergies ?? ''} onChange={(e) => set('allergies', e.target.value)} size="small" fullWidth multiline minRows={2} />
            <TextField label={t('health.chronic')} value={form.chronicConditions ?? ''} onChange={(e) => set('chronicConditions', e.target.value)} size="small" fullWidth multiline minRows={2} />
            <TextField label={t('health.medications')} value={form.medications ?? ''} onChange={(e) => set('medications', e.target.value)} size="small" fullWidth multiline minRows={2} />
            <TextField label={t('health.injuries')} value={form.previousInjuries ?? ''} onChange={(e) => set('previousInjuries', e.target.value)} size="small" fullWidth multiline minRows={2} />
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
              <TextField label={t('health.insurance')} value={form.healthInsurance ?? ''} onChange={(e) => set('healthInsurance', e.target.value)} size="small" fullWidth />
              <TextField label={t('health.insurance_number')} value={form.healthInsuranceNumber ?? ''} onChange={(e) => set('healthInsuranceNumber', e.target.value)} size="small" fullWidth />
            </Stack>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
              <TextField label={t('health.emergency_name')} value={form.emergencyContactName ?? ''} onChange={(e) => set('emergencyContactName', e.target.value)} size="small" fullWidth />
              <TextField label={t('health.emergency_phone')} value={form.emergencyContactPhone ?? ''} onChange={(e) => set('emergencyContactPhone', e.target.value)} size="small" fullWidth />
            </Stack>
            <TextField label={t('health.observations')} value={form.observations ?? ''} onChange={(e) => set('observations', e.target.value)} size="small" fullWidth multiline minRows={2} />
          </Stack>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setEditing(null)}>{t('common.cancel')}</Button>
          <Can I="update" a="Student" fallback={<span />}>
            <Button variant="contained" onClick={() => void save()} disabled={saving}>
              {saving ? '…' : t('common.save')}
            </Button>
          </Can>
        </DialogActions>
      </Dialog>
    </Layout>
  )
}
