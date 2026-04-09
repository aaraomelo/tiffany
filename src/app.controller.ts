import { Controller, Get } from "@nestjs/common";
import { RequestContext } from "./request-context";
import { DailySummaryService } from "./daily-summary.service";

@Controller("api")
export class AppController {
  constructor(private dailySummary: DailySummaryService) {}

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
}
