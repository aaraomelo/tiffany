-- CreateTable
CREATE TABLE "papers" (
    "slug" TEXT NOT NULL,
    "date" TIMESTAMP(3) NOT NULL,
    "authors" TEXT[],
    "tags" TEXT[],
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "papers_pkey" PRIMARY KEY ("slug")
);

-- CreateTable
CREATE TABLE "paper_i18n" (
    "id" SERIAL NOT NULL,
    "paper_slug" TEXT NOT NULL,
    "lang" TEXT NOT NULL,
    "title" TEXT NOT NULL,
    "abstract" TEXT NOT NULL,

    CONSTRAINT "paper_i18n_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "paper_files" (
    "id" SERIAL NOT NULL,
    "paper_slug" TEXT NOT NULL,
    "lang" TEXT NOT NULL,
    "kind" TEXT NOT NULL,
    "data" BYTEA NOT NULL,
    "size_bytes" INTEGER NOT NULL,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "paper_files_pkey" PRIMARY KEY ("id")
);

CREATE INDEX "paper_i18n_lang_idx" ON "paper_i18n"("lang");
CREATE UNIQUE INDEX "paper_i18n_paper_slug_lang_key" ON "paper_i18n"("paper_slug", "lang");
CREATE INDEX "paper_files_paper_slug_lang_idx" ON "paper_files"("paper_slug", "lang");
CREATE UNIQUE INDEX "paper_files_paper_slug_lang_kind_key" ON "paper_files"("paper_slug", "lang", "kind");

ALTER TABLE "paper_i18n" ADD CONSTRAINT "paper_i18n_paper_slug_fkey" FOREIGN KEY ("paper_slug") REFERENCES "papers"("slug") ON DELETE CASCADE ON UPDATE CASCADE;
ALTER TABLE "paper_files" ADD CONSTRAINT "paper_files_paper_slug_fkey" FOREIGN KEY ("paper_slug") REFERENCES "papers"("slug") ON DELETE CASCADE ON UPDATE CASCADE;
