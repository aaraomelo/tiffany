/* tests/lei_unica_u.js — lei única, período Ind^8=id, fecho MetaInd(L7)=L0.
 *
 * Lê fis:thm:tecidos (não inventa operador). Sem Lei 8. Sem fundir 0 com 7.
 *   node tests/lei_unica_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import {
  DIM, e, Ind, MetaInd, piU, paridadeAnd, medeCiclo, residuoCiclo,
  residuoGlhPi, lei, L0, L7, campoLei, leIndiceDoEspectro, residuoComposto,
} from '../app/src/banco_lei_unica_u.js'
import { medeTransformada } from '../app/src/banco_transf_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const TEX = join(RAIZ, 'corpo_universal.tex')
const MOTOR = join(RAIZ, 'app', 'src', 'banco_lei_unica_u.js')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const tex = readFileSync(TEX, 'utf8')

ok('§U0 motor no disco', existsSync(MOTOR))
ok('§U0 tex: labels do ciclo e da unidade',
  /\\label\{univ:def:lei-unica\}/.test(tex) &&
  /\\label\{univ:cor:metaind-fecho\}/.test(tex) &&
  /\\label\{univ:def:ind-pi\}/.test(tex) &&
  /\\label\{univ:thm:metaind-pi\}/.test(tex) &&
  /\\label\{univ:thm:retorno-canonico\}/.test(tex) &&
  /\\label\{univ:def:unidade\}/.test(tex) &&
  /\\label\{univ:obs:tecidos-periodo\}/.test(tex) &&
  /\\label\{univ:obs:nucleo\}/.test(tex) &&
  /\\label\{univ:obs:real-S\}/.test(tex))
ok('§U0 tex: ciclo Ind / MetaInd boxed; Ind^8 = id',
  /\\mathrm\{Ind\}/.test(tex) &&
  /\\mathrm\{MetaInd\}/.test(tex) &&
  /Ind\}\^\{8\}/.test(tex) &&
  /\\mathrm\{id\}/.test(tex))
ok('§U0 tex: MetaInd iff pi_U; nao contradiz alonzo-idemp',
  /\\label\{univ:thm:metaind-pi\}/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}/.test(tex) &&
  /operatorname\{MetaInd\}\(L_7\)=L_0/.test(tex) &&
  /iff/.test(tex) &&
  /univ:def:alonzo-idemp/.test(tex) &&
  /N[aã]o contradiz/.test(tex))
ok('§U0 tex: U_an = realizacao analitica de pi_U; tres projeccoes',
  /\\label\{univ:def:linguas\}/.test(tex) &&
  /\\label\{univ:def:glh-an\}/.test(tex) &&
  /mathcal\{U\}_\{\\mathrm\{an\}\}/.test(tex) &&
  /realiza[cç][aã]o anal[ií]tica de/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}/.test(tex) &&
  /o que opera/.test(tex) &&
  /onde conecta/.test(tex) &&
  /o que permanece/.test(tex) &&
  /unifica[cç][aã]o por redu[cç][aã]o/.test(tex) &&
  /pi\(F\)=\\pi\(B\)=\\pi\(N\)=L_0/.test(tex) &&
  /F\\cap B\\cap N/.test(tex))
ok('§U0 tex: GLH em U_an; camadas; composto Res=0; GLH nao localizada',
  /\\label\{univ:def:camadas\}/.test(tex) &&
  /\\label\{univ:def:glh-an\}/.test(tex) &&
  /\\label\{univ:obs:residuo-glh\}/.test(tex) &&
  /\\label\{univ:thm:retorno-canonico\}/.test(tex) &&
  /mathrm\{I\}_\{4\}=\\mathrm\{GLH\}/.test(tex) &&
  /Real\}_\{\\mathrm\{an\}\}/.test(tex) &&
  /Res[ií]duo/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}\\bigl\(\\mathcal\{F\}\(L_7\)\\bigr\)=L_0/.test(tex) &&
  /fis:thm:central/.test(tex) &&
  /fis:def:transf/.test(tex) &&
  /operatorname\{Res\}=0/.test(tex) &&
  /unidade operacional/.test(tex) &&
  /n[aã]o localizada/.test(tex) &&
  /[Nn][aã]o promove/.test(tex) &&
  /mathcal\{F\}_\{\\mathcal\{U\}\}/.test(tex))
ok('§U0 tex: Sistema Universal = leis 0-7 e retorno; sem L_8',
  /Sistema Universal/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}\^\{2\}=\\pi_\{\\mathcal\{U\}\}/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}\(\\mathcal\{F\}\(L_7\)\)=L_0/.test(tex) &&
  /Sem \\\(L_8\\\)/.test(tex))
ok('§U0 tex: Ind pi_k boxed; interseccao das imagens',
  /\\label\{univ:def:ind-pi\}/.test(tex) &&
  /pi_k\\circ\\pi_k=\\pi_k/.test(tex) &&
  /bigcap_k/.test(tex) &&
  /operatorname\{Img\}/.test(tex))
ok('§U0 tex: nucleo refina Def U sem a apagar',
  /\\mathcal\{U\}=\(X,X\^\{\*\},\\mathsf\{Mor\},\\mathsf\{Aut\}\)/.test(tex) &&
  /\\mathcal\{L\},\\mathbf\{1\},X,X\^\{\*\}/.test(tex))
ok('§U0 tex: 1 = vinco / cl; sem star novo como operador',
  /operatorname\{vinco\}/.test(tex) &&
  /operatorname\{cl\}/.test(tex) &&
  /N[aã]o se decreta/.test(tex))
ok('§U0 tex: I0 periodos distintos',
  /fis:thm:tecidos/.test(tex) &&
  /fis:thm:largura/.test(tex) &&
  /1,2,3,4,6/.test(tex) &&
  /mathrm\{lcm\}/.test(tex) &&
  /8\\notin/.test(tex))
ok('§U0 tex: L_S(id) e leitura da lei unica; C=Real_S',
  /leitura\} da lei [uú]nica/.test(tex) &&
  /Real\}_\{S\}/.test(tex) &&
  /n[aã]o teorema de instancia/.test(tex))
ok('§U0 tex nao institui Lei 8 nem funde L0 com L7',
  /N[aã]o h[aá] Lei~8/.test(tex) &&
  /N[aã]o se identifica/.test(tex) &&
  /L_0/.test(tex) && /L_7/.test(tex) &&
  /n[aã]o como \}X_\{k\+1\}/.test(tex))

ok('§U1 dim X = 8; e_k = 2^k',
  DIM === 8 &&
  [0, 1, 2, 3, 4, 5, 6, 7].every((k) => e(k) === (1 << k)) &&
  e(8) === null && e(-1) === null)
ok('§U1 Ind^k(L0)=L_k no indice',
  [0, 1, 2, 3, 4, 5, 6].every((k) => Ind(k) === k + 1) &&
  Ind(7) === 0)
ok('§U1 MetaInd so no extremo 7',
  MetaInd(7) === 0 &&
  MetaInd(0) === null &&
  MetaInd(6) === null &&
  MetaInd(8) === null)
ok('§U1 pi_U e o mesmo passo que MetaInd',
  piU(7) === MetaInd(7) &&
  piU(0) === MetaInd(0) &&
  piU(6) === MetaInd(6) &&
  piU(8) === MetaInd(8) &&
  piU(7) === 0)
ok('§U1 extremos nao fundem: Gram <e0,e7>=0',
  e(0) !== e(7) && paridadeAnd(e(0), e(7)) === 0)

{
  const m = residuoCiclo()
  ok('§U2 identidade efectiva Ind^k e Gram = Id',
    m.ind_k && m.gramId && m.leis.length === 8)
  ok('§U2 Ind^8 = id (periodo do catalogo)', m.ind8)
  ok('§U2 MetaInd(L7)=L0 (fibra, nao escada)', m.meta)
  ok('§U2 MetaInd iff pi_U; residuo 0',
    m.piU && m.metaind_iff_piU && m.residuo === 0)
  ok('§U2 2^8 != e_0: subir recusado', m.sobe_recusado)
  ok('§U2 vinco dos extremos sem fusao', m.nao_funde && m.vinco_neutro)
  ok('§U2 residuo 0', m.residuo === 0)
}

{
  const m = medeCiclo()
  ok('§U3 medeCiclo nao inventa 9a lei',
    m.leis.every((L) => L.k >= 0 && L.k <= 7) &&
    !m.leis.some((L) => L.k === 8))
}

{
  ok('§U4 L_7 e L_0 sao o indice e o byte e_k=2^k',
    L7.k === 7 && L7.e === 128 &&
    L0.k === 0 && L0.e === 1 &&
    lei(7).e === e(7) && lei(0).e === e(0))
  const t = medeTransformada()
  const m = residuoComposto()
  const g = residuoGlhPi(t.parseval)
  ok('§U4 campo δ_{e_7}; Gram recupera 7; π_U(byte)=null',
    Array.isArray(campoLei(7)) &&
    campoLei(7)[128] === 1 &&
    m.recuperado === 7 &&
    m.pi === 0 &&
    m.pi_no_byte === null &&
    leIndiceDoEspectro(campoLei(0)) === null)
  ok('§U4 Res = π_U(F(L_7))−L_0 = 0; Parseval 0',
    m.res === 0 &&
    m.parseval === 0 &&
    g.res === 0 &&
    g.parseval_L7 === 0 &&
    g.F_parseval === 0)
  ok('§U4 residuo GLH/F/π: π realizado; composto realizado; GLH nao localizada',
    g.nome === 'Residuo(pi_U(F(L_7))-L_0)' &&
    g.fonte_GLH === 'fis:thm:central' &&
    g.fonte_F === 'fis:def:transf' &&
    g.fonte_pi === 'univ:thm:metaind-pi' &&
    g.fonte_retorno === 'univ:thm:retorno-canonico' &&
    g.pi_realizado === true &&
    g.glh === 'nao localizada' &&
    g.residuo_ciclo === 0 &&
    g.composto === 'realizado' &&
    g.promove_tripla === false)
}

{
  const m = residuoComposto()
  console.log('')
  console.log('  ciclo: L0 -Ind-> ... -Ind-> L7 -MetaInd-> L0; Ind^8=id')
  console.log('  recusas: Lei 8, fundir 0 com 7, 2^8 = e_0, F_U, F∩B∩N, GLH no byte')
  console.log(`  Res = pi_U(F(L_7))-L_0 = ${m.res}  composto=${m.composto}  glh=${m.glh}`)
  console.log(`#TOTAL ${feitas} ${falhas}`)
}
process.exit(falhas ? 1 : 0)
