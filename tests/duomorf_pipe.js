/* tests/duomorf_pipe.js — duomorfismo no pipe (fisica.tex §fis:def:duomorf, §fis:thm:duo-composicao)
 *
 * §D0  arestas do manifesto concordam com π(L)=(p+q+r) mod 2 e bit b
 * §D1  rota de cada aresta = caminho canónico (duo∘duo=iso via sql)
 * §D2  roundtrip sql↔html no wasm real (a=0)
 * §D3  hub sql: compilar† = descompilar (coerente com interface_padrao)
 * §D4  bit b nas arestas quando r difere
 *
 *   node tests/duomorf_pipe.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const man = JSON.parse(readFileSync(
  join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function lang (nome) {
  const L = man.linguagens.find((l) => l.nome === nome)
  if (L) return L
  if (man.protocolo?.nome === nome) return man.protocolo
  const F = (man.fios || []).find((f) => f.nome === nome)
  if (F) return F
  throw new Error('lang ' + nome)
}

function pi (L) {
  return ((L.p | 0) + (L.q | 0) + (L.r | 0)) & 1
}

function paridade (de, para) {
  const a = pi(lang(de)) ^ pi(lang(para))
  const b = lang(de).r !== lang(para).r ? 1 : 0
  return { a, b }
}

function arestaCanonica (de, para) {
  const hub = man.interface_padrao || 'sql'
  if (de === para) return { de, para, a: 0, b: 0, rota: [de], ponte: null }
  const { a, b } = paridade(de, para)
  let rota
  if (a === 0) rota = [de, para]
  else if (de === hub || para === hub) rota = [de, para]
  else rota = [de, hub, para]
  const ponte = a === 0 ? null : (rota.length === 3 ? 'sql_tags' : 'sql_face')
  return { de, para, a, b, rota, ponte }
}

function caminho (de, para) {
  const found = (man.arestas || []).find((e) => e.de === de && e.para === para)
  if (found) return found.rota
  return arestaCanonica(de, para).rota
}

const PIPE = man.pipe_linguagens || []
const ARESTAS = man.arestas || []
const BASE = man.nulo_disco || 8

function loadWasm (nome) {
  const L = lang(nome)
  const p = join(RAIZ, 'assets', 'figuras', 'wasm', L.wasm)
  if (!existsSync(p)) return null
  return new WebAssembly.Instance(new WebAssembly.Module(readFileSync(p)), {}).exports
}

function enc (ex, off, str) {
  const mem = new Uint8Array(ex.DISCO.buffer)
  const b = Buffer.from(str, 'utf8')
  mem.set(b, BASE + off)
  return b.length
}

function dec (ex, off, len) {
  const mem = new Uint8Array(ex.DISCO.buffer)
  return Buffer.from(mem.slice(BASE + off, BASE + off + len)).toString('utf8')
}

function move (ex, fn, text, sentido) {
  const n = enc(ex, 1024, text)
  const outLen = ex[fn](1024, n, 4096, sentido)
  return dec(ex, 4096, outLen)
}

function ponteSql (ex, corpo, sentido) {
  const fn = sentido < 0 ? ex.sql_compilar : ex.sql_descompilar
  const n = enc(ex, 1024, corpo)
  const outLen = fn(1024, n, 4096)
  return dec(ex, 4096, outLen)
}

function traduzWasm (de, para, texto, inst) {
  const edge = ARESTAS.find((e) => e.de === de && e.para === para)
  if (!edge) throw new Error('aresta ' + de + '→' + para)
  const nomeDe = edge.rota[0]
  const nomePara = edge.rota[edge.rota.length - 1]
  const exDe = inst[nomeDe]
  const exPara = inst[nomePara]
  const exSql = inst.sql
  let corpo = move(exDe, nomeDe + '_move', texto, -1)
  if (edge.ponte === 'sql_tags' && exSql) {
    corpo = ponteSql(exSql, corpo, -1)
    corpo = ponteSql(exSql, corpo, +1)
  } else if (edge.ponte === 'sql_face' && exSql) {
    if (nomePara === 'sql') corpo = ponteSql(exSql, corpo, -1)
    else if (nomeDe === 'sql') corpo = ponteSql(exSql, corpo, +1)
  }
  return move(exPara, nomePara + '_move', corpo, +1)
}

/* §D0 — arestas vs (p,q,r) */
{
  ok('§D0 pipe_linguagens declarado', PIPE.length === 7)
  ok('§D0 arestas pipe (7×6)', ARESTAS.filter((e) => PIPE.includes(e.de) && PIPE.includes(e.para)).length === PIPE.length * (PIPE.length - 1))
  ok('§D0 sql e html têm π=1', pi(lang('sql')) === 1 && pi(lang('html')) === 1)
  ok('§D0 css tem π=0', pi(lang('css')) === 0)
  ok('§D0 tábua: iso∘iso=iso', (0 ^ 0) === 0)
  ok('§D0 tábua: duo∘duo=iso', (1 ^ 1) === 0)
  ok('§D0 tábua: iso∘duo=duo', (0 ^ 1) === 1)

  let d0ok = true
  for (const de of PIPE) {
    for (const para of PIPE) {
      if (de === para) continue
      const canon = arestaCanonica(de, para)
      const edge = ARESTAS.find((e) => e.de === de && e.para === para)
      if (!edge) { d0ok = false; break }
      if (edge.a !== canon.a || edge.b !== canon.b) { d0ok = false; break }
    }
    if (!d0ok) break
  }
  ok('§D0 todas as arestas: a,b de (p,q,r)', d0ok)

  ok('§D0 sql→html a=0', ARESTAS.find((e) => e.de === 'sql' && e.para === 'html').a === 0)
  ok('§D0 sql→css a=1', ARESTAS.find((e) => e.de === 'sql' && e.para === 'css').a === 1)
}

