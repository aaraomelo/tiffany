CREATE TABLE "outbound_emails" (
    "id" SERIAL NOT NULL,
    "to_email" TEXT NOT NULL,
    "to_name" TEXT,
    "to_company" TEXT,
    "subject" TEXT NOT NULL,
    "body" TEXT NOT NULL,
    "status" TEXT NOT NULL DEFAULT 'draft',
    "campaign_tag" TEXT,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "sent_at" TIMESTAMP(3),
    "replied_at" TIMESTAMP(3),
    "error_msg" TEXT,
    "metadata" JSONB,

    CONSTRAINT "outbound_emails_pkey" PRIMARY KEY ("id")
);

CREATE INDEX "outbound_emails_status_idx" ON "outbound_emails"("status");
CREATE INDEX "outbound_emails_campaign_tag_idx" ON "outbound_emails"("campaign_tag");
CREATE INDEX "outbound_emails_to_email_idx" ON "outbound_emails"("to_email");
