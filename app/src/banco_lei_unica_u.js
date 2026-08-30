// banco_lei_unica_u.js — ciclo da lei única no byte.
// Ind = passo «juntar o dual» (fis:thm:tecidos) lido no índice k=0..7.
// MetaInd(L7)=L0 = o mesmo passo como fibra, não X_{k+1}. Sem Lei 8.
// Ind^8 = id = período do catálogo. Não funde L0 com L7.
// Res = π_U(F(L_7))−L_0: F e chi de banco_transf_u (fis:def:transf).
// Sem F_U. Sem GLH no byte — este número não é o rectângulo contínuo.

import { F, TRANSF_N, residuoParseval } from './banco_transf_u.js'

export const DIM = 8

export function e (k) {
  if (k < 0 || k > 7) return null
  return 1 << k
}

export function Ind (k) {
  return (k + 1) % DIM
}

/** Fecho 7→0: mesma regra, não a escada. Fora de 7 = não localizada. */
export function MetaInd (k) {
  if (k !== 7) return null
  return 0
}

/** π_U é o mesmo passo que MetaInd: fibra, não operador novo. */
export function piU (k) {
  return MetaInd(k)
}

export function paridadeAnd (a, b) {
  let x = (a & b) >>> 0
  let p = 0
  while (x) {
    p ^= x & 1
    x >>>= 1
  }
  return p
}

/** Identidade efectiva: Ind^k(L0)=L_k, MetaInd(L7)=L0, Gram, sem fusão. */
export function medeCiclo () {
  const leis = []
  let k = 0
  for (let i = 0; i < DIM; i++) {
    leis.push({ k, e: e(k) })
    if (i < 7) k = Ind(k)
  }
  let apos8 = 0
  for (let i = 0; i < DIM; i++) apos8 = Ind(apos8)
  let gramId = true
  for (let i = 0; i < DIM; i++) {
    for (let j = 0; j < DIM; j++) {
      const g = paridadeAnd(e(i), e(j))
      if (g !== (i === j ? 1 : 0)) gramId = false
    }
  }
  const e0 = e(0)
  const e7 = e(7)
  const sobe = 2 ** 8
  return {
    leis,
    ind_k: leis.every((L, i) => L.k === i && L.e === (1 << i)),
    ind8: apos8 === 0,
    meta: MetaInd(7) === 0 && MetaInd(0) === null && MetaInd(6) === null,
    nao_funde: e0 !== e7 && paridadeAnd(e0, e7) === 0,
    sobe_recusado: sobe !== e0 && sobe !== e7,
    gramId,
    vinco_neutro: paridadeAnd(e0, e7) === 0,
    residuo: 0,
    piU: piU(7) === 0 && piU(0) === null,
    metaind_iff_piU: (MetaInd(7) === 0) === (piU(7) === 0),
  }
}

export function residuoCiclo () {
  const m = medeCiclo()
  const ok = m.ind_k && m.ind8 && m.meta && m.nao_funde &&
    m.sobe_recusado && m.gramId && m.vinco_neutro
  m.residuo = ok ? 0 : 1
  return m
}

/** L_k = o objecto que os testes já usam: índice + e_k=2^k. */
export function lei (k) {
  return Object.freeze({ k, e: e(k) })
}

export const L0 = lei(0)
export const L7 = lei(7)

/** Campo em X: delta no byte e_k. Não é GLH. */
export function campoLei (k) {
  const ek = e(k)
  if (ek == null) return null
  const f = new Array(TRANSF_N).fill(0)
  f[ek] = 1
  return f
}

/**
 * Lê o índice no espectro pela Gram já medida: entre e_0..e_7,
 * F(δ_{e_j})_{e_i} = χ_{e_i}(e_j) = −1 iff i=j.
 * π_U(e_7)=null — o byte não é o índice. Sem mapa novo.
 */
export function leIndiceDoEspectro (hat) {
  if (!Array.isArray(hat) || hat.length !== TRANSF_N) return null
  let hit = null
  for (let i = 0; i < DIM; i++) {
    if (hat[e(i)] === -1) {
      if (hit !== null) return null
      hit = i
    }
  }
  return hit
}

/**
 * Res = π_U(F(L_7)) − L_0, no índice.
 * L_7, L_0 = {k, e} do ciclo. F = fis:def:transf no campo δ_{e_7}.
 * π_U = MetaInd no índice recuperado. GLH não entra.
 */
export function residuoComposto () {
  const f = campoLei(7)
  const hat = F(f)
  const parseval = residuoParseval(f, hat)
  const recuperado = leIndiceDoEspectro(hat)
  const pi = recuperado == null ? null : piU(recuperado)
  const res = (pi === null || L0.k == null) ? 1 : (pi - L0.k)
  return Object.freeze({
    L7,
    L0,
    parseval,
    recuperado,
    pi,
    res,
    pi_no_byte: piU(L7.e),
    composto: res === 0 ? 'realizado' : 'descartado',
    glh: 'nao localizada',
  })
}

/**
 * Resíduo(π_U(F(L_7))−L_0). Mede o composto. Não mede GLH.
 * F é fis:def:transf — não se inventa F_U.
 */
export function residuoGlhPi (parsevalResiduo) {
  const c = residuoCiclo()
  const m = residuoComposto()
  const piOk = !!(c.residuo === 0 && c.piU && c.metaind_iff_piU)
  const parseval = parsevalResiduo !== undefined ? parsevalResiduo : m.parseval
  return Object.freeze({
    nome: 'Residuo(pi_U(F(L_7))-L_0)',
    fonte_GLH: 'fis:thm:central',
    fonte_F: 'fis:def:transf',
    fonte_pi: 'univ:thm:metaind-pi',
    fonte_retorno: 'univ:thm:retorno-canonico',
    pi_realizado: piOk,
    F_parseval: parseval,
    parseval_L7: m.parseval,
    res: m.res,
    recuperado: m.recuperado,
    pi: m.pi,
    glh: 'nao localizada',
    composto: m.composto,
    residuo_ciclo: c.residuo,
    promove_tripla: false,
  })
}
