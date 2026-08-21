-- CreateEnum
CREATE TYPE "ProjectStatus" AS ENUM ('planning', 'awaiting_review', 'approved', 'executing', 'paused', 'completed', 'failed', 'cancelled');

-- AlterTable
ALTER TABLE "tasks" ADD COLUMN     "depends_on_id" TEXT,
ADD COLUMN     "project_id" TEXT,
ADD COLUMN     "sort_order" INTEGER NOT NULL DEFAULT 0;

-- CreateTable
CREATE TABLE "projects" (
    "id" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "description" TEXT,
    "status" "ProjectStatus" NOT NULL DEFAULT 'planning',
    "auto_approve" BOOLEAN NOT NULL DEFAULT false,
    "created_by" TEXT NOT NULL DEFAULT 'patricia',
    "channel" TEXT NOT NULL DEFAULT 'whatsapp',
    "target" TEXT NOT NULL DEFAULT '+5511977808883',
    "total_subtasks" INTEGER NOT NULL DEFAULT 0,
    "done_subtasks" INTEGER NOT NULL DEFAULT 0,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "projects_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "project_plannings" (
    "id" TEXT NOT NULL,
    "project_id" TEXT NOT NULL,
    "role" TEXT NOT NULL,
    "content" TEXT NOT NULL,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "project_plannings_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "projects_status_idx" ON "projects"("status");

-- CreateIndex
CREATE INDEX "project_plannings_project_id_idx" ON "project_plannings"("project_id");

-- CreateIndex
CREATE INDEX "tasks_project_id_sort_order_idx" ON "tasks"("project_id", "sort_order");

-- AddForeignKey
ALTER TABLE "project_plannings" ADD CONSTRAINT "project_plannings_project_id_fkey" FOREIGN KEY ("project_id") REFERENCES "projects"("id") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "tasks" ADD CONSTRAINT "tasks_project_id_fkey" FOREIGN KEY ("project_id") REFERENCES "projects"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "tasks" ADD CONSTRAINT "tasks_depends_on_id_fkey" FOREIGN KEY ("depends_on_id") REFERENCES "tasks"("id") ON DELETE SET NULL ON UPDATE CASCADE;
