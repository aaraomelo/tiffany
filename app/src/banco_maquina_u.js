// banco_maquina_u.js — projecção da máquina mínima no schema U (tiffany://u).
// M = (Exec, D, C). M_min = (WASM, D_*, Memória local). C = MOVE: não entra
// M_Docker = nao localizada (univ:def:maquina; cat:nucleo-u). Sem fingir contentor.
// no iso de disco; invariante nas fases A e B (S_ESTADO 9210/9211, JSON GKBANCO).
// Homónimo: kind realizacao = instância da máquina (no catálogo = Parte realizada).
// Mesmo $id. Discos = filhos com suporte=disco. U não decide capítulos.

import { completa } from './banco_schema.js'
import { estadoVazio, normalizaEstado, gravaShell, leEstado, gravaEstado } from './banco_disco.js'

export const SLOTS_ESTADO = { in: 9210, out: 9211, base: 'S_CANAL' }

/** Decomposição canónica. C não entra no iso D_LS ≅ D_IDB. */
export const SLOTS_M = { Exec: 'WASM', D: 'disco', C: 'canal' }
export const SLOTS_M_MIN = { Exec: 'WASM', D: 'D_*', Memoria: 'local' }

/** JSON vazio que a realização remota declara (canal_patria.c / gk/banco/estado.json). */
export const ESTADO_VAZIO_REMOTO =
  '{"magia":"GKBANCO","v":2,"shells":{},"atualizado":null,"pendente":[]}'

export const REALIZACOES = [
  {
    id: 'wasm',
    estatuto: 'realizado',
    suporte: 'wasm',
    evidencia: 'M_WASM+LS — consultar.wasm + GKBANCO localStorage (memoriaLS em Node); linha de base',
    fonte: 'app/src/banco_disco.js',
    slots: { Exec: 'WASM', D: 'localStorage' },
    nota: 'M_min sob D_LS',
  },
  {
    id: 'remota',
    estatuto: 'realizado',
    suporte: 'canal',
    evidencia: 'S_ESTADO 9210/9211; gk/banco/estado.json; mesmo JSON GKBANCO',
    fonte: 'banco/canal_patria.c',
    slots: { ...SLOTS_ESTADO },
    nota: 'C = MOVE; nao entra no iso de disco',
  },
  {
    id: 'wasm+idb',
    estatuto: 'nao localizada',
    suporte: 'idb',
    evidencia: 'candidato 1 — Fase A no shim (iso+hash+ordem+custo IO); falha parcial mapeada no disco IDB; boot default = LS; sem paridade em producao; custo != correcao; nao promover',
    fonte: 'app/src/banco_disco.js',
    slots: { Exec: 'WASM', D: 'IndexedDB' },
    nota: 'candidato 1',
    proibicao: 'IDB nao substitui LS no boot; quota LS nao muda a semantica; sem promocao da maquina sem S1 no browser',
  },
  {
    id: 'docker',
    estatuto: 'nao localizada',
    suporte: 'docker',
    evidencia: 'candidato 2 — Fase B ingestao (fis:def:objeto; fis:thm:troca-realizacao sobre Exec, apos D); M=(docker, volume, C); C=S_ESTADO 9210/9211 invariante; Exec isolado: mesmo S0+I (sql_move em consultar.wasm / gravaShell t fixo) + D=volume + hashCanonico; Exec nativo no volume medido; banco/Dockerfile.volume = objecto de ingestao (imagem+VOLUME /gk/banco); sem privileged; sem host network; S_DEPOSITO 9220/9221 != S_ESTADO; Identidade intocada (resolveIdentidade ignora Exec; nenhum nodo redefine id); par WASM/volume preparado, sem paridade de contentor medida; sem cold-start inventado; nao promover',
    fonte: 'banco/Dockerfile.volume',
    slots: { Exec: 'docker', D: 'volume', C: 'canal' },
    nota: 'candidato 2',
    proibicao: 'docker != metal != runtime do banco; docker-compose nao e o runtime; nao e «o metal»; sem cold-start inventado; sem privileged; sem host network; S_DEPOSITO != S_ESTADO; nao redefine id',
  },
  {
    id: 'mongo',
    estatuto: 'nao localizada',
    suporte: 'mongo',
    evidencia: 'candidato 3 — censo; documental+indices+BSON; sem driver; bijecao BSON↔U sem medidor',
    fonte: null,
    nota: 'candidato 3',
    proibicao: 'mongo != disco; sem mongoose; BSON nao e GKBANCO',
  },
]

