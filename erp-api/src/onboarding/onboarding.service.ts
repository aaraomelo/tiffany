import { Injectable } from '@nestjs/common';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';

export interface Step {
  key: string;
  done: boolean;
  route: string;
}

@Injectable()
export class OnboardingService {
  constructor(private readonly prisma: PrismaService) {}

  /** Estado dos "primeiros passos" do tenant (checklist de onboarding). */
  async status() {
    const tenantId = requireTenantId();

    const [tenant, modules] = await Promise.all([
      this.prisma.tenant.findUnique({
        where: { id: tenantId },
        select: { landingConfig: true },
      }),
      this.prisma.tenantModule.findMany({
        where: { tenantId, enabled: true },
        select: { moduleSlug: true },
      }),
    ]);
    const on = new Set(modules.map((m) => m.moduleSlug));

    const landing = (tenant?.landingConfig ?? null) as { headline?: string } | null;
    const steps: Step[] = [];

    // 1) site
    steps.push({ key: 'site', done: !!landing?.headline, route: '/site' });

    // 2) primeiro cadastro de pessoas (aluno OU cliente, conforme o segmento)
    if (on.has('student')) {
      const n = await this.prisma.student.count({ where: { tenantId, deletedAt: null } });
      steps.push({ key: 'student', done: n > 0, route: '/students' });
    } else if (on.has('customer-supplier')) {
      const n = await this.prisma.customerSupplier.count({ where: { tenantId, deletedAt: null } });
      steps.push({ key: 'customer', done: n > 0, route: '/customers' });
    }

    // 3) produtos (se o módulo estiver ativo)
    if (on.has('product')) {
      const n = await this.prisma.product.count({ where: { tenantId, deletedAt: null } });
      steps.push({ key: 'product', done: n > 0, route: '/products' });
    }

    // 4) plano (se Educação/Saúde)
    if (on.has('enrollment-plan')) {
      const n = await this.prisma.enrollmentPlan.count({ where: { tenantId } });
      steps.push({ key: 'plan', done: n > 0, route: '/enrollment-plans' });
    }

    const total = steps.length;
    const done = steps.filter((s) => s.done).length;
    return { steps, total, done, allDone: done === total };
  }
}
