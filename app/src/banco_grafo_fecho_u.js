// banco_grafo_fecho_u.js — grafo de composição das 8 leis + auxiliares.
// Arestas só as que o código já mede. Sem Lei 8. Sem par 1–7.
// Sem fundir π²=π, x²=1, x²=x+1, x²=-1, Star(U)=D.
// F^{-1}F=I e π_U F(L_7)=L_0 já existem; aqui mede-se a volta conjunta.

import {
  F, Finv, residuoReversao,
} from './banco_transf_u.js'
import {
  Ind, MetaInd, piU, campoLei, leIndiceDoEspectro, residuoComposto,
  L0, L7,
} from './banco_lei_unica_u.js'

export const NOS_LEI = Object.freeze(['L0', 'L1', 'L2', 'L3', 'L4', 'L5', 'L6', 'L7'])
export const NOS_AUX = Object.freeze(['F', 'piU', 'Duo', 'i'])

function aresta (de, para, op, label, codigo) {
  return Object.freeze({ de, para, op, label, codigo })
}

/** Arestas medidas: operador existente L_i→L_j ou auxiliar. Sem dobra L0→L1. */
export function arestasMedidas () {
  const ind = []
  for (let k = 0; k < 7; k++) {
    ind.push(aresta(`L${k}`, `L${k + 1}`, 'Ind', 'univ:def:lei-unica', 'Ind'))
  }
  ind.push(aresta('L7', 'L0', 'Ind', 'univ:obs:tecidos-periodo', 'Ind'))
  return Object.freeze([
    ...ind,
    aresta('L7', 'L0', 'MetaInd', 'univ:thm:metaind-pi', 'MetaInd'),
    aresta('L7', 'L0', 'piU', 'univ:thm:metaind-pi', 'piU'),
    aresta('L7', 'F', 'F', 'fis:def:transf', 'F'),
    aresta('F', 'L7', 'Finv', 'univ:def:finv', 'Finv'),
    aresta('F', 'L0', 'piU o F', 'univ:thm:retorno-canonico', 'piU(leIndiceDoEspectro(F))'),
  ])
}

/**
 * Narrativa pedida sem operador L_i→L_j no motor.
 * lei2_bidual / lei1_dual / lei5_rotor actuam no byte, não no índice das leis.
 */
export function arestasAusentes () {
  return Object.freeze([
    Object.freeze({
      de: 'L0', para: 'L1', op: 'dobra',
      label: 'fis:def:op', porque: 'paper nao tem seta L0→L1 com ∂',
    }),
    Object.freeze({
      de: 'L2', para: 'L2', op: 'K**=K',
      label: 'lei2_bidual', porque: 'byte id; palco espectral nao localizada; nao e L_i→L_j',
    }),
    Object.freeze({
      de: 'L6', para: 'Duo', op: 'hexal',
      label: 'lei6_coincide', porque: 'Lei 6 ⊕=⊗ ≠ Duo (N/A)',
    }),
    Object.freeze({
      de: 'L1', para: 'L7', op: 'dual 1†=7',
      label: null, porque: 'so 0†=∞; par 1–7 nao se inventa',
    }),
    Object.freeze({
      de: 'L5', para: 'Duo', op: 'pental=Duo',
      label: null, porque: 'x^2=-1 ≠ Star(U)=D',
    }),
    Object.freeze({
      de: 'L5', para: 'L5', op: 'Born |psi|^2',
      label: null, porque: 'P=k/N realizado; P_i=|psi_i|^2 nao localizada',
    }),
  ])
}

/** Ciclo A: F^{-1} F = I no campo δ_{e_7}. Sem transformação nova. */
export function residuoCicloA (f) {
  const campo = f || campoLei(7)
  const res = residuoReversao(campo)
  return Object.freeze({
    nome: 'A',
    formula: 'F^{-1} F = I',
    fonte: 'univ:def:finv',
    thm: 'univ:thm:reversao-byte',
    codigo: 'Finv(F(f))',
    res,
    estatuto: res === 0 ? 'realizado' : 'descartado',
  })
}

