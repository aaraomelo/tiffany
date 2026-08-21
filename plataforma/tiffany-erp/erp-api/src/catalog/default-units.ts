/**
 * Unidades de medida padrão semeadas em todo tenant novo (e preenchidas
 * sob demanda em tenants existentes sem unidades, via CatalogService.listUnits).
 *
 * Conjunto voltado a motopeças e assistência eletrônica: peças avulsas,
 * pares/jogos/kits, óleo (L/ML), cabo/fio (M), e mão de obra (H).
 * UN — Unidade é o padrão usado no cadastro de produto.
 */
export interface DefaultUnit {
  code: string;
  description: string;
  fractional: boolean;
}

export const DEFAULT_UNIT_CODE = 'UN';

export const DEFAULT_UNITS: DefaultUnit[] = [
  { code: 'UN', description: 'Unidade', fractional: false },
  { code: 'PC', description: 'Peça', fractional: false },
  { code: 'PAR', description: 'Par', fractional: false },
  { code: 'JG', description: 'Jogo', fractional: false },
  { code: 'KIT', description: 'Kit', fractional: false },
  { code: 'CX', description: 'Caixa', fractional: false },
  { code: 'M', description: 'Metro', fractional: true },
  { code: 'L', description: 'Litro', fractional: true },
  { code: 'ML', description: 'Mililitro', fractional: true },
  { code: 'KG', description: 'Quilograma', fractional: true },
  { code: 'H', description: 'Hora', fractional: true },
];

/** Linhas prontas para prisma.unit.createMany de um tenant. */
export function defaultUnitRows(tenantId: string) {
  return DEFAULT_UNITS.map((u) => ({ tenantId, ...u }));
}
