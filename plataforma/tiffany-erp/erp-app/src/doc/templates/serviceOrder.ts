import { MUTED } from '../engine/color'
import { brl, dateBR } from '../engine/format'
import type { DocBlock, DocumentSpec } from '../schema/document'
import type { ServiceOrderData } from '../serviceOrderData'

// Traduz uma Ordem de Serviço no documento declarativo A4, replicando o layout
// do PDF modelo: header em 3 colunas, barra de título, cliente, informações
// básicas (grid 2col), tabelas de serviços/peças, subtotais, pagamento,
// garantia, observações, local+data e assinaturas.

const FAMILY = 'Inter'

export function buildServiceOrder(data: ServiceOrderData): DocumentSpec {
  const c = data.company

  const companyLines = [c.responsible, c.cnpj, c.address, c.cityLine, c.cep ? `CEP ${c.cep}` : '']
    .filter(Boolean)
    .join('\n')

  const contactLines = [c.email, ...(c.phones ?? [])].filter(Boolean).join('\n')

  const servicesTotal = data.services.reduce((s, x) => s + x.price, 0)
  const partsTotal = data.parts.reduce((s, x) => s + x.price, 0)
  const total = servicesTotal + partsTotal

  const blocks: DocBlock[] = [
    // --- Cabeçalho (3 colunas) ---
    {
      kind: 'columns',
      gap: 18,
      columns: [
        {
          flex: 5,
          blocks: [
            ...(c.logo
              ? ([{ kind: 'image', src: c.logo, width: 120, height: 40 }] as DocBlock[])
              : []),
            { kind: 'paragraph', text: c.name, size: 15, weight: 'bold' },
            { kind: 'spacer', height: 3 },
            { kind: 'paragraph', text: companyLines, size: 8.5, color: [70, 70, 70] },
          ],
        },
        {
          flex: 4,
          blocks: [{ kind: 'paragraph', text: contactLines, size: 8.5, color: [70, 70, 70] }],
        },
        {
          flex: 2,
          blocks: [
            { kind: 'paragraph', text: dateBR(data.date), size: 9, weight: 'bold', align: 'right' },
          ],
        },
      ],
    },
    ...(c.instagram
      ? ([{ kind: 'paragraph', text: c.instagram, size: 8.5, color: MUTED }] as DocBlock[])
      : []),

    { kind: 'spacer', height: 8 },

    // --- Título ---
    { kind: 'sectionBar', variant: 'title', text: `Ordem de serviço ${data.number}` },

    { kind: 'spacer', height: 2 },
    { kind: 'paragraph', text: `Cliente: ${data.customer.name}`, size: 11, weight: 'bold' },
    ...(data.customer.phone
      ? ([{ kind: 'paragraph', text: data.customer.phone, size: 9 }] as DocBlock[])
      : []),

    // --- Informações básicas ---
    { kind: 'sectionBar', text: 'Informações básicas' },
    {
      kind: 'columns',
      columns: [
        {
          flex: 1,
          blocks: [
            { kind: 'field', label: 'Marca', value: data.device.brand ?? '—' },
            { kind: 'spacer', height: 8 },
            { kind: 'field', label: 'Aparelho', value: data.device.kind ?? '—' },
          ],
        },
        {
          flex: 1,
          blocks: [
            { kind: 'field', label: 'Modelo', value: data.device.model ?? '—' },
            { kind: 'spacer', height: 8 },
            { kind: 'field', label: 'Defeito', value: data.device.problem ?? '—' },
          ],
        },
      ],
    },

    // --- Serviços ---
    { kind: 'sectionBar', text: 'Serviços' },
    {
      kind: 'table',
      columns: [
        { header: 'Descrição', flex: 6, align: 'left' },
        { header: 'Preço', flex: 2, align: 'right' },
      ],
      rows: data.services.map((s) => [
        { primary: s.description, secondary: s.detail, weight: 'bold' },
        brl(s.price),
      ]),
    },

    // --- Peças ---
    { kind: 'sectionBar', text: 'Peças' },
    {
      kind: 'table',
      columns: [
        { header: 'Descrição', flex: 5, align: 'left' },
        { header: 'Unidade', flex: 2, align: 'left' },
        { header: 'Preço unitário', flex: 2, align: 'right' },
        { header: 'Qtd.', flex: 1, align: 'right' },
        { header: 'Preço', flex: 2, align: 'right' },
      ],
      rows: data.parts.map((p) => [
        { primary: p.name, secondary: p.detail, weight: 'bold' },
        p.unit,
        brl(p.unitPrice),
        String(p.qty),
        brl(p.price),
      ]),
    },
    { kind: 'spacer', height: 6 },
    {
      kind: 'summary',
      widthFraction: 0.5,
      rows: [
        { label: 'Serviços', value: brl(servicesTotal) },
        { label: 'Peças', value: brl(partsTotal) },
        { label: 'Total', value: brl(total), strong: true },
      ],
    },

    // --- Pagamento ---
    { kind: 'sectionBar', text: 'Pagamento' },
    {
      kind: 'columns',
      columns: [
        {
          flex: 1,
          blocks: [{ kind: 'field', label: 'Meios de pagamento', value: data.payment.methods ?? '—' }],
        },
        {
          flex: 1,
          blocks: [
            { kind: 'field', label: 'Condições de pagamento', value: data.payment.terms ?? '—' },
          ],
        },
      ],
    },

    // --- Garantia ---
    { kind: 'sectionBar', text: 'Garantia' },
    { kind: 'field', label: 'Período de garantia', value: data.warranty.period ?? '—' },

    // --- Informações adicionais ---
    ...(data.notes
      ? ([
          { kind: 'sectionBar', text: 'Informações adicionais' },
          { kind: 'paragraph', text: data.notes, size: 9, color: [70, 70, 70] },
        ] as DocBlock[])
      : []),

    { kind: 'spacer', height: 8 },
    {
      kind: 'paragraph',
      text: `${c.city ?? ''}${c.city ? ', ' : ''}${dateBR(data.date)}`,
      size: 9,
      weight: 'bold',
      align: 'center',
    },
    { kind: 'spacer', height: 10 },
    {
      kind: 'signatures',
      items: [{ label: c.name }, { label: data.customer.name }],
    },
  ]

  return {
    size: 'A4',
    orientation: 'portrait',
    margin: { top: 38, right: 40, bottom: 40, left: 40 },
    fontFamily: FAMILY,
    footer: 'Página {page}/{pages}',
    blocks,
  }
}