/** Ciclo B: π_U F(L_7) = L_0. Já medido em residuoComposto. */
export function residuoCicloB () {
  const m = residuoComposto()
  return Object.freeze({
    nome: 'B',
    formula: 'pi_U F(L_7) = L_0',
    fonte: 'univ:thm:retorno-canonico',
    codigo: 'piU(leIndiceDoEspectro(F(campoLei(7))))',
    res: m.res,
    recuperado: m.recuperado,
    pi: m.pi,
    estatuto: m.res === 0 ? 'realizado' : 'descartado',
  })
}

/**
 * Volta conjunta: A e B no mesmo campo, sem operador novo.
 * A∘B: reverte δ_{e_7}, depois o retorno no campo recuperado.
 * B∘A: retorno primeiro; reversão em δ_{e_0} (L_0 já obtido).
 */
export function residuoComposicao () {
  const f7 = campoLei(7)
  const a = residuoCicloA(f7)
  const b = residuoCicloB()

  const fA = Finv(F(f7))
  let iguais = fA.length === f7.length
  for (let i = 0; i < f7.length && iguais; i++) {
    if (fA[i] !== f7[i]) iguais = false
  }
  const hatAB = F(fA)
  const recAB = leIndiceDoEspectro(hatAB)
  const piAB = recAB == null ? null : piU(recAB)
  const resAB = (piAB === null || L0.k == null) ? 1 : (piAB - L0.k)

  const a0 = residuoCicloA(campoLei(0))
  const resBA = (b.res === 0 && a0.res === 0) ? 0 : 1

  const conjunta = (a.res === 0 && b.res === 0 && resAB === 0 && resBA === 0 && iguais)
    ? 0
    : 1

  return Object.freeze({
    A: a,
    B: b,
    AoB: Object.freeze({
      formula: 'pi_U F (F^{-1} F (L_7)) = L_0',
      res: resAB,
      recuperado: recAB,
      pi: piAB,
      campo_reverteu: iguais,
      estatuto: resAB === 0 ? 'realizado' : 'descartado',
    }),
    BoA: Object.freeze({
      formula: 'pi_U F(L_7)=L_0 e F^{-1} F(L_0)=L_0',
      res: resBA,
      reversao_L0: a0.res,
      estatuto: resBA === 0 ? 'realizado' : 'descartado',
    }),
    res: conjunta,
    estatuto: conjunta === 0 ? 'realizado' : 'descartado',
    transformacao_nova: false,
    L7,
    L0,
  })
}

/** Pental: i^4=1, i^2=-id. psi_roda de lib/quantico.h. Não é Duo. */
export function psiRoda (p) {
  return { a: -p.b, b: p.a, c: -p.d, d: p.c }
}

function psiEq (x, y) {
  return x.a === y.a && x.b === y.b && x.c === y.c && x.d === y.d
}

export function residuoPental () {
  const p = { a: 1, b: 2, c: 3, d: 4 }
  const i1 = psiRoda(p)
  const i2 = psiRoda(i1)
  const i3 = psiRoda(i2)
  const i4 = psiRoda(i3)
  const neg = { a: -p.a, b: -p.b, c: -p.c, d: -p.d }
  const ok4 = psiEq(i4, p)
  const ok2 = psiEq(i2, neg)
  const res = ok4 && ok2 ? 0 : 1
  return Object.freeze({
    formula: 'i^4=1',
    fonte: 'lib/quantico.h',
    codigo: 'psi_roda',
    i2_neg: ok2,
    i4_id: ok4,
    res,
    estatuto: res === 0 ? 'realizado' : 'descartado',
    nao_e_duo: true,
    nao_e_estrela: true,
    nao_e_alonzo: true,
    nao_e_m0: true,
    aresta_lei: false,
  })
}

/** Duo estrutural: troca das duas ops; D^2=id. ≠ Lei 6. ≠ pental. */
export function duoTroca (ops) {
  return { plus: ops.times, times: ops.plus }
}

export function residuoDuo () {
  const ops = Object.freeze({
    plus: (a, b) => (a ^ b) >>> 0,
    times: (a, b) => (a & b) >>> 0,
  })
  const d2 = duoTroca(duoTroca(ops))
  const res = (d2.plus === ops.plus && d2.times === ops.times) ? 0 : 1
  return Object.freeze({
    formula: 'D^2=id',
    fonte: 'univ:def:star',
    star: 'Star(U)=D',
    res,
    estatuto: res === 0 ? 'realizado' : 'descartado',
    nao_e_lei6: true,
    nao_e_pental: true,
    aresta_lei: false,
  })
}

