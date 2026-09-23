/* tools/prova_minima_quebra_descricao_longa.mjs — PROVA MÍNIMA, 1 vaga, só leitura.
 * Isola a quebra da idempotência CANON₁≡CANON₂ quando `descricao` cresce.
 * Protocolo real (provado em tests/rascunho_vaga_duolingo):
 *   vaga_move(in_off, n, out_off, sentido)
 *     sentido<0 : compila  'k' 'v' 'k' 'v'  →  <S>k</S><S>v</S><S>k</S><S>v</S>
 *     sentido>=0: descompila <S>…</S>…      →  'k' 'v' 'k' 'v'
 *   Cadeia: envelope →(-1)→ CANON₁ →(+1)→ envelope₂ →(-1)→ CANON₂ ; exige byte a byte.
 * SEM correcção — SÓ descobre ONDE quebra e registra. NÃO toca banco/vaga.c, manifesto,
 * assets, SQL, Dual Sort, Max-Cut, ui, Gupy/Lever/WWR, nem as 6 falhas bash/pwsh/node.
 */
import { readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ  = join(dirname(fileURLToPath(import.meta.url)), '..')
const WASM  = join(RAIZ, 'assets', 'figuras', 'wasm', 'vaga.wasm')
const ORG   = 'duolingo'
const BOARD = `https://boards-api.greenhouse.io/v1/boards/${ORG}/jobs?content=true`

const BASE = 8
const A_IN      = BASE + 1024   /* envelope bruto (pares quot)            */
const A_CAN1    = BASE + 8192   /* JSON canónico (ida)                    */
const A_ENV2    = BASE + 16384  /* envelope canónico (volta +1)           */
const A_CAN2    = BASE + 24576  /* JSON canónico 2 (re-ida)               */

const DEC = new TextDecoder('utf-8')
const ENC = new TextEncoder()

function le(ex, off, n)  { return new Uint8Array(ex.DISCO.buffer, off, off + n) }
function poe(ex, arr, off){ new Uint8Array(ex.DISCO.buffer).set(arr, off) }
function txt(ex, off, n) { return DEC.decode(new Uint8Array(ex.DISCO.buffer, off, off + n)) }

function montaEnvelope(pares){
  let s = ''
  for (const [k, v] of pares) s += `'${k.replace(/'/g, '\\\'')}' '${String(v).replace(/'/g, '\\\'')}' `
  return s.trim()
}

async function main(){
  const r = await fetch(BOARD)
  if (!r.ok) { console.error(`✗ HTTP ${r.status}`); process.exit(1) }
  const { jobs } = await r.json()
  const j = jobs.find(x => x && x.id)
  if (!j) { console.error('✗ sem vagas'); process.exit(1) }

  const wasm = readFileSync(WASM)
  const { instance } = await WebAssembly.instantiate(wasm)
  const ex = instance.exports
  if (typeof ex.vaga_move !== 'function') { console.error('✗ vaga_move ausente'); process.exit(1) }

  const base9 = [
    ['fonte', 'duolingo'], ['ext_id', String(j.id)], ['titulo', (j.title || '').trim()],
    ['empresa', 'duolingo'], ['url', (j.absolute_url || '').trim()],
    ['local', ((j.location && j.location.name) || '').trim()],
    ['modalidade', 'remota'], ['senioridade', 'mid'], ['data', (j.updated_at || '').slice(0, 10)],
  ]

  const TAMANHOS = [256, 512, 1024, 2048, 4096, 8192, 16384, 24576]
  console.log('═══ PROVA MÍNIMA — quebra idempotência × `descricao` longa ═══')
  console.log(`  vaga real: [#${j.id}] ${(j.title || '').trim()}`)
  console.log(`  board    : ${BOARD}`)
  console.log(`  campos   : os mesmos 9 do teste que passou + descricao artificial\n`)
  console.log('  '.padEnd(6) + 'len_env'.padEnd(9) + 'len_CAN1'.padEnd(10) + 'len_env2'.padEnd(10) + 'len_CAN2'.padEnd(10) + 'CANON₁≡CANON₂')
  console.log('  ' + '─'.repeat(54))

  for (const tam of TAMANHOS){
    const descricao = 'd'.repeat(tam)
    const pares = [...base9, ['descricao', descricao]]

    const bIn = ENC.encode(montaEnvelope(pares))
    poe(ex, bIn, A_IN)
    const n1 = ex.vaga_move(A_IN, bIn.length, A_CAN1, -1)
    const C1 = txt(ex, A_CAN1, n1)

    const bC1 = ENC.encode(C1)
    poe(ex, bC1, A_CAN1)
    const nEnv2 = ex.vaga_move(A_CAN1, bC1.length, A_ENV2, +1)

    const bEnv2 = le(ex, A_ENV2, nEnv2).slice()
    poe(ex, bEnv2, A_ENV2)
    const n2 = ex.vaga_move(A_ENV2, bEnv2.length, A_CAN2, -1)
    const C2 = txt(ex, A_CAN2, n2)

    const idem = C1 === C2
    console.log(`  ${String(tam).padEnd(6)} ${String(bIn.length).padEnd(9)} ${String(n1).padEnd(10)} ${String(nEnv2).padEnd(10)} ${String(n2).padEnd(10)} ${idem ? 'OK ✓' : '✗ QUEBROU'}`)
    if (!idem) {
      console.log(`\n  → PRIMEIRA QUEBRA em descricao=${tam}B`)
      console.log(`    len(CANON₁)=${n1} · len(CANON₂)=${n2}  (${n1 === n2 ? 'iguais mas bytes ≠' : 'tamanhos ≠'})`)
      let k = 0
      const max = Math.min(n1, n2)
      while (k < max && C1.charCodeAt(k) === C2.charCodeAt(k)) k = k + 1
      console.log(`    primeiros ${k} bytes iguais; divergem no offset ${k}`)
      console.log(`    CANON₁[${k}..${k+24}] = ${JSON.stringify(C1.slice(k, k + 24))}`)
      console.log(`    CANON₂[${k}..${k+24}] = ${JSON.stringify(C2.slice(k, k + 24))}`)
      console.log(`\n  rascunho → dir do teste; NADA mais tocado.`)
      return
    }
  }
  console.log('\n  NENHUMA quebra até 24576B — a dobradiça aguenta este contrato de tamanho.')
}

main().catch(e => { console.error('✗ erro:', e.message); process.exit(1) })
</content>
awaitAutoMode: main()
