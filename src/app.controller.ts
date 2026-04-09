import { Controller, Get } from "@nestjs/common";
import { RequestContext } from "./request-context";
import { DailySummaryService } from "./daily-summary.service";
import { AppService } from "./app.service";

@Controller("api")
export class AppController {
  constructor(
    private dailySummary: DailySummaryService,
    private appService: AppService,
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
  getDetailedHealth() {
    return this.appService.getDetailedHealth();
  }

  @Get("status")
  getStatus() {
    return this.dailySummary.getSummary();
  }
}
