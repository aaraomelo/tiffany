import { useEffect, useState } from 'react'
import { api } from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

type StudentStatus = 'ACTIVE' | 'INACTIVE' | 'SUSPENDED'

interface Student {
  id: string
  name: string
  document: string | null
  phone: string | null
  guardianName: string | null
  position: string | null
  status: StudentStatus
  createdAt: string
}

interface ListResponse {
  items: Student[]
  total: number
  page: number
  pageSize: number
}

export function StudentsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<ListResponse | null>(null)
  const [q, setQ] = useState('')
  const [status, setStatus] = useState<StudentStatus | ''>('')
  const [creating, setCreating] = useState(false)
  const [newName, setNewName] = useState('')
  const [newPhone, setNewPhone] = useState('')
  const [newGuardian, setNewGuardian] = useState('')

  async function load() {
    const params = new URLSearchParams()
    if (q) params.set('q', q)
    if (status) params.set('status', status)
    try {
      const res = await api<ListResponse>(`/api/students?${params.toString()}`)
      setData(res)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!newName.trim()) return
    setCreating(true)
    try {
      await api('/api/students', {
        method: 'POST',
        body: JSON.stringify({
          name: newName,
          phone: newPhone || undefined,
          guardianName: newGuardian || undefined,
        }),
      })
      setNewName(''); setNewPhone(''); setNewGuardian('')
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('students.title')}</h1>

      <Can I="create" a="Student">
        <section style={{ background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1.5rem' }}>
          <h2 style={{ marginTop: 0, fontSize: 18 }}>{t('students.new')}</h2>
          <form onSubmit={handleCreate} style={{ display: 'grid', gridTemplateColumns: '2fr 1fr 1.5fr auto', gap: '0.6rem', alignItems: 'end' }}>
            <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
              {t('common.name')}
              <input value={newName} onChange={(e) => setNewName(e.target.value)} required style={input} />
            </label>
            <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
              {t('common.phone')}
              <input value={newPhone} onChange={(e) => setNewPhone(e.target.value)} style={input} />
            </label>
            <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
              {t('students.guardian')}
              <input value={newGuardian} onChange={(e) => setNewGuardian(e.target.value)} style={input} />
            </label>
            <button type="submit" disabled={creating} style={btn}>
              {creating ? '…' : t('common.create')}
            </button>
          </form>
        </section>
      </Can>

      <section style={{ display: 'flex', gap: '0.6rem', marginBottom: '1rem' }}>
        <input
          placeholder={t('students.search_placeholder')}
          value={q}
          onChange={(e) => setQ(e.target.value)}
          style={{ ...input, flex: 1 }}
        />
        <select value={status} onChange={(e) => setStatus(e.target.value as StudentStatus | '')} style={input}>
          <option value="">{t('common.all')}</option>
          <option value="ACTIVE">{t('students.status.ACTIVE')}</option>
          <option value="INACTIVE">{t('students.status.INACTIVE')}</option>
          <option value="SUSPENDED">{t('students.status.SUSPENDED')}</option>
        </select>
        <button onClick={() => void load()} style={btn}>{t('common.filter')}</button>
      </section>

      {!data && <p>{t('common.loading')}</p>}

      {data && (
        <table style={{ width: '100%', borderCollapse: 'collapse', fontSize: 14 }}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>{t('common.name')}</th>
              <th style={td}>{t('common.phone')}</th>
              <th style={td}>{t('students.guardian')}</th>
              <th style={td}>{t('students.position')}</th>
              <th style={td}>{t('common.status')}</th>
            </tr>
          </thead>
          <tbody>
            {data.items.length === 0 && (
              <tr><td colSpan={5} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('students.empty')}</td></tr>
            )}
            {data.items.map((s) => (
              <tr key={s.id} style={{ borderBottom: '1px solid var(--border)' }}>
                <td style={td}>{s.name}</td>
                <td style={td}>{s.phone ?? '—'}</td>
                <td style={td}>{s.guardianName ?? '—'}</td>
                <td style={td}>{s.position ?? '—'}</td>
                <td style={td}>{t(`students.status.${s.status}`)}</td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
      {data && (
        <p style={{ marginTop: '1rem', color: 'var(--text-muted)', fontSize: 13 }}>
          {t('students.total_page', { total: data.total, page: data.page })}
        </p>
      )}
    </Layout>
  )
}

const input: React.CSSProperties = { padding: '0.55rem 0.7rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
