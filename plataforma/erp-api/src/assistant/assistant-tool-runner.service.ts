import { Injectable, Logger } from '@nestjs/common';
import { UserRole } from '@prisma/client';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { BudgetService } from '../budget/budget.service';
import { CashService } from '../cash/cash.service';
import { CustomerSupplierService } from '../customer-supplier/customer-supplier.service';
import { OrderService } from '../order/order.service';
import { PaymentService } from '../payment/payment.service';
import { PrismaService } from '../prisma/prisma.service';
import { ProductService } from '../product/product.service';
import { ServiceOrderService } from '../service-order/service-order.service';
import { WalletService } from '../wallet/wallet.service';
import { AssistantMemoryService } from './assistant-memory.service';

export interface ToolCallResult {
  ok: boolean;
  data?: unknown;
  error?: string;
  summary?: string;
}

@Injectable()
export class AssistantToolRunnerService {
  private readonly logger = new Logger(AssistantToolRunnerService.name);

  constructor(
    private readonly prisma: PrismaService,
    private readonly customers: CustomerSupplierService,
    private readonly products: ProductService,
    private readonly orders: OrderService,
    private readonly payments: PaymentService,
    private readonly serviceOrders: ServiceOrderService,
    private readonly budgets: BudgetService,
    private readonly cash: CashService,
    private readonly wallet: WalletService,
    private readonly memory: AssistantMemoryService,
  ) {}

  async run(name: string, input: Record<string, unknown>): Promise<ToolCallResult> {
    try {
      switch (name) {
        case 'search_customer':
          return await this._searchCustomer(input);
        case 'create_customer':
          return await this._createCustomer(input);
        case 'search_product_semantic':
          return await this._searchProductSemantic(input);
        case 'check_stock':
          return await this._checkStock(input);
        case 'create_order':
          return await this._createOrder(input);
        case 'add_payment':
          return await this._addPayment(input);
        case 'fulfill_order':
          return await this._fulfillOrder(input);
        case 'create_service_order':
          return await this._createServiceOrder(input);
        case 'change_so_status':
          return await this._changeSoStatus(input);
        case 'create_budget':
          return await this._createBudget(input);
        case 'convert_budget':
          return await this._convertBudget(input);
        case 'open_cash_session':
          return await this._openCashSession(input);
        case 'wallet_summary':
          return await this._walletSummary();
        case 'save_memory':
          return await this._saveMemory(input);
        case 'forget_memory':
          return await this._forgetMemory(input);
        default:
          return { ok: false, error: `Tool desconhecida: ${name}` };
      }
    } catch (err) {
      this.logger.error(`tool ${name} falhou: ${(err as Error).message}`);
      return { ok: false, error: (err as Error).message };
    }
  }

  // -------------------- implementations --------------------

  private async _searchCustomer(input: Record<string, unknown>) {
    const q = String(input.q ?? '');
    const role = (input.role as 'CUSTOMER' | 'SUPPLIER' | 'BOTH' | undefined);
    const res = await this.customers.list({ q, role, page: 1, pageSize: 10 });
    return {
      ok: true,
      data: res.items.map((c) => ({
        id: c.id, name: c.name, document: c.document, email: c.email, phone: c.phone, role: c.role,
      })),
      summary: `${res.items.length} resultado(s) para "${q}"`,
    };
  }

  private async _createCustomer(input: Record<string, unknown>) {
    const cs = await this.customers.create({
      name: String(input.name),
      document: input.document as string | undefined,
      email: input.email as string | undefined,
      phone: input.phone as string | undefined,
      role: (input.role as 'CUSTOMER' | 'SUPPLIER' | 'BOTH' | undefined) ?? 'CUSTOMER',
      personType: input.personType as 'INDIVIDUAL' | 'COMPANY' | undefined,
    });
    return { ok: true, data: { id: cs.id, name: cs.name }, summary: `Cliente criado #${cs.id}` };
  }

  private async _searchProductSemantic(input: Record<string, unknown>) {
    const q = String(input.q ?? '');
    const res = await this.products.semanticSearch(q, 10);
    return {
      ok: true,
      data: res.items.slice(0, 10).map((p: any) => ({
        id: p.id, sku: p.sku, name: p.name,
        salePrice: Number(p.salePrice),
        unit: p.unit?.code,
        similarity: p.similarity ? Number(p.similarity.toFixed(3)) : undefined,
      })),
      summary: `Top ${Math.min(res.items.length, 10)} produtos por semântica`,
    };
  }

  private async _checkStock(input: Record<string, unknown>) {
    const tenantId = requireTenantId();
    let productId = input.productId as string | undefined;
    if (!productId && input.sku) {
      const found = await this.prisma.product.findFirst({
        where: { tenantId, sku: String(input.sku), deletedAt: null },
      });
      if (!found) return { ok: false, error: `SKU ${input.sku} não encontrado` };
      productId = found.id;
    }
    if (!productId) return { ok: false, error: 'productId ou sku obrigatório' };

    const product = await this.prisma.product.findFirst({
      where: { id: productId, tenantId },
    });
    if (!product) return { ok: false, error: 'Produto não encontrado' };

    const stocks = await this.prisma.stock.findMany({
      where: { tenantId, productId },
      include: { warehouse: { select: { code: true, name: true } } },
    });
    return {
      ok: true,
      data: {
        sku: product.sku, name: product.name,
        salePrice: Number(product.salePrice), costPrice: Number(product.costPrice),
        warehouses: stocks.map((s) => ({
          warehouse: s.warehouse.code,
          quantity: Number(s.quantity),
          reserved: Number(s.reserved),
          avgCost: Number(s.avgCost),
        })),
      },
      summary: `${product.sku}: ${stocks.reduce((a, s) => a + Number(s.quantity), 0)} unidades total`,
    };
  }

