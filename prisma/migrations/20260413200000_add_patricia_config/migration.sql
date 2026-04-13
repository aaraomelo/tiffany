-- CreateTable
CREATE TABLE "patricia_config" (
    "key" TEXT NOT NULL,
    "value" TEXT NOT NULL,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "patricia_config_pkey" PRIMARY KEY ("key")
);

-- Seed default model
INSERT INTO "patricia_config" ("key", "value", "updated_at") VALUES ('model', 'claude-haiku-4-5-20251001', NOW());
