/* tests/residuos_totais.js — o vetor de auditoria total no corpus banal
 * (eval 13/08, a ronda final: «meçam o banal»).
 *
 *   R_total = (R_endereço, R_E, R_Φ, R_Φ₂, R_D)
 *
 * RETAIN se e só se TODOS os componentes zeram simultaneamente. Uma
 * transformação admissível conserva o que o corpo é (endereço), a sua massa
 * (E), a sua ordem observável (Φ, Φ₂) e a sua TRANSIÇÃO (D).
 *
 * A leitura de Dirac no texto vem do laboratório (dirac_transicao.js): o
 * operador vive ENTRE os estados. Num corpo textual, os estados são os
 * bytes e as transições são os pares consecutivos (b_t, b_{t+1}) — lidas
 * POR ENDEREÇO. E «metade para cada lado» fecha aqui também: o estado
 * inicial + a sequência de transições determinam o corpo inteiro (a
 * condição de contorno e a derivada).
 *
 * As cinco classes (a tabela do gerente + o caso crítico):
 *   §R1  mesmo corpo                      → (0,0,0,0,0)  RETAIN
 *   §R2  permutação admissível            → (0,0,0,0,0)  RETAIN
 *        (e a membrana NÃO introduz cegueira nova: R_D=0 no admissível)
 *   §R3  alteração de conteúdo            → quebra       REOPEN
 *   §R4  alteração de endereço            → quebra       REOPEN
 *   §R5  O CASO CRÍTICO — o espelhado: E e Φ dizem OK; a CURVATURA (Φ₂)
 *        e a MEMBRANA (R_D) apanham em flagrante           REOPEN
 *
 *   node tests/residuos_totais.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
/* limpeza da dupla árvore (ordem do diretor, 14/08): a forma embutida
 * saiu — a única implementação é a da infraestrutura (lib) */
const { Universal } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')
const U = Universal(sigmaPeano)

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const P = 65537

/* ── o corpus banal real: fala → resposta ─────────────────────────────────── */
const tex = fs.readFileSync(path.join(__dirname, '..', 'papers', 'conversa.tex'), 'utf8')
const pares = []
const rx = /\\section\{([^}]+)\}\s*\n([^\n\\]+)/g
let m
while ((m = rx.exec(tex)) && pares.length < 20) {
  const fala = m[1].trim(), resp = m[2].trim()
  if (fala.length >= 2 && resp.length >= 8) pares.push([fala, resp])
}
console.log(`corpus banal: ${pares.length} pares reais (fala → resposta)`)

/* ── os componentes do vetor ──────────────────────────────────────────────── */
const assinatura = s => U.escada(s)
/* a leitura da membrana: as transições (pares consecutivos) do corpo */
const transicoes = s => U.transicoes(s)

const residuoTotal = (fonte, cand) => U.residuoTotal(fonte, cand)

function mostra (nome, R, veredito) {
  console.log(`#R ${nome.padEnd(38)} R=(${R.Rend},${R.RE},${R.RF1},${R.RF2},${R.RD}) -> ${veredito}`)
}
const retain = U.retain

const fonte = pares.map(([a, s]) => [a, s])

/* §R1 — mesmo corpo */
{
  const R = residuoTotal(fonte, pares.map(([a, s]) => [a, s]))
  mostra('mesmo corpo', R, retain(R) ? 'RETAIN' : 'REOPEN (DEFEITO)')
  ok('§R1 mesmo corpo: R_total = 0 em TODOS os componentes → RETAIN', retain(R))
}

/* §R2 — permutação admissível (a ordem física dos pares) */
{
  const R = residuoTotal(fonte, [...pares].reverse().map(([a, s]) => [a, s]))
  mostra('permutação admissível', R, retain(R) ? 'RETAIN' : 'REOPEN (DEFEITO)')
  ok('§R2 permutação: R_total = 0 → RETAIN (a ordem física não é o corpo)', retain(R))
  ok('§R2 a membrana NÃO introduz cegueira nova: R_D = 0 no admissível', R.RD === 0)
}

