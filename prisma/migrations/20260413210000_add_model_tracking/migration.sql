-- Add model tracking to person_messages and patricia_memories
ALTER TABLE "person_messages" ADD COLUMN "model" TEXT;
ALTER TABLE "patricia_memories" ADD COLUMN "source_model" TEXT;
