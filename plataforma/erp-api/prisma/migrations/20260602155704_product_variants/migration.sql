-- AlterTable
ALTER TABLE "Product" ADD COLUMN     "hasVariants" BOOLEAN NOT NULL DEFAULT false;

-- AlterTable
ALTER TABLE "SupplierProduct" ADD COLUMN     "linkedVariantId" UUID;

-- CreateTable
CREATE TABLE "ProductVariant" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "productId" UUID NOT NULL,
    "sku" TEXT NOT NULL,
    "gtin" TEXT,
    "option0" TEXT,
    "option1" TEXT,
    "option2" TEXT,
    "costPrice" DECIMAL(18,4),
    "salePrice" DECIMAL(18,4),
    "photoUrl" TEXT,
    "active" BOOLEAN NOT NULL DEFAULT true,
    "position" INTEGER NOT NULL DEFAULT 0,
    "embedding" vector(1024),
    "embedding_model" TEXT,
    "embedded_at" TIMESTAMP(3),
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,
    "deletedAt" TIMESTAMP(3),

    CONSTRAINT "ProductVariant_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "ProductVariant_tenantId_productId_idx" ON "ProductVariant"("tenantId", "productId");

-- CreateIndex
CREATE INDEX "ProductVariant_tenantId_gtin_idx" ON "ProductVariant"("tenantId", "gtin");

-- CreateIndex
CREATE UNIQUE INDEX "ProductVariant_tenantId_sku_key" ON "ProductVariant"("tenantId", "sku");

-- AddForeignKey
ALTER TABLE "ProductVariant" ADD CONSTRAINT "ProductVariant_productId_fkey" FOREIGN KEY ("productId") REFERENCES "Product"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "SupplierProduct" ADD CONSTRAINT "SupplierProduct_linkedVariantId_fkey" FOREIGN KEY ("linkedVariantId") REFERENCES "ProductVariant"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- Row-Level Security NATIVA pra ProductVariant (mesma policy de 20260601070000_native_rls).
-- Tabela criada depois daquela migration não herda a policy → aplicamos aqui.
DO $rls$
DECLARE
  t text;
BEGIN
  FOR t IN SELECT unnest(ARRAY['ProductVariant'])
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
