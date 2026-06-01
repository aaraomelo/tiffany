import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { Box, Button, Paper, Stack, TextField, Typography } from '@mui/material'
import { ApiError, api, getTenantAlias, setSession, type StoredUser } from '../api'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'
import { ThemeSwitcher } from '../theme/ThemeSwitcher'

interface LoginResponse {
  accessToken: string
  user: StoredUser
}

export function LoginPage() {
  const navigate = useNavigate()
  const t = useT()
  const snackbar = useSnackbar()
  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [manualAlias, setManualAlias] = useState('')
  const [loading, setLoading] = useState(false)

  // No subdomínio do cliente ({alias}.patriatechnology.com) o tenant é herdado
  // do endereço — não pedimos de novo. Só mostramos o campo no domínio raiz.
  const subdomain = getTenantAlias()
  const tenantAlias = subdomain ?? manualAlias

  async function handleSubmit(e: React.FormEvent) {
    e.preventDefault()
    setLoading(true)
    try {
      const res = await api<LoginResponse>('/api/auth/login', {
        method: 'POST',
        body: JSON.stringify({
          email,
          password,
          ...(tenantAlias ? { tenantAlias } : {}),
        }),
      })
      setSession(res.accessToken, res.user)
      navigate('/customers')
    } catch (err) {
      if (err instanceof ApiError) {
        const body = err.body as { message?: string | string[] }
        const msg = Array.isArray(body?.message)
          ? body.message.join(', ')
          : body?.message ?? `HTTP ${err.status}`
        snackbar.error(msg)
      } else {
        snackbar.error((err as Error).message)
      }
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
        <Typography sx={{ color: 'text.secondary', mb: subdomain ? 0.5 : 3 }}>{t('login.title')}</Typography>
        {subdomain && (
          <Typography variant="body2" sx={{ color: 'text.secondary', mb: 3 }}>
            <strong>{subdomain}</strong>.patriatechnology.com
          </Typography>
        )}

        <Paper variant="outlined" sx={{ p: 3 }}>
          <Stack component="form" spacing={2} onSubmit={handleSubmit}>
            <TextField
              type="email"
              required
              label={t('login.email')}
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              fullWidth
              size="small"
            />
            <TextField
              type="password"
              required
              label={t('login.password')}
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              fullWidth
              size="small"
            />
            {!subdomain && (
              <TextField
                label={`${t('login.tenant_alias')} ${t('common.optional')}`}
                value={manualAlias}
                onChange={(e) => setManualAlias(e.target.value)}
                placeholder="acme"
                fullWidth
                size="small"
              />
            )}
            <Button type="submit" variant="contained" disabled={loading} size="large">
              {loading ? t('login.submitting') : t('login.submit')}
            </Button>
          </Stack>
        </Paper>
      </Box>
    </Box>
  )
}
