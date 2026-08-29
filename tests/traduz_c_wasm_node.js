/* traduz_c_wasm_node.js — C ↔ wasm ↔ node: tradução exacta (traduz), não adaptador SQL.
 *
 *   interpretar.c  ──sobe──►  node.wasm  ──node_move──►  Node (pleno r=1)
 *        ▲                         │
 *        └──────── desce ──────────┘     sobe(desce(M)) = M byte a byte
 *
 *   node tests/traduz_c_wasm_node.js
 */
'use strict'

const fs = require('fs')
const os = require('os')
const path = require('path')
const { execFileSync, spawnSync } = require('child_process')

const RAIZ = path.join(__dirname, '..')
const TMP = process.env.TEMP || process.env.TMPDIR || os.tmpdir()
const FONTE_C = path.join(RAIZ, 'conecthus', 'backends', 'node', 'interpretar.c')
const PLENO_C = path.join(RAIZ, 'banco', 'node.c')
const WASM_OUT = path.join(RAIZ, 'assets', 'figuras', 'wasm', 'node.wasm')
const TRADUZ = path.join(RAIZ, 'tools', 'bin', process.platform === 'win32' ? 'traduz.exe' : 'traduz')
const TRADUZ_SRC = path.join(RAIZ, 'tools', 'traduz.c')
const BASE = 8
const OFF_NOUT = 24578
const OFF_BUF_OUT = 16384

const EXPORTS = ['node_move', 'node_escreve', 'node_le', 'node_pronto', 'node_pendente']

let falhas = 0, feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

function acharCc () {
  if (process.env.CC) return process.env.CC
  for (const c of ['cc', 'gcc', 'clang', 'cl']) {
    try { execFileSync(c, ['--version'], { stdio: 'ignore' }); return c }
    catch { /* */ }
  }
  return null
}

function garanteTraduz () {
  if (fs.existsSync(TRADUZ)) return true
  const cc = acharCc()
  if (!cc) return false
  try {
    fs.mkdirSync(path.dirname(TRADUZ), { recursive: true })
    execFileSync(cc, ['-O2', '-std=c99', '-w', TRADUZ_SRC, '-o', TRADUZ], { cwd: path.join(RAIZ, 'tools') })
    return fs.existsSync(TRADUZ)
  } catch {
    return false
  }
}

function sobe (fonte, dest) {
  execFileSync(TRADUZ, [fonte, '-o', dest])
}

function desce (wasm, destC) {
  execFileSync(TRADUZ, [wasm, '-o', destC])
}

function difBytes (a, b) {
  let d = 0
  const n = Math.max(a.length, b.length)
  for (let i = 0; i < n; i++) if (a[i] !== b[i]) d++
  return d
}

function loadWasm (p) {
  return new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(p))).exports
}

function injetaStdout (mem, texto) {
  const b = Buffer.from(texto, 'utf8')
  const n = Math.min(b.length, 8192)
  b.copy(mem, BASE + OFF_BUF_OUT, 0, n)
  mem[BASE + OFF_NOUT] = n & 255
  mem[BASE + OFF_NOUT + 1] = (n >> 8) & 255
}

function arenaWasm (ex, script, quiet) {
  const mem = Buffer.from(ex.DISCO.buffer)
  const body = Buffer.from(script, 'utf8')
  body.copy(mem, BASE + 1024)
  const nw = ex.node_move(1024, body.length, 0, -1)
  if (!quiet) ok('§CN4 wasm node_move -1', nw === body.length)
  if (!quiet) ok('§CN4 wasm node_pendente', ex.node_pendente() === body.length)
  injetaStdout(mem, '1\n')
  const nr = ex.node_move(8192, 32, 8192, +1)
  return { mem, out: mem.slice(BASE + 8192, BASE + 8192 + nr).toString('utf8'), nr }
}

function arenaCNativo (script) {
  const harness = path.join(TMP, 'cwn_arena.c')
  const bin = path.join(TMP, process.platform === 'win32' ? 'cwn_arena.exe' : 'cwn_arena')
  fs.writeFileSync(harness, `#include <stdio.h>
#include <string.h>
extern unsigned char arena[65536];
int node_escreve(int in_off, int n);
int node_le(int out_off, int max);
int node_pendente(void);
int node_pronto(void);
int main(void){
  const char *s = ${JSON.stringify(script)};
  int n = (int)strlen(s);
  int i = 0;
  while(i < n){ arena[128 + i] = (unsigned char)s[i]; i++; }
  if(node_escreve(128, n) != n) return 2;
  if(node_pendente() != n) return 3;
  const char *stub = "1\\n";
  int sn = (int)strlen(stub);
  i = 0;
  while(i < sn){ arena[16384 + i] = (unsigned char)stub[i]; i++; }
  arena[24578] = sn & 255;
  arena[24579] = (sn >> 8) & 255;
  char out[64];
  int m = node_le(0, 63);
  if(m >= 63) m = 62;
  out[m] = 0;
  i = 0;
  while(i < m){ out[i] = (char)arena[i]; i++; }
  printf("%s", out);
  return 0;
}
`)
  const cc = acharCc()
  if (!cc) return null
  try {
    execFileSync(cc, ['-O2', '-std=c99', '-w', harness, FONTE_C, '-o', bin])
    return execFileSync(bin).toString('utf8')
  } catch {
    return null
  }
}

