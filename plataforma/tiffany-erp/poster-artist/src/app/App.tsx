import { useEffect, useMemo, useState } from 'react'
import {
  AppBar,
  Box,
  Button,
  CircularProgress,
  Container,
  MenuItem,
  TextField,
  Toolbar,
  Typography,
} from '@mui/material'
import PictureAsPdfIcon from '@mui/icons-material/PictureAsPdf'
import { FONT_FAMILIES } from '../doc/engine/fonts'
import { layoutDocument } from '../doc/render/layout'
import { renderDocumentPdf } from '../doc/render/pdf'
import { DocPreview } from '../doc/render/DocPreview'
import type { LaidOutDocument } from '../doc/render/types'
import { buildServiceOrder } from '../doc/templates/serviceOrder'
import { MOCK_SERVICE_ORDER } from '../mocks/serviceOrder'

export function App() {
  const [family, setFamily] = useState('Inter')
  const [zoom, setZoom] = useState(1)
  const [doc, setDoc] = useState<LaidOutDocument | null>(null)
  const [loading, setLoading] = useState(true)
  const [exporting, setExporting] = useState(false)

  const spec = useMemo(() => {
    const s = buildServiceOrder(MOCK_SERVICE_ORDER)
    return { ...s, fontFamily: family }
  }, [family])

  useEffect(() => {
    let alive = true
    setLoading(true)
    layoutDocument(spec)
      .then((laid) => {
        if (alive) {
          setDoc(laid)
          setLoading(false)
        }
      })
      .catch((err) => {
        console.error('[layout] falhou', err)
        if (alive) setLoading(false)
      })
    return () => {
      alive = false
    }
  }, [spec])

  async function handleExport() {
    if (!doc) return
    setExporting(true)
    try {
      // O PDF reusa o MESMO LaidOutDocument do preview → idêntico ao que se vê.
      const bytes = await renderDocumentPdf(doc)
      const blob = new Blob([bytes as BlobPart], { type: 'application/pdf' })
      const url = URL.createObjectURL(blob)
      const a = document.createElement('a')
      a.href = url
      a.download = `ordem-de-servico-${MOCK_SERVICE_ORDER.number}.pdf`
      a.click()
      URL.revokeObjectURL(url)
    } catch (err) {
      console.error('[export] falhou', err)
    } finally {
      setExporting(false)
    }
  }

  return (
    <Box sx={{ minHeight: '100vh', bgcolor: 'background.default' }}>
      <AppBar position="sticky" color="default" elevation={1}>
        <Toolbar sx={{ gap: 2 }}>
          <Typography variant="h6" sx={{ fontWeight: 700, flexShrink: 0 }}>
            Patria Poster Artist
          </Typography>
          <Typography variant="body2" color="text.secondary" sx={{ flexGrow: 1 }}>
            Ordem de Serviço · A4
          </Typography>

          <TextField
            select
            size="small"
            label="Fonte"
            value={family}
            onChange={(e) => setFamily(e.target.value)}
            sx={{ width: 160 }}
          >
            {FONT_FAMILIES.map((f) => (
              <MenuItem key={f} value={f}>
                {f}
              </MenuItem>
            ))}
          </TextField>

          <TextField
            select
            size="small"
            label="Zoom"
            value={zoom}
            onChange={(e) => setZoom(Number(e.target.value))}
            sx={{ width: 100 }}
          >
            {[0.75, 1, 1.25, 1.5].map((z) => (
              <MenuItem key={z} value={z}>
                {Math.round(z * 100)}%
              </MenuItem>
            ))}
          </TextField>

          <Button
            variant="contained"
            startIcon={<PictureAsPdfIcon />}
            onClick={handleExport}
            disabled={!doc || exporting}
          >
            {exporting ? 'Gerando…' : 'Exportar PDF'}
          </Button>
        </Toolbar>
      </AppBar>

      <Container maxWidth={false} sx={{ py: 4 }}>
        {loading || !doc ? (
          <Box
            sx={{
              py: 8,
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              gap: 2,
            }}
          >
            <CircularProgress />
            <Typography color="text.secondary">Montando documento…</Typography>
          </Box>
        ) : (
          <DocPreview doc={doc} zoom={zoom} />
        )}
      </Container>
    </Box>
  )
}
