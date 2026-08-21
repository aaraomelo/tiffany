-- CreateEnum
CREATE TYPE "TenantPlan" AS ENUM ('starter', 'pro', 'enterprise');

-- CreateEnum
CREATE TYPE "TenantStatus" AS ENUM ('trial', 'active', 'suspended');

-- CreateTable
CREATE TABLE "tenants" (
    "id" TEXT NOT NULL,
    "alias" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "company_name" TEXT NOT NULL,
    "plan" "TenantPlan" NOT NULL DEFAULT 'starter',
    "status" "TenantStatus" NOT NULL DEFAULT 'trial',
    "github_org" TEXT,
    "github_repos" TEXT[],
    "trial_ends_at" TIMESTAMP(3),
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "tenants_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "tenant_users" (
    "id" TEXT NOT NULL,
    "tenant_id" TEXT NOT NULL,
    "email" TEXT NOT NULL,
    "password_hash" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "role" TEXT NOT NULL DEFAULT 'member',
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "tenant_users_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE UNIQUE INDEX "tenants_alias_key" ON "tenants"("alias");

-- CreateIndex
CREATE INDEX "tenant_users_tenant_id_idx" ON "tenant_users"("tenant_id");

-- CreateIndex
CREATE UNIQUE INDEX "tenant_users_tenant_id_email_key" ON "tenant_users"("tenant_id", "email");

-- AddForeignKey
ALTER TABLE "tenant_users" ADD CONSTRAINT "tenant_users_tenant_id_fkey" FOREIGN KEY ("tenant_id") REFERENCES "tenants"("id") ON DELETE RESTRICT ON UPDATE CASCADE;
