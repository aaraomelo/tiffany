-- Aluno: filiação (pai/mãe), documentos e foto.
-- Pedido do diretor da escola (Vinicius): endereço completo já existia em colunas
-- inline; aqui adicionamos nome+documento+telefone de pai e mãe, e a foto do aluno
-- (binário em tabela própria StudentPhoto pra não inchar list/findOne com bytea).

-- AlterTable
ALTER TABLE "Student" ADD COLUMN     "fatherDocument" TEXT,
ADD COLUMN     "fatherName" TEXT,
ADD COLUMN     "fatherPhone" TEXT,
ADD COLUMN     "motherDocument" TEXT,
ADD COLUMN     "motherName" TEXT,
ADD COLUMN     "motherPhone" TEXT,
ADD COLUMN     "photoUpdatedAt" TIMESTAMP(3);

-- CreateTable
CREATE TABLE "StudentPhoto" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "studentId" UUID NOT NULL,
    "data" BYTEA NOT NULL,
    "mimeType" TEXT NOT NULL,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "StudentPhoto_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE UNIQUE INDEX "StudentPhoto_studentId_key" ON "StudentPhoto"("studentId");

-- CreateIndex
CREATE INDEX "StudentPhoto_tenantId_idx" ON "StudentPhoto"("tenantId");

-- AddForeignKey
ALTER TABLE "StudentPhoto" ADD CONSTRAINT "StudentPhoto_tenantId_fkey" FOREIGN KEY ("tenantId") REFERENCES "Tenant"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "StudentPhoto" ADD CONSTRAINT "StudentPhoto_studentId_fkey" FOREIGN KEY ("studentId") REFERENCES "Student"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- Row-Level Security NATIVA na nova tabela (StudentPhoto tem tenantId).
-- Mesma policy de tenant_isolation aplicada em 20260601070000_native_rls — o loop
-- de lá só processou as tabelas existentes na época, então replicamos aqui pra
-- StudentPhoto não ficar fora do piso de isolamento quando o app rodar como erp_app.
ALTER TABLE "StudentPhoto" ENABLE ROW LEVEL SECURITY;
ALTER TABLE "StudentPhoto" FORCE ROW LEVEL SECURITY;
DROP POLICY IF EXISTS tenant_isolation ON "StudentPhoto";
CREATE POLICY tenant_isolation ON "StudentPhoto"
  USING ("tenantId" = NULLIF(current_setting('app.tenant_id', true), '')::uuid
         OR current_setting('app.bypass_rls', true) = 'on')
  WITH CHECK ("tenantId" = NULLIF(current_setting('app.tenant_id', true), '')::uuid
         OR current_setting('app.bypass_rls', true) = 'on');
