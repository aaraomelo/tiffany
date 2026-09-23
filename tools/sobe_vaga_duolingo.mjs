/* tools/sobe_vaga_duolingo.mjs — PROVA ÚNICA ponta a ponta (Greenhouse → vaga.wasm → canónico → volta)
 *   Contrato real (probe atómico + conecthus/backends/vaga/canonizar.c):
 *     vaga_move(in_off, n, out_off, sentido)
 *       sentido < 0 : compila  'k' 'v' 'k' 'v'  →  <S>k</S><S>v</S><S>k</S><S>v</S>   (JSON canónico VAGA)
 *       sentido >= 0: descompila <S>…</S>…      →  'k' 'v' 'k' 'v'                   (envelope canónico)
 *   Cadeia (pedida):  envelope bruto → vaga_move(-1) → JSON canónico → vaga_move(+1) → envelope canónico
 *     prova (pedida): CANON → DESCOMPÕE → CANON = CANON   (byte a byte; perda registada, não falha)
 *   PERSISTÊNCIA: rascunho em tests/rascunho_vaga_duolingo/ — NADA em manifesto.json nem assets/ como catálogo.
 *   FORA: banco/vaga.c, SQL novo, Dual Sort, Max-Cut, UI, Gupy/Lever/WWR, 6 falhas bash/powershell/node.
 */
