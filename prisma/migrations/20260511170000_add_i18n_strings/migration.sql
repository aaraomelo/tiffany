-- CreateTable
CREATE TABLE "i18n_strings" (
    "id" SERIAL NOT NULL,
    "lang" TEXT NOT NULL,
    "key" TEXT NOT NULL,
    "value" TEXT NOT NULL,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "i18n_strings_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "i18n_strings_lang_idx" ON "i18n_strings"("lang");

-- CreateIndex
CREATE UNIQUE INDEX "i18n_strings_lang_key_key" ON "i18n_strings"("lang", "key");
