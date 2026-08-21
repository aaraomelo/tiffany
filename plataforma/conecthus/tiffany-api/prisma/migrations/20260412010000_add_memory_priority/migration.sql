-- Add priority column
ALTER TABLE "patricia_memories" ADD COLUMN "priority" TEXT NOT NULL DEFAULT 'long_term';

-- Create index
CREATE INDEX "patricia_memories_priority_idx" ON "patricia_memories"("priority");

-- Reclassify existing memories
UPDATE "patricia_memories" SET priority = 'core' WHERE category IN ('product', 'person');
UPDATE "patricia_memories" SET priority = 'long_term' WHERE category IN ('decision', 'technical');
