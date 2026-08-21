CREATE TABLE "media_jobs" (
    "id" TEXT NOT NULL,
    "kind" TEXT NOT NULL,
    "status" TEXT NOT NULL DEFAULT 'pending',
    "params" JSONB NOT NULL,
    "channel" TEXT NOT NULL,
    "target" TEXT NOT NULL,
    "result_url" TEXT,
    "error" TEXT,
    "attempts" INTEGER NOT NULL DEFAULT 0,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "started_at" TIMESTAMP(3),
    "completed_at" TIMESTAMP(3),

    CONSTRAINT "media_jobs_pkey" PRIMARY KEY ("id")
);

CREATE INDEX "media_jobs_status_idx" ON "media_jobs" ("status");
CREATE INDEX "media_jobs_created_at_idx" ON "media_jobs" ("created_at");
