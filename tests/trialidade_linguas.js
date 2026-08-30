/* tests/trialidade_linguas.js — Tri(S); canónico = π∘π=π (Alonzo).
 *
 * Trialidade realizada = ∩ Img(π_C) ≅ L_7. T³ não é a definição.
 * Duo ≠ Tri ≠ S3 ≠ π. Não apaga o π do Alonzo.
 *   node tests/trialidade_linguas.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import {
  LINGUAS, MAPAS, RECUSAS,
  tri, procurarCiclo, corposComTriplo, testeTrialidade,
  alonzoEstatuto, relacaoDuoTri, distinguirObjectos,
  medirIdempotencia, tresSuportes,
  quatroObjectos, medirMergeIdemp, medirNegroPar, medirMatrizProjeccoes,
} from '../app/src/banco_trialidade_u.js'
import { mergeEstado, estadoVazio } from '../app/src/banco_disco.js'
import { residuoCiclo } from '../app/src/banco_lei_unica_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const TEX = join(RAIZ, 'corpo_universal.tex')
const FIS = join(RAIZ, 'fisica.tex')
const CAT = join(RAIZ, 'catalogo.tex')
const EIXOS = join(RAIZ, 'tests', 'trialidade.c')
const MOTOR = join(RAIZ, 'app', 'src', 'banco_trialidade_u.js')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const NEGRO = join(RAIZ, 'tests', 'negro.c')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

ok('§T0 motor e fontes no disco',
  existsSync(MOTOR) && existsSync(TEX) && existsSync(FIS) &&
  existsSync(EIXOS) && existsSync(MAN) && existsSync(NEGRO))

const tex = readFileSync(TEX, 'utf8')
const fis = readFileSync(FIS, 'utf8')
const cat = existsSync(CAT) ? readFileSync(CAT, 'utf8') : ''
const eixos = readFileSync(EIXOS, 'utf8')
const man = JSON.parse(readFileSync(MAN, 'utf8'))
const fontes = [fis, cat, eixos]

ok('§T0 linguas e Alonzo mantidos (nao reescritos)',
  /\\label\{univ:def:linguas\}/.test(tex) &&
  /\\label\{univ:def:alonzo-real\}/.test(tex) &&
  /\\mathcal\{U\}_\{\\mathrm\{alg\}\}/.test(tex) &&
  /Fractal n[aã]o [eé] quarta l[ií]ngua/.test(tex) &&
  /mathsf\{Real\}_\{\\mathrm\{fractal\}\}/.test(tex))

ok('§T0 Duo != adjuncao mantido',
  /\\label\{univ:obs:morf-i0\}/.test(tex) &&
  /mathbf\{Duo\}/.test(tex) &&
  /delta\\dashv\\varepsilon/.test(tex))

ok('§T1 tex: Tri(S) boxed; nao e axioma T^3',
  /\\label\{univ:def:tri\}/.test(tex) &&
  /mathsf\{Tri\}\(S\)/.test(tex) &&
  /S_\{\\mathrm\{Alg\}\}/.test(tex) &&
  /tr[eê]s l[ií]nguas/.test(tex) &&
  /N[aã]o [eé] axioma/.test(tex) &&
  /T\^\{3\}/.test(tex))

ok('§T1 tex: trialidade realizada = ∩ Img; T^3 nao e a def',
  /\\label\{univ:def:teste-tri\}/.test(tex) &&
  /\\label\{univ:def:trialidade-real\}/.test(tex) &&
  /f_\{\\mathrm\{AT\}\}/.test(tex) &&
  /f_\{\\mathrm\{TA\}\}/.test(tex) &&
  /f_\{\\mathrm\{AA\}\}/.test(tex) &&
  /operatorname\{id\}/.test(tex) &&
  /res[ií]duo/.test(tex) &&
  /N[aã]o se inventa/.test(tex) &&
  /n[aã]o localizada/.test(tex) &&
  /leitura de/.test(tex) &&
  /pi\\circ\\pi=\\pi/.test(tex) &&
  /bigcap_\{C\}/.test(tex) &&
  /n[aã]o a defini[cç][aã]o/.test(tex))

ok('§T1 tex: Alonzo pi realizado; T^3 nao e a def; I0 fechado',
  /\\label\{univ:obs:alonzo-tri\}/.test(tex) &&
  /\\label\{univ:def:alonzo-idemp\}/.test(tex) &&
  /cat:audit:alonzo/.test(tex) &&
  /fis:def:alonzo/.test(tex) &&
  /projec[cç][aã]o idempotente/.test(tex) &&
  /pi\\circ\\pi=\\pi/.test(tex) &&
  /texttt\{realizado\}/.test(tex) &&
  /N[aã]o se promove/.test(tex) &&
  /N[aã]o se reabre/.test(tex) &&
  /mathrm\{I\}_\{0\}/.test(tex) &&
  /n[aã]o localizada/.test(tex))

ok('§T1 tex: tres suportes; negro = par, nao GR',
  /\\label\{univ:obs:tres-suportes\}/.test(tex) &&
  /M_\{\\mathrm\{WASM\}\}/.test(tex) &&
  /fis:thm:fractalnegro/.test(tex) &&
  /buraco negro/.test(tex) &&
  /N[aã]o se fundem/.test(tex))

ok('§T1 tex: I0 Duo != Tri != S3 != pi; nao fundir',
  /\\label\{univ:obs:trialidade\}/.test(tex) &&
  /mathbf\{Duo\}\\neq\\mathsf\{Tri\}\\neq S_\{3\}\\neq\\pi/.test(tex) &&
  /leitura, n[aã]o operador/.test(tex) &&
  /N[aã]o fundir/.test(tex) &&
  /mathsf\{Alg\}/.test(tex) &&
  /mathsf\{Top\}/.test(tex) &&
  /mathsf\{Ana\}/.test(tex) &&
  /alonzo-idemp/.test(tex))

ok('§T1 tex: relacao Duo--Tri via periodos; 6 nao e T^3',
  /\\label\{univ:obs:duo-tri\}/.test(tex) &&
  /univ:def:star/.test(tex) &&
  /univ:obs:morf-i0/.test(tex) &&
  /obs:rei/.test(tex) &&
  /univ:def:toro/.test(tex) &&
  /mathrm\{lcm\}\(2,3\)=6/.test(tex) &&
  /n[aã]o\} [eé]/.test(tex) &&
  /evid[eê]ncia de trialidade/.test(tex) &&
  /Gap:/.test(tex) &&
  /T\^\{3\}\\\) realizado/.test(tex) &&
  /e\\circ e=e/.test(tex))

ok('§T1 tex: diagrama F/B/N e teorema MetaInd iff pi_U',
  /F,\\,B,\\,N/.test(tex) &&
  /pi_\{\\mathcal\{U\}\}/.test(tex) &&
  /\\label\{univ:thm:metaind-pi\}/.test(tex) &&
  /\\label\{univ:def:ind-pi\}/.test(tex) &&
  /pi_\{F\}\\circ\\pi_\{F\}=\\pi_\{F\}/.test(tex))
ok('§T1 tex: GLH em U_an nao promove a tripla',
  /\\label\{univ:def:glh-an\}/.test(tex) &&
  /\\label\{univ:def:camadas\}/.test(tex) &&
  /\\label\{univ:obs:residuo-glh\}/.test(tex) &&
  /F\\cap B\\cap N/.test(tex) &&
  /[Nn][aã]o promove/.test(tex))

ok('§T1 tex: hexal e rotulo; sem Lei 8; sem Ficha 11',
  /r[oó]tulo, n[aã]o/.test(tex) &&
  /mathbf\{Duo\}/.test(tex) &&
  /N[aã]o h[aá] Lei~8/.test(tex) &&
  /N[aã]o h[aá] Ficha~11/.test(tex))

ok('§T1 tex: lei local ortogonal; Star(U)=D',
  /\\label\{univ:obs:linguas-ortogonal\}/.test(tex) &&
  /\\operatorname\{Star\}\(\\mathcal\{U\}\)=\\mathcal\{D\}/.test(tex))

ok('§T2 Tri(S) sao 3 projeccoes do mesmo S',
  LINGUAS.join(',') === 'Alg,Top,Ana' &&
  MAPAS.join(',') === 'f_AT,f_TA,f_AA' &&
  tri('Alonzo').Alg === 'Alonzo_Alg' &&
  tri('Alonzo').Top === 'Alonzo_Top' &&
  tri('Alonzo').Ana === 'Alonzo_Ana')

{
  const ciclo = procurarCiclo(fontes)
  ok('§T2 motor nao inventa os 3 mapas',
    ciclo.existem === false &&
    ciclo.mapas.f_AT == null &&
    ciclo.mapas.f_TA == null &&
    ciclo.mapas.f_AA == null &&
    ciclo.composto_id === false)
}

{
  const corpos = corposComTriplo(tex, man)
  ok('§T2 so Alonzo tem as 3 projeccoes medidas',
    corpos.length === 1 &&
    corpos[0].id === 'Alonzo' &&
    corpos[0].parte === 'Fractal' &&
    corpos[0].evidencias.includes('fis:def:alonzo') &&
    corpos[0].evidencias.includes('univ:def:alonzo-idemp'))
}

{
  const idemp = medirIdempotencia(tex, fis)
  ok('§T2 idempotencia extraida: pi o pi = pi, ordem 2',
    idemp.estatuto === 'realizado' &&
    idemp.formula === 'pi o pi = pi' &&
    idemp.ordem === 2 &&
    idemp.objeto === 'projeccao idempotente' &&
    idemp.gamma === 'abertura' &&
    idemp.lei7 === 'vinco' &&
    idemp.negro === 'par S,S^v' &&
    idemp.mesmos === false)
}

{
  const ss = tresSuportes(tex, fis)
  ok('§T2 tres S: fractal / banco / negro; nao funde',
    ss.length === 3 &&
    ss[0].S === 'fractal' && ss[0].C === 'Alonzo' && ss[0].presente &&
    ss[1].S === 'banco' && ss[1].C === 'M_WASM' && ss[1].presente &&
    ss[2].S === 'negro' && ss[2].C === '{S,S^v}' && ss[2].presente)
}

{
  const merge = medirMergeIdemp(mergeEstado, estadoVazio)
  const negro = medirNegroPar()
  const ciclo = residuoCiclo()
  const extras = { merge, negro, ciclo }
  const r = testeTrialidade(fontes, tex, man, fis, extras)
  ok('§T3 Alonzo: idempotencia realizada; T^3 e leitura; 0 promocoes',
    r.leitura === 'idempotencia da realizacao tripla' &&
    r.idempotencia === 'realizado' &&
    r.T3 === 'leitura' &&
    r.formula === 'pi o pi = pi' &&
    r.promocoes === 0 &&
    r.corpos[0].id === 'Alonzo')
  const a = alonzoEstatuto(tex, fontes, fis)
  ok('§T3 estatuto Alonzo honesto',
    a.tres_linguas === true &&
    a.idempotencia === 'realizado' &&
    a.T3 === 'leitura' &&
    a.leitura === 'idempotencia da realizacao tripla' &&
    a.promocoes === 0)
  ok('§T3 trialidade tripla nao apaga pi; T^3 nao e def',
    r.trialidade_realizada === 'nao localizada' &&
    r.matriz.pi_alonzo === 'realizado' &&
    r.matriz.corpos_novos === 0 &&
    r.matriz.recusa_T3 === true)
}

{
  const d = distinguirObjectos(eixos)
  ok('§T4 trialidade.c mede S3 dos eixos, nao T das linguas',
    d.eixos_S3 === true &&
    d.linguas_T === false &&
    d.mesmos === false)
  ok('§T4 eixos: trial {-1,0,+1} e 6 permutacoes',
    /\{\s*-1,\s*0,\s*\+1\s*\}/.test(eixos) &&
    /perm\[6\]\[3\]/.test(eixos) &&
    /S3/.test(eixos))
}

{
  const rel = relacaoDuoTri()
  ok('§T5 Duo ordem 2; e o e = e ordem 2; T e leitura; lcm=6 e rotulo',
    rel.ord_Duo === 2 &&
    rel.Duo2_id === true &&
    rel.ord_e === 2 &&
    rel.e2e === true &&
    rel.Duo_eq_e === false &&
    rel.ord_T === 'leitura' &&
    rel.T3_id === 'leitura' &&
    rel.T3_e_definicao === false &&
    rel.lcm_2_3 === 6 &&
    rel.hexal_e_T3 === false &&
    rel.hexal_e_Duo === false &&
    rel.canonico === 'pi o pi = pi' &&
    rel.quatro === 'Duo != Tri != S3 != pi')
  ok('§T5 Duo nao actua nas linguas; adjuncao nao e T',
    rel.duo_actua_linguas === 'nao localizada' &&
    rel.diedral_linguas === 'nao localizada' &&
    rel.adjuncao_e_T === false &&
    /f_AA/.test(rel.gap) &&
    /leitura/.test(rel.gap))
  const q = quatroObjectos()
  ok('§T5 quatro objectos: Duo Tri S3 pi',
    q.length === 4 &&
    q[0].nome === 'Duo' && q[0].estatuto === 'realizado' &&
    q[1].nome === 'Tri' && /leitura/.test(q[1].estatuto) &&
    q[2].nome === 'S3' &&
    q[3].nome === 'pi' && q[3].eq === 'pi o pi = pi')
}

ok('§T5 recusas: sem encaixe Duo, sem Mandelbrot, sem I0, sem fundir S',
  RECUSAS.includes('encaixar Tri em Duo') &&
  RECUSAS.includes('Mandelbrot por decreto') &&
  RECUSAS.includes('hexal 6 como evidência de T^3') &&
  RECUSAS.includes('adjuncao delta⊣ε como T') &&
  RECUSAS.includes('forcar T^3=id sobre e o e = e') &&
  RECUSAS.includes('T^3 como definicao de trialidade') &&
  RECUSAS.includes('fundir Duo, Tri, S3 e pi') &&
  RECUSAS.includes('apagar pi do Alonzo se a interseccao tripla nao fechar') &&
  RECUSAS.includes('fundir fractal/banco/negro num corpo') &&
  RECUSAS.includes('reabrir I0 Fractal/Alonzo'))

{
  const merge = medirMergeIdemp(mergeEstado, estadoVazio)
  const negro = medirNegroPar()
  const ciclo = residuoCiclo()
  const M = medirMatrizProjeccoes(tex, fis, { merge, negro, ciclo })
  ok('§T7 matriz: Alonzo pi realizado; merge idempotente; negro par',
    M.F.estatuto === 'realizado' &&
    M.F.idemp && M.F.img_L7 && M.F.residuo === 0 &&
    merge.idemp && merge.residuo === 0 && !merge.img_L7 &&
    negro.produto_1 && negro.residuo === 0 && !negro.idemp &&
    M.B.idemp && !M.B.img_L7 &&
    M.N.par && !M.N.idemp && !M.N.img_L7)
  ok('§T7 ∩ tripla nao localizada; pi Alonzo nao apagado; MetaInd=pi_U',
    M.interseccao === 'nao localizada' &&
    M.pi_alonzo === 'realizado' &&
    M.MetaInd_iff_piU === true &&
    M.corpos_novos === 0 &&
    ciclo.residuo === 0)
}

ok('§T6 fis:def:alonzo nao nomeia f_AT; fecho e a idempotencia',
  /\\label\{fis:def:alonzo\}/.test(fis) &&
  !/f_\{\\mathrm\{AT\}\}/.test(fis) &&
  !/function\s+f_AT\b/.test(fis) &&
  /\\label\{fis:thm:fractalnegro\}/.test(fis))

ok('§T6 ponte no manifesto; medidor no Alonzo',
  man.corpos?.motor?.ponte_trialidade === 'app/src/banco_trialidade_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_trialidade)) &&
  (man.corpos.lista.find((c) => c.parte === 'Fractal')?.medidores || [])
    .includes('tests/trialidade_linguas.js'))

console.log('')
console.log('  Canónico: π∘π=π (projecção, ordem 2). T³ não é a definição.')
console.log('  Duo ≠ Tri ≠ S3 ≠ π. ∩ Img F/B/N ≅ L_7: nao localizada.')
console.log('  π Alonzo realizado. MetaInd(L7)=L0 iff π_U(L7)=L0.')
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
