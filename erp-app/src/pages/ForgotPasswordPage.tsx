import { useState } from 'react'
import { Link as RouterLink } from 'react-router-dom'
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
import { forgotPassword } from '../api'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'
import { ThemeSwitcher } from '../theme/ThemeSwitcher'

export function ForgotPasswordPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [email, setEmail] = useState('')
  const [loading, setLoading] = useState(false)
  const [sent, setSent] = useState(false)

  async function handleSubmit(e: React.FormEvent) {
    e.preventDefault()
    setLoading(true)
    try {
      await forgotPassword(email)
      setSent(true)
    } catch {
      // resposta é sempre genérica; tratamos como enviado mesmo em erro
      setSent(true)
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
        <Typography sx={{ color: 'text.secondary', mb: 3 }}>{t('forgot.title')}</Typography>

        <Paper variant="outlined" sx={{ p: 3 }}>
          {sent ? (
            <Stack spacing={2}>
              <Alert severity="success">{t('forgot.sent')}</Alert>
              <MuiLink component={RouterLink} to="/login" variant="body2">
                {t('forgot.back_to_login')}
              </MuiLink>
            </Stack>
          ) : (
            <Stack component="form" spacing={2} onSubmit={handleSubmit}>
              <Typography variant="body2" color="text.secondary">
                {t('forgot.subtitle')}
              </Typography>
              <TextField
                type="email"
                required
                label={t('login.email')}
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                fullWidth
                size="small"
              />
              <Button type="submit" variant="contained" disabled={loading} size="large">
                {loading ? t('login.submitting') : t('forgot.submit')}
              </Button>
              <MuiLink component={RouterLink} to="/login" variant="body2" sx={{ textAlign: 'center' }}>
                {t('forgot.back_to_login')}
              </MuiLink>
            </Stack>
          )}
        </Paper>
      </Box>
    </Box>
  )
}
