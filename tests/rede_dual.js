/* rede_dual.js — medidor da rede neural dual na UI
 * (app/src/rede_dual.js · papers/corpo_topologico.tex §rede-dual).
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

/* comparação exacta em escala inteira — sem limiar 1e-N */
const esc = (x) => Math.round(x * 1e12)

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
    ok('§R1 u=c−x (W=-1, b=c)', u === c - x)
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
    aprendeRedeDual(e, fala, 'Hurwitz R,C,H,O. papers/corpo_topologico.tex', {
      ...p,
      acao: 'frente',
    })
    ok('§R2 após aprende, c=feat(fala)', e.c === featScalar(fala))
    p = passoRedeDual(e, fala)
    ok('§R2 mesma fala: banda/retenção', p.banda === true && p.acao === 'reter')
    ok('§R2 recall Y na retenção', !!(p.recall && p.recall.Y.includes('Hurwitz')))
    ok('§R2 h′=h na banda', p.h === p.hAnt)
  }

  /* §R3 Hopfield */
  {
    const e = criarEstadoRede({ delta: 0.05 })
    const f1 = 'mostra a fundação'
    const y1 = 'corpus/docs/torre_fundacao.tex'
    const f2 = 'o que faz o maestro'
    const y2 = 'projecta π_k. papers/corpo_topologico.tex'
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
    /* auditoria 20/08: estava `bestOv > 0.3` — um limiar escrito à mão, e a esconder
     * os dois factos que aqui há. O overlap é INTEIRO sobre inteiro (os bits são ±1,
     * logo a soma é um inteiro e o denominador é o comprimento), e medido: o padrão
     * certo dá 32/32 — recall PERFEITO, não «acima de um limiar» — e o outro dá
     * −2/32. Dizer «> 0,3» perdia a exactidão de um lado e a MARGEM do outro.
     *
     * Agora não há número escolhido: afirma-se que o recall é EXACTO (s = n) e que
     * ele está estritamente acima de todos os outros padrões — uma comparação entre
     * quantidades medidas, que é o que separa recuperar de acertar por sorte. */
    const ovs = e.padroes.map((q) => {
      let s = 0
      for (let i = 0; i < atr.length; i++) s += atr[i] * q.bits[i]
      return { Y: q.Y, s, n: atr.length }
    })
    let melhor = ovs[0]
    for (const o of ovs) if (o.s > melhor.s) melhor = o
    const bestY = melhor.Y
    const exacto = melhor.s === melhor.n                     /* overlap = 1, sem vírgula */
    const segundo = ovs.filter((o) => o !== melhor).reduce((a, o) => (a === null || o.s > a.s ? o : a), null)
    const separa = segundo === null || melhor.s > segundo.s
    console.log('      overlaps (inteiros): ' + ovs.map((o) => o.s + '/' + o.n).join('  ') +
                ' — margem ' + (segundo ? melhor.s - segundo.s : '—'))
    ok('§R3 o recall de Hebb encontra y1 e o overlap é EXACTAMENTE 1 (' + melhor.s + '/' +
       melhor.n + '), com margem ' + (segundo ? melhor.s - segundo.s : '—') + ' sobre o padrão' +
       ' seguinte. Sem limiar: os bits são ±1, logo o overlap é um inteiro sobre um inteiro,' +
       ' e o que se afirma é a igualdade s = n e a SEPARAÇÃO entre quantidades medidas. O' +
       ' «> 0,3» que aqui estava era um número meu, e escondia que o recall é perfeito',
      bestY === y1 && exacto && separa)
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
    aprendeRedeDual(e, f2, 'corpus/docs/torre_fundacao.tex', { ...p, acao: 'frente' })
    p = passoRedeDual(e, fala)
    ok('§R4 volta Hopfield com λΣ=0',
      p.acao === 'volta' && p.lambdaSoma === 0)
  }

  /* §R5 conjugação reversível F_H = D∘F_P⁻¹∘D⁻¹ — álgebra exacta, sem diferenças finitas */
  {
    /* auditoria 20/08: `alpha*(1/alpha)===1` e `log(alpha)+log(1/alpha)===0`
     * passavam por ACIDENTE do IEEE, não por álgebra — varridos os α de 1,10 a
     * 3,00 em centésimas, o primeiro falha em 23 de 191 e o segundo em 91 de
     * 191. Com α=1,7 calha darem verdade; com α=1,9 o segundo dá falso e a
     * conjugação é a MESMA. Aqui o objecto é racional (c=17/100, α=17/10), e
     * mede-se em racionais exactos — e os multiplicadores não se escrevem:
     * TIRAM-SE dos mapas, que é o que faz disto dois caminhos e não uma
     * identidade copiada de um lado para o outro. */
    const F = (n, d) => { /* racional reduzido, denominador positivo */
      n = BigInt(n); d = BigInt(d)
      if (d < 0n) { n = -n; d = -d }
      let a = n < 0n ? -n : n, b = d
      while (b) { [a, b] = [b, a % b] }
      const g = a || 1n
      return { n: n / g, d: d / g }
    }
    const add = (x, y) => F(x.n * y.d + y.n * x.d, x.d * y.d)
    const sub = (x, y) => F(x.n * y.d - y.n * x.d, x.d * y.d)
    const mul = (x, y) => F(x.n * y.n, x.d * y.d)
    const div = (x, y) => F(x.n * y.d, x.d * y.n)
    const eq = (x, y) => x.n === y.n && x.d === y.d
    const UM = F(1, 1)

    /* c = 0,17 e α = 1,7 são RACIONAIS escritos em decimal — 17/100 e 17/10 */
    const cq = F(17, 100), aq = F(17, 10)
    const Dm = (x) => sub(mul(F(2, 1), cq), x)                 /* D(x) = 2c − x */
    const frente = (a) => (x) => add(cq, mul(a, sub(x, cq)))   /* F_P(x) = c + α(x−c) */
    const inversa = (a) => (y) => add(cq, div(sub(y, cq), a))  /* F_P⁻¹ */
    const volta = (a) => (x) => Dm(inversa(a)(Dm(x)))          /* F_H = D∘F_P⁻¹∘D */
    const Fp = frente(aq), Fh = volta(aq)

    /* o multiplicador NÃO se escreve: mede-se do mapa, em dois pontos racionais */
    const mult = (G) => { const x0 = F(0, 1), x1 = F(1, 1); return div(sub(G(x1), G(x0)), sub(x1, x0)) }
    const mP = mult(Fp), mH = mult(Fh)

    let mauId = 0
    const xs = [F(-9, 10), F(-2, 5), F(0, 1), F(11, 100), F(1, 2), F(22, 25)]
    for (const x of xs) {
      if (!eq(Fh(Fp(x)), x)) mauId++
    }
    ok('§R5 DF_H·DF_P = I — os dois multiplicadores TIRADOS dos mapas (não escritos), e o produto' +
       ' é 1 em racionais exactos: ' + mP.n + '/' + mP.d + ' · ' + mH.n + '/' + mH.d,
      eq(mul(mP, mH), UM) && !eq(mP, UM))
    ok('§R5 F_H∘F_P = id no admissível (igualdade EXACTA de racionais, sem escala 1e12)', mauId === 0 && xs.length >= 6)
    /* λ⁺+λ⁻ = log m_P + log m_H = 0 ⟺ m_P·m_H = 1. E o que se afirma não é que
     * calha em α=1,7: é que vale em TODA a família — se a construção de F_H
     * estivesse errada, caía aqui e não no α escolhido. */
    let alphas = 0, mausL = 0
    for (let k = 110; k <= 300; k++) {
      const a2 = F(k, 100)
      alphas++
      if (!eq(mul(mult(frente(a2)), mult(volta(a2))), UM)) mausL++
    }
    ok('§R5 λ⁺+λ⁻=0 pela conjugação (não pelo atrator Hopfield): log m_P + log m_H = 0 ⟺ m_P·m_H = 1,' +
       ' e vale em ' + alphas + ' α racionais de 1,10 a 3,00 — não só no 1,7 que estava escrito.' +
       ' Falhas: ' + mausL,
      mausL === 0 && alphas === 191)
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
    let h = 1
    let mauRet = 0
    for (let k = 0; k < 12; k++) {
      const x = c + (k % 2 === 0 ? 0.04 : -0.07)
      const F = aplicacaoHibrida(x, h, { c, delta, alpha })
      if (F.ramo !== 'reter' || F.h !== h || F.x !== x) mauRet++
      h = F.h
    }
    ok('§R6 retenção: h_n=h_0 sob perturbações na banda', mauRet === 0 && h === 1)

    const x0 = c - 0.5
    const Fp = aplicacaoHibrida(x0, 0, { c, delta, alpha, ramo: 'frente' })
    ok('§R6 frente: h′=+1 e x′=F_P(x)',
      Fp.ramo === 'frente' && Fp.h === 1 &&
      Fp.x === mapaFrente(x0, c, alpha))
    const Fh = aplicacaoHibrida(Fp.x, Fp.h, { c, delta, alpha, ramo: 'volta' })
    ok('§R6 volta: x′=F_H(F_P(x₀))≈x₀ (fecho híbrido)',
      Fh.ramo === 'volta' && esc(Fh.x) === esc(x0))
    ok('§R6 mapaVolta = conjugação D∘F_P⁻¹∘D⁻¹',
      mapaVoltaConjugada(Fp.x, c, alpha) === Fh.x)

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
