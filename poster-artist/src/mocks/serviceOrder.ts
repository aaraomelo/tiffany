// Modelo de dados da Ordem de Serviço (genérico, agnóstico de backend). Por ora
// alimentado por mock; depois plugável no domínio ServiceOrder do erp-api.

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
  company: {
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
  }
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

// Dados reais do PDF modelo (Ordem de serviço n. 012-2026).
export const MOCK_SERVICE_ORDER: ServiceOrderData = {
  company: {
    name: 'Sit Tecnologic',
    responsible: 'LUIS ENRIQUE FERREIRA ALVES 02078951200',
    cnpj: '48.716.902/0001-55',
    address: 'Rua Edmundo Sales, 72, casa',
    cityLine: 'Buritis, Boa Vista-RR',
    cep: '69309-225',
    email: 'enrique.gloobalturismo@hotmail.com',
    phones: ['+55 (95) 8112-4442', '+55 (95) 98404-6899'],
    instagram: 'sit_tecnologic',
    city: 'Boa Vista',
  },
  number: '012-2026',
  date: '2026-06-08',
  customer: { name: 'Faelfofo tec', phone: '+55 (95) 8123-4023' },
  device: {
    brand: 'Lenovo',
    model: 'Notebook',
    kind: 'Laptop',
    problem: 'Notebook Lenovo 3D com defeito na placa mãe',
  },
  services: [{ description: 'Mão de obra', detail: 'Mao de obra', price: 100 }],
  parts: [
    {
      name: 'Placa mãe  Lenovo 3i',
      detail: 'Placa-mãe Lenovo 3i completa',
      unit: 'un.',
      unitPrice: 400,
      qty: 1,
      price: 400,
    },
  ],
  payment: {
    methods:
      'Boleto, transferência bancária, dinheiro, cheque, cartão de crédito, cartão de débito, pix, picpay ou link de pagamento.',
    terms: 'À vista.',
  },
  warranty: { period: '30 dias' },
  notes:
    'notebook Lenovo 3D entrou com diagnóstico que ligava e não apresentava vídeo fizemos o diagnóstico e descobrimos que a placa mãe estava em curto na linha principal de 12 v tentamos fazer o reparo mas não teve isso devido o seu processador está danificado também daí a placa foi trocada por completo',
}
