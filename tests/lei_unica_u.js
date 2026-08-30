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
  residuoGlhByte, nucleoU, invTres, leiCanonica, quadruplaFinita,
} from '../app/src/banco_lei_unica_u.js'
import { medeTransformada, Finv, F, residuoReversao, residuoFaceMult, caractere } from '../app/src/banco_transf_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const TEX = join(RAIZ, 'corpo_universal.tex')
const FIS = join(RAIZ, 'fisica.tex')
const CAT = join(RAIZ, 'catalogo.tex')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const MOTOR = join(RAIZ, 'app', 'src', 'banco_lei_unica_u.js')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const tex = readFileSync(TEX, 'utf8')
const fis = readFileSync(FIS, 'utf8')
const cat = readFileSync(CAT, 'utf8')
const man = JSON.parse(readFileSync(MAN, 'utf8'))

ok('§U0 motor no disco', existsSync(MOTOR))
ok('§U0 tex: 3^{-1} no corte chi via MetaInd; != 1/3',
  /\\label\{univ:obs:inv-tres-corte\}/.test(tex) &&
  /\\label\{univ:def:base-tres\}/.test(tex) &&
  /3\^\{-1\}/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}\\circ\\mathcal\{F\}/.test(tex) &&
  /chi\^\{\(3\)\}/.test(tex) &&
  /e\^\{\(3\)\}=2\^\{2\}/.test(tex) &&
  /neq\s*\\tfrac\{1\}\{3\}/.test(tex) &&
  /n[aã]o localizada/.test(tex) &&
  /fis:def:transf/.test(tex) &&
  /fis:thm:metainducao/.test(tex) &&
  /Real\}_\{\\mathrm\{an\}\}/.test(tex) &&
  /simetria aritm[eé]tica/.test(tex) &&
  /l[ií]ngua an/.test(tex) &&
  /mathbf\{Duo\}\^\{2\}/.test(tex) &&
  /2\^\{3\}/.test(tex))
ok('§U0 tex: BAI/reticulado 2^n nao localiza 1/3; escada so duplica',
  /\\label\{univ:obs:bai-nao-terco\}/.test(tex) &&
  /Biblioteca de Autoriza/.test(tex) &&
  /obs:bai/.test(tex) &&
  /fis:def:B/.test(tex) &&
  /2\^\{n\}/.test(tex) &&
  /mathbb\{Z\}\/2/.test(tex) &&
  /rho_3=\\tfrac\{1\}\{8\}/.test(tex) &&
  /neq\s*\\tfrac\{1\}\{3\}/.test(tex) &&
  /n[aã]o localiza/.test(tex) &&
  /n[aã]o localizada/.test(tex) &&
  /fis:def:arvore/.test(tex) &&
  /univ:def:lcm/.test(tex))
ok('§U0 tex: CF fecha Star(K)=phi; nao fecha 3^{-1}_chi',
  /\\label\{univ:obs:cf-estrela\}/.test(tex) &&
  /\[1;\\overline\{1\}\]/.test(tex) &&
  /operatorname\{Star\}\(K\)/.test(tex) &&
  /3\^\{-1\}_\{\\chi\}/.test(tex) &&
  /mathbb\{N\}\^\{\\mathbb\{N\}\}/.test(tex) &&
  /thm:corte-ponto-fixo/.test(tex) &&
  /thm:ouro/.test(tex) &&
  /sec:codificacoes/.test(tex) &&
  /Hurwitz das CF/.test(tex) &&
  /n[aã]o localizada/.test(tex))
ok('§U0 cat: 3^{-1} via corte registado; 1/3 nao localizada',
  /univ:obs:inv-tres-corte/.test(cat) &&
  /3\^\{-1\}=1\/3/.test(cat) &&
  /univ:obs:cf-estrela/.test(cat) &&
  /thm:corte-ponto-fixo/.test(cat) &&
  /univ:obs:bai-nao-terco/.test(cat) &&
  /obs:bai/.test(cat))
