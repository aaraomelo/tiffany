import { Controller, Get } from "@nestjs/common";
import * as path from "path";
import * as fs from "fs";
import { RequestContext } from "./request-context";
import { DailySummaryService } from "./daily-summary.service";
import { PrismaService } from "./prisma.service";
import { TasksService } from "./tasks.service";

@Controller("api")
export class AppController {
  constructor(
    private dailySummary: DailySummaryService,
    private prisma: PrismaService,
    private tasks: TasksService,
  ) {}

  @Get()
  getRoot() {
    return { name: "patria-api", client: RequestContext.alias, status: "ok" };
  }

  @Get("health")
  getHealth() {
    return { status: "healthy", timestamp: new Date().toISOString() };
  }

  @Get("status")
  getStatus() {
    return this.dailySummary.getSummary();
  }

  @Get("health/detailed")
  async getHealthDetailed() {
    const pkgPath = path.join(__dirname, "..", "package.json");
    const version = JSON.parse(fs.readFileSync(pkgPath, "utf8")).version;

    let postgres: "ok" | "error" = "error";
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      postgres = "ok";
    } catch {}

    const tasks_total = await this.prisma.task.count();

    return {
      version,
      uptime: process.uptime(),
      timestamp: new Date().toISOString(),
      postgres,
      tasks_total,
    };
  }
}
