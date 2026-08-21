import {
  AccessTimeOutlined,
  CallOutlined,
  EmailOutlined,
  Facebook,
  Instagram,
  LanguageOutlined,
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
  Fab,
  IconButton,
  Stack,
  Tooltip,
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
    fetchPublicSite()
      .then((s) => {
        setSite(s)
        if (s?.name) document.title = s.name
      })
      .catch(() => setError(true))
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

        {/* galeria */}
        {l.gallery && l.gallery.length > 0 && (
          <Container maxWidth="lg" sx={{ py: { xs: 5, md: 8 } }}>
            <Typography variant="h4" sx={{ fontWeight: 700, mb: 3, textAlign: 'center' }}>{t('landing.gallery')}</Typography>
            <Box sx={{ display: 'grid', gap: 1.5, gridTemplateColumns: { xs: 'repeat(2, 1fr)', sm: 'repeat(3, 1fr)', md: 'repeat(4, 1fr)' } }}>
              {l.gallery.filter(Boolean).map((url, i) => (
                <Box key={i} component="img" src={url} alt="" loading="lazy"
                  sx={{ width: '100%', aspectRatio: '1 / 1', objectFit: 'cover', borderRadius: 2, border: 1, borderColor: 'divider' }} />
              ))}
            </Box>
          </Container>
        )}

        {/* horários */}
        {l.hours && l.hours.length > 0 && (
          <Box sx={{ bgcolor: 'action.hover', py: { xs: 5, md: 8 } }}>
            <Container maxWidth="sm">
              <Stack direction="row" spacing={1} sx={{ alignItems: 'center', justifyContent: 'center', mb: 3 }}>
                <AccessTimeOutlined color="primary" />
                <Typography variant="h4" sx={{ fontWeight: 700 }}>{t('landing.hours')}</Typography>
              </Stack>
              <Stack spacing={1}>
                {l.hours.filter((h) => h.label).map((h, i) => (
                  <Stack key={i} direction="row" sx={{ justifyContent: 'space-between', borderBottom: 1, borderColor: 'divider', pb: 1 }}>
                    <Typography sx={{ fontWeight: 600 }}>{h.label}</Typography>
                    <Typography color="text.secondary">{h.value || '—'}</Typography>
                  </Stack>
                ))}
              </Stack>
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
        <Box sx={{ borderTop: 1, borderColor: 'divider', py: 4, textAlign: 'center' }}>
          {l.social && (l.social.instagram || l.social.facebook || l.social.website) && (
            <Stack direction="row" spacing={1} sx={{ justifyContent: 'center', mb: 1.5 }}>
              {l.social.instagram && (
                <Tooltip title="Instagram"><IconButton color="primary" href={socialUrl('instagram', l.social.instagram)} target="_blank" rel="noopener"><Instagram /></IconButton></Tooltip>
              )}
              {l.social.facebook && (
                <Tooltip title="Facebook"><IconButton color="primary" href={socialUrl('facebook', l.social.facebook)} target="_blank" rel="noopener"><Facebook /></IconButton></Tooltip>
              )}
              {l.social.website && (
                <Tooltip title="Site"><IconButton color="primary" href={socialUrl('website', l.social.website)} target="_blank" rel="noopener"><LanguageOutlined /></IconButton></Tooltip>
              )}
            </Stack>
          )}
          <Typography variant="body2" color="text.secondary">
            © {site?.companyName || site?.name} · {t('landing.powered')}
          </Typography>
        </Box>
      </Box>

      {/* WhatsApp flutuante */}
      {l.contact?.whatsapp && (
        <Fab
          color="success"
          href={`https://wa.me/${l.contact.whatsapp}`}
          target="_blank"
          rel="noopener"
          aria-label="WhatsApp"
          sx={{ position: 'fixed', bottom: 20, right: 20, bgcolor: '#25D366', '&:hover': { bgcolor: '#1da851' } }}
        >
          <WhatsApp />
        </Fab>
      )}
    </ThemeProvider>
  )
}

// normaliza handle/url das redes sociais
function socialUrl(kind: 'instagram' | 'facebook' | 'website', v: string): string {
  const s = v.trim()
  if (/^https?:\/\//i.test(s)) return s
  const handle = s.replace(/^@/, '')
  if (kind === 'instagram') return `https://instagram.com/${handle}`
  if (kind === 'facebook') return `https://facebook.com/${handle}`
  return `https://${s}`
}

function ContactLine({ icon, text }: { icon: React.ReactNode; text: string }) {
  return (
    <Stack direction="row" spacing={1} sx={{ alignItems: 'center', color: 'text.secondary' }}>
      {icon}
      <Typography>{text}</Typography>
    </Stack>
  )
}
