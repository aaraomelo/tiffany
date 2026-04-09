import { Controller, Get } from "@nestjs/common";
import * as path from "path";
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
    const pkg = require(path.join(__dirname, "..", "package.json"));
    const uptime = process.uptime();

    let dbStatus: "connected" | "disconnected" = "disconnected";
    let taskCount = 0;

    try {
      await this.prisma.$queryRaw`SELECT 1`;
      dbStatus = "connected";
      taskCount = await this.prisma.task.count();
    } catch {
      dbStatus = "disconnected";
    }

    return {
      version: pkg.version,
      uptime,
      database: {
        status: dbStatus,
        taskCount,
      },
      timestamp: new Date().toISOString(),
    };
  }
}
