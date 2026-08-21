import { Add, DeleteOutlined, OpenInNew } from '@mui/icons-material'
import {
  Box,
  Button,
  Divider,
  IconButton,
  Link as MuiLink,
  Paper,
  Stack,
  TextField,
  Typography,
} from '@mui/material'
import { useEffect, useState } from 'react'
import {
  fetchLanding,
  getTenantAlias,
  updateLanding,
  type LandingConfig,
} from '../api'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

export function SiteEditorPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const [form, setForm] = useState<LandingConfig>({})
  const [loaded, setLoaded] = useState(false)
  const [saving, setSaving] = useState(false)

  useEffect(() => {
    fetchLanding()
      .then((r) => { setForm(r.landing ?? {}); setLoaded(true) })
      .catch((e) => snackbar.error((e as Error).message))
  }, [])

  function set<K extends keyof LandingConfig>(key: K, value: LandingConfig[K]) {
    setForm((f) => ({ ...f, [key]: value }))
  }
  function setContact(key: string, value: string) {
    setForm((f) => ({ ...f, contact: { ...f.contact, [key]: value } }))
  }
  function setService(i: number, key: 'title' | 'description', value: string) {
    setForm((f) => {
      const services = [...(f.services ?? [])]
      services[i] = { ...services[i], [key]: value }
      return { ...f, services }
    })
  }
  function addService() {
    setForm((f) => ({ ...f, services: [...(f.services ?? []), { title: '' }] }))
  }
  function removeService(i: number) {
    setForm((f) => ({ ...f, services: (f.services ?? []).filter((_, j) => j !== i) }))
  }
  function setSocial(key: string, value: string) {
    setForm((f) => ({ ...f, social: { ...f.social, [key]: value } }))
  }
  // galeria
  function setGallery(i: number, value: string) {
    setForm((f) => { const g = [...(f.gallery ?? [])]; g[i] = value; return { ...f, gallery: g } })
  }
  function addGallery() { setForm((f) => ({ ...f, gallery: [...(f.gallery ?? []), ''] })) }
  function removeGallery(i: number) { setForm((f) => ({ ...f, gallery: (f.gallery ?? []).filter((_, j) => j !== i) })) }
  // horários
  function setHour(i: number, key: 'label' | 'value', value: string) {
    setForm((f) => { const h = [...(f.hours ?? [])]; h[i] = { ...h[i], [key]: value }; return { ...f, hours: h } })
  }
  function addHour() { setForm((f) => ({ ...f, hours: [...(f.hours ?? []), { label: '', value: '' }] })) }
  function removeHour(i: number) { setForm((f) => ({ ...f, hours: (f.hours ?? []).filter((_, j) => j !== i) })) }

  async function save() {
    setSaving(true)
    try {
      // remove itens vazios
      const clean: LandingConfig = {
        ...form,
        services: (form.services ?? []).filter((s) => s.title.trim()),
        gallery: (form.gallery ?? []).filter((u) => u.trim()),
        hours: (form.hours ?? []).filter((h) => h.label.trim()),
      }
      const r = await updateLanding(clean)
      setForm(r.landing ?? {})
      snackbar.success(t('site.saved'))
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setSaving(false)
    }
  }

  const alias = getTenantAlias()
  const previewHref = alias ? `/?tenant=${encodeURIComponent(alias)}` : '/'

  return (
    <Layout>
      <Stack direction="row" sx={{ alignItems: 'center', justifyContent: 'space-between', mb: 2, flexWrap: 'wrap', gap: 1 }}>
        <Box>
          <Typography variant="h5" sx={{ fontWeight: 700 }}>{t('site.title')}</Typography>
          <Typography variant="body2" color="text.secondary">{t('site.subtitle')}</Typography>
        </Box>
        <Stack direction="row" spacing={1}>
          <Button component={MuiLink} href={previewHref} target="_blank" rel="noopener" variant="outlined" startIcon={<OpenInNew />}>
            {t('site.preview')}
          </Button>
          <Button onClick={() => void save()} disabled={!loaded || saving} variant="contained">
            {saving ? '…' : t('common.save')}
          </Button>
        </Stack>
      </Stack>

      {!loaded ? (
        <Typography color="text.secondary">{t('common.loading')}</Typography>
      ) : (
        <Stack spacing={2}>
          <Paper variant="outlined" sx={{ p: 2 }}>
            <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('site.hero')}</Typography>
            <Stack spacing={1.5}>
              <TextField label={t('site.headline')} value={form.headline ?? ''} onChange={(e) => set('headline', e.target.value)} size="small" fullWidth />
              <TextField label={t('site.subheadline')} value={form.subheadline ?? ''} onChange={(e) => set('subheadline', e.target.value)} size="small" fullWidth />
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                <TextField label={t('site.cta_text')} value={form.ctaText ?? ''} onChange={(e) => set('ctaText', e.target.value)} size="small" fullWidth />
                <TextField label={t('site.cta_url')} value={form.ctaUrl ?? ''} onChange={(e) => set('ctaUrl', e.target.value)} size="small" fullWidth />
              </Stack>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                <TextField label={t('site.logo_url')} value={form.logoUrl ?? ''} onChange={(e) => set('logoUrl', e.target.value)} size="small" fullWidth />
                <TextField label={t('site.hero_image_url')} value={form.heroImageUrl ?? ''} onChange={(e) => set('heroImageUrl', e.target.value)} size="small" fullWidth />
              </Stack>
            </Stack>
          </Paper>

          <Paper variant="outlined" sx={{ p: 2 }}>
            <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('site.about')}</Typography>
            <TextField value={form.about ?? ''} onChange={(e) => set('about', e.target.value)} size="small" fullWidth multiline minRows={4} />
          </Paper>

          <Paper variant="outlined" sx={{ p: 2 }}>
            <Stack direction="row" sx={{ alignItems: 'center', justifyContent: 'space-between', mb: 1.5 }}>
              <Typography variant="subtitle1" sx={{ fontWeight: 600 }}>{t('site.services')}</Typography>
              <Button size="small" startIcon={<Add />} onClick={addService}>{t('site.add_service')}</Button>
            </Stack>
            <Stack spacing={1.5}>
              {(form.services ?? []).length === 0 && (
                <Typography variant="body2" color="text.secondary">{t('site.no_services')}</Typography>
              )}
              {(form.services ?? []).map((s, i) => (
                <Stack key={i} direction="row" spacing={1} sx={{ alignItems: 'center' }}>
                  <TextField label={t('site.service_title')} value={s.title} onChange={(e) => setService(i, 'title', e.target.value)} size="small" sx={{ flex: 1 }} />
                  <TextField label={t('site.service_desc')} value={s.description ?? ''} onChange={(e) => setService(i, 'description', e.target.value)} size="small" sx={{ flex: 2 }} />
                  <IconButton color="error" onClick={() => removeService(i)} aria-label="remover"><DeleteOutlined /></IconButton>
                </Stack>
              ))}
            </Stack>
          </Paper>

          {/* galeria */}
          <Paper variant="outlined" sx={{ p: 2 }}>
            <Stack direction="row" sx={{ alignItems: 'center', justifyContent: 'space-between', mb: 1.5 }}>
              <Typography variant="subtitle1" sx={{ fontWeight: 600 }}>{t('site.gallery')}</Typography>
              <Button size="small" startIcon={<Add />} onClick={addGallery}>{t('site.add_photo')}</Button>
            </Stack>
            <Typography variant="caption" color="text.secondary" sx={{ display: 'block', mb: 1 }}>{t('site.gallery_hint')}</Typography>
            <Stack spacing={1}>
              {(form.gallery ?? []).length === 0 && (
                <Typography variant="body2" color="text.secondary">{t('site.no_photos')}</Typography>
              )}
              {(form.gallery ?? []).map((url, i) => (
                <Stack key={i} direction="row" spacing={1} sx={{ alignItems: 'center' }}>
                  {url && <Box component="img" src={url} alt="" sx={{ width: 40, height: 40, objectFit: 'cover', borderRadius: 1, border: 1, borderColor: 'divider' }} />}
                  <TextField label={t('site.photo_url')} value={url} onChange={(e) => setGallery(i, e.target.value)} size="small" sx={{ flex: 1 }} />
                  <IconButton color="error" onClick={() => removeGallery(i)} aria-label="remover"><DeleteOutlined /></IconButton>
                </Stack>
              ))}
            </Stack>
          </Paper>

          {/* horários */}
          <Paper variant="outlined" sx={{ p: 2 }}>
            <Stack direction="row" sx={{ alignItems: 'center', justifyContent: 'space-between', mb: 1.5 }}>
              <Typography variant="subtitle1" sx={{ fontWeight: 600 }}>{t('site.hours')}</Typography>
              <Button size="small" startIcon={<Add />} onClick={addHour}>{t('site.add_hour')}</Button>
            </Stack>
            <Stack spacing={1}>
              {(form.hours ?? []).length === 0 && (
                <Typography variant="body2" color="text.secondary">{t('site.no_hours')}</Typography>
              )}
              {(form.hours ?? []).map((h, i) => (
                <Stack key={i} direction="row" spacing={1} sx={{ alignItems: 'center' }}>
                  <TextField label={t('site.hour_label')} placeholder="Seg a Sex" value={h.label} onChange={(e) => setHour(i, 'label', e.target.value)} size="small" sx={{ flex: 1 }} />
                  <TextField label={t('site.hour_value')} placeholder="08:00 – 18:00" value={h.value ?? ''} onChange={(e) => setHour(i, 'value', e.target.value)} size="small" sx={{ flex: 1 }} />
                  <IconButton color="error" onClick={() => removeHour(i)} aria-label="remover"><DeleteOutlined /></IconButton>
                </Stack>
              ))}
            </Stack>
          </Paper>

          <Paper variant="outlined" sx={{ p: 2 }}>
            <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('site.contact')}</Typography>
            <Stack spacing={1.5}>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                <TextField label={t('site.phone')} value={form.contact?.phone ?? ''} onChange={(e) => setContact('phone', e.target.value)} size="small" fullWidth />
                <TextField label="WhatsApp" value={form.contact?.whatsapp ?? ''} onChange={(e) => setContact('whatsapp', e.target.value)} size="small" fullWidth helperText={t('site.whatsapp_hint')} />
              </Stack>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
                <TextField label={t('common.email')} value={form.contact?.email ?? ''} onChange={(e) => setContact('email', e.target.value)} size="small" fullWidth />
                <TextField label={t('site.address')} value={form.contact?.address ?? ''} onChange={(e) => setContact('address', e.target.value)} size="small" fullWidth />
              </Stack>
            </Stack>
          </Paper>

          {/* redes sociais */}
          <Paper variant="outlined" sx={{ p: 2 }}>
            <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1.5 }}>{t('site.social')}</Typography>
            <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5}>
              <TextField label="Instagram" placeholder="@seunegocio" value={form.social?.instagram ?? ''} onChange={(e) => setSocial('instagram', e.target.value)} size="small" fullWidth />
              <TextField label="Facebook" value={form.social?.facebook ?? ''} onChange={(e) => setSocial('facebook', e.target.value)} size="small" fullWidth />
              <TextField label={t('site.website')} value={form.social?.website ?? ''} onChange={(e) => setSocial('website', e.target.value)} size="small" fullWidth />
            </Stack>
          </Paper>

          <Divider />
          <Box>
            <Button onClick={() => void save()} disabled={saving} variant="contained">
              {saving ? '…' : t('common.save')}
            </Button>
          </Box>
        </Stack>
      )}
    </Layout>
  )
}
