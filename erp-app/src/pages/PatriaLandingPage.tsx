import { BoltOutlined, RocketLaunchOutlined, StorefrontOutlined } from '@mui/icons-material'
import { Box, Button, Card, CardContent, Container, Typography } from '@mui/material'
import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { fetchSegments, getToken, type Segment } from '../api'
import { useT } from '../i18n/LangContext'
import { segLabel } from './SignupPage'

export function PatriaLandingPage() {
  const t = useT()
  const [segments, setSegments] = useState<Segment[]>([])

  useEffect(() => {
    fetchSegments().then(setSegments).catch(() => {})
  }, [])

  const features = [
    { icon: <RocketLaunchOutlined fontSize="large" />, title: t('patria.feat1_title'), text: t('patria.feat1_text') },
    { icon: <StorefrontOutlined fontSize="large" />, title: t('patria.feat2_title'), text: t('patria.feat2_text') },
    { icon: <BoltOutlined fontSize="large" />, title: t('patria.feat3_title'), text: t('patria.feat3_text') },
  ]

  return (
    <Box sx={{ minHeight: '100vh', bgcolor: 'background.default' }}>
      {/* topo */}
      <Box sx={{ position: 'absolute', top: 0, right: 0, p: 2, zIndex: 2, display: 'flex', gap: 1 }}>
        <Button component={Link} to={getToken() ? '/customers' : '/login'} variant="text" sx={{ color: 'primary.contrastText' }}>
          {getToken() ? t('landing.panel') : t('landing.login')}
        </Button>
      </Box>

      {/* hero */}
      <Box
        sx={{
          color: 'primary.contrastText',
          background: (th) => `linear-gradient(135deg, ${th.palette.primary.main}, ${th.palette.primary.dark})`,
          py: { xs: 9, md: 16 }, px: 2, textAlign: 'center',
        }}
      >
        <Container maxWidth="md">
          <Typography variant="overline" sx={{ opacity: 0.85, letterSpacing: 2 }}>Patria Technology</Typography>
          <Typography variant="h2" sx={{ fontWeight: 800, fontSize: { xs: 34, md: 56 }, mt: 1 }}>
            {t('patria.hero_title')}
          </Typography>
          <Typography sx={{ mt: 2, fontSize: { xs: 17, md: 21 }, opacity: 0.92 }}>
            {t('patria.hero_subtitle')}
          </Typography>
          <Button component={Link} to="/signup" variant="contained" size="large"
            sx={{ mt: 4, bgcolor: 'primary.contrastText', color: 'primary.main', px: 4, '&:hover': { bgcolor: 'primary.contrastText', opacity: 0.9 } }}>
            {t('patria.cta')}
          </Button>
          <Typography variant="body2" sx={{ mt: 1.5, opacity: 0.8 }}>{t('patria.cta_note')}</Typography>
        </Container>
      </Box>

      {/* features */}
      <Container maxWidth="lg" sx={{ py: { xs: 6, md: 9 } }}>
        <Box sx={{ display: 'grid', gap: 2, gridTemplateColumns: { xs: '1fr', md: 'repeat(3, 1fr)' } }}>
          {features.map((f, i) => (
            <Card key={i} variant="outlined">
              <CardContent sx={{ textAlign: 'center', py: 4 }}>
                <Box sx={{ color: 'primary.main', mb: 1 }}>{f.icon}</Box>
                <Typography variant="h6" sx={{ fontWeight: 700 }}>{f.title}</Typography>
                <Typography sx={{ mt: 1, color: 'text.secondary' }}>{f.text}</Typography>
              </CardContent>
            </Card>
          ))}
        </Box>
      </Container>

      {/* segmentos */}
      <Box sx={{ bgcolor: 'action.hover', py: { xs: 6, md: 9 } }}>
        <Container maxWidth="lg">
          <Typography variant="h4" sx={{ fontWeight: 700, textAlign: 'center', mb: 1 }}>{t('patria.segments_title')}</Typography>
          <Typography sx={{ textAlign: 'center', color: 'text.secondary', mb: 4 }}>{t('patria.segments_subtitle')}</Typography>
          <Box sx={{ display: 'grid', gap: 1.5, gridTemplateColumns: { xs: '1fr', sm: 'repeat(2, 1fr)', md: 'repeat(3, 1fr)' } }}>
            {segments.map((s) => (
              <Card key={s.slug} variant="outlined" component={Link} to={`/signup?segment=${s.slug}`} sx={{ textDecoration: 'none', transition: '0.15s', '&:hover': { borderColor: 'primary.main' } }}>
                <CardContent>
                  <Typography variant="h6" sx={{ fontWeight: 700 }}>{segLabel(t, s, 'name')}</Typography>
                  <Typography variant="body2" sx={{ mt: 0.5, color: 'text.secondary' }}>{segLabel(t, s, 'desc')}</Typography>
                </CardContent>
              </Card>
            ))}
          </Box>
        </Container>
      </Box>

      {/* cta final */}
      <Container maxWidth="md" sx={{ py: { xs: 6, md: 9 }, textAlign: 'center' }}>
        <Typography variant="h4" sx={{ fontWeight: 700 }}>{t('patria.final_cta_title')}</Typography>
        <Button component={Link} to="/signup" variant="contained" size="large" sx={{ mt: 3, px: 4 }}>
          {t('patria.cta')}
        </Button>
      </Container>

      <Box sx={{ borderTop: 1, borderColor: 'divider', py: 3, textAlign: 'center' }}>
        <Typography variant="body2" color="text.secondary">© Patria Technology</Typography>
      </Box>
    </Box>
  )
}
