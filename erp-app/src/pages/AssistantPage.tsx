import { useEffect, useRef, useState } from 'react'
import { ApiError, api } from '../api'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

interface Conversation {
  id: string
  title: string | null
  createdAt: string
  updatedAt: string
}

interface ToolCall { id?: string; name: string; input?: unknown }

interface Message {
  id: string
  role: 'user' | 'assistant' | 'tool'
  content: string
  model: string | null
  createdAt: string
  toolCalls?: ToolCall[] | null
  toolResult?: { ok?: boolean; summary?: string } | null
}

interface ChatResponse {
  conversationId: string
  reply: string
  model: string
  toolCalls: Array<{ name: string; summary: string }>
}

export function AssistantPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [conversations, setConversations] = useState<Conversation[]>([])
  const [activeId, setActiveId] = useState<string | null>(null)
  const [messages, setMessages] = useState<Message[]>([])
  const [input, setInput] = useState('')
  const [busy, setBusy] = useState(false)
  const scrollRef = useRef<HTMLDivElement | null>(null)

  async function loadConversations() {
    try {
      const list = await api<Conversation[]>('/api/assistant/conversations')
      setConversations(list)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  async function loadMessages(id: string) {
    setActiveId(id)
    try {
      const msgs = await api<Message[]>(`/api/assistant/conversations/${id}/messages`)
      setMessages(msgs)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => { void loadConversations() }, [])
  useEffect(() => {
    if (scrollRef.current) scrollRef.current.scrollTop = scrollRef.current.scrollHeight
  }, [messages])

  async function send() {
    if (!input.trim() || busy) return
    const text = input.trim()
    setInput('')
    setBusy(true)

    // Otimismo: mostra mensagem do user logo
    const tempUserMsg: Message = {
      id: `temp-${Date.now()}`,
      role: 'user',
      content: text,
      model: null,
      createdAt: new Date().toISOString(),
    }
    setMessages((m) => [...m, tempUserMsg])

    try {
      const body: Record<string, string> = { text }
      if (activeId) body.conversationId = activeId
      const res = await api<ChatResponse>('/api/assistant/chat', {
        method: 'POST',
        body: JSON.stringify(body),
      })
      if (!activeId) {
        setActiveId(res.conversationId)
        void loadConversations()
      } else {
        void loadConversations()
      }
      // Recarrega mensagens completas (com toolCalls/result do servidor)
      await loadMessages(res.conversationId)
    } catch (e) {
      if (e instanceof ApiError) {
        const body = e.body as { message?: string } | null
        snackbar.error(body?.message ?? `HTTP ${e.status}`)
      } else {
        snackbar.error((e as Error).message)
      }
      // tira o otimismo se falhou
      setMessages((m) => m.filter((x) => x.id !== tempUserMsg.id))
    } finally {
      setBusy(false)
    }
  }

  function newChat() {
    setActiveId(null)
    setMessages([])
    setInput('')
  }

  async function deleteConv(id: string) {
    if (!confirm(t('assistant.delete_confirm'))) return
    try {
      await api(`/api/assistant/conversations/${id}`, { method: 'DELETE' })
      if (activeId === id) newChat()
      void loadConversations()
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  function handleKey(e: React.KeyboardEvent<HTMLTextAreaElement>) {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault()
      void send()
    }
  }

  return (
    <Layout>
      <div style={{ display: 'grid', gridTemplateColumns: '260px 1fr', gap: '1rem', height: 'calc(100vh - 130px)' }}>
        {/* Sidebar */}
        <aside style={{ background: 'var(--surface)', border: '1px solid var(--border)', borderRadius: 'var(--radius-lg, 8px)', overflow: 'hidden', display: 'flex', flexDirection: 'column' }}>
          <div style={{ padding: '0.7rem', borderBottom: '1px solid var(--border)' }}>
            <button onClick={newChat} style={{ width: '100%', padding: '0.55rem', background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer', fontWeight: 600 }}>
              + {t('assistant.new_chat')}
            </button>
          </div>
          <div style={{ flex: 1, overflow: 'auto', padding: '0.4rem' }}>
            {conversations.length === 0 && <p style={{ color: 'var(--text-muted)', fontSize: 13, padding: '0.5rem' }}>{t('assistant.no_conversations')}</p>}
            {conversations.map((c) => (
              <div
                key={c.id}
                onClick={() => loadMessages(c.id)}
                style={{
                  padding: '0.55rem 0.7rem',
                  borderRadius: 6,
                  cursor: 'pointer',
                  background: activeId === c.id ? 'var(--primary-light)' : 'transparent',
                  color: activeId === c.id ? 'var(--primary)' : 'var(--text)',
                  fontWeight: activeId === c.id ? 600 : 400,
                  fontSize: 13,
                  marginBottom: 2,
                  display: 'flex',
                  justifyContent: 'space-between',
                  alignItems: 'center',
                }}
              >
                <span style={{ overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap', flex: 1 }}>
                  {c.title ?? t('assistant.untitled')}
                </span>
                <button onClick={(e) => { e.stopPropagation(); void deleteConv(c.id) }}
                  style={{ background: 'transparent', border: 'none', color: 'var(--text-muted)', cursor: 'pointer', padding: '0 0.3rem' }}
                  title={t('common.delete')}>×</button>
              </div>
            ))}
          </div>
        </aside>

        {/* Chat panel */}
        <main style={{ background: 'var(--surface)', border: '1px solid var(--border)', borderRadius: 'var(--radius-lg, 8px)', display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>
          <header style={{ padding: '0.7rem 1rem', borderBottom: '1px solid var(--border)', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
            <strong>{t('assistant.title')}</strong>
            <small style={{ color: 'var(--text-muted)' }}>{t('assistant.subtitle')}</small>
          </header>

          <div ref={scrollRef} style={{ flex: 1, overflow: 'auto', padding: '1rem' }}>
            {messages.length === 0 && (
              <div style={{ color: 'var(--text-muted)', textAlign: 'center', marginTop: '2rem' }}>
                <p>{t('assistant.empty_title')}</p>
                <p style={{ fontSize: 14 }}>{t('assistant.empty_examples')}</p>
              </div>
            )}
            {messages.map((m) => <MessageBubble key={m.id} m={m} t={t} />)}
            {busy && (
              <div style={{ ...bubble('assistant'), color: 'var(--text-muted)', fontStyle: 'italic' }}>
                {t('assistant.thinking')}
              </div>
            )}
          </div>

          <div style={{ padding: '0.7rem 1rem', borderTop: '1px solid var(--border)', display: 'flex', gap: '0.5rem', alignItems: 'flex-end' }}>
            <textarea
              value={input}
              onChange={(e) => setInput(e.target.value)}
              onKeyDown={handleKey}
              placeholder={t('assistant.input_placeholder')}
              style={{ flex: 1, padding: '0.6rem', borderRadius: 6, resize: 'none', fontFamily: 'inherit', fontSize: 14, minHeight: 44, maxHeight: 200 }}
              rows={1}
            />
            <button onClick={send} disabled={busy || !input.trim()} style={{ padding: '0.6rem 1.2rem', background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer', fontWeight: 600 }}>
              {t('assistant.send')}
            </button>
          </div>
        </main>
      </div>
    </Layout>
  )
}

function MessageBubble({ m }: { m: Message; t: (k: string) => string }) {
  if (m.role === 'tool') {
    return (
      <div style={{ padding: '0.3rem 0.7rem', margin: '0.3rem 0', fontSize: 12, color: 'var(--text-muted)', fontStyle: 'italic' }}>
        <strong>⚙ {m.toolCalls?.[0]?.name ?? 'tool'}</strong> · {m.toolResult?.summary ?? m.content}
      </div>
    )
  }
  return (
    <div style={bubble(m.role)}>
      <div style={{ whiteSpace: 'pre-wrap', wordBreak: 'break-word' }}>{m.content}</div>
      {m.model && (
        <div style={{ fontSize: 10, color: 'var(--text-muted)', marginTop: 4 }}>
          {m.model} · {new Date(m.createdAt).toLocaleTimeString()}
        </div>
      )}
    </div>
  )
}

function bubble(role: 'user' | 'assistant' | 'tool'): React.CSSProperties {
  if (role === 'user') {
    return {
      background: 'var(--primary)',
      color: 'var(--text-on-primary)',
      padding: '0.6rem 0.9rem',
      borderRadius: '14px 14px 4px 14px',
      maxWidth: '70%',
      marginLeft: 'auto',
      marginBottom: '0.5rem',
      fontSize: 14,
    }
  }
  return {
    background: 'var(--surface-alt)',
    color: 'var(--text)',
    padding: '0.6rem 0.9rem',
    borderRadius: '14px 14px 14px 4px',
    maxWidth: '70%',
    marginRight: 'auto',
    marginBottom: '0.5rem',
    fontSize: 14,
  }
}
