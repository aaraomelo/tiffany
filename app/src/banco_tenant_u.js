// banco_tenant_u.js — borda de hospedagem: host → tenant → rota.
// Nginx realiza; o motor cataloga a semântica. Nginx ≠ motor.
// tenant ≠ id ≠ K_i. Apex = ausência (não tenant=goldenkingdom).
// canal.patriatechnology.com = barramento, N/A como tenant GK.
// Disco: prefixo gk:t:{slug}: só sessão + livro + nav. S_ESTADO não se parte (F2: o Reino é outro objecto).

import { completa } from './banco_schema.js'

export const ID_BORDA = 'borda'
export const APEX_HOST = 'goldenkingdom.patriatechnology.com'
export const SUFIXO_TENANT = '.goldenkingdom.patriatechnology.com'
export const CANAL_HOST = 'canal.patriatechnology.com'
export const ROTAS_BORDA = Object.freeze(['/', '/banco/', '/canal'])
export const RESERVADOS = Object.freeze(['www', 'canal', 'mail', 'ftp', 'goldenkingdom'])
/** Nginx deriva do Host (PRODUCT.md / F0.5). O cliente não substitui a chave. */
export const CABECALHO_TENANT = 'X-Tenant'

const RE_SLUG = /^[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?$/

export function hostSemPorta (host) {
  const s = String(host || '').trim().toLowerCase()
  if (!s) return ''
  if (s.startsWith('[')) {
    const i = s.indexOf(']')
    return i >= 0 ? s.slice(1, i) : s
  }
  const i = s.lastIndexOf(':')
  if (i > 0 && /^\d+$/.test(s.slice(i + 1))) return s.slice(0, i)
  return s
}

export function slugOk (s) {
  const t = String(s || '').toLowerCase()
  return RE_SLUG.test(t) && !RESERVADOS.includes(t)
}

export function hostELocal (host) {
  const h = hostSemPorta(host)
  return !h || h === 'localhost' || h === '127.0.0.1' || h === '::1'
}

export function tenantDeHost (host) {
  const h = hostSemPorta(host)
  if (hostELocal(h)) {
    return { host: h, tenant: '', estatuto: 'ausencia' }
  }
  if (h === APEX_HOST) {
    return { host: h, tenant: '', estatuto: 'ausencia' }
  }
  if (h === CANAL_HOST) {
    return { host: h, tenant: '', estatuto: 'N/A', nota: 'barramento, nao tenant GK' }
  }
  if (h.endsWith(SUFIXO_TENANT)) {
    const slug = h.slice(0, -SUFIXO_TENANT.length)
    if (!slug || slug.includes('.') || !slugOk(slug)) {
      return { host: h, tenant: '', estatuto: 'nao localizada', nota: 'wildcard desconhecido' }
    }
    return { host: h, tenant: slug, estatuto: 'realizado' }
  }
  return { host: h, tenant: '', estatuto: 'nao localizada' }
}

export function rotaDaPath (path) {
  const p = String(path || '/').split('?')[0]
  if (p === '/canal' || p.startsWith('/canal/')) return '/canal'
  if (p === '/banco' || p.startsWith('/banco/')) return '/banco/'
  return '/'
}

export function prefixoDisco (tenant) {
  const t = String(tenant || '').trim().toLowerCase()
  if (!t) return ''
  return 'gk:t:' + t + ':'
}

export function chaveIsolada (k) {
  const s = String(k || '')
  return s === 'gk:banco:sessao' || s === 'gk:banco:nav' || s === 'gk:reino:card' ||
    s === 'gk:reino:latex' || s === 'gk:reino:glsl' || s === 'gk:reino:relogio' ||
    s === 'gk:reino:cena' || s === 'gk:reino:lei' ||
    s.startsWith('gk:banco:cristalchain:')
}

function valorCabecalho (headers, nome) {
  if (!headers) return ''
  if (typeof headers.get === 'function') {
    return String(headers.get(nome) || headers.get(nome.toLowerCase()) || '').trim()
  }
  const want = String(nome || '').toLowerCase()
  for (const k of Object.keys(headers)) {
    if (String(k).toLowerCase() === want) return String(headers[k] || '').trim()
  }
  return ''
}

/**
 * Host ganha. X-Tenant é o que o nginx emite a partir do Host;
 * no apex/local o cliente não impõe tenant (excepto localhost, para o medidor).
 */
export function tenantDeHeaders (headers, host) {
  const t = tenantDeHost(host)
  const raw = valorCabecalho(headers, CABECALHO_TENANT).toLowerCase()
  const hdr = slugOk(raw) ? raw : ''
  if (t.tenant) {
    if (hdr && hdr !== t.tenant) {
      return { ...t, estatuto: 'nao localizada', nota: 'X-Tenant != Host' }
    }
    return t
  }
  if (hdr && hostELocal(t.host || host)) {
    return { host: t.host, tenant: hdr, estatuto: 'realizado' }
  }
  return t
}

export function cabecalhosTenant (tenant) {
  const t = String(tenant || '').trim().toLowerCase()
  if (!t || !slugOk(t)) return {}
  return { [CABECALHO_TENANT]: t }
}

export function discoIsolado (storage, tenant) {
  const p = prefixoDisco(tenant)
  if (!p || !storage) return storage
  return {
    getItem (k) { return storage.getItem(chaveIsolada(k) ? p + k : k) },
    setItem (k, v) { storage.setItem(chaveIsolada(k) ? p + k : k, v) },
    removeItem (k) { storage.removeItem(chaveIsolada(k) ? p + k : k) },
  }
}

/** WSS no mesmo Host: o tenant persiste porque o cabeçalho Host é o subdomínio. */
export function canalDoHost (host, https) {
  const h = String(host || '').trim()
  if (!h) return null
  return (https ? 'wss://' : 'ws://') + h.replace(/\/$/, '') + '/canal'
}

export function bordaDe (host, path) {
  const t = tenantDeHost(host)
  return { host: t.host, tenant: t.tenant, estatuto: t.estatuto, nota: t.nota || '', rota: rotaDaPath(path) }
}

function estatutoU (b) {
  if ((b && b.estatuto) === 'N/A') return 'N/A'
  if ((b && b.estatuto) === 'realizado' || (b && b.estatuto) === 'ausencia') return 'realizado'
  return 'nao localizada'
}

export function tenantParaU (b) {
  const host = (b && b.host) || ''
  const tenant = (b && b.tenant) || ''
  const rota = (b && b.rota) || '/'
  const estatuto = estatutoU(b)
  const evidencia = estatuto === 'N/A'
    ? 'canal.patriatechnology.com e barramento; N/A como tenant GK'
    : (tenant
      ? 'subdominio ' + host + ' → tenant=' + tenant + ' → ' + rota
      : (estatuto === 'realizado'
        ? 'apex/ausencia; host=' + host + ' rota=' + rota
        : 'wildcard/host sem tenant valido: ' + host))
  return completa({
    kind: 'realizacao',
    id: ID_BORDA,
    sentido: 0,
    formato: 'json',
    camada: 'hospedagem',
    estatuto,
    evidencia,
    proibicao: 'tenant != id != K_i; Nginx != motor; wildcard != chave; ERP tenantId != tenant DNS',
    fonte: host,
    endereco: host,
    tenant,
    nota: tenant ? 'tenant=' + tenant + '; rota=' + rota : 'ausencia; rota=' + rota,
  })
}

export function uParaTenant (u) {
  const nota = (u && u.nota) || ''
  const rm = nota.match(/rota=(\/\S*)/)
  const tenant = (u && u.tenant) || ''
  let estatuto = 'ausencia'
  if ((u && u.estatuto) === 'N/A') estatuto = 'N/A'
  else if ((u && u.estatuto) === 'nao localizada') estatuto = 'nao localizada'
  else if (tenant) estatuto = 'realizado'
  return {
    host: (u && (u.fonte || u.endereco)) || '',
    tenant,
    estatuto,
    rota: rm ? rm[1] : '/',
  }
}

export function igualBorda (a, b) {
  return !!(a && b &&
    a.host === b.host &&
    a.tenant === b.tenant &&
    a.rota === b.rota &&
    a.estatuto === b.estatuto)
}
