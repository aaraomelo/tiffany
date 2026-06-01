import {
  AppBar,
  Box,
  Button,
  IconButton,
  Toolbar,
  Tooltip,
  Typography,
} from '@mui/material'
import DarkModeOutlinedIcon from '@mui/icons-material/DarkModeOutlined'
import HomeOutlinedIcon from '@mui/icons-material/HomeOutlined'
import ForumOutlinedIcon from '@mui/icons-material/ForumOutlined'
import GridViewOutlinedIcon from '@mui/icons-material/GridViewOutlined'
import LightModeOutlinedIcon from '@mui/icons-material/LightModeOutlined'
import LogoutOutlinedIcon from '@mui/icons-material/LogoutOutlined'
import LanguageOutlinedIcon from '@mui/icons-material/LanguageOutlined'
import PaletteOutlinedIcon from '@mui/icons-material/PaletteOutlined'
import SettingsOutlinedIcon from '@mui/icons-material/SettingsOutlined'
import ShieldOutlinedIcon from '@mui/icons-material/ShieldOutlined'
import { useEffect } from 'react'
import { Link, useLocation, useNavigate } from 'react-router-dom'
import { SUBJECT_BY_MODULE, useAbility } from '../access/AbilityContext'
import { clearSession, getUser } from '../api'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'
import { useModules } from '../modules/ModulesContext'
import { useTheme } from '../theme/ThemeContext'

// slug do módulo → chave i18n do label no menu. Slugs sem entrada aqui
// (ex: 'theme', 'assistant') não aparecem no menu principal — vão no header.
const MENU_I18N_KEY: Record<string, string> = {
  pos: 'nav.pos',
  order: 'nav.orders',
  'service-order': 'nav.service_orders',
  budget: 'nav.budgets',
  cash: 'nav.cash',
  wallet: 'nav.wallet',
  'customer-supplier': 'nav.customers',
  product: 'nav.products',
  stock: 'nav.stock',
  student: 'nav.students',
  'enrollment-plan': 'nav.plans',
  enrollment: 'nav.enrollments',
  tuition: 'nav.tuitions',
  'health-record': 'nav.health',
}

// rota estática → chave i18n, para o título da aba do navegador
const PAGE_TITLE_KEY: Record<string, string> = {
  '/inicio': 'nav.home',
  '/site': 'site.title',
  '/modules': 'modules.title',
  '/access': 'access.title',
  '/assistant': 'nav.assistant',
  '/theme': 'nav.theme',
  '/settings': 'nav.settings',
}

