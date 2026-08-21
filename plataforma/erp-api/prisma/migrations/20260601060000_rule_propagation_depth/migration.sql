-- Intensidade δ (orçamento de re-delegação) por regra/capacidade.
-- null = ∞ (irrestrito/legado); 0 = não re-delegável.
ALTER TABLE "AccessRule" ADD COLUMN "propagationDepth" INTEGER;
