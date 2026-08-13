/* rede_dual.js — medidor da rede neural dual na UI
 * (app/src/rede_dual.js · papers/corpo_peano.tex §rede-dual).
 *
 *   §R1  estaca: W=-1, b=c ⇒ u=c−x
 *   §R2  retenção: |u|≤Δ ⇒ h′=h e mesma fala reusa Y
 *   §R3  Hopfield: após aprender, volta recupera Y (match exacto)
 *   §R4  λ⁺+λ⁻≈0 na volta com recall (assinatura dual)
 *
 *   node tests/rede_dual.js
 */
'use strict'

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

;(async () => {
  const {
    criarEstadoRede,
    passoRedeDual,
    aprendeRedeDual,
    potencialEstaca,
    atualizaH,
    featScalar,
    featBits,
    hebbW,
    hopfieldRecall,
    REDE_DELTA,
    aplicacaoHibrida,
    mapaFrente,
    mapaVoltaConjugada,
    medeConjugacao2,
  } = await import('../app/src/rede_dual.js')

  console.log('=== REDE DUAL: estaca · retenção · Hopfield · λ ===\n')

  /* §R1 */
  {
    const c = 0.25
    const x = 0.1
    const u = potencialEstaca(x, c)
    ok('§R1 u=c−x (W=-1, b=c)', Math.abs(u - (c - x)) < 1e-12)
    ok('§R1 |u|≤Δ ⇒ h′=h', atualizaH(0.05, 1, 0.18) === 1)
    ok('§R1 u>Δ ⇒ h=+1', atualizaH(0.5, 0, 0.18) === 1)
    ok('§R1 u<−Δ ⇒ h=−1', atualizaH(-0.5, 0, 0.18) === -1)
  }

  /* §R2 retenção */
  {
    const e = criarEstadoRede({ delta: REDE_DELTA })
    const fala = 'o que e hurwitz'
    let p = passoRedeDual(e, fala)
    ok('§R2 primeira fala: frente (sem padrões)', p.acao === 'frente')
    aprendeRedeDual(e, fala, 'Hurwitz R,C,H,O. papers/corpo_peano.tex', {
      ...p,
      acao: 'frente',
    })
    ok('§R2 após aprende, c=feat(fala)', Math.abs(e.c - featScalar(fala)) < 1e-12)
    p = passoRedeDual(e, fala)
    ok('§R2 mesma fala: banda/retenção', p.banda === true && p.acao === 'reter')
    ok('§R2 recall Y na retenção', !!(p.recall && p.recall.Y.includes('Hurwitz')))
    ok('§R2 h′=h na banda', p.h === p.hAnt)
  }

  /* §R3 Hopfield */
  {
    const e = criarEstadoRede({ delta: 0.05 })
    const f1 = 'mostra a fundação'
    const y1 = 'papers/torre_fundacao.tex'
    const f2 = 'o que faz o maestro'
    const y2 = 'projecta π_k. papers/corpo_peano.tex'
    let p = passoRedeDual(e, f1)
    aprendeRedeDual(e, f1, y1, { ...p, acao: 'frente' })
    p = passoRedeDual(e, f2)
    aprendeRedeDual(e, f2, y2, { ...p, acao: 'frente' })
    ok('§R3 padrões Hebb armazenados (≥2)', e.padroes.length >= 2)
    p = passoRedeDual(e, f1)
    ok('§R3 volta Hopfield recupera Y de f1',
      p.acao === 'volta' && p.recall && p.recall.Y === y1)
    ok('§R3 overlap exacto = 1', !!(p.recall && p.recall.overlap === 1))
    const bits = featBits(f1)
    const W = hebbW(e.padroes)
    const atr = hopfieldRecall(bits, W)
    let bestOv = -2
    let bestY = null
    for (const q of e.padroes) {
      let s = 0
      for (let i = 0; i < atr.length; i++) s += atr[i] * q.bits[i]
      const ov = s / atr.length
      if (ov > bestOv) { bestOv = ov; bestY = q.Y }
    }
    ok('§R3 recall Hebb encontra y1 com overlap>0.3', bestY === y1 && bestOv > 0.3)
  }

  /* §R4 λ */
  {
    const e = criarEstadoRede()
    const fala = 'o que e a lei 1'
    let p = passoRedeDual(e, fala)
    aprendeRedeDual(e, fala, '1†=-1. teoria.tex', { ...p, acao: 'frente' })
    p = passoRedeDual(e, fala)
    ok('§R4 retenção tem recall', p.acao === 'reter' && !!p.recall)
    const f2 = 'mostra a fundação'
    p = passoRedeDual(e, f2)
    aprendeRedeDual(e, f2, 'papers/torre_fundacao.tex', { ...p, acao: 'frente' })
    p = passoRedeDual(e, fala)
    ok('§R4 volta Hopfield com λΣ=0',
      p.acao === 'volta' && Math.abs(p.lambdaSoma) < 1e-12)
  }

  /* §R5 conjugação reversível F_H = D∘F_P⁻¹∘D⁻¹
   * (eval.txt / corpo_peano thm:rede-dual): Hopfield = memória; λ⁻ vem da dualidade.
   * Experimento: DF_H DF_P = I e λ⁺+λ⁻=0 medidos por diferenças finitas. */
  {
    const c = 0.17
    const alpha = 1.7 // |DF_P|≠1 — expansão da estaca (frente)
    const D = (x) => 2 * c - x // involução Dual Sort / Lei 1
    const Fp = (x) => c + alpha * (x - c)
    const FpInv = (y) => c + (y - c) / alpha
    const Fh = (x) => D(FpInv(D(x))) // conjugação: memória da volta dual

    const eps = 1e-7
    let mauJac = 0
    let mauId = 0
    let mauLam = 0
    const xs = [-0.9, -0.4, 0, 0.11, 0.5, 0.88]
    for (const x of xs) {
      const dFp = (Fp(x + eps) - Fp(x - eps)) / (2 * eps)
      const dFh = (Fh(x + eps) - Fh(x - eps)) / (2 * eps)
      if (Math.abs(dFp * dFh - 1) > 1e-6) mauJac++
      if (Math.abs(Fh(Fp(x)) - x) > 1e-10) mauId++
      const lamP = Math.log(Math.abs(dFp))
      const lamH = Math.log(Math.abs(dFh))
      if (Math.abs(lamP + lamH) > 1e-8) mauLam++
    }
    ok('§R5 DF_H·DF_P = I (jacobiano numérico)', mauJac === 0)
    ok('§R5 F_H∘F_P = id no admissível', mauId === 0)
    ok('§R5 λ⁺+λ⁻=0 pela conjugação (não pelo atrator Hopfield)', mauLam === 0)
    // Hopfield overlap ≠ prova de inversão: só memória
    const e = criarEstadoRede({ delta: 0.05 })
    const f = 'perturba pela banda'
    let p = passoRedeDual(e, f)
    aprendeRedeDual(e, f, 'Y-memoria', { ...p, acao: 'frente' })
    p = passoRedeDual(e, f)
    ok('§R5 Hopfield/retenção devolve memória (≠ prova DF_H=DF_P⁻¹)',
      p.acao === 'reter' && p.recall && p.recall.Y === 'Y-memoria')
  }

  /* §R6 ℱ:(x,h)↦(x′,h′) — sistema híbrido (objecto do teorema) */
  {
    const c = 0.2
    const delta = 0.15
    const alpha = 1.7
    // retenção: sequência na banda conserva h
    let h = 1
    let mauRet = 0
    for (let k = 0; k < 12; k++) {
      const x = c + (k % 2 === 0 ? 0.04 : -0.07) // |u|≤Δ
      const F = aplicacaoHibrida(x, h, { c, delta, alpha })
      if (F.ramo !== 'reter' || F.h !== h || F.x !== x) mauRet++
      h = F.h
    }
    ok('§R6 retenção: h_n=h_0 sob perturbações na banda', mauRet === 0 && h === 1)

    // frente (u>Δ) depois volta conjugada fecha em x
    const x0 = c - 0.5 // u=c-x0=+0.5>Δ → h′=+1
    const Fp = aplicacaoHibrida(x0, 0, { c, delta, alpha, ramo: 'frente' })
    ok('§R6 frente: h′=+1 e x′=F_P(x)',
      Fp.ramo === 'frente' && Fp.h === 1 &&
      Math.abs(Fp.x - mapaFrente(x0, c, alpha)) < 1e-12)
    const Fh = aplicacaoHibrida(Fp.x, Fp.h, { c, delta, alpha, ramo: 'volta' })
    ok('§R6 volta: x′=F_H(F_P(x₀))≈x₀ (fecho híbrido)',
      Fh.ramo === 'volta' && Math.abs(Fh.x - x0) < 1e-9)
    ok('§R6 mapaVolta = conjugação D∘F_P⁻¹∘D⁻¹',
      Math.abs(mapaVoltaConjugada(Fp.x, c, alpha) - Fh.x) < 1e-12)

    // passoRedeDual expõe X=(x,h)
    const e = criarEstadoRede({ c: 0, delta: 0.18 })
    const p = passoRedeDual(e, 'primeira fala nova')
    ok('§R6 passo devolve X e X′=ℱ(X)',
      p.X && p.Xn && typeof p.X.x === 'number' && p.F && p.F.ramo)
  }

  /* §R7 conjugação em ℝ² — DF_H DF_P = I (sobe a dim do experimento) */
  {
    const m = medeConjugacao2()
    ok('§R7 dim2: DF_H·DF_P = I (Frobenius)', m.mauJac === 0)
    ok('§R7 dim2: F_H∘F_P = id', m.mauId === 0)
    ok('§R7 dim2: λ⁺+λ⁻=0 via log|det|', m.mauLam === 0)
    // matriz com shear (não diagonal) — mesma dualidade
    const m2 = medeConjugacao2({
      A: [[1.2, 0.8], [0.1, 1.9]],
      c: [-0.15, 0.25],
    })
    ok('§R7 dim2 shear: conjugação fecha', m2.ok === true)
  }

  console.log(`\n#TOTAL ${feitas} ${falhas}`)
  process.exit(falhas ? 1 : 0)
})().catch((err) => {
  console.error(err)
  process.exit(1)
})
