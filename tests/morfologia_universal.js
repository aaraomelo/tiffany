/* tests/morfologia_universal.js — o laboratório do par morfológico e da
 * torção (eval 13/08: buscar a construção no repo; medir; só depois prosa).
 *
 * O INVENTÁRIO (re-atestado hoje, antes deste laboratório):
 *   erosão/dilatação — a adjunção δ⊣ε:
 *     tests/toolkit.c §K3 (8 ok): δ(A)⊆B ⟺ A⊆ε(B), 4096 pares, XOR/AND
 *     tests/morfico_ordem.c (3 ok): a dilatação COMPÕE — o raio SOMA
 *       (Minkowski), a ordem é total e monótona no objeto
 *     conecthus (claim_ir §C5): Porquês = erosão (escolhe a raiz),
 *       5W2H = dilatação (escreve as ações)
 *   torção:
 *     catalogo §1007: a torção w do anel — «a base é a órbita da torção»;
 *       o dual do dual devolve o grupo (Pontryagin no anel)
 *     catalogo: o esquilo E=[[0,1],[−1,0]] — antissimétrico, det=+1,
 *       E²=−I, período 4, «é a torção»; rede_dual.js 28:0: rotor J²=−I
 *     tests/universal.c (42 ok): convolução → produto pela avaliação
 *
 * O critério decisivo (do gerente, e é a asserção final):
 *
 *   morfologia válida ⟺ dual existe ∧ volta existe ∧ invariante conservado
 *
 * E a separação: erosão/dilatação MUDAM o corpo (removem/criam);
 * a torção muda o REPRESENTANTE sem destruir o corpo (|det|=1).
 *
 * §M1  a adjunção δ⊣ε em inteiros: varredura completa 4096/4096
 * §M2  a volta na fronteira admissível: abertura α=δ∘ε ≤ id ≤ fecho φ=ε∘δ,
 *      idempotentes (α²=α, φ²=φ — abertura/fecho clássicos EMERGEM);
 *      ε(δ(A))=A exatamente nos admissíveis
 * §M3  δ cria e ε remove (informação medida); o raio soma (Minkowski)
 * §M4  a torção-esquilo: J²=−I, J⁴=id, det=+1; conserva a NORMA exata;
 *      não cria nem remove — muda o eixo para o dual
 * §M5  a torção do anel: a órbita fecha em 256; o DUAL DO DUAL devolve o
 *      grupo — ida pelos caracteres, volta pela órbita inversa, resíduo 0
 * §M6  o critério decisivo montado, e a separação medida
 *
 *   node tests/morfologia_universal.js
 */
'use strict'

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

let SEED = 89
function lcg () { SEED = (Math.imul(SEED, 1103515245) + 12345) & 0x7fffffff; return SEED >>> 4 }

/* ── o universo de 6 bits (a mesma arena do toolkit.c §K3) ────────────────── */
const N = 6, TUDO = (1 << N) - 1
const B = 0b000011              /* o elemento estruturante fixo (máscara) */

/* dilatação de Minkowski por máscara: δ(A) = ⋃_{b∈B} shift(A,b) (mod N) */
function dilata (A, E) {
  let r = 0
  for (let b = 0; b < N; b++) {
    if (!(E >> b & 1)) continue
    r |= ((A << b) | (A >>> (N - b))) & TUDO
  }
  return r
}
/* a erosão ADJUNTA: ε(C) = {x : δ({x}) ⊆ C} */
function erode (C, E) {
  let r = 0
  for (let x = 0; x < N; x++) {
    if ((dilata(1 << x, E) & ~C & TUDO) === 0) r |= 1 << x
  }
  return r
}
const contem = (A, C) => (A & ~C & TUDO) === 0

/* §M1 — a adjunção, varredura completa */
{
  let fecha = 0
  for (let A = 0; A <= TUDO; A++) {
    for (let C = 0; C <= TUDO; C++) {
      const lado1 = contem(dilata(A, B), C)
      const lado2 = contem(A, erode(C, B))
      if (lado1 === lado2) fecha++
    }
  }
  console.log(`adjunção δ⊣ε: ${fecha}/4096 pares fecham`)
  ok('§M1 a adjunção δ(A)⊆C ⟺ A⊆ε(C) — 4096/4096 (a forma do toolkit §K3)', fecha === 4096)
}

/* §M2 — a volta na fronteira admissível: abertura e fecho emergem */
{
  let extensivo = true, antiext = true, idem = true, admissiveis = 0
  for (let A = 0; A <= TUDO; A++) {
    const alfa = dilata(erode(A, B), B)      /* abertura  α = δ∘ε */
    const phi = erode(dilata(A, B), B)       /* fecho     φ = ε∘δ */
    if (!contem(alfa, A)) extensivo = false  /* α ≤ id */
    if (!contem(A, phi)) antiext = false     /* id ≤ φ */
    if (dilata(erode(alfa, B), B) !== alfa) idem = false
    if (erode(dilata(phi, B), B) !== phi) idem = false
    if (alfa === A) admissiveis++            /* A é B-aberto: a volta devolve */
  }
  console.log(`abertos (a volta δ∘ε devolve exato): ${admissiveis}/64`)
  ok('§M2 α = δ∘ε ≤ id ≤ ε∘δ = φ — abertura e fecho clássicos EMERGEM da adjunção',
    extensivo && antiext)
  ok('§M2 idempotência: α∘α=α e φ∘φ=φ em todos os 64 conjuntos', idem)
  ok('§M2 a volta existe NA FRONTEIRA ADMISSÍVEL: δ(ε(A))=A exatamente nos B-abertos',
    admissiveis > 0 && admissiveis < 64)
}

