// banco_cristalchain_u.js — Cadeia de Cristais (CristalChain) no motor.
// Livro-razão cuja cifra conserva det=+1 (ouro branco). Não é blockchain.
// sha256 selecciona identidade (banco_identidade_u); NÃO é a cifra.
// Contratos: liquidam-se (smartcontract.c); o dono é o id da identidade.
// I0: det=+1 ≠ |det|=1 ≠ P=k/N ≠ Born. Passo B^k ≠ régua do ouro {1,-1}.

import { completa } from './banco_schema.js'
import { idEstavelDaChave } from './banco_identidade_u.js'

export const CHAVE_CADEIA = 'gk:banco:cristalchain:'

/** Companheira do ouro branco. cripto:thm:det */
export const B_OURO_BRANCO = Object.freeze([[4, -1], [1, 0]])

export const AGENTES = Object.freeze(['gira', 'estica', 'limite'])

/**
 * Os cinco contratos de tests/smartcontract.c §S1.
 * Liquidam-se sozinhos; ninguém assina. Todos cabem no livro do dono.
 */
export const CONTRATOS = Object.freeze([
  Object.freeze({ nome: 'ouro', termos: Object.freeze([0, 1, 1, 2, 3, 5]) }),
  Object.freeze({ nome: 'prata', termos: Object.freeze([0, 1, 2, 5, 12, 29]) }),
  Object.freeze({ nome: 'i', termos: Object.freeze([1, 0, -1, 0, 1, 0]) }),
  Object.freeze({ nome: 'omega', termos: Object.freeze([1, 0, -1, 1, 0, -1]) }),
  Object.freeze({ nome: 'PA', termos: Object.freeze([100, 110, 120, 130, 140, 150]) }),
])

export function det2 (M) {
  return M[0][0] * M[1][1] - M[0][1] * M[1][0]
}

/** det(A_n)=−1 na família metálica; det(B)=+1 no ouro branco. */
export function detFamilia (n) {
  return det2([[n, 1], [1, 0]])
}

/** Conservar = det(B^k)=+1. Não é saldo económico. */
export function detPotenciaOuroBranco (k) {
  const d = det2(B_OURO_BRANCO)
  let acc = 1
  const n = k | 0
  for (let i = 0; i < n; i++) acc *= d
  return acc
}

export function reguaDe (x) {
  const n = x && x.length
  const r = { B: 0, C: 0, fechou: 0 }
  if (!n || n < 4) return r
  const det = x[1] * x[1] - x[0] * x[2]
  if (det === 0) return r
  const pn = x[2] * x[1] - x[0] * x[3]
  const qn = x[1] * x[3] - x[2] * x[2]
  if (pn % det || qn % det) return r
  const p = Math.trunc(pn / det)
  const q = Math.trunc(qn / det)
  r.B = p
  r.C = -q
  r.fechou = 1
  for (let k = 0; k + 2 < n; k++) {
    if (x[k + 2] !== p * x[k + 1] + q * x[k]) {
      r.fechou = 0
      break
    }
  }
  return r
}

export function norma (r, a, b) {
  return a * a + r.B * a * b + r.C * b * b
}

export function prod (r, a, b, c, d) {
  return {
    a: a * c - r.C * b * d,
    b: a * d + b * c + r.B * b * d,
  }
}

/** Aplicação da companheira B do ouro branco: (a,b) ↦ B(a,b). */
export function passoCifra (a, b) {
  return {
    a: B_OURO_BRANCO[0][0] * a + B_OURO_BRANCO[0][1] * b,
    b: B_OURO_BRANCO[1][0] * a + B_OURO_BRANCO[1][1] * b,
  }
}

export function qualAgente (r) {
  const D = r.B * r.B - 4 * r.C
  return D < 0 ? 0 : (D > 0 ? 1 : 2)
}

