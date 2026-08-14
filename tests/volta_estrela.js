/* volta_estrela.js — ν∘ν = id NO DOCUMENTO REAL, E MEDIDO POR RESÍDUO E NÃO POR COMPARAÇÃO.
 *
 * O Aarão mandou-me perguntar ao `papers/corpo-estelar.tex` em vez de a ele. A última linha
 * da especificação responde:
 *
 *     medir | resíduo 0, NÃO COMPARAÇÃO | a prova dos nove: resolver e provar
 *
 * e sem margem: «uma asserção que compara contra um valor escrito passa no objecto certo E
 * NO TROCADO — ela verifica a aritmética de quem a escreveu, não o objecto. A REVERSÃO
 * SEPARA-OS.» O `tools/compara.js` é uma comparação contra o pdflatex; isto é o outro
 * caminho, e não precisa de oráculo nenhum.
 *
 * A ESTRELA TEM `MOVE` NOS DOIS SENTIDOS: `-1` emite (compor o PDF), `+1` absorve (lê-lo de
 * volta). O `estrela_emite.c` já media a volta — mas em QUATRO itens de brincar. Aqui é o
 * enredo inteiro.
 *
 * E a involução mede-se no CORPO, não numa roupa: o `.tex` não tem onde guardar `x` e `y`,
 * e escrever o corpo nele deita a posição fora — que é o que o documento chama de APAGAR.
 * Absorve-se o PDF e re-emite-se, nas mesmas posições.
 *
 *   §V1  ν∘ν = id: absorver e re-emitir dá o MESMO corpo, resíduo 0 exacto
 *   §V2  MUTAÇÃO: um glifo trocado dá resíduo 1 — nem 0 nem 2
 *   §V3  CONTROLO: contra outro documento o resíduo é grande, logo a medida distingue
 *   §V4  e a ESTRELA NÃO TEM FILA: a absorção irradia, sem vector nenhum
 *
 * §2 e §3 são o que impede §1 de ser tautologia: absorver e re-emitir com o mesmo código, e
 * depois ler os dois com o mesmo varredor, faria um defeito aparecer dos dois lados e
 * cancelar. A mutação e o controlo separam-nos.
 *
 *   node tests/volta_estrela.js
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TEX = path.join(__dirname, 'tex')
const TMP = '/tmp/volta_estrela'

let feitas = 0, falhas = 0
function ok (q, cond) {
  feitas++; if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
  console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`)
}
const sh = (c, o) => { try { return execSync(c, Object.assign({ encoding: 'utf8', stdio: 'pipe' }, o || {})) } catch (e) { return String((e.stdout || '') + (e.stderr || '')) } }
const resid = (a, b) => {
  const t = sh(`${JSON.stringify(TEX)} -residuo ${JSON.stringify(a)} ${JSON.stringify(b)}`, { cwd: __dirname, timeout: 900000 })
  const m = /RESIDUO:\s*(\d+)/.exec(t)
  const c = /nos (\d+) comuns/.exec(t)
  return { r: m ? +m[1] : null, comuns: c ? +c[1] : 0 }
}

fs.mkdirSync(TMP, { recursive: true })
console.log('=== ν∘ν = id NO DOCUMENTO REAL ===========================================\n')

const A = path.join(TMP, 'A.pdf'), A2 = path.join(TMP, 'A2.pdf')
sh(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(RAIZ, 'enredo.tex'))} ${JSON.stringify(A)}`,
   { cwd: __dirname, timeout: 900000 })
const saida = sh(`${JSON.stringify(TEX)} -refaz ${JSON.stringify(A)} ${JSON.stringify(A2)}`,
                 { cwd: __dirname, timeout: 900000 })
const npost = +(/(\d+) postos re-emitidos/.exec(saida) || [0, 0])[1]
const ntd = +(/(\d+) Td nao lidos/.exec(saida) || [0, -1])[1]

/* ─── §V1 a involução ──────────────────────────────────────────────────────────────── */
const v1 = resid(A, A2)
console.log(`§V1  ${npost} postos absorvidos e re-emitidos, ${ntd} Td não lidos`)
console.log(`     resíduo da volta: ${v1.r}   (${v1.comuns} postos comuns)`)
/* o `> 100000` não é um limiar de qualidade — é a guarda contra o resíduo 0 de um par vazio,
 * que é a forma mais barata de esta asserção passar sem medir nada */
