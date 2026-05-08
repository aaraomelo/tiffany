-- Organism nerve events from GEX44

CREATE TABLE "organism_events" (
    "id" BIGSERIAL NOT NULL,
    "ts" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "received_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "source" TEXT NOT NULL,
    "kind" TEXT NOT NULL,
    "severity" INTEGER NOT NULL DEFAULT 3,
    "data" JSONB,
    "embedding" vector(768),
    "consumed_at" TIMESTAMP(3),
    "consumed_by" TEXT,
    "notified_at" TIMESTAMP(3),

    CONSTRAINT "organism_events_pkey" PRIMARY KEY ("id")
);

CREATE INDEX "organism_events_ts_idx" ON "organism_events" ("ts" DESC);
CREATE INDEX "organism_events_kind_idx" ON "organism_events" ("kind");
CREATE INDEX "organism_events_source_idx" ON "organism_events" ("source");
CREATE INDEX "organism_events_consumed_at_idx" ON "organism_events" ("consumed_at");
CREATE INDEX "organism_events_unconsumed_idx" ON "organism_events" ("ts" DESC) WHERE "consumed_at" IS NULL;
CREATE INDEX "organism_events_embedding_idx" ON "organism_events"
    USING ivfflat ("embedding" vector_cosine_ops) WITH (lists = 10);
