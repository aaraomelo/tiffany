import { Controller, Get } from "@nestjs/common";
import { RequestContext } from "./request-context";
import { DailySummaryService } from "./daily-summary.service";
import { PrismaService } from "./prisma.service";

// eslint-disable-next-line @typescript-eslint/no-var-requires
const pkg = require("../../package.json");

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
    const taskCount = await this.prisma.task.count();

    let dbStatus: string;
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      dbStatus = "healthy";
    } catch {
      dbStatus = "unhealthy";
    }

    return {
      version: pkg.version,
      uptime: process.uptime(),
      taskCount,
      dbStatus,
      timestamp: new Date().toISOString(),
    };
  }
}
