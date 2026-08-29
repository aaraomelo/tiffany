// banco_absorve.js — absorção fio a fio (como latex §W4 / bash §W8 / node §W10).
// MOVE(−1) emite na arena; canal sincroniza slots; MOVE(+1) absorve. Sem string SQL adaptadora.

import { moveWasm, u8, NULO } from './banco_move.js'
import { manifestoAtual } from './manifesto_loader.js'
import { shellRemoto } from './banco_sql_interno.js'
import { discoBrowser, leEstado, gravaEstado, gravaShell } from './banco_disco.js'
import { celulaDeWasm, secErgNome } from './wasm_sec_browser.js'
import { correBackendMetal, fetchCorreErg } from './corre_metal_browser.js'

const SHELLS = ['bash', 'node', 'powershell']
const OFF_NOUT = 24578
const OFF_BUF_OUT = 16384

let cache = new Map()

async function wasmBackend (nome, wasmBase = '/wasm/') {
  const k = nome + '@' + wasmBase
  if (cache.has(k)) return cache.get(k)
  const L = manifestoAtual().linguagens.find((l) => l.nome === nome)
  if (!L) throw new Error('backend «' + nome + '» ausente')
  const r = await fetch(wasmBase + L.wasm)
  if (!r.ok) throw new Error('wasm ' + L.wasm + ': ' + r.status)
  const wasm = new Uint8Array(await r.arrayBuffer())
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
      temAsm: celula.temErg && celula.erg.includes(nome + '_move'),
      temFita: celula.temFita,
    },
  }
  cache.set(k, pack)
  return pack
}

function moveExport (L) {
  return L.exports?.find((e) => e.endsWith('_move')) || L.nome + '_move'
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
 * Absorve script num backend shell.
 * ctx.motor: 'auto' (fita se embutida) | 'wasm' (node_move) | 'fita' (isa+erg.fita)
 * ctx.remoto: com canal, envia script à Patria (pleno) em vez de fita/wasm local.
 */
export async function absorveBackend (nome, script, ctx = {}) {
  if (!SHELLS.includes(nome)) throw new Error('absorve: shell «' + nome + '»')
  const pack = await wasmBackend(nome, ctx.wasmBase)
  const { ex, L, celula } = pack
  const fn = moveExport(L)
  const body = nome === 'bash' && script && !script.endsWith('\n') ? script + '\n' : script
  const motor = ctx.motor ?? 'auto'
  const querFita = motor === 'fita' || (motor === 'auto' && celula?.temFita && !ctx.remoto)

  if (querFita && celula?.temFita) {
    const correErg = ctx.correErg ?? await fetchCorreErg(nome, ctx.ergBase)
    const metal = await correBackendMetal(nome, body, {
      wasmBase: ctx.wasmBase,
      wasmCelula: pack,
      correErg,
      maxSteps: ctx.maxSteps,
    })
    const st = discoBrowser(ctx)
    const estado = leEstado(st)
    gravaShell(estado, nome, body, metal.out)
    gravaEstado(estado, st)
    return {
      out: metal.out,
      meta: {
        via: metal.meta.via + '+GKBANCO',
        backend: nome,
        bytesIn: body.length,
        bytesOut: metal.out.length,
        move: fn,
        motor: 'fita',
        passos: metal.meta.passos,
        celula,
        secErg: celula?.secErg,
        fitaLen: celula?.fitaLen ?? 0,
      },
    }
  }

  const nw = moveWasm(ex, fn, body, -1)
  let via = 'arena'

  if (ctx.canal && ctx.remoto) {
    const raw = await shellRemoto(body, ctx.canal, nome)
    injetaStdoutArena(ex, raw)
    via = 'canal'
  }

  const out = moveWasm(ex, fn, '', +1, 8192, 8192)

  const st = discoBrowser(ctx)
  const estado = leEstado(st)
  gravaShell(estado, nome, body, out)
  gravaEstado(estado, st)

  return {
    out,
    meta: {
      via: via + '+GKBANCO',
      backend: nome,
      bytesIn: nw,
      bytesOut: out.length,
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
