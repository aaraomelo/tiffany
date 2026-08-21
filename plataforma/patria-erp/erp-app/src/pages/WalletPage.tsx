import { useEffect, useState } from 'react'
import type { SxProps, Theme } from '@mui/material'
import {
  Paper,
  Stack,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Typography,
} from '@mui/material'
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

const TYPE_COLOR: Record<string, string> = {
  SALE_CREDIT: 'success.main',
  FEE_DEBIT: 'warning.main',
  PAYOUT_DEBIT: 'text.secondary',
  REFUND_DEBIT: 'error.main',
  CHARGEBACK_DEBIT: 'error.main',
}

export function WalletPage() {
  const t = useT()
  const [data, setData] = useState<{ wallet: Wallet; items: Tx[] } | null>(null)
  useEffect(() => {
    api<{ wallet: Wallet; items: Tx[] }>('/api/wallet/transactions').then(setData)
  }, [])

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mb: 2 }}>{t('wallet.title')}</Typography>
      {!data ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <>
          <Paper variant="outlined" sx={{ p: 2, mb: 2 }}>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={3} sx={{ flexWrap: 'wrap' }}>
              <Stat label={t('wallet.balance')} value={data.wallet.balance} highlight />
              <Stat label={t('wallet.blocked')} value={data.wallet.blocked} />
              <Stat label={t('wallet.total_received')} value={data.wallet.totalReceived} />
              <Stat label={t('wallet.total_paid_out')} value={data.wallet.totalPaidOut} />
            </Stack>
          </Paper>
          <TableContainer component={Paper} variant="outlined">
            <Table size="small">
              <TableHead>
                <TableRow sx={{ '& th': { fontWeight: 700, bgcolor: 'action.hover' } }}>
                  <TableCell>{t('wallet.col_date')}</TableCell>
                  <TableCell>{t('wallet.tx_type')}</TableCell>
                  <TableCell>{t('wallet.col_source')}</TableCell>
                  <TableCell align="right">{t('wallet.col_value')}</TableCell>
                  <TableCell align="right">{t('wallet.col_balance')}</TableCell>
                </TableRow>
              </TableHead>
              <TableBody>
                {data.items.length === 0 && (
                  <TableRow><TableCell colSpan={5} sx={{ color: 'text.secondary' }}>{t('wallet.empty')}</TableCell></TableRow>
                )}
                {data.items.map((tx) => {
                  const color = TYPE_COLOR[tx.type]
                  return (
                    <TableRow key={tx.id} hover>
                      <TableCell>{new Date(tx.createdAt).toLocaleString()}</TableCell>
                      <TableCell sx={color ? { color } : undefined}>{tx.type}</TableCell>
                      <TableCell>{tx.refType ?? '—'}</TableCell>
                      <TableCell align="right" sx={color ? { color } : undefined}>R$ {Number(tx.amount).toFixed(2)}</TableCell>
                      <TableCell align="right">R$ {Number(tx.balanceAfter).toFixed(2)}</TableCell>
                    </TableRow>
                  )
                })}
              </TableBody>
            </Table>
          </TableContainer>
        </>
      )}
    </Layout>
  )
}

function Stat({ label, value, highlight }: { label: string; value: string; highlight?: boolean }) {
  const valueSx: SxProps<Theme> = {
    fontSize: highlight ? 24 : 18,
    fontWeight: 600,
    color: highlight ? 'primary.main' : 'text.primary',
  }
  return (
    <Stack spacing={0.5}>
      <Typography variant="caption" color="text.secondary">{label}</Typography>
      <Typography sx={valueSx}>R$ {Number(value).toFixed(2)}</Typography>
    </Stack>
  )
}