export const DISCOS = [
  {
    id: 'localStorage',
    estatuto: 'realizado',
    evidencia: 'GKBANCO/GKCORPO — disco canonico da realizacao wasm (linha de base)',
    slots: { D: 'LS' },
  },
  {
    id: 'estado.json',
    estatuto: 'realizado',
    evidencia: 'gk/banco/estado.json — disco da realizacao remota',
    slots: { D: 'remoto' },
  },
  {
    id: 'IndexedDB',
    estatuto: 'realizado',
    evidencia: 'mapeamento gk:banco:* → object stores; ponte isomorfa atras de opts.disco=idb; iso S0+I no shim; boot default = LS; falha parcial: LS QuotaExceeded = setItem sem escrita e gravaEstado corta/retenta; IDB abort = setItem sem escrita (rollback do write-through, AbortError); observavel da API = valor anterior; corte/retenta so no LS; custo IO no medidor (medeIO); custo != correcao',
    fonte: 'app/src/banco_disco.js',
    slots: { D: 'IDB' },
    nota: 'persistencia alternativa; nao e a maquina M_WASM+IDB',
    proibicao: 'disco IDB != promocao de M_WASM+IDB; LS continua a linha de base',
  },
  {
    id: 'volume',
    estatuto: 'realizado',
    evidencia: 'gk/banco/estado.json — mesmo snapshot GKBANCO; D neutro da Fase B (par Exec WASM/volume); hashCanonico oraculo; Exec nativo no contrato VOLUME /gk/banco; nao e M_Docker (fis:thm:troca-realizacao: D antes de Exec)',
    fonte: 'banco/canal_patria.c',
    slots: { D: 'volume' },
    nota: 'substrato de disco da Fase B; Exec=docker permanece nao localizada sem contentor medido',
    proibicao: 'volume != contentor; sem compose; sem cold-start inventado; volume != promocao de M_Docker; S_DEPOSITO != S_ESTADO',
  },
]

export function nodoRealizacao (r) {
  const n = {
    kind: 'realizacao',
    id: r.id,
    sentido: 0,
    formato: 'json',
    estatuto: r.estatuto,
    evidencia: r.evidencia,
    proibicao: r.proibicao || 'realizacao != hospedeiro; browser != metal; docker != runtime do banco',
  }
  if (r.suporte) n.suporte = r.suporte
  if (r.fonte) n.fonte = r.fonte
  if (r.slots) n.slots = { ...r.slots }
  if (r.nota) n.nota = r.nota
  return completa(n)
}

/** Disco = filho no mesmo schema; suporte=disco. Não é segundo $id. */
export function nodoDisco (d) {
  return nodoRealizacao({
    ...d,
    suporte: d.suporte || 'disco',
    proibicao: d.proibicao || 'disco != maquina; quota LS nao muda a semantica GKBANCO',
  })
}

/** Máquina mínima realizada por U. Filhos = realizações + discos.
 *  slots = M=(Exec,D,C); nota = M_min. Sem propriedades fora do schema. */
export function maquinaParaU () {
  return completa({
    kind: 'realizacao',
    id: 'maquina',
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    suporte: 'wasm',
    evidencia: 'M=(Exec,D,C); M_min=(WASM, D_*, Memoria local); minima = WASM + memoria + disco; U reconhece/transporta; browser realiza',
    proibicao: 'U nao descreve um browser; metal era abuso; sem hierarquia browser→sistema→metal; C nao entra no iso de disco',
    slots: { ...SLOTS_M },
    nota: 'M_min=(WASM, D_*, Memoria local); C=MOVE invariante nas fases A e B (S_ESTADO 9210/9211, JSON GKBANCO)',
    filhos: [
      ...REALIZACOES.map(nodoRealizacao),
      ...DISCOS.map(nodoDisco),
    ],
  })
}

/** Mesma instrução sobre o mesmo S0 — o t fixo torna o estado comparável. */
export function aplicaInstrucao (estado, nome, entrada, saida, t) {
  const e = normalizaEstado(JSON.parse(JSON.stringify(estado)))
  gravaShell(e, nome, entrada, saida)
  if (t) e.shells[nome].t = t
  return e
}

/** JSON estável: chaves ordenadas. Sem Date. */
function jsonEstavel (o) {
  if (o === null || typeof o !== 'object') return JSON.stringify(o)
  if (Array.isArray(o)) return '[' + o.map(jsonEstavel).join(',') + ']'
  return '{' + Object.keys(o).sort().map((k) => JSON.stringify(k) + ':' + jsonEstavel(o[k])).join(',') + '}'
}

