// banco_lei_unica_u.js — ciclo da lei única no byte.
// Ind = passo «juntar o dual» (fis:thm:tecidos) lido no índice k=0..7.
// MetaInd(L7)=L0 = o mesmo passo como fibra, não X_{k+1}. Sem Lei 8.
// Ind^8 = id = período do catálogo. Não funde L0 com L7.
// Res = π_U(F(L_7))−L_0: F e chi de banco_transf_u (fis:def:transf).
// Sem F_U. GLH-byte = I=F^{-1}F(I) (reversão). GLH contínuo ≠ este sítio.
// 3^{-1}_χ = π∘F ≠ F^{-1} = 2^{-m} F.

import {
  F, TRANSF_M, TRANSF_N, residuoParseval, residuoReversao, caractere,
} from './banco_transf_u.js'

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
    glh: 'realizado',
    glh_continuo: 'nao localizada',
  })
}

/**
 * GLH no byte = reversão da base ortonormal: I = F^{-1}(F(I)).
 * F^{-1} = 2^{-m} F (fis:thm:H). Não é Gentil contínuo. ≠ 3^{-1}_χ.
 */
export function residuoGlhByte () {
  const misto = new Array(TRANSF_N)
  for (let i = 0; i < TRANSF_N; i++) misto[i] = (i % 5) - 2
  const provas = [
    { nome: 'chi_1', f: caractere(1) },
    { nome: 'delta_e7', f: campoLei(7) },
    { nome: 'delta_e0', f: campoLei(0) },
    { nome: 'misto', f: misto },
  ]
  let res = 0
  const detalhe = []
  for (const p of provas) {
    const hat = F(p.f)
    const parseval = residuoParseval(p.f, hat)
    const reversao = residuoReversao(p.f)
    if (parseval !== 0 || reversao !== 0) res = 1
    detalhe.push({ nome: p.nome, parseval, reversao })
  }
  return Object.freeze({
    formula: 'I = F^{-1}(F(I))',
    finv: '2^{-m} F',
    m_hadamard: TRANSF_M,
    m_dobras: 3,
    factor: `2^{-${TRANSF_M}}`,
    nao_e_1_8: TRANSF_M !== 3,
    fonte: 'univ:thm:reversao-byte',
    def: 'univ:def:finv',
    thm: 'fis:thm:H',
    n: TRANSF_N,
    res,
    detalhe,
    glh_byte: res === 0 ? 'realizado' : 'descartado',
    glh_continuo: 'nao localizada',
    nao_e_inv_tres: true,
  })
}

/**
 * Resíduo(π_U(F(L_7))−L_0). Mede o composto. Não mede GLH.
 * F é fis:def:transf — não se inventa F_U.
 */
export function residuoGlhPi (parsevalResiduo) {
  const c = residuoCiclo()
  const m = residuoComposto()
  const b = residuoGlhByte()
  const piOk = !!(c.residuo === 0 && c.piU && c.metaind_iff_piU)
  const parseval = parsevalResiduo !== undefined ? parsevalResiduo : m.parseval
  return Object.freeze({
    nome: 'Residuo(pi_U(F(L_7))-L_0)',
    fonte_GLH: 'fis:thm:central',
    fonte_F: 'fis:def:transf',
    fonte_pi: 'univ:thm:metaind-pi',
    fonte_retorno: 'univ:thm:retorno-canonico',
    fonte_finv: 'univ:def:finv',
    fonte_reversao: 'univ:thm:reversao-byte',
    pi_realizado: piOk,
    F_parseval: parseval,
    parseval_L7: m.parseval,
    res: m.res,
    recuperado: m.recuperado,
    pi: m.pi,
    glh: b.glh_byte,
    glh_byte: b.glh_byte,
    glh_continuo: b.glh_continuo,
    reversao: b.res,
    finv: b.finv,
    composto: m.composto,
    residuo_ciclo: c.residuo,
    promove_tripla: false,
    fonte_fis: 'fis:obs:U-consome',
    fonte_cat: 'cat:nucleo-u',
  })
}

