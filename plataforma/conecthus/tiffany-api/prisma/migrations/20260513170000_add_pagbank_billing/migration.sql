-- Add PagBank billing fields to tenants (substitui Stripe que nunca foi pra prod)

ALTER TABLE "tenants"
  ADD COLUMN IF NOT EXISTS "api_key" TEXT,
  ADD COLUMN IF NOT EXISTS "billing_provider" TEXT,
  ADD COLUMN IF NOT EXISTS "billing_email" TEXT,
  ADD COLUMN IF NOT EXISTS "subscription_code" TEXT,
  ADD COLUMN IF NOT EXISTS "subscription_status" TEXT,
  ADD COLUMN IF NOT EXISTS "plan_renews_at" TIMESTAMPTZ;

CREATE UNIQUE INDEX IF NOT EXISTS "tenants_api_key_key" ON "tenants"("api_key");

-- billing events (auditoria PagBank)
CREATE TABLE IF NOT EXISTS "billing_events" (
    "id" SERIAL PRIMARY KEY,
    "ts" TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    "provider" TEXT NOT NULL,
    "event_type" TEXT NOT NULL,
    "tenant_id" TEXT,
    "payload" JSONB NOT NULL,
    "processed" BOOLEAN DEFAULT FALSE
);

CREATE INDEX IF NOT EXISTS "idx_billing_events_tenant" ON "billing_events"("tenant_id");
CREATE INDEX IF NOT EXISTS "idx_billing_events_ts" ON "billing_events"("ts" DESC);

-- email verifications
CREATE TABLE IF NOT EXISTS "email_verifications" (
    "id" TEXT NOT NULL DEFAULT gen_random_uuid()::text,
    "user_id" TEXT NOT NULL,
    "token" TEXT NOT NULL UNIQUE,
    "expires_at" TIMESTAMPTZ NOT NULL,
    "verified_at" TIMESTAMPTZ,
    "created_at" TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    PRIMARY KEY ("id"),
    FOREIGN KEY ("user_id") REFERENCES "users"("id") ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS "email_verifications_user_id_idx" ON "email_verifications"("user_id");

-- NcoCall (log)
CREATE TABLE IF NOT EXISTS "nco_calls" (
    "id" BIGSERIAL PRIMARY KEY,
    "ts" TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    "api_key" TEXT NOT NULL,
    "api_agent" TEXT NOT NULL,
    "endpoint" TEXT NOT NULL,
    "n" INTEGER NOT NULL,
    "model_name" TEXT,
    "rollouts" INTEGER,
    "result" JSONB,
    "time_ms" DOUBLE PRECISION,
    "status_code" INTEGER NOT NULL DEFAULT 200,
    "error_msg" TEXT
);

CREATE INDEX IF NOT EXISTS "nco_calls_ts_idx" ON "nco_calls"("ts" DESC);
CREATE INDEX IF NOT EXISTS "nco_calls_api_key_idx" ON "nco_calls"("api_key");
CREATE INDEX IF NOT EXISTS "nco_calls_endpoint_idx" ON "nco_calls"("endpoint");

-- Add tenant_id + email verification fields to users
ALTER TABLE "users"
  ADD COLUMN IF NOT EXISTS "tenant_id" TEXT,
  ADD COLUMN IF NOT EXISTS "email_verified" BOOLEAN NOT NULL DEFAULT FALSE,
  ADD COLUMN IF NOT EXISTS "email_verified_at" TIMESTAMPTZ;

CREATE UNIQUE INDEX IF NOT EXISTS "users_tenant_id_key" ON "users"("tenant_id") WHERE "tenant_id" IS NOT NULL;

DO $$ BEGIN
  ALTER TABLE "users" ADD CONSTRAINT "users_tenant_id_fkey" FOREIGN KEY ("tenant_id") REFERENCES "tenants"("id") ON DELETE SET NULL ON UPDATE CASCADE;
EXCEPTION WHEN duplicate_object THEN null; END $$;
