// banco_sessao_u.js — sessão: qual realização está ligada (mesmo schema U).
// Solo = só M_wasm. Remoto = addr+pub → M_wasm e M_remota no mesmo U.
// Shells bash/powershell = línguas, não órbita Hopfield.

import { completa, nodoLingua } from './banco_schema.js'
import { hostELocal, slugOk, tenantDeHeaders } from './banco_tenant_u.js'

export const SHELLS_SESSAO = ['bash', 'powershell']

/** 32 bytes hex da ed25519 Patria (não o texto OpenSSH). app/banco/patria.json */
export const PATRIA_PUB = '2e8d3d16a9e66df19ab0bd9cf09cfa769b715321aa256ecaf9b97779d6953dc9'
/** Alias genérico do barramento (nginx). ≠ goldenkingdom. */
export const CANAL_ALIAS = 'canal.patriatechnology.com'

function addrOmissaoPatria () {
  if (typeof location === 'undefined' || !location.host) return null
  return (location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/canal'
}

function linguaDoManifesto (man, nome) {
  const L = (man && man.linguagens || []).find((l) => l.nome === nome)
  if (L) return L
  return { nome, faz: 'interpretar', p: 1, q: 1, r: 0, absorcao: { move: nome + '_move' } }
}

/** Query do browser: endereço do canal + chave pública do utilizador.
 * Tenant vem do Host (subdomínio). ?tenant= só em localhost — não substitui a chave. */
export function paramsDaSessao (search, hostArg, headers) {
  const q = String(search || (typeof location !== 'undefined' ? location.search : '') || '')
  const p = new URLSearchParams(q.startsWith('?') ? q : '?' + q)
  const patria = p.get('patria') === '1' || p.get('patria') === 'true'
  let pub = (p.get('pub') || p.get('chave') || '').trim()
  let addr = (p.get('addr') || p.get('endereco') || '').trim()
  const hostParam = (p.get('host') || '').trim()
  if (!addr && hostParam) {
    const https = typeof location !== 'undefined' && location.protocol === 'https:'
    const path = hostParam.includes('/') ? '' : '/canal'
    addr = (https ? 'wss://' : 'ws://') + hostParam.replace(/\/$/, '') + path
  }
  if (patria) {
    if (!pub) pub = PATRIA_PUB
    if (!addr) {
      const o = addrOmissaoPatria()
      if (o) addr = o
    }
  }
  const iRaw = p.get('i') || p.get('indice') || ''
  const indice = iRaw === '' ? null : (parseInt(iRaw, 10) | 0)
  const modo = p.get('modo') === 'fuse' ? 'fuse' : 'parte'
  const host = hostArg != null
    ? String(hostArg)
    : (typeof location !== 'undefined' ? location.host : '')
  const tHost = tenantDeHeaders(headers, host)
  let tenant = tHost.tenant
  const qTenant = (p.get('tenant') || '').trim().toLowerCase()
  if (!tenant && qTenant && hostELocal(host) && slugOk(qTenant)) tenant = qTenant
  return {
    endereco: addr || null,
    chave: pub || null,
    papel: (p.get('papel') || '').trim(),
    indice,
    patria,
    modo,
    tenant,
  }
}

/** Solo = só M_wasm se faltar endereço ou chave. Remoto = duas realizações, só com os dois. */
export function modoDaSessao (s) {
  const endereco = (s && s.endereco) || ''
  const chave = (s && s.chave) || ''
  return endereco && chave ? 'remoto' : 'solo'
}

export function urlCanal (endereco, omissao) {
  if (endereco) {
    if (/^wss?:\/\//i.test(endereco)) return endereco
    return (typeof location !== 'undefined' && location.protocol === 'https:' ? 'wss://' : 'ws://') +
      endereco.replace(/\/$/, '') + (endereco.includes('/') ? '' : '/canal')
  }
  return omissao || null
}

export function sessaoParaU (s, man) {
  const endereco = (s && s.endereco) || ''
  const chave = (s && s.chave) || ''
  const modo = modoDaSessao({ endereco, chave })
  return completa({
    kind: 'sessao',
    id: 'sessao',
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    evidencia: modo === 'solo'
      ? 'M_wasm: WASM + memoria + GKBANCO localStorage; canal nao ligado'
      : 'M_wasm e M_remota no mesmo U; canal S_BASH/S_PWSH; banda=sha256(chave); disco local + S_ESTADO',
    proibicao: 'bash/powershell != orbita Hopfield; js != node; fetch nao e canal',
    endereco,
    chave,
    modo,
    slots: { in: 9210, out: 9211, base: 'S_CANAL' },
    filhos: SHELLS_SESSAO.map((nome) => {
      const n = nodoLingua(linguaDoManifesto(man, nome))
      n.formato = nome === 'powershell' ? 'ps1' : 'sh'
      return n
    }),
  })
}

export function uParaSessao (u) {
  return {
    endereco: (u && u.endereco) || '',
    chave: (u && u.chave) || '',
  }
}
