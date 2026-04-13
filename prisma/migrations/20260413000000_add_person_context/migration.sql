-- Centralized person context: one record per person with all info
ALTER TABLE "people" ADD COLUMN "context" JSONB DEFAULT '{}';
-- context stores: { lastChannel, lastMessageAt, channels: [...], conversationSummary, ... }

-- Conversation messages: centralized per person (not per session)
CREATE TABLE "person_messages" (
    "id" TEXT NOT NULL,
    "person_id" TEXT NOT NULL,
    "channel" TEXT NOT NULL,
    "role" TEXT NOT NULL,
    "content" TEXT NOT NULL,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "person_messages_pkey" PRIMARY KEY ("id")
);

CREATE INDEX "person_messages_person_id_idx" ON "person_messages"("person_id");
CREATE INDEX "person_messages_created_at_idx" ON "person_messages"("created_at");

ALTER TABLE "person_messages" ADD CONSTRAINT "person_messages_person_id_fkey"
    FOREIGN KEY ("person_id") REFERENCES "people"("id") ON DELETE CASCADE ON UPDATE CASCADE;