export function agenteCorre (qual, r, q) {
  let a = 0
  let b = 1
  let passos = 0
  const teto = q * q + 1
  if (qual === 0) {
    const a0 = a
    const b0 = b
    do {
      const p = prod(r, a, b, 0, 1)
      a = ((p.a % q) + q) % q
      b = ((p.b % q) + q) % q
      passos++
    } while (!(a === a0 && b === b0) && passos < teto)
  } else if (qual === 1) {
    a = 1
    b = 0
    for (let k = 0; k < 12; k++) {
      const p = prod(r, a, b, 0, 1)
      if (Math.abs(p.a) > 1000000000) break
      a = p.a
      b = p.b
      passos++
    }
  } else {
    for (let k = 0; k < 12; k++) {
      a += 1
      passos++
    }
  }
  return passos
}

/** Fibonacci mod q, sem tocar na órbita. smartcontract.c §S6. */
export function pisano (q) {
  let f0 = 0
  let f1 = 1
  let pis = 0
  const teto = 6 * q + 1
  do {
    const f2 = (f0 + f1) % q
    f0 = f1
    f1 = f2
    pis++
  } while (!(f0 === 0 && f1 === 1) && pis <= teto)
  return pis
}

/** Órbita do ouro {B=1,C=-1} até fechar. ≠ passo B do ouro branco. */
export function orbitaOuro (q) {
  const r = { B: 1, C: -1, fechou: 1 }
  let a = 0
  let b = 1
  let passos = 0
  const teto = q * q + 1
  do {
    const p = prod(r, a, b, 0, 1)
    a = ((p.a % q) + q) % q
    b = ((p.b % q) + q) % q
    passos++
  } while (!(a === 0 && b === 1) && passos <= teto)
  return { passos, fechou: a === 0 && b === 1, passouTeto: passos > q * q }
}

export function liquida (termos, q) {
  const L = {
    liquidado: 0,
    r: { B: 0, C: 0, fechou: 0 },
    agente: -1,
    trabalho: 0,
    motivo: '',
  }
  const x = Array.isArray(termos) ? termos : []
  if (x.length < 4) {
    L.motivo = 'termos a menos: o minimo e n+2'
    return L
  }
  L.r = reguaDe(x)
  if (!L.r.fechou) {
    L.motivo = 'os termos nao sao de um corpo de grau 2'
    return L
  }
  for (let a = -5; a <= 5; a++) {
    for (let b = -5; b <= 5; b++) {
      const va = a + L.r.B * b
      const vb = -b
      const va2 = va + L.r.B * vb
      const vb2 = -vb
      if (va2 !== a || vb2 !== b) {
        L.motivo = 'nu o nu != id: a reversao nao fecha'
        return L
      }
      const p = prod(L.r, a, b, va, vb)
      if (p.a !== norma(L.r, a, b) || p.b !== 0) {
        L.motivo = 'N != x·nu(x)'
        return L
      }
    }
  }
  L.agente = qualAgente(L.r)
  L.trabalho = agenteCorre(L.agente, L.r, q || 12)
  L.liquidado = L.trabalho > 0 ? 1 : 0
  if (!L.liquidado) L.motivo = 'o agente nao conseguiu trabalhar'
  return L
}

export function chaveLivro (dono) {
  return CHAVE_CADEIA + String(dono || '')
}

function livroVazio (dono, camada) {
  return {
    dono: String(dono || ''),
    camada: camada || null,
    registos: [],
    vistos: {},
  }
}

export function leLivro (storage, dono) {
  if (!storage || !dono) return livroVazio(dono, null)
  try {
    const s = storage.getItem(chaveLivro(dono))
    if (!s) return livroVazio(dono, null)
    const o = JSON.parse(s)
    return {
      dono: String(o.dono || dono),
      camada: o.camada || null,
      registos: Array.isArray(o.registos) ? o.registos.slice() : [],
      vistos: o.vistos && typeof o.vistos === 'object' ? { ...o.vistos } : {},
    }
  } catch {
    return livroVazio(dono, null)
  }
}

export function gravaLivro (storage, livro) {
  if (!storage || !livro || !livro.dono) return
  storage.setItem(chaveLivro(livro.dono), JSON.stringify({
    dono: livro.dono,
    camada: livro.camada || null,
    registos: livro.registos || [],
    vistos: livro.vistos || {},
  }))
}

