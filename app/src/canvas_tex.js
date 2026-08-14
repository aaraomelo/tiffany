// canvas_tex.js — o pintor do dialecto: o PDF da casa desenhado no canvas.
// «Mesmo esquema do PDF»: o tradutor compõe (tex.wasm, slots, MOVE) e o
// canvas pinta o NOSSO dialecto — texto plano (Flate fora de propósito),
// glifos como Form XObjects de traços m/l, e um punhado de operadores:
//
//     q Q cm Do rg RG w m l h f S re
//
// Sem dependência externa: o PDF é auto-produzido, o pintor lê o rasto.
// E os CORPOS viajam com a página: /SementeEstrela é Alonzo (a composição),
// /AssinaturaOito é Caelum (a Lei 8) — corposDoPdf devolve os dois.

const OPS = new Set(['q', 'Q', 'cm', 'Do', 'rg', 'RG', 'w', 'm', 'l', 'h', 'f', 'S', 're'])

function tokeniza (s) {
  return s.split(/[\s\n]+/).filter(t => t.length)
}

/** Lê o PDF da casa: páginas, formas (glifos), selos. */
export function parsePdfCasa (bytes) {
  const s = typeof bytes === 'string' ? bytes
    : new TextDecoder('latin1').decode(bytes)
  const objs = new Map()
  const rx = /(\d+) 0 obj(<<[\s\S]*?>>)?(?:\s*stream\n([\s\S]*?)endstream)?\s*endobj/g
  let m
  while ((m = rx.exec(s))) {
    objs.set(Number(m[1]), { dict: m[2] || '', stream: m[3] || '' })
  }
  const forms = new Map()
  const paginas = []
  const desconhecidas = new Set()
  for (const [id, o] of objs) {
    if (o.dict.includes('/Subtype/Form')) {
      const bb = /\/BBox\[([^\]]*)\]/.exec(o.dict)
      forms.set(id, {
        bbox: bb ? bb[1].trim().split(/\s+/).map(Number) : null,
        tokens: tokeniza(o.stream),
      })
    }
  }
  for (const [, o] of objs) {
    if (!o.dict.includes('/Type/Page') || o.dict.includes('/Type/Pages')) continue
    const mb = /\/MediaBox\[([^\]]*)\]/.exec(o.dict)
    const media = mb ? mb[1].trim().split(/\s+/).map(Number) : [0, 0, 595, 842]
    const cont = /\/Contents\[([^\]]*)\]/.exec(o.dict) ||
      /\/Contents (\d+) 0 R/.exec(o.dict)
    const ids = (cont ? cont[1] : '').match(/\d+(?= 0 R)/g) || []
    const resR = /\/Resources (\d+) 0 R/.exec(o.dict)
    const resDict = resR && objs.get(Number(resR[1]))
      ? objs.get(Number(resR[1])).dict : o.dict
    const xmap = new Map()
    const xo = /\/XObject<<([\s\S]*?)>>/.exec(resDict)
    if (xo) {
      const rx2 = /\/(\S+?) (\d+) 0 R/g
      let m2
      while ((m2 = rx2.exec(xo[1]))) xmap.set(m2[1], Number(m2[2]))
    }
    const tokens = []
    for (const cid of ids) {
      const co = objs.get(Number(cid))
      if (co && co.stream) tokens.push(...tokeniza(co.stream))
    }
    paginas.push({ media, tokens, xmap })
  }
  // os corpos que viajam: Alonzo e Caelum
  const semente = /\/Type\/SementeEstrela([^>]*)>>/.exec(s)
  const oitoM = /\/Type\/AssinaturaOito[^\[]*\/Sel\[([^\]]*)\]/.exec(s)
  const oito = oitoM ? oitoM[1].trim().split(/\s+/).map(Number) : null
  return { paginas, forms, semente: semente ? semente[1].trim() : null, oito, desconhecidas }
}

/** Executa tokens do dialecto sobre um ctx (canvas 2D ou mock). */
function executa (tokens, ctx, forms, xmap, desconhecidas, fundo) {
  const pilha = []
  let n = { fill: [0, 0, 0], stroke: [0, 0, 0] }
  const args = []
  for (let i = 0; i < tokens.length; i++) {
    const t = tokens[i]
    if (/^-?[\d.]+$/.test(t)) { args.push(Number(t)); continue }
    if (t.startsWith('/')) { args.length = 0; args.nome = t.slice(1); continue }
    switch (t) {
      case 'q': ctx.save(); pilha.push(n); n = { ...n }; break
      case 'Q': ctx.restore(); n = pilha.pop() || n; break
      case 'cm': ctx.transform(args[0], args[1], args[2], args[3], args[4], args[5]); break
      case 'Do': {
        const fid = xmap.get(args.nome)
        const f = fid && forms.get(fid)
        if (f) executa(f.tokens, ctx, forms, xmap, desconhecidas, fundo)
        break
      }
      case 'rg': n.fill = [args[0], args[1], args[2]]
        ctx.fillStyle = `rgb(${args.map(v => Math.round(v * 255)).join(',')})`; break
      case 'RG': n.stroke = [args[0], args[1], args[2]]
        ctx.strokeStyle = `rgb(${args.map(v => Math.round(v * 255)).join(',')})`; break
      case 'w': ctx.lineWidth = args[0]; break
      case 'm': ctx.beginPath(); ctx.moveTo(args[0], args[1]); break
      case 'l': ctx.lineTo(args[0], args[1]); break
      case 'h': ctx.closePath(); break
      case 'f': ctx.fill(); break
      case 'S': ctx.stroke(); break
      case 're': ctx.beginPath(); ctx.rect(args[0], args[1], args[2], args[3]); ctx.fill(); break
      default: desconhecidas.add(t)
    }
    args.length = 0
  }
}

/** Pinta a página `i` do doc no canvas (2D). Devolve {larg, alt} em pt. */
export function pintaPagina (doc, i, canvas, escala = 1.5) {
  const pag = doc.paginas[i]
  if (!pag) return null
  const [x0, y0, x1, y1] = pag.media
  const larg = x1 - x0, alt = y1 - y0
  canvas.width = Math.round(larg * escala)
  canvas.height = Math.round(alt * escala)
  const ctx = canvas.getContext('2d')
  ctx.fillStyle = '#ffffff'
  ctx.fillRect(0, 0, canvas.width, canvas.height)
  // PDF sobe, canvas desce: vira o eixo
  ctx.setTransform(escala, 0, 0, -escala, -x0 * escala, y1 * escala)
  executa(pag.tokens, ctx, doc.forms, pag.xmap, doc.desconhecidas)
  ctx.setTransform(1, 0, 0, 1, 0, 0)
  return { larg, alt }
}

/** Os corpos que viajam no PDF: Alonzo (a semente) e Caelum (a Lei 8). */
export function corposDoPdf (doc) {
  return {
    alonzo: doc.semente,                       /* /SementeEstrela — a composição */
    caelum: doc.oito,                          /* /AssinaturaOito — o espectro, 256 */
  }
}