/** ν∘ν=id no byte (lei1_dual). NÃO é aresta L0→L1. */
export function lei1Dual (x) {
  return (~x) & 0xFF
}

export function residuoNuByte () {
  let res = 0
  for (let x = 0; x < 256; x++) {
    if (lei1Dual(lei1Dual(x)) !== x) {
      res = 1
      break
    }
  }
  return Object.freeze({
    formula: 'nu o nu = id',
    fonte: 'lib/umbit.h',
    codigo: 'lei1_dual',
    def: 'fis:def:op',
    res,
    estatuto: res === 0 ? 'realizado' : 'descartado',
    aresta_L0_L1: false,
    palco_poincare: 'nao localizada',
  })
}

/** Rotor do byte (lei5_rotor): período 4. ≠ psi_roda; mesma Lei 5 no chão. */
export function lei5Rotor (x) {
  return ((x << 2) | (x >> 6)) & 0xFF
}

export function residuoRotorByte () {
  let res = 0
  for (let x = 0; x < 256; x++) {
    let y = x
    for (let k = 0; k < 4; k++) y = lei5Rotor(y)
    if (y !== x) {
      res = 1
      break
    }
  }
  return Object.freeze({
    formula: 'lei5_rotor^4 = id',
    fonte: 'lib/umbit.h',
    codigo: 'lei5_rotor',
    res,
    estatuto: res === 0 ? 'realizado' : 'descartado',
    nao_e_psi_roda: true,
    aresta_lei: false,
  })
}

export function residuoInd8 () {
  let k = 0
  for (let i = 0; i < 8; i++) k = Ind(k)
  const meta = MetaInd(7) === 0 && MetaInd(0) === null
  const res = (k === 0 && meta && piU(7) === 0) ? 0 : 1
  return Object.freeze({
    formula: 'Ind^8 = id',
    fonte: 'univ:obs:tecidos-periodo',
    res,
    meta_so_em_7: meta,
    estatuto: res === 0 ? 'realizado' : 'descartado',
  })
}

export function medeGrafo () {
  const comp = residuoComposicao()
  const pental = residuoPental()
  const duo = residuoDuo()
  const nu = residuoNuByte()
  const rotor = residuoRotorByte()
  const ind8 = residuoInd8()
  const ciclos = Object.freeze([
    Object.freeze({ id: 'A', res: comp.A.res, estatuto: comp.A.estatuto }),
    Object.freeze({ id: 'B', res: comp.B.res, estatuto: comp.B.estatuto }),
    Object.freeze({ id: 'AoB', res: comp.AoB.res, estatuto: comp.AoB.estatuto }),
    Object.freeze({ id: 'BoA', res: comp.BoA.res, estatuto: comp.BoA.estatuto }),
    Object.freeze({ id: 'conjunta', res: comp.res, estatuto: comp.estatuto }),
    Object.freeze({ id: 'pental', res: pental.res, estatuto: pental.estatuto }),
    Object.freeze({ id: 'Duo', res: duo.res, estatuto: duo.estatuto }),
    Object.freeze({ id: 'nu2', res: nu.res, estatuto: nu.estatuto }),
    Object.freeze({ id: 'rotor4', res: rotor.res, estatuto: rotor.estatuto }),
    Object.freeze({ id: 'Ind8', res: ind8.res, estatuto: ind8.estatuto }),
  ])
  return Object.freeze({
    nos: Object.freeze([...NOS_LEI, ...NOS_AUX]),
    arestas: arestasMedidas(),
    ausentes: arestasAusentes(),
    composicao: comp,
    pental,
    duo,
    nu,
    rotor,
    ind8,
    ciclos,
    res: ciclos.every((c) => c.res === 0) ? 0 : 1,
    fonte: 'univ:obs:grafo-fecho',
    fis: 'fis:obs:quantica-grafo-fecho',
    sem_lei8: true,
    sem_par_1_7: true,
  })
}
