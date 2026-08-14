/* tests/encaixe_continuo.js — os intervalos encaixantes e o contínuo
 * (ordem da mesa, eval 14/08: «ler Viviani na Lei trial e derivar
 * propriedade dos intervalos encaixantes para o contínuo»; fila selada
 * Viviani → trial → ENCAIXE → resíduos → Clifford).
 *
 * O eixo é o de Pontryagin (papers, três partes): a ÁLGEBRA opera e não
 * alcança a completude; a TOPOLOGIA alcança ℝ e não opera. Aqui mede-se
 * exatamente a fronteira: a álgebra inteira constrói uma cadeia de
 * intervalos encaixantes cujos comprimentos são a UNIDADE dividida por
 * q_k·q_{k+1} (o fator de potência |det|=1 é o que faz o encaixe), e a
 * varredura exaustiva mostra que NENHUM racional fica em todos — o
 * ponto comum é um buraco de ℚ. O contínuo é o que o preenche: essa é a
 * propriedade dos intervalos encaixantes, DERIVADA e não postulada.
 *
 *   A MÁQUINA É DA LIB: os convergentes do áureo saem de Am(1)^k
 *   (mat2), a recorrência de Fibonacci é o segundo caminho, e
 *   |det|=±1 (Lei 4) é simultaneamente: os extremos são vizinhos de
 *   Farey, o comprimento é 1/(q_k q_{k+1}), e o sinal alterna — o
 *   zigue-zague que encaixa.
 *
 *   A DUALIDADE TEM AS DUAS PARTES: o par é (φ, 1−φ) com
 *   φ·(1−φ) = −1 em ℤ[φ] — o x·x† = −1 da involução x†=−1/x. A cadeia
 *   conjugada (1−p/q) encaixa para o outro membro, com a mesma unidade.
 *
 *   E A MESMA LEI, A OUTRA MEMBRANA: L_{2k} = L_k² − 2(−1)^k é o R da
 *   lib com d=(−1)^k — a escada áurea é a membrana d=−1 (hipérbole) da
 *   MESMA renormalização cuja membrana d=1 (círculo) desenhou Viviani
 *   (viviani_universal §V2). O par círculo/hipérbole é o par das
 *   membranas, não duas leis.
 *
 * §E0  dois caminhos: Fibonacci por recorrência == Am(1)^k da lib
 *      (k≤40 em Number; a recorrência segue em BigInt até k=90)
 * §E1  a unidade que encaixa: p_{k+1}q_k − p_k q_{k+1} = ±1 alternado,
 *      == det(Am(1)^k+1?) — e o gume: um numerador mutado quebra
 * §E2  o encaixe: I_{k+1} ⊂ I_k por desigualdades cruzadas BigInt até
 *      k=90, e o comprimento fecha: (q_k q_{k+1})·|I_k| = 1 EXATO
 * §E3  o buraco: TODO racional r/s com s≤50 sai de algum I_k (varredura
 *      exaustiva, 1325 candidatos) e nenhum convergente é o ponto:
 *      p²−pq−q² = ±1 ≠ 0 sempre — o ponto comum não está em ℚ
 * §E4  a dualidade com os dois membros: φ·(1−φ) = −1 em ℤ[φ] (pares
 *      inteiros), e a cadeia conjugada encaixa com a MESMA unidade
 * §E5  a outra membrana: L_{2k} = R(L_k) com d=(−1)^k, dois caminhos;
 *      gume: fixar d=1 falha exatamente nos k ímpares
 */
