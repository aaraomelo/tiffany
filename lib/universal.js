/* lib/universal.js — 𝒰: o Corpo Universal como INFRAESTRUTURA
 * (ordem da mesa, eval 14/08: «Universal não é mais um paper. É a
 * infraestrutura.» — migração cirúrgica, fase 2).
 *
 * Este módulo é AGNÓSTICO: parametriza-se por uma assinatura
 *   σ = { p (o anel), bytes (a leitura do corpo), endereco (o id) }
 * e NÃO conhece nenhuma realização: as realizações concretas fornecem σ
 * e recebem os operadores — este ficheiro não nomeia nenhuma delas.
 *
 * PROVENIÊNCIA (nenhuma linha nova de teoria — só promoção do que os
 * medidores já atestaram, semântica byte a byte):
 *   energia/escada       ← equivalencia_universal.js (lado 𝒰, 7:0)
 *   rEndereco            ← cristal_volta.js v2 (16:0), pelo ENDEREÇO
 *   transicoes           ← residuos_totais.js (a leitura da membrana)
 *   residuoTotal/retain  ← residuos_totais.js (9:0): RETAIN ⟺ tudo zera
 *   funde/esqueleto/fibra← fusao_conceitos.js (8:0) e cristal_curadoria.js
 *   monodromia (ν)       ← contorno_riemann.js (23:0)
 *   contorno             ← contorno_riemann.js (pilha fora das cordas)
 *   mat2 (J, espelho, A_m, W, estaca, cartas)
 *                        ← contorno_riemann.js / morfologia_universal.js
 *
 * Regras da casa: inteiro puro (zero doubles), sem estado (funções puras,
 * sem memória), toda operação com a sua volta.
 */
'use strict'