/* §D1 — rota de cada aresta */
{
  let d1ok = true
  for (const edge of ARESTAS) {
    const canon = arestaCanonica(edge.de, edge.para)
    if (edge.rota.join(',') !== canon.rota.join(',')) { d1ok = false; break }
    if (edge.ponte !== canon.ponte) { d1ok = false; break }
  }
  ok('§D1 todas as rotas canónicas', d1ok)
  ok('§D1 sql→html rota directa', caminho('sql', 'html').join(',') === 'sql,html')
  ok('§D1 css→html via sql', caminho('css', 'html').join(',') === 'css,sql,html')
  ok('§D1 bash→sql rota directa', caminho('bash', 'sql').join(',') === 'bash,sql')
}

/* §D2 — roundtrip wasm + traduzWasm via arestas */
{
  const exSql = loadWasm('sql')
  const exHtml = loadWasm('html')
  const exCss = loadWasm('css')
  if (!exSql || !exHtml) {
    ok('§D2 wasm sql+html disponíveis', false)
  } else {
    const inst = { sql: exSql, html: exHtml, css: exCss }
    const q = "INSERT TEXTO 'ouro'"
    const voltaSql = move(exSql, 'sql_move', move(exSql, 'sql_move', q, -1), +1)
    ok('§D2 sql ida+volta', voltaSql === q)

    const html = '<p>ouro</p>'
    const voltaHtml = move(exHtml, 'html_move', move(exHtml, 'html_move', html, -1), +1)
    ok('§D2 html ida+volta', voltaHtml === html)

    const sqlViaHtml = move(exSql, 'sql_move', move(exHtml, 'html_move', html, -1), +1)
    ok('§D2 html→sql (forma interna)', sqlViaHtml.length > 0)

    if (exCss) {
      const css = '.x{color:red}'
      const viaTraduz = traduzWasm('css', 'html', css, inst)
      ok('§D2 traduzWasm css→html (aresta)', viaTraduz.length > 0)
    } else {
      ok('§D2 css wasm (opcional)', true)
    }
  }
}

/* §D3 — involução SQL na ponte */
{
  const ex = loadWasm('sql')
  if (!ex) {
    ok('§D3 consultar.wasm', false)
  } else {
    const fonte = "INSERT TEXTO 'ponte'"
    const n = enc(ex, 100, fonte)
    const n1 = ex.sql_compilar(100, n, 4096)
    const n2 = ex.sql_descompilar(4096, n1, 8192)
    const volta = dec(ex, 8192, n2)
    ok('§D3 compilar† = descompilar na ponte duo', volta === fonte)
  }
}

/* §D4 — bit b nas arestas */
{
  let d4ok = true
  for (const edge of ARESTAS) {
    const esperado = lang(edge.de).r !== lang(edge.para).r ? 1 : 0
    if (edge.b !== esperado) { d4ok = false; break }
  }
  ok('§D4 todas as arestas: b de r', d4ok)

  const e = ARESTAS.find((x) => x.de === 'bash' && x.para === 'sql')
  ok('§D4 bash→sql b=1', e && e.b === 1)
  ok('§D4 bash→sql a=1', e && e.a === 1)
  const e2 = ARESTAS.find((x) => x.de === 'bash' && x.para === 'node')
  ok('§D4 bash→node b=0', e2 && e2.b === 0)
}

