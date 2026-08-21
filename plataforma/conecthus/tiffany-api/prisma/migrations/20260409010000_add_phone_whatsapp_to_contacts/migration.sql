-- AlterTable
ALTER TABLE "contacts" ADD COLUMN "phone" TEXT;
ALTER TABLE "contacts" ADD COLUMN "whatsapp_consent" BOOLEAN NOT NULL DEFAULT false;
