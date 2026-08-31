// banco_inventario_u.js — mapa do Reino: cards ≠ kernels. Sem GLSL, sem execução.
// F3 = o que é um card. Inventário = o que declara (op + legenda). Semântica = K realiza lei.
// F8 = como entra em cena — ainda não. Não um F por demonstração.
// op ≠ régua ≠ tag ≠ fórmula do desc. As leis estão na legenda.

import { completa } from './banco_schema.js'
import { catalogoCards, PECAS_NOMEADAS } from './banco_cards_u.js'

export const ID_INVENTARIO_GK = 'gk'
/** Etapas anteriores à ingestão em cena. F8 não corre aqui. */
export const ETAPAS = Object.freeze(['F3', 'inventario', 'semantica', 'lei', 'F8'])
/** Tomografia: extrair a máquina que já corre, não projectar outra. */
export const METODO = Object.freeze(['observar', 'relacionar', 'provar', 'promover'])
export const MIGRAR_POR = 'equivalencia observavel de instrumento e lei'
export const PROVA_PROMOCAO = 'o instrumento novo realiza as mesmas leis observaveis'
export const CHIPS_HERO = Object.freeze(['aura', 'galaxia', 'floco', 'ferramenta', 'pulso', 'julia'])
/** Fórmula do instrumento aura, escrita 12 vezes no desc (corte+elenco). */
export const FASE_AURA = 'fase = f·log₂r + a·χ·cos(nθ)'
export const ROT_REGUA = 'régua'
export const ROT_FP = 'fator de potência'
export const ROT_REVELA = 'revela'

export function fatiaPecasArt (src) {
  const i = String(src).lastIndexOf('"pecas":')
  if (i < 0) return {}
  return JSON.parse('{' + src.slice(i)).pecas || {}
}

/** pintor + mapa, sem o objecto kernels (GLSL). */
export function fatiaPintorMapa (src) {
  const s = String(src)
  const a = s.indexOf('"pintor":')
  const b = s.lastIndexOf('"pecas":')
  if (a < 0 || b < 0 || b <= a) return { pintor: {}, mapa: {} }
  const o = JSON.parse('{' + s.slice(a, b) + '"_":0}')
  return { pintor: o.pintor || {}, mapa: o.mapa || {} }
}

export function camposPeca (p) {
  const skip = new Set(['kernel', 'secao', 'titulo', 'stops'])
  return Object.keys(p || {}).filter((k) => !skip.has(k)).sort()
}

export function tagsDoPeca (p) {
  return ((p && p.tags) || []).map((t) => ({
    rot: String((t && t.rot) || ''),
    val: String((t && t.val) || ''),
  }))
}

export function faseNaDesc (desc) {
  const m = String(desc || '').match(/fase\s*=\s*[^.]+/)
  return m ? m[0].trim() : ''
}

function norma (s) {
  return String(s || '').replace(/\s+/g, ' ').trim()
}

/** Legendas do manifesto: tags + fórmula no desc. Não é a lista de ops. */
export function legendasDe (man) {
  const pecas = []
  for (const s of (man && man.secoes) || []) {
    for (const p of s.pecas || []) {
      pecas.push({
        secao: String(s.id || ''),
        nome: String(p.nome || ''),
        op: String(p.op || ''),
        desc: String(p.desc || ''),
        tags: tagsDoPeca(p),
      })
    }
  }
  const rots = {}
  const pares = new Set()
  const reguas = new Set()
  const fps = new Set()
  const revelas = new Set()
  let iguais = 0
  let diferentes = 0
  let semRegua = 0
  let nTags = 0
  const fases = []
  for (const p of pecas) {
    nTags += p.tags.length
    const regua = p.tags.find((t) => t.rot === ROT_REGUA)
    if (!regua) semRegua++
    else {
      const v = norma(regua.val)
      reguas.add(v)
      if (v === norma(p.op)) iguais++
      else diferentes++
    }
    for (const t of p.tags) {
      rots[t.rot] = (rots[t.rot] || 0) + 1
      pares.add(t.rot + '\t' + t.val)
      if (t.rot === ROT_FP) fps.add(t.val)
      if (t.rot === ROT_REVELA) revelas.add(t.val)
    }
    const f = faseNaDesc(p.desc)
    if (f) fases.push({ nome: p.nome, secao: p.secao, fase: f })
  }
  return {
    nCards: pecas.length,
    nTags,
    nRots: Object.keys(rots).length,
    nPares: pares.size,
    nReguas: reguas.size,
    nFP: fps.size,
    nRevela: revelas.size,
    nFaseDesc: fases.length,
    nFaseUniq: new Set(fases.map((x) => x.fase)).size,
    faseAura: FASE_AURA,
    cardsComTags: pecas.filter((p) => p.tags.length).length,
    cardsComFP: pecas.filter((p) => p.tags.some((t) => t.rot === ROT_FP)).length,
    reguaVsOp: { iguais, diferentes, semRegua },
    revelas: [...revelas].sort(),
    fases,
  }
}

