import { Box, CircularProgress } from '@mui/material'
import { lazy, Suspense } from 'react'
import { BrowserRouter, Navigate, Route, Routes } from 'react-router-dom'
import { AbilityProvider } from './access/AbilityContext'
import { getToken } from './api'
import { isDefaultTenant } from './api'
import { ModuleRoute, ModulesProvider, RequirePack } from './modules/ModulesContext'
import { LandingPage } from './pages/LandingPage'
import { LoginPage } from './pages/LoginPage'
import { PatriaLandingPage } from './pages/PatriaLandingPage'

// Páginas carregadas sob demanda (code-splitting por rota).
const AccessControlPage = lazy(() => import('./pages/AccessControlPage').then((m) => ({ default: m.AccessControlPage })))
const AssistantPage = lazy(() => import('./pages/AssistantPage').then((m) => ({ default: m.AssistantPage })))
const BudgetsPage = lazy(() => import('./pages/BudgetsPage').then((m) => ({ default: m.BudgetsPage })))
const CashPage = lazy(() => import('./pages/CashPage').then((m) => ({ default: m.CashPage })))
const CustomersPage = lazy(() => import('./pages/CustomersPage').then((m) => ({ default: m.CustomersPage })))
const EnrollmentPlansPage = lazy(() => import('./pages/EnrollmentPlansPage').then((m) => ({ default: m.EnrollmentPlansPage })))
const EnrollmentsPage = lazy(() => import('./pages/EnrollmentsPage').then((m) => ({ default: m.EnrollmentsPage })))
const HealthPage = lazy(() => import('./pages/HealthPage').then((m) => ({ default: m.HealthPage })))
const ModulesPage = lazy(() => import('./pages/ModulesPage').then((m) => ({ default: m.ModulesPage })))
const OnboardingPackPage = lazy(() => import('./pages/OnboardingPackPage').then((m) => ({ default: m.OnboardingPackPage })))
const OrdersPage = lazy(() => import('./pages/OrdersPage').then((m) => ({ default: m.OrdersPage })))
const PosPage = lazy(() => import('./pages/PosPage').then((m) => ({ default: m.PosPage })))
const ProductsPage = lazy(() => import('./pages/ProductsPage').then((m) => ({ default: m.ProductsPage })))
const ServiceOrderDetailPage = lazy(() => import('./pages/ServiceOrderDetailPage').then((m) => ({ default: m.ServiceOrderDetailPage })))
const ServiceOrdersPage = lazy(() => import('./pages/ServiceOrdersPage').then((m) => ({ default: m.ServiceOrdersPage })))
const SettingsPage = lazy(() => import('./pages/SettingsPage').then((m) => ({ default: m.SettingsPage })))
const SignupPage = lazy(() => import('./pages/SignupPage').then((m) => ({ default: m.SignupPage })))
const SiteEditorPage = lazy(() => import('./pages/SiteEditorPage').then((m) => ({ default: m.SiteEditorPage })))
const StockPage = lazy(() => import('./pages/StockPage').then((m) => ({ default: m.StockPage })))
const StudentsPage = lazy(() => import('./pages/StudentsPage').then((m) => ({ default: m.StudentsPage })))
const ThemePage = lazy(() => import('./pages/ThemePage').then((m) => ({ default: m.ThemePage })))
const TuitionsPage = lazy(() => import('./pages/TuitionsPage').then((m) => ({ default: m.TuitionsPage })))
const WalletPage = lazy(() => import('./pages/WalletPage').then((m) => ({ default: m.WalletPage })))

function RequireAuth({ children }: { children: React.ReactNode }) {
  if (!getToken()) return <Navigate to="/login" replace />
  return (
    <ModulesProvider>
      <AbilityProvider>{children}</AbilityProvider>
    </ModulesProvider>
  )
}

function Authed({ children }: { children: React.ReactNode }) {
  return <RequireAuth><RequirePack>{children}</RequirePack></RequireAuth>
}

function PageFallback() {
  return (
    <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '60vh' }}>
      <CircularProgress />
    </Box>
  )
}

export default function App() {
  return (
    <BrowserRouter>
      <Suspense fallback={<PageFallback />}>
        <Routes>
          {/* Raiz: LP institucional da Patria (domínio default) ou LP do cliente (subdomínio) */}
          <Route path="/" element={isDefaultTenant() ? <PatriaLandingPage /> : <LandingPage />} />
          <Route path="/signup" element={<SignupPage />} />
          <Route path="/login" element={<LoginPage />} />

          {/* Onboarding: autenticado, mas sem RequirePack pra evitar loop */}
          <Route path="/onboarding/pack" element={<RequireAuth><OnboardingPackPage /></RequireAuth>} />
          <Route path="/site" element={<Authed><SiteEditorPage /></Authed>} />

          {/* CORE — sempre disponíveis pra qualquer pack */}
          <Route path="/customers" element={<Authed><CustomersPage /></Authed>} />
          <Route path="/products" element={<Authed><ProductsPage /></Authed>} />
          <Route path="/stock" element={<Authed><StockPage /></Authed>} />
          <Route path="/theme" element={<Authed><ThemePage /></Authed>} />
          <Route path="/settings" element={<Authed><SettingsPage /></Authed>} />
          <Route path="/modules" element={<Authed><ModulesPage /></Authed>} />
          <Route path="/access" element={<Authed><AccessControlPage /></Authed>} />
          <Route path="/assistant" element={<Authed><AssistantPage /></Authed>} />

          {/* SALES */}
          <Route path="/pos" element={<Authed><ModuleRoute slug="pos"><PosPage /></ModuleRoute></Authed>} />
          <Route path="/orders" element={<Authed><ModuleRoute slug="order"><OrdersPage /></ModuleRoute></Authed>} />
          <Route path="/cash" element={<Authed><ModuleRoute slug="cash"><CashPage /></ModuleRoute></Authed>} />
          <Route path="/wallet" element={<Authed><ModuleRoute slug="wallet"><WalletPage /></ModuleRoute></Authed>} />

          {/* SERVICE */}
          <Route path="/service-orders" element={<Authed><ModuleRoute slug="service-order"><ServiceOrdersPage /></ModuleRoute></Authed>} />
          <Route path="/service-orders/:id" element={<Authed><ModuleRoute slug="service-order"><ServiceOrderDetailPage /></ModuleRoute></Authed>} />
          <Route path="/budgets" element={<Authed><ModuleRoute slug="budget"><BudgetsPage /></ModuleRoute></Authed>} />

          {/* SCHOOL (Escola de Futebol) */}
          <Route path="/students" element={<Authed><ModuleRoute slug="student"><StudentsPage /></ModuleRoute></Authed>} />
          <Route path="/enrollment-plans" element={<Authed><ModuleRoute slug="enrollment-plan"><EnrollmentPlansPage /></ModuleRoute></Authed>} />
          <Route path="/enrollments" element={<Authed><ModuleRoute slug="enrollment"><EnrollmentsPage /></ModuleRoute></Authed>} />
          <Route path="/tuitions" element={<Authed><ModuleRoute slug="tuition"><TuitionsPage /></ModuleRoute></Authed>} />

          {/* HEALTH (Saúde) */}
          <Route path="/health" element={<Authed><ModuleRoute slug="health-record"><HealthPage /></ModuleRoute></Authed>} />

          <Route path="*" element={<Navigate to="/pos" replace />} />
        </Routes>
      </Suspense>
    </BrowserRouter>
  )
}
