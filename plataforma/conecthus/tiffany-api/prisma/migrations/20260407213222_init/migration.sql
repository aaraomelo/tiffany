-- CreateEnum
CREATE TYPE "TaskStatus" AS ENUM ('pending', 'planning', 'needs_info', 'awaiting_approval', 'approved', 'replanning', 'executing', 'deploying', 'completed', 'failed', 'rejected', 'cancelled', 'timed_out');

-- CreateEnum
CREATE TYPE "ExecutionPhase" AS ENUM ('planning', 'replanning', 'execution');

-- CreateTable
CREATE TABLE "tasks" (
    "id" TEXT NOT NULL,
    "status" "TaskStatus" NOT NULL DEFAULT 'pending',
    "command" TEXT NOT NULL,
    "description" TEXT,
    "project" TEXT,
    "created_by" TEXT NOT NULL DEFAULT 'patricia',
    "channel" TEXT NOT NULL DEFAULT 'whatsapp',
    "target" TEXT NOT NULL DEFAULT '+5511977808883',
    "template_id" TEXT,
    "replan_count" INTEGER NOT NULL DEFAULT 0,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "tasks_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "task_executions" (
    "id" TEXT NOT NULL,
    "task_id" TEXT NOT NULL,
    "phase" "ExecutionPhase" NOT NULL,
    "plan" TEXT,
    "question" TEXT,
    "feedback" TEXT,
    "result" TEXT,
    "commit_sha" TEXT,
    "started_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "finished_at" TIMESTAMP(3),

    CONSTRAINT "task_executions_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "task_transitions" (
    "id" TEXT NOT NULL,
    "task_id" TEXT NOT NULL,
    "from_status" TEXT NOT NULL,
    "to_status" TEXT NOT NULL,
    "actor" TEXT NOT NULL,
    "metadata" JSONB,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "task_transitions_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "task_templates" (
    "id" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "slug" TEXT NOT NULL,
    "command_template" TEXT NOT NULL,
    "description_template" TEXT,
    "project" TEXT,
    "fields" JSONB NOT NULL DEFAULT '[]',
    "created_by" TEXT NOT NULL,
    "is_active" BOOLEAN NOT NULL DEFAULT true,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "task_templates_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "tasks_status_idx" ON "tasks"("status");

-- CreateIndex
CREATE INDEX "tasks_status_updated_at_idx" ON "tasks"("status", "updated_at");

-- CreateIndex
CREATE INDEX "task_executions_task_id_idx" ON "task_executions"("task_id");

-- CreateIndex
CREATE INDEX "task_transitions_task_id_idx" ON "task_transitions"("task_id");

-- CreateIndex
CREATE UNIQUE INDEX "task_templates_slug_key" ON "task_templates"("slug");

-- CreateIndex
CREATE INDEX "task_templates_slug_idx" ON "task_templates"("slug");

-- AddForeignKey
ALTER TABLE "tasks" ADD CONSTRAINT "tasks_template_id_fkey" FOREIGN KEY ("template_id") REFERENCES "task_templates"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "task_executions" ADD CONSTRAINT "task_executions_task_id_fkey" FOREIGN KEY ("task_id") REFERENCES "tasks"("id") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "task_transitions" ADD CONSTRAINT "task_transitions_task_id_fkey" FOREIGN KEY ("task_id") REFERENCES "tasks"("id") ON DELETE RESTRICT ON UPDATE CASCADE;
