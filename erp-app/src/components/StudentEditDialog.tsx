import { useEffect, useRef, useState } from 'react'
import {
  Avatar,
  Box,
  Button,
  CircularProgress,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Divider,
  MenuItem,
  Stack,
  TextField,
  Typography,
} from '@mui/material'
import {
  deleteStudentPhoto,
  fetchStudent,
  fetchStudentPhotoUrl,
  saveStudentHealthRecord,
  updateStudent,
  uploadStudentPhoto,
  type FootPreference,
  type Student,
  type StudentStatus,
} from '../api'
import { useSnackbar } from './Snackbar'
import { useT } from '../i18n/LangContext'
import { resizeImageFile } from '../utils/image'

interface Props {
  studentId: string | null
  open: boolean
  onClose: () => void
  onSaved: () => void
}

type FormState = Partial<Student> & { bloodType?: string | null }

const EMPTY: FormState = { name: '', status: 'ACTIVE' }

const BLOOD_TYPES = ['A+', 'A-', 'B+', 'B-', 'AB+', 'AB-', 'O+', 'O-']

export function StudentEditDialog({ studentId, open, onClose, onSaved }: Props) {
  const t = useT()
  const snackbar = useSnackbar()
  const fileRef = useRef<HTMLInputElement>(null)
  const [form, setForm] = useState<FormState>(EMPTY)
  const [photoUrl, setPhotoUrl] = useState<string | null>(null)
  const [loading, setLoading] = useState(false)
  const [saving, setSaving] = useState(false)
  const [uploading, setUploading] = useState(false)

  useEffect(() => {
    if (!open || !studentId) return
    let active = true
    let objectUrl: string | null = null
    setLoading(true)
    setPhotoUrl(null)
    void (async () => {
      try {
        const s = await fetchStudent(studentId)
        if (!active) return
        setForm({ ...s, bloodType: s.healthRecord?.bloodType ?? '' })
        if (s.photoUpdatedAt) {
          const u = await fetchStudentPhotoUrl(studentId, s.photoUpdatedAt)
          if (!active) {
            if (u) URL.revokeObjectURL(u)
            return
          }
          objectUrl = u
          setPhotoUrl(u)
        }
      } catch (e) {
        if (active) snackbar.error((e as Error).message)
      } finally {
        if (active) setLoading(false)
      }
    })()
    return () => {
      active = false
      if (objectUrl) URL.revokeObjectURL(objectUrl)
    }
  }, [open, studentId])

  function set<K extends keyof FormState>(key: K, value: FormState[K]) {
    setForm((f) => ({ ...f, [key]: value }))
  }

  async function handlePhotoSelected(e: React.ChangeEvent<HTMLInputElement>) {
    const file = e.target.files?.[0]
    e.target.value = '' // permite re-selecionar o mesmo arquivo
    if (!file || !studentId) return
    setUploading(true)
    try {
      const { dataUrl, mimeType } = await resizeImageFile(file)
      await uploadStudentPhoto(studentId, dataUrl, mimeType)
      if (photoUrl) URL.revokeObjectURL(photoUrl)
      setPhotoUrl(dataUrl) // preview imediato com o próprio data URL
      snackbar.success(t('students.photo_saved'))
      onSaved()
    } catch (err) {
      snackbar.error((err as Error).message)
    } finally {
      setUploading(false)
    }
  }

  async function handleRemovePhoto() {
    if (!studentId) return
    setUploading(true)
    try {
      await deleteStudentPhoto(studentId)
      if (photoUrl) URL.revokeObjectURL(photoUrl)
      setPhotoUrl(null)
      onSaved()
    } catch (err) {
      snackbar.error((err as Error).message)
    } finally {
      setUploading(false)
    }
  }

  async function handleSave() {
    if (!studentId) return
    setSaving(true)
    try {
      const { bloodType, ...rest } = form
      const patch: Partial<Student> = {
        name: rest.name,
        birthDate: rest.birthDate || null,
        document: rest.document || null,
        gender: rest.gender || null,
        email: rest.email || null,
        phone: rest.phone || null,
        whatsapp: rest.whatsapp || null,
        zipCode: rest.zipCode || null,
        street: rest.street || null,
        number: rest.number || null,
        neighborhood: rest.neighborhood || null,
        city: rest.city || null,
        state: rest.state || null,
        fatherName: rest.fatherName || null,
        fatherDocument: rest.fatherDocument || null,
        fatherPhone: rest.fatherPhone || null,
        motherName: rest.motherName || null,
        motherDocument: rest.motherDocument || null,
        motherPhone: rest.motherPhone || null,
        guardianName: rest.guardianName || null,
        guardianDocument: rest.guardianDocument || null,
        guardianPhone: rest.guardianPhone || null,
        guardianRelation: rest.guardianRelation || null,
        position: rest.position || null,
        preferredFoot: rest.preferredFoot || null,
        jerseyNumber: rest.jerseyNumber ? Number(rest.jerseyNumber) : null,
        status: rest.status,
        notes: rest.notes || null,
      }
      await updateStudent(studentId, patch)
      // bloodType vive no HealthRecord (fonte única com a ficha de saúde)
      if ((bloodType ?? '') !== '') {
        await saveStudentHealthRecord(studentId, { bloodType })
      }
      snackbar.success(t('students.saved'))
      onSaved()
      onClose()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setSaving(false)
    }
  }

  const birthForInput = form.birthDate ? String(form.birthDate).slice(0, 10) : ''

  return (
    <Dialog open={open} onClose={onClose} maxWidth="md" fullWidth>
      <DialogTitle>{t('students.edit_title')}</DialogTitle>
      <DialogContent dividers>
        {loading ? (
          <Box sx={{ display: 'flex', justifyContent: 'center', py: 6 }}>
            <CircularProgress />
          </Box>
        ) : (
          <Stack spacing={3} sx={{ pt: 1 }}>
            {/* Topo: foto + identificação (pedido do Vinicius: foto junto do nome) */}
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={2} sx={{ alignItems: 'center' }}>
              <Stack spacing={1} sx={{ alignItems: 'center' }}>
                <Avatar src={photoUrl ?? undefined} sx={{ width: 96, height: 96, fontSize: 32 }}>
                  {!photoUrl && (form.name?.trim()?.[0]?.toUpperCase() ?? '?')}
                </Avatar>
                <Stack direction="row" spacing={0.5}>
                  <Button size="small" onClick={() => fileRef.current?.click()} disabled={uploading}>
                    {uploading ? '…' : t('students.change_photo')}
                  </Button>
                  {photoUrl && (
                    <Button size="small" color="error" onClick={() => void handleRemovePhoto()} disabled={uploading}>
                      {t('common.remove')}
                    </Button>
                  )}
                </Stack>
                <input ref={fileRef} type="file" accept="image/*" hidden onChange={(e) => void handlePhotoSelected(e)} />
              </Stack>
              <Stack spacing={1.5} sx={{ flex: 1, width: '100%' }}>
                <TextField label={t('common.name')} value={form.name ?? ''} onChange={(e) => set('name', e.target.value)} required size="small" fullWidth />
                <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                  <TextField label={t('students.field.birthDate')} type="date" value={birthForInput} onChange={(e) => set('birthDate', e.target.value)} size="small" fullWidth InputLabelProps={{ shrink: true }} />
                  <TextField select label={t('common.status')} value={form.status ?? 'ACTIVE'} onChange={(e) => set('status', e.target.value as StudentStatus)} size="small" fullWidth>
                    <MenuItem value="ACTIVE">{t('students.status.ACTIVE')}</MenuItem>
                    <MenuItem value="INACTIVE">{t('students.status.INACTIVE')}</MenuItem>
                    <MenuItem value="SUSPENDED">{t('students.status.SUSPENDED')}</MenuItem>
                  </TextField>
                </Stack>
              </Stack>
            </Stack>

            <Section title={t('students.section.personal')}>
              <Row>
                <TextField label={t('students.field.document')} value={form.document ?? ''} onChange={(e) => set('document', e.target.value)} size="small" fullWidth />
                <TextField select label={t('students.field.gender')} value={form.gender ?? ''} onChange={(e) => set('gender', e.target.value)} size="small" fullWidth>
                  <MenuItem value="">—</MenuItem>
                  <MenuItem value="M">{t('students.gender.M')}</MenuItem>
                  <MenuItem value="F">{t('students.gender.F')}</MenuItem>
                  <MenuItem value="OUTRO">{t('students.gender.OTHER')}</MenuItem>
                </TextField>
                <TextField select label={t('students.field.bloodType')} value={form.bloodType ?? ''} onChange={(e) => set('bloodType', e.target.value)} size="small" fullWidth>
                  <MenuItem value="">—</MenuItem>
                  {BLOOD_TYPES.map((b) => <MenuItem key={b} value={b}>{b}</MenuItem>)}
                </TextField>
              </Row>
              <Row>
                <TextField label={t('common.phone')} value={form.phone ?? ''} onChange={(e) => set('phone', e.target.value)} size="small" fullWidth />
                <TextField label={t('students.field.whatsapp')} value={form.whatsapp ?? ''} onChange={(e) => set('whatsapp', e.target.value)} size="small" fullWidth />
                <TextField label={t('students.field.email')} value={form.email ?? ''} onChange={(e) => set('email', e.target.value)} size="small" fullWidth />
              </Row>
            </Section>

            <Section title={t('students.section.parents')}>
              <Typography variant="caption" color="text.secondary">{t('students.section.father')}</Typography>
              <Row>
                <TextField label={t('common.name')} value={form.fatherName ?? ''} onChange={(e) => set('fatherName', e.target.value)} size="small" fullWidth />
                <TextField label={t('students.field.document')} value={form.fatherDocument ?? ''} onChange={(e) => set('fatherDocument', e.target.value)} size="small" fullWidth />
                <TextField label={t('common.phone')} value={form.fatherPhone ?? ''} onChange={(e) => set('fatherPhone', e.target.value)} size="small" fullWidth />
              </Row>
              <Typography variant="caption" color="text.secondary" sx={{ mt: 1 }}>{t('students.section.mother')}</Typography>
              <Row>
                <TextField label={t('common.name')} value={form.motherName ?? ''} onChange={(e) => set('motherName', e.target.value)} size="small" fullWidth />
                <TextField label={t('students.field.document')} value={form.motherDocument ?? ''} onChange={(e) => set('motherDocument', e.target.value)} size="small" fullWidth />
                <TextField label={t('common.phone')} value={form.motherPhone ?? ''} onChange={(e) => set('motherPhone', e.target.value)} size="small" fullWidth />
              </Row>
            </Section>

            <Section title={t('students.section.guardian')}>
              <Row>
                <TextField label={t('common.name')} value={form.guardianName ?? ''} onChange={(e) => set('guardianName', e.target.value)} size="small" fullWidth />
                <TextField label={t('students.field.document')} value={form.guardianDocument ?? ''} onChange={(e) => set('guardianDocument', e.target.value)} size="small" fullWidth />
                <TextField label={t('common.phone')} value={form.guardianPhone ?? ''} onChange={(e) => set('guardianPhone', e.target.value)} size="small" fullWidth />
                <TextField label={t('students.field.guardianRelation')} value={form.guardianRelation ?? ''} onChange={(e) => set('guardianRelation', e.target.value)} size="small" fullWidth />
              </Row>
            </Section>

            <Section title={t('students.section.address')}>
              <Row>
                <TextField label={t('students.field.zipCode')} value={form.zipCode ?? ''} onChange={(e) => set('zipCode', e.target.value)} size="small" sx={{ flex: 1 }} fullWidth />
                <TextField label={t('students.field.street')} value={form.street ?? ''} onChange={(e) => set('street', e.target.value)} size="small" sx={{ flex: 3 }} fullWidth />
                <TextField label={t('students.field.number')} value={form.number ?? ''} onChange={(e) => set('number', e.target.value)} size="small" sx={{ flex: 1 }} fullWidth />
              </Row>
              <Row>
                <TextField label={t('students.field.neighborhood')} value={form.neighborhood ?? ''} onChange={(e) => set('neighborhood', e.target.value)} size="small" fullWidth />
                <TextField label={t('students.field.city')} value={form.city ?? ''} onChange={(e) => set('city', e.target.value)} size="small" fullWidth />
                <TextField label={t('students.field.state')} value={form.state ?? ''} onChange={(e) => set('state', e.target.value)} size="small" sx={{ maxWidth: 100 }} fullWidth />
              </Row>
            </Section>

            <Section title={t('students.section.sports')}>
              <Row>
                <TextField label={t('students.position')} value={form.position ?? ''} onChange={(e) => set('position', e.target.value)} size="small" fullWidth />
                <TextField select label={t('students.field.preferredFoot')} value={form.preferredFoot ?? ''} onChange={(e) => set('preferredFoot', (e.target.value || null) as FootPreference | null)} size="small" fullWidth>
                  <MenuItem value="">—</MenuItem>
                  <MenuItem value="RIGHT">{t('students.foot.RIGHT')}</MenuItem>
                  <MenuItem value="LEFT">{t('students.foot.LEFT')}</MenuItem>
                  <MenuItem value="BOTH">{t('students.foot.BOTH')}</MenuItem>
                </TextField>
                <TextField label={t('students.field.jerseyNumber')} type="number" value={form.jerseyNumber ?? ''} onChange={(e) => set('jerseyNumber', e.target.value ? Number(e.target.value) : null)} size="small" fullWidth inputProps={{ min: 1, max: 99 }} />
              </Row>
            </Section>

            <Divider />
            <TextField label={t('students.field.notes')} value={form.notes ?? ''} onChange={(e) => set('notes', e.target.value)} size="small" fullWidth multiline minRows={2} />
          </Stack>
        )}
      </DialogContent>
      <DialogActions>
        <Button onClick={onClose} disabled={saving}>{t('common.cancel')}</Button>
        <Button variant="contained" onClick={() => void handleSave()} disabled={saving || loading}>
          {saving ? '…' : t('common.save')}
        </Button>
      </DialogActions>
    </Dialog>
  )
}

function Section({ title, children }: { title: string; children: React.ReactNode }) {
  return (
    <Stack spacing={1.5}>
      <Typography variant="subtitle2" sx={{ fontWeight: 700 }}>{title}</Typography>
      {children}
    </Stack>
  )
}

function Row({ children }: { children: React.ReactNode }) {
  return (
    <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
      {children}
    </Stack>
  )
}
