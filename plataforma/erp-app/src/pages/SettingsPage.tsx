import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import {
  Box,
  Button,
  Checkbox,
  Chip,
  FormControlLabel,
  Link as MuiLink,
  MenuItem,
  Paper,
  Stack,
  TextField,
  Typography,
} from '@mui/material'
import { api, toggleModule, type TenantModule } from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { useModules } from '../modules/ModulesContext'

interface AssistantConfig {
  llmProvider: string
  model: string
  hasApiKey: boolean
  apiKeyMasked: string | null
  soulPrompt: string | null
  active: boolean
}

const MODELS = [
  { id: 'claude-haiku-4-5', label: 'Claude Haiku 4.5 (rápido, barato)' },
  { id: 'claude-sonnet-4-6', label: 'Claude Sonnet 4.6 (qualidade alta)' },
  { id: 'claude-opus-4-7', label: 'Claude Opus 4.7 (premium)' },
]

export function SettingsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [cfg, setCfg] = useState<AssistantConfig | null>(null)
  const [busy, setBusy] = useState(false)
  const [form, setForm] = useState({ model: '', apiKey: '', soulPrompt: '' })
  const [showKey, setShowKey] = useState(false)

  async function load() {
    try {
      const c = await api<AssistantConfig>('/api/assistant/config')
      setCfg(c)
      setForm({
        model: c.model,
        apiKey: '',
        soulPrompt: c.soulPrompt ?? '',
      })
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }
  useEffect(() => { void load() }, [])

  async function save() {
    setBusy(true)
    try {
      const body: Record<string, string | null> = {
        model: form.model,
      }
      if (form.apiKey.trim()) body.apiKey = form.apiKey.trim()
      if (form.soulPrompt.trim()) body.soulPrompt = form.soulPrompt.trim()
      const updated = await api<AssistantConfig>('/api/assistant/config', {
        method: 'PUT',
        body: JSON.stringify(body),
      })
      setCfg(updated)
      setForm({ ...form, apiKey: '' })
      snackbar.success('Configurações salvas')
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  async function clearKey() {
    if (!confirm('Remover a chave de API?')) return
    setBusy(true)
    try {
      await api('/api/assistant/config', {
        method: 'PUT',
        body: JSON.stringify({ apiKey: null }),
      })
      await load()
      snackbar.success('Chave removida')
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mt: 0 }}>{t('settings.title')}</Typography>
      <Typography color="text.secondary" sx={{ mt: 0, mb: 2 }}>{t('settings.subtitle')}</Typography>

      <Can I="update" a="Theme">
        <CompanyCard />
      </Can>

      <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
        <Stack direction="row" spacing={1} sx={{ alignItems: 'flex-start', justifyContent: 'space-between' }}>
          <Box>
            <Typography variant="h6" sx={{ fontSize: 16 }}>{t('settings.assistant_title')}</Typography>
            <Typography color="text.secondary" sx={{ fontSize: 13 }}>
              {t('settings.assistant_subtitle')}
            </Typography>
          </Box>
          {cfg?.hasApiKey && (
            <Chip color="success" size="small" label={t('settings.configured')} />
          )}
        </Stack>

        {!cfg && <Typography color="text.secondary" sx={{ mt: 1 }}>{t('common.loading')}</Typography>}
        {cfg && (
          <Stack spacing={2} sx={{ mt: 2 }}>
            <TextField
              select
              disabled
              size="small"
              label={t('settings.provider')}
              value="anthropic"
            >
              <MenuItem value="anthropic">Anthropic (Claude)</MenuItem>
            </TextField>

            <TextField
              select
              size="small"
              label={t('settings.model')}
              value={form.model}
              onChange={(e) => setForm({ ...form, model: e.target.value })}
            >
              {MODELS.map((m) => <MenuItem key={m.id} value={m.id}>{m.label}</MenuItem>)}
            </TextField>

            <Box>
              <Stack direction="row" spacing={1} sx={{ alignItems: 'flex-start' }}>
                <TextField
                  type={showKey ? 'text' : 'password'}
                  size="small"
                  fullWidth
                  label={t('settings.api_key')}
                  value={form.apiKey}
                  onChange={(e) => setForm({ ...form, apiKey: e.target.value })}
                  placeholder={cfg.hasApiKey ? t('settings.replace_key_placeholder') : 'sk-ant-...'}
                  helperText={
                    cfg.apiKeyMasked
                      ? `${t('settings.current')}: ${cfg.apiKeyMasked}`
                      : t('settings.api_key_hint')
                  }
                  slotProps={{ htmlInput: { style: { fontFamily: 'monospace' } } }}
                />
                <Button type="button" variant="outlined" onClick={() => setShowKey(!showKey)} sx={{ height: 40 }}>
                  {showKey ? '🙈' : '👁'}
                </Button>
                {cfg.hasApiKey && (
                  <Button type="button" variant="outlined" color="error" onClick={clearKey} disabled={busy} sx={{ height: 40 }}>
                    {t('settings.clear')}
                  </Button>
                )}
              </Stack>
            </Box>

            <TextField
              multiline
              minRows={4}
              size="small"
              label={t('settings.soul_prompt')}
              value={form.soulPrompt}
              onChange={(e) => setForm({ ...form, soulPrompt: e.target.value })}
              placeholder={t('settings.soul_prompt_placeholder')}
              helperText={t('settings.soul_prompt_hint')}
              slotProps={{ htmlInput: { style: { fontFamily: 'monospace', fontSize: 13 } } }}
            />

            <Stack direction="row" spacing={1} sx={{ justifyContent: 'flex-end' }}>
              <Button variant="contained" onClick={save} disabled={busy}>
                {busy ? '…' : t('common.save')}
              </Button>
            </Stack>
          </Stack>
        )}
      </Paper>

      <ModulesCard />

      <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
        <Stack direction="row" spacing={1} sx={{ alignItems: 'center', justifyContent: 'space-between' }}>
          <Typography variant="h6" sx={{ fontSize: 16 }}>{t('settings.theme_title')}</Typography>
          <MuiLink component={Link} to="/theme" sx={{ fontSize: 14 }}>{t('settings.go_to')}</MuiLink>
        </Stack>
        <Typography color="text.secondary" sx={{ mt: 1 }}>
          {t('settings.theme_subtitle')}
        </Typography>
      </Paper>
    </Layout>
  )
}

function ModulesCard() {
  const t = useT()
  const snackbar = useSnackbar()
  const { packSlug, modules, loading, refresh } = useModules()
  const [busy, setBusy] = useState<string | null>(null)

  async function onToggle(m: TenantModule, next: boolean) {
    setBusy(m.slug)
    try {
      await toggleModule(m.slug, next)
      await refresh()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(null)
    }
  }

  // Agrupar por categoria, ocultar infra de plataforma sem tela própria
  const visible = modules.filter((m) => m.routePath)
  const byCategory = visible.reduce<Record<string, TenantModule[]>>((acc, m) => {
    ;(acc[m.category] ??= []).push(m)
    return acc
  }, {})
  const order: TenantModule['category'][] = ['REGISTRY', 'INVENTORY', 'SALES', 'SERVICE', 'FOOD', 'EVENT', 'EDUCATION', 'HEALTH', 'SCHOOL', 'SYSTEM', 'FISCAL', 'AI', 'CORE']

  return (
    <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
      <Stack direction="row" spacing={1} sx={{ alignItems: 'center', justifyContent: 'space-between' }}>
        <Box>
          <Typography variant="h6" sx={{ fontSize: 16 }}>{t('settings.modules_title')}</Typography>
          <Typography color="text.secondary" sx={{ fontSize: 13 }}>
            {t('settings.modules_subtitle')}
          </Typography>
        </Box>
        <Stack direction="row" spacing={1} sx={{ alignItems: 'center' }}>
          {packSlug && (
            <Chip size="small" variant="outlined" label={
              <span>{t('settings.modules_current_pack')}: <strong>{packSlug}</strong></span>
            } />
          )}
          <MuiLink component={Link} to="/modules" sx={{ fontSize: 14 }}>
            {t('settings.manage_modules')} →
          </MuiLink>
        </Stack>
      </Stack>

      {loading && <Typography color="text.secondary" sx={{ mt: 2 }}>{t('common.loading')}</Typography>}

      {!loading && (
        <Stack spacing={1.5} sx={{ mt: 2 }}>
          {order
            .filter((cat) => byCategory[cat]?.length)
            .map((cat) => (
              <Box key={cat}>
                <Typography variant="overline" color="text.secondary" sx={{ display: 'block', mb: 0.5 }}>
                  {t(`modules.cat.${cat}`)}
                </Typography>
                <Stack spacing={0.5}>
                  {byCategory[cat]
                    .sort((a, b) => a.sortOrder - b.sortOrder)
                    .map((m) => (
                      <FormControlLabel
                        key={m.slug}
                        title={m.isCore ? t('settings.modules_core_hint') : undefined}
                        disabled={m.isCore || busy === m.slug}
                        control={
                          <Checkbox
                            checked={m.enabled}
                            onChange={(e) => onToggle(m, e.target.checked)}
                          />
                        }
                        label={
                          <Stack direction="row" spacing={1} sx={{ alignItems: 'center' }}>
                            <span>{m.name}</span>
                            {m.isCore && (
                              <Typography variant="caption" color="text.secondary">
                                {t('settings.modules_core_badge')}
                              </Typography>
                            )}
                          </Stack>
                        }
                      />
                    ))}
                </Stack>
              </Box>
            ))}
        </Stack>
      )}
    </Paper>
  )
}

const COMPANY_FIELDS = [
  'companyName', 'document', 'responsible', 'email', 'phone', 'phone2',
  'instagram', 'zipCode', 'street', 'number', 'complement', 'neighborhood',
  'cityName', 'state', 'paymentMethods', 'paymentTerms',
] as const
type CompanyField = (typeof COMPANY_FIELDS)[number]
type CompanyForm = Record<CompanyField, string>

const emptyCompany = (): CompanyForm =>
  COMPANY_FIELDS.reduce((acc, k) => ({ ...acc, [k]: '' }), {} as CompanyForm)

function CompanyCard() {
  const t = useT()
  const snackbar = useSnackbar()
  const [form, setForm] = useState<CompanyForm>(emptyCompany())
  const [loaded, setLoaded] = useState(false)
  const [busy, setBusy] = useState(false)

  useEffect(() => {
    api<Partial<Record<CompanyField, unknown>>>('/api/tenants/company')
      .then((c) => {
        const next = emptyCompany()
        for (const k of COMPANY_FIELDS) next[k] = c[k] == null ? '' : String(c[k])
        setForm(next)
        setLoaded(true)
      })
      .catch((e) => snackbar.error((e as Error).message))
  }, [snackbar])

  const set = (k: CompanyField, v: string) => setForm((f) => ({ ...f, [k]: v }))

  async function save() {
    setBusy(true)
    try {
      await api('/api/tenants/company', { method: 'PATCH', body: JSON.stringify(form) })
      snackbar.success(t('company.saved'))
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  const field = (k: CompanyField, opts?: { multiline?: boolean; flex?: number }) => (
    <TextField
      key={k}
      size="small"
      fullWidth
      multiline={opts?.multiline}
      minRows={opts?.multiline ? 2 : undefined}
      label={t(`company.${k}`)}
      value={form[k]}
      onChange={(e) => set(k, e.target.value)}
      sx={opts?.flex ? { flex: opts.flex } : undefined}
    />
  )

  return (
    <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
      <Box sx={{ mb: 1.5 }}>
        <Typography variant="h6" sx={{ fontSize: 16 }}>{t('company.title')}</Typography>
        <Typography color="text.secondary" sx={{ fontSize: 13 }}>{t('company.subtitle')}</Typography>
      </Box>

      {!loaded ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <Stack spacing={1.5}>
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
            {field('companyName', { flex: 2 })}
            {field('document', { flex: 1 })}
          </Stack>
          {field('responsible')}
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
            {field('email', { flex: 2 })}
            {field('phone', { flex: 1 })}
            {field('phone2', { flex: 1 })}
          </Stack>
          {field('instagram')}
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
            {field('zipCode', { flex: 1 })}
            {field('street', { flex: 2 })}
            {field('number', { flex: 1 })}
          </Stack>
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
            {field('complement', { flex: 1 })}
            {field('neighborhood', { flex: 1 })}
            {field('cityName', { flex: 1 })}
            {field('state', { flex: 1 })}
          </Stack>
          {field('paymentMethods', { multiline: true })}
          {field('paymentTerms', { multiline: true })}
          <Stack direction="row" spacing={1} sx={{ justifyContent: 'flex-end' }}>
            <Button variant="contained" onClick={save} disabled={busy}>
              {busy ? '…' : t('common.save')}
            </Button>
          </Stack>
        </Stack>
      )}
    </Paper>
  )
}
