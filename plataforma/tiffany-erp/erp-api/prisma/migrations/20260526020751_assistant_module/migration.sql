-- CreateTable
CREATE TABLE "assistant_config" (
    "tenantId" UUID NOT NULL,
    "llmProvider" TEXT NOT NULL DEFAULT 'anthropic',
    "model" TEXT NOT NULL DEFAULT 'claude-haiku-4-5',
    "api_key_encrypted" TEXT,
    "soul_prompt" TEXT,
    "active" BOOLEAN NOT NULL DEFAULT true,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "assistant_config_pkey" PRIMARY KEY ("tenantId")
);

-- CreateTable
CREATE TABLE "assistant_profiles" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "role" "UserRole" NOT NULL,
    "name" TEXT NOT NULL,
    "systemPrompt" TEXT NOT NULL,
    "allowedTools" TEXT[],
    "memoryAccess" TEXT NOT NULL DEFAULT 'own',
    "active" BOOLEAN NOT NULL DEFAULT true,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "assistant_profiles_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "assistant_memories" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "userId" UUID,
    "category" TEXT NOT NULL,
    "priority" TEXT NOT NULL DEFAULT 'long_term',
    "state" TEXT NOT NULL DEFAULT 'active',
    "visibility" TEXT NOT NULL DEFAULT 'private',
    "title" TEXT NOT NULL,
    "content" TEXT NOT NULL,
    "access_count" INTEGER NOT NULL DEFAULT 0,
    "last_accessed_at" TIMESTAMP(3),
    "promoted_at" TIMESTAMP(3),
    "source_model" TEXT,
    "embedding" vector(1024),
    "embedding_model" TEXT,
    "embedded_at" TIMESTAMP(3),
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "assistant_memories_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "assistant_conversations" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "userId" UUID NOT NULL,
    "title" TEXT,
    "channel" TEXT NOT NULL DEFAULT 'web',
    "metadata" JSONB NOT NULL DEFAULT '{}',
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "assistant_conversations_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "assistant_messages" (
    "id" UUID NOT NULL,
    "conversation_id" UUID NOT NULL,
    "role" TEXT NOT NULL,
    "content" TEXT NOT NULL,
    "model" TEXT,
    "tool_calls" JSONB,
    "tool_result" JSONB,
    "embedding" vector(1024),
    "embedded_at" TIMESTAMP(3),
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "assistant_messages_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "assistant_profiles_tenantId_idx" ON "assistant_profiles"("tenantId");

-- CreateIndex
CREATE UNIQUE INDEX "assistant_profiles_tenantId_role_key" ON "assistant_profiles"("tenantId", "role");

-- CreateIndex
CREATE INDEX "assistant_memories_tenantId_idx" ON "assistant_memories"("tenantId");

-- CreateIndex
CREATE INDEX "assistant_memories_tenantId_category_idx" ON "assistant_memories"("tenantId", "category");

-- CreateIndex
CREATE INDEX "assistant_memories_tenantId_userId_idx" ON "assistant_memories"("tenantId", "userId");

-- CreateIndex
CREATE INDEX "assistant_memories_priority_idx" ON "assistant_memories"("priority");

-- CreateIndex
CREATE INDEX "assistant_memories_state_idx" ON "assistant_memories"("state");

-- CreateIndex
CREATE INDEX "assistant_conversations_tenantId_userId_updated_at_idx" ON "assistant_conversations"("tenantId", "userId", "updated_at");

-- CreateIndex
CREATE INDEX "assistant_messages_conversation_id_created_at_idx" ON "assistant_messages"("conversation_id", "created_at");

-- AddForeignKey
ALTER TABLE "assistant_config" ADD CONSTRAINT "assistant_config_tenantId_fkey" FOREIGN KEY ("tenantId") REFERENCES "Tenant"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "assistant_profiles" ADD CONSTRAINT "assistant_profiles_tenantId_fkey" FOREIGN KEY ("tenantId") REFERENCES "Tenant"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "assistant_messages" ADD CONSTRAINT "assistant_messages_conversation_id_fkey" FOREIGN KEY ("conversation_id") REFERENCES "assistant_conversations"("id") ON DELETE CASCADE ON UPDATE CASCADE;
