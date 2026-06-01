import { useEffect, useState } from 'react'
import {
  Box,
  Button,
  MenuItem,
  Paper,
  Stack,
  TextField,
  Typography,
} from '@mui/material'
import { api } from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

interface Cash { id: string; name: string }
interface Session {
  id: string
  cashId: string
  status: 'OPEN' | 'PARTIALLY_CLOSED' | 'CLOSED'
  openingAmount: string
  closingAmount: string | null
  openedAt: string
  closedAt: string | null
  openedBy?: { id: string; name: string }
}

const ACTIVE_SESSION_KEY = 'erp.activeSession'

export function setActiveSession(s: { cashId: string; sessionId: string } | null) {
  if (s) localStorage.setItem(ACTIVE_SESSION_KEY, JSON.stringify(s))
  else localStorage.removeItem(ACTIVE_SESSION_KEY)
}
export function getActiveSession(): { cashId: string; sessionId: string } | null {
  const raw = localStorage.getItem(ACTIVE_SESSION_KEY)
  return raw ? JSON.parse(raw) : null
}

export function CashPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [cashes, setCashes] = useState<Cash[]>([])
  const [selected, setSelected] = useState<string | null>(null)
  const [session, setSession] = useState<Session | null>(null)
  const [openAmt, setOpenAmt] = useState('100.00')
  const [closeAmt, setCloseAmt] = useState('')
  const [opAmt, setOpAmt] = useState('')
  const [opNotes, setOpNotes] = useState('')
  const [newCashName, setNewCashName] = useState('')

  async function refresh() {
    try {
      const list = await api<Cash[]>('/api/cash')
      setCashes(list)
      const sel = selected ?? list[0]?.id ?? null
      setSelected(sel)
      if (sel) {
        const s = await api<Session | null>(`/api/cash/${sel}/session/current`)
        setSession(s)
        if (s) setActiveSession({ cashId: sel, sessionId: s.id })
        else setActiveSession(null)
      } else {
        setSession(null)
        setActiveSession(null)
      }
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void refresh() }, [])
  useEffect(() => { void refresh() }, [selected])

  async function createCash(e: React.FormEvent) {
    e.preventDefault()
    if (!newCashName.trim()) return
    try {
      await api('/api/cash', { method: 'POST', body: JSON.stringify({ name: newCashName }) })
      setNewCashName('')
      void refresh()
    } catch (e) { snackbar.error((e as Error).message) }
  }

  async function openSession() {
    if (!selected) return
    try {
      await api(`/api/cash/${selected}/session/open`, {
        method: 'POST',
        body: JSON.stringify({ openingAmount: Number(openAmt) }),
      })
      void refresh()
    } catch (e) { snackbar.error((e as Error).message) }
  }

  async function closeSession() {
    if (!selected || !session) return
    try {
      await api(`/api/cash/${selected}/session/${session.id}/close`, {
        method: 'POST',
        body: JSON.stringify({ closingAmount: Number(closeAmt) }),
      })
      setActiveSession(null)
      setCloseAmt('')
      void refresh()
    } catch (e) { snackbar.error((e as Error).message) }
  }

  async function operation(kind: 'withdrawal' | 'reinforcement') {
    if (!selected || !session) return
    try {
      await api(`/api/cash/${selected}/${kind}`, {
        method: 'POST',
        body: JSON.stringify({ amount: Number(opAmt), notes: opNotes || undefined }),
      })
      setOpAmt(''); setOpNotes('')
      void refresh()
    } catch (e) { snackbar.error((e as Error).message) }
  }

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 2 }}>{t('cash.title')}</Typography>

      <Paper variant="outlined" sx={{ p: 2, mb: 3 }}>
        <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('cash.list')}</Typography>
        {cashes.length === 0 && <Typography color="text.secondary" sx={{ mb: 1.5 }}>{t('cash.no_cash')}</Typography>}
        {cashes.length > 0 && (
          <TextField select value={selected ?? ''} onChange={(e) => setSelected(e.target.value)} size="small" sx={{ mb: 1.5, minWidth: 220 }} fullWidth>
            {cashes.map((c) => <MenuItem key={c.id} value={c.id}>{c.name}</MenuItem>)}
          </TextField>
        )}
        <Can I="create" a="Cash">
          <Box component="form" onSubmit={createCash}>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ alignItems: { sm: 'flex-start' } }}>
              <TextField placeholder={t('cash.new_placeholder')} value={newCashName} onChange={(e) => setNewCashName(e.target.value)} size="small" sx={{ flex: 1 }} fullWidth />
              <Button type="submit" variant="contained" sx={{ minWidth: 110, height: 40 }}>{t('cash.add')}</Button>
            </Stack>
          </Box>
        </Can>
      </Paper>

      {selected && (
        <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('cash.session')}</Typography>
          {!session && (
            <Can I="update" a="Cash">
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ alignItems: { sm: 'flex-end' } }}>
                <TextField type="number" label={t('cash.opening_amount')} value={openAmt} onChange={(e) => setOpenAmt(e.target.value)} size="small" sx={{ flex: 1 }} fullWidth slotProps={{ htmlInput: { step: '0.01' } }} />
                <Button variant="contained" onClick={openSession} sx={{ minWidth: 110, height: 40 }}>{t('cash.open')}</Button>
              </Stack>
            </Can>
          )}
          {session && (
            <>
              <Typography variant="body2" sx={{ my: 0.5 }}>
                {t('common.status')}: <strong>{session.status}</strong> · {t('cash.operator')}: {session.openedBy?.name ?? '—'} · {t('cash.opened_at')}: {new Date(session.openedAt).toLocaleString()}
              </Typography>
              <Typography variant="body2" sx={{ my: 0.5 }}>
                {t('cash.opening')}: R$ {Number(session.openingAmount).toFixed(2)}
              </Typography>

              <Can I="update" a="Cash">
                <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ mt: 2, alignItems: { sm: 'flex-end' } }}>
                  <TextField type="number" label={t('cash.value_label')} value={opAmt} onChange={(e) => setOpAmt(e.target.value)} size="small" sx={{ flex: 1 }} fullWidth slotProps={{ htmlInput: { step: '0.01' } }} />
                  <TextField label={t('cash.observation')} value={opNotes} onChange={(e) => setOpNotes(e.target.value)} size="small" sx={{ flex: 2 }} fullWidth />
                  <Button variant="contained" color="warning" onClick={() => operation('withdrawal')} sx={{ minWidth: 110, height: 40 }}>{t('cash.withdrawal')}</Button>
                  <Button variant="contained" color="success" onClick={() => operation('reinforcement')} sx={{ minWidth: 110, height: 40 }}>{t('cash.reinforcement')}</Button>
                </Stack>

                <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ mt: 3, alignItems: { sm: 'flex-end' } }}>
                  <TextField type="number" label={t('cash.closing_amount')} value={closeAmt} onChange={(e) => setCloseAmt(e.target.value)} size="small" sx={{ flex: 1 }} fullWidth slotProps={{ htmlInput: { step: '0.01' } }} />
                  <Button variant="outlined" onClick={closeSession} sx={{ minWidth: 110, height: 40 }}>{t('cash.close')}</Button>
                </Stack>
              </Can>
            </>
          )}
        </Paper>
      )}
    </Layout>
  )
}
