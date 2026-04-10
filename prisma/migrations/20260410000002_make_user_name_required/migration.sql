-- Make user name required
UPDATE "users" SET "name" = '' WHERE "name" IS NULL;

-- AlterTable
ALTER TABLE "users" ALTER COLUMN "name" SET NOT NULL;
