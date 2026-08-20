/* tests/canvas_front.js — o daemon renderiza LaTeX no canvas por default:
 * o pintor do dialecto (canvas_tex.js) contra uma composição REAL, no mesmo
 * esquema do PDF — e os corpos de Caelum e Alonzo à vista (eval 13/08).
 *
 * §C0  compõe papers/arquitetura.tex pelo tex.wasm em node (o esquema do PDF:
 *      poe_ficheiro/compila/MOVE(14,+1) — o espelho do tex_wasm.js)
 * §C1  o pintor lê o dialecto: páginas com MediaBox, formas de glifo,
 *      e NENHUM operador desconhecido (o dialecto fecha: q Q cm Do rg RG
 *      w m l h f S re)
 * §C2  a pintura acontece: um ctx-espelho conta os traços — moveTo/lineTo/
 *      fill aos milhares, o eixo virado, a página com dimensão certa
 * §C3  OS CORPOS: Alonzo (/SementeEstrela) e Caelum (/AssinaturaOito, 256
 *      componentes) viajam no PDF e o pintor devolve-os — ver o corpo de
 *      Caelum e Alonzo também
 * §C4  o embrulho do daemon: uma RESPOSTA solta embrulhada num documento
 *      mínimo compõe pelo mesmo esquema — o default é viável
 *
 *   node tests/canvas_front.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execFileSync } = require('child_process')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const RAIZ = path.resolve(__dirname, '..')
const TMP = process.env.TMPDIR || '/tmp'

;(async () => {
  const { parsePdfCasa, pintaPagina, corposDoPdf } =
    await import('../app/src/canvas_tex.js')

  /* §C0 — compõe de verdade, no esquema do PDF (espelho do tex_wasm.js) */
  const wasm = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'tex.wasm')
  const { instance } = await WebAssembly.instantiate(fs.readFileSync(wasm), {
    env: { __fich_miss: () => 0 },
  }).catch(async () => WebAssembly.instantiate(fs.readFileSync(wasm)))
  const E = instance.exports
  E.inicia_wasm()
  const num = x => (typeof x === 'bigint' ? Number(x) : x)
  const mem = () => new Uint8Array(E.DISCO.buffer)
  function reserva (n) {
    const p = num(E.vfs_reserva(n))
    if (!p) throw new Error('vfs_reserva')
    return p
  }
  function poeStr (s) {
    const nb = Buffer.from(s, 'latin1')
    const p = reserva(nb.length + 1)
    mem().set(nb, p); mem()[p + nb.length] = 0
    return p
  }
  function poeFich (nome, bytes) {
    const pN = poeStr(nome)
    const pD = reserva(bytes.length + 1)
    if (bytes.length) mem().set(bytes, pD)
    mem()[pD + bytes.length] = 0
    if (!E.poe_ficheiro(pN, pD, bytes.length)) throw new Error('poe ' + nome)
  }
  function compoe (nome) {
    E.limpa_saida()
    const rc = num(E.compila_ficheiro(poeStr(nome), poeStr('saida.pdf')))
    const tam = num(E.tam_saida())
    const end = num(E.MOVE(14, 1))
    return { rc, pdf: Buffer.from(mem().slice(end, end + tam)) }
  }
  const man = JSON.parse(fs.readFileSync(path.join(RAIZ, 'app', 'src', 'corpo.json'), 'utf8'))
  for (const f of man.ficheiros) poeFich(f, fs.readFileSync(path.join(RAIZ, f)))
  const r = compoe('papers/arquitetura.tex')
  ok('§C0 o computacional compõe pelo esquema do PDF (rc=0, %PDF…%%EOF)',
    r.rc === 0 && r.pdf.slice(0, 4).toString() === '%PDF' &&
    r.pdf.includes('%%EOF'))

  /* §C1 — o pintor lê o dialecto, e o dialecto fecha */
  const doc = parsePdfCasa(r.pdf)
  ok('§C1 páginas com MediaBox', doc.paginas.length >= 1 &&
    doc.paginas[0].media.length === 4)
  ok('§C1 formas de glifo no doc (Forms m/l)', doc.forms.size > 50)

  /* §C2 — a pintura: um ctx-espelho conta os traços */
  const conta = { save: 0, restore: 0, transform: 0, moveTo: 0, lineTo: 0, fill: 0, stroke: 0, rect: 0, beginPath: 0, closePath: 0, setTransform: 0, fillRect: 0 }
  const ctx = new Proxy({}, {
    get: (o, k) => {
      if (k === 'fillStyle' || k === 'strokeStyle' || k === 'lineWidth') return undefined
      return (...a) => { conta[k] = (conta[k] || 0) + 1; void a }
    },
    set: () => true,
  })
  const canvasMock = {
    width: 0, height: 0,
    getContext: () => ctx,
  }
  const dim = pintaPagina(doc, 0, canvasMock, 1.5)
  console.log(`página 1: ${dim.larg.toFixed(0)}×${dim.alt.toFixed(0)} pt · ` +
    `moveTo=${conta.moveTo} lineTo=${conta.lineTo} fill=${conta.fill} · ` +
    `desconhecidos=${doc.desconhecidas.size}`)
  ok('§C2 a página tem a dimensão A4 (595×842 pt)',
    Math.round(dim.larg) === 595 && Math.round(dim.alt) === 842)
  ok('§C2 a pintura traça: moveTo/lineTo aos milhares, fill a centenas',
    conta.moveTo > 100 && conta.lineTo > 1000 && conta.fill > 100)
  ok('§C1 NENHUM operador desconhecido — o dialecto fecha',
    doc.desconhecidas.size === 0)

  /* §C3 — os corpos de Alonzo e Caelum, vistos pelo pintor */
  const corpos = corposDoPdf(doc)
  ok('§C3 ALONZO viaja: /SementeEstrela presente e lida', !!corpos.alonzo)
  ok('§C3 CAELUM viaja: /AssinaturaOito com 256 componentes',
    Array.isArray(corpos.caelum) && corpos.caelum.length === 256)

  /* §C4 — o embrulho do daemon: uma resposta solta compõe */
  const resposta = 'Bom dia! A energia conserva-se: $E(u\\otimes v)=E(u)E(v)$ --- o cristal de Hurwitz. E a volta devolve.'
  const embrulho = '\\documentclass[11pt,a4paper]{article}\n\\begin{document}\n' +
    resposta + '\n\\end{document}\n'
  poeFich('resposta.tex', Buffer.from(embrulho, 'latin1'))
  const r2 = compoe('resposta.tex')
  const doc2 = r2.rc === 0 ? parsePdfCasa(r2.pdf) : null
  ok('§C4 a resposta do daemon embrulhada COMPÕE pelo mesmo esquema (rc=0)',
    r2.rc === 0 && doc2 && doc2.paginas.length >= 1)
  if (doc2) {
    const conta2 = { moveTo: 0, lineTo: 0, fill: 0 }
    const ctx2 = new Proxy({}, {
      get: (o, k) => (...a) => { conta2[k] = (conta2[k] || 0) + 1; void a },
      set: () => true,
    })
    pintaPagina(doc2, 0, { width: 0, height: 0, getContext: () => ctx2 }, 1.5)
    ok('§C4 e PINTA no canvas: a resposta vira traços (o default é viável)',
      conta2.lineTo > 100 && doc2.desconhecidas.size === 0)
    const c2 = corposDoPdf(doc2)
    ok('§C4 com os corpos a bordo: Alonzo e Caelum também na resposta',
      !!c2.alonzo && Array.isArray(c2.caelum) && c2.caelum.length === 256)
  }

  console.log('')
  if (!falhas) {
    console.log('  O daemon pode renderizar LaTeX no canvas por default: o mesmo')
    console.log('  esquema do PDF compõe, o pintor lê o dialecto inteiro (zero ops')
    console.log('  desconhecidos), e os corpos de Alonzo e Caelum viajam à vista.')
  }
  console.log(`#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch(e => { console.error(e); process.exit(1) })
