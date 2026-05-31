import { useEffect, useState } from 'react'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

type TuitionStatus = 'PENDING' | 'PAID' | 'OVERDUE' | 'CANCELLED'

interface Tuition {
  id: string
  referenceMonth: number
  referenceYear: number
  dueDate: string
  amount: string
  status: TuitionStatus
  paidAt: string | null
  student: { id: string; name: string }
}

interface ListResponse {
  items: Tuition[]
  total: number
  page: number
  pageSize: number
}

function money(v: string | number) {
  return Number(v).toLocaleString('pt-BR', { style: 'currency', currency: 'BRL' })
}

function ref(m: number, y: number) {
  return `${String(m).padStart(2, '0')}/${y}`
}

const now = new Date()

export function TuitionsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [data, setData] = useState<ListResponse | null>(null)
  const [status, setStatus] = useState<TuitionStatus | ''>('')
  const [generating, setGenerating] = useState(false)

  async function load() {
    const params = new URLSearchParams()
    if (status) params.set('status', status)
    try {
      setData(await api<ListResponse>(`/api/tuitions?${params.toString()}`))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [status])

  async function pay(tu: Tuition) {
    try {
      await api(`/api/tuitions/${tu.id}/pay`, { method: 'POST', body: JSON.stringify({}) })
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  async function cancel(tu: Tuition) {
    try {
      await api(`/api/tuitions/${tu.id}/cancel`, { method: 'POST', body: JSON.stringify({}) })
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  async function generateBatch() {
    setGenerating(true)
    try {
      const res = await api<{ created: number; skipped: number }>('/api/enrollments/generate-tuitions', {
        method: 'POST',
        body: JSON.stringify({
          referenceMonth: now.getMonth() + 1,
          referenceYear: now.getFullYear(),
        }),
      })
      snackbar.success(t('tuitions.batch_done', { created: res.created, skipped: res.skipped }))
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setGenerating(false)
    }
  }

  return (
    <Layout>
      <div style={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
        <h1 style={{ marginTop: 0, flex: 1 }}>{t('tuitions.title')}</h1>
        <button onClick={() => void generateBatch()} disabled={generating} style={btn}>
          {generating ? '…' : t('tuitions.generate_batch')}
        </button>
      </div>

      <section style={{ display: 'flex', gap: '0.6rem', marginBottom: '1rem' }}>
        <select value={status} onChange={(e) => setStatus(e.target.value as TuitionStatus | '')} style={input}>
          <option value="">{t('common.all')}</option>
          <option value="PENDING">{t('tuitions.status.PENDING')}</option>
          <option value="PAID">{t('tuitions.status.PAID')}</option>
          <option value="OVERDUE">{t('tuitions.status.OVERDUE')}</option>
          <option value="CANCELLED">{t('tuitions.status.CANCELLED')}</option>
        </select>
      </section>

      {!data && <p>{t('common.loading')}</p>}

      {data && (
        <table style={{ width: '100%', borderCollapse: 'collapse', fontSize: 14 }}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>{t('enrollments.student')}</th>
              <th style={td}>{t('tuitions.reference')}</th>
              <th style={td}>{t('tuitions.due_date')}</th>
              <th style={td}>{t('common.amount')}</th>
              <th style={td}>{t('common.status')}</th>
              <th style={td}>{t('common.actions')}</th>
            </tr>
          </thead>
          <tbody>
            {data.items.length === 0 && (
              <tr><td colSpan={6} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('tuitions.empty')}</td></tr>
            )}
            {data.items.map((tu) => (
              <tr key={tu.id} style={{ borderBottom: '1px solid var(--border)' }}>
                <td style={td}>{tu.student.name}</td>
                <td style={td}>{ref(tu.referenceMonth, tu.referenceYear)}</td>
                <td style={td}>{new Date(tu.dueDate).toLocaleDateString('pt-BR')}</td>
                <td style={td}>{money(tu.amount)}</td>
                <td style={td}>{t(`tuitions.status.${tu.status}`)}</td>
                <td style={{ ...td, display: 'flex', gap: '0.8rem' }}>
                  {(tu.status === 'PENDING' || tu.status === 'OVERDUE') && (
                    <>
                      <button onClick={() => void pay(tu)} style={linkBtn}>{t('tuitions.pay')}</button>
                      <button onClick={() => void cancel(tu)} style={{ ...linkBtn, color: 'var(--danger)' }}>{t('common.cancel')}</button>
                    </>
                  )}
                </td>
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
const linkBtn: React.CSSProperties = { background: 'transparent', border: 'none', color: 'var(--primary)', cursor: 'pointer', fontSize: 13, padding: 0 }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
