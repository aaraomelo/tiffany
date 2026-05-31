import { useEffect, useState } from 'react'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'

type PartyRole = 'CUSTOMER' | 'SUPPLIER' | 'BOTH' | 'EMPLOYEE'

interface CustomerSupplier {
  id: string
  name: string
  tradeName: string | null
  document: string | null
  email: string | null
  phone: string | null
  role: PartyRole
  personType: 'INDIVIDUAL' | 'COMPANY'
  active: boolean
  createdAt: string
}

interface ListResponse {
  items: CustomerSupplier[]
  total: number
  page: number
  pageSize: number
}

export function CustomersPage() {
  const t = useT()
  const [data, setData] = useState<ListResponse | null>(null)
  const [q, setQ] = useState('')
  const [role, setRole] = useState<PartyRole | ''>('')
  const [error, setError] = useState<string | null>(null)
  const [creating, setCreating] = useState(false)
  const [newName, setNewName] = useState('')
  const [newDoc, setNewDoc] = useState('')
  const [newRole, setNewRole] = useState<PartyRole>('CUSTOMER')

  async function load() {
    setError(null)
    const params = new URLSearchParams()
    if (q) params.set('q', q)
    if (role) params.set('role', role)
    try {
      const res = await api<ListResponse>(
        `/api/customer-suppliers?${params.toString()}`,
      )
      setData(res)
    } catch (e) {
      setError((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!newName.trim()) return
    setCreating(true)
    try {
      await api('/api/customer-suppliers', {
        method: 'POST',
        body: JSON.stringify({
          name: newName,
          document: newDoc || undefined,
          role: newRole,
        }),
      })
      setNewName(''); setNewDoc('')
      void load()
    } catch (e) {
      setError((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('customers.title')}</h1>

      <section style={{ background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1.5rem' }}>
        <h2 style={{ marginTop: 0, fontSize: 18 }}>{t('customers.new')}</h2>
        <form onSubmit={handleCreate} style={{ display: 'grid', gridTemplateColumns: '2fr 1fr 1fr auto', gap: '0.6rem', alignItems: 'end' }}>
          <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
            {t('common.name')}
            <input value={newName} onChange={(e) => setNewName(e.target.value)} required style={input} />
          </label>
          <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
            {t('common.document')}
            <input value={newDoc} onChange={(e) => setNewDoc(e.target.value)} style={input} />
          </label>
          <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
            {t('customers.type')}
            <select value={newRole} onChange={(e) => setNewRole(e.target.value as PartyRole)} style={input}>
              <option value="CUSTOMER">{t('customers.role.CUSTOMER')}</option>
              <option value="SUPPLIER">{t('customers.role.SUPPLIER')}</option>
              <option value="BOTH">{t('customers.role.BOTH')}</option>
              <option value="EMPLOYEE">{t('customers.role.EMPLOYEE')}</option>
            </select>
          </label>
          <button type="submit" disabled={creating} style={btn}>
            {creating ? '…' : t('common.create')}
          </button>
        </form>
      </section>

      <section style={{ display: 'flex', gap: '0.6rem', marginBottom: '1rem' }}>
        <input
          placeholder={t('customers.search_placeholder')}
          value={q}
          onChange={(e) => setQ(e.target.value)}
          style={{ ...input, flex: 1 }}
        />
        <select value={role} onChange={(e) => setRole(e.target.value as PartyRole | '')} style={input}>
          <option value="">{t('common.all')}</option>
          <option value="CUSTOMER">{t('customers.role.CUSTOMER')}</option>
          <option value="SUPPLIER">{t('customers.role.SUPPLIER')}</option>
          <option value="BOTH">{t('customers.role.BOTH')}</option>
          <option value="EMPLOYEE">{t('customers.role.EMPLOYEE')}</option>
        </select>
        <button onClick={() => void load()} style={btn}>{t('common.filter')}</button>
      </section>

      {error && <p style={{ color: 'var(--danger)' }}>{error}</p>}
      {!data && !error && <p>{t('common.loading')}</p>}

      {data && (
        <table style={{ width: '100%', borderCollapse: 'collapse', fontSize: 14 }}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>{t('customers.col_name')}</th>
              <th style={td}>{t('customers.col_doc')}</th>
              <th style={td}>{t('customers.col_type')}</th>
              <th style={td}>{t('customers.col_email')}</th>
              <th style={td}>{t('customers.col_phone')}</th>
            </tr>
          </thead>
          <tbody>
            {data.items.length === 0 && (
              <tr><td colSpan={5} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('customers.empty')}</td></tr>
            )}
            {data.items.map((cs) => (
              <tr key={cs.id} style={{ borderBottom: '1px solid var(--border)' }}>
                <td style={td}>{cs.name}</td>
                <td style={td}>{cs.document ?? '—'}</td>
                <td style={td}>{t(`customers.role.${cs.role}`)}</td>
                <td style={td}>{cs.email ?? '—'}</td>
                <td style={td}>{cs.phone ?? '—'}</td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
      {data && (
        <p style={{ marginTop: '1rem', color: 'var(--text-muted)', fontSize: 13 }}>
          {t('customers.total_page', { total: data.total, page: data.page })}
        </p>
      )}
    </Layout>
  )
}

const input: React.CSSProperties = { padding: '0.55rem 0.7rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