/**
 * Estado lógico GKBANCO: t e atualizado são relógio — fora.
 * pendente.t também relógio.
 */
export function estadoCanonico (estado) {
  const E = normalizaEstado(estado)
  const shells = {}
  for (const n of Object.keys(E.shells).sort()) {
    const s = E.shells[n] || {}
    shells[n] = { in: s.in || '', out: s.out || '' }
  }
  const out = {
    magia: E.magia,
    v: E.v,
    shells,
    pendente: (E.pendente || []).map((p) => ({ nome: p.nome || '', in: p.in || '' })),
  }
  if (E.pagina) {
    out.pagina = {
      html: E.pagina.html || '',
      css: E.pagina.css || '',
      js: E.pagina.js || '',
    }
  }
  return out
}

/** Assinatura canónica FNV-1a 32 — pura, sem Date, sem crypto. */
export function hashCanonico (estado) {
  const s = jsonEstavel(estadoCanonico(estado))
  let h = 2166136261
  for (let i = 0; i < s.length; i++) {
    h ^= s.charCodeAt(i)
    h = Math.imul(h, 16777619)
  }
  return ('00000000' + (h >>> 0).toString(16)).slice(-8)
}

/** Ordem de visibilidade: sequência de key(i) → getItem. Map preserva inserção. */
export function ordemVisibilidade (storage) {
  const out = []
  const n = storage && typeof storage.length === 'number' ? storage.length : 0
  for (let i = 0; i < n; i++) {
    const k = storage.key(i)
    out.push([k, k == null ? null : storage.getItem(k)])
  }
  return out
}

/**
 * Isomorfismo operacional: mesma magia, mesmo v, mesmos in/out por shell.
 * t e atualizado são relógio, não o contrato.
 */
export function isoOperacional (a, b) {
  const A = normalizaEstado(a)
  const B = normalizaEstado(b)
  if (A.magia !== B.magia || A.v !== B.v) return false
  const names = new Set([...Object.keys(A.shells), ...Object.keys(B.shells)])
  for (const n of names) {
    const sa = A.shells[n] || {}
    const sb = B.shells[n] || {}
    if ((sa.in || '') !== (sb.in || '') || (sa.out || '') !== (sb.out || '')) return false
  }
  const pa = A.pagina
  const pb = B.pagina
  if (pa || pb) {
    if (!pa || !pb) return false
    if (pa.html !== pb.html || pa.css !== pb.css || pa.js !== pb.js) return false
  }
  return true
}

/** M_wasm: persiste S no disco (LS, memoriaLS ou ponte IDB isomorfa). */
export function cicloWasm (estado, storage) {
  gravaEstado(JSON.parse(JSON.stringify(estado)), storage)
  return leEstado(storage)
}

/** Mesmo ciclo sobre o disco IDB (shim ou nativo). Medidor = isoOperacional(S1_LS, S1_IDB). */
export function cicloIDB (estado, storage) {
  return cicloWasm(estado, storage)
}

/** M_remota: o JSON que o C declara é o mesmo disco (contrato, sem UDP). */
export function cicloRemoto (estado) {
  return normalizaEstado(JSON.parse(JSON.stringify(estado)))
}

export function estadoVazioRemoto () {
  return normalizaEstado(JSON.parse(ESTADO_VAZIO_REMOTO))
}

/** Relógio do medidor: performance.now no Node/browser; Date.now só se faltar. */
function agoraMs () {
  return (typeof performance !== 'undefined' && typeof performance.now === 'function')
    ? performance.now()
    : Date.now()
}

/**
 * Custo I/O: n ciclos grava/le do mesmo S no mesmo storage.
 * Custo ≠ correcao — nao decide estatuto. Shim IDB ≠ producao.
 */
export function medeIO (estado, storage, n = 32) {
  const s = JSON.parse(JSON.stringify(estado))
  const t0 = agoraMs()
  for (let i = 0; i < n; i++) gravaEstado(JSON.parse(JSON.stringify(s)), storage)
  const grava_ms = agoraMs() - t0
  const t1 = agoraMs()
  let last
  for (let i = 0; i < n; i++) last = leEstado(storage)
  const le_ms = agoraMs() - t1
  return {
    n,
    grava_ms,
    le_ms,
    grava_ms_media: grava_ms / n,
    le_ms_media: le_ms / n,
    bytes: JSON.stringify(s).length,
    last,
  }
}

/** Ficheiro de volume = mesmo JSON que canal_patria.c / C declara. */
export const VOLUME_ESTADO = 'gk/banco/estado.json'

