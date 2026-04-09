import * as fs from "fs";
import * as path from "path";
import { Controller, Get } from "@nestjs/common";
import { RequestContext } from "./request-context";
import { DailySummaryService } from "./daily-summary.service";
import { PrismaService } from "./prisma.service";

@Controller("api")
export class AppController {
  constructor(
    private dailySummary: DailySummaryService,
    private prisma: PrismaService,
  ) {}

  @Get()
  getRoot() {
    return { name: "patria-api", client: RequestContext.alias, status: "ok" };
  }

  @Get("health")
  getHealth() {
    return { status: "healthy", timestamp: new Date().toISOString() };
  }

  @Get("health/detailed")
  async getHealthDetailed() {
    let dbStatus = "connected";
    let taskCount = 0;
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      taskCount = await this.prisma.task.count();
    } catch {
      dbStatus = "error";
    }
    const pkg = JSON.parse(fs.readFileSync(path.join(__dirname, "..", "package.json"), "utf-8"));
    return {
      version: pkg.version,
      uptime: process.uptime(),
      database: { status: dbStatus, taskCount },
      timestamp: new Date().toISOString(),
    };
  }

  @Get("status")
  getStatus() {
    return this.dailySummary.getSummary();
  }

  @Get("pending-actions")
  async getPendingActions() {
    const projectsAwaitingReview = await this.prisma.project.findMany({
      where: { status: "awaiting_review" },
      select: { id: true, name: true, status: true, environment: true, totalSubtasks: true, doneSubtasks: true },
    });

    const projectsAwaitingApproval = await this.prisma.project.findMany({
      where: { status: { in: ["planning", "approved"] } },
      select: { id: true, name: true, status: true },
    });

    const tasksAwaitingApproval = await this.prisma.task.findMany({
      where: { status: "awaiting_approval", projectId: null },
      select: { id: true, command: true, status: true },
    });

    const tasksNeedingInfo = await this.prisma.task.findMany({
      where: { status: "needs_info" },
      select: { id: true, command: true, status: true },
    });

    const projectsPaused = await this.prisma.project.findMany({
      where: { status: "paused" },
      select: { id: true, name: true, status: true },
    });

    const actions = [];

    for (const p of projectsAwaitingReview) {
      actions.push({
        type: "project",
        action: "promote_or_finalize",
        id: p.id,
        name: p.name,
        environment: p.environment,
        hint: `Projeto "${p.name}" concluido em ${p.environment}. Ações: promover (homolog/prod), finalizar, ou pedir mais.`,
      });
    }

    for (const p of projectsAwaitingApproval) {
      actions.push({
        type: "project",
        action: "approve",
        id: p.id,
        name: p.name,
        hint: `Projeto "${p.name}" aguardando aprovação.`,
      });
    }

    for (const t of tasksAwaitingApproval) {
      actions.push({
        type: "task",
        action: "approve",
        id: t.id,
        name: t.command,
        hint: `Tarefa "${t.command}" aguardando aprovação.`,
      });
    }

    for (const t of tasksNeedingInfo) {
      actions.push({
        type: "task",
        action: "provide_info",
        id: t.id,
        name: t.command,
        hint: `Tarefa "${t.command}" precisa de informação.`,
      });
    }

    for (const p of projectsPaused) {
      actions.push({
        type: "project",
        action: "resume_or_cancel",
        id: p.id,
        name: p.name,
        hint: `Projeto "${p.name}" pausado (subtarefa falhou). Ações: retenta, pula, ou cancela.`,
      });
    }

    return { actions, count: actions.length };
  }
}
