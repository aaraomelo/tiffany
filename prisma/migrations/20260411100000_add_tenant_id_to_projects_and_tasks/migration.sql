-- AlterTable
ALTER TABLE "projects" ADD COLUMN "tenant_id" TEXT NOT NULL DEFAULT '';

-- AlterTable
ALTER TABLE "tasks" ADD COLUMN "tenant_id" TEXT NOT NULL DEFAULT '';

-- CreateIndex
CREATE INDEX "projects_tenant_id_idx" ON "projects"("tenant_id");

-- CreateIndex
CREATE INDEX "tasks_tenant_id_idx" ON "tasks"("tenant_id");
