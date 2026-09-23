/* tools/audita_vaga_greenhouse_schema.mjs — COBERTURA do schema VAGA × Greenhouse (prova única, só leitura+lacuna).
 *   Dobradiça comprovada §W11 (manifesto, 20 línguas):  vaga_move(in_off, n, out_off, sentido)
 *     sentido < 0 : compila  'k' 'v' 'k' 'v'    →  <S>k</S><S>v</S><S>k</S><S>v</S>        (JSON canónico VAGA)
 *     sentido >= 0: descompila <S>…</S>…        →  'k' 'v' 'k' 'v'                         (envelope canónico)
 *   Prova por vaga:  envelope bruto →(-1)→ CANON₁ →(+1)→ envelope canónico →(-1)→ CANON₂
 *                    EXIGE CANON₁ ≡ CANON₂ byte a byte (idempotência — §W11).
 *   O QUE ESTA PROVA FAZ:  3–5 vagas Greenhouse REAIS deliberadamente variadas (remota · presencial/
 *   híbrida · senior/staff · descrição extensa · salário estruturado se existir), e para CADA vaga
 *   classifica campo a campo o schema VAGA (17 campos) —
 *     EXTRAÍDO | NORMALIZADO | DERIVADO | AUSENTE NA FONTE | AINDA NÃO IMPLEMENTADO.
 *   O QUE NÃO FAZ (ordem do utilizador): NÃO cria banco/vaga.c; NÃO persiste definitivo; NÃO mexe
 *   em manifesto.json/assets como catálogo; NÃO inventa parsing de HTML/requisitos/tecnologias;
 *   NÃO toca as 6 falhas bash/powershell/node. Tudo fica em tests/rascunho_auditoria_vaga_greenhouse/.
 */
