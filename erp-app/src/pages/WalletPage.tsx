import { useEffect, useState } from 'react'
import { api } from '../api'
import { Layout } from '../components/Layout'
import { useT } from '../i18n/LangContext'

interface Wallet {
  id: string
  currency: string
  balance: string
  blocked: string
  totalReceived: string
  totalPaidOut: string
}
interface Tx {
  id: string
  type: string
  amount: string
  balanceAfter: string
  createdAt: string
  refType: string | null
}

const TYPE_STYLE: Record<string, React.CSSProperties> = {
  SALE_CREDIT: { color: 'var(--success)' },
  FEE_DEBIT: { color: 'var(--warn)' },
  PAYOUT_DEBIT: { color: 'var(--text-muted)' },
  REFUND_DEBIT: { color: 'var(--danger)' },
  CHARGEBACK_DEBIT: { color: 'var(--danger)' },
}

export function WalletPage() {
  const t = useT()
  const [data, setData] = useState<{ wallet: Wallet; items: Tx[] } | null>(null)
  useEffect(() => {
    api<{ wallet: Wallet; items: Tx[] }>('/api/wallet/transactions').then(setData)
  }, [])

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('wallet.title')}</h1>
      {!data && <p>{t('common.loading')}</p>}
      {data && (
        <>
          <section style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '1rem', marginBottom: '1.5rem' }}>
            <Stat label={t('wallet.balance')} value={data.wallet.balance} highlight />
            <Stat label={t('wallet.blocked')} value={data.wallet.blocked} />
            <Stat label={t('wallet.total_received')} value={data.wallet.totalReceived} />
            <Stat label={t('wallet.total_paid_out')} value={data.wallet.totalPaidOut} />
          </section>
          <table style={tbl}>
            <thead>
              <tr style={{ background: 'var(--surface-alt)', textAlign: 'left' }}>
                <th style={td}>{t('wallet.col_date')}</th>
                <th style={td}>{t('wallet.tx_type')}</th>
                <th style={td}>{t('wallet.col_source')}</th>
                <th style={{ ...td, textAlign: 'right' }}>{t('wallet.col_value')}</th>
                <th style={{ ...td, textAlign: 'right' }}>{t('wallet.col_balance')}</th>
              </tr>
            </thead>
            <tbody>
              {data.items.length === 0 && <tr><td colSpan={5} style={{ padding: '1rem', color: 'var(--text-muted)' }}>{t('wallet.empty')}</td></tr>}
              {data.items.map((tx) => (
                <tr key={tx.id} style={{ borderBottom: '1px solid var(--border)' }}>
                  <td style={td}>{new Date(tx.createdAt).toLocaleString()}</td>
                  <td style={{ ...td, ...(TYPE_STYLE[tx.type] ?? {}) }}>{tx.type}</td>
                  <td style={td}>{tx.refType ?? '—'}</td>
                  <td style={{ ...td, textAlign: 'right', ...(TYPE_STYLE[tx.type] ?? {}) }}>R$ {Number(tx.amount).toFixed(2)}</td>
                  <td style={{ ...td, textAlign: 'right' }}>R$ {Number(tx.balanceAfter).toFixed(2)}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </>
      )}
    </Layout>
  )
}

function Stat({ label, value, highlight }: { label: string; value: string; highlight?: boolean }) {
  return (
    <div style={{ background: 'var(--surface)', padding: '0.8rem 1rem', borderRadius: 8, border: '1px solid var(--border)' }}>
      <div style={{ fontSize: 12, color: 'var(--text-muted)' }}>{label}</div>
      <div style={{ fontSize: highlight ? 24 : 18, fontWeight: 600, color: highlight ? 'var(--primary)' : 'var(--text)' }}>
        R$ {Number(value).toFixed(2)}
      </div>
    </div>
  )
}

const tbl: React.CSSProperties = { width: '100%', borderCollapse: 'collapse', fontSize: 14, background: 'var(--surface)', borderRadius: 6, overflow: 'hidden', border: '1px solid var(--border)' }
const td: React.CSSProperties = { padding: '0.55rem 0.7rem' }
