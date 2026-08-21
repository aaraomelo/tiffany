-- Dados de cabeçalho de documento da empresa (Ordem de Serviço, orçamento,
-- recibo...). O Tenant não tinha endereço/contato/pagamento — gap do cabeçalho.
-- Tenant NÃO tem coluna tenantId (ele é o próprio tenant) → não recebe RLS.

-- AlterTable
ALTER TABLE "Tenant"
    ADD COLUMN "responsible" TEXT,
    ADD COLUMN "email" TEXT,
    ADD COLUMN "phone" TEXT,
    ADD COLUMN "phone2" TEXT,
    ADD COLUMN "instagram" TEXT,
    ADD COLUMN "logoUrl" TEXT,
    ADD COLUMN "paymentMethods" TEXT,
    ADD COLUMN "paymentTerms" TEXT,
    ADD COLUMN "addressId" UUID;

-- AddForeignKey: reaproveita o model Address compartilhado (rua, nº, bairro,
-- cidade/UF via City, CEP). Address não tem tenantId (estrutura compartilhada).
ALTER TABLE "Tenant" ADD CONSTRAINT "Tenant_addressId_fkey" FOREIGN KEY ("addressId") REFERENCES "Address"("id") ON DELETE SET NULL ON UPDATE CASCADE;
