-- person_messages: adiciona embedding pra busca semântica cross-channel
ALTER TABLE "person_messages" ADD COLUMN "embedding" vector(768);
ALTER TABLE "person_messages" ADD COLUMN "embedded_at" TIMESTAMP(3);

CREATE INDEX "person_messages_embedded_at_idx" ON "person_messages" ("embedded_at");
CREATE INDEX "person_messages_embedding_idx" ON "person_messages"
    USING ivfflat ("embedding" vector_cosine_ops) WITH (lists = 30);
