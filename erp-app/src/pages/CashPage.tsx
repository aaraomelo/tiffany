import { useEffect, useState } from 'react'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'

interface Cash { id: string; name: string }
interface Session {
  id: string
  cashId: string
  status: 'OPEN' | 'PARTIALLY_CLOSED' | 'CLOSED'
  openingAmount: string
  closingAmount: string | null
  openedAt: string
  closedAt: string | null
  openedBy?: { id: string; name: string }
}

const ACTIVE_SESSION_KEY = 'erp.activeSession'

export function setActiveSession(s: { cashId: string; sessionId: string } | null) {
  if (s) localStorage.setItem(ACTIVE_SESSION_KEY, JSON.stringify(s))
  else localStorage.removeItem(ACTIVE_SESSION_KEY)
}
export function getActiveSession(): { cashId: string; sessionId: string } | null {
  const raw = localStorage.getItem(ACTIVE_SESSION_KEY)
  return raw ? JSON.parse(raw) : null
}

export function CashPage() {
  const t = useT()
  const [cashes, setCashes] = useState<Cash[]>([])
  const [selected, setSelected] = useState<string | null>(null)
  const [session, setSession] = useState<Session | null>(null)
  const [openAmt, setOpenAmt] = useState('100.00')
  const [closeAmt, setCloseAmt] = useState('')
  const [opAmt, setOpAmt] = useState('')
  const [opNotes, setOpNotes] = useState('')
  const [newCashName, setNewCashName] = useState('')
  const [error, setError] = useState<string | null>(null)

  async function refresh() {
    setError(null)
    try {
      const list = await api<Cash[]>('/api/cash')
      setCashes(list)
      const sel = selected ?? list[0]?.id ?? null
      setSelected(sel)
      if (sel) {
        const s = await api<Session | null>(`/api/cash/${sel}/session/current`)
        setSession(s)
        if (s) setActiveSession({ cashId: sel, sessionId: s.id })
        else setActiveSession(null)
      } else {
        setSession(null)
        setActiveSession(null)
      }
    } catch (e) {
      setError((e as Error).message)
    }
  }

  useEffect(() => { void refresh() }, [])
  useEffect(() => { void refresh() }, [selected])

  async function createCash(e: React.FormEvent) {
    e.preventDefault()
    if (!newCashName.trim()) return
    try {
      await api('/api/cash', { method: 'POST', body: JSON.stringify({ name: newCashName }) })
      setNewCashName('')
      void refresh()
    } catch (e) { setError((e as Error).message) }
  }

  async function openSession() {
    if (!selected) return
    try {
      await api(`/api/cash/${selected}/session/open`, {
        method: 'POST',
        body: JSON.stringify({ openingAmount: Number(openAmt) }),
      })
      void refresh()
    } catch (e) { setError((e as Error).message) }
  }

  async function closeSession() {
    if (!selected || !session) return
    try {
      await api(`/api/cash/${selected}/session/${session.id}/close`, {
        method: 'POST',
        body: JSON.stringify({ closingAmount: Number(closeAmt) }),
      })
      setActiveSession(null)
      setCloseAmt('')
      void refresh()
    } catch (e) { setError((e as Error).message) }
  }

  async function operation(kind: 'withdrawal' | 'reinforcement') {
    if (!selected || !session) return
    try {
      await api(`/api/cash/${selected}/${kind}`, {
        method: 'POST',
        body: JSON.stringify({ amount: Number(opAmt), notes: opNotes || undefined }),
      })
      setOpAmt(''); setOpNotes('')
      void refresh()
    } catch (e) { setError((e as Error).message) }
  }

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('cash.title')}</h1>

      {error && <p style={{ color: 'var(--danger)' }}>{error}</p>}

      <section style={card}>
        <h2 style={{ marginTop: 0, fontSize: 16 }}>{t('cash.list')}</h2>
        {cashes.length === 0 && <p style={{ color: 'var(--text-muted)' }}>{t('cash.no_cash')}</p>}
        {cashes.length > 0 && (
          <select value={selected ?? ''} onChange={(e) => setSelected(e.target.value)} style={{ ...input, marginBottom: '0.6rem' }}>
            {cashes.map((c) => <option key={c.id} value={c.id}>{c.name}</option>)}
          </select>
        )}
        <form onSubmit={createCash} style={{ display: 'flex', gap: '0.5rem' }}>
          <input placeholder={t('cash.new_placeholder')} value={newCashName} onChange={(e) => setNewCashName(e.target.value)} style={{ ...input, flex: 1 }} />
          <button type="submit" style={btn}>{t('cash.add')}</button>
        </form>
      </section>

      {selected && (
        <section style={card}>
          <h2 style={{ marginTop: 0, fontSize: 16 }}>{t('cash.session')}</h2>
          {!session && (
            <div style={{ display: 'flex', gap: '0.5rem', alignItems: 'flex-end' }}>
              <label style={{ ...lbl, flex: 1 }}>
                {t('cash.opening_amount')}
                <input type="number" step="0.01" value={openAmt} onChange={(e) => setOpenAmt(e.target.value)} style={input} />
              </label>
              <button onClick={openSession} style={btn}>{t('cash.open')}</button>
            </div>
          )}
          {session && (
            <>
              <p style={{ margin: '0.2rem 0', fontSize: 14 }}>
                {t('common.status')}: <strong>{session.status}</strong> · {t('cash.operator')}: {session.openedBy?.name ?? '—'} · {t('cash.opened_at')}: {new Date(session.openedAt).toLocaleString()}
              </p>
              <p style={{ margin: '0.2rem 0', fontSize: 14 }}>
                {t('cash.opening')}: R$ {Number(session.openingAmount).toFixed(2)}
              </p>

              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(3, 1fr) auto auto', gap: '0.5rem', alignItems: 'end', marginTop: '1rem' }}>
                <label style={lbl}>
                  {t('cash.value_label')}
                  <input type="number" step="0.01" value={opAmt} onChange={(e) => setOpAmt(e.target.value)} style={input} />
                </label>
                <label style={{ ...lbl, gridColumn: 'span 2' }}>
                  {t('cash.observation')}
                  <input value={opNotes} onChange={(e) => setOpNotes(e.target.value)} style={input} />
                </label>
                <button onClick={() => operation('withdrawal')} style={{ ...btn, background: 'var(--warn)' }}>{t('cash.withdrawal')}</button>
                <button onClick={() => operation('reinforcement')} style={{ ...btn, background: 'var(--success)' }}>{t('cash.reinforcement')}</button>
              </div>

              <div style={{ marginTop: '1.5rem', display: 'flex', gap: '0.5rem', alignItems: 'end' }}>
                <label style={{ ...lbl, flex: 1 }}>
                  {t('cash.closing_amount')}
                  <input type="number" step="0.01" value={closeAmt} onChange={(e) => setCloseAmt(e.target.value)} style={input} />
                </label>
                <button onClick={closeSession} style={{ ...btn, background: 'var(--text-muted)' }}>{t('cash.close')}</button>
              </div>
            </>
          )}
        </section>
      )}
    </Layout>
  )
}

const card: React.CSSProperties = { background: 'var(--surface-alt)', padding: '1rem', borderRadius: 8, marginBottom: '1.5rem' }
const lbl: React.CSSProperties = { display: 'grid', gap: 4, fontSize: 13 }
const input: React.CSSProperties = { padding: '0.5rem 0.6rem', fontSize: 14, borderRadius: 6 }
const btn: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer' }