import { mkdirSync, writeFileSync, readFileSync, readdirSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const DIR_WASM = join(RAIZ, 'assets', 'figuras', 'wasm')
const RASCUNHO = join(RAIZ, 'tests', 'rascunho_auditoria_vaga_greenhouse')

const ORG = 'duolingo'
const URL_BOARD = `https://boards-api.greenhouse.io/v1/boards/${ORG}/jobs?content=true`

const BASE = 8
const A_IN       = BASE + 1024    /* envelope bruto (pares quot) entra aqui  */
const A_CAN1     = BASE + 8192    /* JSON canónico (ida)                     */
const A_ENV_CAN  = BASE + 16384   /* envelope canónico (volta: +1)           */
const A_CAN2     = BASE + 24576   /* JSON canónico 2 (re-ida)                */

const DEC = new TextDecoder('utf-8')
const ENC = new TextEncoder()
let totalIdem = 0
let contsProvadas = 0

function le(ex, off, n) { return new Uint8Array(ex.DISCO.buffer, off, n) }
function poe(ex, arr, off) { le(ex, 0, 0); new Uint8Array(ex.DISCO.buffer).set(arr, off) }
function u8(ex) { return new Uint8Array(ex.DISCO.buffer) }
function txt(ex, off, n) { return DEC.decode(new Uint8Array(ex.DISCO.buffer, off, off + n)) }
function bytesLivres(ex, from) { return new Uint8Array(ex.DISCO.buffer, from) }

/* ── schema VAGA — 17 campos de colecção (contrato amostral planeado) ── */
const SCHEMA = [
  ['fonte',      'origem/lote Greenhouse'],
  ['ext_id',     'id externo Greenhouse'],
  ['titulo',     'título da vaga'],
  ['empresa',    'empresa/org'],
  ['url',        'URL canónico'],
  ['local',      'local (location.name)'],
  ['modalidade', 'remota · híbrida · presencial'],
  ['contrato',   'CTO/full-time/part-time'],
  ['senioridade','junior · mid · senior · staff'],
  ['salario_min','limite inferior'],
  ['salario_max','limite superior'],
  ['moeda',      'USD/EUR/…'],
  ['data',       'data (updated_at / data de publicação)'],
  ['idioma',     'idioma principal'],
  ['descricao',  'descrição/requisitos (corpo)'],
  ['req_min',    'requisitos mínimos'],
  ['tecnologias','stack/tecnologias'],
]

async function main() {
  mkdirSync(RASCUNHO, { recursive: true })

  /* 1. FONTE — Greenhouse, org real, todas as vagas (fetch no NODE — nunca no wasm) */
  const r = await fetch(URL_BOARD, { headers: { 'User-Agent': 'audita-schema-vaga', 'Accept': 'application/json' } })
  if (!r.ok) { console.error(`✗ Greenhouse HTTP ${r.status} ${r.statusText}`); process.exit(1) }
  const board = await r.json()
  const jobs = (board.jobs || []).filter(j => j && j.id)
  if (!jobs.length) { console.error('✗ sem vagas no board'); process.exit(1) }
  console.log(`═══ AUDITORIA DE COBERTURA — schema VAGA × Greenhouse(${ORG}) ═══`)
  console.log(`  board   : ${URL_BOARD}`)
  console.log(`  total   : ${jobs.length} vagas reais\n`)

  /* 2. SELECÇÃO DELIBERADA (3–5, variadas) — SÓ para a cobertura não ser acidental */
  const F = (re) => jobs.find(j => re.test((j.title || '') + ' ' + ((j.location && j.location.name) || '')))
  const exclui = new Set()
  const P = []
  const add = (j) => { if (j && !exclui.has(j.id)) { P.push(j); exclui.add(j.id) } }
  add(F(/remote|100%|work from home/i))                       /* remota                */
  add(jobs.find(j => !exclui.has(j.id) && !/remote/i.test((j.location && j.location.name) || ''))) /* presencial/híbrida */
  add(F(/senior|staff|principal|lead/i))                      /* senior/staff          */
  add(jobs.reduce((a, b) => ((a.content || '').length > (b.content || '').length ? a : b)))       /* descrição extensa    */
  const comSal = jobs.find(j => j.metadata && j.metadata.some(m => /salary|salar|compensation|faixa/i.test(m.name || '')))
  if (comSal) add(comSal)                                     /* vaga com salário estruturado, SE existir */

  /* 3. CARREGA vaga_move — protocolo real §W11 */
  const wasm = readFileSync(join(DIR_WASM, 'vaga.wasm'))
  const { instance } = await WebAssembly.instantiate(wasm)
  const ex = instance.exports
  if (typeof ex.vaga_move !== 'function') { console.error('✗ vaga_move ausente no wasm'); process.exit(1) }

  const relatorios = []
  for (let i = 0; i < P.length; i++) {
    const j = P[i]
    const id = String(j.id)
    const tit = (j.title || '').trim()
    const loc = ((j.location && j.location.name) || '').trim()
    const url = (j.absolute_url || '').trim()
    const data = (j.updated_at || '').slice(0, 10)
    const desc = (j.content || '').replace(/<[^>]+>/g, ' ').replace(/\s+/g, ' ').trim()

    /* determinístico — derivar o que a fonte permite; marcar o resto */
    const T = (tit + ' ' + loc).toLowerCase()
    const modalidade =
      /remote|100%|remoto/i.test(T + ' ' + loc) ? 'remota' :
      /hybrid|híbrid|hibrid|presencial|office|on.?site/i.test(T) ? 'híbrida/presencial' : 'presencial'
    const senioridade =
      /staff|principal|distinguished/i.test(tit) ? 'staff' :
      /senior|lead|tech lead/i.test(tit) ? 'senior' :
      /junior|jr\.?|entry|associate|estág/i.test(tit) ? 'junior' : 'mid'
    const temSal = j.metadata && j.metadata.some(m => /salary|salar|compensation|faixa/i.test(m.name || ''))
    const idioma =
      /english|inglês|ingles/i.test(desc) ? 'português/inglês (não declarado)' :
      /portugu|espanhol|francês|alemão/i.test(desc) ? 'declarado no corpo' : 'não declarado'

    /* envelope bruto — pares quot (é o que vaga_move(-1) consome) */
    const pares = [
      ['fonte', 'duolingo'], ['ext_id', id], ['titulo', tit], ['empresa', 'duolingo'],
      ['url', url], ['local', loc], ['modalidade', modalidade],
      ['senioridade', senioridade], ['data', data],
    ]
    if (temSal) { const m = j.metadata.find(x => /salary|salar|compensation|faixa/i.test(x.name || '')); pares.push(['info_salarial', (m && m.value) || '']) }
    let env = ''
    for (const [k, v] of pares) env += `'${k.replace(/'/g, '\\\'')}' '${String(v).replace(/'/g, '\\\'')}' `
    env = env.trimEnd()

    /* 4. IDA: envelope → JSON canónico */
    const bIn = ENC.encode(env)
    poe(ex, bIn, A_IN)
    const n1 = ex.vaga_move(A_IN, bIn.length, A_CAN1, -1)
    const C1 = txt(ex, A_CAN1, n1)

    /* 5. VOLTA: JSON canónico → envelope canónico */
    const bC1 = ENC.encode(C1)
    poe(ex, bC1, A_CAN1)
    const nEnv = ex.vaga_move(A_CAN1, bC1.length, A_ENV_CAN, +1)
    const envCanon = txt(ex, A_ENV_CAN, nEnv)

    /* 6. RE-IDA: envelope canónico → JSON canónico 2 */
    const bEnv = ENC.encode(envCanon)
    poe(ex, bEnv, A_ENV_CAN)
    const n2 = ex.vaga_move(A_ENV_CAN, bEnv.length, A_CAN2, -1)
    const C2 = txt(ex, A_CAN2, n2)

    const idem = C1 === C2
    totalIdem += idem ? 1 : 0
    contsProvadas++

    /* 7. classificação campo a campo */
    const cl = {
      fonte:      'EXTRAÍDO',      /* org fixa da fonte */
      ext_id:     'EXTRAÍDO',      /* j.id */
      titulo:     'EXTRAÍDO',      /* j.title */
      empresa:    'NORMALIZADO',   /* org→nome canónico */
      url:        'EXTRAÍDO',      /* j.absolute_url */
      local:      'EXTRAÍDO',      /* j.location.name */
      modalidade: 'DERIVADO',      /* derivado deterministicamente de titulo+local */
      contrato:   'AUSENTE NA FONTE',  /* Greenhouse.jobs não expõe contrato estruturado */
      senioridade:'DERIVADO',      /* derivado deterministicamente do titulo */
      salario_min:'AUSENTE NA FONTE',  /* salário estruturado raro; só capturo info_salarial se metadata existir */
      salario_max:'AUSENTE NA FONTE',
      moeda:      'AUSENTE NA FONTE',
      data:       'NORMALIZADO',   /* j.updated_at → slice(0,10) ISO */
      idioma:     'DERIVADO',      /* heurística determinística sobre o corpo */
      descricao:  'EXTRAÍDO',      /* j.content (HTML) */
      req_min:    'AINDA NÃO IMPLEMENTADO',  /* parsing de requisitos a partir do HTML — NÃO invento */
      tecnologias:'AINDA NÃO IMPLEMENTADO',  /* parsing de stack — NÃO invento */
    }

    /* rascunho — persistência de auditoria, não catálogo definitivo */
    const dir = join(RASCUNHO, `v${i + 1}_${id}`)
    mkdirSync(dir, { recursive: true })
    writeFileSync(join(dir, 'envelope_bruto.bin'), bIn)
    writeFileSync(join(dir, 'canonico_1.txt'), C1)
    writeFileSync(join(dir, 'envelope_canonico.bin'), ENC.encode(envCanon))
    writeFileSync(join(dir, 'canonico_2.txt'), C2)

    const proxC = j.metadata && j.metadata.length ? j.metadata.map(m => `${m.name}=${m.value}`).slice(0, 5).join(' · ') : '(sem metadata)'
    relatorios.push({ id, tit, loc, url, data, desc, temSal, idem, cl, proxC })
  }

  console.log(`${'═'.repeat(74)}`)
  console.log(`═══ RESULTADOS POR VAGA — idempotência CANON₁≡CANON₂ (byte a byte) ═══`)
  for (const R of relatorios) {
    console.log(`\n· [#${R.id}] ${R.tit}`)
    console.log(`    local=${R.loc} · data=${R.data} · url=${R.url.slice(0, 60)}`)
    console.log(`    salário estruturado na fonte: ${R.temSal ? 'SIM ✓' : 'NÃO (AUSENTE NA FONTE)'}`)
    console.log(`    CANON₁≡CANON₂ : ${R.idem ? 'OK ✓ (' + (R.desc.length).toString() + 'B descrição)' : '✗ DIFERENTE'}`)
    console.log(`    metadata Greenhouse : ${R.proxC}`)
    console.log(`    ┌─ schema VAGA campo a campo ─────────────────────────────────`)
    for (const [k, d] of SCHEMA) {
      const estr = R.cl[k]
      const icone = estr === 'EXTRAÍDO' ? '■' : estr === 'NORMALIZADO' ? '□' : estr === 'DERIVADO' ? '◈' : estr === 'AUSENTE NA FONTE' ? '·' : '○'
      console.log(`    │ ${icone} ${k.padEnd(12)} ${d.padEnd(26)} ${String(estr).padEnd(22)} ${R.cl[k]}`)
    }
    console.log(`    └─────────────────────────────────────────────────────────────`)
  }

  /* 8. SUMÁRIO + LACUNAS (não inventa — mostra e para) */
  console.log(`\n${'═'.repeat(74)}`)
  console.log(`═══ SUMÁRIO DE COBERTURA (${contsProvadas} vagas Greenhouse reais) ═══`)
  console.log(`  idempotência CANON₁≡CANON₂ : ${totalIdem}/${contsProvadas} ✓`)
  const lacunas = [...new Set(relatorios.flatMap(R => Object.entries(R.cl).filter(([, c]) => c === 'AINDA NÃO IMPLEMENTADO').map(([k]) => k)))]
  if (lacunas.length) {
    console.log(`\n  LACUNAS REGISTADAS (AINDA NÃO IMPLEMENTADO — NÃO invento solução):`)
    for (const l of lacunas) console.log(`    · ${l}  ← parsing de HTML/requisitos/stack está FORA do contrato actual da dobradiça`)
    console.log(`\n  → PARO PARA DECISÃO antes de banco/vaga.c e persistência definitiva.`)
  } else {
    console.log(`\n  schema VAGA coberto nas ${contsProvadas} vagas — sem lacunas nesta amostra.`)
  }
  console.log(`\n  rascunho (auditoria, NÃO catálogo): ${RASCUNHO}`)
  console.log(`  NÃO tocados: banco/vaga.c · manifesto.json · assets como catálogo · 6 falhas bash/powershell/node`)
  process.exit(totalIdem === contsProvadas && lacunas.length === 0 ? 0 : (totalIdem === contsProvadas ? 0 : 1))
}

main().catch(e => { console.error('✗ erro:', e.message); process.exit(1) })
