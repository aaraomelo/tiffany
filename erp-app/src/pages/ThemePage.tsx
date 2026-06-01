import { Box, Button, Paper, Slider, Stack, Typography } from '@mui/material'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'
import { useTheme } from '../theme/ThemeContext'
import type { ThemeConfig } from '../theme/palettes'

type ColorKey =
  | 'primary' | 'primaryHover' | 'primaryLight'
  | 'secondary' | 'accent'
  | 'bg' | 'surface' | 'surfaceAlt' | 'border'
  | 'text' | 'textMuted'
  | 'success' | 'warn' | 'danger'
  | 'textOnPrimary'

const COLOR_GROUPS: { label: string; keys: ColorKey[] }[] = [
  { label: 'theme.group.brand', keys: ['primary', 'primaryHover', 'primaryLight', 'secondary', 'accent', 'textOnPrimary'] },
  { label: 'theme.group.surfaces', keys: ['bg', 'surface', 'surfaceAlt', 'border'] },
  { label: 'theme.group.text', keys: ['text', 'textMuted'] },
  { label: 'theme.group.feedback', keys: ['success', 'warn', 'danger'] },
]

export function ThemePage() {
  const { config, applyPreset, updateConfig, saveToServer, resetServer, presets, saving, dirty } = useTheme()
  const t = useT()
  const snackbar = useSnackbar()

  async function handleSave() {
    try {
      await saveToServer()
      snackbar.success(t('theme.saved'))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  async function handleReset() {
    try {
      await resetServer()
      snackbar.success(t('theme.reset_ok'))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  return (
    <Layout>
      <Stack direction="row" spacing={1} sx={{ justifyContent: 'space-between', alignItems: 'flex-start', mb: 2, flexWrap: 'wrap' }}>
        <Box>
          <Typography variant="h5" sx={{ fontWeight: 700 }}>{t('theme.title')}</Typography>
          <Typography variant="body2" color="text.secondary">{t('theme.subtitle')}</Typography>
        </Box>
        <Stack direction="row" spacing={1} sx={{ alignItems: 'center' }}>
          <Button variant="outlined" onClick={handleReset} disabled={saving}>{t('theme.reset')}</Button>
          <Button variant="contained" onClick={handleSave} disabled={saving || !dirty}>
            {saving ? '…' : (dirty ? t('theme.save') : t('theme.saved_clean'))}
          </Button>
        </Stack>
      </Stack>

      <Box sx={{ display: 'grid', gridTemplateColumns: { xs: '1fr', md: '1.4fr 1fr' }, gap: 2, alignItems: 'start' }}>
        <Box
          sx={{
            // No desktop, os controles rolam sozinhos; o preview (sticky) fica parado
            maxHeight: { md: 'calc(100vh - 150px)' },
            overflowY: { md: 'auto' },
            pr: { md: 1 },
          }}
        >
          <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
            <Typography variant="h6" sx={{ mb: 1.5 }}>{t('theme.presets')}</Typography>
            <Box sx={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(160px, 1fr))', gap: 1 }}>
              {presets.map((p) => {
                const active = config.presetId === p.id
                return (
                  <Paper
                    key={p.id}
                    variant="outlined"
                    onClick={() => applyPreset(p.id)}
                    sx={{
                      p: 1.2,
                      cursor: 'pointer',
                      ...(active && { borderColor: 'primary.main', borderWidth: 2 }),
                    }}
                  >
                    <Stack direction="row" spacing={0.5} sx={{ mb: 1 }}>
                      {[p.config.primary, p.config.secondary, p.config.accent, p.config.surface].map((c, i) => (
                        <Box key={i} sx={{ width: 20, height: 20, borderRadius: 1, bgcolor: c, border: '1px solid rgba(0,0,0,0.1)' }} />
                      ))}
                    </Stack>
                    <Typography variant="body2" sx={{ fontWeight: 600 }}>{t(p.labelKey)}</Typography>
                  </Paper>
                )
              })}
            </Box>
          </Paper>

          {COLOR_GROUPS.map((g) => (
            <Paper key={g.label} variant="outlined" sx={{ p: 2, mb: 2 }}>
              <Typography variant="h6" sx={{ mb: 1.5 }}>{t(g.label)}</Typography>
              <Box sx={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(220px, 1fr))', gap: 1 }}>
                {g.keys.map((k) => (
                  <ColorField key={k} k={k} config={config} onChange={updateConfig} t={t} />
                ))}
              </Box>
            </Paper>
          ))}

          <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
            <Typography variant="h6" sx={{ mb: 1.5 }}>{t('theme.group.geometry')}</Typography>
            <RangeField
              label={t('theme.radius')} value={config.radius} min={0} max={20}
              onChange={(v) => updateConfig({ radius: v })} unit="px"
            />
            <RangeField
              label={t('theme.radius_lg')} value={config.radiusLg} min={0} max={32}
              onChange={(v) => updateConfig({ radiusLg: v })} unit="px"
            />
            <RangeField
              label={t('theme.border_width')} value={config.borderWidth} min={0} max={4}
              onChange={(v) => updateConfig({ borderWidth: v })} unit="px"
            />
            <RangeField
              label={t('theme.spacing_base')} value={config.spacingBase} min={20} max={150}
              onChange={(v) => updateConfig({ spacingBase: v })} unit="/100rem"
            />
          </Paper>
        </Box>

        <Box component="aside" sx={{ position: 'sticky', top: '1rem', alignSelf: 'flex-start' }}>
          <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
            <Typography variant="h6" sx={{ mb: 1.5 }}>{t('theme.preview')}</Typography>
            <Preview t={t} />
          </Paper>
        </Box>
      </Box>
    </Layout>
  )
}

function ColorField({ k, config, onChange, t }: { k: ColorKey; config: ThemeConfig; onChange: (p: Partial<ThemeConfig>) => void; t: (k: string) => string }) {
  const value = config[k]
  return (
    <Stack direction="row" spacing={1} sx={{ alignItems: 'center' }}>
      <Box
        component="input"
        type="color"
        value={value}
        onChange={(e) => onChange({ [k]: e.target.value } as Partial<ThemeConfig>)}
        sx={{ width: 40, height: 32, border: 0, p: 0, bgcolor: 'transparent', cursor: 'pointer' }}
      />
      <Box
        component="input"
        type="text"
        value={value}
        onChange={(e) => onChange({ [k]: e.target.value } as Partial<ThemeConfig>)}
        sx={{ width: 80, fontFamily: 'monospace', fontSize: 12, p: '0.3rem 0.4rem', borderRadius: 1, border: '1px solid', borderColor: 'divider' }}
      />
      <Typography variant="body2" sx={{ flex: 1 }}>{t(`theme.color.${k}`)}</Typography>
    </Stack>
  )
}

function RangeField({ label, value, min, max, onChange, unit }: { label: string; value: number; min: number; max: number; onChange: (v: number) => void; unit: string }) {
  return (
    <Box sx={{ display: 'grid', gridTemplateColumns: '160px 1fr 80px', gap: 1, alignItems: 'center', mb: 1 }}>
      <Typography variant="body2">{label}</Typography>
      <Slider
        size="small"
        min={min}
        max={max}
        value={value}
        onChange={(_, v) => onChange(typeof v === 'number' ? v : v[0])}
      />
      <Typography variant="caption" sx={{ fontFamily: 'monospace' }} color="text.secondary">{value}{unit}</Typography>
    </Box>
  )
}

function Preview({ t }: { t: (k: string) => string }) {
  return (
    <div>
      <div style={{ background: 'var(--primary)', color: 'var(--text-on-primary)', padding: 'var(--spacing-base, 0.5rem) calc(var(--spacing-base, 0.5rem) * 2)', borderRadius: 'var(--radius-lg, 8px)', marginBottom: '0.8rem', fontWeight: 600 }}>
        {t('theme.preview_header')}
      </div>
      <p style={{ color: 'var(--text)', fontSize: 14, margin: '0.5rem 0' }}>{t('theme.preview_text')}</p>
      <p style={{ color: 'var(--text-muted)', fontSize: 13, margin: '0.3rem 0' }}>{t('theme.preview_muted')}</p>
      <div style={{ display: 'flex', gap: '0.5rem', flexWrap: 'wrap', margin: '0.8rem 0' }}>
        <button style={previewBtnPrimary}>{t('theme.preview_btn_primary')}</button>
        <button style={previewBtnSecondary}>{t('theme.preview_btn_secondary')}</button>
      </div>
      <div style={{ background: 'var(--surface)', border: `var(--border-width, 1px) solid var(--border)`, padding: '0.8rem', borderRadius: 'var(--radius-lg, 8px)', marginBottom: '0.5rem' }}>
        <strong>{t('theme.preview_card_title')}</strong>
        <div style={{ fontSize: 13, color: 'var(--text-muted)', marginTop: 4 }}>{t('theme.preview_card_body')}</div>
      </div>
      <div style={{ display: 'flex', gap: '0.4rem' }}>
        <Pill bg="var(--success)">{t('theme.feedback.success')}</Pill>
        <Pill bg="var(--warn)">{t('theme.feedback.warn')}</Pill>
        <Pill bg="var(--danger)">{t('theme.feedback.danger')}</Pill>
        <Pill bg="var(--accent)">{t('theme.feedback.accent')}</Pill>
      </div>
    </div>
  )
}

function Pill({ bg, children }: { bg: string; children: React.ReactNode }) {
  return (
    <span style={{ background: bg, color: '#fff', padding: '0.2rem 0.55rem', borderRadius: 'var(--radius, 6px)', fontSize: 12, fontWeight: 600 }}>{children}</span>
  )
}

const previewBtnPrimary: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 'var(--radius, 6px)', cursor: 'pointer', fontWeight: 600 }
const previewBtnSecondary: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--surface-alt)', color: 'var(--text)', border: `var(--border-width, 1px) solid var(--border)`, borderRadius: 'var(--radius, 6px)', cursor: 'pointer' }
