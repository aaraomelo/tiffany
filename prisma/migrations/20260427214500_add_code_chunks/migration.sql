-- Code chunks: GEX44 source pieces with embedding

CREATE TABLE "code_chunks" (
    "id" BIGSERIAL NOT NULL,
    "file" TEXT NOT NULL,
    "symbol" TEXT,
    "kind" TEXT NOT NULL,
    "start_line" INTEGER NOT NULL,
    "end_line" INTEGER NOT NULL,
    "content" TEXT NOT NULL,
    "content_hash" TEXT NOT NULL,
    "char_count" INTEGER NOT NULL,
    "embedding" vector(768),
    "embedded_at" TIMESTAMP(3),
    "updated_at" TIMESTAMP(3) NOT NULL,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "code_chunks_pkey" PRIMARY KEY ("id")
);

CREATE UNIQUE INDEX "code_chunks_file_start_line_key" ON "code_chunks" ("file", "start_line");
CREATE INDEX "code_chunks_file_idx" ON "code_chunks" ("file");
CREATE INDEX "code_chunks_symbol_idx" ON "code_chunks" ("symbol");
CREATE INDEX "code_chunks_embedded_at_idx" ON "code_chunks" ("embedded_at");
CREATE INDEX "code_chunks_content_hash_idx" ON "code_chunks" ("content_hash");
CREATE INDEX "code_chunks_pending_idx" ON "code_chunks" ("id") WHERE "embedded_at" IS NULL;
CREATE INDEX "code_chunks_embedding_idx" ON "code_chunks"
    USING ivfflat ("embedding" vector_cosine_ops) WITH (lists = 30);
