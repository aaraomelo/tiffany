import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { ApiError, api, setSession, type StoredUser } from '../api'
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
  const [email, setEmail] = useState('admin@acme.com')
  const [password, setPassword] = useState('changeme123')
  const [tenantAlias, setTenantAlias] = useState('')
  const [loading, setLoading] = useState(false)

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
    <div style={{ minHeight: '100vh', background: 'var(--bg)', color: 'var(--text)', fontFamily: 'system-ui' }}>
      <header style={{ background: 'var(--primary)', padding: '0.6rem 1.5rem', display: 'flex', justifyContent: 'flex-end', gap: '0.6rem' }}>
        <ThemeSwitcher />
        <LangSwitcher />
      </header>
      <main style={{ maxWidth: 380, margin: '5rem auto', padding: '0 1rem' }}>
        <h1 style={{ color: 'var(--primary)' }}>{t('app.name')}</h1>
        <p style={{ color: 'var(--text-muted)', marginBottom: '2rem' }}>{t('login.title')}</p>
        <form onSubmit={handleSubmit} style={{ display: 'grid', gap: '1rem' }}>
          <label style={{ display: 'grid', gap: '0.3rem' }}>
            <span>{t('login.email')}</span>
            <input
              type="email"
              required
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              style={inputStyle}
            />
          </label>
          <label style={{ display: 'grid', gap: '0.3rem' }}>
            <span>{t('login.password')}</span>
            <input
              type="password"
              required
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              style={inputStyle}
            />
          </label>
          <label style={{ display: 'grid', gap: '0.3rem' }}>
            <span style={{ color: 'var(--text-muted)', fontSize: 13 }}>
              {t('login.tenant_alias')} {t('common.optional')}
            </span>
            <input
              value={tenantAlias}
              onChange={(e) => setTenantAlias(e.target.value)}
              placeholder="acme"
              style={inputStyle}
            />
          </label>
          <button type="submit" disabled={loading} style={btnStyle}>
            {loading ? t('login.submitting') : t('login.submit')}
          </button>
        </form>
      </main>
    </div>
  )
}

const inputStyle: React.CSSProperties = {
  padding: '0.6rem 0.7rem',
  fontSize: 15,
  borderRadius: 6,
}

const btnStyle: React.CSSProperties = {
  padding: '0.7rem 1rem',
  fontSize: 15,
  background: 'var(--primary)',
  color: 'var(--text-on-primary)',
  border: 'none',
  borderRadius: 6,
  cursor: 'pointer',
  marginTop: '0.5rem',
}
