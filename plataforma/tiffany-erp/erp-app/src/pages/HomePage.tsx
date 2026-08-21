import {
  CheckCircle,
  ChevronRight,
  ForumOutlined,
  GridViewOutlined,
  HubOutlined,
  LanguageOutlined,
  PaletteOutlined,
  RadioButtonUnchecked,
  SettingsOutlined,
  ShieldOutlined,
} from '@mui/icons-material'
import {
  Alert,
  Box,
  Button,
  Card,
  CardActionArea,
  CardContent,
  LinearProgress,
  Link as MuiLink,
  List,
  ListItemButton,
  ListItemIcon,
  ListItemText,
  Paper,
  Stack,
  Typography,
} from '@mui/material'
import { useEffect, useState } from 'react'
import { Link, useNavigate } from 'react-router-dom'
import {
  fetchDashboard,
  fetchOnboarding,
  getUser,
  type DashboardMetric,
  type OnboardingStatus,
} from '../api'
import { SUBJECT_BY_MODULE, useAbility } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'
import { useModules } from '../modules/ModulesContext'

const MENU_KEY: Record<string, string> = {
  pos: 'nav.pos', order: 'nav.orders', 'service-order': 'nav.service_orders',
  budget: 'nav.budgets', cash: 'nav.cash', wallet: 'nav.wallet',
  'customer-supplier': 'nav.customers', product: 'nav.products', stock: 'nav.stock',
  student: 'nav.students', 'enrollment-plan': 'nav.plans', enrollment: 'nav.enrollments',
  tuition: 'nav.tuitions', 'health-record': 'nav.health',
}
const DISMISS_KEY = 'erp.onboarding.dismissed'

// cor do MUI por intent da métrica
const INTENT_COLOR: Record<DashboardMetric['intent'], string> = {
  error: 'error.main',
  warning: 'warning.main',
  success: 'success.main',
  default: 'text.primary',
}

function formatMetric(m: DashboardMetric): string {
  if (m.format === 'currency') {
    return new Intl.NumberFormat('pt-BR', { style: 'currency', currency: 'BRL' }).format(m.value)
  }
  return new Intl.NumberFormat('pt-BR').format(m.value)
}