function Universal (sigma) {
  /* a escada do observador: I₃ = (E, Φ, Φ₂) no anel σ.p */
  function escada (texto) {
    const b = sigma.bytes(texto)
    let E = 0, f1 = 0, f2 = 0
    for (let i = 0; i < b.length; i++) {
      E += b[i] * b[i]
      f1 = (f1 + (i + 1) * b[i]) % sigma.p
      f2 = (f2 + ((i + 1) * (i + 1) % sigma.p) * b[i]) % sigma.p
    }
    return { E, f1, f2 }
  }

  /* a energia (a massa da cruz): E = Σ b², inteiro exato */
  function energia (obj) {
    let E = 0
    for (const item of obj) {
      const b = sigma.bytes(item)
      for (let i = 0; i < b.length; i++) E += b[i] * b[i]
    }
    return E
  }

  /* o resíduo pelo ENDEREÇO: R(a) = Back(a) − a — faltantes, alterados,
   * excedentes e duplicados; a ordem física é derivada e não participa */
  function rEndereco (fonte, regs) {
    const F = new Map()
    for (let i = 0; i < fonte.length; i++) F.set(sigma.endereco(fonte[i], i), fonte[i])
    const visto = new Map(), vezes = new Map()
    for (let i = 0; i < regs.length; i++) {
      const a = sigma.endereco(regs[i], i)
      if (!visto.has(a)) visto.set(a, regs[i])
      vezes.set(a, (vezes.get(a) || 0) + 1)
    }
    let r = 0
    for (const [a, l] of F) {
      const v = visto.get(a)
      if (v === undefined) r++
      else if (v !== l) r++
    }
    for (const [a, n] of vezes) {
      if (!F.has(a)) r++
      if (n > 1) r += n - 1
    }
    return r
  }

  /* a leitura da membrana: as transições (pares consecutivos) do corpo */
  function transicoes (texto) {
    const b = sigma.bytes(texto)
    const t = []
    for (let i = 0; i + 1 < b.length; i++) t.push(b[i] * 256 + b[i + 1])
    return t
  }

  /* o vetor de auditoria total: R = (R_end, R_E, R_Φ, R_Φ₂, R_D) sobre
   * listas de [endereço, corpo]; RETAIN ⟺ TODOS os componentes zeram */
  function residuoTotal (fonte, cand) {
    const F = new Map(fonte)
    const C = new Map()
    const vezes = new Map()
    for (const [a, s] of cand) {
      if (!C.has(a)) C.set(a, s)
      vezes.set(a, (vezes.get(a) || 0) + 1)
    }
    let Rend = 0, RE = 0, RF1 = 0, RF2 = 0, RD = 0
    for (const [a, s0] of F) {
      const s1 = C.get(a)
      if (s1 === undefined) { Rend++; continue }
      if (s1 !== s0) Rend++
      const i0 = escada(s0), i1 = escada(s1)
      RE += Math.abs(i1.E - i0.E)
      RF1 += (i1.f1 - i0.f1 + sigma.p) % sigma.p
      RF2 += (i1.f2 - i0.f2 + sigma.p) % sigma.p
      const t0 = transicoes(s0), t1 = transicoes(s1)
      if (t0.length !== t1.length) RD++
      else if (t0.some((v, i) => v !== t1[i])) RD++
    }
    for (const [a, n] of vezes) {
      if (!F.has(a)) Rend++
      if (n > 1) Rend += n - 1
    }
    return { Rend, RE, RF1, RF2, RD }
  }
  const retain = R =>
    R.Rend === 0 && R.RE === 0 && R.RF1 === 0 && R.RF2 === 0 && R.RD === 0

  /* a fusão: soma direta com contorno — as partes viajam VERBATIM */
  function funde (idZ, lx, ly) {
    return '{"fusao":[' + lx + ',' + ly + '],"id":"' + idZ + '","tipo":"conceito"}'
  }
  function esqueleto (idZ) { return funde(idZ, '', '') }

  /* a fibra (a divisão): corta o TEXTO na vírgula de profundidade 0 —
   * a volta é byte a byte, por isso não se re-serializa */
  function corte (lz) {
    const ini = lz.indexOf('[') + 1
    const fim = lz.lastIndexOf(']')
    let prof = 0
    for (let i = ini; i < fim; i++) {
      if (lz[i] === '{') prof++
      else if (lz[i] === '}') prof--
      else if (lz[i] === ',' && prof === 0) return i
    }
    return -1
  }
  function fibra (lz) {
    const c = corte(lz)
    if (c < 0) return null
    const ini = lz.indexOf('[') + 1
    const fim = lz.lastIndexOf(']')
    return [lz.slice(ini, c), lz.slice(c + 1, fim)]
  }

  /* a monodromia: troca as folhas; ν∘ν = id byte a byte */
  function monodromia (lz) {
    const p = fibra(lz)
    if (!p) return null
    return funde(sigma.endereco(p[1], 0), p[1], p[0])
  }

  /* o contorno lido: pilha de tipos FORA das cordas, com escape */
  function contorno (lz) {
    const pilha = []
    let minOk = true, emCorda = false, escapa = false, cruzou = false
    const cortes = []
    let arrayProf = -1
    for (let i = 0; i < lz.length; i++) {
      const ch = lz[i]
      if (emCorda) {
        if (escapa) escapa = false
        else if (ch === '\\') escapa = true
        else if (ch === '"') emCorda = false
        continue
      }
      if (ch === '"') emCorda = true
      else if (ch === '{' || ch === '[') {
        if (ch === '[' && arrayProf < 0) arrayProf = pilha.length + 1
        pilha.push(ch)
      } else if (ch === '}' || ch === ']') {
        const abriu = pilha.pop()
        if (abriu === undefined) minOk = false
        else if ((ch === '}') !== (abriu === '{')) cruzou = true
      } else if (ch === ',' && pilha.length === arrayProf && arrayProf > 0) {
        cortes.push(i)
      }
    }
    return { fecha: pilha.length === 0 && minOk && !emCorda, cruzou, cortes }
  }

  return {
    escada,
    energia,
    rEndereco,
    transicoes,
    residuoTotal,
    retain,
    funde,
    esqueleto,
    fibra,
    corte,          /* a posição do corte da fibra cega — o segundo caminho */
    monodromia,
    contorno,
  }
}

/* ── a geometria 2×2 inteira: a torção, o espelho e o recobrimento ───────── */
const mat2 = {
  mul (X, Y) {
    return [X[0] * Y[0] + X[1] * Y[2], X[0] * Y[1] + X[1] * Y[3],
      X[2] * Y[0] + X[3] * Y[2], X[2] * Y[1] + X[3] * Y[3]]
  },
  soma: (X, Y) => X.map((v, i) => v + Y[i]),
  escala: (k, X) => X.map(v => k * v),
  det: X => X[0] * X[3] - X[1] * X[2],
  tr: X => X[0] + X[3],
  igual: (X, Y) => X.every((v, i) => v === Y[i]),
  I: [1, 0, 0, 1],
  J: [0, 1, -1, 0],                 /* o esquilo: J²=−I, período 4, det=+1 */
  espelho: [1, 0, 0, -1],           /* R²=I, det=−1; RJ=−JR — o par */
  Am: m => [m, 1, 1, 0],            /* a companheira de x² = mx + 1 */
}
/* w = 2x − m: W² = (m²+4)I — o recobrimento realizado um andar acima */
mat2.W = m => mat2.soma(mat2.escala(2, mat2.Am(m)), mat2.escala(-m, mat2.I))
/* a estaca (a troca de folha): A·(mI−A) = −I */
mat2.estaca = m => mat2.soma(mat2.escala(m, mat2.I), mat2.escala(-1, mat2.Am(m)))
/* as duas cartas: (x,Jx) da torção (a²+b², definida) e a do corpo
 * (a²+mab−b², indefinida) — o círculo e a hipérbole, sem os fundir */