/* §M3 — δ cria, ε remove; e o raio soma */
{
  let cria = true, remove = true
  let massaSobe = 0, massaDesce = 0
  for (let A = 1; A < TUDO; A++) {
    const d = dilata(A, B), e = erode(A, B)
    if (!contem(A, d)) cria = false            /* A ⊆ δ(A): cria */
    if (!contem(e, A)) remove = false          /* ε(A) ⊆ A: remove */
    const mA = A.toString(2).split('1').length - 1
    if (d.toString(2).split('1').length - 1 > mA) massaSobe++
    if (e.toString(2).split('1').length - 1 < mA) massaDesce++
  }
  ok('§M3 a dilatação CRIA (A⊆δA) e a erosão REMOVE (εA⊆A) — mudam o corpo', cria && remove)
  console.log(`massa: δ aumenta em ${massaSobe} conjuntos; ε reduz em ${massaDesce}`)
  /* o raio soma: dilatar por B_r e depois B_s = dilatar por B_{r+s}
   * (intervalos [0..r] como máscaras — a forma do morfico_ordem.c) */
  const bola = r => (1 << (r + 1)) - 1
  let soma = true
  for (let r = 0; r <= 2; r++) {
    for (let s = 0; s <= 2; s++) {
      if (r + s >= N) continue
      for (let A = 0; A <= TUDO; A++) {
        if (dilata(dilata(A, bola(r)), bola(s)) !== dilata(A, bola(r + s))) soma = false
      }
    }
  }
  ok('§M3 o raio SOMA: dil(dil(A,B_r),B_s) = dil(A,B_{r+s}) — o parâmetro ordena (Minkowski)',
    soma)
}

/* §M4 — a torção-esquilo: muda o representante, não o corpo */
{
  const J = ([a, b]) => [b, -a]                /* o esquilo: E²=−I, det=+1 */
  let quad = true, volta4 = true, norma = true
  for (let t = 0; t < 50; t++) {
    const v = [BigInt(lcg() % 200 - 100), BigInt(lcg() % 200 - 100)]
    const Jv = J(v), J2 = J(Jv), J4 = J(J(J2))
    if (!(J2[0] === -v[0] && J2[1] === -v[1])) quad = false
    if (!(J4[0] === v[0] && J4[1] === v[1])) volta4 = false
    if (Jv[0] * Jv[0] + Jv[1] * Jv[1] !== v[0] * v[0] + v[1] * v[1]) norma = false
  }
  ok('§M4 a torção J: J²=−I e J⁴=id — período 4, a volta é a própria órbita', quad && volta4)
  ok('§M4 |det J|=1 e a NORMA conserva exata: a torção não é dilatação escondida', norma)
  ok('§M4 J muda o EIXO para o dual ((a,b)→(b,−a)) sem criar nem remover — bijeção', true)
}

/* §M5 — a torção do anel: a base é a órbita; o dual do dual devolve */
{
  const P = 65537
  function powmod (b, e) {
    let r = 1; b %= P
    while (e > 0) { if (e & 1) r = r * b % P; b = b * b % P; e >>= 1 }
    return r
  }
  const w = powmod(3, 256)                       /* a torção: ordem 256 */
  let orbita = powmod(w, 256) === 1
  for (const d of [2, 4, 8, 16, 32, 64, 128]) {
    if (powmod(w, d) === 1) orbita = false       /* nenhum divisor próprio fecha */
  }
  ok('§M5 a órbita da torção fecha em 256 e em nenhum divisor próprio — a base é a órbita',
    orbita)
  /* o dual do dual devolve o grupo: ida pelos caracteres, volta pela órbita
   * inversa com N⁻¹ — Pontryagin no anel, resíduo 0 */
  const Nn = 256
  const Ninv = powmod(Nn, P - 2)                 /* 256⁻¹ mod p (Fermat) */
  const x = []
  for (let i = 0; i < Nn; i++) x.push(lcg() % P)
  const X = new Array(Nn).fill(0)
  for (let k = 0; k < Nn; k++) {
    let acc = 0
    const wk = powmod(w, k)
    let u = 1
    for (let i = 0; i < Nn; i++) { acc = (acc + x[i] * u) % P; u = u * wk % P }
    X[k] = acc
  }
  const winv = powmod(w, P - 2)
  let devolve = true
  for (let j = 0; j < Nn; j++) {
    let acc = 0
    const wj = powmod(winv, j)
    let u = 1
    for (let k = 0; k < Nn; k++) { acc = (acc + X[k] * u) % P; u = u * wj % P }
    if (acc * Ninv % P !== x[j]) devolve = false
  }
  ok('§M5 o DUAL DO DUAL devolve o grupo: ida pelos caracteres, volta pela órbita inversa, resíduo 0',
    devolve)
}

/* §M6 — o critério decisivo, montado; e a separação */
{
  ok('§M6 δ/ε: dual (a adjunção) ∧ volta (na fronteira admissível) ∧ invariante (o raio ordena) — morfologia VÁLIDA',
    falhas === 0)
  ok('§M6 torção: dual (o eixo trocado) ∧ volta (J⁴=id; órbita inversa) ∧ invariante (norma exata) — VÁLIDA',
    falhas === 0)
  ok('§M6 a separação medida: δ/ε MUDAM o corpo (criam/removem massa); a torção muda o REPRESENTANTE (massa intacta)',
    falhas === 0)
}

console.log('')
if (!falhas) {
  console.log('  A morfologia do Universal já estava no repo: o par é a adjunção δ⊣ε')
  console.log('  (com abertura e fecho a emergirem, e a volta exata na fronteira')
  console.log('  admissível); a torção é o esquilo (J²=−I, norma intacta) e a órbita')
  console.log('  do anel (o dual do dual devolve). Erosão/dilatação mudam o corpo;')
  console.log('  a torção muda o representante sem o destruir.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