ok('§U0 tex: lei canonica U_can e reconhecimento',
  /\\label\{univ:def:lei-canonica\}/.test(tex) &&
  /\\label\{univ:thm:reconhecimento\}/.test(tex) &&
  /mathcal\{U\}_\{\\mathrm\{can\}\}/.test(tex) &&
  /\(L_0,\\;\\mathbf\{Duo\},\\;\\pi_\{\\mathcal\{U\}\}\\circ\\mathcal\{F\},\\;\\operatorname\{vinco\}\)/.test(tex) &&
  /3\^\{-1\}_\{\\chi\}/.test(tex) &&
  /mathbf\{Duo\}/.test(tex) &&
  /retorna a torre/.test(tex) &&
  /reverte/.test(tex) &&
  /nRightarrow/.test(tex) &&
  /C=\\mathcal\{U\}/.test(tex) &&
  /univ:def:corpo-canonico/.test(tex))
ok('§U0 tex: quadrupla finita U_fin; I0 nao funde com U_can; face *; banach obs',
  /\\label\{univ:def:quadrupla-finita\}/.test(tex) &&
  /\\label\{univ:def:face-mult\}/.test(tex) &&
  /\\label\{univ:obs:banach\}/.test(tex) &&
  /mathcal\{U\}_\{\\mathrm\{fin\}\}/.test(tex) &&
  /\(\\mathbf\{Duo\},\\;\\pi_\{\\mathcal\{U\}\},\\;\\mathcal\{F\},\\;\\mathcal\{F\}\^\{-1\}\)/.test(tex) &&
  /mathcal\{U\}_\{\\mathrm\{can\}\}[\s\S]*?neq[\s\S]*?mathcal\{U\}_\{\\mathrm\{fin\}\}/.test(tex) &&
  /\\mathcal\{F\}\(f\*g\)=\\mathcal\{F\}\(f\)\\cdot\\mathcal\{F\}\(g\)/.test(tex) &&
  /ferramentas de fecho/.test(tex) &&
  /se fundem/.test(tex) &&
  /n[aã]o\} pe[cç]as/.test(tex))
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
ok('§U0 tex: GLH em U_an; camadas; composto Res=0; GLH-byte realizado; continuo nao localizada',
  /\\label\{univ:def:camadas\}/.test(tex) &&
  /\\label\{univ:def:glh-an\}/.test(tex) &&
  /\\label\{univ:obs:residuo-glh\}/.test(tex) &&
  /\\label\{univ:thm:retorno-canonico\}/.test(tex) &&
  /\\label\{univ:def:finv\}/.test(tex) &&
  /\\label\{univ:thm:reversao-byte\}/.test(tex) &&
  /\\label\{univ:thm:glh-dual-fecha\}/.test(tex) &&
  /\\label\{univ:def:XX\}/.test(tex) &&
  /X\^\{X\}/.test(tex) &&
  /mathbb\{N\}\^\{\\mathbb\{N\}\}/.test(tex) &&
  /thm:pisot/.test(tex) &&
  /thm:constr/.test(tex) &&
  /thm:cone/.test(tex) &&
  /fecho finito/.test(tex) &&
  /fis:thm:mu/.test(tex) &&
  /fis:cor:mobius/.test(tex) &&
  /mathrm\{I\}_\{4\}=\\mathrm\{GLH\}/.test(tex) &&
  /Real\}_\{\\mathrm\{an\}\}/.test(tex) &&
  /Res[ií]duo/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}\\bigl\(\\mathcal\{F\}\(L_7\)\\bigr\)=L_0/.test(tex) &&
  /fis:thm:central/.test(tex) &&
  /fis:def:transf/.test(tex) &&
  /operatorname\{Res\}=0/.test(tex) &&
  /unidade operacional/.test(tex) &&
  /mathcal\{F\}\^\{-1\}=2\^\{-m\}\\mathcal\{F\}/.test(tex) &&
  /I=\\mathcal\{F\}\^\{-1\}/.test(tex) &&
  /m_\{\\mathrm\{Hadamard\}\}/.test(tex) &&
  /m_\{\\mathrm\{dobras\}\}=3/.test(tex) &&
  /2\^\{-8\}\\mathcal\{F\}/.test(tex) &&
  /tfrac\{1\}\{8\}\\mathcal\{F\}/.test(tex) &&
  /mathcal\{F\}\^\{-1\}\\neq 3\^\{-1\}_\{\\chi\}/.test(tex) &&
  /n[aã]o localizada/.test(tex) &&
  /[Nn][aã]o promove/.test(tex) &&
  /mathcal\{F\}_\{\\mathcal\{U\}\}/.test(tex) &&
  /3\^\{-1\}_\{\\chi\}/.test(tex) &&
  /Hurwitz no byte/.test(tex))
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
ok('§U0 fis: U consome; nao copia o tratado',
  /\\label\{fis:obs:U-consome\}/.test(fis) &&
  /univ:def:lei-canonica/.test(fis) &&
  /univ:def:quadrupla-finita/.test(fis) &&
  /univ:obs:banach/.test(fis) &&
  /fis:thm:conv/.test(fis) &&
  /univ:thm:reconhecimento/.test(fis) &&
  /univ:thm:retorno-canonico/.test(fis) &&
  /univ:def:lei-local-canonica/.test(fis) &&
  /univ:def:alonzo-idemp/.test(fis) &&
  /univ:def:finv/.test(fis) &&
  /univ:thm:reversao-byte/.test(fis) &&
  /univ:thm:glh-dual-fecha/.test(fis) &&
  /GLH-byte/.test(fis) &&
  /univ:cor:metaind-fecho/.test(fis) &&
  /pi\\circ\\pi=\\pi/.test(fis) &&
  /X_\{k\+1\}/.test(fis) &&
  /N[aã]o se promove/.test(fis) &&
  /Mandelbrot/.test(fis) &&
  /cat:bloco4/.test(fis))
