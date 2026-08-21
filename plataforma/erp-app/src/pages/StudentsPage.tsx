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
import { StudentAvatar } from '../components/StudentAvatar'
import { StudentEditDialog } from '../components/StudentEditDialog'
import { useT } from '../i18n/LangContext'

type StudentStatus = 'ACTIVE' | 'INACTIVE' | 'SUSPENDED'

interface Student {
  id: string
  name: string
  document: string | null
  phone: string | null
  guardianName: string | null
  position: string | null
  status: StudentStatus
  photoUpdatedAt: string | null
  createdAt: string
}

interface ListResponse {
  items: Student[]
  total: number
  page: number
  pageSize: number
}

export function StudentsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<ListResponse | null>(null)
  const [q, setQ] = useState('')
  const [status, setStatus] = useState<StudentStatus | ''>('')
  const [editingId, setEditingId] = useState<string | null>(null)
  const [dialogOpen, setDialogOpen] = useState(false)

  async function load() {
    const params = new URLSearchParams()
    if (q) params.set('q', q)
    if (status) params.set('status', status)
    try {
      const res = await api<ListResponse>(`/api/students?${params.toString()}`)
      setData(res)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  function openCreate() { setEditingId(null); setDialogOpen(true) }
  function openEdit(id: string) { setEditingId(id); setDialogOpen(true) }

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 2 }}>{t('students.title')}</Typography>

      <Can I="create" a="Student">
        <Box sx={{ mb: 3 }}>
          <Button variant="contained" onClick={openCreate}>{t('students.new')}</Button>
        </Box>
      </Can>

      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} sx={{ mb: 2 }}>
        <TextField placeholder={t('students.search_placeholder')} value={q} onChange={(e) => setQ(e.target.value)} size="small" sx={{ flex: 1 }} />
        <TextField select value={status} onChange={(e) => setStatus(e.target.value as StudentStatus | '')} size="small" sx={{ minWidth: 160 }}>
          <MenuItem value="">{t('common.all')}</MenuItem>
          <MenuItem value="ACTIVE">{t('students.status.ACTIVE')}</MenuItem>
          <MenuItem value="INACTIVE">{t('students.status.INACTIVE')}</MenuItem>
          <MenuItem value="SUSPENDED">{t('students.status.SUSPENDED')}</MenuItem>
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
                <TableCell sx={{ width: 56 }}>{t('students.photo')}</TableCell>
                <TableCell>{t('common.name')}</TableCell>
                <TableCell>{t('common.phone')}</TableCell>
                <TableCell>{t('students.guardian')}</TableCell>
                <TableCell>{t('common.status')}</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {data.items.length === 0 && (
                <TableRow><TableCell colSpan={5} sx={{ color: 'text.secondary' }}>{t('students.empty')}</TableCell></TableRow>
              )}
              {data.items.map((s) => (
                <TableRow key={s.id} hover sx={{ cursor: 'pointer' }} onClick={() => openEdit(s.id)}>
                  <TableCell><StudentAvatar studentId={s.id} name={s.name} photoUpdatedAt={s.photoUpdatedAt} size={36} /></TableCell>
                  <TableCell>{s.name}</TableCell>
                  <TableCell>{s.phone ?? '—'}</TableCell>
                  <TableCell>{s.guardianName ?? '—'}</TableCell>
                  <TableCell>{t(`students.status.${s.status}`)}</TableCell>
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

      <StudentEditDialog
        studentId={editingId}
        open={dialogOpen}
        onClose={() => setDialogOpen(false)}
        onSaved={() => void load()}
      />
    </Layout>
  )
}
