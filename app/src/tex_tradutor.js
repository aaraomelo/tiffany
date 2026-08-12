// ── O TRADUTOR .tex→PDF NO CLIENTE: tex.wasm + /corpo/, sem TeX Live ──────────
//
// O Aarão: «isso tem que rodar no front do cliente, PDF gerado no front via WASM,
// sem servidor». Localmente o middleware Node era um atalho — em produção o nginx
// só serve ficheiros, e a composição É no browser.
//
// Porta: MOVE(slot, ±1) no DISCO — inicia_wasm prende as fatias, o host escreve
// os corpos no vfs (vfs_reserva), compila, lê o PDF no slot 14. Sem monte SAIDA.
// /corpo/ medido por tools/corpo.sh. Alonzo compõe, Caelum assina (§T5–T6).
import manifesto from './corpo.json'

const DOCS = {
  teoria: 'teoria.tex',
  catalogo: 'catalogo.tex',
  enredo: 'enredo.tex',
  livro: 'livro.tex',
  'corpo-estelar': 'papers/corpo-estelar.tex',
  dualsort: 'papers/dualsort.tex',
  fisica: 'papers/fisica.tex',
  medida: 'papers/medida.tex',
  milenio: 'papers/milenio.tex',
  arquitetura: 'papers/arquitetura.tex',
}

let motor = null // { exports, view, encoder }

function num (x) {
  // long → i64 no módulo: o motor devolve BigInt; os índices do ArrayBuffer pedem Number
  return typeof x === 'bigint' ? Number(x) : x
}

function memView (E) {
  // memory.grow invalida o buffer — sempre se reata
  return new Uint8Array(E.DISCO.buffer)
}

function reserva (E, n) {
  const p = typeof E.vfs_reserva === 'function'
    ? num(E.vfs_reserva(n))
    : num(E.malloc(BigInt(n)))
  if (!p) throw new Error('tex.wasm: disco cheio')
  return p
}

function poeBytes (E, nome, bytes) {
  const enc = new TextEncoder()
  const nb = enc.encode(nome)
  const pNome = reserva(E, nb.length + 1)
  const pDados = reserva(E, (bytes.length || 0) + 1)
  const v = memView(E)
  v.set(nb, pNome)
  v[pNome + nb.length] = 0
  if (bytes.length) v.set(bytes, pDados)
  v[pDados + bytes.length] = 0
  if (!E.poe_ficheiro(pNome, pDados, bytes.length))
    throw new Error('tex.wasm: poe_ficheiro recusou ' + nome)
}

async function fetchCorpo (caminho) {
  const r = await fetch('/corpo/' + caminho)
  if (!r.ok) throw new Error(`corpo/${caminho} → ${r.status}`)
  return new Uint8Array(await r.arrayBuffer())
}

async function carregaMotor () {
  if (motor) return motor
  const buf = await (await fetch('/wasm/tex.wasm')).arrayBuffer()
  const { instance } = await WebAssembly.instantiate(buf)
  const E = instance.exports
  if (typeof E.inicia_wasm !== 'function')
    throw new Error('tex.wasm sem inicia_wasm — reconstrói com tools/sobe_tex_wasm.sh')
  if (typeof E.compila_ficheiro !== 'function')
    throw new Error('tex.wasm sem compila_ficheiro')
  E.inicia_wasm()
  // corpos no disco (MOVE −1): a lista medida, não adivinhada
  for (const f of manifesto.ficheiros) {
    poeBytes(E, f, await fetchCorpo(f))
  }
  motor = { exports: E }
  return motor
}

/** Compõe `id` (chave de DOCS) no browser e devolve { bytes, ms }. */
export async function comporDoc (id) {
  const fonte = DOCS[id]
  if (!fonte) throw new Error(`documento desconhecido: ${id}`)
  const t0 = performance.now()
  const { exports: E } = await carregaMotor()
  E.limpa_saida()
  const enc = new TextEncoder()
  const nEnt = enc.encode(fonte)
  const nSai = enc.encode('saida.pdf')
  const pEnt = reserva(E, nEnt.length + 1)
  const pSai = reserva(E, nSai.length + 1)
  const v = memView(E)
  v.set(nEnt, pEnt); v[pEnt + nEnt.length] = 0
  v.set(nSai, pSai); v[pSai + nSai.length] = 0
  const rc = E.compila_ficheiro(pEnt, pSai)
  if (rc !== 0) throw new Error(`compila_ficheiro(${fonte}) → ${rc}`)
  const n = num(E.tam_saida())
  const addr = typeof E.MOVE === 'function' ? num(E.MOVE(14, 1)) : num(E.end_saida())
  if (n < 100 || !addr) throw new Error(`saída vazia (${n} bytes)`)
  const out = memView(E).slice(addr, addr + n)
  if (out[0] !== 0x25 || out[1] !== 0x50) // %P
    throw new Error('a saída não começa por %PDF')
  const latin = new TextDecoder('latin1').decode(out)
  if (!latin.includes('%%EOF'))
    throw new Error('a saída não fecha com %%EOF')
  if (!latin.includes('/Type/SementeEstrela'))
    throw new Error('Alonzo: falta /SementeEstrela — a composição não viajou')
  if (!latin.includes('/Type/AssinaturaOito'))
    throw new Error('Caelum: falta /AssinaturaOito — o esqueleto não assinou')
  return { bytes: out, ms: Math.round(performance.now() - t0), fonte }
}

/** Compõe e abre o PDF. `janela` (opcional) é um tab já aberto no click
 *  síncrono — senão o browser bloqueia o popup depois do await. */
export async function abrirDoc (id, janela) {
  const { bytes, ms } = await comporDoc(id)
  const url = URL.createObjectURL(new Blob([bytes], { type: 'application/pdf' }))
  if (janela && !janela.closed) {
    janela.location = url
  } else {
    window.open(url, '_blank', 'noopener')
  }
  setTimeout(() => URL.revokeObjectURL(url), 60_000)
  return ms
}

export function idDeArquivo (href) {
  const m = /\/docs\/([A-Za-z0-9_-]+)\.pdf/.exec(href || '')
  return m ? m[1] : null
}

export { DOCS }
