import {
  Box,
  Card,
  CardContent,
  Chip,
  LinearProgress,
  Paper,
  Stack,
  Tooltip,
  Typography,
} from '@mui/material'
import { useEffect, useState } from 'react'
import { fetchCosmos, type CosmosData } from '../api'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'

// cor "estrela" pela massa: roxo escuro (baixa) → dourado (alta), estilo inferno
function starColor(frac: number): string {
  const f = Math.max(0, Math.min(1, frac))
  const hue = 280 - 240 * f // 280 (roxo) → 40 (âmbar)
  const light = 30 + 35 * f
  return `hsl(${hue}, 85%, ${light}%)`
}

function Kpi({ label, value, hint, intent }: { label: string; value: string; hint?: string; intent?: 'default' | 'warning' | 'success' }) {
  const color = intent === 'warning' ? 'warning.main' : intent === 'success' ? 'success.main' : 'text.primary'
  return (
    <Card variant="outlined">
      <CardContent sx={{ py: 2 }}>
        <Typography variant="caption" color="text.secondary" sx={{ display: 'block' }}>{label}</Typography>
        <Typography variant="h5" sx={{ fontWeight: 700, color }}>{value}</Typography>
        {hint && <Typography variant="caption" color="text.secondary">{hint}</Typography>}
      </CardContent>
    </Card>
  )
}

function Lorenz({ points, gini }: { points: Array<{ x: number; y: number }>; gini: number }) {
  const path = points.map((p) => `${p.x * 100},${100 - p.y * 100}`).join(' ')
  return (
    <svg viewBox="0 0 100 100" style={{ width: '100%', maxWidth: 320, aspectRatio: '1', border: '1px solid #8884', borderRadius: 4 }}>
      {/* área de concentração */}
      <polygon points={`0,100 ${path} 100,0`} fill="#1f4e7922" />
      <line x1="0" y1="100" x2="100" y2="0" stroke="#999" strokeDasharray="3" strokeWidth="0.6" />
      <polyline points={path} fill="none" stroke="#1f4e79" strokeWidth="1.6" />
      <text x="50" y="96" fontSize="6" textAnchor="middle" fill="#888">Gini = {gini.toFixed(2)}</text>
    </svg>
  )
}

export function GovernancePage() {
  const t = useT()
  const [data, setData] = useState<CosmosData | null>(null)
  const [err, setErr] = useState(false)

  useEffect(() => {
    fetchCosmos().then(setData).catch(() => setErr(true))
  }, [])

  if (err) {
    return (
      <Layout>
        <Typography color="error">{t('governance.error')}</Typography>
      </Layout>
    )
  }
  if (!data) {
    return (
      <Layout>
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      </Layout>
    )
  }

  const m = data.metrics
  const topHub = data.hubs[0]
  const giniIntent = m.gini > 0.6 ? 'warning' : m.gini < 0.3 ? 'success' : 'default'

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700 }}>{t('governance.title')}</Typography>
      <Typography color="text.secondary" sx={{ mb: 3 }}>{t('governance.subtitle')}</Typography>

      {/* sinais vitais geométricos */}
      <Box sx={{ display: 'grid', gap: 1.5, mb: 3, gridTemplateColumns: { xs: 'repeat(2, 1fr)', sm: 'repeat(3, 1fr)', md: 'repeat(5, 1fr)' } }}>
        <Kpi label={t('governance.gini')} value={m.gini.toFixed(2)} hint={t('governance.gini_hint')} intent={giniIntent} />
        <Kpi label={t('governance.orbits')} value={String(m.orbits)} hint={m.orbits === 0 ? t('governance.orbits_ok') : t('governance.orbits_warn')} intent={m.orbits > 0 ? 'warning' : 'success'} />
        <Kpi label={t('governance.horizon')} value={String(m.horizonRoles)} hint={t('governance.horizon_hint')} />
        <Kpi label={t('governance.roles')} value={String(m.totalRoles)} hint={t('governance.users_n', { n: m.totalUsers })} />
        <Kpi label={t('governance.top_hub')} value={topHub ? topHub.name : '—'} hint={topHub ? t('governance.hub_score', { n: topHub.userCount }) : ''} intent="warning" />
      </Box>

      <Box sx={{ display: 'grid', gap: 2, gridTemplateColumns: { xs: '1fr', md: '1fr 2fr' } }}>
        {/* concentração */}
        <Paper variant="outlined" sx={{ p: 2 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 700, mb: 1 }}>{t('governance.concentration')}</Typography>
          <Lorenz points={data.lorenz} gini={m.gini} />
          <Typography variant="caption" color="text.secondary" sx={{ display: 'block', mt: 1 }}>{t('governance.concentration_hint')}</Typography>
        </Paper>

        {/* perfis como estrelas (massa autorizativa) */}
        <Paper variant="outlined" sx={{ p: 2 }}>
          <Typography variant="subtitle1" sx={{ fontWeight: 700, mb: 1.5 }}>{t('governance.stars')}</Typography>
          <Stack spacing={1.2}>
            {data.roles.map((r) => {
              const frac = m.maxRoleMass ? r.mass / m.maxRoleMass : 0
              return (
                <Box key={r.id} sx={{ display: 'grid', gridTemplateColumns: '1fr 2fr auto', gap: 1.5, alignItems: 'center' }}>
                  <Stack direction="row" sx={{ alignItems: 'center', gap: 0.5, minWidth: 0 }}>
                    <Box sx={{ width: 10, height: 10, borderRadius: '50%', bgcolor: starColor(frac), flexShrink: 0, boxShadow: `0 0 ${4 + 8 * frac}px ${starColor(frac)}` }} />
                    <Typography variant="body2" sx={{ fontWeight: 600, overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>{r.name}</Typography>
                  </Stack>
                  <Tooltip title={t('governance.mass_tip', { mass: r.mass, max: m.maxRoleMass })}>
                    <LinearProgress variant="determinate" value={frac * 100} sx={{ height: 8, borderRadius: 4, '& .MuiLinearProgress-bar': { bgcolor: starColor(frac) } }} />
                  </Tooltip>
                  <Stack direction="row" sx={{ gap: 0.5, justifyContent: 'flex-end' }}>
                    <Chip size="small" label={t('governance.n_users', { n: r.userCount })} variant="outlined" />
                    {r.horizon && <Chip size="small" color="error" label={t('governance.horizon_badge')} />}
                  </Stack>
                </Box>
              )
            })}
          </Stack>
        </Paper>
      </Box>
    </Layout>
  )
}