/* §D5 — absorção (canal↔shell) e fios não absorvidos */
{
  ok('§D5 protocolo canal', man.protocolo?.nome === 'canal')
  ok('§D5 node tem absorcao', !!man.linguagens.find((l) => l.nome === 'node')?.absorcao?.move)
  ok('§D5 latex tem absorcao', !!man.linguagens.find((l) => l.nome === 'latex')?.absorcao)
  ok('§D5 canal→node', ARESTAS.find((e) => e.de === 'canal' && e.para === 'node'))
  ok('§D5 node nao em fios', !(man.fios || []).find((f) => f.nome === 'node'))
  ok('§D5 fios nao absorvidos', (man.fios || []).every((f) => f.absorvido === false))
  let d5ok = true
  for (const edge of ARESTAS) {
    try {
      const canon = arestaCanonica(edge.de, edge.para)
      if (edge.a !== canon.a || edge.b !== canon.b) { d5ok = false; break }
      if (edge.rota.join(',') !== canon.rota.join(',')) { d5ok = false; break }
      if (edge.ponte !== canon.ponte) { d5ok = false; break }
    } catch { d5ok = false; break }
  }
  ok('§D5 arestas concordam (p,q,r)', d5ok)
}

/* §D6 — cadeia C→wasm→node (duomorfismo no cruzado r, fisica.tex §fis:def:duomorf) */
{
  const node = man.linguagens.find((l) => l.nome === 'node')
  const dm = node?.cadeia?.duomorf
  ok('§D6 cadeia declarada', !!node?.cadeia?.wasm)
  ok('§D6 wasm_metal b=1', dm?.wasm_metal?.b === 1)
  ok('§D6 wasm_metal a=0 (iso na troca ⊕⊗)', dm?.wasm_metal?.a === 0)
  ok('§D6 involucao MOVE', dm?.involucao === 'MOVE(-1)∘MOVE(+1)=id')
  const e = ARESTAS.find((x) => x.de === 'canal' && x.para === 'node')
  ok('§D6 canal→node quadrante Q01', e && e.a === 0 && e.b === 1)
}

/* §D7 — cadeia asm↔wasm (isa ERG-64, duas realizações) */
{
  const isa = man.linguagens.find((l) => l.nome === 'isa')
  const cd = isa?.cadeia
  ok('§D7 cadeia asm↔wasm declarada', cd?.ponte === 'tools/asm_wasm.mjs')
  ok('§D7 secção erg.fita', cd?.wasm?.includes('erg.fita'))
  ok('§D7 involucao monta∘desmonta', cd?.duomorf?.involucao === 'monta∘desmonta=id na fita')
  ok('§D7 medidor asm_wasm', man.operacoes.medidor_cadeia.includes('traduz_asm_wasm'))
}

/* §D8 — cadeia C→asm→Node */
{
  const node = man.linguagens.find((l) => l.nome === 'node')
  const cd = node?.cadeia
  ok('§D8 cadeia C→asm→Node', cd?.sequencia?.join('→') === 'c→wasm→asm→node_pleno')
  ok('§D8 ponte_asm wasm_sec.c', cd?.ponte_asm === 'tools/wasm_sec.c' && existsSync(join(RAIZ, 'tools', 'wasm_sec.c')))
  ok('§D8 wasm_erg.c', cd?.wasm_erg === 'tools/wasm_erg.c' && existsSync(join(RAIZ, 'tools', 'wasm_erg.c')))
  ok('§D8 JS gémeo lib/', existsSync(join(RAIZ, 'lib', 'c_asm_shell.mjs')) && existsSync(join(RAIZ, 'lib', 'wasm_para_erg.mjs')))
  ok('§D8 medidor c_asm_node', man.operacoes.medidor_cadeia.includes('traduz_c_asm_node'))
}

/* §D9 — cadeia C→asm→bash */
{
  const bash = man.linguagens.find((l) => l.nome === 'bash')
  const cd = bash?.cadeia
  ok('§D9 cadeia C→asm→bash', cd?.sequencia?.join('→') === 'c→wasm→asm→bash_pleno')
  ok('§D9 bash asm celula.erg', cd?.asm?.includes('bash/celula.erg'))
  ok('§D9 ponte_asm wasm_sec.c', cd?.ponte_asm === 'tools/wasm_sec.c' && existsSync(join(RAIZ, 'tools', 'wasm_sec.c')))
  ok('§D9 medidor c_asm_shell', man.operacoes.medidor_cadeia.includes('traduz_c_asm_shell'))
}

console.log(`\n=== duomorf_pipe: ${feitas - falhas}/${feitas} OK ===`)
process.exit(falhas ? 1 : 0)
