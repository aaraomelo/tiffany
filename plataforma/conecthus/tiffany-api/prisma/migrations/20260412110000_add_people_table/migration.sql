-- People: unified identity across channels
CREATE TABLE "people" (
    "id" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "role" TEXT NOT NULL DEFAULT 'member',
    "email" TEXT,
    "phone" TEXT,
    "tenant_id" TEXT NOT NULL DEFAULT 'patria',
    "metadata" JSONB,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "people_pkey" PRIMARY KEY ("id")
);

-- Link messaging_contacts to people
ALTER TABLE "messaging_contacts" ADD COLUMN "person_id" TEXT;
ALTER TABLE "messaging_contacts" ADD CONSTRAINT "messaging_contacts_person_id_fkey"
    FOREIGN KEY ("person_id") REFERENCES "people"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- Indexes
CREATE INDEX "people_role_idx" ON "people"("role");
CREATE INDEX "people_phone_idx" ON "people"("phone");
CREATE INDEX "people_tenant_id_idx" ON "people"("tenant_id");
CREATE INDEX "messaging_contacts_person_id_idx" ON "messaging_contacts"("person_id");

-- Seed: directors
INSERT INTO "people" (id, name, role, phone, updated_at) VALUES
    (gen_random_uuid(), 'Aarão Melo', 'director', '+5511977808883', NOW()),
    (gen_random_uuid(), 'Patrícia Cunha', 'director', NULL, NOW()),
    (gen_random_uuid(), 'Carlos Daniel', 'director', NULL, NOW());

-- Link existing Telegram contact to Aarão
UPDATE "messaging_contacts"
SET person_id = (SELECT id FROM people WHERE name = 'Aarão Melo' LIMIT 1)
WHERE display_name = 'Aarão Melo' OR remote_id = '1105680913';