  private async _createOrder(input: Record<string, unknown>) {
    const items = (input.items as Array<{ productId: string; quantity: number; unitPrice?: number }>) ?? [];
    const order = await this.orders.create({
      customerId: input.customerId as string | undefined,
      items,
      discount: input.discount as number | undefined,
    });
    return {
      ok: true,
      data: { id: order.id, number: order.number, total: Number(order.total), status: order.status },
      summary: `Pedido #${order.number} criado · R$ ${Number(order.total).toFixed(2)}`,
    };
  }

  private async _addPayment(input: Record<string, unknown>) {
    const p = await this.payments.create({
      orderId: String(input.orderId),
      method: input.method as never,
      amount: Number(input.amount),
    });
    return {
      ok: true,
      data: { id: p.id, status: p.status, amount: Number(p.amount) },
      summary: `Pagamento ${p.status} R$ ${Number(p.amount).toFixed(2)}`,
    };
  }

  private async _fulfillOrder(input: Record<string, unknown>) {
    const o = await this.orders.fulfill(String(input.orderId));
    return { ok: true, data: { id: o.id, status: o.status }, summary: `Pedido ${o.status}` };
  }

  private async _createServiceOrder(input: Record<string, unknown>) {
    const so = await this.serviceOrders.create({
      customerId: String(input.customerId),
      vehicleId: input.vehicleId as string | undefined,
      description: String(input.description),
      parts: (input.parts as Array<{ productId: string; quantity: number }> | undefined) ?? [],
      labors: ((input.labors as Array<{ description: string; unitPrice: number }> | undefined) ?? []).map((l) => ({
        description: l.description, quantity: 1, unitPrice: l.unitPrice,
      })),
    });
    return {
      ok: true,
      data: { id: so.id, number: so.number, total: Number(so.total), status: so.status },
      summary: `OS #${so.number} aberta · R$ ${Number(so.total).toFixed(2)}`,
    };
  }

  private async _changeSoStatus(input: Record<string, unknown>) {
    const so = await this.serviceOrders.changeStatus(String(input.serviceOrderId), { status: input.status as never });
    return { ok: true, data: { id: so.id, status: so.status }, summary: `OS → ${so.status}` };
  }

  private async _createBudget(input: Record<string, unknown>) {
    const b = await this.budgets.create({
      customerId: input.customerId as string | undefined,
      target: input.target as never,
      items: (input.items as never[]) ?? [],
    });
    return {
      ok: true,
      data: { id: b.id, number: b.number, total: Number(b.total), status: b.status },
      summary: `Orçamento #${b.number} criado · R$ ${Number(b.total).toFixed(2)}`,
    };
  }

  private async _convertBudget(input: Record<string, unknown>) {
    const r = await this.budgets.convert(String(input.budgetId), {
      vehicleId: input.vehicleId as string | undefined,
    });
    return {
      ok: true,
      data: r,
      summary: `Orçamento convertido em ${r.type} #${r.number}`,
    };
  }

  private async _openCashSession(input: Record<string, unknown>) {
    const s = await this.cash.openSession(String(input.cashId), { openingAmount: Number(input.openingAmount) });
    return { ok: true, data: { id: s.id, status: s.status }, summary: `Caixa aberto · R$ ${Number(s.openingAmount).toFixed(2)}` };
  }

  private async _walletSummary() {
    const w = await this.wallet.getOrCreate();
    return {
      ok: true,
      data: {
        balance: Number(w.balance), blocked: Number(w.blocked),
        totalReceived: Number(w.totalReceived), totalPaidOut: Number(w.totalPaidOut),
      },
      summary: `Wallet: R$ ${Number(w.balance).toFixed(2)} disponível`,
    };
  }

  private async _saveMemory(input: Record<string, unknown>) {
    const r = await this.memory.save({
      title: String(input.title),
      content: String(input.content),
      category: String(input.category),
      priority: (input.priority as 'long_term' | 'short_term' | undefined) ?? 'short_term',
      visibility: (input.visibility as 'tenant_global' | 'private' | undefined) ?? 'private',
    });
    return { ok: true, data: r, summary: r.deduped ? 'Memória atualizada (similar existia)' : 'Memória salva' };
  }

  private async _forgetMemory(input: Record<string, unknown>) {
    const ok = await this.memory.forget(String(input.target));
    return { ok, summary: ok ? 'Memória esquecida' : 'Memória não encontrada' };
  }

  // ----- helpers -----

  /// Limita o set de tools ao perfil do role atual.
  filterByAllowed(allowed: string[], tools: typeof import('./assistant-tools').ASSISTANT_TOOLS) {
    if (allowed.length === 0) return [];
    return tools.filter((t) => allowed.includes(t.name));
  }
}
