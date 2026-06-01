import {
  CallOutlined,
  EmailOutlined,
  LocationOnOutlined,
  WhatsApp,
} from '@mui/icons-material'
import {
  Box,
  Button,
  Card,
  CardContent,
  Container,
  CssBaseline,
  Stack,
  Typography,
} from '@mui/material'
import { ThemeProvider } from '@mui/material/styles'
import { useEffect, useMemo, useState } from 'react'
import { Link } from 'react-router-dom'
import { fetchPublicSite, getToken, type PublicSite } from '../api'
import { useT } from '../i18n/LangContext'
import { buildMuiTheme } from '../theme/muiTheme'
import { defaultConfig, type ThemeConfig } from '../theme/palettes'

export function LandingPage() {
  const t = useT()
  const [site, setSite] = useState<PublicSite | null>(null)
  const [error, setError] = useState(false)

  useEffect(() => {
    fetchPublicSite().then(setSite).catch(() => setError(true))
  }, [])

  // Tema da landing: usa o tema salvo do tenant (se houver) só para esta página.
  const theme = useMemo(() => {
    const cfg = { ...defaultConfig(), ...(site?.theme as Partial<ThemeConfig> | undefined) }
    return buildMuiTheme(cfg)
  }, [site])

  if (error) {
    return (
      <Box sx={{ minHeight: '100vh', display: 'grid', placeItems: 'center', p: 3 }}>
        <Typography color="text.secondary">{t('landing.not_found')}</Typography>
      </Box>
    )
  }

  const l = site?.landing ?? {}
  const title = l.headline || site?.name || ''
  const adminHref = getToken() ? '/customers' : '/login'

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Box sx={{ minHeight: '100vh', bgcolor: 'background.default' }}>
        {/* topo */}
        <Box sx={{ position: 'absolute', top: 0, right: 0, p: 2, zIndex: 2 }}>
          <Button component={Link} to={adminHref} variant="text" sx={{ color: 'primary.contrastText' }}>
            {getToken() ? t('landing.panel') : t('landing.login')}
          </Button>
        </Box>

        {/* hero */}
        <Box
          sx={{
            color: 'primary.contrastText',
            background: (th) =>
              `linear-gradient(135deg, ${th.palette.primary.main}, ${th.palette.primary.dark})`,
            py: { xs: 8, md: 14 },
            px: 2,
            textAlign: 'center',
            ...(l.heroImageUrl && {
              backgroundImage: `linear-gradient(rgba(0,0,0,0.55), rgba(0,0,0,0.55)), url(${l.heroImageUrl})`,
              backgroundSize: 'cover',
              backgroundPosition: 'center',
            }),
          }}
        >
          <Container maxWidth="md">
            {l.logoUrl && (
              <Box component="img" src={l.logoUrl} alt={site?.name} sx={{ height: 72, mb: 3, objectFit: 'contain' }} />
            )}
            <Typography variant="h2" sx={{ fontWeight: 800, fontSize: { xs: 32, md: 52 } }}>
              {title}
            </Typography>
            {l.subheadline && (
              <Typography sx={{ mt: 2, fontSize: { xs: 16, md: 20 }, opacity: 0.92 }}>
                {l.subheadline}
              </Typography>
            )}
            {l.ctaText && (
              <Button
                href={l.ctaUrl || undefined}
                target={l.ctaUrl ? '_blank' : undefined}
                rel="noopener"
                variant="contained"
                size="large"
                sx={{ mt: 4, bgcolor: 'primary.contrastText', color: 'primary.main', '&:hover': { bgcolor: 'primary.contrastText', opacity: 0.9 } }}
              >
                {l.ctaText}
              </Button>
            )}
          </Container>
        </Box>

        {/* sobre */}
        {l.about && (
          <Container maxWidth="md" sx={{ py: { xs: 5, md: 8 } }}>
            <Typography variant="h4" sx={{ fontWeight: 700, mb: 2 }}>{t('landing.about')}</Typography>
            <Typography sx={{ fontSize: 17, color: 'text.secondary', whiteSpace: 'pre-line' }}>{l.about}</Typography>
          </Container>
        )}

        {/* serviços */}
        {l.services && l.services.length > 0 && (
          <Box sx={{ bgcolor: 'action.hover', py: { xs: 5, md: 8 } }}>
            <Container maxWidth="lg">
              <Typography variant="h4" sx={{ fontWeight: 700, mb: 3, textAlign: 'center' }}>{t('landing.services')}</Typography>
              <Box sx={{ display: 'grid', gap: 2, gridTemplateColumns: { xs: '1fr', sm: 'repeat(2, 1fr)', md: 'repeat(3, 1fr)' } }}>
                {l.services.map((s, i) => (
                  <Card key={i} variant="outlined">
                    <CardContent>
                      <Typography variant="h6" sx={{ fontWeight: 700 }}>{s.title}</Typography>
                      {s.description && (
                        <Typography sx={{ mt: 1, color: 'text.secondary' }}>{s.description}</Typography>
                      )}
                    </CardContent>
                  </Card>
                ))}
              </Box>
            </Container>
          </Box>
        )}

        {/* contato */}
        {l.contact && (l.contact.phone || l.contact.whatsapp || l.contact.email || l.contact.address) && (
          <Container maxWidth="md" sx={{ py: { xs: 5, md: 8 } }}>
            <Typography variant="h4" sx={{ fontWeight: 700, mb: 3 }}>{t('landing.contact')}</Typography>
            <Stack spacing={1.5}>
              {l.contact.whatsapp && (
                <Button startIcon={<WhatsApp />} variant="outlined" href={`https://wa.me/${l.contact.whatsapp}`} target="_blank" rel="noopener" sx={{ alignSelf: 'flex-start' }}>
                  WhatsApp
                </Button>
              )}
              {l.contact.phone && <ContactLine icon={<CallOutlined />} text={l.contact.phone} />}
              {l.contact.email && <ContactLine icon={<EmailOutlined />} text={l.contact.email} />}
              {l.contact.address && <ContactLine icon={<LocationOnOutlined />} text={l.contact.address} />}
            </Stack>
          </Container>
        )}

        {/* rodapé */}
        <Box sx={{ borderTop: 1, borderColor: 'divider', py: 3, textAlign: 'center' }}>
          <Typography variant="body2" color="text.secondary">
            © {site?.companyName || site?.name} · {t('landing.powered')}
          </Typography>
        </Box>
      </Box>
    </ThemeProvider>
  )
}

function ContactLine({ icon, text }: { icon: React.ReactNode; text: string }) {
  return (
    <Stack direction="row" spacing={1} sx={{ alignItems: 'center', color: 'text.secondary' }}>
      {icon}
      <Typography>{text}</Typography>
    </Stack>
  )
}