ok('§U0 cat: nucleo registado; U_can; sem Ficha 11',
  /\\label\{cat:nucleo-u\}/.test(cat) &&
  /univ:def:lei-canonica/.test(cat) &&
  /univ:def:quadrupla-finita/.test(cat) &&
  /univ:def:face-mult/.test(cat) &&
  /univ:obs:banach/.test(cat) &&
  /univ:thm:reconhecimento/.test(cat) &&
  /mathcal\{U\}_\{\\mathrm\{can\}\}/.test(cat) &&
  /mathcal\{U\}_\{\\mathrm\{fin\}\}/.test(cat) &&
  /univ:thm:retorno-canonico/.test(cat) &&
  /univ:def:finv/.test(cat) &&
  /univ:thm:reversao-byte/.test(cat) &&
  /univ:thm:glh-dual-fecha/.test(cat) &&
  /mathcal\{F\}\^\{-1\}\\neq 3\^\{-1\}_\{\\chi\}/.test(cat) &&
  /m_\{\\mathrm\{Hadamard\}\}=8/.test(cat) &&
  /m_\{\\mathrm\{dobras\}\}=3/.test(cat) &&
  /Gentil/.test(cat) &&
  /tests\/lei\\_unica\\_u\.js/.test(cat) &&
  /F\\cap B\\cap N/.test(cat) &&
  /M_\{\\mathrm\{Docker\}\}/.test(cat) &&
  /T\^\{3\}/.test(cat) &&
  /sem Ficha~11/.test(cat) &&
  /mathrm\{I\}_\{9\}/.test(cat) &&
  !/\\fichaingestao\{Universal\}/.test(cat) &&
  !/\\fichacapitulo\{Universal\}/.test(cat))
