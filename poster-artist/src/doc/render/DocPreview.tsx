import { Fragment } from 'react'
import { cssColor } from '../engine/color'
import type { DrawOp, LaidOutDocument } from './types'

// Preview HTML do documento. Consome as MESMAS DrawOps que o PDF (quebra de
// linha e tamanhos já resolvidos no layout), então é fiel ao PDF gerado.

const PT_TO_PX = 96 / 72

function opNode(op: DrawOp, i: number, s: number) {
  switch (op.kind) {
    case 'rect':
      return (
        <div
          key={i}
          style={{
            position: 'absolute',
            left: op.x * s,
            top: op.y * s,
            width: op.w * s,
            height: op.h * s,
            background: cssColor(op.color),
          }}
        />
      )
    case 'line': {
      const left = Math.min(op.x1, op.x2)
      const top = Math.min(op.y1, op.y2)
      const w = Math.abs(op.x2 - op.x1) || op.width
      const h = Math.abs(op.y2 - op.y1) || op.width
      return (
        <div
          key={i}
          style={{
            position: 'absolute',
            left: left * s,
            top: top * s,
            width: Math.max(w * s, op.width * s),
            height: Math.max(h * s, op.width * s),
            background: cssColor(op.color),
          }}
        />
      )
    }
    case 'text':
      if (!op.text) return null
      return (
        <div
          key={i}
          style={{
            position: 'absolute',
            left: op.x * s,
            top: op.yTop * s,
            fontFamily: `'${op.family}', sans-serif`,
            fontSize: op.size * s,
            lineHeight: `${op.lineHeight * s}px`,
            fontWeight: op.weight === 'bold' ? 700 : 400,
            color: cssColor(op.color),
            whiteSpace: 'nowrap',
          }}
        >
          {op.text}
        </div>
      )
    case 'image':
      return (
        <img
          key={i}
          src={op.src}
          alt=""
          style={{
            position: 'absolute',
            left: op.x * s,
            top: op.y * s,
            width: op.w * s,
            height: op.h * s,
            objectFit: 'contain',
          }}
        />
      )
  }
}

export function DocPreview({
  doc,
  zoom = 1,
}: {
  doc: LaidOutDocument
  zoom?: number
}) {
  const s = PT_TO_PX * zoom
  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: 16, alignItems: 'center' }}>
      {doc.pages.map((page, pi) => (
        <div
          key={pi}
          style={{
            position: 'relative',
            width: page.width * s,
            height: page.height * s,
            background: '#fff',
            boxShadow: '0 1px 6px rgba(0,0,0,0.18)',
            overflow: 'hidden',
            flex: '0 0 auto',
          }}
        >
          {page.ops.map((op, i) => (
            <Fragment key={i}>{opNode(op, i, s)}</Fragment>
          ))}
        </div>
      ))}
    </div>
  )
}