/** Arena nulo — mesmo contrato que celula_browser (nulo_disco=8). */
export const NULO_ARENA = 8

/**
 * D_volume: persiste o JSON GKBANCO via io.escrever/io.ler (relativo a VOLUME_ESTADO).
 * Sem Docker. Sem compose. O oraculo e hashCanonico.
 */
export function cicloVolume (estado, io) {
  const json = JSON.stringify(JSON.parse(JSON.stringify(estado)))
  io.escrever(VOLUME_ESTADO, json)
  return normalizaEstado(JSON.parse(io.ler(VOLUME_ESTADO)))
}

/**
 * Exec nativo no contrato de volume (Dockerfile VOLUME /gk/banco = estado.json).
 * Não é M_Docker. Identidade não entra.
 */
export function cicloExecNativo (estado, io) {
  return cicloVolume(estado, io)
}

/**
 * I = sql_move(-1) em consultar.wasm já no disco. Sem rede.
 * O texto compilado é o S observável da instrução (entra no GKBANCO via gravaShell).
 */
export function instrucaoSqlWasm (bytes, texto) {
  const u8 = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes)
  const inst = new WebAssembly.Instance(new WebAssembly.Module(u8), {})
  const ex = inst.exports
  if (typeof ex.sql_move !== 'function' || !ex.DISCO) {
    throw new Error('consultar.wasm sem sql_move/DISCO')
  }
  const mem = new Uint8Array(ex.DISCO.buffer)
  const enc = new TextEncoder().encode(String(texto))
  const inOff = 1024
  const outOff = 4096
  mem.set(enc, NULO_ARENA + inOff)
  const n = ex.sql_move(inOff, enc.length, outOff, -1)
  return new TextDecoder().decode(mem.subarray(NULO_ARENA + outOff, NULO_ARENA + outOff + n))
}

/**
 * Sonda o binário docker sem inventar paridade. Sem cold-start.
 * spawnSync injectável (teste / Node).
 */
export function sondaDocker (spawnSync) {
  if (typeof spawnSync !== 'function') {
    return { disponivel: false, motivo: 'sem spawnSync' }
  }
  try {
    const r = spawnSync('docker', ['version', '--format', '{{.Server.Version}}'], {
      encoding: 'utf8',
      timeout: 8000,
      windowsHide: true,
    })
    const out = String(r.stdout || '').trim()
    if (r.status === 0 && out) {
      return { disponivel: true, versao: out }
    }
    const err = r.error || r.stderr || 'docker sem daemon'
    return { disponivel: false, motivo: String(err).slice(0, 200) }
  } catch (e) {
    return { disponivel: false, motivo: String(e && e.message ? e.message : e).slice(0, 200) }
  }
}

/**
 * Contentor no contrato de volume: monta D, lê estado.json.
 * Sem --privileged, sem --network host. Sem pull (imagem ausente = nao localizada).
 * Sem contentor: { correu: false, estatuto: 'nao localizada' } — não inventa S1.
 */
export function cicloDocker (estado, opts = {}) {
  const spawnSync = opts.spawnSync
  const host = opts.volumeHost
  const sonda = opts.sonda || (spawnSync ? sondaDocker(spawnSync) : { disponivel: false })
  if (!sonda.disponivel || typeof spawnSync !== 'function' || !host) {
    return { correu: false, estatuto: 'nao localizada', estado: null }
  }
  const inspect = spawnSync('docker', ['image', 'inspect', 'alpine:3.20'], {
    encoding: 'utf8',
    timeout: 8000,
    windowsHide: true,
  })
  if (!inspect || inspect.status !== 0) {
    return {
      correu: false,
      estatuto: 'nao localizada',
      estado: null,
      motivo: 'imagem alpine:3.20 nao localizada (sem pull)',
    }
  }
  const r = spawnSync('docker', [
    'run', '--rm',
    '--network', 'none',
    '-v', host + ':/gk/banco',
    'alpine:3.20',
    'cat', '/gk/banco/estado.json',
  ], { encoding: 'utf8', timeout: 20000, windowsHide: true })
  if (!r || r.status !== 0) {
    return { correu: false, estatuto: 'nao localizada', estado: null }
  }
  try {
    return {
      correu: true,
      estatuto: 'medido',
      estado: normalizaEstado(JSON.parse(String(r.stdout || ''))),
    }
  } catch {
    return { correu: false, estatuto: 'nao localizada', estado: null }
  }
}

export { estadoVazio }
