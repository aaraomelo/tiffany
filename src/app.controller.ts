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

  @Get("status")
  getStatus() {
    return this.dailySummary.getSummary();
  }

  @Get("health/detailed")
  async getHealthDetailed() {
    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const { version } = require("../../package.json");

    let postgres: "ok" | "error" = "ok";
    try {
      await this.prisma.$queryRaw`SELECT 1`;
    } catch {
      postgres = "error";
    }

    const taskCount = await this.prisma.task.count();

    return {
      version,
      uptime: process.uptime(),
      taskCount,
      postgres,
    };
  }
}
