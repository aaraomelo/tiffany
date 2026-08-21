-- AlterTable: galeria cacheada no SupplierProduct (mesma p/ todas as variações do produto)
ALTER TABLE "SupplierProduct" ADD COLUMN "images" JSONB;

-- CreateTable: galeria de fotos do Product (e-commerce)
CREATE TABLE "ProductImage" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "productId" UUID NOT NULL,
    "url" TEXT NOT NULL,
    "position" INTEGER NOT NULL DEFAULT 0,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "ProductImage_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "ProductImage_tenantId_productId_idx" ON "ProductImage"("tenantId", "productId");

-- CreateIndex
CREATE UNIQUE INDEX "ProductImage_tenantId_productId_url_key" ON "ProductImage"("tenantId", "productId", "url");

-- AddForeignKey
ALTER TABLE "ProductImage" ADD CONSTRAINT "ProductImage_productId_fkey" FOREIGN KEY ("productId") REFERENCES "Product"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- Row-Level Security NATIVA pra ProductImage (mesma policy de 20260601070000_native_rls).
DO $rls$
DECLARE
  t text;
BEGIN
  FOR t IN SELECT unnest(ARRAY['ProductImage'])
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
