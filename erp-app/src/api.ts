const TOKEN_KEY = 'erp.token'
const USER_KEY = 'erp.user'

export type StoredUser = {
  id: string
  email: string
  name: string
  role: string
  tenantId: string
  tenantAlias: string
  tenantName: string
}

export function getToken(): string | null {
  return localStorage.getItem(TOKEN_KEY)
}

export function setSession(token: string, user: StoredUser) {
  localStorage.setItem(TOKEN_KEY, token)
  localStorage.setItem(USER_KEY, JSON.stringify(user))
}

export function getUser(): StoredUser | null {
  const raw = localStorage.getItem(USER_KEY)
  return raw ? (JSON.parse(raw) as StoredUser) : null
}

export function clearSession() {
  localStorage.removeItem(TOKEN_KEY)
  localStorage.removeItem(USER_KEY)
}

export class ApiError extends Error {
  constructor(public status: number, public body: unknown) {
    super(`HTTP ${status}`)
  }
}

export type TenantModule = {
  slug: string
  enabled: boolean
  enabledAt: string
  disabledAt: string | null
  name: string
  description: string | null
  category: 'CORE' | 'REGISTRY' | 'INVENTORY' | 'SALES' | 'SERVICE' | 'FOOD' | 'EVENT' | 'SCHOOL' | 'SYSTEM' | 'FISCAL' | 'AI'
  routePath: string | null
  iconKey: string | null
  isCore: boolean
  sortOrder: number
}

export type TenantModulesResponse = {
  packSlug: string | null
  activePacks: string[]
  modules: TenantModule[]
}

export type ModulePack = {
  slug: string
  name: string
  segment: string
  description: string | null
  isDefault: boolean
  sortOrder: number
  items: { moduleSlug: string }[]
}

export async function fetchTenantModules(): Promise<TenantModulesResponse> {
  return api<TenantModulesResponse>('/api/tenant/modules')
}

export async function fetchPacks(): Promise<ModulePack[]> {
  return api<ModulePack[]>('/api/module-packs')
}

export async function applyPack(
  packSlug: string,
  mode: 'replace' | 'merge' = 'replace',
): Promise<TenantModulesResponse> {
  return api<TenantModulesResponse>('/api/tenant/modules/apply-pack', {
    method: 'POST',
    body: JSON.stringify({ packSlug, mode }),
  })
}

export async function togglePack(
  packSlug: string,
  enabled: boolean,
): Promise<TenantModulesResponse> {
  return api<TenantModulesResponse>('/api/tenant/modules/toggle-pack', {
    method: 'POST',
    body: JSON.stringify({ packSlug, enabled }),
  })
}

// ---------- Controle de acesso (RBAC) ----------
export interface AccessRule {
  id?: string
  action: string | string[]
  subject: string | string[]
  fields?: string | string[] | null
  conditions?: Record<string, unknown> | null
  inverted?: boolean
  reason?: string | null
}

export interface AccessRoleItem {
  id: string
  name: string
  description: string | null
  isSystem: boolean
  userCount: number
  rules: AccessRule[]
}

export interface AccessSubjectMeta {
  key: string
  module: string
}

export interface AccessCatalog {
  actions: string[]
  subjects: AccessSubjectMeta[]
}

export interface AccessUser {
  id: string
  name: string
  email: string
  role: string
  active: boolean
  accessRoles: { id: string; name: string }[]
}

export function fetchAccessCatalog() {
  return api<AccessCatalog>('/api/access/catalog')
}

export function fetchRoles() {
  return api<AccessRoleItem[]>('/api/access/roles')
}

export function createRole(body: { name: string; description?: string; rules: AccessRule[] }) {
  return api<AccessRoleItem>('/api/access/roles', { method: 'POST', body: JSON.stringify(body) })
}

export function updateRole(id: string, body: { name?: string; description?: string; rules?: AccessRule[] }) {
  return api<AccessRoleItem>(`/api/access/roles/${id}`, { method: 'PATCH', body: JSON.stringify(body) })
}

export function deleteRole(id: string) {
  return api<{ ok: boolean }>(`/api/access/roles/${id}`, { method: 'DELETE' })
}

export function fetchAccessUsers() {
  return api<AccessUser[]>('/api/access/users')
}

export function setUserRoles(userId: string, roleIds: string[]) {
  return api<AccessUser[]>(`/api/access/users/${userId}/roles`, {
    method: 'PUT',
    body: JSON.stringify({ roleIds }),
  })
}

export async function toggleModule(slug: string, enabled: boolean): Promise<TenantModule> {
  return api<TenantModule>(`/api/tenant/modules/${encodeURIComponent(slug)}`, {
    method: 'PATCH',
    body: JSON.stringify({ enabled }),
  })
}

export async function api<T>(
  path: string,
  init: RequestInit = {},
): Promise<T> {
  const token = getToken()
  const headers: Record<string, string> = {
    'Content-Type': 'application/json',
    ...(init.headers as Record<string, string> | undefined),
  }
  if (token) headers.Authorization = `Bearer ${token}`

  const res = await fetch(path, { ...init, headers })
  if (!res.ok) {
    let body: unknown = null
    try {
      body = await res.json()
    } catch {
      body = await res.text()
    }
    // Sessão expirada ou tenant inválido → limpa e volta pro login
    if (res.status === 401 && !path.includes('/auth/login')) {
      clearSession()
      if (typeof window !== 'undefined' && !window.location.pathname.startsWith('/login')) {
        window.location.href = '/login'
      }
    }
    throw new ApiError(res.status, body)
  }
  if (res.status === 204) return undefined as T
  const text = await res.text()
  if (!text) return null as T
  try {
    return JSON.parse(text) as T
  } catch {
    return text as unknown as T
  }
}