mat2.carta = (a, b) => mat2.soma(mat2.escala(a, mat2.I), mat2.escala(b, mat2.J))
mat2.corpo = (a, b, m) => mat2.soma(mat2.escala(a, mat2.I), mat2.escala(b, mat2.Am(m)))

/* ── o NÚCLEO UNIFICADO (fase 4 — ordem da mesa, eval 14/08) ─────────────
 *
 * A quádrupla (X, S, H, J) e as relações do Teorema Universal
 * (teorema_universal.js 13:0). O Universal é dono da ESTRUTURA; as
 * instâncias são donas apenas da realização — e NÃO se nomeiam aqui:
 * o 𝒰 é agnóstico (§M0 mede isto no texto). «Unifica sem apagar as
 * diferenças»:
 *
 *     X = S·J                (o par roda/espelha GERA a troca)
 *     H = X + S,  H² = 2I    (a dobra; Hadamard)
 *     H·X·H = 2S, H·S·H = 2X (a dobra é a DUALIDADE do par)
 *     H·J·H = 2·J⁻¹          (o rotor vai ao inverso — diedral: a medida
 *                             impede colapsar espelho, troca e torção)
 *
 * X e H são DERIVADOS aqui, não escritos: mudar o espelho ou o J muda a
 * quádrupla sozinha (a referência não é cópia). */
const nucleo = (() => {
  const { mul, soma, escala, igual, I, J, espelho } = mat2
  const X = mul(espelho, J)                 /* a troca, derivada */
  const H = soma(X, espelho)                /* a dobra, derivada */
  return {
    X, S: espelho, J, H, I,
    verifica () {
      const Jinv = escala(-1, J)            /* J² = −I ⟹ J⁻¹ = −J */
      return igual(X, mul(espelho, J)) &&
        igual(mul(H, H), escala(2, I)) &&
        igual(mul(mul(H, X), H), escala(2, espelho)) &&
        igual(mul(mul(H, espelho), H), escala(2, X)) &&
        igual(mul(mul(H, J), H), escala(2, Jinv)) &&
        igual(mul(J, Jinv), I)
    },
  }
})()

/* ── o anel, a transformada e a renormalização (fase 3 da migração) ─────── */
function anel (q) {
  const mod = x => ((x % q) + q) % q
  const powm = (b, e) => {
    let r = 1; b = mod(b)
    while (e > 0) { if (e & 1) r = r * b % q; b = b * b % q; e >>= 1 }
    return r
  }
  const inv = a => powm(a, q - 2)
  return { q, mod, powm, inv }
}

/* a DFT no anel: c_k = Σ x_n ω^{−kn}; a inversa devolve byte a byte */
function dft (xs, A, w) {
  const M = xs.length
  const c = []
  for (let k = 0; k < M; k++) {
    let s = 0
    const wk = A.powm(w, M - (k % M))
    let f = 1
    for (let n = 0; n < M; n++) { s = (s + xs[n] * f) % A.q; f = f * wk % A.q }
    c.push(s)
  }
  return c
}
function idft (cs, A, w) {
  const M = cs.length
  const iM = A.inv(M)
  const xs = []
  for (let n = 0; n < M; n++) {
    let s = 0
    const wn = A.powm(w, n)
    let f = 1
    for (let k = 0; k < M; k++) { s = (s + cs[k] * f) % A.q; f = f * wn % A.q }
    xs.push(s * iM % A.q)
  }
  return xs
}

/* a renormalização: a DOBRA lida nos traços — t↦t²−2d, d↦d² (o operador
 * de leitura das leis; serve Number e BigInt, sem misturar) */
const renormaliza = ({ t, d }) => ({ t: t * t - (d + d), d: d * d })

/* a morfologia em ℤ (sem truncar na borda — truncar quebra φ ≥ id) */
const morfo = {
  dilata (S) { const R = new Set(); for (const i of S) { R.add(i); R.add(i + 1) } return R },
  erode (S) { const R = new Set(); for (const i of S) if (S.has(i + 1)) R.add(i); return R },
}
morfo.abre = S => morfo.dilata(morfo.erode(S))
morfo.fecha = S => morfo.erode(morfo.dilata(S))