'use strict'
const { mat2, renormaliza } = require('../lib/universal.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

/* Fibonacci em BigInt: F[1]=F[2]=1; convergente c_k = F[k+1]/F[k] */
const K = 90
const F = [0n, 1n, 1n]
for (let k = 3; k <= K + 2; k++) F.push(F[k - 1] + F[k - 2])
const p = k => F[k + 1], q = k => F[k]

/* §E0 — dois caminhos: a recorrência e a matriz da lib */
{
  let iguais = 0
  let M = mat2.Am(1)                       /* [1,1,1,0] — a torção m=1 */
  for (let k = 1; k <= 40; k++) {
    /* Am(1)^k = [F_{k+1}, F_k, F_k, F_{k-1}] */
    if (BigInt(M[0]) === F[k + 1] && BigInt(M[1]) === F[k] && BigInt(M[3]) === F[k - 1]) iguais++
    M = mat2.mul(M, mat2.Am(1))
  }
  console.log(`\n§E0  Am(1)^k == Fibonacci em ${iguais}/40 (Number exato até F_41); BigInt segue até F_${K + 2} = ${String(F[K + 2]).length} dígitos`)
  ok('§E0 dois caminhos: a recorrência de Fibonacci é a potência Am(1)^k da lib, entrada a entrada', iguais === 40)
}

/* §E1 — a unidade que encaixa: o determinante alterna ±1 */
{
  let unidade = 0, alterna = 0
  for (let k = 1; k <= K; k++) {
    const d = p(k + 1) * q(k) - p(k) * q(k + 1)      /* det da matriz de vizinhos */
    if (d === 1n || d === -1n) unidade++
    if (d === (k % 2 === 1 ? 1n : -1n)) alterna++    /* Catalão: F_k F_{k+2} − F_{k+1}² = (−1)^{k+1} */
  }
  /* o gume: numerador mutado quebra a unidade */
  const dm = (p(3) + 1n) * q(2) - p(2) * q(3)
  console.log(`\n§E1  |p_{k+1}q_k − p_k q_{k+1}| = 1 em ${unidade}/${K}, sinal (−1)^k em ${alterna}/${K} · mutado: ${dm}`)
  ok('§E1 o fator de potência faz o encaixe: a unidade ±1 entre vizinhos, com o sinal a alternar — o zigue-zague', unidade === K && alterna === K)
  ok('§E1 o gume: p_3+1 dá det ≠ ±1 — a unidade não é folga', dm !== 1n && dm !== -1n)
}

/* os intervalos: I_k = [min(c_k,c_{k+1}), max(c_k,c_{k+1})], frações BigInt */
const menorIgual = (a, b, c, d) => a * d <= c * b        /* a/b ≤ c/d, b,d > 0 */
const I = []
for (let k = 1; k <= K; k++) {
  const lo = menorIgual(p(k), q(k), p(k + 1), q(k + 1)) ? [p(k), q(k)] : [p(k + 1), q(k + 1)]
  const hi = menorIgual(p(k), q(k), p(k + 1), q(k + 1)) ? [p(k + 1), q(k + 1)] : [p(k), q(k)]
  I.push({ lo, hi })
}

/* §E2 — o encaixe e o comprimento exato */
{
  let encaixa = 0, comprimento = 0
  for (let k = 0; k < K - 1; k++) {
    const A2 = I[k], B = I[k + 1]
    if (menorIgual(A2.lo[0], A2.lo[1], B.lo[0], B.lo[1]) && menorIgual(B.hi[0], B.hi[1], A2.hi[0], A2.hi[1])) encaixa++
  }
  for (let k = 0; k < K; k++) {
    const { lo, hi } = I[k]
    /* |I_k| = hi−lo = (hi0·lo1 − lo0·hi1)/(lo1·hi1); q_k q_{k+1}·|I_k| = 1 ⟺ numerador == 1 */
    if (hi[0] * lo[1] - lo[0] * hi[1] === 1n) comprimento++
  }
  /* o gume do encaixe: trocar um extremo quebra */
  const quebra = !(menorIgual(I[2].lo[0], I[2].lo[1], I[3].hi[0], I[3].hi[1]) &&
                   menorIgual(I[3].hi[0], I[3].hi[1], I[2].lo[0], I[2].lo[1]))
  console.log(`\n§E2  I_{k+1}⊂I_k em ${encaixa}/${K - 1} · (q_k q_{k+1})·|I_k| = 1 em ${comprimento}/${K} · extremos trocados quebram: ${quebra}`)
  ok('§E2 a cadeia encaixa: I_{k+1} ⊂ I_k por desigualdades cruzadas inteiras, até k=90', encaixa === K - 1)
  ok('§E2 o comprimento é a UNIDADE: (q_k q_{k+1})·|I_k| = 1 exato — é o det de §E1 a medir o intervalo', comprimento === K)
  ok('§E2 o gume: com os extremos trocados a inclusão falha', quebra)
}

/* §E3 — o buraco: nenhum racional fica em todos os intervalos */
{
  let candidatos = 0, saemTodos = 0, primeiroTeimoso = -1
  for (let s = 1n; s <= 50n; s++) {
    for (let r = s; r <= 2n * s; r++) {          /* φ ∈ (1,2): basta r/s ∈ [1,2] */
      candidatos++
      let saiu = false
      for (let k = 0; k < K; k++) {
        const { lo, hi } = I[k]
        if (!(menorIgual(lo[0], lo[1], r, s) && menorIgual(r, s, hi[0], hi[1]))) { saiu = true; break }
      }
      if (saiu) saemTodos++
      else if (primeiroTeimoso < 0) primeiroTeimoso = Number(r * 1000n / s)
    }
  }
  /* e nenhum convergente É o ponto: a norma p²−pq−q² = ±1 ≠ 0 */
  let normaUnidade = 0
  for (let k = 1; k <= K; k++) {
    const n = p(k) * p(k) - p(k) * q(k) - q(k) * q(k)
    if (n === 1n || n === -1n) normaUnidade++
  }
  console.log(`\n§E3  candidatos r/s, s≤50: ${candidatos} · saem de algum I_k: ${saemTodos} · normas ±1: ${normaUnidade}/${K}`)
  ok('§E3 o BURACO: todos os ' + candidatos + ' racionais com s≤50 saem de algum I_k — nenhum é o ponto comum', saemTodos === candidatos)
  ok('§E3 e nenhum convergente o é: p²−pq−q² = ±1 ≠ 0 sempre — a cadeia aponta para fora de ℚ; o contínuo é o que preenche', normaUnidade === K)
}

/* §E4 — a dualidade com os DOIS membros: φ e 1−φ, e x·x† = −1 */
{
  /* ℤ[φ] como pares (a,b) = a+bφ, com φ² = φ+1 */
  const mulZ = ([a, b], [c, d]) => [a * c + b * d, a * d + b * c + b * d]
  const prod = mulZ([0n, 1n], [1n, -1n])             /* φ · (1−φ) */
  /* a cadeia conjugada: c_k ↦ 1 − p_k/q_k = (q_k−p_k)/q_k encaixa para o outro membro */
  const Ic = I.map(({ lo, hi }) => ({ lo: [hi[1] - hi[0], hi[1]], hi: [lo[1] - lo[0], lo[1]] }))
  let encaixaC = 0, comprimentoC = 0
  for (let k = 0; k < K - 1; k++) {
    const A2 = Ic[k], B = Ic[k + 1]
    if (menorIgual(A2.lo[0], A2.lo[1], B.lo[0], B.lo[1]) && menorIgual(B.hi[0], B.hi[1], A2.hi[0], A2.hi[1])) encaixaC++
  }
  for (const { lo, hi } of Ic) if (hi[0] * lo[1] - lo[0] * hi[1] === 1n) comprimentoC++
  console.log(`\n§E4  φ·(1−φ) = ${prod[0]}+${prod[1]}φ · cadeia conjugada encaixa ${encaixaC}/${K - 1}, comprimento-unidade ${comprimentoC}/${K}`)
  ok('§E4 o par tem os dois membros: φ·(1−φ) = −1 em ℤ[φ] — a involução x†=−1/x com x·x†=−1, em pares inteiros', prod[0] === -1n && prod[1] === 0n)
  ok('§E4 a cadeia conjugada encaixa para o OUTRO membro com a mesma unidade — a dualidade guarda a segunda metade', encaixaC === K - 1 && comprimentoC === K)
}

/* §E5 — a mesma lei, a outra membrana: Lucas e o R com d=(−1)^k */
{
  const L = [2n, 1n]
  for (let k = 2; k <= 60; k++) L.push(L[k - 1] + L[k - 2])
  let fecha = 0, gumeD1 = 0
  for (let k = 1; k <= 30; k++) {
    const r = renormaliza({ t: L[k], d: k % 2 === 0 ? 1n : -1n })
    if (r.t === L[2 * k] && r.d === 1n) fecha++
    if (renormaliza({ t: L[k], d: 1n }).t !== L[2 * k]) gumeD1++      /* d fixo em 1: só os pares sobrevivem */
  }
  console.log(`\n§E5  L_{2k} = R(L_k) com d=(−1)^k: ${fecha}/30 · d fixo em 1 falha ${gumeD1}/30 (os 15 ímpares)`)
  ok('§E5 a escada áurea é a MEMBRANA d=−1 da mesma lei: L_{2k} = L_k²−2(−1)^k = R da lib, e d²=1 fecha a membrana', fecha === 30)
  ok('§E5 o gume: fixar d=1 (a membrana do círculo, a de Viviani) falha exatamente nos 15 k ímpares — as membranas não se confundem', gumeD1 === 15)
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  A propriedade dos intervalos encaixantes, derivada: a álgebra')
  console.log('  inteira constrói a cadeia (a unidade ±1 encaixa e mede), a')
  console.log('  varredura mostra o buraco em ℚ, e o contínuo é o que o')
  console.log('  preenche — a álgebra opera, a topologia alcança. E a escada')
  console.log('  áurea é a membrana d=−1 da lei que desenhou Viviani com d=1.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