export function Layout({ children }: { children: React.ReactNode }) {
  const navigate = useNavigate()
  const location = useLocation()
  const user = getUser()
  const t = useT()
  const { modules } = useModules()
  const { ability } = useAbility()
  const { mode, toggleMode } = useTheme()

  function logout() {
    clearSession()
    navigate('/login')
  }

  const links = modules
    .filter((m) => m.enabled && m.routePath && MENU_I18N_KEY[m.slug])
    // esconde itens cujo recurso o usuário não pode ao menos ler
    .filter((m) => {
      const subj = SUBJECT_BY_MODULE[m.slug]
      return !subj || ability.can('read', subj)
    })
    .sort((a, b) => a.sortOrder - b.sortOrder)
    .map((m) => ({ to: m.routePath!, label: t(MENU_I18N_KEY[m.slug]) }))

  // título da aba: "Página · Empresa"
  useEffect(() => {
    const company = user?.tenantName ?? t('app.name')
    const key = PAGE_TITLE_KEY[location.pathname]
    const fromModule = links.find((l) => location.pathname.startsWith(l.to))
    const page = key ? t(key) : fromModule?.label
    document.title = page ? `${page} · ${company}` : company
  }, [location.pathname, user, links, t])

  function iconBtn(to: string, label: string, icon: React.ReactNode) {
    const active = location.pathname === to
    return (
      <Tooltip title={label}>
        <IconButton
          component={Link}
          to={to}
          aria-label={label}
          size="small"
          sx={{
            color: 'primary.contrastText',
            bgcolor: active ? 'rgba(255,255,255,0.18)' : 'transparent',
            '&:hover': { bgcolor: 'rgba(255,255,255,0.12)' },
          }}
        >
          {icon}
        </IconButton>
      </Tooltip>
    )
  }

  return (
    <Box sx={{ minHeight: '100vh', bgcolor: 'background.default', color: 'text.primary' }}>
      <AppBar position="static" color="primary" elevation={1}>
        <Toolbar sx={{ gap: 1, flexWrap: 'wrap' }}>
          <Typography component={Link} to="/inicio" variant="h6" sx={{ fontWeight: 700, mr: 1.5, color: 'primary.contrastText', textDecoration: 'none' }}>
            {user?.tenantName ?? t('app.name')}
          </Typography>
          {iconBtn('/inicio', t('nav.home'), <HomeOutlinedIcon fontSize="small" />)}

          <Box component="nav" sx={{ display: 'flex', gap: 0.5, flex: 1, flexWrap: 'wrap', ml: 0.5 }}>
            {links.map((l) => {
              const active = location.pathname === l.to
              return (
                <Button
                  key={l.to}
                  component={Link}
                  to={l.to}
                  size="small"
                  sx={{
                    color: 'primary.contrastText',
                    opacity: active ? 1 : 0.75,
                    fontWeight: active ? 700 : 500,
                    borderBottom: active ? '2px solid' : '2px solid transparent',
                    borderRadius: 0,
                    '&:hover': { opacity: 1, bgcolor: 'rgba(255,255,255,0.08)' },
                  }}
                >
                  {l.label}
                </Button>
              )
            })}
          </Box>

          {iconBtn('/assistant', t('nav.assistant'), <ForumOutlinedIcon fontSize="small" />)}
          {iconBtn('/site', t('site.title'), <LanguageOutlinedIcon fontSize="small" />)}
          {iconBtn('/modules', t('modules.title'), <GridViewOutlinedIcon fontSize="small" />)}
          {ability.can('read', 'Role') &&
            iconBtn('/access', t('access.title'), <ShieldOutlinedIcon fontSize="small" />)}
          {iconBtn('/theme', t('nav.theme'), <PaletteOutlinedIcon fontSize="small" />)}
          {iconBtn('/settings', t('nav.settings'), <SettingsOutlinedIcon fontSize="small" />)}

          <Tooltip title={mode === 'dark' ? t('nav.light_mode') : t('nav.dark_mode')}>
            <IconButton
              onClick={toggleMode}
              aria-label={mode === 'dark' ? t('nav.light_mode') : t('nav.dark_mode')}
              size="small"
              sx={{ color: 'primary.contrastText', '&:hover': { bgcolor: 'rgba(255,255,255,0.12)' } }}
            >
              {mode === 'dark' ? <LightModeOutlinedIcon fontSize="small" /> : <DarkModeOutlinedIcon fontSize="small" />}
            </IconButton>
          </Tooltip>

          <LangSwitcher />

          <Typography variant="body2" sx={{ opacity: 0.85, mx: 1, display: { xs: 'none', sm: 'block' } }}>
            {user?.name}
          </Typography>

          <Button
            onClick={logout}
            size="small"
            startIcon={<LogoutOutlinedIcon fontSize="small" />}
            variant="outlined"
            sx={{
              color: 'primary.contrastText',
              borderColor: 'rgba(255,255,255,0.4)',
              '&:hover': { borderColor: 'primary.contrastText', bgcolor: 'rgba(255,255,255,0.08)' },
            }}
          >
            {t('nav.logout')}
          </Button>
        </Toolbar>
      </AppBar>

      <Box component="main" sx={{ width: '100%', px: { xs: 2, sm: 3 }, py: 2.5, boxSizing: 'border-box' }}>
        {children}
      </Box>
    </Box>
  )
}
