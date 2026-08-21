# Patria Poster Artist

Estúdio de **templates de documento** em PDF para o ecossistema Patria. Reconstrução,
generalizada de "cartaz de preço" para "documento", do motor declarativo do
`easysync-poster-artist` — agora focado em **Ordem de Serviço** (A4).

Stack alinhada ao `erp-app`: React 19 + Vite 5 + TypeScript + MUI 9/emotion.
Motor de PDF 100% client-side: **pdf-lib + @pdf-lib/fontkit + opentype.js**.

## Rodar

```bash
npm install
npm run dev      # http://localhost:5175
npm run build    # tsc -b && vite build
```

Abre o preview da Ordem de Serviço (dados mock do PDF modelo) e o botão
**Exportar PDF** baixa o arquivo. Trocar a fonte re-renderiza preview e PDF juntos.

## Arquitetura

```
ServiceOrderData (mock)
   └─ buildServiceOrder()        templates/serviceOrder.ts → DocumentSpec (blocos declarativos)
        └─ layoutDocument()      render/layout.ts → LaidOutDocument (DrawOps posicionadas, paginadas)
             ├─ DocPreview        render/DocPreview.tsx → HTML
             └─ renderDocumentPdf render/pdf.ts → pdf-lib bytes
```

**Paridade HTML × PDF**: a quebra de linha e o dimensionamento são resolvidos uma
única vez no `layoutDocument` (medindo com opentype.js, a mesma fonte embarcada no
PDF). Os dois renderers consomem as MESMAS `DrawOp`. O Exportar PDF reusa o mesmo
`LaidOutDocument` do preview — o que você vê é o que sai.

### Camadas

- `src/doc/engine/` — núcleo agnóstico portado do poster-artist: fit de fonte
  (`text.ts`, considera ascender+descender), catálogo de fontes, embed pdf-lib,
  cores, `{path}` interpolation, formatadores ptBR (moeda/data).
- `src/doc/schema/document.ts` — modelo declarativo de documento em **fluxo**:
  blocos (`sectionBar`, `field`, `columns`, `table`, `summary`, `signatures`,
  `paragraph`, `image`, `spacer`) que empilham e quebram em páginas.
- `src/doc/render/` — motor de layout (mede + empilha + pagina, com **repetição
  de cabeçalho de tabela** ao quebrar) e os dois renderers.
- `src/doc/templates/` — templates concretos (Ordem de Serviço). Novos documentos
  = novo builder `Data → DocumentSpec`.

### Por que estendeu o engine?

O poster-artist é layout **absoluto de página única** (bom pra cartaz). Documento
é **fluxo**: seções empilham, tabelas têm N linhas e podem transbordar pra página 2.
A camada `src/doc` reaproveita as joias do poster (fit de fonte, embed, interpolação)
e adiciona o modelo de blocos + paginação.

## Próximos passos

- Estúdio de edição (overrides por bloco, arrastar/posicionar) — reaproveitar o
  padrão de overrides do poster-artist (zustand).
- Plugar dados reais do domínio `ServiceOrder` do `erp-api` (hoje mock).
- Logo do tenant no cabeçalho (campo `company.logo` já suportado).
- Novos templates (orçamento, recibo, etiqueta).
