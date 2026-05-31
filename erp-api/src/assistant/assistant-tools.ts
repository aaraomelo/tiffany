/// Lista de tools que o assistente pode chamar. Schema compatível com Anthropic SDK.
/// O ToolRunner executa cada uma chamando os services do ERP.
export const ASSISTANT_TOOLS = [
  // ---------- Clientes / fornecedores ----------
  {
    name: 'search_customer',
    description: 'Busca cliente ou fornecedor por nome, documento ou email. Retorna até 10 resultados. USE antes de criar pedido pra confirmar o cliente.',
    input_schema: {
      type: 'object' as const,
      properties: {
        q: { type: 'string', description: 'Texto da busca (nome, CPF/CNPJ, email)' },
        role: { type: 'string', enum: ['CUSTOMER', 'SUPPLIER', 'BOTH'], description: 'Filtro de papel (default CUSTOMER)' },
      },
      required: ['q'],
    },
  },
  {
    name: 'create_customer',
    description: 'Cria um novo cliente ou fornecedor. CONFIRME com o usuário os dados antes de chamar.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome completo ou razão social' },
        document: { type: 'string', description: 'CPF ou CNPJ (opcional)' },
        email: { type: 'string', description: 'Email (opcional)' },
        phone: { type: 'string', description: 'Telefone (opcional)' },
        role: { type: 'string', enum: ['CUSTOMER', 'SUPPLIER', 'BOTH'], description: 'default CUSTOMER' },
        personType: { type: 'string', enum: ['INDIVIDUAL', 'COMPANY'] },
      },
      required: ['name'],
    },
  },

  // ---------- Produto / estoque ----------
  {
    name: 'search_product_semantic',
    description: 'Busca produto por similaridade semântica (BGE-M3). Aceita linguagem natural ex: "cimento branco pra parede" ou "agregado pra concreto". Retorna até 10 resultados com saldo se houver.',
    input_schema: {
      type: 'object' as const,
      properties: {
        q: { type: 'string', description: 'Texto da busca em linguagem natural' },
      },
      required: ['q'],
    },
  },
  {
    name: 'check_stock',
    description: 'Saldo de estoque, custo médio e preço de venda de um produto específico. Pode passar productId direto ou identificar via SKU.',
    input_schema: {
      type: 'object' as const,
      properties: {
        productId: { type: 'string', description: 'UUID do produto' },
        sku: { type: 'string', description: 'SKU exato (alternativa a productId)' },
      },
    },
  },

  // ---------- Pedido / venda ----------
  {
    name: 'create_order',
    description: 'Cria pedido de venda com itens. CONFIRME total com o usuário antes. O pedido fica em AWAITING_PAYMENT até registrar pagamento via add_payment. Retorna orderId e total calculado.',
    input_schema: {
      type: 'object' as const,
      properties: {
        customerId: { type: 'string', description: 'UUID do cliente (opcional para venda balcão)' },
        items: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              productId: { type: 'string' },
              quantity: { type: 'number' },
              unitPrice: { type: 'number', description: 'Opcional, default = salePrice do produto' },
            },
            required: ['productId', 'quantity'],
          },
        },
        discount: { type: 'number', description: 'Desconto total (opcional)' },
      },
      required: ['items'],
    },
  },
  {
    name: 'add_payment',
    description: 'Registra um pagamento em um pedido (parcial ou total). Métodos CASH e CREDIT_NOTE confirmam imediatamente. PIX/cartão ficam PENDING (precisa webhook).',
    input_schema: {
      type: 'object' as const,
      properties: {
        orderId: { type: 'string' },
        method: { type: 'string', enum: ['CASH', 'PIX', 'CREDIT_CARD', 'DEBIT_CARD', 'CREDIT_NOTE'] },
        amount: { type: 'number' },
      },
      required: ['orderId', 'method', 'amount'],
    },
  },
  {
    name: 'fulfill_order',
    description: 'Finaliza o pedido: aplica baixa de estoque e marca como COMPLETED. Só funciona em pedido AWAITING_PAYMENT/PAID/FULFILLING.',
    input_schema: {
      type: 'object' as const,
      properties: { orderId: { type: 'string' } },
      required: ['orderId'],
    },
  },

  // ---------- Ordem de serviço ----------
  {
    name: 'create_service_order',
    description: 'Abre uma OS. Pode incluir peças (drawable do estoque) e mão de obra. Status inicial OPEN.',
    input_schema: {
      type: 'object' as const,
      properties: {
        customerId: { type: 'string' },
        vehicleId: { type: 'string', description: 'UUID do veículo (opcional)' },
        description: { type: 'string' },
        parts: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              productId: { type: 'string' },
              quantity: { type: 'number' },
            },
            required: ['productId', 'quantity'],
          },
        },
        labors: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              description: { type: 'string' },
              unitPrice: { type: 'number' },
            },
            required: ['description', 'unitPrice'],
          },
        },
      },
      required: ['customerId', 'description'],
    },
  },
  {
    name: 'change_so_status',
    description: 'Muda status de uma OS. Transições válidas: OPEN→IN_PROGRESS/WAITING_PARTS/WAITING_CUSTOMER/CANCELLED, IN_PROGRESS→FINISHED, FINISHED→DELIVERED. FINISHED aplica baixa de estoque.',
    input_schema: {
      type: 'object' as const,
      properties: {
        serviceOrderId: { type: 'string' },
        status: { type: 'string', enum: ['IN_PROGRESS', 'WAITING_PARTS', 'WAITING_CUSTOMER', 'FINISHED', 'DELIVERED', 'CANCELLED'] },
      },
      required: ['serviceOrderId', 'status'],
    },
  },

  // ---------- Orçamento ----------
  {
    name: 'create_budget',
    description: 'Cria orçamento. target=SERVICE_ORDER permite mix peça+mão de obra, target=ORDER só produtos. Status inicial DRAFT.',
    input_schema: {
      type: 'object' as const,
      properties: {
        customerId: { type: 'string' },
        target: { type: 'string', enum: ['ORDER', 'SERVICE_ORDER'] },
        items: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              productId: { type: 'string', description: 'opcional pra itens de mão de obra' },
              description: { type: 'string' },
              quantity: { type: 'number' },
              unitPrice: { type: 'number' },
              isLabor: { type: 'boolean', description: 'true = mão de obra (sem productId)' },
            },
            required: ['description', 'quantity', 'unitPrice'],
          },
        },
      },
      required: ['items'],
    },
  },
  {
    name: 'convert_budget',
    description: 'Converte um orçamento APPROVED em Order (se target=ORDER) ou ServiceOrder (se target=SERVICE_ORDER).',
    input_schema: {
      type: 'object' as const,
      properties: {
        budgetId: { type: 'string' },
        vehicleId: { type: 'string', description: 'Para SERVICE_ORDER, opcional' },
      },
      required: ['budgetId'],
    },
  },

  // ---------- Caixa / wallet ----------
  {
    name: 'open_cash_session',
    description: 'Abre uma sessão de caixa. Só uma sessão aberta por caixa por vez.',
    input_schema: {
      type: 'object' as const,
      properties: {
        cashId: { type: 'string' },
        openingAmount: { type: 'number' },
      },
      required: ['cashId', 'openingAmount'],
    },
  },
  {
    name: 'wallet_summary',
    description: 'Saldo atual da carteira do tenant: balance, blocked, totalReceived, totalPaidOut.',
    input_schema: {
      type: 'object' as const,
      properties: {},
    },
  },

  // ---------- Memória ----------
  {
    name: 'save_memory',
    description: 'Guarda uma memória que o assistente deve lembrar em conversas futuras. USE quando o usuário compartilhar decisão importante, preferência, ou informação estratégica.',
    input_schema: {
      type: 'object' as const,
      properties: {
        title: { type: 'string', description: 'Título curto (até 80 chars)' },
        content: { type: 'string', description: 'Conteúdo da memória' },
        category: {
          type: 'string',
          enum: ['decision', 'preference', 'sale', 'customer', 'product', 'technical', 'person'],
        },
        priority: {
          type: 'string',
          enum: ['long_term', 'short_term'],
          description: 'long_term = permanente, short_term = 30 dias (default)',
        },
        visibility: {
          type: 'string',
          enum: ['tenant_global', 'private'],
          description: 'tenant_global = todos do tenant veem, private = só o usuário (default)',
        },
      },
      required: ['title', 'content', 'category'],
    },
  },
  {
    name: 'forget_memory',
    description: 'Esquece (arquiva) uma memória. Pode passar id ou título parcial. Não esquece memórias core.',
    input_schema: {
      type: 'object' as const,
      properties: {
        target: { type: 'string', description: 'UUID ou parte do título' },
      },
      required: ['target'],
    },
  },
] as const;

export type AssistantToolName = (typeof ASSISTANT_TOOLS)[number]['name'];
