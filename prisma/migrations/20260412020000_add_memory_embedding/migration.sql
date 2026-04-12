-- Enable pgvector extension (if not already enabled)
CREATE EXTENSION IF NOT EXISTS vector;

-- Add embedding column
ALTER TABLE "patricia_memories" ADD COLUMN "embedding" vector(768);

-- Create index for cosine similarity search
CREATE INDEX "patricia_memories_embedding_idx" ON "patricia_memories" USING ivfflat ("embedding" vector_cosine_ops) WITH (lists = 10);
