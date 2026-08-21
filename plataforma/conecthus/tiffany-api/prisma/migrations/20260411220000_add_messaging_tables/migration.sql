-- CreateEnum
CREATE TYPE "ChannelType" AS ENUM ('whatsapp', 'telegram', 'web');
CREATE TYPE "MessageDirection" AS ENUM ('inbound', 'outbound');

-- CreateTable
CREATE TABLE "messaging_contacts" (
    "id" TEXT NOT NULL,
    "channel_type" "ChannelType" NOT NULL,
    "remote_id" TEXT NOT NULL,
    "display_name" TEXT,
    "phone" TEXT,
    "is_group" BOOLEAN NOT NULL DEFAULT false,
    "tenant_id" TEXT NOT NULL DEFAULT 'patria',
    "metadata" JSONB,
    "last_seen_at" TIMESTAMP(3),
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "messaging_contacts_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "message_logs" (
    "id" TEXT NOT NULL,
    "contact_id" TEXT NOT NULL,
    "direction" "MessageDirection" NOT NULL,
    "content" TEXT NOT NULL,
    "message_id" TEXT,
    "status" TEXT NOT NULL DEFAULT 'pending',
    "error_message" TEXT,
    "metadata" JSONB,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "message_logs_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE UNIQUE INDEX "messaging_contacts_channel_type_remote_id_key" ON "messaging_contacts"("channel_type", "remote_id");
CREATE INDEX "messaging_contacts_tenant_id_idx" ON "messaging_contacts"("tenant_id");
CREATE INDEX "messaging_contacts_phone_idx" ON "messaging_contacts"("phone");
CREATE INDEX "message_logs_contact_id_idx" ON "message_logs"("contact_id");
CREATE INDEX "message_logs_created_at_idx" ON "message_logs"("created_at");

-- AddForeignKey
ALTER TABLE "message_logs" ADD CONSTRAINT "message_logs_contact_id_fkey" FOREIGN KEY ("contact_id") REFERENCES "messaging_contacts"("id") ON DELETE RESTRICT ON UPDATE CASCADE;
