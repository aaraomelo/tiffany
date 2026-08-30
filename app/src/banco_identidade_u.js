// banco_identidade_u.js — identidade operacional no schema U (tiffany://u).
// NÃO é realização da máquina M=(Exec,D,C). Catálogo ≠ manifesto ≠ órbita.
// π/F: banco_transf_u (fis:def:transf). Núcleo: fis:obs:U-consome, cat:nucleo-u.
// Hierarquia por força: chave > sessao > fingerprint (sinal) > oauth (censo).
// Star(U)=D. nao localizada ≠ N/A. Sem segundo $id. Sem additionalProperties.

import { completa } from './banco_schema.js'
import { parseChavePublica, bandaDeChavePublica, sha256, selaBlob, abreBlob } from './banda.js'
import {
  bandaDeId, selectorDoId, validaBanda, F, supp, chi, caractere,
} from './banco_transf_u.js'

export { bandaDeId, selectorDoId, validaBanda, F, supp, chi, caractere }

export { selaBlob, abreBlob }

/** Persistido no disco GKBANCO (LS; mesma ponte IDB se disco=idb → STORE_KV). */
export const CHAVE_SESSAO = 'gk:banco:sessao'

/** Força = prioridade. Em conflito a maior vence. oauth nunca escolhido. */
export const FORCA = Object.freeze({
  chave: 4,
  sessao: 3,
  fingerprint: 2,
  oauth: 1,
})

/**
 * Tabela de slots (espírito Fase A/B).
 * estatuto do censo: realizado | candidato | nao localizada.
 * No nodo U, candidato → estatuto='nao localizada' + nota='candidato'
 * (enum do schema não tem candidato; hopfield no manifesto usa a mesma distinção).
 */
export const SLOTS_IDENTIDADE = Object.freeze([
  {
    id: 'chave',
    forca: FORCA.chave,
    estatuto: 'realizado',
    evidencia: '?pub= + TIFFANY_PUB; banda=sha256(bytes da chave), nao UTF-8 do hex; id=hex da banda',
    proibicao: 'identidade != Exec/D/C; chave atravessa LS/IDB/Docker sem ser realizacao',
    nota: 'canonica operacional',
  },
  {
    id: 'sessao',
    forca: FORCA.sessao,
    estatuto: 'realizado',
    evidencia: 'UUID gk:banco:sessao no primeiro acesso; bind(t,de,para) sessao→chave',
    proibicao: 'sessao != soberana; upgrade so por bind explicito',
    nota: 'efemera / nao soberana',
  },
  {
    id: 'fingerprint',
    forca: FORCA.fingerprint,
    estatuto: 'nao localizada',
    evidencia: 'sinal de dispositivo; hash do que o ambiente der; similarity nao igualdade; sem medidor de binding',
    proibicao: 'fingerprint != login; nao vence chave; sem user-agent+resolucao como unica prova',
    nota: 'candidato',
  },
  {
    id: 'oauth',
    forca: FORCA.oauth,
    estatuto: 'nao localizada',
    evidencia: 'censo; Google/Apple nao implementados; bootstrap de chave, nunca unica forma; nao e N/A',
    proibicao: 'oauth != N/A; nao e ausencia demonstrada; sem implementar terceiros',
    nota: 'censo',
  },
])

/** Bind actual = associação no estado. Binding assinado pela privada = outro slot. */
export const NATUREZA_BIND = 'soft'

/**
 * Ciclo de vida da chave (não reabre o contrato criptográfico).
 * banda (fio) ≡ id (sujeito): hex(sha256(bytes(pub))), sem prefixo gk:id:v1.
 */
export const SLOTS_CICLO = Object.freeze([
  {
    id: 'bootstrap',
    estatuto: 'realizado',
    evidencia: '?pub= ou importacao da pub; disco LS + bind de sessao se UUID existia; entre maquinas D_patria opaco (S_DEPOSITO)',
    proibicao: 'oauth != autoridade de id; Patria nao le claro; sem Google/Apple',
    nota: 'operacional',
  },
  {
    id: 'bind',
    estatuto: 'realizado',
    evidencia: 'natureza=soft — associacao no estado {t,de,para}; binding cripto (assinatura pela privada) = candidato',
    proibicao: 'bind soft != prova criptografica; nao promover a assinado sem medidor',
    nota: 'soft',
  },
  {
    id: 'rotacao',
    estatuto: 'nao localizada',
    evidencia: 'transicao auditavel {t,de:id_old,para:id_new}; sem privada da old nao ha prova',
    proibicao: 'rotacao sem prova != mesmo sujeito; nao inventar autoridade de terceiros',
    nota: 'candidato',
  },
  {
    id: 'recuperacao',
    estatuto: 'nao localizada',
    evidencia: 'sem protocolo de shares; sem a privada ninguem le o deposito; nova chave = novo sujeito; terceiros nao redefinem id',
    proibicao: 'recuperacao != N/A; oauth/google/apple != autoridade; nao e ausencia demonstrada de shares',
    nota: '',
  },
  {
    id: 'privada_wasm',
    estatuto: 'nao localizada',
    evidencia: 'candidato WebCrypto/memoria; sem medidor; diferenca nao vaza para resolveIdentidade',
    proibicao: 'privada_wasm != camada de identidade; sem promover IDB/WebCrypto',
    nota: 'candidato',
  },
  {
    id: 'privada_docker',
    estatuto: 'nao localizada',
    evidencia: 'sem medidor; tratamento de realizacao nao entra na identidade',
    proibicao: 'privada_docker != resolveIdentidade; diferenca nao vaza; sem promover Docker',
    nota: '',
  },
])

