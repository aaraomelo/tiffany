CREATE TABLE "specialist_sessions" (
    "id" TEXT NOT NULL,
    "channel" TEXT NOT NULL,
    "target" TEXT NOT NULL,
    "repo" TEXT NOT NULL DEFAULT 'patria-api',
    "claude_session_id" TEXT,
    "status" TEXT NOT NULL DEFAULT 'active',
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "closed_at" TIMESTAMP(3),
    CONSTRAINT "specialist_sessions_pkey" PRIMARY KEY ("id")
);

CREATE TABLE "specialist_messages" (
    "id" TEXT NOT NULL,
    "session_ref" TEXT NOT NULL,
    "role" TEXT NOT NULL,
    "content" TEXT NOT NULL,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT "specialist_messages_pkey" PRIMARY KEY ("id")
);

CREATE INDEX "specialist_messages_session_ref_idx" ON "specialist_messages"("session_ref");
CREATE UNIQUE INDEX "specialist_sessions_channel_target_status_key" ON "specialist_sessions"("channel", "target", "status");