/* §R3 — alteração de conteúdo (1 byte numa resposta real) */
{
  const cand = pares.map(([a, s]) => [a, s])
  const alvo = cand[7][1]
  cand[7] = [cand[7][0], alvo.slice(0, 4) +
    String.fromCharCode(alvo.charCodeAt(4) ^ 1) + alvo.slice(5)]
  const R = residuoTotal(fonte, cand)
  mostra('alteração de conteúdo (1 byte)', R, retain(R) ? 'RETAIN (DEFEITO)' : 'REOPEN')
  ok('§R3 conteúdo: quebra em endereço, E, Φ e D → REOPEN',
    R.Rend > 0 && R.RE > 0 && R.RD > 0 && !retain(R))
}

/* §R4 — alteração de endereço (a fala perde o corpo) */
{
  const cand = pares.map(([a, s]) => [a, s])
  cand[3] = ['fala_corrompida_' + cand[3][0], cand[3][1]]
  const R = residuoTotal(fonte, cand)
  mostra('alteração de endereço', R, retain(R) ? 'RETAIN (DEFEITO)' : 'REOPEN')
  ok('§R4 endereço: o par colapsa (faltante+excedente) → REOPEN',
    R.Rend >= 2 && !retain(R))
}

/* §R5 — O CASO CRÍTICO: o espelhado (E e Φ cegos; Φ₂ e D apanham) */
{
  function espelha (l) {
    for (let i = 0; i + 1 < l.length; i++) {
      const a = l[i], b = l[i + 1]
      if (a === b || a === ' ' || b === ' ') continue
      const j = l.indexOf(b + a, i + 2)
      if (j < 0) continue
      const y = l.slice(0, i) + b + a + l.slice(i + 2, j) + a + b + l.slice(j + 2)
      if (y !== l) return y
    }
    return null
  }
  let flagrantes = 0, testados = 0, membranaApanha = 0, curvaturaApanha = 0
  for (let k = 0; k < pares.length; k++) {
    const y = espelha(pares[k][1])
    if (!y) continue
    testados++
    const cand = pares.map(([a, s]) => [a, s])
    cand[k] = [cand[k][0], y]
    const R = residuoTotal(fonte, cand)
    const cego = R.RE === 0 && R.RF1 === 0        /* E e Φ dizem OK */
    if (cego && !retain(R)) flagrantes++
    if (R.RF2 > 0) curvaturaApanha++
    if (R.RD > 0) membranaApanha++
    if (testados <= 3) {
      mostra(`espelhado «${pares[k][1].slice(0, 22)}…»`, R,
        cego ? (retain(R) ? 'PASSOU (DEFEITO)' : 'REOPEN em flagrante') : 'REOPEN')
    }
  }
  console.log(`espelhados testados: ${testados} · flagrantes (E,Φ OK mas REOPEN): ${flagrantes} · Φ₂ apanha: ${curvaturaApanha} · membrana D apanha: ${membranaApanha}`)
  ok('§R5 CASO CRÍTICO: E e Φ dizem OK e o vetor total abre na mesma (flagrante)',
    testados > 0 && flagrantes === testados)
  ok('§R5 a curvatura Φ₂ apanha TODOS os espelhados', curvaturaApanha === testados)
  ok('§R5 a membrana D apanha TODOS os espelhados (dupla vigilância independente)',
    membranaApanha === testados)
}

/* o fecho: estado inicial + transições determinam o corpo (metade para cada lado) */
{
  const s = pares[0][1]
  const b = Buffer.from(s, 'utf8')
  const t = transicoes(s)
  const rec = [b[0]]
  for (const par of t) rec.push(par & 255)
  ok('§R6 metade para cada lado: 1º estado + transições reconstroem o corpo byte a byte',
    Buffer.from(rec).equals(b))
}

console.log('')
if (!falhas) {
  console.log('  O vetor total atravessa a prosa real sem quebrar as réguas:')
  console.log('  RETAIN só quando TUDO zera; o espelhado cai em flagrante pela')
  console.log('  curvatura E pela membrana — duas vigilâncias independentes; e o')
  console.log('  corpo é o 1º estado mais as suas transições: metade para cada lado.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
