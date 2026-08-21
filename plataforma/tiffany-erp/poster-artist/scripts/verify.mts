// Verificação headless: roda o MESMO pipeline (layoutDocument → renderDocumentPdf)
// em Node, com um polyfill de fetch que serve as TTF de public/fonts. Gera
// /tmp/os-verify.pdf pra conferência visual contra o PDF modelo.
import { readFile, writeFile } from 'node:fs/promises'
import { fileURLToPath } from 'node:url'
import { dirname, join } from 'node:path'

const here = dirname(fileURLToPath(import.meta.url))
const publicDir = join(here, '..', 'public')

const realFetch = globalThis.fetch
// @ts-expect-error - polyfill mínimo só pro que o engine usa
globalThis.fetch = async (url: string) => {
  if (typeof url === 'string' && url.startsWith('/')) {
    const buf = await readFile(join(publicDir, url))
    const ab = buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.byteLength)
    return {
      ok: true,
      arrayBuffer: async () => ab,
      headers: { get: () => '' },
    }
  }
  return realFetch(url as string)
}

const { layoutDocument } = await import('../src/doc/render/layout.ts')
const { renderDocumentPdf } = await import('../src/doc/render/pdf.ts')
const { buildServiceOrder } = await import('../src/doc/templates/serviceOrder.ts')
const { MOCK_SERVICE_ORDER } = await import('../src/mocks/serviceOrder.ts')

const spec = buildServiceOrder(MOCK_SERVICE_ORDER)
const laid = await layoutDocument(spec)
console.log(`páginas: ${laid.pages.length}, ops pág.1: ${laid.pages[0].ops.length}`)
const bytes = await renderDocumentPdf(laid)
await writeFile('/tmp/os-verify.pdf', bytes)
console.log('PDF escrito em /tmp/os-verify.pdf', bytes.length, 'bytes')
