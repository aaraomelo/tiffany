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
  category: 'CORE' | 'REGISTRY' | 'INVENTORY' | 'SALES' | 'SERVICE' | 'FOOD' | 'EVENT' | 'SCHOOL' | 'EDUCATION' | 'HEALTH' | 'SYSTEM' | 'FISCAL' | 'AI'
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

// ---------- Cosmos / governança (observabilidade por geometria) ----------
export interface CosmosData {
  metrics: {
    gini: number
    totalRoles: number
    totalUsers: number
    horizonRoles: number
    orbits: number
    avgUserMass: number
    maxRoleMass: number
  }
  roles: Array<{
    id: string
    name: string
    isSystem: boolean
    mass: number
    userCount: number
    horizon: boolean
    depth: number | null
  }>
  hubs: Array<{ name: string; mass: number; userCount: number; score: number }>
  lorenz: Array<{ x: number; y: number }>
}

export function fetchCosmos() {
  return api<CosmosData>('/api/access/cosmos')
}

export async function toggleModule(slug: string, enabled: boolean): Promise<TenantModule> {
  return api<TenantModule>(`/api/tenant/modules/${encodeURIComponent(slug)}`, {
    method: 'PATCH',
    body: JSON.stringify({ enabled }),
  })
}

// ---------- Landing page pública do cliente ----------
export interface LandingConfig {
  headline?: string
  subheadline?: string
  about?: string
  logoUrl?: string
  heroImageUrl?: string
  ctaText?: string
  ctaUrl?: string
  services?: { title: string; description?: string }[]
  gallery?: string[]
  hours?: { label: string; value?: string }[]
  contact?: { phone?: string; whatsapp?: string; email?: string; address?: string }
  social?: { instagram?: string; facebook?: string; website?: string }
}

export interface PublicSite {
  alias: string
  name: string
  companyName: string | null
  theme: Record<string, unknown> | null
  landing: LandingConfig | null
}

export interface LandingResponse {
  alias: string
  name: string
  companyName: string | null
  landing: LandingConfig | null
}

// Alias do CLIENTE (tenant). null = domínio raiz / tenant default (Patria).
// Produção: nginx injeta X-Tenant pelo subdomínio. Aqui derivamos pro front
// decidir entre a LP institucional da Patria e a LP do cliente.
const RESERVED_SUBDOMAINS = new Set(['www', 'dev', 'homolog', 'app', 'patriatechnology'])

export function getTenantAlias(): string | null {
  if (typeof window === 'undefined') return null
  const fromQuery = new URLSearchParams(window.location.search).get('tenant')
  if (fromQuery) return fromQuery.toLowerCase()
  const host = window.location.hostname
  if (host === 'localhost' || /^\d+\.\d+\.\d+\.\d+$/.test(host)) return null // dev → default
  const parts = host.split('.')
  // {alias}.dominio.tld → primeiro rótulo é o tenant (se não for reservado)
  if (parts.length >= 3 && !RESERVED_SUBDOMAINS.has(parts[0])) return parts[0]
  return null // apex (dominio.tld) ou www → tenant default (Patria)
}

/** true quando estamos no domínio raiz / tenant default (Patria). */
export function isDefaultTenant(): boolean {
  return getTenantAlias() === null
}

export async function fetchPublicSite(): Promise<PublicSite> {
  const alias = getTenantAlias()
  const qs = alias ? `?tenant=${encodeURIComponent(alias)}` : ''
  return api<PublicSite>(`/api/public/site${qs}`)
}

export function fetchLanding() {
  return api<LandingResponse>('/api/tenant/landing')
}

export function updateLanding(landing: LandingConfig) {
  return api<LandingResponse>('/api/tenant/landing', {
    method: 'PUT',
    body: JSON.stringify(landing),
  })
}

// ---------- Cadastro self-service (público) ----------
export interface Segment {
  slug: string
  name: string
  segment: string
  description: string | null
  isDefault: boolean
}

export interface SignupPayload {
  alias: string
  name: string
  packSlugs: string[]
  adminEmail: string
  adminName: string
  adminPassword: string
}

export function fetchSegments() {
  return api<Segment[]>('/api/public/segments')
}

export function checkAlias(alias: string) {
  return api<{ available: boolean; reason: string | null }>(
    `/api/public/alias-available?alias=${encodeURIComponent(alias)}`,
  )
}

export function signup(payload: SignupPayload) {
  return api<{ alias: string; name: string; packSlugs: string[] }>('/api/public/signup', {
    method: 'POST',
    body: JSON.stringify(payload),
  })
}

// ---------- Recuperação de senha (público) ----------
export function forgotPassword(email: string) {
  const alias = getTenantAlias()
  return api<{ ok: boolean }>('/api/auth/forgot-password', {
    method: 'POST',
    headers: alias ? { 'X-Tenant': alias } : {},
    body: JSON.stringify({ email }),
  })
}

