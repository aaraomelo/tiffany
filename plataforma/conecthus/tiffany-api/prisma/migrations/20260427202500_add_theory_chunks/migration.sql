-- Theory chunks: .tex pieces with embeddings for semantic search

CREATE TABLE "theory_chunks" (
    "id" BIGSERIAL NOT NULL,
    "file" TEXT NOT NULL,
    "section" TEXT,
    "chunk_idx" INTEGER NOT NULL,
    "content" TEXT NOT NULL,
    "content_hash" TEXT NOT NULL,
    "char_count" INTEGER NOT NULL,
    "embedding" vector(768),
    "embedded_at" TIMESTAMP(3),
    "updated_at" TIMESTAMP(3) NOT NULL,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "theory_chunks_pkey" PRIMARY KEY ("id")
);

CREATE UNIQUE INDEX "theory_chunks_file_chunk_idx_key" ON "theory_chunks" ("file", "chunk_idx");
CREATE INDEX "theory_chunks_file_idx" ON "theory_chunks" ("file");
CREATE INDEX "theory_chunks_embedded_at_idx" ON "theory_chunks" ("embedded_at");
CREATE INDEX "theory_chunks_content_hash_idx" ON "theory_chunks" ("content_hash");
CREATE INDEX "theory_chunks_pending_idx" ON "theory_chunks" ("id") WHERE "embedded_at" IS NULL;
CREATE INDEX "theory_chunks_embedding_idx" ON "theory_chunks"
    USING ivfflat ("embedding" vector_cosine_ops) WITH (lists = 50);
