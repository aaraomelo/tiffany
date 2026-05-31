import { useEffect, useMemo, useState } from 'react'
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
        <h1 style={{ marginTop: 0 }}>{t('access.title')}</h1>
        <p style={{ color: 'var(--text-muted)' }}>{t('access.no_permission')}</p>
      </Layout>
    )
  }

  const actions = catalog?.actions ?? ['manage', 'create', 'read', 'update', 'delete']

  return (
    <Layout>
      <h1 style={{ marginTop: 0 }}>{t('access.title')}</h1>
      <p style={{ color: 'var(--text-muted)', marginTop: 0 }}>{t('access.subtitle')}</p>

      <div style={{ display: 'flex', gap: '0.5rem', marginBottom: '1rem' }}>
        <button onClick={() => setTab('roles')} style={tabBtn(tab === 'roles')}>{t('access.tab.roles')}</button>
        <button onClick={() => setTab('users')} style={tabBtn(tab === 'users')}>{t('access.tab.users')}</button>
      </div>

      {/* ---------------- PERFIS ---------------- */}
      {tab === 'roles' && !draft && (
        <>
          <div style={{ display: 'flex', justifyContent: 'flex-end', marginBottom: '0.8rem' }}>
            <button onClick={newDraft} style={btnPrimary}>{t('access.new_role')}</button>
          </div>
          <div style={{ display: 'grid', gap: '0.6rem' }}>
            {roles.map((r) => (
              <div key={r.id} style={roleCard}>
                <div style={{ flex: 1 }}>
                  <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <strong>{r.name}</strong>
                    {r.isSystem && <span style={sysBadge}>{t('access.system_badge')}</span>}
                  </div>
                  <div style={{ fontSize: 13, color: 'var(--text-muted)' }}>
                    {r.description || '—'} · {t('access.users_count', { count: r.userCount })} · {r.rules.length} {t('access.rules_label')}
                  </div>
                </div>
                <button onClick={() => editDraft(r)} style={btnGhost}>{t('access.edit')}</button>
                {!r.isSystem && (
                  <button onClick={() => void removeRole(r)} style={{ ...btnGhost, color: 'var(--danger)', borderColor: 'var(--danger)' }}>
                    {t('common.delete')}
                  </button>
                )}
              </div>
            ))}
          </div>
        </>
      )}

      {/* ---------------- EDITOR DE PERFIL ---------------- */}
      {tab === 'roles' && draft && (
        <section style={card}>
          <div style={{ display: 'grid', gridTemplateColumns: '1fr 2fr', gap: '0.6rem', marginBottom: '1rem' }}>
            <label style={lbl}>{t('access.role_name')}
              <input value={draft.name} onChange={(e) => setDraft({ ...draft, name: e.target.value })} style={input} />
            </label>
            <label style={lbl}>{t('access.role_desc')}
              <input value={draft.description} onChange={(e) => setDraft({ ...draft, description: e.target.value })} style={input} />
            </label>
          </div>

          <h3 style={{ margin: '0 0 0.5rem' }}>{t('access.permissions')}</h3>
          <div style={{ overflowX: 'auto' }}>
            <table style={{ borderCollapse: 'collapse', width: '100%', fontSize: 13 }}>
              <thead>
                <tr>
                  <th style={{ ...matTd, textAlign: 'left' }}>{t('access.resource')}</th>
                  {actions.map((a) => <th key={a} style={matTd}>{t(`access.action.${a}`)}</th>)}
                </tr>
              </thead>
              <tbody>
                {Object.entries(groups).map(([mod, subs]) => (
                  <SubjectGroup key={mod} mod={mod} subs={subs} actions={actions} matrix={draft.matrix} onToggle={toggleCell} t={t} />
                ))}
              </tbody>
            </table>
          </div>

          <div style={{ display: 'flex', gap: '0.5rem', justifyContent: 'flex-end', marginTop: '1rem' }}>
            <button onClick={() => setDraft(null)} style={btnGhost}>{t('common.cancel')}</button>
            <button onClick={() => void save()} disabled={saving || !draft.name.trim()} style={btnPrimary}>
              {saving ? '…' : t('common.save')}
            </button>
          </div>
        </section>
      )}

      {/* ---------------- USUÁRIOS ---------------- */}
      {tab === 'users' && (
        <div style={{ display: 'grid', gap: '0.6rem' }}>
          {users.map((u) => (
            <div key={u.id} style={roleCard}>
              <div style={{ flex: 1 }}>
                <strong>{u.name}</strong>
                <div style={{ fontSize: 13, color: 'var(--text-muted)' }}>{u.email}</div>
              </div>
              <div style={{ display: 'flex', gap: '0.8rem', flexWrap: 'wrap' }}>
                {roles.map((r) => {
                  const on = u.accessRoles.some((ur) => ur.id === r.id)
                  return (
                    <label key={r.id} style={{ display: 'flex', alignItems: 'center', gap: 4, fontSize: 13 }}>
                      <input type="checkbox" checked={on} onChange={(e) => void toggleUserRole(u, r.id, e.target.checked)} />
                      {r.name}
                    </label>
                  )
                })}
              </div>
            </div>
          ))}
        </div>
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
  return (
    <>
      <tr>
        <td colSpan={actions.length + 1} style={groupTd}>{t(`modules.cat.${moduleCategory(mod)}`) !== `modules.cat.${moduleCategory(mod)}` ? t(`modules.cat.${moduleCategory(mod)}`) : mod}</td>
      </tr>
      {subs.map((s) => (
        <tr key={s}>
          <td style={{ ...matTd, textAlign: 'left' }}>{t(`access.subject.${s}`)}</td>
          {actions.map((a) => (
            <td key={a} style={matTd}>
              <input type="checkbox" checked={matrix[s]?.[a] ?? false} onChange={() => onToggle(s, a)} />
            </td>
          ))}
        </tr>
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

const tabBtn = (active: boolean): React.CSSProperties => ({
  padding: '0.5rem 1rem', fontSize: 14, borderRadius: 6, cursor: 'pointer',
  background: active ? 'var(--primary)' : 'var(--surface-alt)',
  color: active ? 'var(--text-on-primary)' : 'var(--text)',
  border: `1px solid ${active ? 'var(--primary)' : 'var(--border)'}`, fontWeight: active ? 600 : 400,
})
const card: React.CSSProperties = { background: 'var(--surface)', border: '1px solid var(--border)', padding: '1.2rem', borderRadius: 8 }
const roleCard: React.CSSProperties = { display: 'flex', alignItems: 'center', gap: '0.8rem', background: 'var(--surface)', border: '1px solid var(--border)', borderRadius: 8, padding: '0.8rem 1rem' }
const sysBadge: React.CSSProperties = { background: 'var(--surface-alt)', border: '1px solid var(--border)', borderRadius: 4, padding: '0.1rem 0.4rem', fontSize: 11, color: 'var(--text-muted)' }
const lbl: React.CSSProperties = { display: 'grid', gap: 4, fontSize: 13 }
const input: React.CSSProperties = { padding: '0.55rem 0.7rem', fontSize: 14, borderRadius: 6 }
const btnPrimary: React.CSSProperties = { padding: '0.55rem 1rem', fontSize: 14, background: 'var(--primary)', color: 'var(--text-on-primary)', border: 'none', borderRadius: 6, cursor: 'pointer', fontWeight: 600 }
const btnGhost: React.CSSProperties = { padding: '0.45rem 0.9rem', fontSize: 13, background: 'transparent', color: 'var(--primary)', border: '1px solid var(--border)', borderRadius: 6, cursor: 'pointer' }
const matTd: React.CSSProperties = { padding: '0.4rem 0.6rem', textAlign: 'center', borderBottom: '1px solid var(--border)' }
const groupTd: React.CSSProperties = { padding: '0.5rem 0.6rem', fontSize: 11, fontWeight: 700, textTransform: 'uppercase', letterSpacing: 0.5, color: 'var(--text-muted)', background: 'var(--surface-alt)' }
