// ── O DISCO DO CORPO: mapa GKCORPO no localStorage, o motor lê daí ──
//
// Formato mapeado: uma tabela nome→slot (os mesmos .tex/.otf/.txt do manifesto),
// cada slot é o ficheiro deflate. localStorage é o disco; o wasm só recebe o que
// o fopen deste documento precisa — sem poe-all 5 MiB no DISCO linear.
//
// UTF-16 do LS: o deflate cabe (~4,8 MiB) onde o binário cru não (9,81 MiB).

export const MAGIA = 'GKCORPO'
export const CHAVE_MAPA = 'gk:corpo:mapa'
export const chaveSlot = (i) => 'gk:corpo:' + i

const CHUNK = 0x8000

export function u8ParaLatin1 (u8) {
  let s = ''
  for (let i = 0; i < u8.length; i += CHUNK)
    s += String.fromCharCode.apply(null, u8.subarray(i, Math.min(i + CHUNK, u8.length)))
  return s
}

export function latin1ParaU8 (s) {
  const u8 = new Uint8Array(s.length)
  for (let i = 0; i < s.length; i++) u8[i] = s.charCodeAt(i) & 255
  return u8
}

export async function deflateU8 (u8) {
  const cs = new CompressionStream('deflate')
  const out = new Uint8Array(await new Response(new Blob([u8]).stream().pipeThrough(cs)).arrayBuffer())
  if (out.length < 2 || out[0] !== 0x78)
    throw new Error('deflate: esperado zlib (0x78), não este CompressionStream')
  return out
}

export async function inflateU8 (u8) {
  const ds = new DecompressionStream('deflate')
  return new Uint8Array(await new Response(new Blob([u8]).stream().pipeThrough(ds)).arrayBuffer())
}

/** Arquivo mapeado: magia + n + (namelen, nome, tam)×n + blobs. Resíduo 0 na volta. */
export function empacota (pares) {
  let cab = 8 + 2
  for (const { nome } of pares) cab += 1 + nome.length + 4
  let blobs = 0
  for (const { bytes } of pares) blobs += bytes.length
  const out = new Uint8Array(cab + blobs)
  const mag = MAGIA + '\0'
  for (let i = 0; i < 8; i++) out[i] = mag.charCodeAt(i)
  out[8] = pares.length & 255
  out[9] = (pares.length >> 8) & 255
  let p = 10
  for (const { nome, bytes } of pares) {
    out[p++] = nome.length
    for (let i = 0; i < nome.length; i++) out[p++] = nome.charCodeAt(i)
    const n = bytes.length
    out[p++] = n & 255
    out[p++] = (n >> 8) & 255
    out[p++] = (n >> 16) & 255
    out[p++] = (n >> 24) & 255
  }
  for (const { bytes } of pares) {
    out.set(bytes, p)
    p += bytes.length
  }
  return out
}

export function desempacota (buf) {
  const u8 = buf instanceof Uint8Array ? buf : new Uint8Array(buf)
  let mag = ''
  for (let i = 0; i < 7; i++) mag += String.fromCharCode(u8[i])
  if (mag !== MAGIA || u8[7] !== 0) throw new Error('GKCORPO: magia')
  const n = u8[8] | (u8[9] << 8)
  const slots = []
  let p = 10
  for (let i = 0; i < n; i++) {
    const ln = u8[p++]
    let nome = ''
    for (let j = 0; j < ln; j++) nome += String.fromCharCode(u8[p++])
    const tam = u8[p] | (u8[p + 1] << 8) | (u8[p + 2] << 16) | (u8[p + 3] << 24)
    p += 4
    slots.push({ nome, tam })
  }
  const mapa = new Map()
  for (const s of slots) {
    mapa.set(s.nome, u8.subarray(p, p + s.tam))
    p += s.tam
  }
  if (p !== u8.length) throw new Error('GKCORPO: tamanho')
  return mapa
}

/** Base comum (fontes + estilo + classe + capas) + o .tex deste documento. */
export function ficheirosPara (fonte, lista) {
  return lista.filter((f) =>
    f === fonte ||
    f.endsWith('.otf') ||
    f.endsWith('.txt') ||
    f === 'estilo.tex' ||
    f === 'papers/estilo.tex' ||
    f === 'gkcapa.tex' ||
    f === 'papers/gkcapa.tex' ||
    f === 'livro.tex')
}

