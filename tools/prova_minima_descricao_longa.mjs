/* tools/prova_minima_descricao_longa.mjs — prova mínima da quebra idempotência × descricao longa.
 * Protocolo REAL (provado em sobe_vaga_duolingo.mjs, §W11, bytes idênticos):
 *   vaga_move(in_off, n, out_off, sentido)
 *     -1 : compila  'k' 'v' … 'k' 'v'  →  <S>k</S><S>v</S><S>k</S><S>v</S>
 *     +1 : descompila <S>…</S>         →  'k' 'v' 'k' 'v'
 *   Prova: envelope→(-1)→CANON₁→(+1)→envelope₂→(-1)→CANON₂ ; exige CANON₁≡CANON₂ byte a byte.
 * AQUI: mesma vaga real duolingo, mesmos 8 pares curtos, + descricao artificiais:
 *   · limpo  : 'a'×tam   (sem aspas/espacos — isola tamanho do corpo)
 *   · sujo   : 'b'×N + " com 'aspas' internas ' " (isola escape/citacao no corpo)
 * para tam ∈ {256, 512, 1024, 2048, 4096, 8192, 16384, 24000}.
 * NÃO corrige; NÃO toca banco/vaga.c · manifesto · assets · SQL/DualSort/MaxCut/UI · 6 falhas.
 * Tudo (só leitura + registo de tamanhos) num rascunho de auditoria. */
