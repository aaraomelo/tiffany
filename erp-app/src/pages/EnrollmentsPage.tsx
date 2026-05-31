import { useEffect, useState } from 'react'
import { api } from '../api'
import { Can } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

type EnrollmentStatus = 'ACTIVE' | 'SUSPENDED' | 'CANCELLED' | 'COMPLETED'

interface Enrollment {
  id: string
  status: EnrollmentStatus
  startDate: string
  dueDay: number
  monthlyPrice: string
  student: { id: string; name: string }
  plan: { id: string; name: string }
}

interface StudentLite { id: string; name: string }
interface PlanLite { id: string; name: string; price: string }

function money(v: string | number) {
  return Number(v).toLocaleString('pt-BR', { style: 'currency', currency: 'BRL' })
}

const now = new Date()

export function EnrollmentsPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [items, setItems] = useState<Enrollment[] | null>(null)
  const [students, setStudents] = useState<StudentLite[]>([])
  const [plans, setPlans] = useState<PlanLite[]>([])
  const [studentId, setStudentId] = useState('')
  const [planId, setPlanId] = useState('')
  const [dueDay, setDueDay] = useState('10')
  const [creating, setCreating] = useState(false)

  async function load() {
    try {
      const [list, st, pl] = await Promise.all([
        api<Enrollment[]>('/api/enrollments'),
        api<{ items: StudentLite[] }>('/api/students?status=ACTIVE&pageSize=200'),
        api<PlanLite[]>('/api/enrollment-plans?active=true'),
      ])
      setItems(list)
      setStudents(st.items)
      setPlans(pl)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void load() }, [])

  async function handleCreate(e: React.FormEvent) {
    e.preventDefault()
    if (!studentId || !planId) return
    setCreating(true)
    try {
      await api('/api/enrollments', {
        method: 'POST',
        body: JSON.stringify({ studentId, planId, dueDay: Number(dueDay) }),
      })
      setStudentId(''); setPlanId('')
      void load()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setCreating(false)
    }
  }

  async function generateTuition(en: Enrollment) {
    try {
      await api(`/api/enrollments/${en.id}/generate-tuition`, {
        method: 'POST',
        body: JSON.stringify({
          referenceMonth: now.getMonth() + 1,
          referenceYear: now.getFullYear(),
        }),
      })
      snackbar.success(t('enrollments.tuition_generated', { name: en.student.name }))
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('enrollments.title')}</h1>

      <Can I="create" a="Enrollment">
        <section style={{ background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1.5rem' }}>
          <h2 style={{ marginTop: 0, fontSize: 18 }}>{t('enrollments.new')}</h2>
          <form onSubmit={handleCreate} style={{ display: 'grid', gridTemplateColumns: '2fr 2fr 1fr auto', gap: '0.6rem', alignItems: 'end' }}>
            <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
              {t('enrollments.student')}
              <select value={studentId} onChange={(e) => setStudentId(e.target.value)} required style={input}>
                <option value="">—</option>
                {students.map((s) => <option key={s.id} value={s.id}>{s.name}</option>)}
              </select>
            </label>
            <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
              {t('enrollments.plan')}
              <select value={planId} onChange={(e) => setPlanId(e.target.value)} required style={input}>
                <option value="">—</option>
                {plans.map((p) => <option key={p.id} value={p.id}>{p.name} · {money(p.price)}</option>)}
              </select>
            </label>
            <label style={{ display: 'grid', gap: 4, fontSize: 13 }}>
              {t('enrollments.due_day')}
              <input type="number" min="1" max="28" value={dueDay} onChange={(e) => setDueDay(e.target.value)} style={input} />
            </label>
            <button type="submit" disabled={creating} style={btn}>
              {creating ? '…' : t('common.create')}
            </button>
          </form>
        </section>
      </Can>

      {!items && <p>{t('common.loading')}</p>}

      {items && (
        <table style={{ width: '100%', borderCollapse: 'collapse', fontSize: 14 }}>
          <thead>
            <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
              <th style={td}>{t('enrollments.student')}</th>
              <th style={td}>{t('enrollments.plan')}</th>
              <th style={td}>{t('enrollments.monthly')}</th>
              <th style={td}>{t('enrollments.due_day')}</th>
              <th style={td}>{t('common.status')}</th>
              <th style={td}>{t('common.actions')}</th>
            </tr>
          </thead>
          <tbody>
            {items.length === 0 && (
              <tr><td colSpan={6} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('enrollments.empty')}</td></tr>
            )}
            {items.map((en) => (
              <tr key={en.id} style={{ borderBottom: '1px solid var(--border)' }}>
                <td style={td}>{en.student.name}</td>
                <td style={td}>{en.plan.name}</td>
                <td style={td}>{money(en.monthlyPrice)}</td>
                <td style={td}>{en.dueDay}</td>
                <td style={td}>{t(`enrollments.status.${en.status}`)}</td>
                <td style={td}>
                  {en.status === 'ACTIVE' && (
                    <Can I="create" a="Tuition">
                      <button onClick={() => void generateTuition(en)} style={linkBtn}>
                        {t('enrollments.generate_tuition')}
                      </button>
                    </Can>
                  )}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </Layout>
  )
}

const input: React.CSSProperties = { padding: '0.55rem 0.7rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
const linkBtn: React.CSSProperties = { background: 'transparent', border: 'none', color: 'var(--primary)', cursor: 'pointer', fontSize: 13, padding: 0 }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
