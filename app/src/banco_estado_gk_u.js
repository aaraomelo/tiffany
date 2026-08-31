// banco_estado_gk_u.js — F2: classifica o estado do Reino. Não é S_ESTADO.
// S_ESTADO ≠ sessão ≠ livro ≠ coordenação ≠ fusão.
// GK = descrição estática + estado de sessão + capacidades. GLSL/LaTeX quietos.
// main.js não persiste localStorage; a âncora é o URL; velEstado é efémero (default).
// iframe continua oráculo. Não fundir velEstado.v com GKBANCO.v.

import { completa } from './banco_schema.js'
import { ancoraDeHash } from './banco_nav_u.js'

export const ID_ESTADO_GK = 'gk'
export const VEL_OMISSAO = Object.freeze({ v: 2, tocando: true })

/** Fatias do GK — não um estado único. */
export const FATIAS = Object.freeze(['descricao', 'sessao', 'capacidades'])

/** Imports de 1º nível de main.js, por fatia. Listam-se; não executam. */
export const FONTES_FATIA = Object.freeze({
  descricao: Object.freeze([
    './manifesto.json', './kernels_campo.json', './substratos.json', './pontes.json', './style.css',
  ]),
  sessao: Object.freeze(['./vel_estado.js']),
  capacidades: Object.freeze([
    './cards_kernel.js', './cards_campo.js', './tex_tradutor.js',
    './substratos.js', './motor_campo.js', './motor_wasm.js', './trailer_campo.js',
    './assistente.js', './terminal.js',
  ]),
})

export const CAPACIDADES_TARDAS = Object.freeze([
  Object.freeze({ id: 'cards', fonte: './cards_kernel.js', ciclo: 'F3' }),
  Object.freeze({ id: 'latex', fonte: './tex_tradutor.js', ciclo: 'F4' }),
  Object.freeze({ id: 'glsl', fonte: './motor_campo.js', ciclo: 'F5' }),
])

export function fatiaDeFonte (spec) {
  const s = String(spec || '')
  for (const fatia of FATIAS) {
    if ((FONTES_FATIA[fatia] || []).includes(s)) return fatia
  }
  return 'hospedeiro'
}

export function parseDescricaoGk (man) {
  const secoes = (man && man.secoes || []).map((s) => String(s.id || '')).filter(Boolean)
  const kernels = (man && man.kernels || []).map((k) => String(k.nome || '')).filter(Boolean)
  let npecas = 0
  for (const s of (man && man.secoes) || []) npecas += (s.pecas || []).length
  return {
    titulo: (man && man.titulo) || '',
    secoes,
    kernels,
    npecas,
    ndocs: ((man && man.docs) || []).length,
  }
}

export function parseVelOmissao (src) {
  const t = String(src || '')
  const vm = t.match(/\bv:\s*([0-9]+(?:\.[0-9]+)?)/)
  const tm = t.match(/\btocando:\s*(true|false)/)
  return {
    v: vm ? Number(vm[1]) : VEL_OMISSAO.v,
    tocando: tm ? tm[1] === 'true' : VEL_OMISSAO.tocando,
  }
}

/** Projecção observável: original ≡ motor numa sessão simples. */
export function estadoGkDe (opts = {}) {
  const velIn = opts.vel || VEL_OMISSAO
  return {
    tenant: opts.tenant || '',
    descricao: parseDescricaoGk(opts.man),
    sessao: {
      ancora: ancoraDeHash(opts.ancora),
      vel: { v: Number(velIn.v), tocando: !!velIn.tocando },
    },
    capacidades: CAPACIDADES_TARDAS.map((c) => c.id),
  }
}

export function igualEstadoGk (a, b) {
  if (!a || !b) return a === b
  if (a.tenant !== b.tenant) return false
  if ((a.sessao && a.sessao.ancora) !== (b.sessao && b.sessao.ancora)) return false
  const va = (a.sessao && a.sessao.vel) || VEL_OMISSAO
  const vb = (b.sessao && b.sessao.vel) || VEL_OMISSAO
  if (va.v !== vb.v || !!va.tocando !== !!vb.tocando) return false
  const da = a.descricao || {}
  const db = b.descricao || {}
  if (da.titulo !== db.titulo || da.npecas !== db.npecas || da.ndocs !== db.ndocs) return false
  const sa = da.secoes || []
  const sb = db.secoes || []
  if (sa.length !== sb.length || sa.some((x, i) => x !== sb[i])) return false
  const ka = da.kernels || []
  const kb = db.kernels || []
  if (ka.length !== kb.length || ka.some((x, i) => x !== kb[i])) return false
  const ca = a.capacidades || []
  const cb = b.capacidades || []
  if (ca.length !== cb.length || ca.some((x, i) => x !== cb[i])) return false
  return true
}

