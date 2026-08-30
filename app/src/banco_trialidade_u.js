// banco_trialidade_u.js — Tri(S); canónico = π∘π=π (Alonzo).
// Trialidade realizada = ∩ Img(π_C) ≅ L_7, resíduo 0. T³ não é a definição.
// Lê o construído. Não inventa f_AT. 0 promoções. Sem Lei 8.
// Duo ≠ Tri ≠ S3 ≠ π. Distingue S3 dos eixos de T das línguas.

export const LINGUAS = Object.freeze(['Alg', 'Top', 'Ana'])
export const MAPAS = Object.freeze(['f_AT', 'f_TA', 'f_AA'])

export const RECUSAS = Object.freeze([
  'T^3 por axioma',
  'T^3 como definicao de trialidade',
  'encaixar Tri em Duo',
  'fundir S3 dos eixos com T das linguas',
  'fundir Duo, Tri, S3 e pi',
  'Mandelbrot por decreto',
  'hexal 6 como evidência de T^3',
  'adjuncao delta⊣ε como T',
  'forcar T^3=id sobre e o e = e',
  'apagar pi do Alonzo se a interseccao tripla nao fechar',
  'Lei 8',
  'Ficha 11',
  'reabrir I0 Fractal/Alonzo',
  'fundir fractal/banco/negro num corpo',
])

