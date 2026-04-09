import { Injectable, Logger } from '@nestjs/common';
import { execSync } from 'child_process';

@Injectable()
export class ClaudeService {
  private readonly logger = new Logger(ClaudeService.name);

  async inferRepo(command: string, description?: string): Promise<string | null> {
    const prompt = `Analise a tarefa abaixo e determine em qual repositório ela deve ser executada.

Repositórios:
- landpage: Landing page institucional React + Vite. Componentes visuais (.jsx + .css), seções da página (Header, Hero, Services, About, HowItWorks, Results, Pricing, Footer, Contact), formulários frontend, CSS puro, Lucide React para ícones.
- patria-api: API backend NestJS + TypeScript + Prisma + PostgreSQL. Endpoints REST, controllers, services, DTOs, migrations, Swagger, validação com class-validator.
- patria-app: App multi-tenant React + Vite. Dashboard do cliente acessível via subdomínio {alias}.patriatechnology.com.

Tarefa:
- Comando: ${command}
- Descrição: ${description || 'N/A'}

Responda APENAS com o nome do repositório, sem explicação: landpage, patria-api ou patria-app`;

    try {
      const result = execSync(
        'node /claude-code/cli.js -p --max-turns 1',
        {
          encoding: 'utf-8',
          timeout: 30_000,
          input: prompt,
          env: { ...process.env, HOME: '/root' },
          shell: '/bin/sh',
        },
      ).trim().toLowerCase();

      if (result.includes('patria-api')) return 'patria-api';
      if (result.includes('patria-app')) return 'patria-app';
      if (result.includes('landpage')) return 'landpage';

      this.logger.warn(`Unexpected Claude response: ${result}`);
      return null;
    } catch (err) {
      this.logger.warn(`Claude inferRepo failed: ${err.message}`);
      return null;
    }
  }
}