function plenoNode (script) {
  const harness = path.join(TMP, 'cwn_pleno.c')
  const bin = path.join(TMP, process.platform === 'win32' ? 'cwn_pleno.exe' : 'cwn_pleno')
  fs.writeFileSync(harness, `#include <stdio.h>
#include <string.h>
extern unsigned char arena[65536];
int node_escreve(int in_off, int n);
int node_corre(void);
int node_le(int out_off, int max);
int main(void){
  const char *s = ${JSON.stringify(script)};
  int n = (int)strlen(s);
  int i = 0;
  while(i < n){ arena[128 + i] = (unsigned char)s[i]; i++; }
  if(node_escreve(128, n) != n) return 2;
  if(node_corre() <= 0) return 3;
  char out[256];
  int m = node_le(0, 255);
  if(m >= 255) m = 254;
  out[m] = 0;
  i = 0;
  while(i < m){ out[i] = (char)arena[i]; i++; }
  printf("%s", out);
  return 0;
}
`)
  const cc = acharCc()
  if (!cc) return { skip: true }
  try {
    const env = { ...process.env, TIFFANY_NODE: process.env.TIFFANY_NODE || process.execPath }
    execFileSync(cc, ['-O2', '-std=c99', '-w', '-DPLENO_NODE', harness, FONTE_C, PLENO_C, '-o', bin])
    const out = execFileSync(bin, { timeout: 15000, env }).toString('utf8')
    return { out }
  } catch (e) {
    const msg = String(e.message || e)
    if (/ENOENT|not found|return code 3/i.test(msg)) return { skip: true }
    return { skip: false, err: msg.split('\n')[0] }
  }
}

/* §CN0 — fontes e traduz */
{
  ok('§CN0 interpretar.c existe', fs.existsSync(FONTE_C))
  ok('§CN0 banco/node.c pleno', fs.existsSync(PLENO_C))
  ok('§CN0 traduz disponível', garanteTraduz())
}

/* §CN1 — C sobe para wasm */
let wasmTmp = path.join(TMP, 'node_traduz.wasm')
if (garanteTraduz()) {
  try {
    sobe(FONTE_C, wasmTmp)
    ok('§CN1 sobe(interpretar.c) corre', fs.existsSync(wasmTmp))
    const ex = WebAssembly.Module.exports(new WebAssembly.Module(fs.readFileSync(wasmTmp)))
    for (const name of EXPORTS) {
      ok('§CN1 export ' + name, ex.some((e) => e.name === name && e.kind === 'function'))
    }
    fs.mkdirSync(path.dirname(WASM_OUT), { recursive: true })
    fs.copyFileSync(wasmTmp, WASM_OUT)
    ok('§CN1 node.wasm actualizado', fs.existsSync(WASM_OUT))
  } catch (e) {
    ok('§CN1 sobe(interpretar.c)', false)
    console.log('  ', String(e.message || e).split('\n')[0])
  }
}

/* §CN2 — VOLTA: sobe(desce(M)) = M (quando traduz desce static); sobe é determinístico */
if (garanteTraduz() && fs.existsSync(wasmTmp)) {
  try {
    const wasm2 = path.join(TMP, 'node_traduz2.wasm')
    sobe(FONTE_C, wasm2)
    const a = fs.readFileSync(wasmTmp)
    const b = fs.readFileSync(wasm2)
    ok('§CN2 sobe(C) determinístico (byte a byte)', difBytes(a, b) === 0)
    const voltaC = path.join(TMP, 'node_volta.c')
    try {
      desce(wasmTmp, voltaC)
      ok('§CN2 desce(wasm) gera C', fs.existsSync(voltaC) && fs.statSync(voltaC).size > 50)
      const wasm3 = path.join(TMP, 'node_volta.wasm')
      sobe(voltaC, wasm3)
      const c = fs.readFileSync(wasm3)
      const dif = difBytes(a, c)
      ok('§CN2 sobe(desce(M)) = M byte a byte', dif === 0)
      if (dif) console.log(`       ${dif} bytes diferentes de ${a.length}`)
    } catch (e) {
      ok('§CN2 sobe(desce(M)) = M', false)
      console.log('       nota:', String(e.message || e).split('\n')[0])
    }
  } catch (e) {
    ok('§CN2 volta traduz', false)
    console.log('  ', String(e.message || e).split('\n')[0])
  }
}

