-- CreateTable
CREATE TABLE "TenantPack" (
    "tenantId" UUID NOT NULL,
    "packSlug" TEXT NOT NULL,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "TenantPack_pkey" PRIMARY KEY ("tenantId","packSlug")
);

-- CreateIndex
CREATE INDEX "TenantPack_tenantId_idx" ON "TenantPack"("tenantId");

-- AddForeignKey
ALTER TABLE "TenantPack" ADD CONSTRAINT "TenantPack_tenantId_fkey" FOREIGN KEY ("tenantId") REFERENCES "Tenant"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "TenantPack" ADD CONSTRAINT "TenantPack_packSlug_fkey" FOREIGN KEY ("packSlug") REFERENCES "ModulePack"("slug") ON DELETE CASCADE ON UPDATE CASCADE;
