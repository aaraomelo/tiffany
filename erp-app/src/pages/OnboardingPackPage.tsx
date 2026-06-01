import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { Box, Button, Chip, Paper, Stack, Typography } from '@mui/material'
import { applyPack, fetchPacks, type ModulePack } from '../api'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'
import { useModules } from '../modules/ModulesContext'
import { ThemeSwitcher } from '../theme/ThemeSwitcher'

export function OnboardingPackPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const navigate = useNavigate()
  const { packSlug, refresh } = useModules()
  const [packs, setPacks] = useState<ModulePack[]>([])
  const [selected, setSelected] = useState<string | null>(null)
  const [loading, setLoading] = useState(true)
  const [saving, setSaving] = useState(false)

  useEffect(() => {
    void (async () => {
      try {
        const list = await fetchPacks()
        setPacks(list)
        const initial = packSlug ?? list.find((p) => p.isDefault)?.slug ?? list[0]?.slug ?? null
        setSelected(initial)
      } catch (e) {
        snackbar.error((e as Error).message)
      } finally {
        setLoading(false)
      }
    })()
  }, [packSlug])

  async function confirm() {
    if (!selected) return
    setSaving(true)
    try {
      await applyPack(selected)
      await refresh()
      navigate('/pos')
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setSaving(false)
    }
  }

  return (
    <Box sx={{ minHeight: '100vh', bgcolor: 'background.default', color: 'text.primary' }}>
      <Stack
        component="header"
        direction="row"
        spacing={2}
        sx={{ alignItems: 'center', bgcolor: 'primary.main', color: 'primary.contrastText', px: 3, py: 1 }}
      >
        <Typography component="strong" sx={{ fontSize: 18, fontWeight: 700, flex: 1 }}>{t('app.name')}</Typography>
        <ThemeSwitcher />
        <LangSwitcher />
      </Stack>

      <Box component="main" sx={{ maxWidth: 880, mx: 'auto', my: 5, px: 3 }}>
        <Typography variant="h5" sx={{ mt: 0, fontWeight: 700, color: 'primary.main' }}>{t('onboarding.pack.title')}</Typography>
        <Typography color="text.secondary" sx={{ mt: 0, mb: 4 }}>
          {t('onboarding.pack.subtitle')}
        </Typography>

        {loading && <Typography color="text.secondary">{t('common.loading')}</Typography>}

        {!loading && (
          <>
            <Box
              sx={{
                display: 'grid',
                gap: 2,
                gridTemplateColumns: 'repeat(auto-fit, minmax(260px, 1fr))',
              }}
            >
              {packs.map((p) => {
                const active = selected === p.slug
                return (
                  <Paper
                    key={p.slug}
                    variant="outlined"
                    onClick={() => setSelected(p.slug)}
                    sx={{
                      p: 2,
                      cursor: 'pointer',
                      borderWidth: 2,
                      borderColor: active ? 'primary.main' : 'divider',
                      bgcolor: active ? 'action.selected' : 'background.paper',
                    }}
                  >
                    <Stack direction="row" spacing={1} sx={{ alignItems: 'center', mb: 0.5 }}>
                      <Typography variant="h6" sx={{ fontSize: 17 }}>{p.name}</Typography>
                      {p.isDefault && (
                        <Chip color="primary" size="small" label={t('onboarding.pack.recommended')} />
                      )}
                    </Stack>
                    <Typography color="text.secondary" sx={{ fontSize: 13, mb: 1 }}>
                      {p.description}
                    </Typography>
                    <Typography variant="caption" color="text.secondary">
                      {t('onboarding.pack.modules_count', { count: p.items.length })}
                    </Typography>
                  </Paper>
                )
              })}
            </Box>

            <Stack direction="row" spacing={1} sx={{ justifyContent: 'flex-end', mt: 4 }}>
              <Button
                type="button"
                variant="outlined"
                onClick={() => navigate('/settings')}
                disabled={saving}
              >
                {t('onboarding.pack.customize_later')}
              </Button>
              <Button
                type="button"
                variant="contained"
                onClick={confirm}
                disabled={!selected || saving}
              >
                {saving ? '…' : t('onboarding.pack.confirm')}
              </Button>
            </Stack>
          </>
        )}
      </Box>
    </Box>
  )
}
