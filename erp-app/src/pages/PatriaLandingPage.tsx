import {
  AccountBalanceWalletOutlined,
  BoltOutlined,
  EventNoteOutlined,
  Groups2Outlined,
  Inventory2Outlined,
  LanguageOutlined,
  MonetizationOnOutlined,
  PointOfSaleOutlined,
  RocketLaunchOutlined,
  ViewModuleOutlined,
} from '@mui/icons-material'
import { Box, Button, Card, CardContent, Chip, Container, Stack, Typography } from '@mui/material'
import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { fetchSegments, getToken, type Segment } from '../api'
import { useT } from '../i18n/LangContext'
import { LangSwitcher } from '../i18n/LangSwitcher'
import { segLabel } from './SignupPage'

export function PatriaLandingPage() {
  const t = useT()
  const [segments, setSegments] = useState<Segment[]>([])

  useEffect(() => {
    fetchSegments().then(setSegments).catch(() => {})
  }, [])

  const logged = !!getToken()

  const features = [
    { icon: <BoltOutlined fontSize="large" />, title: t('patria.feat_fast_title'), text: t('patria.feat_fast_text') },
    { icon: <MonetizationOnOutlined fontSize="large" />, title: t('patria.feat_free_title'), text: t('patria.feat_free_text') },
    { icon: <ViewModuleOutlined fontSize="large" />, title: t('patria.feat_allinone_title'), text: t('patria.feat_allinone_text') },
    { icon: <LanguageOutlined fontSize="large" />, title: t('patria.feat_site_title'), text: t('patria.feat_site_text') },
  ]

  const steps = [
    { n: 1, title: t('patria.step1_title'), text: t('patria.step1_text') },
    { n: 2, title: t('patria.step2_title'), text: t('patria.step2_text') },
    { n: 3, title: t('patria.step3_title'), text: t('patria.step3_text') },
  ]

  const capabilities = [
    { icon: <PointOfSaleOutlined />, label: t('patria.cap_pos') },
    { icon: <Inventory2Outlined />, label: t('patria.cap_stock') },
    { icon: <AccountBalanceWalletOutlined />, label: t('patria.cap_cash') },
    { icon: <Groups2Outlined />, label: t('patria.cap_customers') },
    { icon: <EventNoteOutlined />, label: t('patria.cap_enrollments') },
    { icon: <LanguageOutlined />, label: t('patria.cap_site') },
  ]

  return (
    <Box sx={{ minHeight: '100vh', bgcolor: 'background.default' }}>
      {/* nav */}
      <Box sx={{ position: 'absolute', top: 0, left: 0, right: 0, zIndex: 2, px: { xs: 2, md: 4 }, py: 1.5, display: 'flex', alignItems: 'center', color: 'primary.contrastText' }}>
        <Typography sx={{ fontWeight: 800, flex: 1, letterSpacing: 0.3 }}>Patria Technology</Typography>
        <LangSwitcher />
        <Button component={Link} to={logged ? '/customers' : '/login'} variant="text" sx={{ color: 'primary.contrastText', ml: 1 }}>
          {logged ? t('landing.panel') : t('landing.login')}
        </Button>
      </Box>

      {/* hero */}
      <Box
        sx={{
          color: 'primary.contrastText',
          background: (th) => `linear-gradient(135deg, ${th.palette.primary.main}, ${th.palette.primary.dark})`,
          pt: { xs: 12, md: 18 }, pb: { xs: 9, md: 14 }, px: 2, textAlign: 'center',
        }}
      >
        <Container maxWidth="md">
          <Chip label={t('patria.tagline')} sx={{ mb: 2, bgcolor: 'rgba(255,255,255,0.18)', color: 'primary.contrastText', fontWeight: 600 }} />
          <Typography variant="h2" sx={{ fontWeight: 800, fontSize: { xs: 32, md: 56 }, lineHeight: 1.1 }}>
            {t('patria.hero_title')}
          </Typography>
          <Typography sx={{ mt: 2.5, fontSize: { xs: 17, md: 22 }, opacity: 0.92, maxWidth: 680, mx: 'auto' }}>
            {t('patria.hero_subtitle')}
          </Typography>
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ mt: 4, justifyContent: 'center', alignItems: 'center' }}>
            <Button component={Link} to="/signup" variant="contained" size="large"
              sx={{ bgcolor: 'primary.contrastText', color: 'primary.main', px: 4, fontSize: 16, '&:hover': { bgcolor: 'primary.contrastText', opacity: 0.9 } }}>
              {t('patria.cta')}
            </Button>
            <Button component={Link} to="/signup" variant="outlined" size="large"
              sx={{ color: 'primary.contrastText', borderColor: 'rgba(255,255,255,0.5)', px: 3, '&:hover': { borderColor: 'primary.contrastText' } }}>
              {t('patria.cta_secondary')}
            </Button>
          </Stack>
          <Typography variant="body2" sx={{ mt: 1.5, opacity: 0.85 }}>✓ {t('patria.cta_note')}</Typography>
        </Container>
      </Box>

      {/* diferenciais */}
      <Container maxWidth="lg" sx={{ py: { xs: 6, md: 9 } }}>
        <Box sx={{ display: 'grid', gap: 2, gridTemplateColumns: { xs: '1fr', sm: 'repeat(2, 1fr)', md: 'repeat(4, 1fr)' } }}>
          {features.map((f, i) => (
            <Card key={i} variant="outlined" sx={{ height: '100%' }}>
              <CardContent sx={{ textAlign: 'center', py: 4 }}>
                <Box sx={{ color: 'primary.main', mb: 1 }}>{f.icon}</Box>
                <Typography variant="h6" sx={{ fontWeight: 700 }}>{f.title}</Typography>
                <Typography sx={{ mt: 1, color: 'text.secondary', fontSize: 14 }}>{f.text}</Typography>
              </CardContent>
            </Card>
          ))}
        </Box>
      </Container>

      {/* como funciona */}
      <Box sx={{ bgcolor: 'action.hover', py: { xs: 6, md: 9 } }}>
        <Container maxWidth="lg">
          <Typography variant="h4" sx={{ fontWeight: 700, textAlign: 'center', mb: 1 }}>{t('patria.how_title')}</Typography>
          <Typography sx={{ textAlign: 'center', color: 'text.secondary', mb: 5 }}>{t('patria.how_subtitle')}</Typography>
          <Box sx={{ display: 'grid', gap: 3, gridTemplateColumns: { xs: '1fr', md: 'repeat(3, 1fr)' } }}>
            {steps.map((s) => (
              <Box key={s.n} sx={{ textAlign: 'center' }}>
                <Box sx={{ width: 56, height: 56, mx: 'auto', mb: 1.5, borderRadius: '50%', bgcolor: 'primary.main', color: 'primary.contrastText', display: 'flex', alignItems: 'center', justifyContent: 'center', fontWeight: 800, fontSize: 22 }}>
                  {s.n}
                </Box>
                <Typography variant="h6" sx={{ fontWeight: 700 }}>{s.title}</Typography>
                <Typography sx={{ mt: 0.5, color: 'text.secondary' }}>{s.text}</Typography>
              </Box>
            ))}
          </Box>
        </Container>
      </Box>

      {/* segmentos */}
      <Container maxWidth="lg" sx={{ py: { xs: 6, md: 9 } }}>
        <Typography variant="h4" sx={{ fontWeight: 700, textAlign: 'center', mb: 1 }}>{t('patria.segments_title')}</Typography>
        <Typography sx={{ textAlign: 'center', color: 'text.secondary', mb: 4 }}>{t('patria.segments_subtitle')}</Typography>
        <Box sx={{ display: 'grid', gap: 1.5, gridTemplateColumns: { xs: '1fr', sm: 'repeat(2, 1fr)', md: 'repeat(3, 1fr)' } }}>
          {segments.map((s) => (
            <Card key={s.slug} variant="outlined" component={Link} to={`/signup?segment=${s.slug}`}
              sx={{ textDecoration: 'none', transition: '0.15s', '&:hover': { borderColor: 'primary.main', transform: 'translateY(-2px)' } }}>
              <CardContent>
                <Typography variant="h6" sx={{ fontWeight: 700 }}>{segLabel(t, s, 'name')}</Typography>
                <Typography variant="body2" sx={{ mt: 0.5, color: 'text.secondary' }}>{segLabel(t, s, 'desc')}</Typography>
              </CardContent>
            </Card>
          ))}
        </Box>
      </Container>

      {/* tudo num lugar */}
      <Box sx={{ bgcolor: 'action.hover', py: { xs: 6, md: 9 } }}>
        <Container maxWidth="md" sx={{ textAlign: 'center' }}>
          <Typography variant="h4" sx={{ fontWeight: 700, mb: 1 }}>{t('patria.included_title')}</Typography>
          <Typography sx={{ color: 'text.secondary', mb: 4 }}>{t('patria.included_subtitle')}</Typography>
          <Stack direction="row" spacing={1} useFlexGap sx={{ flexWrap: 'wrap', justifyContent: 'center' }}>
            {capabilities.map((c, i) => (
              <Chip key={i} icon={c.icon} label={c.label} variant="outlined" sx={{ py: 2.2, px: 0.5, fontSize: 14, bgcolor: 'background.paper' }} />
            ))}
          </Stack>
        </Container>
      </Box>

      {/* preço + cta final */}
      <Container maxWidth="md" sx={{ py: { xs: 7, md: 10 }, textAlign: 'center' }}>
        <RocketLaunchOutlined sx={{ fontSize: 44, color: 'primary.main', mb: 1 }} />
        <Typography variant="h4" sx={{ fontWeight: 800 }}>{t('patria.pricing_title')}</Typography>
        <Typography sx={{ mt: 1, color: 'text.secondary', fontSize: 18 }}>{t('patria.pricing_text')}</Typography>
        <Button component={Link} to="/signup" variant="contained" size="large" sx={{ mt: 3, px: 5, fontSize: 16 }}>
          {t('patria.cta')}
        </Button>
        <Typography variant="body2" sx={{ mt: 1.5, color: 'text.secondary' }}>{t('patria.cta_note')}</Typography>
      </Container>

      {/* footer */}
      <Box sx={{ borderTop: 1, borderColor: 'divider', py: 4, textAlign: 'center' }}>
        <Typography sx={{ fontWeight: 700 }}>Patria Technology</Typography>
        <Typography variant="body2" color="text.secondary" sx={{ mt: 0.5 }}>{t('patria.footer_tagline')}</Typography>
        <Typography variant="caption" color="text.secondary" sx={{ display: 'block', mt: 1 }}>© Patria Technology</Typography>
      </Box>
    </Box>
  )
}
