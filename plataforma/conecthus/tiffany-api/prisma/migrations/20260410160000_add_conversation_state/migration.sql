-- CreateEnum
CREATE TYPE "ConversationPhase" AS ENUM ('idle', 'creating_task', 'discussing_project', 'monitoring', 'reviewing', 'diagnosing');

-- CreateTable
CREATE TABLE "conversation_sessions" (
    "id" TEXT NOT NULL,
    "channel" TEXT NOT NULL,
    "target" TEXT NOT NULL,
    "phase" "ConversationPhase" NOT NULL DEFAULT 'idle',
    "active_project_id" TEXT,
    "active_task_id" TEXT,
    "last_user_message" TEXT,
    "last_action_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "metadata" JSONB,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "conversation_sessions_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "conversation_actions" (
    "id" TEXT NOT NULL,
    "session_id" TEXT NOT NULL,
    "action" TEXT NOT NULL,
    "endpoint" TEXT,
    "request_body" JSONB,
    "response_code" INTEGER NOT NULL DEFAULT 200,
    "blocked" BOOLEAN NOT NULL DEFAULT false,
    "block_reason" TEXT,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "conversation_actions_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE UNIQUE INDEX "conversation_sessions_channel_target_key" ON "conversation_sessions"("channel", "target");

-- CreateIndex
CREATE INDEX "conversation_actions_session_id_idx" ON "conversation_actions"("session_id");

-- AddForeignKey
ALTER TABLE "conversation_sessions" ADD CONSTRAINT "conversation_sessions_active_project_id_fkey" FOREIGN KEY ("active_project_id") REFERENCES "projects"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "conversation_sessions" ADD CONSTRAINT "conversation_sessions_active_task_id_fkey" FOREIGN KEY ("active_task_id") REFERENCES "tasks"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "conversation_actions" ADD CONSTRAINT "conversation_actions_session_id_fkey" FOREIGN KEY ("session_id") REFERENCES "conversation_sessions"("id") ON DELETE RESTRICT ON UPDATE CASCADE;