/* ── AS OITO LEIS: a interface normativa da infraestrutura ────────────────
 * (fase 3, ordem da mesa: «Universal é dono da lei; instâncias são donas
 * apenas da realização».) Cada lei entra com a sua verificação OPERACIONAL
 * — a identidade que os medidores já atestaram, aqui como contrato que
 * qualquer realização tem de honrar. Nada de teoria nova: só o catálogo. */
const leis = [
  {
    n: 0,
    nome: 'o par (+1)⊕(−1) — os dois nulos',
    verifica () {
      /* a curva w²=m²+4 toca ℤ só na Lei 0: enumeração completa */
      const pontos = []
      for (const d of [-4, -2, -1, 1, 2, 4]) {
        const e = 4 / d
        if ((e - d) % 2 !== 0) continue
        pontos.push((((e - d) / 2) + ((e + d) / 2)) / 2)
      }
      return pontos.length === 2 && pontos.sort().join() === '-1,1'
    },
  },
  {
    n: 1,
    nome: 'a estaca ν∘ν=id — x·x†=−1',
    verifica () {
      for (let m = 0; m <= 8; m++) {
        const A = mat2.Am(m), Ad = mat2.estaca(m)
        if (!mat2.igual(mat2.mul(A, Ad), mat2.escala(-1, mat2.I))) return false
        if (!mat2.igual(mat2.soma(mat2.escala(m, mat2.I), mat2.escala(-1, Ad)), A)) return false
      }
      return true
    },
  },
  {
    n: 2,
    nome: 'o rotor J²=−I e o espelho — o par roda/espelha',
    verifica () {
      const { mul, escala, igual, det, I, J, espelho } = mat2
      return igual(mul(J, J), escala(-1, I)) && igual(mul(mul(J, J), mul(J, J)), I) &&
        det(J) === 1 && igual(mul(espelho, espelho), I) && det(espelho) === -1 &&
        igual(mul(espelho, J), escala(-1, mul(J, espelho)))
    },
  },
  {
    n: 3,
    nome: 'o trial {−1,0,+1} — a batuta',
    verifica () {
      /* x³=x caracteriza o trial nos inteiros */
      const trial = []
      for (let x = -5; x <= 5; x++) if (x * x * x === x) trial.push(x)
      return trial.join() === '-1,0,1' &&
        trial.reduce((a, b) => a + b, 0) === 0 &&
        trial.every(x => trial.includes(-x))
    },
  },
  {
    n: 4,
    nome: 'a dobra T+T* — |det|=1, o fator de potência',
    verifica () {
      for (let m = 0; m <= 8; m++) {
        if (Math.abs(mat2.det(mat2.Am(m))) !== 1) return false
        if (mat2.tr(mat2.W(m)) !== 0) return false        /* w=2x−m: a dobra centra */
      }
      return true
    },
  },
  {
    n: 5,
    nome: 'o bit x²=−1 — o ponto fixo da estaca',
    verifica () {
      const { mul, escala, igual, I, J } = mat2
      /* J realiza x²=−1 e é fixo por ν: J⁻¹=−J (medido, não assumido) */
      return igual(mul(J, J), escala(-1, I)) && igual(mul(J, escala(-1, J)), I)
    },
  },
  {
    n: 6,
    nome: 'a interface ⊕=⊗ — lcm(2,3)=6',
    verifica () {
      const gcd = (a, b) => b ? gcd(b, a % b) : a
      return gcd(2, 3) === 1 && (2 * 3) / gcd(2, 3) === 6 && 2 * 3 === 6
    },
  },
  {
    n: 7,
    nome: 'Hurwitz — a conservação é multiplicativa',
    verifica () {
      for (let a = -2; a <= 2; a++) {
        for (let b = -2; b <= 2; b++) {
          for (let c = -2; c <= 2; c++) {
            for (let d = -2; d <= 2; d++) {
              const X = mat2.carta(a, b), Y = mat2.carta(c, d)
              if (mat2.det(mat2.mul(X, Y)) !== mat2.det(X) * mat2.det(Y)) return false
              const V = mat2.corpo(a, b, 1), Z = mat2.corpo(c, d, 1)
              if (mat2.det(mat2.mul(V, Z)) !== mat2.det(V) * mat2.det(Z)) return false
            }
          }
        }
      }
      return true
    },
  },
]
const verificaLeis = () => leis.map(l => ({ n: l.n, nome: l.nome, ok: l.verifica() }))

module.exports = { Universal, mat2, anel, dft, idft, renormaliza, morfo, leis, verificaLeis, nucleo }