ok('ν∘ν = id: o corpo re-emitido é o mesmo, resíduo 0 EXACTO', v1.r === 0 && v1.comuns > 100000)
ok('e nenhum Td ficou por ler — um sscanf que falha em silêncio congela o y', ntd === 0)

/* ─── §V2 a mutação ────────────────────────────────────────────────────────────────── */
/* troca-se UM glifo. Se o resíduo continuar 0, o §V1 não estava a medir nada; se saltar para
 * um número grande, o varredor não está alinhado e o 0 era sorte. Tem de dar exactamente 1. */
const A3 = path.join(TMP, 'A3.pdf')
const cru = fs.readFileSync(A2)
/* auditoria 14/08: no dialecto não há bytes 'Reino Dourado' — o texto são
 * glifos /Gf_c Do. Troca-se UM: /G0_82 ('R') vira /G0_88 ('X'), mesmo posto. */
const alvo = cru.indexOf(Buffer.from('/G0_82 Do'))
if (alvo > 0) {
  const mut = Buffer.from(cru)
  mut[alvo + 5] = '8'.charCodeAt(0)          /* /G0_82 → /G0_88 */
  fs.writeFileSync(A3, mut)
  const v2 = resid(A, A3)
  console.log(`\n§V2  MUTAÇÃO: um glifo trocado (R→X) no posto da capa`)
  console.log(`     resíduo: ${v2.r}`)
  ok('um glifo trocado dá resíduo EXACTAMENTE 1 — nem 0 nem um salto', v2.r === 1)
} else {
  ok('um glifo trocado dá resíduo EXACTAMENTE 1 — nem 0 nem um salto', false)
}

/* ─── §V3 o controlo ───────────────────────────────────────────────────────────────── */
const T = path.join(TMP, 'T.pdf')
sh(`${JSON.stringify(TEX)} ${JSON.stringify(path.join(RAIZ, 'teoria.tex'))} ${JSON.stringify(T)}`,
   { cwd: __dirname, timeout: 900000 })
const v3 = resid(A, T)
console.log(`\n§V3  CONTROLO: o enredo contra a teoria`)
console.log(`     resíduo: ${v3.r}`)
ok('contra outro documento o resíduo é grande — a medida distingue', v3.r > 100000)

/* ─── §V4 sem fila ─────────────────────────────────────────────────────────────────── */
/* «E UMA ESTRELA NÃO TEM FILA. Não tem buffer, não tem memória, não guarda para depois:
 * IRRADIA.» A primeira versão desta absorção tinha `Posto v[4000000]` — 366 MB, e com tecto,
 * que é o que uma fila sempre tem. Aqui não há vector nenhum na varredura. */
const fonte = fs.readFileSync(path.join(__dirname, 'tex.c'), 'utf8')
/* auditoria 14/08: a âncora antiga («emitir `.tex`») saiu do tex.c e o
 * corte apanhava o ficheiro inteiro — o fim é o fim REAL da função: a
 * próxima declaração static */
const iniV = fonte.indexOf('static long varre_postos')
const restoV = fonte.slice(iniV + 10)
const fimV = restoV.search(/\nstatic /)
const varre = restoV.slice(0, fimV < 0 ? undefined : fimV)
const vetores = [...varre.matchAll(/\[\s*(\d{4,})\s*\]/g)].map((m) => +m[1])
console.log(`\n§V4  vectores dentro da varredura: ${vetores.length ? vetores.join(', ') : 'nenhum'}`)
ok('a absorção irradia: nenhum vector com tecto dentro da varredura', vetores.length === 0)

console.log(`\n${'='.repeat(74)}`)
console.log(`  ${feitas - falhas}/${feitas} unidades`)
console.log('')
console.log('  A reversão não precisa de oráculo. Comparar contra o pdflatex verifica a')
console.log('  aritmética do pdflatex; reverter verifica o objecto.')
console.log('')
process.exit(falhas ? 1 : 0)