export function fichaKernel (nome, pecas, pintor, mapa, cards) {
  const kn = String(nome || '')
  const pin = (pintor && pintor[kn]) || {}
  const mp = (mapa && mapa[kn]) || {}
  const usados = []
  const secoes = new Set()
  const campos = new Set()
  for (const [id, p] of Object.entries(pecas || {})) {
    if (String(p.kernel || '') !== kn) continue
    usados.push(id)
    if (p.secao) secoes.add(String(p.secao))
    for (const c of camposPeca(p)) campos.add(c)
  }
  const realizacoes = (cards || []).filter((c) => c.kernel === kn)
  const ops = [...new Set(realizacoes.map((c) => String(c.op || '')).filter(Boolean))]
  const nFaseDesc = realizacoes.filter((c) => faseNaDesc(c.desc)).length
  const entradas = Object.entries(mp).map(([uni, pair]) => ({
    uniforme: uni,
    campo: Array.isArray(pair) ? pair[0] : '',
    tipo: Array.isArray(pair) ? pair[1] : '',
  }))
  return {
    kernel: kn,
    n: usados.length,
    dim: pin.lim ? '2d' : '3d',
    lim: pin.lim || null,
    chip: CHIPS_HERO.includes(kn),
    escalaA: pin.escala_a || null,
    secoes: [...secoes].sort(),
    campos: [...campos].sort(),
    cards: realizacoes.map((c) => c.id),
    ops,
    nFaseDesc,
    entradas,
    relogio: true,
    wasm: 'painel_motor',
    glsl: false,
  }
}

/** K realiza op — uma aresta por lei declarada nos cards desse instrumento. */
export function realiza (ficha) {
  const k = (ficha && ficha.kernel) || ''
  return ((ficha && ficha.ops) || []).map((op) => ({ kernel: k, op }))
}

function camposBase (ficha) {
  return ((ficha && ficha.campos) || []).filter((c) => c !== 'gemeas' && !/B$/.test(c))
}

/** Relações observadas entre instrumentos — não categorias. */
export function relacoesKernels (fichas) {
  const lista = fichas || []
  const by = Object.fromEntries(lista.map((k) => [k.kernel, k]))
  const out = []
  const aura = by.aura
  const baseAura = aura ? camposBase(aura).join(',') : ''
  for (const k of lista) {
    if (!aura || k.kernel === 'aura') continue
    if (camposBase(k).join(',') === baseAura && baseAura) {
      out.push({ de: k.kernel, para: 'aura', tipo: 'entradas', evidencia: baseAura })
    }
  }
  if (by.venom_rev && aura) {
    out.push({ de: 'venom_rev', para: 'aura', tipo: 'chi', evidencia: 'mesmas entradas; corte_negra' })
  }
  if (by.costura && (by.costura.campos || []).some((c) => /B$/.test(c))) {
    out.push({
      de: 'costura',
      para: 'costura',
      tipo: 'dual_AB',
      evidencia: (by.costura.campos || []).filter((c) => /B$/.test(c)).join(','),
    })
  }
  const luz = ['spec', 'shine', 'ao', 'fres', 'shadow', 'refl']
  if (by.textura && luz.every((c) => (by.textura.campos || []).includes(c))) {
    out.push({ de: 'textura', para: 'textura', tipo: 'parametros_luz', evidencia: luz.join(',') })
  }
  if (by.aura && by.aura.nFaseDesc) {
    out.push({
      de: 'aura',
      para: 'aura',
      tipo: 'fase_desc',
      evidencia: FASE_AURA + ' ×' + by.aura.nFaseDesc,
    })
  }
  return out
}

