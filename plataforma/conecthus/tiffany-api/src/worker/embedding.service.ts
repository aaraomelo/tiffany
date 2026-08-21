import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';

const GEMINI_API_KEY = process.env.GEMINI_API_KEY;

@Injectable()
export class EmbeddingService {
  private readonly logger = new Logger('Embedding');

  constructor(private prisma: PrismaService) {}

  async generateEmbedding(text: string): Promise<number[]> {
    if (!GEMINI_API_KEY) throw new Error('GEMINI_API_KEY not set');
    const res = await fetch(
      `https://generativelanguage.googleapis.com/v1beta/models/gemini-embedding-001:embedContent?key=${GEMINI_API_KEY}`,
      {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content: { parts: [{ text }] }, outputDimensionality: 768 }),
        signal: AbortSignal.timeout(10_000),
      },
    );
    if (!res.ok) throw new Error(`Gemini API ${res.status}: ${await res.text()}`);
    const data = await res.json();
    return data.embedding.values;
  }

  async saveTaskEmbedding(taskId: string, command: string, description?: string, projectId?: string): Promise<void> {
    let projectName = '';
    if (projectId) {
      const proj = await this.prisma.project.findUnique({ where: { id: projectId }, select: { name: true } });
      if (proj) projectName = proj.name;
    }
    const text = `${projectName ? projectName + ' ' : ''}${command} ${description || ''}`.trim();
    const embedding = await this.generateEmbedding(text);
    const vectorStr = `[${embedding.join(',')}]`;
    await this.prisma.$executeRawUnsafe(`UPDATE tasks SET embedding = $1::vector WHERE id = $2`, vectorStr, taskId);
  }

  async saveProjectEmbedding(projectId: string, name: string, description?: string): Promise<void> {
    const text = `${name} ${description || ''}`.trim();
    const embedding = await this.generateEmbedding(text);
    const vectorStr = `[${embedding.join(',')}]`;
    await this.prisma.$executeRawUnsafe(`UPDATE projects SET embedding = $1::vector WHERE id = $2`, vectorStr, projectId);
  }

  async searchTasks(query: string, limit = 5): Promise<any[]> {
    const embedding = await this.generateEmbedding(query);
    const vectorStr = `[${embedding.join(',')}]`;
    return this.prisma.$queryRawUnsafe(
      `SELECT id, command, description, "project" as repo, status, project_id as "projectId",
              embedding <=> $1::vector AS distance
       FROM tasks WHERE embedding IS NOT NULL
       ORDER BY embedding <=> $1::vector LIMIT $2`,
      vectorStr, limit,
    );
  }

  async searchProjects(query: string, limit = 5): Promise<any[]> {
    const embedding = await this.generateEmbedding(query);
    const vectorStr = `[${embedding.join(',')}]`;
    return this.prisma.$queryRawUnsafe(
      `SELECT p.id, p.name, p.description, p.status, p.environment,
              p.total_subtasks as "totalSubtasks", p.done_subtasks as "doneSubtasks",
              p.embedding <=> $1::vector AS distance
       FROM projects p WHERE p.embedding IS NOT NULL
       ORDER BY p.embedding <=> $1::vector LIMIT $2`,
      vectorStr, limit,
    );
  }

  async findSimilarTasks(text: string, limit = 5): Promise<any[]> {
    const embedding = await this.generateEmbedding(text);
    const vectorStr = `[${embedding.join(',')}]`;
    return this.prisma.$queryRawUnsafe(
      `SELECT id, command, description, "project" as repo, status,
              embedding <=> $1::vector AS distance
       FROM tasks WHERE embedding IS NOT NULL AND status = 'completed'
       ORDER BY embedding <=> $1::vector LIMIT $2`,
      vectorStr, limit,
    );
  }

  async backfillAll(): Promise<{ tasks: number; projects: number }> {
    const tasks: any[] = await this.prisma.$queryRawUnsafe(`SELECT id, command, description FROM tasks WHERE embedding IS NULL`);
    let tasksDone = 0;
    for (const t of tasks) {
      try {
        await this.saveTaskEmbedding(t.id, t.command, t.description);
        tasksDone++;
      } catch {}
    }

    const projects: any[] = await this.prisma.$queryRawUnsafe(`SELECT id, name, description FROM projects WHERE embedding IS NULL`);
    let projectsDone = 0;
    for (const p of projects) {
      try {
        await this.saveProjectEmbedding(p.id, p.name, p.description);
        projectsDone++;
      } catch {}
    }

    return { tasks: tasksDone, projects: projectsDone };
  }
}
