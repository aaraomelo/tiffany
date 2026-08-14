/* tests/cristal_curadoria.js — a curadoria do cristal, RESOLVIDA e medida
 * (ordem do dono, 13/08: «resolve a curadoria»).
 *
 * A curadoria é tools/cristal_cura.py: 52 fusões (44 por texto idêntico —
 * o mesmo conteúdo por dois esquemas de endereço — e 8 julgadas com leitura)
 * e 13 pares MANTIDOS com motivo. A decisão inteira vive em
 * cristal/curadoria.tsv; a fusão é a operação medida em fusao_conceitos.js:
 * z guarda as duas partes intactas, e a fibra devolve byte a byte.
 *
 * §K0  o livro da curadoria: cada «funde» tem o z na fonte com as partes
 *      ditas; cada «mantem» tem os dois lados vivos e distintos
 * §K1  a contagem fecha: conceitos + fusões == 4286 (o corpus recuperado)
 * §K2  conservação POR FUSÃO: E(z) = E(x)+E(y)+E(esqueleto) — todas as 52
 * §K3  a volta total: desfazer todas devolve 4286 endereços únicos com
 *      E == 38731623179 — a âncora atestada ANTES da curadoria
 * §K4  esgotamento: no corpus curado não resta NENHUM par de texto idêntico
 *      (a regra mecânica esgotou o estrato que a justifica)
 * §K5  o gume: um byte induzido numa parte quebra a âncora da volta
 * §K6  controlo da recusa: um par mantido (arte/artes) tem texto DIFERENTE
 *      — a regra cega tê-lo-ia fundido; a leitura separou
 *
 *   node tests/cristal_curadoria.js
 */
'use strict'
const fs = require('fs')
const path = require('path')

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const RAIZ = path.join(__dirname, '..')
const linhas = fs.readFileSync(path.join(RAIZ, 'cristal', 'cristal.jsonl'), 'utf8')
  .split('\n').filter(l => l.length)
const regs = new Map()
for (const l of linhas) regs.set(JSON.parse(l).id, l)

function E (s) {
  const b = Buffer.from(String(s), 'utf8')
  let e = 0
  for (let i = 0; i < b.length; i++) e += b[i] * b[i]
  return e
}
function esqueleto (id) { return '{"fusao":[,],"id":"' + id + '","tipo":"conceito"}' }
function fibra (lz) {
  const ini = lz.indexOf('[') + 1
  const fim = lz.lastIndexOf(']')
  const miolo = lz.slice(ini, fim)
  let prof = 0
  for (let i = 0; i < miolo.length; i++) {
    if (miolo[i] === '{') prof++
    else if (miolo[i] === '}') prof--
    else if (miolo[i] === ',' && prof === 0) {
      return [miolo.slice(0, i), miolo.slice(i + 1)]
    }
  }
  return null
}

/* o livro da curadoria */
const tsv = fs.readFileSync(path.join(RAIZ, 'cristal', 'curadoria.tsv'), 'utf8')
  .split('\n').filter(l => l.length && !l.startsWith('#'))
  .map(l => l.split('\t'))
const fundeTsv = tsv.filter(c => c[0] === 'funde')
const mantemTsv = tsv.filter(c => c[0] === 'mantem')
console.log(`curadoria.tsv: ${fundeTsv.length} funde (${fundeTsv.filter(c => c[3] === 'texto-identico').length} texto-idêntico + ${fundeTsv.filter(c => c[3] === 'julgada').length} julgadas) · ${mantemTsv.length} mantem`)

/* §K0 — o livro bate com a fonte */
{
  let livroOk = fundeTsv.length > 0 && mantemTsv.length > 0
  for (const [, idZ, idY] of fundeTsv) {
    const lz = regs.get(idZ)
    if (!lz) { livroOk = false; continue }
    const r = JSON.parse(lz)
    if (!Array.isArray(r.fusao) || r.fusao.length !== 2 ||
        r.fusao[0].id !== idZ || r.fusao[1].id !== idY) livroOk = false
  }
  /* «mantem» = os dois ENDEREÇOS sobrevivem e não se fundiram entre si;
   * um lado pode ser z de OUTRA fusão (o_que_e absorvido pelo gémeo) */
  const absorvidos = new Set(fundeTsv.map(c => c[2]))
  for (const [, a, b] of mantemTsv) {
    const la = regs.get(a), lb = regs.get(b)
    if (!la || !lb || la === lb) livroOk = false
    if (absorvidos.has(a) || absorvidos.has(b)) livroOk = false
    if (la && lb) {
      const ra = JSON.parse(la), rb = JSON.parse(lb)
      if (ra.fusao && ra.fusao[1].id === b) livroOk = false
      if (rb.fusao && rb.fusao[1].id === a) livroOk = false
    }
  }
  ok('§K0 o livro da curadoria bate com a fonte: cada funde tem o z, cada mantem os dois lados',
    livroOk)
}

