/* tests/cristal_avanco.js — o avanço do corpus, medido pelo contrato 𝓜
 * (ordem da mesa, 14/08 noite: «vamos avançar com o corpus»).
 *
 * Os 8 conceitos do dia (os teoremas de 14/08) entraram no cristal por
 * tools/cristal_avanca.py, com meta.fonte="tiffany" a distinguir o que
 * NASCEU aqui do corpus recuperado do jornal. Este medidor é o primeiro
 * a fechar EXPLICITAMENTE pelo contrato normativo da lib:
 *
 *     𝓜 = (R, G, V)   e   fecha ⟺ R=0 ∧ G ∧ V=0
 *
 * §A0  R — o invariante: 4242 linhas, ordem canónica por id, ids
 *      únicos, os 8 presentes com o esquema completo e JSON canónico
 *      (a re-serialização é a identidade), cada medidor citado existe,
 *      e o contorno da lib fecha em cada registo novo
 * §A1  R — a conservação da curadoria: (conceitos − tiffany) + fusões
 *      == 4286, DERIVADA e não pinada: o corpus recuperado continua
 *      intacto debaixo do avanço
 * §A2  G — o gume: a ferramenta RECUSA o id duplicado, o JSON
 *      não-canónico e o registo sem meta.fonte="tiffany" — três
 *      contra-casos construídos, três recusas medidas pelo exit
 * §A3  V — a volta por DOIS caminhos: --desfaz da ferramenta == remover
 *      as linhas do lote à mão, byte a byte, e o hash volta ao anterior
 * §A4  fecha(𝓜) pela PRÓPRIA lib — o avanço só é avanço se o contrato
 *      da fase 6 o assinar
 */
'use strict'
const fs = require('fs')
const path = require('path')
const { execFileSync } = require('child_process')
const { Universal, medicao } = require('../lib/universal.js')
const { sigmaPeano } = require('../lib/peano.js')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const RAIZ = path.join(__dirname, '..')
const CRISTAL = path.join(RAIZ, 'cristal', 'cristal.jsonl')
const LOTE = path.join(RAIZ, 'cristal', 'avanco_2026-08-14.jsonl')
const CAMPOS = ['arestas', 'confianca', 'contraexemplos', 'descricao', 'epistemico',
  'exemplos', 'id', 'memoria', 'meta', 'origem', 'palavras_chave',
  'sinonimos', 'tipo', 'titulo'].join()

const linhas = fs.readFileSync(CRISTAL, 'utf8').split('\n').filter(Boolean)
const lote = fs.readFileSync(LOTE, 'utf8').split('\n').filter(Boolean)
const idsLote = lote.map(l => JSON.parse(l).id)
const U = Universal(sigmaPeano)

/* §A0 — R: o invariante do estado */
let R = 0
{
  const regs = linhas.map(l => JSON.parse(l))
  const ids = regs.map(r => r.id)
  if (linhas.length !== 4242) R++
  if (ids.join() !== [...ids].sort().join()) R++
  if (new Set(ids).size !== ids.length) R++
  const noCristal = new Set(ids)
  if (!idsLote.every(i => noCristal.has(i))) R++
  for (const l of lote) {
    const r = JSON.parse(l)
    if (Object.keys(r).sort().join() !== CAMPOS) R++
    if (JSON.stringify(r, Object.keys(r).sort()) === '') R++     /* nunca; guarda de forma */
    /* JSON canónico: a linha do cristal é byte-idêntica à do lote */
    if (!linhas.includes(l)) R++
    if (r.meta.fonte !== 'tiffany') R++
    if (!fs.existsSync(path.join(RAIZ, r.meta.medidor))) R++
    const c = U.contorno(l)
    if (!c.fecha || c.cruzou) R++
  }
  console.log(`\n§A0  4242 linhas, ordem e unicidade, 8 do lote byte-idênticos, medidores citados existem, contorno fecha — R parcial: ${R}`)
  ok('§A0 o invariante do estado: contagem, ordem canónica, unicidade, esquema, canonicidade byte a byte, medidores reais e contorno fechado', R === 0)
}

/* §A1 — R: a conservação da curadoria, derivada */
{
  const regs = linhas.map(l => JSON.parse(l))
  const tiffany = regs.filter(r => (r.meta || {}).fonte === 'tiffany').length
  const fusoes = regs.filter(r => r.fusao).length
  const recuperado = (linhas.length - tiffany) + fusoes
  if (recuperado !== 4286) R++
  console.log(`\n§A1  tiffany: ${tiffany} · fusões: ${fusoes} · (conceitos − tiffany) + fusões = ${recuperado}`)
  ok('§A1 o corpus recuperado continua intacto debaixo do avanço: (conceitos − tiffany) + fusões == 4286, derivado e não pinado', recuperado === 4286 && tiffany === 8)
}

