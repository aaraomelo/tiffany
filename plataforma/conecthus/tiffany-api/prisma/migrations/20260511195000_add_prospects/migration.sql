CREATE TABLE "prospects" (
    "id" SERIAL NOT NULL,
    "full_name" TEXT NOT NULL,
    "title" TEXT,
    "company" TEXT,
    "primary_email" TEXT,
    "emails" TEXT[],
    "phones" TEXT[],
    "location" TEXT,
    "linkedin_url" TEXT,
    "previous_roles" JSONB,
    "source" TEXT,
    "metadata" JSONB,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,
    CONSTRAINT "prospects_pkey" PRIMARY KEY ("id")
);
CREATE INDEX "prospects_company_idx" ON "prospects"("company");
CREATE INDEX "prospects_primary_email_idx" ON "prospects"("primary_email");
