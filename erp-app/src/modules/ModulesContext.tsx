import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useState,
  type ReactNode,
} from 'react'
import { Navigate, useLocation } from 'react-router-dom'
import { fetchTenantModules, type TenantModule, type TenantModulesResponse } from '../api'

interface ModulesCtx {
  loading: boolean
  packSlug: string | null
  activePacks: string[]
  modules: TenantModule[]
  isEnabled: (slug: string) => boolean
  refresh: () => Promise<void>
  /** Substitui o estado local pelo response do servidor (sem refetch). */
  applyResponse: (data: TenantModulesResponse) => void
  /** Atualiza um módulo localmente (otimista, sem refetch). */
  patchModule: (slug: string, enabled: boolean) => void
}

const ModulesContext = createContext<ModulesCtx>({
  loading: true,
  packSlug: null,
  activePacks: [],
  modules: [],
  isEnabled: () => false,
  refresh: async () => {},
  applyResponse: () => {},
  patchModule: () => {},
})

export function ModulesProvider({ children }: { children: ReactNode }) {
  const [loading, setLoading] = useState(true)
  const [packSlug, setPackSlug] = useState<string | null>(null)
  const [activePacks, setActivePacks] = useState<string[]>([])
  const [modules, setModules] = useState<TenantModule[]>([])

  const load = useCallback(async () => {
    setLoading(true)
    try {
      const data = await fetchTenantModules()
      setPackSlug(data.packSlug)
      setActivePacks(data.activePacks ?? [])
      setModules(data.modules)
    } catch {
      // tenant sem módulos ainda — fica vazio; rotas opcionais ficam bloqueadas
      setPackSlug(null)
      setActivePacks([])
      setModules([])
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => {
    load()
  }, [load])

  const applyResponse = useCallback((data: TenantModulesResponse) => {
    setPackSlug(data.packSlug)
    setActivePacks(data.activePacks ?? [])
    setModules(data.modules)
  }, [])

  const patchModule = useCallback((slug: string, enabled: boolean) => {
    setModules((prev) =>
      prev.map((m) => (m.slug === slug ? { ...m, enabled } : m)),
    )
  }, [])

  const value = useMemo<ModulesCtx>(() => {
    const enabledSet = new Set(modules.filter((m) => m.enabled).map((m) => m.slug))
    return {
      loading,
      packSlug,
      activePacks,
      modules,
      isEnabled: (slug: string) => enabledSet.has(slug),
      refresh: load,
      applyResponse,
      patchModule,
    }
  }, [loading, packSlug, activePacks, modules, load, applyResponse, patchModule])

  return <ModulesContext.Provider value={value}>{children}</ModulesContext.Provider>
}

export function useModules() {
  return useContext(ModulesContext)
}

/**
 * Wrapper de rota: redireciona pro fallback se o módulo não estiver ativo.
 * Durante o load inicial não redireciona (evita flicker), retorna null.
 */
export function ModuleRoute({
  slug,
  fallback = '/',
  children,
}: {
  slug: string
  fallback?: string
  children: ReactNode
}) {
  const { loading, isEnabled } = useModules()
  if (loading) return null
  if (!isEnabled(slug)) return <Navigate to={fallback} replace />
  return <>{children}</>
}

/**
 * Se o tenant ainda não escolheu um pack, manda pro wizard.
 * Usado pelas rotas autenticadas que não são o próprio wizard.
 */
export function RequirePack({ children }: { children: ReactNode }) {
  const { loading, packSlug } = useModules()
  const location = useLocation()
  if (loading) return null
  if (!packSlug && location.pathname !== '/onboarding/pack') {
    return <Navigate to="/onboarding/pack" replace />
  }
  return <>{children}</>
}