export function estadoPerp (a, b) {
  return String((a && a.tenant) || '') !== String((b && b.tenant) || '')
}

function nodoFatia (id, estatuto, evidencia, fonte, texto) {
  const n = {
    kind: 'ficheiro',
    id,
    sentido: 0,
    formato: 'json',
    estatuto,
    evidencia,
    fonte: fonte || '',
    proibicao: 'S_ESTADO != sessao != livro != coord != fuse; velEstado.v != GKBANCO.v',
  }
  if (texto != null && texto !== '') n.texto = String(texto)
  return completa(n)
}

export function estadoGkParaU (e) {
  const tenant = (e && e.tenant) || ''
  const ancora = (e && e.sessao && e.sessao.ancora) || ''
  const vel = (e && e.sessao && e.sessao.vel) || VEL_OMISSAO
  const d = (e && e.descricao) || {}
  const caps = (e && e.capacidades) || CAPACIDADES_TARDAS.map((c) => c.id)
  return completa({
    kind: 'pagina',
    id: ID_ESTADO_GK,
    sentido: 0,
    formato: 'json',
    camada: 'reino',
    estatuto: 'realizado',
    evidencia: 'main.js sem localStorage; manifesto=descricao; ancora=URL; velEstado efemero; tests/estado_gk_u.js',
    proibicao: 'S_ESTADO != sessao != livro != coord != fuse; nao fundir manifesto no fio; velEstado.v != GKBANCO.v; GLSL/LaTeX tardias; iframe oraculo',
    tenant,
    fonte: 'app/src/manifesto.json',
    nota: 'ancora=' + (ancora || 'top') + '; vel=' + vel.v + '; tocando=' + (vel.tocando ? '1' : '0'),
    slots: {
      descricao: 'realizado',
      sessao: 'realizado',
      capacidades: 'tardias',
    },
    filhos: [
      nodoFatia('descricao', 'realizado',
        'manifesto.json + kernels/substratos/pontes; nao e estado de sessao',
        'app/src/manifesto.json',
        JSON.stringify({
          titulo: d.titulo || '',
          secoes: d.secoes || [],
          kernels: d.kernels || [],
          npecas: d.npecas || 0,
          ndocs: d.ndocs || 0,
        })),
      nodoFatia('sessao', 'realizado',
        'ancora no URL (F1); velEstado efemero default 2×',
        './vel_estado.js',
        JSON.stringify({ ancora, vel })),
      nodoFatia('capacidades', 'nao localizada',
        'cards F3; LaTeX F4; GLSL/WASM F5 — listadas, nao executadas',
        '',
        JSON.stringify(caps)),
    ],
  })
}

export function uParaEstadoGk (u) {
  const filhos = (u && u.filhos) || []
  const td = (filhos.find((f) => f.id === 'descricao') || {}).texto
  const ts = (filhos.find((f) => f.id === 'sessao') || {}).texto
  const tc = (filhos.find((f) => f.id === 'capacidades') || {}).texto
  let descricao = { titulo: '', secoes: [], kernels: [], npecas: 0, ndocs: 0 }
  let ancora = ''
  let vel = { ...VEL_OMISSAO }
  let capacidades = CAPACIDADES_TARDAS.map((c) => c.id)
  try { if (td) descricao = { ...descricao, ...JSON.parse(td) } } catch { /* */ }
  try {
    if (ts) {
      const o = JSON.parse(ts)
      ancora = ancoraDeHash(o.ancora)
      if (o.vel) vel = { v: Number(o.vel.v), tocando: !!o.vel.tocando }
    }
  } catch { /* */ }
  try { if (tc) capacidades = JSON.parse(tc) } catch { /* */ }
  return {
    tenant: (u && u.tenant) || '',
    descricao,
    sessao: { ancora, vel },
    capacidades,
  }
}
