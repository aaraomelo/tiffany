-- CreateTable
CREATE TABLE "AccessRole" (
    "id" UUID NOT NULL,
    "tenantId" UUID NOT NULL,
    "name" TEXT NOT NULL,
    "description" TEXT,
    "isSystem" BOOLEAN NOT NULL DEFAULT false,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "AccessRole_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "AccessRule" (
    "id" UUID NOT NULL,
    "roleId" UUID NOT NULL,
    "action" JSONB NOT NULL,
    "subject" JSONB NOT NULL,
    "fields" JSONB,
    "conditions" JSONB,
    "inverted" BOOLEAN NOT NULL DEFAULT false,
    "reason" TEXT,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "AccessRule_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "_UserAccessRoles" (
    "A" UUID NOT NULL,
    "B" UUID NOT NULL,

    CONSTRAINT "_UserAccessRoles_AB_pkey" PRIMARY KEY ("A","B")
);

-- CreateIndex
CREATE INDEX "AccessRole_tenantId_idx" ON "AccessRole"("tenantId");

-- CreateIndex
CREATE UNIQUE INDEX "AccessRole_tenantId_name_key" ON "AccessRole"("tenantId", "name");

-- CreateIndex
CREATE INDEX "AccessRule_roleId_idx" ON "AccessRule"("roleId");

-- CreateIndex
CREATE INDEX "_UserAccessRoles_B_index" ON "_UserAccessRoles"("B");

-- AddForeignKey
ALTER TABLE "AccessRole" ADD CONSTRAINT "AccessRole_tenantId_fkey" FOREIGN KEY ("tenantId") REFERENCES "Tenant"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "AccessRule" ADD CONSTRAINT "AccessRule_roleId_fkey" FOREIGN KEY ("roleId") REFERENCES "AccessRole"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "_UserAccessRoles" ADD CONSTRAINT "_UserAccessRoles_A_fkey" FOREIGN KEY ("A") REFERENCES "AccessRole"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "_UserAccessRoles" ADD CONSTRAINT "_UserAccessRoles_B_fkey" FOREIGN KEY ("B") REFERENCES "TenantUser"("id") ON DELETE CASCADE ON UPDATE CASCADE;
