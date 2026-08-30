/* tests/grafo_fecho_u.js — grafo de fecho: A = F^{-1}F=I, B = π_U F(L_7)=L_0,
 * composição A∘B / B∘A sem transformação nova. Arestas só as medidas.
 *   node tests/grafo_fecho_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import {
  NOS_LEI, NOS_AUX, arestasMedidas, arestasAusentes,
  residuoCicloA, residuoCicloB, residuoComposicao,
  residuoPental, residuoDuo, residuoNuByte, residuoRotorByte,
  residuoInd8, medeGrafo, psiRoda, lei1Dual, lei5Rotor,
} from '../app/src/banco_grafo_fecho_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const TEX = join(RAIZ, 'corpo_universal.tex')
const FIS = join(RAIZ, 'fisica.tex')
const MOTOR = join(RAIZ, 'app', 'src', 'banco_grafo_fecho_u.js')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const tex = readFileSync(TEX, 'utf8')
const fis = readFileSync(FIS, 'utf8')
const man = JSON.parse(readFileSync(MAN, 'utf8'))

ok('§G0 motor no disco', existsSync(MOTOR))
ok('§G0 tex: grafo de composicao e Res da volta conjunta',
  /\\label\{univ:obs:grafo-fecho\}/.test(tex) &&
  /\\label\{univ:thm:composicao-fechos\}/.test(tex) &&
  /mathcal\{F\}\^\{-1\}\\mathcal\{F\}=I/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}\\bigl\(\\mathcal\{F\}\(L_7\)\\bigr\)=L_0/.test(tex) &&
  /operatorname\{Res\}=0/.test(tex) &&
  /widehat\{L_7\}/.test(tex) &&
  /n[aã]o localizada/.test(tex) &&
  /tests\/grafo\\_fecho\\_u\.js/.test(tex))
ok('§G0 fis: palco realiza a base; grafo nao e circuito',
  /\\label\{fis:obs:quantica-grafo-fecho\}/.test(fis) &&
  /fis:obs:quantica-oito-leis/.test(fis) &&
  /n[aã]o\} um circuito qu[aâ]ntico/.test(fis) &&
  /univ:thm:composicao-fechos/.test(fis) &&
  /widehat\{L_7\}/.test(fis) &&
  /P=k\/N/.test(fis) &&
  /P_i=\\|\\psi_i\\|/.test(fis) &&
  /n[aã]o localizada/.test(fis))
ok('§G0 manifesto: ponte_grafo_fecho',
  man.corpos?.motor?.ponte_grafo_fecho === 'app/src/banco_grafo_fecho_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_grafo_fecho)) &&
  /realizado/.test(man.corpos?.motor?.nucleo?.grafo_fecho || '') &&
  /nao localizada/.test(man.corpos?.motor?.nucleo?.grafo_fecho || ''))

ok('§G1 nos: 8 leis + F, piU, Duo, i; sem L8',
  NOS_LEI.length === 8 &&
  NOS_LEI.every((n, i) => n === `L${i}`) &&
  NOS_AUX.includes('F') && NOS_AUX.includes('piU') &&
  NOS_AUX.includes('Duo') && NOS_AUX.includes('i') &&
  !NOS_LEI.includes('L8'))

{
  const a = arestasMedidas()
  const labels = a.map((e) => `${e.de}->${e.para}:${e.op}`)
  ok('§G1 Ind L_k→L_{k+1} e Ind L7→L0; nao e dobra',
    a.filter((e) => e.op === 'Ind').length === 8 &&
    labels.includes('L0->L1:Ind') &&
    labels.includes('L7->L0:Ind') &&
    !a.some((e) => e.de === 'L0' && e.para === 'L1' && e.op === 'dobra'))
  ok('§G1 MetaInd / piU / F / Finv / piU o F existem',
    labels.includes('L7->L0:MetaInd') &&
    labels.includes('L7->L0:piU') &&
    labels.includes('L7->F:F') &&
    labels.includes('F->L7:Finv') &&
    labels.includes('F->L0:piU o F'))
}

{
  const aus = arestasAusentes()
  ok('§G1 ausentes: dobra L0→L1, L2 auto, L6→Duo, par 1–7, pental=Duo, Born',
    aus.some((e) => e.de === 'L0' && e.para === 'L1' && e.op === 'dobra') &&
    aus.some((e) => e.de === 'L2' && e.op === 'K**=K') &&
    aus.some((e) => e.de === 'L6' && e.para === 'Duo') &&
    aus.some((e) => e.op === 'dual 1†=7') &&
    aus.some((e) => e.de === 'L5' && e.para === 'Duo') &&
    aus.some((e) => e.op === 'Born |psi|^2'))
}

{
  const a = residuoCicloA()
  const b = residuoCicloB()
  ok('§G2 ciclo A: F^{-1}F=I Res=0',
    a.res === 0 && a.formula === 'F^{-1} F = I' && a.estatuto === 'realizado')
  ok('§G2 ciclo B: pi_U F(L_7)=L_0 Res=0',
    b.res === 0 && b.pi === 0 && b.recuperado === 7 && b.estatuto === 'realizado')
}

{
  const c = residuoComposicao()
  ok('§G3 A∘B: pi_U F(F^{-1} F(L_7))=L_0 Res=0',
    c.AoB.res === 0 &&
    c.AoB.pi === 0 &&
    c.AoB.recuperado === 7 &&
    c.AoB.campo_reverteu === true)
  ok('§G3 B∘A: retorno e reversao em L_0 Res=0',
    c.BoA.res === 0 && c.BoA.reversao_L0 === 0)
  ok('§G3 volta conjunta Res=0; sem transformacao nova',
    c.res === 0 &&
    c.transformacao_nova === false &&
    c.estatuto === 'realizado')
  console.log(`  Res A=${c.A.res} B=${c.B.res} AoB=${c.AoB.res} BoA=${c.BoA.res} conjunta=${c.res}`)
}

{
  const p = residuoPental()
  const i1 = psiRoda({ a: 1, b: 0, c: 0, d: 0 })
  ok('§G4 pental i^4=1 Res=0; i^2=-id; nao e Duo',
    p.res === 0 && p.i4_id && p.i2_neg &&
    p.nao_e_duo && p.nao_e_estrela && p.aresta_lei === false &&
    i1.a === 0 && i1.b === 1)
}

{
  const d = residuoDuo()
  ok('§G4 Duo D^2=id Res=0; ≠ Lei 6; ≠ pental',
    d.res === 0 && d.nao_e_lei6 && d.nao_e_pental && d.aresta_lei === false)
}

{
  const n = residuoNuByte()
  ok('§G4 nu∘nu=id no byte Res=0; NAO e L0→L1',
    n.res === 0 && n.aresta_L0_L1 === false &&
    lei1Dual(lei1Dual(0xA5)) === 0xA5 &&
    n.palco_poincare === 'nao localizada')
}

{
  const r = residuoRotorByte()
  ok('§G4 lei5_rotor^4=id Res=0; ≠ psi_roda',
    r.res === 0 && r.nao_e_psi_roda && r.aresta_lei === false &&
    lei5Rotor(lei5Rotor(lei5Rotor(lei5Rotor(1)))) === 1)
}

{
  const i = residuoInd8()
  ok('§G4 Ind^8=id; MetaInd so em 7',
    i.res === 0 && i.meta_so_em_7)
}

{
  const g = medeGrafo()
  ok('§G5 medeGrafo: todos os ciclos Res=0',
    g.res === 0 &&
    g.ciclos.every((c) => c.res === 0) &&
    g.arestas.length === arestasMedidas().length &&
    g.ausentes.length === arestasAusentes().length &&
    g.sem_lei8 && g.sem_par_1_7)
}

console.log('')
console.log('  grafo: L0 -Ind-> ... -Ind-> L7 -F-> hat -piU-> L0')
console.log('  ausentes: dobra L0→L1, L2 auto, L6→Duo, 1†=7, pental=Duo, Born')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