/** Par S_DEPOSITO — blob opaco. ≠ S_ESTADO 9210/9211 (JSON GKBANCO). */
export const SLOTS_DEPOSITO = Object.freeze({
  in: 9220,
  out: 9221,
  base: 'S_CANAL',
  disco: 'gk/banco/deposito.bin',
  nota: 'D_patria deposita bytes; sem a banda o blob nao e JSON GKBANCO',
})

export function tabelaSlots () {
  return SLOTS_IDENTIDADE.map((s) => ({
    id: s.id,
    forca: s.forca,
    estatuto: s.nota === 'candidato' ? 'candidato' : s.estatuto,
    evidencia: s.evidencia,
  }))
}

export function tabelaCiclo () {
  return SLOTS_CICLO.map((s) => ({
    id: s.id,
    estatuto: s.nota === 'candidato' ? 'candidato' : s.estatuto,
    evidencia: s.evidencia,
    nota: s.nota || '',
  }))
}

export function hexBanda (banda) {
  return [...banda].map((b) => b.toString(16).padStart(2, '0')).join('')
}

/**
 * Identificador estável = hex da banda (sha256 dos bytes), não UTF-8 do hex.
 * banda (fio do barramento) ≡ id (ciclo de vida). Sem prefixo gk:id:v1 —
 * o canal já usa sha256(bytes) sem domain separator (bandaDeChavePublica).
 */
export async function idEstavelDaChave (pub) {
  return hexBanda(await bandaDeChavePublica(pub))
}

/** OAuth é transporte/censo. Nunca define id. */
export function oauthDefineId () {
  return false
}

export function idPorOAuth (_censo) {
  return null
}

/** Blob em claro é GKBANCO parseável? Sem a banda o depósito não deve ser. */
export function eJsonGKBANCO (bytes) {
  try {
    const t = typeof bytes === 'string' ? bytes : new TextDecoder().decode(bytes)
    const o = JSON.parse(t)
    return !!(o && o.magia === 'GKBANCO')
  } catch {
    return false
  }
}

function chaveNormal (s) {
  const t = String(s || '').trim()
  if (!t || !parseChavePublica(t)) return ''
  return t.replace(/[\s:]+/g, '').toLowerCase()
}

/**
 * Resolução determinística: mesma evidência → mesma camada.
 * Chave vence sessão. Fingerprint nunca vence chave. OAuth não é usado.
 * privada_wasm / privada_docker NÃO entram — Identidade ⊥ Realização.
 */
export function resolveIdentidade (evidencias) {
  const ev = evidencias || {}
  const chave = chaveNormal(ev.chave)
  const sessao = String(ev.sessao || '').trim()
  const fingerprint = ev.fingerprint ? String(ev.fingerprint) : null
  if (chave) {
    return { camada: 'chave', chave, sessao: sessao || null, fingerprint, forca: FORCA.chave }
  }
  if (sessao) {
    return { camada: 'sessao', chave: null, sessao, fingerprint, forca: FORCA.sessao }
  }
  return { camada: null, chave: null, sessao: null, fingerprint, forca: 0 }
}

export function uuidNovo () {
  if (globalThis.crypto && typeof globalThis.crypto.randomUUID === 'function') {
    return globalThis.crypto.randomUUID()
  }
  const b = new Uint8Array(16)
  if (globalThis.crypto && typeof globalThis.crypto.getRandomValues === 'function') {
    globalThis.crypto.getRandomValues(b)
  } else {
    for (let i = 0; i < 16; i++) b[i] = (Math.random() * 256) | 0
  }
  b[6] = (b[6] & 0x0f) | 0x40
  b[8] = (b[8] & 0x3f) | 0x80
  const h = hexBanda(b)
  return h.slice(0, 8) + '-' + h.slice(8, 12) + '-' + h.slice(12, 16) + '-' + h.slice(16, 20) + '-' + h.slice(20)
}