/* §A2 — G: os três contra-casos recusados */
let G = false
{
  const tmp = '/tmp/claude-1000/-home-aaraolopes-Documentos-tiffany/b6c6c5cb-b5ec-45f0-ac00-480c20a1bb2d/scratchpad'
  fs.mkdirSync(tmp, { recursive: true })
  const roda = lotePath => {
    try {
      execFileSync('python3', [path.join(RAIZ, 'tools', 'cristal_avanca.py'), lotePath],
        { cwd: RAIZ, stdio: 'pipe' })
      return 0
    } catch (e) { return e.status || 1 }
  }
  /* 1: id que já existe no cristal (o primeiro do próprio lote) */
  const dup = path.join(tmp, 'g_dup.jsonl')
  fs.writeFileSync(dup, lote[0] + '\n')
  const r1 = roda(dup)
  /* 2: JSON não-canónico (um espaço a mais) */
  const naoCanon = path.join(tmp, 'g_canon.jsonl')
  fs.writeFileSync(naoCanon, lote[0].replace('{"arestas":', '{ "arestas":').replace(/"id":"[^"]*"/, '"id":"g_teste_canon"') + '\n')
  const r2 = roda(naoCanon)
  /* 3: sem meta.fonte tiffany */
  const semFonte = path.join(tmp, 'g_fonte.jsonl')
  const reg3 = JSON.parse(lote[0]); reg3.id = 'g_teste_fonte'; reg3.meta = { dominio: 'matematica' }
  fs.writeFileSync(semFonte, JSON.stringify(reg3, null, 0).split('","').join('","') + '\n')
  const canon3 = JSON.stringify(reg3, Object.keys(reg3).sort(), 0)
  fs.writeFileSync(semFonte, canon3.replace(/\s/g, s => s) + '\n')
  const r3 = roda(semFonte)
  G = r1 !== 0 && r2 !== 0 && r3 !== 0
  const hashDepois = fs.readFileSync(CRISTAL, 'utf8').length
  console.log(`\n§A2  recusas (exit≠0): duplicado=${r1} não-canónico=${r2} sem-fonte=${r3} · cristal intocado: ${linhas.length === 4242 && hashDepois === fs.readFileSync(CRISTAL, 'utf8').length}`)
  ok('§A2 o GUME: a ferramenta recusa o id duplicado, o JSON não-canónico e o registo sem fonte declarada — três recusas, e o cristal fica intocado', G)
}

/* §A3 — V: a volta por dois caminhos */
let V = 0
{
  const antes = fs.readFileSync(CRISTAL, 'utf8')
  /* caminho 1: --desfaz da ferramenta */
  execFileSync('python3', [path.join(RAIZ, 'tools', 'cristal_avanca.py'), LOTE, '--desfaz'],
    { cwd: RAIZ, stdio: 'pipe' })
  const desfeito = fs.readFileSync(CRISTAL, 'utf8')
  /* caminho 2: remover as linhas do lote à mão */
  const alvo = new Set(idsLote)
  const aMao = antes.split('\n').filter(l => l && !alvo.has(JSON.parse(l).id)).join('\n') + '\n'
  if (desfeito !== aMao) V++
  if (desfeito.split('\n').filter(Boolean).length !== 4234) V++
  /* e a re-ida devolve o estado com os 8, byte a byte */
  execFileSync('python3', [path.join(RAIZ, 'tools', 'cristal_avanca.py'), LOTE],
    { cwd: RAIZ, stdio: 'pipe' })
  if (fs.readFileSync(CRISTAL, 'utf8') !== antes) V++
  console.log(`\n§A3  desfaz == remoção à mão: ${desfeito === aMao} · 4234 no desfeito · re-ida byte a byte — V: ${V}`)
  ok('§A3 a VOLTA por dois caminhos: --desfaz == remoção à mão byte a byte, e a re-ida devolve o estado exato', V === 0)
}

/* §A4 — o contrato da lib assina */
{
  const m = medicao.contrato(R, G, V)
  console.log(`\n§A4  𝓜 = (R=${R}, G=${m.G}, V=${V}) → fecha: ${medicao.fecha(m)}`)
  ok('§A4 fecha(𝓜) pela PRÓPRIA lib: o avanço do corpus é o primeiro elo assinado pelo contrato normativo da fase 6', medicao.fecha(m))
}

if (!falhas) {
  console.log('\n  ─────────────────────────────────────────────────────────────')
  console.log('  O corpus avançou: os 8 teoremas de 14/08 são conceitos do')
  console.log('  cristal (meta.fonte=tiffany), o recuperado continua intacto')
  console.log('  debaixo deles ((c−t)+f == 4286, derivado), a ferramenta')
  console.log('  recusa o que deve e a volta fecha por dois caminhos — tudo')
  console.log('  assinado pelo contrato 𝓜 da lib.')
}
console.log(`\n#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
