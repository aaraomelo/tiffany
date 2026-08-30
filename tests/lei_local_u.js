/* tests/lei_local_u.js — varredura de Lei Local: candidatos, 0 promoções.
 *
 * L_S = fibra da face (fis:thm:tecidos (1)) no mesmo S. Não sobe a escada.
 *   node tests/lei_local_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { spawnSync } from 'node:child_process'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { REALIZACOES } from '../app/src/banco_maquina_u.js'
import {
  corposResiduoZero, realizacoesResiduoZero,
  varrerLeiLocal, semPromocoes, RECUSAS, ALVOS,
} from '../app/src/banco_lei_local_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const TEX = join(RAIZ, 'corpo_universal.tex')
const CLI = join(RAIZ, 'tools', 'varredura_lei_local.mjs')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const tex = readFileSync(TEX, 'utf8')
const rel = varrerLeiLocal(man, { spawnSync })

ok('§L0 ponte_varredura no motor',
  man.corpos?.motor?.ponte_varredura === 'app/src/banco_lei_local_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_varredura)))
ok('§L0 ponte_lei_unica e ponte_transf',
  man.corpos?.motor?.ponte_lei_unica === 'app/src/banco_lei_unica_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_lei_unica)) &&
  man.corpos?.motor?.ponte_transf === 'app/src/banco_transf_u.js')
ok('§L0 CLI no disco', existsSync(CLI))
ok('§L0 schema sem kind lei-local / ficha / identidade nova',
  !schema.properties.kind.enum.includes('lei-local') &&
  !schema.properties.kind.enum.includes('lei_local') &&
  schema.properties.kind.enum.includes('realizacao') &&
  schema.properties.estatuto.enum.includes('nao localizada'))

ok('§L1 tex: univ:def:lei-local e fibra dos tecidos',
  /\\label\{univ:def:lei-local\}/.test(tex) &&
  /fis:thm:tecidos/.test(tex) &&
  /univ:def:lei-local-exec/.test(tex) &&
  /univ:obs:lei-local/.test(tex))
ok('§L1 tex: lei local canonica boxed',
  /\\label\{univ:def:lei-local-canonica\}/.test(tex) &&
  /\\mathcal\{L\}_S\(\\mathrm\{id\}\)/.test(tex) &&
  /B\(\\mathrm\{id\}\)/.test(tex) &&
  /underbrace/.test(tex) &&
  /Filtro de Banda/.test(tex) &&
  /selecciona/.test(tex) &&
  /sem ser espectro/.test(tex))
ok('§L1 tex: corte chi_k le supp; Dedekind N/A',
  /\\chi_k/.test(tex) &&
  /fis:thm:H/.test(tex) &&
  /def:corte/.test(tex) &&
  /fis:thm:corte/.test(tex) &&
  /N\/A/.test(tex) &&
  /produz/.test(tex))
ok('§L1 tex: Mecanica fechada; Docker candidato; B(id) medida',
  /\\label\{univ:obs:mecanica\}/.test(tex) &&
  /\\label\{univ:obs:ls-cliente\}/.test(tex) &&
  /cat:ingestao:mecanica/.test(tex) &&
  /fis:def:transf/.test(tex) &&
  /se reabre/.test(tex) &&
  /candidato/.test(tex) &&
  !/\\label\{univ:def:ls-cliente\}/.test(tex))
ok('§L1 tex nao institui Lei 8 nem Ficha 11',
  /N[aã]o h[aá] Lei~8/.test(tex) &&
  /N[aã]o h[aá] Ficha~11/.test(tex) &&
  /n[aã]o se sobe a escada/.test(tex) &&
  /fis:thm:handshake/.test(tex))
{
  const fis = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  const cat = readFileSync(join(RAIZ, 'catalogo.tex'), 'utf8')
  ok('§L1 fis/cat: U consome + nucleo registado',
    /\\label\{fis:obs:U-consome\}/.test(fis) &&
    /\\mathcal\{L\}_S\(\\mathrm\{id\}\)/.test(fis) &&
    /\\label\{cat:nucleo-u\}/.test(cat) &&
    /n[aã]o localizada/.test(cat))
}

ok('§L2 so corpos residuo 0 (realizado + canonico)',
  corposResiduoZero(man).every((c) =>
    c.estatuto === 'realizado' && c.canonico != null) &&
  !corposResiduoZero(man).some((c) => c.parte === 'Eletromagnetismo') &&
  !corposResiduoZero(man).some((c) => c.parte === 'Redes'))
ok('§L2 so realizacoes realizadas; docker fora',
  realizacoesResiduoZero().every((r) => r.estatuto === 'realizado') &&
  !realizacoesResiduoZero().some((r) => r.id === 'docker') &&
  !realizacoesResiduoZero().some((r) => r.id === 'wasm+idb') &&
  !realizacoesResiduoZero().some((r) => r.id === 'mongo'))

ok('§L3 alvos = S_ESTADO, S_DEPOSITO, par WASM/Docker',
  ALVOS.join(',') === 'S_ESTADO,S_DEPOSITO,M_WASM_M_Docker' &&
  rel.alvos.map((a) => a.id).join(',') === ALVOS.join(','))
ok('§L3 0 promocoees; candidatos nao localizada',
  semPromocoes(rel) &&
  rel.promovidos.length === 0 &&
  rel.candidatos.every((c) => c.estatuto === 'nao localizada' &&
    c.nota === 'candidato de Lei Local' &&
    c.estatuto !== 'realizado'))
ok('§L3 M_Docker nao localizada (sonda nao promove)',
  rel.docker.estatuto === 'nao localizada' &&
  REALIZACOES.find((r) => r.id === 'docker')?.estatuto === 'nao localizada' &&
  rel.docker.ciclo_correu === false)
ok('§L3 B(id)/F medidos; L_S_cliente nao promovido a corpo',
  rel.ls_cliente?.estatuto === 'medida' &&
  rel.ls_cliente.transformada_no_motor === true &&
  rel.ls_cliente.B_cliente === 'medida' &&
  rel.ls_cliente.residuo === 0 &&
  rel.ls_cliente.banda_canal === 'sha256' &&
  rel.ls_cliente.corte === 'chi_k' &&
  !rel.candidatos.some((c) => c.id === 'L_S_cliente') &&
  !rel.promovidos.some((c) => c.id === 'L_S_cliente'))
ok('§L3 nucleo no motor: retorno realizado; Docker/GLH/T3 nao localizada',
  rel.nucleo?.retorno?.estatuto === 'realizado' &&
  rel.nucleo?.retorno?.res === 0 &&
  rel.nucleo?.composto?.estatuto === 'realizado' &&
  rel.nucleo?.F_parseval?.estatuto === 'realizado' &&
  rel.nucleo?.glh_byte === 'nao localizada' &&
  rel.nucleo?.M_Docker === 'nao localizada' &&
  rel.nucleo?.T3 === 'nao localizada' &&
  rel.nucleo?.FBN === 'nao localizada')

ok('§L4 recusas I0',
  RECUSAS.includes('Ficha 11') &&
  RECUSAS.includes('corpo novo') &&
  RECUSAS.includes('Lei 8') &&
  RECUSAS.includes('promocao por analogia') &&
  RECUSAS.includes('banda canal = B_cliente') &&
  RECUSAS.includes('FFT inventada') &&
  RECUSAS.includes('L_S^com como corpo') &&
  rel.recusas.length === RECUSAS.length)
ok('§L4 nao inventa corpos',
  rel.corpos_residuo_0.every((id) =>
    (man.corpos.lista || []).some((c) => c.parte === id && c.estatuto === 'realizado')))

{
  const r = spawnSync(process.execPath, [CLI], { encoding: 'utf8' })
  let doc = null
  try { doc = JSON.parse(r.stdout) } catch { doc = null }
  ok('§L5 CLI corre e emite JSON', r.status === 0 && doc && Array.isArray(doc.candidatos))
  ok('§L5 CLI 0 promocoees',
    doc && Array.isArray(doc.promovidos) && doc.promovidos.length === 0 &&
    (doc.candidatos || []).every((c) => c.estatuto === 'nao localizada') &&
    doc.docker?.estatuto === 'nao localizada')
  ok('§L5 lista vazia honesta ou candidatos nao localizada',
    doc && (doc.candidatos.length === 0 ||
      doc.candidatos.every((c) => c.estatuto === 'nao localizada')))
  ok('§L5 CLI mede B(id)/F sem promover L_S_cliente',
    doc && doc.ls_cliente?.estatuto === 'medida' &&
    doc.ls_cliente.transformada_no_motor === true &&
    doc.ls_cliente.residuo === 0 &&
    !(doc.candidatos || []).some((c) => c.id === 'L_S_cliente'))
}

console.log('')
console.log('  candidatos: ' + rel.candidatos.map((c) => c.id + '=' + c.estatuto).join(', ') || '(nenhum)')
console.log('  docker: ' + rel.docker.estatuto +
  (rel.docker.sonda?.disponivel ? ' (sonda ok, sem promocao)' : ' (sonda indisponivel)'))
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