/** Dono = id estável da identidade (chave → hex(banda); senão UUID da sessão). */
export async function donoDaCadeia (identidade) {
  const r = identidade || {}
  if (r.id) return { dono: String(r.id), camada: r.camada || null }
  if (r.camada === 'chave' && r.chave) {
    return { dono: await idEstavelDaChave(r.chave), camada: 'chave' }
  }
  if (r.camada === 'sessao' && (r.sessao || r.uuid)) {
    return { dono: String(r.sessao || r.uuid), camada: 'sessao' }
  }
  return { dono: null, camada: null }
}

function chaveContrato (nome, termos) {
  if (nome) return 'nome:' + nome
  return 'termos:' + JSON.stringify(Array.from(termos || []))
}

/**
 * Liquida um contrato no livro do dono.
 * Primeira vez acrescenta R_i; as seguintes são idempotentes (S4).
 * Sem identidade não há livro (não é cadeia anónima / blockchain).
 */
export function liquidaNaCadeia (termos, opts = {}) {
  const dono = opts.dono
  const storage = opts.storage
  const q = opts.q || 12
  const nome = opts.nome || ''
  if (!dono) {
    return {
      liquidado: 0,
      acrescentou: 0,
      motivo: 'sem identidade: o livro pede dono',
      L: null,
    }
  }
  const livro = leLivro(storage, dono)
  livro.camada = opts.camada || livro.camada
  const ck = chaveContrato(nome, termos)
  if (livro.vistos[ck]) {
    const prev = livro.registos.find((r) => r.ck === ck) || null
    return {
      liquidado: 1,
      acrescentou: 0,
      motivo: 'ja liquidado: so a primeira conta',
      L: prev,
      livro,
    }
  }
  const L = liquida(termos, q)
  if (!L.liquidado) {
    return { liquidado: 0, acrescentou: 0, motivo: L.motivo, L, livro }
  }
  const rec = {
    i: livro.registos.length,
    ck,
    nome,
    termos: Array.from(termos),
    B: L.r.B,
    C: L.r.C,
    agente: AGENTES[L.agente] || '',
    trabalho: L.trabalho,
    liquidado: 1,
    det_cifra: detPotenciaOuroBranco(1),
  }
  livro.registos.push(rec)
  livro.vistos[ck] = 1
  livro.dono = dono
  gravaLivro(storage, livro)
  return { liquidado: 1, acrescentou: 1, motivo: '', L: rec, livro }
}

/** Coloca os cinco contratos do medidor no livro do dono. Idempotente. */
export function liquidaTodos (opts = {}) {
  const out = []
  for (const c of CONTRATOS) {
    out.push(liquidaNaCadeia(c.termos, { ...opts, nome: c.nome }))
  }
  return out
}

/**
 * Integra identidade + livro: dono = id; todos os contratos no livro.
 * Chamado no boot do front/terminal depois de ligaIdentidade.
 */
export async function integraCadeia (identidade, storage, opts = {}) {
  const { dono, camada } = await donoDaCadeia(identidade)
  if (!dono) {
    return { dono: null, camada: null, livro: livroVazio('', null), resultados: [] }
  }
  const resultados = liquidaTodos({ dono, camada, storage, q: opts.q || 12 })
  const livro = leLivro(storage, dono)
  livro.camada = camada
  gravaLivro(storage, livro)
  return { dono, camada, livro, resultados }
}

/** kind=orbita (existente). Não é kind novo. Não é U. */
export function cadeiaParaU (pacote) {
  const p = pacote || {}
  const livro = p.livro || livroVazio(p.dono, p.camada)
  return completa({
    kind: 'orbita',
    id: 'cristalchain',
    sentido: 0,
    formato: 'json',
    estatuto: 'realizado',
    evidencia: 'cripto:def:cadeia; ouro_branco.c det=+1; smartcontract.c liquida; dono=id da identidade',
    proibicao: 'CristalChain != blockchain; sha256 != cifra; det=+1 != |det|=1 != Born; passo B != Pisano ouro; identidade != Exec',
    dono: livro.dono || '',
    camada: livro.camada || '',
    slots: {
      n: String(livro.registos.length),
      det: String(detPotenciaOuroBranco(1)),
    },
  })
}
