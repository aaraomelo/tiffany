-- AlterTable
ALTER TABLE "CustomerSupplier" ADD COLUMN     "embedded_at" TIMESTAMP(3),
ADD COLUMN     "embedding_model" TEXT;

-- AlterTable
ALTER TABLE "Product" ADD COLUMN     "embedded_at" TIMESTAMP(3),
ADD COLUMN     "embedding_model" TEXT;
