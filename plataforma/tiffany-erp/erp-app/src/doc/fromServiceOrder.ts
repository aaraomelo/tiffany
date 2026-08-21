import type { CompanyInfo, ServiceOrderData } from './serviceOrderData'
import { MOCK_COMPANY } from './serviceOrderData'

// Shape do detalhe da OS vindo de GET /api/service-orders/:id (subconjunto
// necessário pro PDF).
export interface SoDetailInput {
  number: number
  description: string
  diagnosis: string | null
  openedAt: string
  customer: { name: string; phone: string | null }
  vehicle: { plate: string | null; brand: string | null; model: string | null } | null
  warrantyTerm: { name: string; days: number } | null
  parts: Array<{
    quantity: string
    unitPrice: string
    total: string
    product: { sku: string; name: string; unit?: { code: string } }
  }>
  labors: Array<{ description: string; total: string }>
}

/** Mapeia a OS real (+ dados da empresa) no modelo do documento. */
export function serviceOrderToData(
  so: SoDetailInput,
  company?: Partial<CompanyInfo>,
): ServiceOrderData {
  const co: CompanyInfo = { ...MOCK_COMPANY, ...(company ?? {}) }
  const dateOnly = so.openedAt.slice(0, 10)
  const year = dateOnly.slice(0, 4)

  return {
    company: co,
    number: `${String(so.number).padStart(3, '0')}-${year}`,
    date: dateOnly,
    customer: { name: so.customer.name, phone: so.customer.phone ?? undefined },
    device: {
      brand: so.vehicle?.brand ?? undefined,
      model: so.vehicle?.model ?? undefined,
      kind: so.vehicle?.plate ?? undefined,
      problem: so.description,
    },
    services: so.labors.map((l) => ({ description: l.description, price: Number(l.total) })),
    parts: so.parts.map((p) => ({
      name: p.product.name,
      detail: p.product.sku,
      unit: p.product.unit?.code ?? 'un.',
      unitPrice: Number(p.unitPrice),
      qty: Number(p.quantity),
      price: Number(p.total),
    })),
    payment: { methods: co.paymentMethods, terms: co.paymentTerms },
    warranty: { period: so.warrantyTerm ? `${so.warrantyTerm.days} dias` : undefined },
    notes: so.diagnosis ?? undefined,
  }
}
