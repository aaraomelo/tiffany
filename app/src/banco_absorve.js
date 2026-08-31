// banco_absorve.js — absorção órbita a órbita (latex §W4 / sql arena / node §W10).
// MOVE(−1) emite na arena; canal sincroniza slots; MOVE(+1) absorve. Sem string SQL adaptadora.
// Qualquer língua do manifesto entra pela absorcao.move; shells (canal) na realização remota se remoto.

import { moveByForma, moveNome, u8, NULO } from './banco_move.js'
import { manifestoAtual } from './manifesto_loader.js'
import { shellRemoto } from './banco_sql_interno.js'
import { discoBrowser, leEstado, gravaEstado, gravaShell, gravaWasmLS, leWasmLS } from './banco_disco.js'
import { sincroniza } from './banco_sync.js'
import { celulaDeWasm, secErgNome } from './wasm_sec_browser.js'
import { correBackendMetal, fetchCorreErg } from './corre_metal_browser.js'

const OFF_NOUT = 24578
const OFF_BUF_OUT = 16384

let cache = new Map()

function linguaDoManifesto (nome) {
  const L = manifestoAtual().linguagens.find((l) => l.nome === nome)
  if (!L) throw new Error('backend «' + nome + '» ausente')
  return L
}

function eCanal (L) {
  return L.absorcao?.orbita === 'canal'
}

async function wasmBackend (nome, wasmBase = '/wasm/', storage) {
  const k = nome + '@' + wasmBase
  if (cache.has(k)) return cache.get(k)
  const L = linguaDoManifesto(nome)
  let wasm
  try {
    const r = await fetch(wasmBase + L.wasm)
    if (!r.ok) throw new Error('wasm ' + L.wasm + ': ' + r.status)
    wasm = new Uint8Array(await r.arrayBuffer())
    if (storage) await gravaWasmLS(storage, L.wasm, wasm)
  } catch (e) {
    wasm = storage ? await leWasmLS(storage, L.wasm) : null
    if (!wasm) throw e
  }
  const celula = celulaDeWasm(wasm, nome)
  const { instance } = await WebAssembly.instantiate(wasm)
  const pack = {
    ex: instance.exports,
    L,
    celula: {
      secErg: secErgNome(nome),
      erg: celula.erg,
      fita: celula.fita,
      fitaLen: celula.fitaLen,
      temAsm: celula.temErg && celula.erg.includes(moveNome(L)),
      temFita: celula.temFita,
    },
  }
  cache.set(k, pack)
  return pack
}

async function persisteGkbanco (nome, body, out, ctx) {
  const st = discoBrowser(ctx)
  const estado = leEstado(st)
  gravaShell(estado, nome, body, out)
  gravaEstado(estado, st, { limite: ctx.limite })
  if (ctx.canal && ctx.remoto) {
    await sincroniza(ctx.canal, st, { limite: ctx.limite })
  }
}
/** Injeta stdout no protocolo da arena (mesmo layout que interpretar.c). */
export function injetaStdoutArena (ex, texto) {
  const mem = u8(ex)
  const b = new TextEncoder().encode(texto)
  const cap = 8192
  const n = Math.min(b.length, cap)
  mem.set(b.subarray(0, n), NULO + OFF_BUF_OUT)
  mem[NULO + OFF_NOUT] = n & 255
  mem[NULO + OFF_NOUT + 1] = (n >> 8) & 255
}

/**
 * Absorve texto numa língua do manifesto (absorcao.move na arena).
 * ctx.motor: 'auto' (fita se embutida) | 'wasm' | 'fita' (isa+erg.fita)
 * ctx.remoto: com canal, envia script à realização remota (pleno) — só órbita canal.
 */
export async function absorveBackend (nome, script, ctx = {}) {
  const pack = await wasmBackend(nome, ctx.wasmBase, discoBrowser(ctx))
  const { ex, L, celula } = pack
  const fn = moveNome(L)
  const body = nome === 'bash' && script && !script.endsWith('\n') ? script + '\n' : (script ?? '')
  const motor = ctx.motor ?? 'auto'
  const querFita = eCanal(L) && (motor === 'fita' || (motor === 'auto' && celula?.temFita && !ctx.remoto))

  if (querFita && celula?.temFita) {
    const correErg = ctx.correErg ?? await fetchCorreErg(nome, ctx.ergBase)
    const fita = await correBackendMetal(nome, body, {
      wasmBase: ctx.wasmBase,
      wasmCelula: pack,
      correErg,
      maxSteps: ctx.maxSteps,
    })
    await persisteGkbanco(nome, body, fita.out, ctx)
    return {
      out: fita.out,
      meta: {
        via: fita.meta.via + '+GKBANCO',
        backend: nome,
        bytesIn: body.length,
        bytesOut: fita.out.length,
        move: fn,
        motor: 'fita',
        passos: fita.meta.passos,
        celula,
        secErg: celula?.secErg,
        fitaLen: celula?.fitaLen ?? 0,
      },
    }
  }

  const nw = moveByForma(ex, L, body, -1)
  let via = 'arena'

  if (eCanal(L) && ctx.canal && ctx.remoto) {
    try {
      const raw = await shellRemoto(body, ctx.canal, nome)
      injetaStdoutArena(ex, raw)
      via = 'canal'
    } catch {
      via = 'arena'
    }
  }

  const out = moveByForma(ex, L, '', +1, 8192, 8192)

  await persisteGkbanco(nome, body, out, ctx)

  return {
    out,
    meta: {
      via: via + '+GKBANCO',
      backend: nome,
      bytesIn: typeof nw === 'string' ? nw.length : nw,
      bytesOut: String(out).length,
      move: fn,
      motor: 'wasm',
      celula: celula || null,
      secErg: celula?.secErg,
      fitaLen: celula?.fitaLen ?? 0,
    },
  }
}

export function limpaCacheWasm () {
  cache = new Map()
}