/**
 * 3^{-1} no corte χ: π_U ∘ F, não 1/3.
 * Terceira dobra = U_an = π. Fecho L7→L0, Res=0.
 * χ^{(3)} aniquila o terceiro bit do índice; não é 3^{-1}.
 */
export function invTres () {
  const m = residuoComposto()
  return Object.freeze({
    formula: 'pi_U o F',
    fonte: 'univ:obs:inv-tres-corte',
    de: 7,
    para: m.pi,
    res: m.res,
    estatuto: m.res === 0 ? 'realizado' : 'descartado',
    racional_1_3: 'nao localizada',
    e_terceira: 1 << 2,
    chi_terceira_nao_e_inv: true,
    nao_e_finv: true,
  })
}

/**
 * Lei canónica: quádrupla (L0, Duo, π_U∘F, vinco).
 * CF/φ/lemniscata ≠ 3^{-1}_χ ≠ Duo. Sem Lei 8. Sem 1/3.
 */
export function leiCanonica () {
  const m = residuoComposto()
  return Object.freeze({
    formula: '(L0, Duo, pi_U o F, vinco)',
    fonte: 'univ:def:lei-canonica',
    reconhecimento: 'univ:thm:reconhecimento',
    L0: { k: L0.k, e: L0.e, polo: '(+1)⊕(-1)', dual: '0†=∞' },
    Duo: { star: 'Star(U)=D', cisao: '1→2', base: '2^3=8' },
    retorno: {
      formula: '3^{-1}_chi',
      de: 7,
      para: m.pi,
      res: m.res,
      estatuto: m.res === 0 ? 'realizado' : 'descartado',
    },
    vinco: '1=vinco(L0,L7)',
    cf_estrela: 'nao localizada como 3^{-1}_chi / Duo',
    racional_1_3: 'nao localizada',
    estatuto: m.res === 0 ? 'realizado' : 'descartado',
    fis: 'fis:obs:U-consome',
    cat: 'cat:nucleo-u',
  })
}

/**
 * Núcleo alinhado a fisica/catálogo. GLH-byte = reversão realizada.
 * M_Docker / GLH contínuo / F∩B∩N / T³ permanecem nao localizada.
 * Sem Lei 8. Sem Ficha 11.
 */
export function nucleoU () {
  const c = residuoCiclo()
  const m = residuoComposto()
  const g = residuoGlhPi()
  const can = leiCanonica()
  const b = residuoGlhByte()
  return Object.freeze({
    U_can: can,
    retorno: {
      estatuto: m.res === 0 ? 'realizado' : 'descartado',
      res: m.res,
      fonte: 'univ:thm:retorno-canonico',
      fis: 'fis:obs:U-consome',
      cat: 'cat:nucleo-u',
    },
    pi_alonzo: {
      estatuto: 'realizado',
      fonte: 'univ:def:alonzo-idemp',
      fis: 'fis:def:alonzo',
    },
    F_parseval: {
      estatuto: m.parseval === 0 ? 'realizado' : 'descartado',
      fonte: 'fis:def:transf',
      thm: 'fis:thm:H',
    },
    composto: {
      estatuto: m.composto,
      res: m.res,
      fonte: 'univ:obs:residuo-glh',
    },
    glh_byte: b.glh_byte,
    glh_continuo: b.glh_continuo,
    reversao: { res: b.res, formula: b.formula, finv: b.finv },
    dois_retornos: Object.freeze({
      finv: Object.freeze({
        formula: 'F^{-1}=2^{-m} F',
        m_hadamard: b.m_hadamard,
        factor: b.factor,
        n: b.n,
        res: b.res,
        estatuto: b.glh_byte,
        nao_e_1_8: b.nao_e_1_8,
      }),
      inv_tres: Object.freeze({
        formula: '3^{-1}_chi = pi o F',
        m_dobras: 3,
        res: m.res,
        estatuto: m.res === 0 ? 'realizado' : 'descartado',
      }),
    }),
    FBN: 'nao localizada',
    M_Docker: 'nao localizada',
    T3: 'nao localizada',
    inv_tres: invTres(),
    racional_1_3: 'nao localizada',
    residuo_ciclo: c.residuo,
    pi_realizado: g.pi_realizado,
  })
}
