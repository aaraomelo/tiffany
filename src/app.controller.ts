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
    const version = require("../package.json").version as string;
    const uptime = Math.floor(process.uptime());
    const tasks = await this.prisma.task.count();
    let database: "connected" | "disconnected";
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      database = "connected";
    } catch {
      database = "disconnected";
    }
    return { version, uptime, tasks, database };
  }
}