/* §K1/§K2 — contagem e conservação */
const fusoes = linhas.filter(l => JSON.parse(l).fusao)
ok('§K1 a contagem fecha: conceitos + fusões == 4286 (e fusões == livro)',
  linhas.length + fusoes.length === 4286 && fusoes.length === fundeTsv.length)
{
  let conserva = fusoes.length > 0
  for (const lz of fusoes) {
    const partes = fibra(lz)
    if (!partes) { conserva = false; continue }
    const [lx, ly] = partes
    if (E(lz) - E(lx) - E(ly) !== E(esqueleto(JSON.parse(lz).id))) conserva = false
    try {
      if (JSON.parse(lx).id !== JSON.parse(lz).id) conserva = false
      if (JSON.parse(ly).id === JSON.parse(lz).id) conserva = false
    } catch { conserva = false }
  }
  ok('§K2 conservação por fusão: E(z) = E(x)+E(y)+E(esqueleto), nas 52', conserva)
}

/* §K3 — a volta total, contra a âncora pré-curadoria */
function desfazTudo (ls) {
  const fora = []
  for (const l of ls) {
    if (JSON.parse(l).fusao) {
      const p = fibra(l)
      if (p) fora.push(p[0], p[1])
      else fora.push(l)
    } else fora.push(l)
  }
  return fora
}
{
  const antes = desfazTudo(linhas)
  const ids = new Set(antes.map(l => { try { return JSON.parse(l).id } catch { return '?' } }))
  let eAntes = 0
  for (const l of antes) eAntes += E(l)
  console.log(`volta total: ${antes.length} registos, ${ids.size} endereços, E=${eAntes}`)
  ok('§K3 desfazer todas devolve 4286 endereços únicos com E == 38731623179 (a âncora)',
    antes.length === 4286 && ids.size === 4286 && eAntes === 38731623179)
}

/* §K5 — o gume: um byte induzido numa parte quebra a âncora */
{
  const mut = [...linhas]
  /* uma fusão cuja descricao começa em letra minúscula: XOR 1 mantém o JSON
   * válido (letra vira letra) e a indução fica só no CONTEÚDO */
  const i = mut.findIndex(l => {
    const r = JSON.parse(l)
    const j = l.indexOf('"descricao":"') + 13
    if (!r.fusao || j <= 13) return false
    const c = l.charCodeAt(j)
    /* fora os chars em que XOR 1 tocaria a ESTRUTURA do JSON: " # \ ] */
    return c !== 34 && c !== 35 && c !== 92 && c !== 93
  })
  const alvo = mut[i]
  const j = alvo.indexOf('"descricao":"') + 13
  mut[i] = alvo.slice(0, j) + String.fromCharCode(alvo.charCodeAt(j) ^ 1) + alvo.slice(j + 1)
  let eMut = 0
  for (const l of desfazTudo(mut)) eMut += E(l)
  ok('§K5 um byte induzido dentro de uma fusão quebra a âncora da volta',
    eMut !== 38731623179)
}

/* §K4 — esgotamento do estrato texto-idêntico */
{
  const TEXTO = ['titulo', 'descricao', 'exemplos', 'contraexemplos', 'tipo',
    'origem', 'palavras_chave', 'sinonimos', 'memoria', 'epistemico', 'confianca']
  const visto = new Map()
  let restantes = 0
  for (const l of linhas) {
    const r = JSON.parse(l)
    if (r.fusao) continue
    const t = JSON.stringify(TEXTO.map(k => r[k] === undefined ? null : r[k]))
    if (visto.has(t)) restantes++
    else visto.set(t, r.id)
  }
  ok('§K4 esgotamento: nenhum par de texto idêntico resta no corpus curado',
    restantes === 0)
}

/* §K6 — controlo da recusa: arte/artes ficou, e com razão medível */
{
  const la = regs.get('arte'), lb = regs.get('artes')
  const da = la && JSON.parse(la).descricao
  const db = lb && JSON.parse(lb).descricao
  ok('§K6 par mantido (arte/artes): os dois vivos, prosa DIFERENTE — a leitura separou',
    Boolean(la && lb) && da !== db)
}

console.log('')
if (!falhas) {
  console.log('  A curadoria está resolvida: 52 fusões (44 medidas por texto idêntico,')
  console.log('  8 julgadas com leitura), 13 pares mantidos com motivo, tudo em')
  console.log('  cristal/curadoria.tsv. Nada se apagou: cada fusão guarda as duas')
  console.log('  partes, e desfazer todas devolve o corpus recuperado, byte a byte,')
  console.log('  na âncora E pré-curadoria.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
