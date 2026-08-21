-- CreateEnum
CREATE TYPE "SupplierPlatform" AS ENUM ('NUVEMSHOP');

-- CreateEnum
CREATE TYPE "SupplierOrderStatus" AS ENUM ('DRAFT', 'DRY_RUN', 'SUBMITTED', 'CONFIRMED', 'FAILED');

-- CreateTable
CREATE TABLE "SupplierAccount" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "platform" "SupplierPlatform" NOT NULL DEFAULT 'NUVEMSHOP',
    "baseUrl" TEXT NOT NULL,
    "label" TEXT,
    "loginEmail" TEXT NOT NULL,
    "passwordEnc" TEXT NOT NULL,
    "sessionCookieEnc" TEXT,
    "sessionExpiresAt" TIMESTAMP(3),
    "customerSupplierId" UUID,
    "defaultMarkupPct" DECIMAL(9,4),
    "active" BOOLEAN NOT NULL DEFAULT true,
    "lastSyncAt" TIMESTAMP(3),
    "lastError" TEXT,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "SupplierAccount_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "SupplierProduct" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "supplierAccountId" UUID NOT NULL,
    "externalProductId" TEXT NOT NULL,
    "externalVariantId" TEXT NOT NULL,
    "productUrl" TEXT,
    "sku" TEXT,
    "gtin" TEXT,
    "name" TEXT NOT NULL,
    "option0" TEXT,
    "option1" TEXT,
    "option2" TEXT,
    "price" DECIMAL(18,4) NOT NULL,
    "pixPrice" DECIMAL(18,4),
    "stock" INTEGER,
    "available" BOOLEAN NOT NULL DEFAULT true,
    "imageUrl" TEXT,
    "linkedProductId" UUID,
    "embedding" vector(1024),
    "embedding_model" TEXT,
    "embedded_at" TIMESTAMP(3),
    "syncedAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "SupplierProduct_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "SupplierOrder" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "supplierAccountId" UUID NOT NULL,
    "status" "SupplierOrderStatus" NOT NULL DEFAULT 'DRY_RUN',
    "dryRun" BOOLEAN NOT NULL DEFAULT true,
    "totalAmount" DECIMAL(18,4) NOT NULL DEFAULT 0,
    "externalOrderRef" TEXT,
    "requestPayload" JSONB,
    "responsePayload" JSONB,
    "error" TEXT,
    "placedByUserId" UUID,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "SupplierOrder_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "SupplierOrderItem" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "supplierOrderId" UUID NOT NULL,
    "supplierProductId" UUID,
    "externalVariantId" TEXT NOT NULL,
    "qty" INTEGER NOT NULL,
    "unitPrice" DECIMAL(18,4) NOT NULL,
    "lineTotal" DECIMAL(18,4) NOT NULL,

    CONSTRAINT "SupplierOrderItem_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "SupplierAccount_tenantId_active_idx" ON "SupplierAccount"("tenantId", "active");

-- CreateIndex
CREATE UNIQUE INDEX "SupplierAccount_tenantId_platform_baseUrl_key" ON "SupplierAccount"("tenantId", "platform", "baseUrl");

-- CreateIndex
CREATE INDEX "SupplierProduct_tenantId_supplierAccountId_idx" ON "SupplierProduct"("tenantId", "supplierAccountId");

-- CreateIndex
CREATE INDEX "SupplierProduct_tenantId_sku_idx" ON "SupplierProduct"("tenantId", "sku");

-- CreateIndex
CREATE INDEX "SupplierProduct_tenantId_gtin_idx" ON "SupplierProduct"("tenantId", "gtin");

-- CreateIndex
CREATE UNIQUE INDEX "SupplierProduct_tenantId_supplierAccountId_externalVariantI_key" ON "SupplierProduct"("tenantId", "supplierAccountId", "externalVariantId");

-- CreateIndex
CREATE INDEX "SupplierOrder_tenantId_supplierAccountId_createdAt_idx" ON "SupplierOrder"("tenantId", "supplierAccountId", "createdAt");

-- CreateIndex
CREATE INDEX "SupplierOrderItem_tenantId_supplierOrderId_idx" ON "SupplierOrderItem"("tenantId", "supplierOrderId");

-- AddForeignKey
ALTER TABLE "SupplierAccount" ADD CONSTRAINT "SupplierAccount_tenantId_fkey" FOREIGN KEY ("tenantId") REFERENCES "Tenant"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "SupplierAccount" ADD CONSTRAINT "SupplierAccount_customerSupplierId_fkey" FOREIGN KEY ("customerSupplierId") REFERENCES "CustomerSupplier"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "SupplierProduct" ADD CONSTRAINT "SupplierProduct_supplierAccountId_fkey" FOREIGN KEY ("supplierAccountId") REFERENCES "SupplierAccount"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "SupplierProduct" ADD CONSTRAINT "SupplierProduct_linkedProductId_fkey" FOREIGN KEY ("linkedProductId") REFERENCES "Product"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "SupplierOrder" ADD CONSTRAINT "SupplierOrder_supplierAccountId_fkey" FOREIGN KEY ("supplierAccountId") REFERENCES "SupplierAccount"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "SupplierOrderItem" ADD CONSTRAINT "SupplierOrderItem_supplierOrderId_fkey" FOREIGN KEY ("supplierOrderId") REFERENCES "SupplierOrder"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- Row-Level Security NATIVA pras tabelas novas (mesma policy de 20260601070000_native_rls).
-- Tabelas criadas DEPOIS daquela migration não herdam a policy → aplicamos aqui.
-- Inerte enquanto o app conecta como superusuário; vale quando conectar como erp_app.
DO $rls$
DECLARE
  t text;
BEGIN
  FOR t IN
    SELECT unnest(ARRAY['SupplierAccount', 'SupplierProduct', 'SupplierOrder', 'SupplierOrderItem'])
  LOOP
    EXECUTE format('ALTER TABLE %I ENABLE ROW LEVEL SECURITY', t);
    EXECUTE format('ALTER TABLE %I FORCE ROW LEVEL SECURITY', t);
    EXECUTE format('DROP POLICY IF EXISTS tenant_isolation ON %I', t);
    EXECUTE format(
      'CREATE POLICY tenant_isolation ON %I'
      || ' USING ("tenantId" = NULLIF(current_setting(''app.tenant_id'', true), '''')::uuid'
      || '        OR current_setting(''app.bypass_rls'', true) = ''on'')'
      || ' WITH CHECK ("tenantId" = NULLIF(current_setting(''app.tenant_id'', true), '''')::uuid'
      || '        OR current_setting(''app.bypass_rls'', true) = ''on'')',
      t);
  END LOOP;
END
$rls$;
