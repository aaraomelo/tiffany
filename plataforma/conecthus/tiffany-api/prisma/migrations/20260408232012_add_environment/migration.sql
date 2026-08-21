-- CreateEnum
CREATE TYPE "Environment" AS ENUM ('dev', 'homolog', 'prod');

-- AlterTable
ALTER TABLE "projects" ADD COLUMN     "environment" "Environment" NOT NULL DEFAULT 'dev',
ADD COLUMN     "promote_to" "Environment";

-- AlterTable
ALTER TABLE "tasks" ADD COLUMN     "environment" "Environment" NOT NULL DEFAULT 'dev',
ADD COLUMN     "promote_to" "Environment";
