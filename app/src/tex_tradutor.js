// ── O TRADUTOR .tex→PDF NO CLIENTE: tex.wasm + /corpo/, sem TeX Live ──────────
//
// O Aarão: «isso tem que rodar no front do cliente, PDF gerado no front via WASM,
// sem servidor». Localmente o middleware Node era um atalho — em produção o nginx
// só serve ficheiros, e a composição É no browser.
//
// Porta medida em tests/tex_wasm.js: poe_ficheiro / inicia_wasm / compila_ficheiro
// / limpa_saida / end_saida / tam_saida / DISCO. O ambiente (fontes, estilo, classe)
// vem de /corpo/, a lista medida por tools/corpo.sh. O que Alonzo compõe, Caelum
// assina: /SementeEstrela e /AssinaturaOito viajam com o PDF (§T5–T6).
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

function poeBytes (E, nome, bytes) {
  const enc = new TextEncoder()
  const nb = enc.encode(nome)
  const pNome = num(E.malloc(BigInt(nb.length + 1)))
  const pDados = num(E.malloc(BigInt(bytes.length || 1)))
  if (!pNome || !pDados) throw new Error('tex.wasm: malloc falhou ao pôr ' + nome)
  const v = memView(E)
  v.set(nb, pNome)
  v[pNome + nb.length] = 0
  if (bytes.length) v.set(bytes, pDados)
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
  // o ambiente completo nos slots — a lista medida, não adivinhada
  for (const f of manifesto.ficheiros) {
    poeBytes(E, f, await fetchCorpo(f))
  }
  E.inicia_wasm()
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
  // nomes C na memória linear
  const enc = new TextEncoder()
  const nEnt = enc.encode(fonte)
  const nSai = enc.encode('saida.pdf')
  const pEnt = num(E.malloc(BigInt(nEnt.length + 1)))
  const pSai = num(E.malloc(BigInt(nSai.length + 1)))
  const v = memView(E)
  v.set(nEnt, pEnt); v[pEnt + nEnt.length] = 0
  v.set(nSai, pSai); v[pSai + nSai.length] = 0
  const rc = E.compila_ficheiro(pEnt, pSai)
  if (rc !== 0) throw new Error(`compila_ficheiro(${fonte}) → ${rc}`)
  const n = num(E.tam_saida())
  const addr = num(E.end_saida())
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