import { readFileSync, mkdirSync, writeFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const WASM = join(RAIZ, 'assets', 'figuras', 'wasm', 'vaga.wasm')
const RASC = join(RAIZ, 'tests', 'rascunho_prova_minima_descricao_longa')
const ORG = 'duolingo'
const URL_BOARD = `https://boards-api.greenhouse.io/v1/boards/${ORG}/jobs?content=true`

const ENC = new TextEncoder()
const DEC = new TextDecoder('utf-8')
const BASE = 8
const A_IN = BASE + 1024    /* envelope bruto (pares quot) */
const A_C1 = BASE + 8192    /* CANON₁ (ida)               */
const A_E2 = BASE + 16384   /* envelope₂ (volta)          */
const A_C2 = BASE + 24576   /* CANON₂ (re-ida)            */

function u8(ex){ return new Uint8Array(ex.DISCO.buffer) }
function poe(ex, arr, off){ new Uint8Array(ex.DISCO.buffer).set(arr, off) }
function txt(ex, off, n){ return DEC.decode(new Uint8Array(ex.DISCO.buffer, off, off + n)) }

function montaEnvelope(pares){
  let s = ''
  for (const [k, v] of pares) s += `'${k.replace(/'/g, "\\'")}' '${String(v).replace(/'/g, "\\'")}' `
  return s.trim()
}

async function main(){
  mkdirSync(RASC, { recursive: true })
  console.log('═══ PROVA MÍNIMA — quebra CANON₁≡CANON₂ × descricao longa ═══')
  console.log(`  org=${ORG} · fonte=${URL_BOARD}\n`)

  const board = await (await fetch(URL_BOARD, { headers: { 'Accept': 'application/json' } })).json()
  const j = (board.jobs || []).find(x => x && x.id)
  if (!j) { console.error('✗ sem vagas'); process.exit(1) }

  const base9 = [
    ['fonte', 'duolingo'], ['ext_id', String(j.id)], ['titulo', (j.title || '').trim()],
    ['empresa', 'duolingo'], ['url', (j.absolute_url || '').trim()],
    ['local', ((j.location && j.location.name) || '').trim()],
    ['modalidade', 'remota'], ['senioridade', 'senior'],
  ]

  const wasm = readFileSync(WASM)
  const { instance } = await WebAssembly.instantiate(wasm)
  const ex = instance.exports
  if (typeof ex.vaga_move !== 'function') { console.error('✗ vaga_move ausente'); process.exit(1) }

  const tamS = [256, 512, 1024, 2048, 4096, 8192, 16384, 24000]
  const casos = []
  for (const tam of tamS){
    for (const [nome, corpo] of [
      ['limpo',  'a'.repeat(tam)],
      ['sujo',   'b'.repeat(Math.max(0, tam - 60)) + " <b>com 'aspas' \"internas\" e espaços</b> e mais 'still going' "],
    ]){
      const pares = [...base9, ['descricao', corpo]]
      const envIn = montaEnvelope(pares)
      const bIn = ENC.encode(envIn)
      poe(ex, bIn, A_IN)
      const n1 = ex.vaga_move(A_IN, bIn.length, A_C1, -1)
      const C1 = txt(ex, A_C1, n1)
      /* volta */
      const bC1 = ENC.encode(C1)
      poe(ex, bC1, A_C1)
      const nE2 = ex.vaga_move(A_C1, bC1.length, A_E2, +1)
      const E2 = txt(ex, A_E2, nE2)
      /* re-ida */
      const bE2 = ENC.encode(E2)
      poe(ex, bE2, A_E2)
      const n2 = ex.vaga_move(A_E2, bE2.length, A_C2, -1)
      const C2 = txt(ex, A_C2, n2)
      const idem = C1 === C2
      casos.push({ tam, nome, lenIn: bIn.length, lenC1: n1, lenE2: nE2, lenC2: n2, idem, C1, C2 })
    }
  }

  console.log('  tam      tipo   lenEnv   lenCAN₁   lenEnv₂   lenCAN₂   CANON₁≡CANON₂')
  console.log('  ' + '─'.repeat(66))
  let quebra = null
  for (const c of casos){
    const ok = c.idem ? 'OK ✓' : '✗ QUEBROU'
    console.log(`  ${String(c.tam).padEnd(8)} ${c.nome.padEnd(6)} ${String(c.lenIn).padEnd(8)} ${String(c.lenC1).padEnd(9)} ${String(c.lenE2).padEnd(9)} ${String(c.lenC2).padEnd(9)} ${ok}`)
    if (!c.idem && !quebra) quebra = c
  }

  if (quebra){
    console.log(`\n═══ QUEBRA CONFIRMADA — primeiro caso ✗ ═══`)
    console.log(`  tam=${quebra.tam} · tipo=${quebra.nome}`)
    console.log(`  lenEnvelope=${quebra.lenIn} → CANON₁=${quebra.lenC1}B`)
    console.log(`  env₂=${quebra.lenE2}B → CANON₁(${quebra.lenC1}B) ${quebra.idem ? '=' : '≠'} CANON₂(${quebra.lenC2}B)`)
    let d = 0
    const m = Math.min(quebra.C1.length, quebra.C2.length)
    while (d < m && quebra.C1.charCodeAt(d) === quebra.C2.charCodeAt(d)) d = d + 1
    console.log(`  divergem no byte ${d}:`)
    console.log(`    CANON₁[${d}..${d + 24}] = ${JSON.stringify(quebra.C1.slice(d, d + 24))}`)
    console.log(`    CANON₂[${d}..${d + 24}] = ${JSON.stringify(quebra.C2.slice(d, d + 24))}`)
    console.log(`  → causa a inspeccionar: escape/citação de aspas no corpo (hipótese) — ver canonizar.c.`)
  } else {
    console.log('\n  NENHUMA quebra até 24KB — contrato aguenta este corpo.')
  }

  /* rascunho de registo — só tamanhos, nada definitivo */
  writeFileSync(join(RASC, 'tabela_tamanhos.txt'), casos.map(c =>
    `${c.tam} ${c.nome} ${c.lenIn} ${c.lenC1} ${c.lenE2} ${c.lenC2} ${c.idem ? 'OK' : 'QUEBRA'}`).join('\n') + '\n')
  console.log(`\n  registo: ${join(RASC, 'tabela_tamanhos.txt')}`)
  console.log('  NÃO tocados: banco/vaga.c · manifesto.json · assets catálogo · 6 falhas bash/pwsh/node')
  process.exit(0)
}

main().catch(e => { console.error('✗ erro:', e.message); process.exit(1) })
