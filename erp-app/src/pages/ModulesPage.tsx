import { useEffect, useState } from 'react'
import {
  Box,
  Button,
  Chip,
  FormControlLabel,
  Paper,
  Stack,
  Switch,
  Typography,
} from '@mui/material'
import { fetchPacks, toggleModule, togglePack, type ModulePack, type TenantModule } from '../api'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { useModules } from '../modules/ModulesContext'

const CATEGORY_ORDER: TenantModule['category'][] = [
  'REGISTRY', 'INVENTORY', 'SALES', 'SERVICE', 'FOOD', 'EVENT', 'SCHOOL', 'SYSTEM', 'FISCAL', 'AI', 'CORE',
]

export function ModulesPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const { modules, activePacks, loading, applyResponse, patchModule } = useModules()
  const [packs, setPacks] = useState<ModulePack[]>([])
  const [busy, setBusy] = useState<string | null>(null)

  useEffect(() => {
    void (async () => {
      try {
        setPacks(await fetchPacks())
      } catch (e) {
        snackbar.error((e as Error).message)
      }
    })()
  }, [snackbar])

  // Liga (aditivo) ou desliga o segmento inteiro. Desligar não apaga dados.
  async function onTogglePack(slug: string, enabled: boolean) {
    setBusy(`pack:${slug}`)
    try {
      const data = await togglePack(slug, enabled)
      applyResponse(data)
      snackbar.success(enabled ? t('modules.applied') : t('modules.removed'))
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setBusy(null)
    }
  }

  // Otimista: reflete na hora e reverte se o servidor recusar.
  async function onToggle(m: TenantModule, next: boolean) {
    patchModule(m.slug, next)
    try {
      await toggleModule(m.slug, next)
    } catch (e) {
      patchModule(m.slug, !next)
      snackbar.error((e as Error).message)
    }
  }

  const activeSet = new Set(activePacks)

  // Só módulos com tela própria (oculta infra de plataforma sem rota),
  // agrupados por categoria
  const visible = modules.filter((m) => m.routePath)
  const byCategory = visible.reduce<Record<string, TenantModule[]>>((acc, m) => {
    ;(acc[m.category] ??= []).push(m)
    return acc
  }, {})

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mt: 0 }}>{t('modules.title')}</Typography>
      <Typography color="text.secondary" sx={{ mt: 0.5, mb: 2 }}>{t('modules.subtitle')}</Typography>

      {/* ---------- Segmentos (packs) ---------- */}
      <Paper variant="outlined" sx={{ p: 2.5, mb: 2 }}>
        <Typography variant="subtitle1" sx={{ fontWeight: 600 }}>{t('modules.segments_title')}</Typography>
        <Typography variant="body2" color="text.secondary" sx={{ mt: 0.25 }}>
          {t('modules.segments_subtitle')}
        </Typography>

        <Box
          sx={{
            display: 'grid',
            gap: 1.5,
            gridTemplateColumns: 'repeat(auto-fit, minmax(240px, 1fr))',
            mt: 2,
          }}
        >
          {packs.map((p) => {
            // Segmento "ativo" = foi explicitamente ativado pelo tenant.
            const active = activeSet.has(p.slug)
            return (
              <Paper
                key={p.slug}
                variant="outlined"
                sx={{
                  p: 2,
                  display: 'flex',
                  flexDirection: 'column',
                  gap: 1,
                  ...(active ? { borderColor: 'primary.main' } : {}),
                }}
              >
                <Stack direction="row" spacing={1} sx={{ alignItems: 'center' }}>
                  <Typography sx={{ flex: 1, fontWeight: 600 }}>{p.name}</Typography>
                  {active && <Chip size="small" color="primary" label={t('modules.active_badge')} />}
                </Stack>
                <Typography variant="body2" color="text.secondary" sx={{ flex: 1 }}>{p.description}</Typography>
                <Typography variant="caption" color="text.secondary">
                  {t('modules.count', { count: p.items.length })}
                </Typography>
                <Button
                  variant={active ? 'outlined' : 'contained'}
                  color={active ? 'error' : 'primary'}
                  onClick={() => void onTogglePack(p.slug, !active)}
                  disabled={busy === `pack:${p.slug}`}
                >
                  {busy === `pack:${p.slug}` ? '…' : active ? t('modules.deactivate') : t('modules.activate')}
                </Button>
              </Paper>
            )
          })}
        </Box>
      </Paper>

      {/* ---------- Módulos individuais ---------- */}
      <Paper variant="outlined" sx={{ p: 2.5, mb: 2 }}>
        <Typography variant="subtitle1" sx={{ fontWeight: 600 }}>{t('modules.modules_title')}</Typography>
        <Typography variant="body2" color="text.secondary" sx={{ mt: 0.25 }}>
          {t('modules.modules_subtitle')}
        </Typography>

        {loading && <Typography sx={{ mt: 2 }}>{t('common.loading')}</Typography>}

        {!loading && (
          <Stack spacing={1.5} sx={{ mt: 2 }}>
            {CATEGORY_ORDER.filter((cat) => byCategory[cat]?.length).map((cat) => (
              <Box key={cat}>
                <Typography variant="overline" color="text.secondary">{t(`modules.cat.${cat}`)}</Typography>
                <Box
                  sx={{
                    display: 'grid',
                    gap: 0.5,
                    gridTemplateColumns: 'repeat(auto-fill, minmax(220px, 1fr))',
                  }}
                >
                  {byCategory[cat]
                    .sort((a, b) => a.sortOrder - b.sortOrder)
                    .map((m) => (
                      <FormControlLabel
                        key={m.slug}
                        title={m.isCore ? t('settings.modules_core_hint') : undefined}
                        sx={{
                          m: 0,
                          px: 1,
                          py: 0.5,
                          border: 1,
                          borderColor: 'divider',
                          borderRadius: 1,
                          bgcolor: 'action.hover',
                          opacity: m.isCore ? 0.7 : 1,
                        }}
                        control={
                          <Switch
                            size="small"
                            checked={m.enabled}
                            disabled={m.isCore || busy === `mod:${m.slug}`}
                            onChange={(e) => onToggle(m, e.target.checked)}
                          />
                        }
                        label={
                          <Stack direction="row" spacing={1} sx={{ alignItems: 'center' }}>
                            <Typography variant="body2" sx={{ flex: 1 }}>{m.name}</Typography>
                            {m.isCore && (
                              <Typography variant="caption" color="text.secondary">{t('settings.modules_core_badge')}</Typography>
                            )}
                          </Stack>
                        }
                      />
                    ))}
                </Box>
              </Box>
            ))}
          </Stack>
        )}
      </Paper>
    </Layout>
  )
}