export function leRegistoSessao (storage) {
  if (!storage) return { uuid: null, binds: [], rotacoes: [] }
  try {
    const s = storage.getItem(CHAVE_SESSAO)
    if (!s) return { uuid: null, binds: [], rotacoes: [] }
    const o = JSON.parse(s)
    return {
      uuid: typeof o.uuid === 'string' && o.uuid ? o.uuid : null,
      binds: Array.isArray(o.binds) ? o.binds.map(normalizaBind).filter(Boolean) : [],
      rotacoes: Array.isArray(o.rotacoes) ? o.rotacoes.map(normalizaBind).filter(Boolean) : [],
    }
  } catch {
    return { uuid: null, binds: [], rotacoes: [] }
  }
}

function normalizaBind (b) {
  if (!b || typeof b !== 'object') return null
  const de = String(b.de || '')
  const para = String(b.para || '')
  const t = String(b.t || '')
  if (!de || !para || !t) return null
  return { t, de, para, natureza: b.natureza === 'assinado' ? 'assinado' : NATUREZA_BIND }
}

export function gravaRegistoSessao (storage, rec) {
  if (!storage) return
  try {
    storage.setItem(CHAVE_SESSAO, JSON.stringify({
      uuid: (rec && rec.uuid) || null,
      binds: Array.isArray(rec && rec.binds) ? rec.binds : [],
      rotacoes: Array.isArray(rec && rec.rotacoes) ? rec.rotacoes : [],
    }))
  } catch { /* quota */ }
}

/** Cria UUID no primeiro acesso; relê se já existir. Disco GKBANCO. */
export function ligaSessao (storage) {
  const rec = leRegistoSessao(storage)
  if (rec.uuid) return rec.uuid
  rec.uuid = uuidNovo()
  gravaRegistoSessao(storage, rec)
  return rec.uuid
}

/** Transição explícita sessão→chave. Natureza=soft até haver assinatura. */
export function bind (sessao, chave, storage, opts = {}) {
  const de = String(sessao || '').trim()
  const para = chaveNormal(chave)
  if (!de || !para) throw new Error('bind pede sessao e chave')
  const lig = {
    t: opts.t || new Date().toISOString(),
    de,
    para,
    natureza: NATUREZA_BIND,
  }
  if (storage) {
    const rec = leRegistoSessao(storage)
    rec.binds = rec.binds || []
    rec.binds.push(lig)
    if (!rec.uuid) rec.uuid = de
    gravaRegistoSessao(storage, rec)
  }
  return lig
}

/**
 * Rotação auditável id_old → id_new. Sem privada da old não há prova
 * (slot = candidato). Não muda resolveIdentidade — a nova pub tem de ser apresentada.
 */
export function rodaIdentidade (idOld, idNew, storage, opts = {}) {
  const de = String(idOld || '').trim()
  const para = String(idNew || '').trim()
  if (!de || !para) throw new Error('rotacao pede id_old e id_new')
  const lig = {
    t: opts.t || new Date().toISOString(),
    de,
    para,
    natureza: NATUREZA_BIND,
  }
  if (storage) {
    const rec = leRegistoSessao(storage)
    rec.rotacoes = rec.rotacoes || []
    rec.rotacoes.push(lig)
    gravaRegistoSessao(storage, rec)
  }
  return lig
}

/** Sem shares: recuperação = nao localizada. Terceiros não redefinem id. */
export function protocoloRecuperacao () {
  const s = SLOTS_CICLO.find((x) => x.id === 'recuperacao')
  return {
    estatuto: s.estatuto,
    evidencia: s.evidencia,
    proibicao: s.proibicao,
  }
}

/**
 * Nova chave = novo sujeito. Não restaura o depósito (ilegível sem a privada old).
 * OAuth não entra.
 */
export async function novoSujeito (storage, pub) {
  const chave = chaveNormal(pub)
  if (!chave) throw new Error('novo sujeito pede pub')
  return bootstrapRealizacao(storage, { chave })
}

/** Nova realização: ?pub= ou importação; LS + bind se já havia UUID. */
export async function bootstrapRealizacao (storage, evidencias = {}) {
  return ligaIdentidade(storage, { chave: evidencias.chave || evidencias.pub })
}

export async function importaPub (storage, pub) {
  return bootstrapRealizacao(storage, { chave: pub })
}

/**
 * Sinais disponíveis — sem fingerprinting agressivo.
 * Critérios: hardwareConcurrency, timezone+idioma; webgl/canvas/audio/fonts só se já vierem.
 * Sem user-agent, sem resolução de ecrã.
 */