/** Implementação: função ou teorema que constrói o mapa. Menção na Def. do teste não conta. */
const PADROES_MAPA = Object.freeze({
  f_AT: [
    /(?:export\s+)?function\s+f_AT\b/,
    /static\s+\w+\s+f_AT\s*\(/,
    /\\label\{(?:univ|fis):thm:(?:ciclo-tri|f-AT|mapa-alg-top)\}/,
  ],
  f_TA: [
    /(?:export\s+)?function\s+f_TA\b/,
    /static\s+\w+\s+f_TA\s*\(/,
    /\\label\{(?:univ|fis):thm:(?:ciclo-tri|f-TA|mapa-top-ana)\}/,
  ],
  f_AA: [
    /(?:export\s+)?function\s+f_AA\b/,
    /static\s+\w+\s+f_AA\s*\(/,
    /\\label\{(?:univ|fis):thm:(?:ciclo-tri|f-AA|mapa-ana-alg)\}/,
  ],
})

export function tri (S) {
  const id = S == null ? '' : String(S)
  return Object.freeze({
    Alg: id === '' ? 'S_Alg' : id + '_Alg',
    Top: id === '' ? 'S_Top' : id + '_Top',
    Ana: id === '' ? 'S_Ana' : id + '_Ana',
  })
}

export function procurarMapa (fonte, nome) {
  const pads = PADROES_MAPA[nome]
  if (!pads || fonte == null) return null
  const txt = String(fonte)
  for (const p of pads) {
    if (p.test(txt)) return nome
  }
  return null
}

export function procurarCiclo (fontes) {
  const encontrados = {}
  for (const nome of MAPAS) {
    let hit = null
    for (const f of fontes || []) {
      hit = procurarMapa(f, nome)
      if (hit) break
    }
    encontrados[nome] = hit
  }
  const existem = MAPAS.every((n) => encontrados[n] != null)
  return {
    mapas: encontrados,
    existem,
    residuo: existem ? null : null,
    composto_id: false,
  }
}

/** Corpos cuja evidência já escreve as três projeções do mesmo S. Não promove os outros. */
export function corposComTriplo (tex, man) {
  const out = []
  const temAlonzo =
    tex &&
    /\\label\{univ:def:alonzo-real\}/.test(tex) &&
    /mathsf\{Real\}_\{\\mathrm\{fractal\}\}/.test(tex)
  if (temAlonzo) {
    const frac = ((man && man.corpos && man.corpos.lista) || [])
      .find((c) => c && c.parte === 'Fractal' && c.canonico === 'Alonzo')
    out.push({
      id: 'Alonzo',
      parte: 'Fractal',
      estatuto_corpo: frac ? frac.estatuto : 'realizado',
      evidencias: [
        'univ:def:alonzo-real',
        'univ:def:alonzo-idemp',
        'fis:def:alonzo',
        'cat:audit:alonzo',
      ],
    })
  }
  return out
}

/** Idempotência já medida na construção Alonzo: π∘π=π. Não exige f_AT. */
export function medirIdempotencia (tex, fis) {
  const t = tex == null ? '' : String(tex)
  const f = fis == null ? '' : String(fis)
  const pi =
    /\\label\{univ:def:alonzo-idemp\}/.test(t) &&
    /\\pi\\circ\\pi=\\pi/.test(t) &&
    /mathsf\{Real\}_\{\\mathrm\{fractal\}\}/.test(t)
  const gamma =
    /\\gamma=\\delta\\varepsilon/.test(t) &&
    /\\gamma\\circ\\gamma=\\gamma/.test(t)
  const lei7 =
    /operatorname\{vinco\}/.test(t) &&
    /L_0/.test(t) &&
    /L_7/.test(t)
  const alonzo =
    /\\label\{univ:def:alonzo-real\}/.test(t) &&
    /\\label\{fis:def:alonzo\}/.test(f)
  const negro = /\\label\{fis:thm:fractalnegro\}/.test(f)
  const realizado = !!(pi && alonzo)
  return Object.freeze({
    objeto: 'projeccao idempotente',
    formula: 'pi o pi = pi',
    ordem: 2,
    estatuto: realizado ? 'realizado' : 'nao localizada',
    gamma: gamma ? 'abertura' : 'nao localizada',
    lei7: lei7 ? 'vinco' : 'nao localizada',
    negro: negro ? 'par S,S^v' : 'nao localizada',
    Duo2: 'involucao',
    e2e: 'projeccao',
    mesmos: false,
  })
}

/** Três S da mesma L. Não funde sítios. Negro ≠ buraco negro. */
export function tresSuportes (tex, fis) {
  const t = tex == null ? '' : String(tex)
  const f = fis == null ? '' : String(fis)
  const tab = /\\label\{univ:obs:tres-suportes\}/.test(t)
  return Object.freeze([
    Object.freeze({
      S: 'fractal',
      C: 'Alonzo',
      mapa: 'Real_fractal',
      parte: 'Fractal',
      fecho: 'pi o pi = pi',
      presente: tab && /\\label\{univ:def:alonzo-idemp\}/.test(t),
    }),
    Object.freeze({
      S: 'banco',
      C: 'M_WASM',
      mapa: 'Real_banco',
      parte: 'operacional',
      fecho: 'merge idempotente',
      presente: tab && /\\label\{univ:def:maquina\}/.test(t),
    }),
    Object.freeze({
      S: 'negro',
      C: '{S,S^v}',
      mapa: 'fractalnegro',
      parte: 'Algebra/Alonzo',
      fecho: 'S · S^v = 1',
      presente: tab && /\\label\{fis:thm:fractalnegro\}/.test(f),
    }),
  ])
}

export function testeTrialidade (fontes, tex, man, fis, extras) {
  const ciclo = procurarCiclo(fontes)
  const corpos = corposComTriplo(tex, man)
  const idemp = medirIdempotencia(tex, fis)
  const setas = ciclo.existem && ciclo.composto_id && ciclo.residuo === 0
  const matriz = medirMatrizProjeccoes(tex, fis, extras)
  return {
    tri: tri('S'),
    corpos,
    ciclo,
    idempotencia: idemp.estatuto,
    formula: idemp.formula,
    leitura: idemp.estatuto === 'realizado'
      ? 'idempotencia da realizacao tripla'
      : 'tres linguas simultaneas',
    T3: setas ? 'realizado' : 'leitura',
    trialidade_realizada: matriz.interseccao,
    matriz,
    promocoes: 0,
    recusas: RECUSAS.slice(),
  }
}

export function alonzoEstatuto (tex, fontes, fis) {
  const ciclo = procurarCiclo(fontes)
  const idemp = medirIdempotencia(tex, fis)
  const tres =
    tex &&
    /\\label\{univ:def:alonzo-real\}/.test(tex) &&
    /\\label\{univ:def:linguas\}/.test(tex) &&
    /\\label\{univ:obs:alonzo-tri\}/.test(tex)
  const setas = ciclo.existem && ciclo.composto_id && ciclo.residuo === 0
  return {
    suporte: 'Alonzo',
    tres_linguas: !!tres,
    idempotencia: idemp.estatuto,
    formula: idemp.formula,
    T3: setas ? 'realizado' : 'leitura',
    leitura: idemp.estatuto === 'realizado'
      ? 'idempotencia da realizacao tripla'
      : 'tres linguas simultaneas',
    promocoes: 0,
  }
}

/** Duo (ordem 2, troca) ≠ e∘e=e (projecção) ≠ Tri (leitura) ≠ S3 (eixos). lcm=6 é rótulo. */
export function relacaoDuoTri () {
  return Object.freeze({
    ord_Duo: 2,
    Duo2_id: true,
    ord_e: 2,
    e2e: true,
    Duo_eq_e: false,
    ord_T: 'leitura',
    T3_id: 'leitura',
    T3_e_definicao: false,
    lcm_2_3: 6,
    hexal_e_T3: false,
    hexal_e_Duo: false,
    duo_actua_linguas: 'nao localizada',
    diedral_linguas: 'nao localizada',
    adjuncao_e_T: false,
    candidato: 'mapa periodos 2,3,6',
    canonico: 'pi o pi = pi',
    quatro: 'Duo != Tri != S3 != pi',
    gap: 'f_AA o f_TA o f_AT = id e leitura da idempotencia; nao axioma T^3',
  })
}

export function quatroObjectos () {
  return Object.freeze([
    Object.freeze({
      nome: 'Duo', faz: 'troca ops na mesma realizacao',
      eq: 'Duo^2 = id', estatuto: 'realizado',
    }),
    Object.freeze({
      nome: 'Tri', faz: 'coexistencia das 3 linguas',
      eq: '—', estatuto: 'leitura, nao operador',
    }),
    Object.freeze({
      nome: 'S3', faz: 'permutacao eixos do trial',
      eq: '—', estatuto: 'simetria de leituras',
    }),
    Object.freeze({
      nome: 'pi', faz: 'projeccao canonica idempotente',
      eq: 'pi o pi = pi', estatuto: 'mecanismo — Alonzo realizado',
    }),
  ])
}

/** S3 dos eixos (trialidade.c) ≠ T das línguas. */
export function distinguirObjectos (fonteEixos) {
  const txt = fonteEixos == null ? '' : String(fonteEixos)
  const eixos =
    /S3/.test(txt) &&
    /eixos/.test(txt) &&
    /\{\s*-1,\s*0,\s*\+1\s*\}/.test(txt)
  const linguas =
    /f_AT/.test(txt) ||
    /S_Alg/.test(txt) ||
    /mathrm\{Alg\}/.test(txt)
  return {
    eixos_S3: eixos,
    linguas_T: linguas,
    mesmos: false,
  }
}

function isoShells (a, b) {
  const ka = Object.keys(a.shells || {}).sort()
  const kb = Object.keys(b.shells || {}).sort()
  if (ka.join('\0') !== kb.join('\0')) return false
  for (const n of ka) {
    const sa = a.shells[n]
    const sb = b.shells[n]
    if (!sa || !sb || sa.in !== sb.in || sa.out !== sb.out || sa.t !== sb.t) return false
  }
  return true
}

/** Merge do banco: π∘π=π no estado. Não é projecção sobre L_7. */
export function medirMergeIdemp (mergeFn, vazioFn) {
  if (typeof mergeFn !== 'function' || typeof vazioFn !== 'function') {
    return Object.freeze({ idemp: false, img_L7: false, residuo: 1 })
  }
  const a = vazioFn()
  a.shells.bash = { in: 'x', out: 'x', t: '2026-01-01T00:00:00.000Z' }
  const b = vazioFn()
  b.shells.node = { in: 'y', out: 'y', t: '2026-02-01T00:00:00.000Z' }
  const m = mergeFn(a, b)
  const mm = mergeFn(m, m)
  const aa = mergeFn(a, a)
  const idemp = isoShells(mm, m) && isoShells(aa, a)
  return Object.freeze({
    idemp,
    img_L7: false,
    residuo: idemp ? 0 : 1,
    eq: 'merge o merge = merge',
  })
}

/** Negro: S·S^∨=1 (par). Não é π∘π=π. */
export function medirNegroPar () {
  let fal = 0
  let n = 0
  for (let d = 2; d <= 6; d++) {
    const s = d - 1
    for (let i = 1; i <= 40; i++) {
      const r = 0.25 * i
      const Sn = r ** s
      const Sb = (1 / r) ** s
      n++
      if (Math.abs(Sn * Sb - 1) > 1e-9) fal++
    }
  }
  const produto = fal === 0
  return Object.freeze({
    n,
    fal,
    produto_1: produto,
    idemp: false,
    img_L7: false,
    residuo: produto ? 0 : 1,
    eq: 'S · S^v = 1',
  })
}

/**
 * Matriz F/B/N. π do Alonzo não se apaga.
 * ∩ Img ≅ L_7 só fecha se os três projectarem na mesma fibra.
 */
export function medirMatrizProjeccoes (tex, fis, extras) {
  const idemp = medirIdempotencia(tex, fis)
  const ciclo = extras && extras.ciclo ? extras.ciclo : null
  const merge = extras && extras.merge
    ? extras.merge
    : Object.freeze({ idemp: false, img_L7: false, residuo: 1 })
  const negro = extras && extras.negro ? extras.negro : medirNegroPar()

  const F = Object.freeze({
    S: 'F',
    nome: 'Alonzo',
    eq: 'pi o pi = pi',
    idemp: idemp.estatuto === 'realizado',
    img_L7: idemp.lei7 === 'vinco',
    residuo: idemp.estatuto === 'realizado' ? 0 : 1,
    estatuto: idemp.estatuto,
  })
  const B = Object.freeze({
    S: 'B',
    nome: 'banco',
    eq: merge.eq || 'merge o merge = merge',
    idemp: !!merge.idemp,
    img_L7: false,
    residuo: merge.residuo,
    estatuto: merge.idemp ? 'realizado (estado, nao L_7)' : 'nao localizada',
  })
  const N = Object.freeze({
    S: 'N',
    nome: 'negro',
    eq: negro.eq || 'S · S^v = 1',
    idemp: false,
    par: !!negro.produto_1,
    img_L7: false,
    residuo: negro.residuo,
    estatuto: negro.produto_1 ? 'realizado (par, nao pi)' : 'nao localizada',
  })

  const tresPiL7 = F.idemp && F.img_L7 && B.idemp && B.img_L7 && N.idemp && N.img_L7
  const meta = ciclo && ciclo.residuo === 0 && ciclo.meta && ciclo.piU
  const interseccao = tresPiL7 && meta ? 'realizado' : 'nao localizada'

  return Object.freeze({
    linhas: Object.freeze([F, B, N]),
    F, B, N,
    pi_alonzo: F.estatuto,
    interseccao,
    formula: 'Img F ∩ Img B ∩ Img N ≅ L_7',
    MetaInd_iff_piU: !!(ciclo && ciclo.metaind_iff_piU && ciclo.residuo === 0),
    recusa_T3: true,
    corpos_novos: 0,
  })
}
