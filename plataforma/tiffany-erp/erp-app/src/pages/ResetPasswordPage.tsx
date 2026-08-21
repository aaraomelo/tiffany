import { useState } from 'react'
import { Link as RouterLink, useNavigate, useSearchParams } from 'react-router-dom'
import {
  Alert,
  Box,
  Button,
  Link as MuiLink,
  Paper,
  Stack,
  TextField,
  Typography,
} from '@mui/material'
import { ApiError, resetPassword } from '../api'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'
import { ThemeSwitcher } from '../theme/ThemeSwitcher'

export function ResetPasswordPage() {
  const t = useT()
  const navigate = useNavigate()
  const snackbar = useSnackbar()
  const [params] = useSearchParams()
  const token = params.get('token') ?? ''

  const [password, setPassword] = useState('')
  const [confirm, setConfirm] = useState('')
  const [loading, setLoading] = useState(false)

  const mismatch = confirm.length > 0 && password !== confirm
  const canSubmit = token && password.length >= 8 && password === confirm

  async function handleSubmit(e: React.FormEvent) {
    e.preventDefault()
    if (!canSubmit) return
    setLoading(true)
    try {
      await resetPassword(token, password)
      snackbar.success(t('reset.success'))
      navigate('/login')
    } catch (err) {
      const msg =
        err instanceof ApiError
          ? ((err.body as { message?: string | string[] })?.message as string) ?? t('reset.error')
          : (err as Error).message
      snackbar.error(Array.isArray(msg) ? msg.join(', ') : msg)
    } finally {
      setLoading(false)
    }
  }

  return (
    <Box sx={{ minHeight: '100vh', bgcolor: 'background.default', color: 'text.primary' }}>
      <Box
        component="header"
        sx={{ bgcolor: 'primary.main', px: 3, py: 1, display: 'flex', justifyContent: 'flex-end', gap: 1 }}
      >
        <ThemeSwitcher />
        <LangSwitcher />
      </Box>

      <Box component="main" sx={{ maxWidth: 400, mx: 'auto', mt: { xs: 6, sm: 10 }, px: 2 }}>
        <Typography variant="h4" sx={{ color: 'primary.main', fontWeight: 700, mb: 0.5 }}>
          {t('app.name')}
        </Typography>
        <Typography sx={{ color: 'text.secondary', mb: 3 }}>{t('reset.title')}</Typography>

        <Paper variant="outlined" sx={{ p: 3 }}>
          {!token ? (
            <Stack spacing={2}>
              <Alert severity="error">{t('reset.no_token')}</Alert>
              <MuiLink component={RouterLink} to="/forgot-password" variant="body2">
                {t('reset.request_new')}
              </MuiLink>
            </Stack>
          ) : (
            <Stack component="form" spacing={2} onSubmit={handleSubmit}>
              <TextField
                type="password"
                required
                label={t('reset.new_password')}
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                fullWidth
                size="small"
                helperText={t('signup.password_hint')}
              />
              <TextField
                type="password"
                required
                label={t('reset.confirm_password')}
                value={confirm}
                onChange={(e) => setConfirm(e.target.value)}
                fullWidth
                size="small"
                error={mismatch}
                helperText={mismatch ? t('reset.mismatch') : ' '}
              />
              <Button type="submit" variant="contained" disabled={!canSubmit || loading} size="large">
                {loading ? t('login.submitting') : t('reset.submit')}
              </Button>
            </Stack>
          )}
        </Paper>
      </Box>
    </Box>
  )
}
