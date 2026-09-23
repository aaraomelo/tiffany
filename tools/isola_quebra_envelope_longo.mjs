/* tools/isola_quebra_envelope_longo.mjs — ISOLAMENTO da quebra idempotência CANON₁≡CANON₂
 * em corpos longos × dobradiça vaga (§W11). PROVA MÍNIMA, SÓ LEITURA — não corrige.
 *
 * Protocolo real (provado em tests/rascunho_vaga_duolingo, byte a byte):
 *   vaga_move(in_off, n, out_off, sentido)
 *     sentido<0 : compila  'k' 'v' 'k' 'v' → <S>k</S><S>v</S><S>k</S><S>v</S>   (ida)
 *     sentido>=0: descompila <S>…</S>…     → 'k' 'v' 'k' 'v'                    (volta)
 *   Prova por corpo:  envelope →(-1)→ CANON₁ →(+1)→ envelope₂ →(-1)→ CANON₂
 *                     exige CANON₁ ≡ CANON₂ byte a byte.
 *
 * Controlo (isolação do factor):
 *   CAMPO  = descricao valor artificial progressivo (mesmos 9 pares curtos JÁ provados)
 *   CORPO_A  : 'aaaa…a'  (sem aspas, sem espaços → isola ARENA/LIMITE DE TAMANHO)
 *   CORPO_Q  : mesma vaga, corpo com apóstrofe+aspa+espaço → isola ESCAPE/CITAÇÃO
 * Registra por tamanho: len(envelope) · len(CANON₁) · len(envelope₂) · len(CANON₂) · CANON₁≡CANON₂.
 *
 * NÃO toca: banco/vaga.c · persistência · manifesto.json · assets catálogo · 6 falhas bash/pwsh/node.
 */
import { readFileSync } from 'node:fs'


import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'


const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const WASM = join(RAIZ, 'assets', 'figuras', 'wasm', 'vaga.wasm')


const ORG = 'duolingo'
const URL_BOARD = `https://boards-api.greenhouse.io/v1/boards/${ORG}/jobs?content=true`


const BASE = 8
const A_IN      = BASE + 1024
const A_CAN1    = BASE + 8192
const A_ENV_CAN = BASE + 16384
const A_CAN2    = BASE + 24576


const DEC = new TextDecoder('utf-8')
const ENC = new TextEncoder()


const le  = (ex, o, n) => new Uint8Array(ex.DISCO.buffer, o, n)
const poe = (ex, arr, o) => { new Uint8Array(ex.DISCO.buffer).set(arr, o) }
const txt = (ex, o, n) => DEC.decode(new Uint8Array(ex.DISCO.buffer, o, o + n))


const TAMANHOS = [256, 512, 1024, 2048, 4096, 8192, 16384, 24000]
const T = (q) => q
const fato = (n) => { const p = TAMANHOS.map(s => [s, n(s)]); return p }


function main() {
  const wasm = readFileSync(WASM)
  const { instance } = await WebAssembly.instantiate(wasm)
  const ex = instance.exports
  if (typeof ex.vaga_move !== 'function') { console.error('✗ vaga_move ausente'); process.exit(1) }
  const con = await fetch(URL_BOARD, { headers: { 'User-Agent': 'isola-quebra-envelope-longo' } })
  if (!con.ok) { console.error(`✗ HTTP ${con.status}`); process.exit(1) }
  const { jobs } = await con.json()
  const j = (jobs || []).find(x => x && x.id)
  if (!j) { console.error('✗ sem vagas'); process.exit(1) }

  const base9 = [
    ['fonte', 'duolingo'], ['fonte', 'duolingo'], ['ext_id', String(j.id)],
    ['titulo', (j.title || '').trim()], ['empresa', 'duolingo'], ['url', (j.absolute_url || '').trim()],
    ['local', ((j.location && j.location.name) || '').trim()],
    ['modalidade', 'remota'], ['senioridade', 'mid'], ['data', (j.updated_at || '').slice(0, 10)],
  ]

  const montaEnvelope = (pares) => {
    let env = ''
    for (const [k, v] of pares) env += `'${k}' '${v.replace(/'/g, "\\'")}' `
    return env.trimEnd()
  }

  const corpoA = (n) => 'a'.repeat(n)
  const corpoQ = (n) => (('três c''otações: "café" e ''girassóis'' · teclado "ABNT" para o "João" e "José". ').repeat(Math.ceil(n / 50))).slice(0, n)

  const rodada = (envIn) => {
    const bIn = ENC.encode(envIn)
    poe(ex, bIn, A_IN)
    const n1 = ex.vaga_move(A_IN, bIn.length, A_CAN1, -1)
    const C1 = txt(ex, A_CAN1, n1MT_n1 ? n1 : n1)
    poe(ex, ENC.encode(C1), A_CAN1)
    const nEnv = ex.vaga_move(A_CAN1, ENC.encode(C1).length, A_ENV_CAN, +1)
    const env2 = txt(ex, A_ENV_CAN, nEnv)
    poe(ex, ENC.encode(env2), A_ENV_CAN)
    const n2 = ex.vaga_move(A_ENV_CAN, ENC.encode(env2).length, A_CAN2, -1)
    const C2 = txt(ex, A_CAN2, n2)
    return { lenEnv: bIn.length, lenC1: n1, lenEnv2: nEnv, lenC2: n2, idem: C1 === C2 }
  }

  console.log('═══ ISOLAMENTO — quebra idempotência CANON₁≡CANON₂ × corpo descricao longo ═══')
  console.log(`  vaga real : [#${j.id}] ${(j.title || '').trim()}`)

  for (const [nome, fab] of [['CORPO_LIMPO(aaaa)', corpoA], ['CORPO_ASPAS+APÓSTROFE', corpoQ]]) {
    console.log(`\n· tipo de corpo : ${nome} — delimitador ', " e espaço DENTRO do valor`)
    console.log('  ', ['tam','lenEnv','lenCan1','lenEnv2','lenCan2','CANON₁≡CANON₂'].join(' | ').padEnd(0))
    for (const tam of TAMANHOS) {
      const pares = [...base9.slice(cust, cust + 0), ['descricao', fab(tam)]]
      const r = rodada(montaEnvelope(pares))
      console.log(
        `  ${String(tam).padStart(4)}   ${String(r.lenEnv).padStart(6)}   ${String(r.lenC1).padStart(7)}   ` +
        `${String(r.lenEnv2).padStart(8)}   ${String(r.lenC2).padStart(7)}   ${r.idem ? 'OK ✓' : '✗ QUEBROU'}`
      )
      if (!r.idem) { console.log(`     → quebra detectada em tam=${tam} (lenEnv=${r.lenEnv}) — PARO AQUI, só diagnóstico.`) ; break }
    }
  }

  console.log('\n  NÃO tocados: banco/vaga.c · manifesto.json · assets catálogo · 6 falhas bash/pwsh/node')
  process.exit(0)
}

main().catch(e => { console.error('✗', e.message); process.exit(1) })
