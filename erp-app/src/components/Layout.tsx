import { Link, useLocation, useNavigate } from 'react-router-dom'
import { SUBJECT_BY_MODULE, useAbility } from '../access/AbilityContext'
import { clearSession, getUser } from '../api'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'
import { useModules } from '../modules/ModulesContext'

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
}

export function Layout({ children }: { children: React.ReactNode }) {
  const navigate = useNavigate()
  const location = useLocation()
  const user = getUser()
  const t = useT()
  const { modules } = useModules()
  const { ability } = useAbility()

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

  const isThemePage = location.pathname === '/theme'
  const isSettingsPage = location.pathname === '/settings'
  const isAssistantPage = location.pathname === '/assistant'
  const isModulesPage = location.pathname === '/modules'
  const isAccessPage = location.pathname === '/access'

  return (
    <div style={{ minHeight: '100vh', fontFamily: 'system-ui', background: 'var(--bg)', color: 'var(--text)' }}>
      <header
        style={{
          background: 'var(--primary)',
          color: 'var(--text-on-primary)',
          padding: '0.6rem 1.5rem',
          display: 'flex',
          alignItems: 'center',
          gap: '1rem',
          flexWrap: 'wrap',
        }}
      >
        <strong style={{ fontSize: 18 }}>{t('app.name')}</strong>
        <nav style={{ display: 'flex', gap: '1rem', flex: 1, flexWrap: 'wrap' }}>
          {links.map((l) => (
            <Link
              key={l.to}
              to={l.to}
              style={{
                color: location.pathname === l.to ? 'var(--text-on-primary)' : 'var(--text-on-primary-muted)',
                textDecoration: 'none',
                fontWeight: location.pathname === l.to ? 600 : 400,
                fontSize: 14,
              }}
            >
              {l.label}
            </Link>
          ))}
        </nav>
        <Link to="/assistant" aria-label={t('nav.assistant')} title={t('nav.assistant')} style={headerBtn(isAssistantPage)}>
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
            <path d="M21 15a2 2 0 0 1-2 2H7l-4 4V5a2 2 0 0 1 2-2h14a2 2 0 0 1 2 2z" />
          </svg>
        </Link>
        <Link to="/modules" aria-label={t('modules.title')} title={t('modules.title')} style={headerBtn(isModulesPage)}>
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
            <rect x="3" y="3" width="7" height="7" rx="1" />
            <rect x="14" y="3" width="7" height="7" rx="1" />
            <rect x="3" y="14" width="7" height="7" rx="1" />
            <rect x="14" y="14" width="7" height="7" rx="1" />
          </svg>
        </Link>
        {ability.can('read', 'Role') && (
          <Link to="/access" aria-label={t('access.title')} title={t('access.title')} style={headerBtn(isAccessPage)}>
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
              <path d="M12 2 4 5v6c0 5 3.4 7.7 8 9 4.6-1.3 8-4 8-9V5z" />
              <path d="m9 12 2 2 4-4" />
            </svg>
          </Link>
        )}
        <Link to="/theme" aria-label={t('nav.theme')} title={t('nav.theme')} style={headerBtn(isThemePage)}>
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
            <circle cx="13.5" cy="6.5" r="0.5" fill="currentColor" />
            <circle cx="17.5" cy="10.5" r="0.5" fill="currentColor" />
            <circle cx="8.5" cy="7.5" r="0.5" fill="currentColor" />
            <circle cx="6.5" cy="12.5" r="0.5" fill="currentColor" />
            <path d="M12 2C6.5 2 2 6.5 2 12s4.5 10 10 10c.926 0 1.648-.746 1.648-1.688 0-.437-.18-.835-.437-1.125-.29-.289-.438-.652-.438-1.125a1.64 1.64 0 0 1 1.668-1.668h1.996c3.051 0 5.555-2.503 5.555-5.554C21.965 6.012 17.461 2 12 2z" />
          </svg>
        </Link>
        <Link to="/settings" aria-label={t('nav.settings')} title={t('nav.settings')} style={headerBtn(isSettingsPage)}>
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
            <circle cx="12" cy="12" r="3" />
            <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z" />
          </svg>
        </Link>
        <LangSwitcher />
        <span style={{ fontSize: 13, opacity: 0.85 }}>
          {user?.tenantName} · {user?.name}
        </span>
        <button
          onClick={logout}
          style={{
            background: 'transparent',
            color: 'var(--text-on-primary)',
            border: '1px solid var(--border-on-primary)',
            padding: '0.3rem 0.7rem',
            borderRadius: 4,
            cursor: 'pointer',
            fontSize: 13,
          }}
        >
          {t('nav.logout')}
        </button>
      </header>
      <main style={{ width: '100%', padding: '1.25rem 1.5rem', boxSizing: 'border-box' }}>
        {children}
      </main>
    </div>
  )
}

function headerBtn(active: boolean): React.CSSProperties {
  return {
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'center',
    background: active ? 'var(--text-on-primary)' : 'transparent',
    color: active ? 'var(--primary)' : 'var(--text-on-primary)',
    border: '1px solid var(--border-on-primary)',
    width: 30,
    height: 30,
    borderRadius: 6,
    textDecoration: 'none',
    lineHeight: 1,
  }
}
