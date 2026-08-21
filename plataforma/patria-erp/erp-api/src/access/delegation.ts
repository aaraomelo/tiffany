// Delegação capacitária com profundidade limitada (intensidade δ).
// Ver doc/casl-propagation.tex §4. δ é o orçamento de RE-delegação:
//   δ=0  → o detentor usa, mas não repassa a ninguém
//   δ=k  → re-delegável por mais k saltos (decrementa 1 por concessão)
//   δ=∞  → irrestrito (legado)
//
// Importante: δ é verificado no ATO DE DELEGAR (criar/atribuir regra), nunca
// na leitura de dados. Estas funções são puras (sem I/O) e 100% testáveis.

export const UNLIMITED = Infinity;

/** Storage (Prisma): null = ∞. Converte para número em código. */
export function depthFromStored(v: number | null | undefined): number {
  return v == null ? UNLIMITED : v;
}

/** Converte de volta para storage: ∞ vira null. */
export function depthToStored(d: number): number | null {
  return d === UNLIMITED ? null : d;
}

/** Um detentor pode delegar sse tem orçamento ≥ 1 (∞ conta como ≥ 1). */
export function canDelegate(holderDepth: number): boolean {
  return holderDepth >= 1;
}

/** Maior δ que o detentor pode conceder (∞−1=∞); null se não pode delegar. */
export function maxDelegableDepth(holderDepth: number): number | null {
  if (!canDelegate(holderDepth)) return null;
  return holderDepth === UNLIMITED ? UNLIMITED : holderDepth - 1;
}

/**
 * Valida a profundidade proposta numa concessão.
 * Retorna a δ permitida (= proposta) ou null se a concessão é inválida.
 * Regra de atenuação: 0 ≤ proposta ≤ holder − 1.
 */
export function attenuatedDepth(
  holderDepth: number,
  proposedDepth: number,
): number | null {
  const max = maxDelegableDepth(holderDepth);
  if (max === null) return null;
  if (proposedDepth < 0) return null;
  if (proposedDepth > max) return null;
  return proposedDepth;
}