export function resetPassword(token: string, password: string) {
  return api<{ ok: boolean }>('/api/auth/reset-password', {
    method: 'POST',
    body: JSON.stringify({ token, password }),
  })
}

// ---------- Saúde (ficha/anamnese das pessoas) ----------
export interface HealthPerson {
  id: string
  name: string
  phone: string | null
  birthDate: string | null
  hasRecord: boolean
  bloodType: string | null
  medicalClearance: boolean
  hasAlerts: boolean
}

export interface HealthRecord {
  id?: string
  bloodType?: string | null
  allergies?: string | null
  chronicConditions?: string | null
  medications?: string | null
  previousInjuries?: string | null
  healthInsurance?: string | null
  healthInsuranceNumber?: string | null
  emergencyContactName?: string | null
  emergencyContactPhone?: string | null
  medicalClearance?: boolean
  medicalClearanceDate?: string | null
  observations?: string | null
}

export function fetchHealthPeople(q?: string) {
  const qs = q ? `?q=${encodeURIComponent(q)}` : ''
  return api<HealthPerson[]>(`/api/health/people${qs}`)
}

export function fetchHealthRecord(id: string) {
  return api<{ student: { id: string; name: string; birthDate: string | null; phone: string | null }; record: HealthRecord | null }>(`/api/health/people/${id}`)
}

export function saveHealthRecord(id: string, record: HealthRecord) {
  return api<HealthRecord>(`/api/health/people/${id}`, {
    method: 'PUT',
    body: JSON.stringify(record),
  })
}

// ---------- Alunos ----------
export type StudentStatus = 'ACTIVE' | 'INACTIVE' | 'SUSPENDED'
export type FootPreference = 'LEFT' | 'RIGHT' | 'BOTH'

export interface Student {
  id: string
  name: string
  birthDate?: string | null
  document?: string | null
  gender?: string | null
  email?: string | null
  phone?: string | null
  whatsapp?: string | null
  zipCode?: string | null
  street?: string | null
  number?: string | null
  neighborhood?: string | null
  city?: string | null
  state?: string | null
  fatherName?: string | null
  fatherDocument?: string | null
  fatherPhone?: string | null
  motherName?: string | null
  motherDocument?: string | null
  motherPhone?: string | null
  guardianName?: string | null
  guardianDocument?: string | null
  guardianPhone?: string | null
  guardianRelation?: string | null
  position?: string | null
  preferredFoot?: FootPreference | null
  jerseyNumber?: number | null
  status: StudentStatus
  notes?: string | null
  photoUpdatedAt?: string | null
  createdAt: string
}

export interface StudentDetail extends Student {
  healthRecord?: HealthRecord | null
}

export function fetchStudent(id: string) {
  return api<StudentDetail>(`/api/students/${id}`)
}

export function updateStudent(id: string, patch: Partial<Student>) {
  return api<Student>(`/api/students/${id}`, {
    method: 'PATCH',
    body: JSON.stringify(patch),
  })
}

/** Busca a foto do aluno autenticada (Bearer) e devolve um objectURL p/ <img>, ou null.
 *  `version` (photoUpdatedAt) entra como query pra quebrar o cache quando a foto muda. */
export async function fetchStudentPhotoUrl(id: string, version?: string | null): Promise<string | null> {
  const token = getToken()
  const qs = version ? `?v=${encodeURIComponent(version)}` : ''
  const res = await fetch(`/api/students/${id}/photo${qs}`, {
    headers: token ? { Authorization: `Bearer ${token}` } : {},
  })
  if (!res.ok) return null
  const blob = await res.blob()
  return URL.createObjectURL(blob)
}

export function uploadStudentPhoto(id: string, dataBase64: string, mimeType: string) {
  return api<{ ok: boolean; photoUpdatedAt: string }>(`/api/students/${id}/photo`, {
    method: 'PUT',
    body: JSON.stringify({ dataBase64, mimeType }),
  })
}

export function deleteStudentPhoto(id: string) {
  return api<{ ok: boolean }>(`/api/students/${id}/photo`, { method: 'DELETE' })
}

export function saveStudentHealthRecord(id: string, record: HealthRecord) {
  return api<HealthRecord>(`/api/students/${id}/health-record`, {
    method: 'PUT',
    body: JSON.stringify(record),
  })
}

// ---------- Onboarding (primeiros passos) ----------
export interface OnboardingStep {
  key: string
  done: boolean
  route: string
}
export interface OnboardingStatus {
  steps: OnboardingStep[]
  total: number
  done: number
  allDone: boolean
}

export function fetchOnboarding() {
  return api<OnboardingStatus>('/api/onboarding/status')
}

// ---------- Dashboard (métricas por módulo ativo) ----------
export interface DashboardMetric {
  key: string
  module: string
  value: number
  format: 'count' | 'currency'
  route: string | null
  intent: 'default' | 'success' | 'warning' | 'error'
}

export function fetchDashboard() {
  return api<{ metrics: DashboardMetric[] }>('/api/dashboard/metrics')
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
