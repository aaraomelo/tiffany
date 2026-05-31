import { BrowserRouter, Navigate, Route, Routes } from 'react-router-dom'
import { AbilityProvider } from './access/AbilityContext'
import { getToken } from './api'
import { ModuleRoute, ModulesProvider, RequirePack } from './modules/ModulesContext'
import { AccessControlPage } from './pages/AccessControlPage'
import { AssistantPage } from './pages/AssistantPage'
import { BudgetsPage } from './pages/BudgetsPage'
import { CashPage } from './pages/CashPage'
import { CustomersPage } from './pages/CustomersPage'
import { EnrollmentPlansPage } from './pages/EnrollmentPlansPage'
import { EnrollmentsPage } from './pages/EnrollmentsPage'
import { LoginPage } from './pages/LoginPage'
import { ModulesPage } from './pages/ModulesPage'
import { OnboardingPackPage } from './pages/OnboardingPackPage'
import { OrdersPage } from './pages/OrdersPage'
import { PosPage } from './pages/PosPage'
import { ProductsPage } from './pages/ProductsPage'
import { ServiceOrderDetailPage } from './pages/ServiceOrderDetailPage'
import { ServiceOrdersPage } from './pages/ServiceOrdersPage'
import { SettingsPage } from './pages/SettingsPage'
import { StockPage } from './pages/StockPage'
import { StudentsPage } from './pages/StudentsPage'
import { ThemePage } from './pages/ThemePage'
import { TuitionsPage } from './pages/TuitionsPage'
import { WalletPage } from './pages/WalletPage'

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

export default function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/login" element={<LoginPage />} />

        {/* Onboarding: autenticado, mas sem RequirePack pra evitar loop */}
        <Route path="/onboarding/pack" element={<RequireAuth><OnboardingPackPage /></RequireAuth>} />

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

        <Route path="*" element={<Navigate to="/pos" replace />} />
      </Routes>
    </BrowserRouter>
  )
}