export function inventarioDe (man, pecas, pintor, mapa) {
  const cards = catalogoCards(man, pecas)
  const nomesK = [...new Set(Object.values(pecas || {}).map((p) => String(p.kernel || '')).filter(Boolean))].sort()
  const kernels = nomesK.map((k) => fichaKernel(k, pecas, pintor, mapa, cards)).sort((a, b) => b.n - a.n)
  const nomeados = cards.filter((c) => !c.kernel).map((c) => ({
    nome: c.nome,
    secao: c.secao,
    titulo: c.titulo,
    op: c.op,
  }))
  const ops = new Set(cards.map((c) => c.op).filter(Boolean))
  const arestas = kernels.flatMap(realiza)
  const opsNomeadas = nomeados.map((n) => n.op).filter(Boolean)
  const legendas = legendasDe(man)
  return {
    nCards: cards.length,
    nKernels: kernels.length,
    nNomeados: nomeados.length,
    nOps: ops.size,
    nArestas: arestas.length,
    nSecoes: ((man && man.secoes) || []).length,
    nTags: legendas.nTags,
    nReguas: legendas.nReguas,
    kernels,
    relacoes: relacoesKernels(kernels),
    arestas,
    nomeados,
    chips: CHIPS_HERO.slice(),
    opsNomeadas,
    legendas,
    secoes: ((man && man.secoes) || []).map((s) => ({
      id: String(s.id || ''),
      titulo: String(s.titulo || ''),
      n: (s.pecas || []).length,
    })),
  }
}

export function igualInventario (a, b) {
  if (!a || !b) return a === b
  return a.nCards === b.nCards && a.nKernels === b.nKernels && a.nNomeados === b.nNomeados &&
    a.nOps === b.nOps && a.nArestas === b.nArestas &&
    a.nTags === b.nTags && a.nReguas === b.nReguas
}

export function inventarioParaU (inv) {
  const I = inv || { nCards: 0, nKernels: 0, nNomeados: 0, kernels: [], nomeados: [] }
  return completa({
    kind: 'pagina',
    id: ID_INVENTARIO_GK,
    sentido: 0,
    formato: 'json',
    camada: 'descricao',
    estatuto: 'realizado',
    evidencia: 'manifesto.secoes.pecas (op+tags+desc); indice pecas.kernel (nao o GLSL); pintor+mapa; K realiza lei da legenda; cards sao evidencia da maquina, nao a maquina; tests/inventario_u.js',
    proibicao: 'observar→relacionar→provar→promover; op != regua != tag != formula do desc; nao inventar categoria; nao migrar por aparencia/arquivo/card; prova de promocao = mesmas leis observaveis, nao portar shader; F8 ainda nao',
    nota: 'nCards=' + I.nCards + '; nKernels=' + I.nKernels + '; nNomeados=' + I.nNomeados +
      '; nOps=' + (I.nOps || 0) + '; nArestas=' + (I.nArestas || 0) +
      '; nTags=' + (I.nTags || 0) + '; nReguas=' + (I.nReguas || 0) +
      '; etapas=' + ETAPAS.join('>') + '; metodo=' + METODO.join('>') +
      '; migrar=' + MIGRAR_POR + '; chips=' + (I.chips || CHIPS_HERO).join(','),
    slots: {
      cards: 'realizado',
      kernels: 'realizado',
      glsl: 'N/A',
      execucao: 'N/A',
    },
    filhos: (I.kernels || []).map((k) => completa({
      kind: 'ficheiro',
      id: k.kernel,
      sentido: 0,
      formato: 'json',
      estatuto: 'realizado',
      fonte: k.kernel,
      evidencia: 'n=' + k.n + '; nOps=' + (k.ops || []).length + '; dim=' + k.dim + '; chip=' + k.chip,
      proibicao: 'nao ingerir source GLSL; compile e pelo nome (F5)',
    })),
  })
}

export function uParaInventario (u) {
  const nota = (u && u.nota) || ''
  const nc = nota.match(/nCards=([0-9]+)/)
  const nk = nota.match(/nKernels=([0-9]+)/)
  return {
    nCards: nc ? Number(nc[1]) : 0,
    nKernels: nk ? Number(nk[1]) : ((u && u.filhos) || []).length,
    filhos: ((u && u.filhos) || []).map((f) => f.id),
  }
}

export { PECAS_NOMEADAS }