import { mkdirSync, writeFileSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const OUT_WASM = join(RAIZ, 'assets', 'figuras', 'wasm')
const RASCUNHO = join(RAIZ, 'tests', 'rascunho_vaga_duolingo')

const BASE = 8
const ENV_IN = BASE + 1024   // envelope bruto (quot-pairs) entra aqui
const CAN_OUT = BASE + 8192  // JSON canónico
const ENV_CANON = BASE + 16384 // envelope canónico (vol-ta)
const CAN_OUT2 = BASE + 24576 // JSON canónico 2 (recompilação da volta)

const ORG = 'duolingo'
const URL_BOARD = `https://boards-api.greenhouse.io/v1/boards/${ORG}/jobs?content=true`
const WASM = 'canonizar.wasm'

const enc = new TextEncoder()
const dec = new TextDecoder('utf-8', { fatal: false }).decode.bind(new TextDecoder())

function bytes(ex) { return new Uint8Array(ex.DISCO.buffer) }
function poe(ex, arr, off) { bytes(ex).set(arr, off) }
function pega(ex, off, n) { return new Uint8Array(ex.DISCO.buffer, off, n) }
function txt(ex, off, n) { return dec(new Uint8Array(ex.DISCO.buffer, off, n)) }

/* escapa byte para o formal de pares quot-pairs (aplanação Greenhouse → envelope bruto) */
function fstr(s) { return s.replace(/'/g, '\\\'') }

async function main() {
  mkdirSync(RASCUNHO, { recursive: true })

  /* 1. FONTE — Greenhouse, 1 vaga real (fetch no host node; NUNCA no wasm) */
  const r = await fetch(URL_BOARD, { headers: { 'User-Agent': 'vaga-duolingo-prova', 'Accept': 'application/json' } })
  if (!r.ok) { console.error(`✗ Greenhouse HTTP ${r.status} ${r.statusText}`); process.exit(1) }
  const board = await r.json()
  const jobs = (board.jobs || []).filter(j => j && j.id)
  if (!jobs.length) { console.error('✗ board sem vagas'); process.exit(1) }
  const V = jobs[0]
  console.log(`═══ FONTE Greenhouse ═══`)
  console.log(`  org   : ${ORG}`)
  console.log(`  url   : ${URL_BOARD}`)
  console.log(`  vaga  : [#${V.id}] ${V.title}`)
  console.log(`  local : ${(V.location && V.location.name) || '(sem local)'}`)
  console.log(`  url   : ${V.absolute_url || '(sem url)'}`)
  console.log()

  /* 2. ENVELOPE BRUTO — 'nome' 'valor' … (o que vaga_move(-1) consome) */
  const pares = [
    ['fonte', 'duolingo'],
    ['ext_id', String(V.id)],
    ['titulo', V.title || ''],
    ['url', V.absolute_url || ''],
    ['local', (V.location && V.location.name) || ''],
  ]
  let envIn = ''
  for (const [k, v] of pares) envIn += `'${fstr(k)}' '${fstr(v)}' `
  envIn = envIn.trimEnd()

  /* 3. CARREGA vaga.wasm — protocolo idêntico ao harness §W0/§W11 */
  const buf = readFileSync(join(OUT_WASM, WASM))
  const { instance } = await WebAssembly.instantiate(buf)
  const ex = instance.exports
  if (typeof ex.vaga_move !== 'function') { console.error('✗ vaga_move ausente'); process.exit(1) }

  /* 4. IDA: envelope bruto → JSON canónico */
  const bIn = enc.encode(envIn)
  poe(ex, bIn, ENV_IN)
  const nCan = ex.vaga_move(ENV_IN, bIn.length, CAN_OUT, -1)
  const CANON = txt(ex, CAN_OUT, nCan)
  writeFileSync(join(RASCUNHO, 'envelope_bruto.bin'), bIn)
  writeFileSync(join(RASCUNHO, 'canonico_1.txt'), CANON)

  /* 5. VOLTA: JSON canónico → envelope canónico */
  const bCan = enc.encode(CANON)
  poe(ex, bCan, CAN_OUT)
  const nEnv = ex.vaga_move(CAN_OUT, bCan.length, ENV_CANON, +1)
  const ENV_CAN = txt(ex, ENV_CANON, nEnv)
  writeFileSync(join(RASCUNHO, 'envelope_canonico.bin'), enc.encode(ENV_CAN))

  /* 6. RECOMPILAÇÃO DA VOLTA: envelope canónico → CANON₂ */
  const bEnv = enc.encode(ENV_CAN)
  poe(ex, bEnv, ENV_CANON)
  const nCan2 = ex.vaga_move(ENV_CANON, bEnv.length, CAN_OUT2, -1)
  const CANON2 = txt(ex, CAN_OUT2, nCan2)
  writeFileSync(join(RASCUNHO, 'canonico_2.txt'), CANON2)

  /* 7. PROVA DETERMINÍSTICA — CANON → DESCOMPÕE → CANON = CANON (byte a byte) */
  const idem = CANON === CANON2
  console.log(`═══ CANON → DESCOMPÕE → CANON (byte a byte) ═══`)
  console.log(`  CANON₁ (${CANON.length}B) ≡ CANON₂ (${CANON2.length}B) : ${idem ? 'OK ✓' : '✗ DIFERENTE'}`)

  /* perda registada (não falha): campos/ordem descartados pela canonicidade */
  const perdas = []
  const reTitulo = /titulo/i
  if (!IN_HARM(ENV_CAN, pares, perdas)) {}
  console.log(`  envelope canónico : ${ENV_CAN}`)
  console.log(`  perda registada   : ${perdas.length ? perdas.join('; ') : 'nenhuma — canónico é inclusivo'}`)
  console.log()
  console.log(`═══ VAGA CANÓNICA (JSON canónico VAGA) ═══`)
  console.log(CANON)
  console.log()
  console.log(`═══ BYTES usados na comparação ═══`)
  console.log(`  envelope_bruto.bin  ${bIn.length}B  →  canonico_1.txt ${CANON.length}B`)
  console.log(`  envelope_canonico.bin ${ENV_CAN.length}B  →  canonico_2.txt ${CANON2.length}B`)
  console.log(`  rascunho → ${RASCUNHO}`)
  process.exit(idem ? 0 : 1)
}

function IN_HARM(envCanon, pares, perdas) {
  for (const [k, v] of pares) {
    const pat = `'${k}' '${v}'`
    if (!envCanon.includes(pat)) perdas.push(k)
  }
}

main().catch(e => { console.error('✗ erro na prova:', e.message); process.exit(1) })
