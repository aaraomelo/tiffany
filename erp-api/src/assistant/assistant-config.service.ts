import { ForbiddenException, Injectable, UnauthorizedException } from '@nestjs/common';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { decryptApiKey, encryptApiKey, maskApiKey } from './crypto.util';

export interface AssistantConfigInput {
  llmProvider?: string;
  model?: string;
  apiKey?: string | null; // null = clear, undefined = keep
  soulPrompt?: string | null;
  active?: boolean;
}

const PRIVILEGED_ROLES = ['OWNER', 'ADMIN', 'MANAGER'];

@Injectable()
export class AssistantConfigService {
  constructor(private readonly prisma: PrismaService) {}

  private assertPrivileged() {
    const { role } = getTenantContext();
    if (!role || !PRIVILEGED_ROLES.includes(role)) {
      throw new ForbiddenException(
        'Apenas OWNER/ADMIN/MANAGER podem alterar a configuração do assistente',
      );
    }
  }

  async getOrCreate() {
    const tenantId = requireTenantId();
    const existing = await this.prisma.assistantConfig.findUnique({
      where: { tenantId },
    });
    if (existing) return existing;
    // Validar que o tenant existe — JWT pode estar apontando para tenant antigo
    // (caso comum após migrate reset em dev).
    const tenant = await this.prisma.tenant.findUnique({ where: { id: tenantId } });
    if (!tenant) {
      throw new UnauthorizedException(
        'Tenant da sessão não existe mais. Faça logout e login novamente.',
      );
    }
    return this.prisma.assistantConfig.create({
      data: { tenantId },
    });
  }

  /// View segura — não retorna a key, só máscara.
  async getView() {
    const cfg = await this.getOrCreate();
    return {
      llmProvider: cfg.llmProvider,
      model: cfg.model,
      hasApiKey: !!cfg.apiKeyEncrypted,
      apiKeyMasked: maskApiKey(cfg.apiKeyEncrypted),
      soulPrompt: cfg.soulPrompt,
      active: cfg.active,
      updatedAt: cfg.updatedAt,
    };
  }

  async update(input: AssistantConfigInput) {
    this.assertPrivileged();
    const tenantId = requireTenantId();
    await this.getOrCreate(); // ensure row exists

    const data: Record<string, unknown> = {};
    if (input.llmProvider !== undefined) data.llmProvider = input.llmProvider;
    if (input.model !== undefined) data.model = input.model;
    if (input.apiKey === null) data.apiKeyEncrypted = null;
    else if (typeof input.apiKey === 'string' && input.apiKey.length > 0) {
      data.apiKeyEncrypted = encryptApiKey(input.apiKey);
    }
    if (input.soulPrompt !== undefined) data.soulPrompt = input.soulPrompt;
    if (input.active !== undefined) data.active = input.active;

    await this.prisma.assistantConfig.update({
      where: { tenantId },
      data,
    });
    return this.getView();
  }

  /// Resolve API key descriptografada para uso interno (LLM client).
  async resolveApiKey(): Promise<string | null> {
    const cfg = await this.getOrCreate();
    if (!cfg.apiKeyEncrypted) {
      // fallback: env var global (dev)
      return process.env.ANTHROPIC_API_KEY ?? null;
    }
    return decryptApiKey(cfg.apiKeyEncrypted);
  }

  /// Resolve model name efetivo do tenant.
  async resolveModel(): Promise<string> {
    const cfg = await this.getOrCreate();
    return cfg.model;
  }

  /// Soul prompt (com fallback para default).
  async resolveSoulPrompt(): Promise<string | null> {
    const cfg = await this.getOrCreate();
    return cfg.soulPrompt;
  }
}
