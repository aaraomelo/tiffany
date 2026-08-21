-- Add description and embedding to people
ALTER TABLE "people" ADD COLUMN "description" TEXT;
ALTER TABLE "people" ADD COLUMN "embedding" vector(768);

-- Index for semantic search
CREATE INDEX "people_embedding_idx" ON "people" USING ivfflat ("embedding" vector_cosine_ops) WITH (lists = 10);
