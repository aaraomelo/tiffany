-- Catálogo persistente dos voluntários do multiverso distribuído
-- + auditoria de comandos despachados pela Patrícia.

CREATE TABLE "multiverso_voluntarios" (
    "id" TEXT NOT NULL,
    "vid" TEXT NOT NULL,
    "display_name" TEXT NOT NULL,
    "host" TEXT,
    "role" TEXT NOT NULL DEFAULT 'voluntary',
    "runtime" TEXT NOT NULL DEFAULT 'python',
    "is_lab" BOOLEAN NOT NULL DEFAULT false,
    "description" TEXT NOT NULL,
    "embedding" vector(768),
    "embedded_at" TIMESTAMP(3),
    "last_seen_at" TIMESTAMP(3),
    "last_metrics" JSONB,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "multiverso_voluntarios_pkey" PRIMARY KEY ("id")
);

CREATE UNIQUE INDEX "multiverso_voluntarios_vid_key" ON "multiverso_voluntarios" ("vid");
CREATE INDEX "multiverso_voluntarios_role_idx" ON "multiverso_voluntarios" ("role");
CREATE INDEX "multiverso_voluntarios_runtime_idx" ON "multiverso_voluntarios" ("runtime");
CREATE INDEX "multiverso_voluntarios_embedded_at_idx" ON "multiverso_voluntarios" ("embedded_at");

CREATE TABLE "multiverso_control_log" (
    "id" BIGSERIAL NOT NULL,
    "ts" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "target" TEXT NOT NULL,
    "cmd" TEXT NOT NULL,
    "factor" DOUBLE PRECISION,
    "duration_sec" INTEGER,
    "expires_at" TIMESTAMP(3),
    "issued_by" TEXT NOT NULL DEFAULT 'patricia',
    "reason" TEXT,
    "ack_ok" BOOLEAN,

    CONSTRAINT "multiverso_control_log_pkey" PRIMARY KEY ("id")
);

CREATE INDEX "multiverso_control_log_ts_idx" ON "multiverso_control_log" ("ts" DESC);
CREATE INDEX "multiverso_control_log_target_idx" ON "multiverso_control_log" ("target");
