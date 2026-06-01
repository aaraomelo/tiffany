import { CheckCircle, ErrorOutlined } from '@mui/icons-material'
import {
  Box,
  Button,
  Card,
  CardActionArea,
  CardContent,
  Container,
  InputAdornment,
  Stack,
  TextField,
  Typography,
} from '@mui/material'
import { useEffect, useMemo, useState } from 'react'
import { useNavigate, useSearchParams } from 'react-router-dom'
import {
  api,
  checkAlias,
  fetchSegments,
  setSession,
  signup,
  type Segment,
  type StoredUser,
} from '../api'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'

function slugify(s: string): string {
  return s.toLowerCase().normalize('NFD').replace(/[̀-ͯ]/g, '')
    .replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 32)
}

export function SignupPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const navigate = useNavigate()
  const [params] = useSearchParams()

  const [segments, setSegments] = useState<Segment[]>([])
  const [packSlug, setPackSlug] = useState(params.get('segment') ?? '')
  const [name, setName] = useState('')
  const [alias, setAlias] = useState('')
  const [aliasTouched, setAliasTouched] = useState(false)
  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [aliasState, setAliasState] = useState<'idle' | 'checking' | 'ok' | 'taken' | 'invalid'>('idle')
  const [busy, setBusy] = useState(false)

  useEffect(() => {
    fetchSegments().then((s) => {
      setSegments(s)
      if (!params.get('segment')) setPackSlug((cur) => cur || s.find((x) => x.isDefault)?.slug || s[0]?.slug || '')
    }).catch(() => {})
  }, [])

  // alias acompanha o nome até o usuário editar manualmente
  const suggested = useMemo(() => slugify(name), [name])
  const effectiveAlias = aliasTouched ? alias : suggested

  useEffect(() => {
    const a = effectiveAlias
    if (!a) { setAliasState('idle'); return }
    setAliasState('checking')
    const id = setTimeout(() => {
      checkAlias(a)
        .then((r) => setAliasState(r.available ? 'ok' : (r.reason === 'invalid' ? 'invalid' : 'taken')))
        .catch(() => setAliasState('idle'))
    }, 400)
    return () => clearTimeout(id)
  }, [effectiveAlias])

  const canSubmit = packSlug && name.trim() && effectiveAlias && aliasState === 'ok' && email.trim() && password.length >= 8

  async function submit(e: React.FormEvent) {
    e.preventDefault()
    if (!canSubmit) return
    setBusy(true)
    try {
      await signup({ alias: effectiveAlias, name, packSlug, adminEmail: email, adminName: name, adminPassword: password })
      // auto-login (fricção zero)
      const res = await api<{ accessToken: string; user: StoredUser }>('/api/auth/login', {
        method: 'POST',
        body: JSON.stringify({ email, password, tenantAlias: effectiveAlias }),
      })
      setSession(res.accessToken, res.user)
      snackbar.success(t('signup.welcome', { name }))
      navigate('/site')
    } catch (err) {
      const body = (err as { body?: { message?: string | string[] } }).body
      const msg = Array.isArray(body?.message) ? body!.message.join(', ') : (body?.message ?? (err as Error).message)
      snackbar.error(msg)
    } finally {
      setBusy(false)
    }
  }

  return (
    <Box sx={{ minHeight: '100vh', bgcolor: 'background.default' }}>
      <Box sx={{ bgcolor: 'primary.main', color: 'primary.contrastText', px: 2, py: 1, display: 'flex', alignItems: 'center', gap: 1 }}>
        <Typography variant="h6" sx={{ fontWeight: 700, flex: 1 }}>Patria Technology</Typography>
        <LangSwitcher />
      </Box>

      <Container maxWidth="sm" sx={{ py: { xs: 4, md: 6 } }}>
        <Typography variant="h4" sx={{ fontWeight: 800 }}>{t('signup.title')}</Typography>
        <Typography color="text.secondary" sx={{ mt: 0.5, mb: 3 }}>{t('signup.subtitle')}</Typography>

        <Box component="form" onSubmit={submit}>
          <Stack spacing={2.5}>
            <Box>
              <Typography variant="subtitle2" sx={{ fontWeight: 700, mb: 1 }}>{t('signup.segment')}</Typography>
              <Box sx={{ display: 'grid', gap: 1, gridTemplateColumns: { xs: '1fr', sm: 'repeat(2, 1fr)' } }}>
                {segments.map((s) => (
                  <Card key={s.slug} variant="outlined" sx={{ borderColor: packSlug === s.slug ? 'primary.main' : 'divider', borderWidth: packSlug === s.slug ? 2 : 1 }}>
                    <CardActionArea onClick={() => setPackSlug(s.slug)}>
                      <CardContent sx={{ py: 1.5 }}>
                        <Typography sx={{ fontWeight: 600 }}>{s.name}</Typography>
                        <Typography variant="caption" color="text.secondary">{s.description}</Typography>
                      </CardContent>
                    </CardActionArea>
                  </Card>
                ))}
              </Box>
            </Box>

            <TextField label={t('signup.business_name')} value={name} onChange={(e) => setName(e.target.value)} required size="small" fullWidth />

            <TextField
              label={t('signup.address')}
              value={effectiveAlias}
              onChange={(e) => { setAliasTouched(true); setAlias(slugify(e.target.value)) }}
              required size="small" fullWidth
              helperText={aliasState === 'taken' ? t('signup.alias_taken') : aliasState === 'invalid' ? t('signup.alias_invalid') : t('signup.address_hint', { alias: effectiveAlias || 'seu-negocio' })}
              error={aliasState === 'taken' || aliasState === 'invalid'}
              slotProps={{
                input: {
                  endAdornment: (
                    <InputAdornment position="end">
                      {aliasState === 'ok' && <CheckCircle color="success" fontSize="small" />}
                      {(aliasState === 'taken' || aliasState === 'invalid') && <ErrorOutlined color="error" fontSize="small" />}
                    </InputAdornment>
                  ),
                },
              }}
            />

            <TextField label={t('common.email')} type="email" value={email} onChange={(e) => setEmail(e.target.value)} required size="small" fullWidth />
            <TextField label={t('login.password')} type="password" value={password} onChange={(e) => setPassword(e.target.value)} required size="small" fullWidth helperText={t('signup.password_hint')} />

            <Button type="submit" variant="contained" size="large" disabled={!canSubmit || busy}>
              {busy ? '…' : t('signup.submit')}
            </Button>
            <Typography variant="caption" color="text.secondary" sx={{ textAlign: 'center' }}>{t('signup.free_note')}</Typography>
          </Stack>
        </Box>
      </Container>
    </Box>
  )
}
