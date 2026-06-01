import { useEffect, useMemo, useState } from 'react'
import {
  Box,
  Button,
  Checkbox,
  Chip,
  FormControlLabel,
  Paper,
  Stack,
  Tab,
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableRow,
  Tabs,
  TextField,
  Typography,
} from '@mui/material'
import {
  createRole,
  deleteRole,
  fetchAccessCatalog,
  fetchAccessUsers,
  fetchRoles,
  setUserRoles,
  updateRole,
  type AccessCatalog,
  type AccessRoleItem,
  type AccessRule,
  type AccessUser,
} from '../api'
import { useAbility } from '../access/AbilityContext'
import { Layout } from '../components/Layout'
import { useSnackbar } from '../components/Snackbar'
import { useT } from '../i18n/LangContext'

type Matrix = Record<string, Record<string, boolean>> // subject -> action -> on

function asArr(v: string | string[]): string[] {
  return Array.isArray(v) ? v : [v]
}

// rules da API -> matriz de checkboxes
function rulesToMatrix(rules: AccessRule[]): Matrix {
  const m: Matrix = {}
  for (const r of rules) {
    if (r.inverted) continue
    for (const s of asArr(r.subject)) {
      m[s] ??= {}
      for (const a of asArr(r.action)) m[s][a] = true
    }
  }
  return m
}

// matriz -> rules (manage anula as demais ações daquele recurso)
function matrixToRules(m: Matrix): AccessRule[] {
  const rules: AccessRule[] = []
  for (const [subject, actions] of Object.entries(m)) {
    const checked = Object.entries(actions).filter(([, on]) => on).map(([a]) => a)
    if (checked.length === 0) continue
    const finalActions = checked.includes('manage') ? ['manage'] : checked
    rules.push({
      action: finalActions.length === 1 ? finalActions[0] : finalActions,
      subject,
    })
  }
  return rules
}

interface Draft {
  id: string | null // null = novo
  name: string
  description: string
  isSystem: boolean
  matrix: Matrix
}