ok('§U0 manifesto: ponte_lei_unica + nucleo',
  man.corpos?.motor?.ponte_lei_unica === 'app/src/banco_lei_unica_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_lei_unica)) &&
  man.corpos?.motor?.ponte_transf === 'app/src/banco_transf_u.js' &&
  /realizado/.test(man.corpos?.motor?.nucleo?.U_can || '') &&
  /lei-canonica/.test(man.corpos?.motor?.nucleo?.U_can || '') &&
  /realizado/.test(man.corpos?.motor?.nucleo?.U_fin || '') &&
  /quadrupla-finita/.test(man.corpos?.motor?.nucleo?.U_fin || '') &&
  /U_can != U_fin/.test(man.corpos?.motor?.nucleo?.U_fin || '') &&
  /ferramentas/.test(man.corpos?.motor?.nucleo?.banach_bw || '') &&
  /univ:obs:banach/.test(man.corpos?.motor?.nucleo?.banach_bw || '') &&
  /realizado/.test(man.corpos?.motor?.nucleo?.retorno || '') &&
  /nao localizada/.test(man.corpos?.motor?.nucleo?.M_Docker || '') &&
  /realizado/.test(man.corpos?.motor?.nucleo?.glh_byte || '') &&
  /m_Hadamard=8/.test(man.corpos?.motor?.nucleo?.glh_byte || '') &&
  /m_dobras=3/.test(man.corpos?.motor?.nucleo?.glh_byte || '') &&
  /2\^\{-8\}F/.test(man.corpos?.motor?.nucleo?.glh_byte || '') &&
  /nao localizada/.test(man.corpos?.motor?.nucleo?.glh_continuo || '') &&
  /denso-XX/.test(man.corpos?.motor?.nucleo?.glh_continuo || '') &&
  /nao localizada/.test(man.corpos?.motor?.nucleo?.T3 || '') &&
  /inv-tres-corte/.test(man.corpos?.motor?.nucleo?.inv_tres || '') &&
  /nao localizada/.test(man.corpos?.motor?.nucleo?.racional_1_3 || '') &&
  /cf-estrela/.test(man.corpos?.motor?.nucleo?.cf_estrela || '') &&
  /Star\(K\)/.test(man.corpos?.motor?.nucleo?.cf_estrela || ''))

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
  ok('§U4 residuo GLH/F/π: π realizado; composto realizado; GLH-byte realizado',
    g.nome === 'Residuo(pi_U(F(L_7))-L_0)' &&
    g.fonte_GLH === 'fis:thm:central' &&
    g.fonte_F === 'fis:def:transf' &&
    g.fonte_pi === 'univ:thm:metaind-pi' &&
    g.fonte_retorno === 'univ:thm:retorno-canonico' &&
    g.fonte_finv === 'univ:def:finv' &&
    g.fonte_reversao === 'univ:thm:reversao-byte' &&
    g.pi_realizado === true &&
    g.glh === 'realizado' &&
    g.glh_byte === 'realizado' &&
    g.glh_continuo === 'nao localizada' &&
    g.reversao === 0 &&
    g.finv === '2^{-m} F' &&
    g.residuo_ciclo === 0 &&
    g.composto === 'realizado' &&
    g.promove_tripla === false &&
    g.fonte_fis === 'fis:obs:U-consome' &&
    g.fonte_cat === 'cat:nucleo-u')
  const n = nucleoU()
  const can = leiCanonica()
  ok('§U4 nucleoU alinhado a fisica/catalogo; recusas intactas',
    n.retorno.estatuto === 'realizado' && n.retorno.res === 0 &&
    n.retorno.fis === 'fis:obs:U-consome' && n.retorno.cat === 'cat:nucleo-u' &&
    n.pi_alonzo.estatuto === 'realizado' &&
    n.F_parseval.estatuto === 'realizado' &&
    n.composto.estatuto === 'realizado' &&
    n.glh_byte === 'realizado' &&
    n.glh_continuo === 'nao localizada' &&
    n.reversao.res === 0 &&
    n.FBN === 'nao localizada' &&
    n.M_Docker === 'nao localizada' &&
    n.T3 === 'nao localizada' &&
    n.racional_1_3 === 'nao localizada' &&
    n.inv_tres.estatuto === 'realizado' &&
    n.inv_tres.res === 0 &&
    n.dois_retornos.finv.m_hadamard === 8 &&
    n.dois_retornos.finv.factor === '2^{-8}' &&
    n.dois_retornos.finv.nao_e_1_8 === true &&
    n.dois_retornos.finv.res === 0 &&
    n.dois_retornos.inv_tres.m_dobras === 3 &&
    n.dois_retornos.inv_tres.res === 0 &&
    n.dois_retornos.finv.formula !== n.dois_retornos.inv_tres.formula)
  ok('§U4 U_can = (L0, Duo, pi_U o F, vinco); reconhecimento',
    n.U_can.formula === '(L0, Duo, pi_U o F, vinco)' &&
    n.U_can.fonte === 'univ:def:lei-canonica' &&
    n.U_can.reconhecimento === 'univ:thm:reconhecimento' &&
    n.U_can.estatuto === 'realizado' &&
    n.U_can.L0.polo === '(+1)⊕(-1)' && n.U_can.L0.dual === '0†=∞' &&
    n.U_can.Duo.star === 'Star(U)=D' && n.U_can.Duo.base === '2^3=8' &&
    n.U_can.retorno.formula === '3^{-1}_chi' &&
    n.U_can.retorno.res === 0 &&
    n.U_can.vinco === '1=vinco(L0,L7)' &&
    n.U_can.racional_1_3 === 'nao localizada' &&
    can.fonte === n.U_can.fonte &&
    can.estatuto === 'realizado')
  const fin = quadruplaFinita()
  ok('§U4 U_fin = (Duo, pi, F, F^{-1}); I0 nao funde com U_can; Banach/BW ferramentas',
    n.U_fin.formula === '(Duo, pi, F, F^{-1})' &&
    n.U_fin.fonte === 'univ:def:quadrupla-finita' &&
    n.U_fin.nao_funde_com === 'univ:def:lei-canonica' &&
    n.U_fin.i0 === 'U_can != U_fin' &&
    n.U_fin.estatuto === 'realizado' &&
    n.U_fin.formula !== n.U_can.formula &&
    n.U_fin.fonte !== n.U_can.fonte &&
    n.duas_quadruplas.fundem === false &&
    n.U_fin.Duo.contrato === 'cisao' &&
    n.U_fin.pi.contrato === 'pi^2=pi' &&
    n.U_fin.pi.estatuto === 'realizado' &&
    n.U_fin.F.contrato === 'F(f*g)=F(f)·F(g)' &&
    n.U_fin.F.face === 'univ:def:face-mult' &&
    n.U_fin.F.res === 0 &&
    n.U_fin.Finv.contrato === 'F^{-1}F=I' &&
    n.U_fin.Finv.res === 0 &&
    n.U_fin.banach_bw.papel === 'ferramentas' &&
    n.U_fin.banach_bw.fonte === 'univ:obs:banach' &&
    n.U_fin.banach_bw.pecas === false &&
    n.U_fin.pisot === 'nao localizada' &&
    n.U_fin.Gentil === 'nao localizada' &&
    n.banach_bw.pecas === false &&
    fin.fonte === n.U_fin.fonte &&
    fin.estatuto === 'realizado')
}

