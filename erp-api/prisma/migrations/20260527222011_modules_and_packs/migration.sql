-- CreateEnum
CREATE TYPE "ModuleCategory" AS ENUM ('CORE', 'SALES', 'SERVICE', 'FOOD', 'EVENT', 'FISCAL', 'AI');

-- AlterTable
ALTER TABLE "Tenant" ADD COLUMN     "packSlug" TEXT;

-- CreateTable
CREATE TABLE "Module" (
    "slug" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "description" TEXT,
    "category" "ModuleCategory" NOT NULL,
    "routePath" TEXT,
    "iconKey" TEXT,
    "isCore" BOOLEAN NOT NULL DEFAULT false,
    "sortOrder" INTEGER NOT NULL DEFAULT 0,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "Module_pkey" PRIMARY KEY ("slug")
);

-- CreateTable
CREATE TABLE "ModulePack" (
    "slug" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "segment" TEXT NOT NULL,
    "description" TEXT,
    "isDefault" BOOLEAN NOT NULL DEFAULT false,
    "sortOrder" INTEGER NOT NULL DEFAULT 0,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "ModulePack_pkey" PRIMARY KEY ("slug")
);

-- CreateTable
CREATE TABLE "ModulePackItem" (
    "packSlug" TEXT NOT NULL,
    "moduleSlug" TEXT NOT NULL,

    CONSTRAINT "ModulePackItem_pkey" PRIMARY KEY ("packSlug","moduleSlug")
);

-- CreateTable
CREATE TABLE "TenantModule" (
    "tenantId" UUID NOT NULL,
    "moduleSlug" TEXT NOT NULL,
    "enabled" BOOLEAN NOT NULL DEFAULT true,
    "enabledAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "disabledAt" TIMESTAMP(3),

    CONSTRAINT "TenantModule_pkey" PRIMARY KEY ("tenantId","moduleSlug")
);

-- CreateIndex
CREATE INDEX "Module_category_sortOrder_idx" ON "Module"("category", "sortOrder");

-- CreateIndex
CREATE INDEX "TenantModule_tenantId_enabled_idx" ON "TenantModule"("tenantId", "enabled");

-- AddForeignKey
ALTER TABLE "Tenant" ADD CONSTRAINT "Tenant_packSlug_fkey" FOREIGN KEY ("packSlug") REFERENCES "ModulePack"("slug") ON DELETE SET NULL ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "ModulePackItem" ADD CONSTRAINT "ModulePackItem_packSlug_fkey" FOREIGN KEY ("packSlug") REFERENCES "ModulePack"("slug") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "ModulePackItem" ADD CONSTRAINT "ModulePackItem_moduleSlug_fkey" FOREIGN KEY ("moduleSlug") REFERENCES "Module"("slug") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "TenantModule" ADD CONSTRAINT "TenantModule_tenantId_fkey" FOREIGN KEY ("tenantId") REFERENCES "Tenant"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "TenantModule" ADD CONSTRAINT "TenantModule_moduleSlug_fkey" FOREIGN KEY ("moduleSlug") REFERENCES "Module"("slug") ON DELETE RESTRICT ON UPDATE CASCADE;