export function AccessControlPage() {
  const t = useT()
  const snackbar = useSnackbar()
  const { ability, refresh: refreshAbility } = useAbility()
  const canManage = ability.can('manage', 'Role') || ability.can('read', 'Role')

  const [tab, setTab] = useState<'roles' | 'users'>('roles')
  const [catalog, setCatalog] = useState<AccessCatalog | null>(null)
  const [roles, setRoles] = useState<AccessRoleItem[]>([])
  const [users, setUsers] = useState<AccessUser[]>([])
  const [draft, setDraft] = useState<Draft | null>(null)
  const [saving, setSaving] = useState(false)

  async function loadAll() {
    try {
      const [cat, rs, us] = await Promise.all([
        fetchAccessCatalog(),
        fetchRoles(),
        fetchAccessUsers(),
      ])
      setCatalog(cat)
      setRoles(rs)
      setUsers(us)
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  useEffect(() => {
    if (canManage) void loadAll()
  }, [canManage])

  // recursos agrupados por módulo
  const groups = useMemo(() => {
    const g: Record<string, string[]> = {}
    for (const s of catalog?.subjects ?? []) (g[s.module] ??= []).push(s.key)
    return g
  }, [catalog])

  function newDraft() {
    setDraft({ id: null, name: '', description: '', isSystem: false, matrix: {} })
  }

  function editDraft(r: AccessRoleItem) {
    setDraft({
      id: r.id,
      name: r.name,
      description: r.description ?? '',
      isSystem: r.isSystem,
      matrix: rulesToMatrix(r.rules),
    })
  }

  function toggleCell(subject: string, action: string) {
    setDraft((d) => {
      if (!d) return d
      const cur = d.matrix[subject]?.[action] ?? false
      const matrix = { ...d.matrix, [subject]: { ...d.matrix[subject], [action]: !cur } }
      return { ...d, matrix }
    })
  }

  async function save() {
    if (!draft || !draft.name.trim()) return
    setSaving(true)
    try {
      const rules = matrixToRules(draft.matrix)
      if (draft.id) {
        await updateRole(draft.id, { name: draft.name, description: draft.description, rules })
        snackbar.success(t('access.saved'))
      } else {
        await createRole({ name: draft.name, description: draft.description, rules })
        snackbar.success(t('access.created'))
      }
      setDraft(null)
      await loadAll()
      await refreshAbility()
    } catch (e) {
      snackbar.error((e as Error).message)
    } finally {
      setSaving(false)
    }
  }

  async function removeRole(r: AccessRoleItem) {
    if (!confirm(t('access.delete_confirm', { name: r.name }))) return
    try {
      await deleteRole(r.id)
      snackbar.success(t('access.deleted'))
      await loadAll()
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  async function toggleUserRole(u: AccessUser, roleId: string, on: boolean) {
    const ids = new Set(u.accessRoles.map((r) => r.id))
    if (on) ids.add(roleId)
    else ids.delete(roleId)
    try {
      const updated = await setUserRoles(u.id, [...ids])
      setUsers(updated)
      snackbar.success(t('access.roles_updated'))
      await refreshAbility()
    } catch (e) {
      snackbar.error((e as Error).message)
    }
  }

  if (!canManage) {
    return (
      <Layout>
        <Typography variant="h5" sx={{ fontWeight: 700, mt: 0 }}>{t('access.title')}</Typography>
        <Typography color="text.secondary">{t('access.no_permission')}</Typography>
      </Layout>
    )
  }

  const actions = catalog?.actions ?? ['manage', 'create', 'read', 'update', 'delete']

  return (
    <Layout>
      <Typography variant="h5" sx={{ fontWeight: 700, mt: 0 }}>{t('access.title')}</Typography>
      <Typography color="text.secondary" sx={{ mt: 0.5 }}>{t('access.subtitle')}</Typography>

      <Tabs value={tab} onChange={(_, v) => setTab(v as 'roles' | 'users')} sx={{ mb: 2 }}>
        <Tab value="roles" label={t('access.tab.roles')} />
        <Tab value="users" label={t('access.tab.users')} />
      </Tabs>

      {/* ---------------- PERFIS ---------------- */}
      {tab === 'roles' && !draft && (
        <>
          <Box sx={{ display: 'flex', justifyContent: 'flex-end', mb: 1.5 }}>
            <Button variant="contained" onClick={newDraft}>{t('access.new_role')}</Button>
          </Box>
          <Stack spacing={1}>
            {roles.map((r) => (
              <Paper
                key={r.id}
                variant="outlined"
                sx={{ display: 'flex', alignItems: 'center', gap: 1.5, p: 1.5 }}
              >
                <Box sx={{ flex: 1 }}>
                  <Stack direction="row" spacing={1} sx={{ alignItems: 'center' }}>
                    <Typography sx={{ fontWeight: 600 }}>{r.name}</Typography>
                    {r.isSystem && <Chip size="small" label={t('access.system_badge')} />}
                  </Stack>
                  <Typography variant="body2" color="text.secondary">
                    {r.description || '—'} · {t('access.users_count', { count: r.userCount })} · {r.rules.length} {t('access.rules_label')}
                  </Typography>
                </Box>
                <Button size="small" onClick={() => editDraft(r)}>{t('access.edit')}</Button>
                {!r.isSystem && (
                  <Button size="small" color="error" onClick={() => void removeRole(r)}>
                    {t('common.delete')}
                  </Button>
                )}
              </Paper>
            ))}
          </Stack>
        </>
      )}

      {/* ---------------- EDITOR DE PERFIL ---------------- */}
      {tab === 'roles' && draft && (
        <Paper variant="outlined" sx={{ p: 2.5 }}>
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1.5} sx={{ mb: 2 }}>
            <TextField
              label={t('access.role_name')}
              value={draft.name}
              onChange={(e) => setDraft({ ...draft, name: e.target.value })}
              size="small"
              sx={{ flex: 1 }}
            />
            <TextField
              label={t('access.role_desc')}
              value={draft.description}
              onChange={(e) => setDraft({ ...draft, description: e.target.value })}
              size="small"
              sx={{ flex: 2 }}
            />
          </Stack>

          <Typography variant="subtitle1" sx={{ fontWeight: 600, mb: 1 }}>{t('access.permissions')}</Typography>
          <Box sx={{ overflowX: 'auto' }}>
            <Table size="small">
              <TableHead>
                <TableRow>
                  <TableCell>{t('access.resource')}</TableCell>
                  {actions.map((a) => <TableCell key={a} align="center">{t(`access.action.${a}`)}</TableCell>)}
                </TableRow>
              </TableHead>
              <TableBody>
                {Object.entries(groups).map(([mod, subs]) => (
                  <SubjectGroup key={mod} mod={mod} subs={subs} actions={actions} matrix={draft.matrix} onToggle={toggleCell} t={t} />
                ))}
              </TableBody>
            </Table>
          </Box>

          <Box sx={{ display: 'flex', gap: 1, justifyContent: 'flex-end', mt: 2 }}>
            <Button onClick={() => setDraft(null)}>{t('common.cancel')}</Button>
            <Button variant="contained" onClick={() => void save()} disabled={saving || !draft.name.trim()}>
              {saving ? '…' : t('common.save')}
            </Button>
          </Box>
        </Paper>
      )}

      {/* ---------------- USUÁRIOS ---------------- */}
      {tab === 'users' && (
        <Stack spacing={1}>
          {users.map((u) => (
            <Paper
              key={u.id}
              variant="outlined"
              sx={{ display: 'flex', alignItems: 'center', gap: 1.5, p: 1.5 }}
            >
              <Box sx={{ flex: 1 }}>
                <Typography sx={{ fontWeight: 600 }}>{u.name}</Typography>
                <Typography variant="body2" color="text.secondary">{u.email}</Typography>
              </Box>
              <Box sx={{ display: 'flex', gap: 1.5, flexWrap: 'wrap' }}>
                {roles.map((r) => {
                  const on = u.accessRoles.some((ur) => ur.id === r.id)
                  return (
                    <FormControlLabel
                      key={r.id}
                      control={
                        <Checkbox
                          size="small"
                          checked={on}
                          onChange={(e) => void toggleUserRole(u, r.id, e.target.checked)}
                        />
                      }
                      label={<Typography variant="body2">{r.name}</Typography>}
                    />
                  )
                })}
              </Box>
            </Paper>
          ))}
        </Stack>
      )}
    </Layout>
  )
}

function SubjectGroup({
  mod, subs, actions, matrix, onToggle, t,
}: {
  mod: string
  subs: string[]
  actions: string[]
  matrix: Matrix
  onToggle: (s: string, a: string) => void
  t: (k: string, v?: Record<string, string | number>) => string
}) {
  const catKey = `modules.cat.${moduleCategory(mod)}`
  const groupLabel = t(catKey) !== catKey ? t(catKey) : mod
  return (
    <>
      <TableRow>
        <TableCell colSpan={actions.length + 1} sx={{ bgcolor: 'action.hover' }}>
          <Typography variant="overline" color="text.secondary">{groupLabel}</Typography>
        </TableCell>
      </TableRow>
      {subs.map((s) => (
        <TableRow key={s}>
          <TableCell>{t(`access.subject.${s}`)}</TableCell>
          {actions.map((a) => (
            <TableCell key={a} align="center">
              <Checkbox size="small" checked={matrix[s]?.[a] ?? false} onChange={() => onToggle(s, a)} />
            </TableCell>
          ))}
        </TableRow>
      ))}
    </>
  )
}

// mapeia slug de módulo -> categoria i18n (pra rótulo do grupo); fallback ao slug
function moduleCategory(mod: string): string {
  const map: Record<string, string> = {
    'customer-supplier': 'REGISTRY', product: 'REGISTRY',
    stock: 'INVENTORY',
    order: 'SALES', cash: 'SALES', wallet: 'SALES',
    'service-order': 'SERVICE', budget: 'SERVICE',
    student: 'SCHOOL', 'enrollment-plan': 'SCHOOL', enrollment: 'SCHOOL', tuition: 'SCHOOL',
    'access-control': 'SYSTEM', theme: 'SYSTEM',
  }
  return map[mod] ?? mod
}