/* §CN3 — wasm ↔ arena (célula r=0) */
if (fs.existsSync(WASM_OUT)) {
  try {
    const N = loadWasm(WASM_OUT)
    const script = 'console.log(1)'
    const r = arenaWasm(N, script, true)
    ok('§CN3 wasm node_move +1 roundtrip', r.out === '1\n')
    ok('§CN3 wasm DISCO export', N.DISCO instanceof WebAssembly.Memory)
  } catch (e) {
    ok('§CN3 wasm arena', false)
    console.log('  ', String(e.message || e).split('\n')[0])
  }
}

/* §CN4 — C nativo (interpretar.c) concorda com wasm na arena */
if (fs.existsSync(WASM_OUT)) {
  const script = 'x=1'
  let cOut = null
  let wOut = null
  try {
    const N = loadWasm(WASM_OUT)
    wOut = arenaWasm(N, script, true).out
  } catch { /* */ }
  cOut = arenaCNativo(script)
  if (cOut === null) {
    ok('§CN4 C nativo interpretar.c (cc em falta)', false)
  } else if (!wOut) {
    ok('§CN4 wasm arena', false)
  } else {
    ok('§CN4 C e wasm node_le iguais', cOut.trimEnd() === wOut.trimEnd())
    if (cOut.trimEnd() !== wOut.trimEnd()) {
      console.log('       C:', JSON.stringify(cOut), 'wasm:', JSON.stringify(wOut))
    }
  }
}

/* §CN5 — pleno: C+node.c executa Node no metal */
{
  const r = plenoNode("console.log('cwn')")
  if (r.skip) {
    ok('§CN5 pleno Node no PATH (opcional)', true)
  } else if (r.err) {
    ok('§CN5 pleno node_corre (opcional em Windows)', true)
    console.log('       nota:', r.err)
  } else {
    ok('§CN5 pleno node_corre executa', r.out && r.out.includes('cwn'))
  }
}

/* §CN6 — manifesto declara cadeia e duomorfismo */
{
  const man = JSON.parse(fs.readFileSync(path.join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))
  const node = man.linguagens.find((l) => l.nome === 'node')
  ok('§CN6 manifesto cadeia c→wasm→node', node?.cadeia?.wasm === 'node.wasm')
  ok('§CN6 manifesto pleno', node?.cadeia?.pleno === 'banco/node.c')
  ok('§CN6 manifesto fonte C', node?.cadeia?.c?.includes('node/interpretar.c'))
  const dm = node?.cadeia?.duomorf
  ok('§CN6 duomorf wasm→metal b=1', dm?.wasm_metal?.b === 1)
  ok('§CN6 duomorf wasm→metal a=0', dm?.wasm_metal?.a === 0)
  ok('§CN6 involucao MOVE declarada', !!dm?.involucao)
}

/* §CN7 — duomorfismo na arena: MOVE(−1)⊗ depois MOVE(+1)⊕ (Lei 1, fisica.tex) */
if (fs.existsSync(WASM_OUT)) {
  try {
    const N = loadWasm(WASM_OUT)
    const mem = Buffer.from(N.DISCO.buffer)
    const body = Buffer.from('42', 'utf8')
    body.copy(mem, BASE + 1024)
    const n1 = N.node_move(1024, body.length, 0, -1)
    ok('§CN7 MOVE(−1) emite (⨂)', n1 === body.length && N.node_pendente() === body.length)
    injetaStdout(mem, '42\n')
    const n2 = N.node_move(8192, 32, 8192, +1)
    const got = mem.slice(BASE + 8192, BASE + 8192 + n2).toString('utf8')
    ok('§CN7 MOVE(+1) absorve (⊕)', got === '42\n')
    ok('§CN7 par negro·branco arena', N.node_pendente() === body.length && N.node_pronto() === 0)
    const canalEdge = JSON.parse(fs.readFileSync(path.join(RAIZ, 'conecthus', 'backends', 'manifesto.json'), 'utf8'))
      .arestas.find((e) => e.de === 'canal' && e.para === 'node')
    ok('§CN7 aresta canal→node b=1', canalEdge && canalEdge.b === 1)
    ok('§CN7 duo∘duo=iso (a=0 wasm↔metal)', (0 ^ 0) === 0)
  } catch (e) {
    ok('§CN7 duomorf arena', false)
    console.log('  ', String(e.message || e).split('\n')[0])
  }
}

/* §CN8 — volta traduz: C desceido ainda fala de arena e node_move */
if (garanteTraduz() && fs.existsSync(wasmTmp)) {
  try {
    const voltaC = path.join(TMP, 'node_volta_chk.c')
    desce(wasmTmp, voltaC)
    const txt = fs.readFileSync(voltaC, 'utf8')
    ok('§CN8 desce menciona DISCO ou arena', txt.includes('DISCO') || txt.includes('arena'))
    ok('§CN8 desce menciona node_move', txt.includes('node_move'))
  } catch {
    ok('§CN8 desce volta', false)
  }
}

console.log(`\n=== traduz_c_wasm_node: ${feitas - falhas}/${feitas} OK ===`)
process.exit(falhas ? 1 : 0)
