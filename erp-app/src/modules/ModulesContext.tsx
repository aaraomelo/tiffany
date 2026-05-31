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
import { fetchTenantModules, type TenantModule } from '../api'

interface ModulesCtx {
  loading: boolean
  packSlug: string | null
  modules: TenantModule[]
  isEnabled: (slug: string) => boolean
  refresh: () => Promise<void>
}

const ModulesContext = createContext<ModulesCtx>({
  loading: true,
  packSlug: null,
  modules: [],
  isEnabled: () => false,
  refresh: async () => {},
})

export function ModulesProvider({ children }: { children: ReactNode }) {
  const [loading, setLoading] = useState(true)
  const [packSlug, setPackSlug] = useState<string | null>(null)
  const [modules, setModules] = useState<TenantModule[]>([])

  const load = useCallback(async () => {
    setLoading(true)
    try {
      const data = await fetchTenantModules()
      setPackSlug(data.packSlug)
      setModules(data.modules)
    } catch {
      // tenant sem módulos ainda — fica vazio; rotas opcionais ficam bloqueadas
      setPackSlug(null)
      setModules([])
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => {
    load()
  }, [load])

  const value = useMemo<ModulesCtx>(() => {
    const enabledSet = new Set(modules.filter((m) => m.enabled).map((m) => m.slug))
    return {
      loading,
      packSlug,
      modules,
      isEnabled: (slug: string) => enabledSet.has(slug),
      refresh: load,
    }
  }, [loading, packSlug, modules, load])

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
