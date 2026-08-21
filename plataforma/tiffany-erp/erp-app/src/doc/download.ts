import { layoutDocument } from './render/layout'
import { renderDocumentPdf } from './render/pdf'
import { buildServiceOrder } from './templates/serviceOrder'
import type { ServiceOrderData } from './serviceOrderData'

/** Gera o PDF da Ordem de Serviço (client-side) e dispara o download. */
export async function downloadServiceOrderPdf(data: ServiceOrderData): Promise<void> {
  const spec = buildServiceOrder(data)
  const laid = await layoutDocument(spec)
  const bytes = await renderDocumentPdf(laid)
  const blob = new Blob([bytes as BlobPart], { type: 'application/pdf' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = `ordem-de-servico-${data.number}.pdf`
  a.click()
  URL.revokeObjectURL(url)
}