{
  const v = invTres()
  ok('§U5 3^{-1}=pi_U o F no corte; Res=0; != 1/3',
    v.formula === 'pi_U o F' &&
    v.fonte === 'univ:obs:inv-tres-corte' &&
    v.de === 7 &&
    v.para === 0 &&
    v.res === 0 &&
    v.estatuto === 'realizado' &&
    v.racional_1_3 === 'nao localizada' &&
    v.e_terceira === 4 &&
    v.chi_terceira_nao_e_inv === true &&
    v.nao_e_finv === true)
}

{
  const t = medeTransformada()
  const b = residuoGlhByte()
  const f = campoLei(7)
  const rec = Finv(F(f))
  ok('§U6 F^{-1}=2^{-m} F; I = F^{-1}(F(I)); Res=0 no byte',
    t.finv === '2^{-m} F' &&
    t.reversao === 0 &&
    t.inversa === 'univ:def:finv' &&
    t.m === 8 && t.m_hadamard === 8 && t.m_dobras === 3 &&
    t.n === 256 && t.factor === '2^{-8}' && t.nao_e_1_8 === true &&
    residuoReversao(f) === 0 &&
    rec.length === f.length &&
    rec.every((v, i) => v === f[i]) &&
    b.res === 0 &&
    b.formula === 'I = F^{-1}(F(I))' &&
    b.finv === '2^{-m} F' &&
    b.m_hadamard === 8 && b.m_dobras === 3 &&
    b.factor === '2^{-8}' && b.nao_e_1_8 === true &&
    b.glh_byte === 'realizado' &&
    b.glh_continuo === 'nao localizada' &&
    b.nao_e_inv_tres === true &&
    b.n === 256 &&
    b.detalhe.every((d) => d.parseval === 0 && d.reversao === 0))
  ok('§U6 face *: F(f*g)=F(f)·F(g); Res=0',
    residuoFaceMult(campoLei(7), campoLei(0)) === 0 &&
    residuoFaceMult(caractere(1), campoLei(7)) === 0)
}

{
  const m = residuoComposto()
  console.log('')
  console.log('  ciclo: L0 -Ind-> ... -Ind-> L7 -MetaInd-> L0; Ind^8=id')
  console.log('  recusas: Lei 8, fundir 0 com 7, 2^8 = e_0, F_U, F∩B∩N, GLH continuo')
  console.log(`  Res = pi_U(F(L_7))-L_0 = ${m.res}  composto=${m.composto}  glh_byte=${m.glh}`)
  console.log(`#TOTAL ${feitas} ${falhas}`)
}
process.exit(falhas ? 1 : 0)
