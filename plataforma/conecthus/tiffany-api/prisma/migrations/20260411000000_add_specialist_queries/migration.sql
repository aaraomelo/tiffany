CREATE TABLE "specialist_queries" (
    "id" TEXT NOT NULL,
    "question" TEXT NOT NULL,
    "type" TEXT NOT NULL DEFAULT 'diagnose',
    "repo" TEXT,
    "project_id" TEXT,
    "channel" TEXT NOT NULL DEFAULT 'whatsapp',
    "target" TEXT NOT NULL DEFAULT '+5511977808883',
    "status" TEXT NOT NULL DEFAULT 'pending',
    "answer" TEXT,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,
    CONSTRAINT "specialist_queries_pkey" PRIMARY KEY ("id")
);
CREATE INDEX "specialist_queries_status_idx" ON "specialist_queries"("status");