/** Resolve pedido do fopen (../gkcapa, gkcapa, …) para o nome canónico do mapa. */
export function resolveCorpoNome (pedido, nomes) {
  if (!pedido) return null
  const has = (n) => (typeof nomes.has === 'function' ? nomes.has(n) : nomes.includes(n))
  const keys = typeof nomes.keys === 'function' ? [...nomes.keys()] : [...nomes]
  if (has(pedido)) return pedido
  const cands = []
  const push = (s) => { if (s && !cands.includes(s)) cands.push(s) }
  push(pedido)
  const semDot = pedido.replace(/^(\.\.\/)+/, '')
  push(semDot)
  const base = semDot.includes('/') ? semDot.slice(semDot.lastIndexOf('/') + 1) : semDot
  push(base)
  if (base && !base.includes('.')) {
    push(base + '.tex')
    push(semDot + '.tex')
  }
  for (const c of cands) if (has(c)) return c
  const stem = (s) => {
    let b = s
    const i = b.lastIndexOf('/')
    if (i >= 0) b = b.slice(i + 1)
    if (b.endsWith('.tex')) b = b.slice(0, -4)
    return b
  }
  const want = stem(pedido)
  if (!want) return null
  for (const k of keys) if (stem(k) === want) return k
  return null
}

export function leMapa (storage) {
  if (!storage) return null
  const s = storage.getItem(CHAVE_MAPA)
  if (!s) return null
  try {
    const m = JSON.parse(s)
    if (m.magia !== MAGIA || !Array.isArray(m.slots)) return null
    return m
  } catch {
    return null
  }
}

export function mapaBate (mapa, soma, lista) {
  if (!mapa || mapa.soma !== soma || mapa.slots.length !== lista.length) return false
  for (let i = 0; i < lista.length; i++) if (mapa.slots[i].nome !== lista[i]) return false
  return true
}

export async function leFicheiro (storage, mapa, nome) {
  const i = mapa.slots.findIndex((s) => s.nome === nome)
  if (i < 0) return null
  const latin = storage.getItem(chaveSlot(i))
  if (latin == null) return null
  const u8 = await inflateU8(latin1ParaU8(latin))
  if (u8.length !== mapa.slots[i].tam)
    throw new Error('corpo LS: tam ≠ slot ' + nome)
  return u8
}

export function bytesLS (storage) {
  if (!storage) return 0
  let n = 0
  const N = storage.length
  for (let i = 0; i < N; i++) {
    const k = storage.key(i)
    if (!k || k.indexOf('gk:corpo') !== 0) continue
    const v = storage.getItem(k) || ''
    n += 2 * (k.length + v.length)
  }
  return n
}

export function apagaCorpo (storage) {
  if (!storage) return
  const mapa = leMapa(storage)
  if (mapa) for (let i = 0; i < mapa.slots.length; i++) storage.removeItem(chaveSlot(i))
  // restos de uma gravação a meio
  const extra = []
  for (let i = 0; i < storage.length; i++) {
    const k = storage.key(i)
    if (k && k.indexOf('gk:corpo:') === 0) extra.push(k)
  }
  for (const k of extra) storage.removeItem(k)
}

export async function gravaCorpo (storage, pares, soma) {
  if (!storage) throw new Error('sem localStorage')
  const t0 = agora()
  apagaCorpo(storage)
  const slots = []
  let zTotal = 0
  let tamTotal = 0
  try {
    for (let i = 0; i < pares.length; i++) {
      const { nome, bytes } = pares[i]
      const z = await deflateU8(bytes)
      storage.setItem(chaveSlot(i), u8ParaLatin1(z))
      slots.push({ nome, tam: bytes.length, z: z.length })
      zTotal += z.length
      tamTotal += bytes.length
    }
    storage.setItem(CHAVE_MAPA, JSON.stringify({ magia: MAGIA, soma, slots }))
  } catch (e) {
    try { apagaCorpo(storage) } catch { /* quota a meio: não deixa mapa partido */ }
    throw e
  }
  return { soma, n: slots.length, zTotal, tamTotal, ms: agora() - t0 }
}

function agora () {
  return typeof performance !== 'undefined' && performance.now ? performance.now() : Date.now()
}

/** localStorage de teste (Node) — o mesmo contrato, sem quota de origem. */
export function memoriaLS () {
  const m = new Map()
  return {
    getItem (k) { return m.has(k) ? m.get(k) : null },
    setItem (k, v) { m.set(String(k), String(v)) },
    removeItem (k) { m.delete(String(k)) },
    key (i) { return [...m.keys()][i] ?? null },
    get length () { return m.size },
  }
}
