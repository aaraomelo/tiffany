-- Memory state machine: add state, access tracking, promotion

-- State: active, archived, expired (default: active)
ALTER TABLE "patricia_memories" ADD COLUMN "state" TEXT NOT NULL DEFAULT 'active';

-- Access tracking
ALTER TABLE "patricia_memories" ADD COLUMN "access_count" INTEGER NOT NULL DEFAULT 0;
ALTER TABLE "patricia_memories" ADD COLUMN "last_accessed_at" TIMESTAMP(3);

-- Promotion threshold (auto-promote short_term → long_term after N accesses)
-- Default: 5 accesses promotes short_term to long_term
ALTER TABLE "patricia_memories" ADD COLUMN "promoted_at" TIMESTAMP(3);

-- Indexes
CREATE INDEX "patricia_memories_state_idx" ON "patricia_memories"("state");
CREATE INDEX "patricia_memories_access_count_idx" ON "patricia_memories"("access_count" DESC);

-- Set all existing memories to active
UPDATE "patricia_memories" SET state = 'active';
