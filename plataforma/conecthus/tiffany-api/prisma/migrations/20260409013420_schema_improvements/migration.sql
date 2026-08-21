-- AlterTable
ALTER TABLE "projects" ADD COLUMN     "branch_deleted" BOOLEAN NOT NULL DEFAULT false,
ADD COLUMN     "deployed_dev_at" TIMESTAMP(3),
ADD COLUMN     "deployed_homolog_at" TIMESTAMP(3),
ADD COLUMN     "deployed_prod_at" TIMESTAMP(3);

-- AlterTable
ALTER TABLE "tasks" ADD COLUMN     "branch_deleted" BOOLEAN NOT NULL DEFAULT false;
