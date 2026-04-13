-- CreateEnum
CREATE TYPE "BillingStatus" AS ENUM ('trialing', 'active', 'past_due', 'canceled', 'unpaid', 'incomplete');

-- AlterTable
ALTER TABLE "tenants" ADD COLUMN "stripe_customer_id" TEXT,
ADD COLUMN "stripe_subscription_id" TEXT,
ADD COLUMN "stripe_price_id" TEXT,
ADD COLUMN "billing_status" "BillingStatus",
ADD COLUMN "current_period_end" TIMESTAMP(3);

-- CreateIndex
CREATE UNIQUE INDEX "tenants_stripe_customer_id_key" ON "tenants"("stripe_customer_id");

-- CreateIndex
CREATE UNIQUE INDEX "tenants_stripe_subscription_id_key" ON "tenants"("stripe_subscription_id");

-- CreateTable
CREATE TABLE "subscriptions" (
    "id" TEXT NOT NULL,
    "tenant_id" TEXT NOT NULL,
    "stripe_subscription_id" TEXT NOT NULL,
    "stripe_price_id" TEXT NOT NULL,
    "status" "BillingStatus" NOT NULL,
    "current_period_start" TIMESTAMP(3) NOT NULL,
    "current_period_end" TIMESTAMP(3) NOT NULL,
    "cancel_at_period_end" BOOLEAN NOT NULL DEFAULT false,
    "canceled_at" TIMESTAMP(3),
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "subscriptions_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE UNIQUE INDEX "subscriptions_stripe_subscription_id_key" ON "subscriptions"("stripe_subscription_id");

-- CreateIndex
CREATE INDEX "subscriptions_tenant_id_idx" ON "subscriptions"("tenant_id");

-- CreateIndex
CREATE INDEX "subscriptions_status_idx" ON "subscriptions"("status");

-- CreateTable
CREATE TABLE "invoices" (
    "id" TEXT NOT NULL,
    "tenant_id" TEXT NOT NULL,
    "subscription_id" TEXT,
    "stripe_invoice_id" TEXT NOT NULL,
    "amount_cents" INTEGER NOT NULL,
    "currency" TEXT NOT NULL DEFAULT 'brl',
    "status" TEXT NOT NULL,
    "invoice_url" TEXT,
    "paid_at" TIMESTAMP(3),
    "due_date" TIMESTAMP(3),
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "invoices_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE UNIQUE INDEX "invoices_stripe_invoice_id_key" ON "invoices"("stripe_invoice_id");

-- CreateIndex
CREATE INDEX "invoices_tenant_id_idx" ON "invoices"("tenant_id");

-- CreateIndex
CREATE INDEX "invoices_status_idx" ON "invoices"("status");

-- CreateTable
CREATE TABLE "usage_counters" (
    "id" TEXT NOT NULL,
    "tenant_id" TEXT NOT NULL,
    "period" TEXT NOT NULL,
    "tasks_used" INTEGER NOT NULL DEFAULT 0,
    "projects_used" INTEGER NOT NULL DEFAULT 0,
    "repos_used" INTEGER NOT NULL DEFAULT 0,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "usage_counters_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "usage_counters_tenant_id_idx" ON "usage_counters"("tenant_id");

-- CreateIndex
CREATE UNIQUE INDEX "usage_counters_tenant_id_period_key" ON "usage_counters"("tenant_id", "period");

-- AddForeignKey
ALTER TABLE "subscriptions" ADD CONSTRAINT "subscriptions_tenant_id_fkey" FOREIGN KEY ("tenant_id") REFERENCES "tenants"("id") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "invoices" ADD CONSTRAINT "invoices_tenant_id_fkey" FOREIGN KEY ("tenant_id") REFERENCES "tenants"("id") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "invoices" ADD CONSTRAINT "invoices_subscription_id_fkey" FOREIGN KEY ("subscription_id") REFERENCES "subscriptions"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "usage_counters" ADD CONSTRAINT "usage_counters_tenant_id_fkey" FOREIGN KEY ("tenant_id") REFERENCES "tenants"("id") ON DELETE RESTRICT ON UPDATE CASCADE;
