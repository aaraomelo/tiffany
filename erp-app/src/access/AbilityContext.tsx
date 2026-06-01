import { createMongoAbility, type MongoAbility } from '@casl/ability'
import { unpackRules } from '@casl/ability/extra'
import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useState,
  type ReactNode,
} from 'react'
import { api } from '../api'

export type Action = 'manage' | 'create' | 'read' | 'update' | 'delete'
export type Subject =
  | 'all' | 'Customer' | 'Product' | 'Stock' | 'Order' | 'Cash' | 'Wallet'
  | 'ServiceOrder' | 'Budget' | 'Student' | 'EnrollmentPlan' | 'Enrollment'
  | 'Tuition' | 'Role' | 'User' | 'Module' | 'Theme'

export type AppAbility = MongoAbility<[Action, Subject]>

// slug do módulo -> recurso (subject) protegido, para gating de menu/rotas
export const SUBJECT_BY_MODULE: Record<string, Subject> = {
  'customer-supplier': 'Customer',
  product: 'Product',
  stock: 'Stock',
  order: 'Order',
  pos: 'Order',
  cash: 'Cash',
  wallet: 'Wallet',
  'service-order': 'ServiceOrder',
  budget: 'Budget',
  student: 'Student',
  'enrollment-plan': 'EnrollmentPlan',
  enrollment: 'Enrollment',
  tuition: 'Tuition',
  'health-record': 'Student',
}

interface MeResponse {
  roles: { id: string; name: string }[]
  rules: unknown[] // packed rules
}

interface AbilityCtx {
  ability: AppAbility
  roles: { id: string; name: string }[]
  loading: boolean
  refresh: () => Promise<void>
}

const emptyAbility = createMongoAbility<AppAbility>([])

const AbilityContext = createContext<AbilityCtx>({
  ability: emptyAbility,
  roles: [],
  loading: true,
  refresh: async () => {},
})

export function AbilityProvider({ children }: { children: ReactNode }) {
  const [ability, setAbility] = useState<AppAbility>(emptyAbility)
  const [roles, setRoles] = useState<{ id: string; name: string }[]>([])
  const [loading, setLoading] = useState(true)

  const load = useCallback(async () => {
    setLoading(true)
    try {
      const me = await api<MeResponse>('/api/access/me')
      const rules = unpackRules(me.rules as never)
      setAbility(createMongoAbility<AppAbility>(rules as never))
      setRoles(me.roles)
    } catch {
      setAbility(emptyAbility)
      setRoles([])
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => { void load() }, [load])

  const value = useMemo<AbilityCtx>(
    () => ({ ability, roles, loading, refresh: load }),
    [ability, roles, loading, load],
  )

  return <AbilityContext.Provider value={value}>{children}</AbilityContext.Provider>
}

export function useAbility() {
  return useContext(AbilityContext)
}

/** Renderiza os filhos só se o usuário puder `I` sobre `a`. */
export function Can({
  I,
  a,
  not = false,
  children,
  fallback = null,
}: {
  I: Action
  a: Subject
  not?: boolean
  children: ReactNode
  fallback?: ReactNode
}) {
  const { ability } = useAbility()
  const allowed = ability.can(I, a)
  const show = not ? !allowed : allowed
  return <>{show ? children : fallback}</>
}
