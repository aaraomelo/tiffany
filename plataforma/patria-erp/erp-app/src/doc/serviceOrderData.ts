// Modelo de dados da Ordem de Serviço (agnóstico de backend). Alimentado pelo
// domínio ServiceOrder + dados da empresa (Tenant). Onde o cadastro ainda não
// tem o dado, cai no mock.

export interface CompanyInfo {
  name: string
  responsible?: string
  cnpj?: string
  address?: string
  cityLine?: string
  cep?: string
  email?: string
  phones?: string[]
  instagram?: string
  city?: string
  logo?: string // data URL ou URL (opcional)
  paymentMethods?: string
  paymentTerms?: string
}

export interface ServiceItem {
  description: string
  detail?: string
  price: number
}

export interface PartItem {
  name: string
  detail?: string
  unit: string
  unitPrice: number
  qty: number
  price: number
}

export interface ServiceOrderData {
  company: CompanyInfo
  number: string
  date: string // ISO (yyyy-mm-dd) — formatado no template
  customer: { name: string; phone?: string }
  device: { brand?: string; model?: string; kind?: string; problem?: string }
  services: ServiceItem[]
  parts: PartItem[]
  payment: { methods?: string; terms?: string }
  warranty: { period?: string }
  notes?: string
}

// Fallback usado quando a empresa ainda não preencheu o cadastro (cabeçalho da
// OS). Substituído pelos dados reais do Tenant assim que configurados.
export const MOCK_COMPANY: CompanyInfo = {
  name: 'Sua Empresa',
  responsible: 'Responsável pela empresa',
  cnpj: '00.000.000/0001-00',
  address: 'Endereço da empresa, nº',
  cityLine: 'Bairro, Cidade-UF',
  cep: '00000-000',
  email: 'contato@suaempresa.com',
  phones: ['+55 (00) 00000-0000'],
  instagram: '@suaempresa',
  city: 'Cidade',
  paymentMethods:
    'Dinheiro, pix, cartão de crédito, cartão de débito, transferência bancária.',
  paymentTerms: 'À vista.',
}