export function sinaisDoAmbiente (env = globalThis) {
  const s = {}
  const nav = env && env.navigator
  if (nav) {
    if (typeof nav.hardwareConcurrency === 'number') s.hardwareConcurrency = nav.hardwareConcurrency
    if (nav.language) s.language = String(nav.language)
    if (Array.isArray(nav.languages) && nav.languages.length) {
      s.languages = nav.languages.slice().map(String)
    }
  }
  try {
    const tz = Intl.DateTimeFormat().resolvedOptions().timeZone
    if (tz) s.timezone = tz
  } catch { /* */ }
  if (env && env.webglRenderer) s.webgl = String(env.webglRenderer)
  if (env && env.canvasHash) s.canvas = String(env.canvasHash)
  if (env && env.audioHash) s.audio = String(env.audioHash)
  if (env && Array.isArray(env.fonts) && env.fonts.length) {
    s.fonts = env.fonts.slice().map(String).sort()
  }
  return s
}

function jsonEstavel (o) {
  if (o === null || typeof o !== 'object') return JSON.stringify(o)
  if (Array.isArray(o)) return '[' + o.map(jsonEstavel).join(',') + ']'
  return '{' + Object.keys(o).sort().map((k) => JSON.stringify(k) + ':' + jsonEstavel(o[k])).join(',') + '}'
}

/** Hash estável do que o ambiente der. Estatuto do slot: candidato. */
export async function hashFingerprint (sinais) {
  const bytes = new TextEncoder().encode(jsonEstavel(sinais || {}))
  return hexBanda(await sha256(bytes))
}

/**
 * Front/terminal: sem pub → sessão local; com pub → identidade=chave
 * e bind só se já havia sessão (upgrade auditável).
 */
export async function ligaIdentidade (storage, evidencias = {}) {
  const tinhaSessao = !!leRegistoSessao(storage).uuid
  const uuid = ligaSessao(storage)
  const r = resolveIdentidade({
    chave: evidencias.chave,
    sessao: uuid,
    fingerprint: evidencias.fingerprint || null,
  })
  let ligacao = null
  if (r.camada === 'chave' && tinhaSessao) {
    ligacao = bind(uuid, r.chave, storage)
  }
  const id = r.camada === 'chave'
    ? await idEstavelDaChave(r.chave)
    : (r.camada === 'sessao' ? r.sessao : null)
  return { ...r, id, uuid, bind: ligacao }
}

function nodoSlot (s, resolucao) {
  const slots = {}
  if (s.forca != null) slots.forca = String(s.forca)
  if (s.nota) slots.nota = s.nota
  const n = {
    kind: 'sessao',
    id: s.id,
    sentido: 0,
    formato: 'json',
    estatuto: s.estatuto,
    evidencia: s.evidencia,
    proibicao: s.proibicao,
    slots,
  }
  if (s.nota) n.nota = s.nota
  if (s.id === 'chave' && resolucao && resolucao.chave) n.chave = resolucao.chave
  return completa(n)
}

/** Identidade → U. kind=sessao (kind existente). Não é kind=realizacao. */
export function identidadeParaU (resolucao, opts = {}) {
  const r = resolucao || {}
  const slots = {}
  for (const s of SLOTS_IDENTIDADE) {
    slots[s.id] = s.nota === 'candidato' ? 'candidato' : s.estatuto
  }
  for (const s of SLOTS_CICLO) {
    slots[s.id] = s.nota === 'candidato' ? 'candidato' : s.estatuto
  }
  const evidencia = r.camada === 'chave'
    ? 'identidade=chave; banda=sha256(bytes); id=hex(banda); sem gk:id:v1; atravessa realizacoes sem depender do disco'
    : (r.camada === 'sessao'
      ? 'identidade=sessao; UUID gk:banco:sessao; nao soberana'
      : 'sem evidencia de chave nem sessao')
  const n = {
    kind: 'sessao',
    id: 'identidade',
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    evidencia,
    proibicao: 'identidade != Exec/D/C; fingerprint != login; oauth != unica forma; catalogo != manifesto != orbita; Identidade != Realizacao != Estado',
    chave: r.chave || '',
    modo: opts.modo === 'remoto' ? 'remoto' : 'solo',
    slots,
    filhos: [...SLOTS_IDENTIDADE, ...SLOTS_CICLO].map((s) => nodoSlot(s, r)),
  }
  if (opts.endereco) n.endereco = String(opts.endereco)
  return completa(n)
}

export function uParaIdentidade (u) {
  return {
    chave: (u && u.chave) || '',
    camada: (u && u.chave) ? 'chave' : null,
    slots: (u && u.slots) ? { ...u.slots } : {},
  }
}