export function HomePage() {
  const t = useT()
  const navigate = useNavigate()
  const { modules } = useModules()
  const { ability } = useAbility()
  const user = getUser()
  const [status, setStatus] = useState<OnboardingStatus | null>(null)
  const [metrics, setMetrics] = useState<DashboardMetric[]>([])
  const [dismissed, setDismissed] = useState(() => localStorage.getItem(DISMISS_KEY) === '1')

  useEffect(() => {
    fetchOnboarding().then(setStatus).catch(() => {})
    fetchDashboard().then((r) => setMetrics(r.metrics)).catch(() => {})
  }, [])

  function dismiss() {
    localStorage.setItem(DISMISS_KEY, '1')
    setDismissed(true)
  }

  const shortcuts = modules
    .filter((m) => m.enabled && m.routePath && MENU_KEY[m.slug])
    .filter((m) => {
      const subj = SUBJECT_BY_MODULE[m.slug]
      return !subj || ability.can('read', subj)
    })
    .sort((a, b) => a.sortOrder - b.sortOrder)
    .map((m) => ({ to: m.routePath!, label: t(MENU_KEY[m.slug]) }))

  // opções de gestão = os mesmos cards do header
  const manage = [
    { to: '/site', label: t('site.title'), icon: <LanguageOutlined /> },
    { to: '/modules', label: t('modules.title'), icon: <GridViewOutlined /> },
    ...(ability.can('read', 'Role')
      ? [
          { to: '/access', label: t('access.title'), icon: <ShieldOutlined /> },
          { to: '/governance', label: t('governance.title'), icon: <HubOutlined /> },
        ]
      : []),
    { to: '/assistant', label: t('nav.assistant'), icon: <ForumOutlined /> },
    { to: '/theme', label: t('nav.theme'), icon: <PaletteOutlined /> },
    { to: '/settings', label: t('nav.settings'), icon: <SettingsOutlined /> },
  ]

  const showChecklist = status && !status.allDone && !dismissed

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700 }}>
        {t('home.welcome', { name: user?.name ?? '' })}
      </Typography>
      <Typography color="text.secondary" sx={{ mb: 3 }}>{t('home.subtitle')}</Typography>

      {status?.allDone && !dismissed && (
        <Alert severity="success" sx={{ mb: 3 }} onClose={dismiss}>{t('home.all_done')}</Alert>
      )}

      {showChecklist && (
        <Paper variant="outlined" sx={{ p: 2.5, mb: 3 }}>
          <Stack direction="row" sx={{ alignItems: 'center', justifyContent: 'space-between', mb: 1 }}>
            <Typography variant="subtitle1" sx={{ fontWeight: 700 }}>{t('home.checklist_title')}</Typography>
            <MuiLink component="button" type="button" onClick={dismiss} variant="body2" color="text.secondary">
              {t('home.dismiss')}
            </MuiLink>
          </Stack>
          <Stack direction="row" sx={{ alignItems: 'center', gap: 1.5, mb: 1.5 }}>
            <LinearProgress variant="determinate" value={(status.done / status.total) * 100} sx={{ flex: 1, height: 8, borderRadius: 4 }} />
            <Typography variant="body2" color="text.secondary">{status.done}/{status.total}</Typography>
          </Stack>
          <List disablePadding>
            {status.steps.map((s) => (
              <ListItemButton key={s.key} onClick={() => navigate(s.route)} disabled={s.done} sx={{ borderRadius: 1, opacity: s.done ? 0.6 : 1 }}>
                <ListItemIcon sx={{ minWidth: 40 }}>
                  {s.done ? <CheckCircle color="success" /> : <RadioButtonUnchecked color="disabled" />}
                </ListItemIcon>
                <ListItemText
                  primary={t(`home.step.${s.key}`)}
                  slotProps={{ primary: { sx: { textDecoration: s.done ? 'line-through' : 'none', fontWeight: s.done ? 400 : 600 } } }}
                />
                {!s.done && <Button size="small" endIcon={<ChevronRight />}>{t('home.do')}</Button>}
              </ListItemButton>
            ))}
          </List>
        </Paper>
      )}

      {/* visão geral — métricas dos módulos ativos */}
      {metrics.length > 0 && (
        <>
          <Typography variant="subtitle1" sx={{ fontWeight: 700, mb: 1.5 }}>{t('dash.overview')}</Typography>
          <Box sx={{ display: 'grid', gap: 1.5, mb: 3, gridTemplateColumns: { xs: 'repeat(2, 1fr)', sm: 'repeat(3, 1fr)', md: 'repeat(4, 1fr)' } }}>
            {metrics.map((m) => {
              const card = (
                <CardContent sx={{ py: 2 }}>
                  <Typography variant="caption" color="text.secondary" sx={{ display: 'block', mb: 0.5 }}>
                    {t(`dash.metric.${m.key}`)}
                  </Typography>
                  <Typography variant="h5" sx={{ fontWeight: 700, color: INTENT_COLOR[m.intent] }}>
                    {formatMetric(m)}
                  </Typography>
                </CardContent>
              )
              return (
                <Card
                  key={`${m.module}.${m.key}`}
                  variant="outlined"
                  sx={{ borderColor: m.intent === 'default' ? 'divider' : INTENT_COLOR[m.intent], borderLeftWidth: 3 }}
                >
                  {m.route ? <CardActionArea component={Link} to={m.route}>{card}</CardActionArea> : card}
                </Card>
              )
            })}
          </Box>
        </>
      )}

      {/* atalhos de módulos */}
      {shortcuts.length > 0 && (
        <>
          <Typography variant="subtitle1" sx={{ fontWeight: 700, mb: 1.5 }}>{t('home.shortcuts')}</Typography>
          <Box sx={{ display: 'grid', gap: 1.5, mb: 3, gridTemplateColumns: { xs: 'repeat(2, 1fr)', sm: 'repeat(3, 1fr)', md: 'repeat(4, 1fr)' } }}>
            {shortcuts.map((s) => (
              <Card key={s.to} variant="outlined">
                <CardActionArea component={Link} to={s.to}>
                  <CardContent sx={{ py: 2.5, textAlign: 'center' }}>
                    <Typography sx={{ fontWeight: 600 }}>{s.label}</Typography>
                  </CardContent>
                </CardActionArea>
              </Card>
            ))}
          </Box>
        </>
      )}

      {/* gestão — mesmas opções do header */}
      <Typography variant="subtitle1" sx={{ fontWeight: 700, mb: 1.5 }}>{t('dash.manage')}</Typography>
      <Box sx={{ display: 'grid', gap: 1.5, gridTemplateColumns: { xs: 'repeat(2, 1fr)', sm: 'repeat(3, 1fr)', md: 'repeat(6, 1fr)' } }}>
        {manage.map((m) => (
          <Card key={m.to} variant="outlined">
            <CardActionArea component={Link} to={m.to}>
              <CardContent sx={{ py: 2, textAlign: 'center' }}>
                <Box sx={{ color: 'primary.main', display: 'flex', justifyContent: 'center', mb: 0.5 }}>{m.icon}</Box>
                <Typography variant="body2" sx={{ fontWeight: 600 }}>{m.label}</Typography>
              </CardContent>
            </CardActionArea>
          </Card>
        ))}
      </Box>
    </Layout>
  )
}
