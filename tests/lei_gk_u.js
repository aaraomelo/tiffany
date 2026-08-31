/* tests/lei_gk_u.js — conhecer a lei que os cards declaram. Sem realizar, sem cena.
 *
 * original declara ≡ banco reconhece. Cards = evidências, não donos.
 *   node tests/lei_gk_u.js
 */
import { existsSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'
import { memoriaLS } from '../app/src/corpo_disco.js'
import { MAGIA, CHAVE_ESTADO, estadoVazio, leEstado, gravaEstado } from '../app/src/banco_disco.js'
import { discoIsolado, chaveIsolada } from '../app/src/banco_tenant_u.js'
import { ligaIdentidade, idEstavelDaChave } from '../app/src/banco_identidade_u.js'
import { CHAVE_CADEIA } from '../app/src/banco_cristalchain_u.js'
import { CAPACIDADES_TARDAS } from '../app/src/banco_estado_gk_u.js'
import {
  fatiaPecasArt, FASE_AURA, ETAPAS, METODO,
} from '../app/src/banco_inventario_u.js'
import {
  ID_LEI_GK, CHAVE_LEI, PIPE, CICLOS, CICLO, CAMPOS_DECL, LIMIAR_TESTEMUNHAS,
  ID_FASE_AURA, ID_FP_1, ID_REVELA, ID_TRIADE_FECHA, ID_DUAL_SMIN,
  PARAMS_FASE_AURA, DOMINIO_FASE_AURA, LEI_FASE_AURA, LEIS_CANON, OBSERVACOES, INSTRUMENTO_AURA,
  CAMPOS_VARREDURA, EXPR_TRIADE, EXPR_PHI2, EXPR_SIGMA, EXPR_JULIA,
  CONGELADO, FUNIL, EIXOS, STATUS_INTERNO, CRUZ,
  DIM_X, DOBRAS_GERADORAS, LEIS_BASE, EXPR_AFIM, EXPR_VINCO, EXPR_GRUPO, ESPACO, EIXO_AFIM,
  PAPEIS, ONTOLOGIA, CAMPOS_LEI, CAMPOS_GERADOR, CAMPOS_REALIZACAO, CAMPOS_ESPACO, CAMPOS_LEITURA, FUNIL_OBJECTO, FUNIL_TEOREMA, FUNIL_BUSCA, CAMPOS_IDENTIDADE, FUNIL_DESTINO, CAMPOS_ALOJAR, MOTIVO_DESCONHECIDO, ESTATUTO_ALOJAMENTO, ALOJAR_E, REGRA_SUB, GRAU_NA, REGRA_GRAU, PROPS_INTERNAS, REGRA_PROP, REGRA_SUPORTE, REGRA_VIVE,   ONTOLOGIA_FECHADA, FUNIL_CASA, REGRA_ONTO, CASAS, ESPACO_UNICO, REGRA_V, REGRA_ENUN, REGRA_FATOR, REGRA_DOIS, REGRA_SIM, REGRA_2VB, REGRA_CASA_LE, REGRA_INJ, REGRA_FG, REGRA_OFICIO, REGRA_PARTE, REGRA_VE, REGRA_TRESGEO, REGRA_D, REGRA_SUP_EX, REGRA_EX_OBJ, REGRA_PROP_EX, REGRA_BB, REGRA_NT, REGRA_IT, REGRA_PG, REGRA_EULER, REGRA_CICLO, REGRA_HS, REGRA_INC, REGRA_GH, REGRA_PM, REGRA_SG, REGRA_PARES, REGRA_ORD, REGRA_PLANOS, REGRA_CL, REGRA_PS, REGRA_IVG, REGRA_IC, REGRA_IXI, REGRA_PAL, REGRA_TT, REGRA_ALF, REGRA_CC, REGRA_G1, REGRA_KER, REGRA_PR, REGRA_BUR, REGRA_FOL, REGRA_MESMA, REGRA_BIJ, REGRA_BV, REGRA_POS, REGRA_VS, REGRA_ACC, REGRA_DX, REGRA_TR, REGRA_PX, REGRA_UD, REGRA_BID, REGRA_DOP, REGRA_IF, REGRA_HX, REGRA_LF, REGRA_ITR, REGRA_TF, REGRA_QM, REGRA_GI, REGRA_AX, REGRA_2E, REGRA_EQ, REGRA_3S, REGRA_TAB, REGRA_SOMA, REGRA_QAB2, REGRA_4Q, REGRA_CTR, REGRA_LD, REGRA_2F, REGRA_EQN, REGRA_STU, REGRA_STQ, REGRA_STF8, REGRA_STG, REGRA_ZCB, REGRA_REV, REGRA_DS, REGRA_RISO, PENDENTES_V,
  SIGMAS, GERADOR_VIZDOBRA, GERADORES, VINCO_AFIM, VINCO, REALIZACOES,
  EXPR_BASE, EXPR_LEITURA, EXPR_PESO, EXPR_PAR, EXPR_CHI, EXPR_V, EXPR_XOR, EXPR_CONV, EXPR_CF, EXPR_HAT, EXPR_H, EXPR_NORMA_V, EXPR_2P, EXPR_HAM, EXPR_ALPHA, EXPR_A, EXPR_AUTO, EXPR_FG, EXPR_INJ, EXPR_INV, EXPR_END, EXPR_VE, EXPR_VIZ, EXPR_QM, EXPR_PROF, EXPR_DULTRA, EXPR_PERC, EXPR_SIMETRIA, EXPR_ULTRA, EXPR_BOLA, EXPR_PREF, EXPR_COND_D, EXPR_PIR, EXPR_NT, EXPR_CARD_T, EXPR_CAMINHO, EXPR_CHI_E, EXPR_CICLO, EXPR_HS, EXPR_INC, EXPR_GRADE, EXPR_VIZ_G, EXPR_DIM, EXPR_SIG_G, EXPR_PARES, EXPR_R4, EXPR_R2, EXPR_PLANOS, EXPR_COD, EXPR_LIN, EXPR_BLOQ, EXPR_IXI, EXPR_CONT, EXPR_ESC, EXPR_PAL, EXPR_ALF, EXPR_TLETRA, EXPR_REC, EXPR_G1, EXPR_KER, EXPR_RHO, EXPR_BUR, EXPR_FOL, EXPR_G1SUM, EXPR_S, EXPR_GUM, EXPR_VARPHI, EXPR_POSL, EXPR_DX, EXPR_DMAX, EXPR_TRAV, EXPR_PIP, EXPR_UNI, EXPR_DOP, EXPR_FACES, EXPR_IF, EXPR_M, EXPR_TFIO, EXPR_AA, EXPR_GMEDE, EXPR_D2, EXPR_EQA, EXPR_TAB, EXPR_SOMA, EXPR_QDT, EXPR_QUAD, EXPR_CTR, EXPR_L0, EXPR_L1, EXPR_F0, EXPR_F1, EXPR_EQF, EXPR_ST, EXPR_ENC, EXPR_REV, EXPR_ROTOR, EXPR_QUARTETO, EXPR_QAB, ROTULOS_QAB, RELS, RELS_LEITURA, CADEIA, ARESTAS, ARESTAS_LEITURA, HOSPEDES,
  LEITURA_COORD, LEITURAS, grafoDependencia, registroLeitura, estruturaX,
  ek, coord, coords, pesoHamming, prof, dUltra, prefixoIgual, irmaoArvore, chiEuler, handshakeLocal, vizB, grauM, grau2n, nPlanos, viznisoBloqueia, xorPalavra, prefixosColidem, rho2t, rhoBuracos, folga, serpentina, serpentinaInv, phiEnumera, phiInv, supDx, leituraR, permPi, qPi, precoPi, aplicaTrav, eixosF, compoeA, dualOps, paridadeD, parAB, compoePar, rotuloPar, estrelaEmB, fibU, cassini, estrelaEmZc, sigmaBFixa, gramOrtonormal, sigmaI, grupoGeradores8, qAb, quartetoN2, grauNaConstrucao, pertenceA, geradoPorVizdobra, viveEmX,
  doisAndares, xorX, convRamos, operadorCf, hatF, matrizH, ramosDoAndarV, dobraNorma, modosRamos, convTeorema, topoTese, arvoreRamos, ultraLema, bolasRamos, vizArvoreRamos, percursoRamos, eulerRamos, handshakeRamos, gradeRamos, vizdobraTopo, fechoTopo, viznisoRamos, matrizRamos, palavraRamos, palavraTeorema, contrariaRamos, folgaRamos, bijeccaoRamos, enumeraRamos, navegaRamos, travessiaRamos, duomorfRamos, octoniaoObs, dominioInterface, eixosRamos, dualInvolutiva, duoComposicao, quartetoDuo, trialRamos, duasFacesRamos, estrelaRamos, estrelaEncaixe, duoEstrelaRamos,
  ondeEntraAfim, ondeEntraVinco, classificaObjecto, camposDoPapel, temCampos,
  expressaoNaoELei, registroVinco, registroVizdobra, alojarTeorema, medirELer, registroCandidato, buscaFecho, cruzarIdentidade, pesoK, baseClausula3, paridadeKJ, dualParidade, destinosDoTeorema,
  CANDIDATO_ROTOR, CANDIDATO_DTC, CANDIDATOS,
  declaracoesDoMan, originalDeclara, montaLei, reconheceLei,
  catalogoLeis, igualLei, igualCatalogoLeis,
  varreduraLegendas, normalizaExpr, eixosDaLei, admiteLei,
  leLeiSelecionada, disparaLei,
  leiParaU, uParaLei, faseNaDesc,
} from '../app/src/banco_lei_gk_u.js'

const RAIZ = join(dirname(fileURLToPath(import.meta.url)), '..')
const MAN = join(RAIZ, 'conecthus', 'backends', 'manifesto.json')
const SCHEMA = join(RAIZ, 'conecthus', 'schema', 'u.schema.json')
const INST = join(RAIZ, 'conecthus', 'schema', 'lei_gk.json')
const FRONT = join(RAIZ, 'app', 'src', 'banco_front.js')
const PONTE = join(RAIZ, 'app', 'src', 'banco_lei_gk_u.js')
const INV = join(RAIZ, 'app', 'src', 'banco_inventario_u.js')
const DISCO = join(RAIZ, 'app', 'src', 'banco_disco.js')
const ESTADO = join(RAIZ, 'app', 'src', 'banco_estado_gk_u.js')
const MAN_GK = join(RAIZ, 'app', 'src', 'manifesto.json')
const ART = join(RAIZ, 'app', 'src', 'kernels_campo.json')
const CARDS = join(RAIZ, 'app', 'src', 'banco_cards_u.js')

let falhas = 0
let feitas = 0
function ok (q, cond) {
  feitas++
  if (!cond) falhas++
  console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`)
}

const man = JSON.parse(readFileSync(MAN, 'utf8'))
const schema = JSON.parse(readFileSync(SCHEMA, 'utf8'))
const inst = JSON.parse(readFileSync(INST, 'utf8'))
const front = readFileSync(FRONT, 'utf8')
const ponte = readFileSync(PONTE, 'utf8')
const discoSrc = readFileSync(DISCO, 'utf8')
const estadoSrc = readFileSync(ESTADO, 'utf8')
const cardsSrc = readFileSync(CARDS, 'utf8')
const gkMan = JSON.parse(readFileSync(MAN_GK, 'utf8'))
const pecas = fatiaPecasArt(readFileSync(ART, 'utf8'))
const pub = '00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff'

const prova = reconheceLei(gkMan, pecas, LEI_FASE_AURA)
const cat = catalogoLeis(gkMan, pecas)
const lei = cat[0]
const dump = JSON.stringify(cat)

ok('§K0 instancia id=gk, nao kind novo',
  inst.kind === 'pagina' && inst.id === ID_LEI_GK && inst.camada === 'descricao' &&
  !schema.properties.kind.enum.includes('lei') &&
  !schema.properties.kind.enum.includes('lei_gk'))
ok('§K0 ponte_lei_gk no motor',
  man.corpos?.motor?.ponte_lei_gk === 'app/src/banco_lei_gk_u.js' &&
  existsSync(join(RAIZ, man.corpos.motor.ponte_lei_gk)))
ok('§K0 schema_lei_gk no disco',
  man.corpos?.motor?.schema_lei_gk === 'conecthus/schema/lei_gk.json' &&
  existsSync(join(RAIZ, man.corpos.motor.schema_lei_gk)))
ok('§K0 nucleo: conhecer != realizar != cena; cards evidencias',
  /conhecer != realizar != cena/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
  /cards sao evidencias/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
  /fase-aura/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
  /limiar 12/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
  /fisica\.tex/.test(man.corpos?.motor?.nucleo?.lei_gk || ''))

ok('§K1 F0-F7 nao reabertos; F8 fora; capacidades tardias intactas',
  CAPACIDADES_TARDAS.map((c) => c.id).join(' ') === 'cards latex glsl' &&
  !/ciclo: 'F8'/.test(estadoSrc) &&
  ETAPAS.join('>') === 'F3>inventario>semantica>lei>F8' &&
  METODO.join('>') === 'observar>relacionar>provar>promover' &&
  CICLO === 'conhecer' &&
  CICLOS.join('>') === 'conhecer>realizar>cena' &&
  PIPE.join('>') === 'card>declaracao>lei>instrumento' &&
  CAMPOS_DECL.join('>') === 'op>regua>tag>formula' &&
  ponte.startsWith('// banco_lei_gk'))

ok('§K1 pipe: op != regua != tag != formula',
  CAMPOS_DECL.length === 4 &&
  new Set(CAMPOS_DECL).size === 4 &&
  /op != regua != tag != formula/.test(ponte) &&
  /op != regua != tag != formula/.test(leiParaU(cat).proibicao))

{
  const decls = declaracoesDoMan(gkMan)
  const comFormula = decls.filter((d) => d.formula)
  ok('§K1 declaracao != titulo: 92 cards, formula so em 12',
    decls.length === 92 &&
    comFormula.length === 12 &&
    comFormula.every((d) => d.formula === FASE_AURA) &&
    decls.filter((d) => d.op === d.regua && d.regua).length === 8)
}

ok('§K2 original declara fase-aura ≡ banco reconhece',
  prova.ok &&
  prova.orig.length === 12 &&
  lei && igualLei(lei, prova.banco) &&
  lei.id === ID_FASE_AURA &&
  lei.expressao === FASE_AURA &&
  lei.instrumento === INSTRUMENTO_AURA &&
  lei.nEvidencias === 12 &&
  lei.ciclo === 'conhecer' &&
  lei.realiza === false &&
  lei.cena === false &&
  lei.mesmoInstrumento === true &&
  lei.parametros.join(' ') === PARAMS_FASE_AURA.join(' ') &&
  lei.dominio.join(' ') === DOMINIO_FASE_AURA.join(' ') &&
  PARAMS_FASE_AURA.join(',') === 'f,r,a,χ,n')

ok('§K2 12 evidencias: kernel=aura; corte|elenco; cards nao sao donos',
  prova.orig.every((e) => e.kernel === 'aura') &&
  prova.orig.every((e) => e.secao === 'corte' || e.secao === 'elenco') &&
  prova.orig.every((e) => e.formula === FASE_AURA) &&
  new Set(prova.orig.map((e) => e.op)).size === 12 &&
  prova.orig.some((e) => e.nome === 'rainha_tiffany') &&
  !prova.orig.some((e) => e.nome === 'captura') &&
  originalDeclara(gkMan, pecas, LEI_FASE_AURA).length === 12)

ok('§K2 GLSL nao entra; lei nao e shader',
  !dump.includes('#version') &&
  !dump.includes('precision highp') &&
  !/from ['"]\.\/kernels_campo/.test(ponte) &&
  !ponte.includes('art.kernels') &&
  faseNaDesc(prova.orig[0].formula) === FASE_AURA)

ok('§K2 P→U→P; camada descricao; 5 leis conhecidas',
  leiParaU(cat).kind === 'pagina' &&
  leiParaU(cat).camada === 'descricao' &&
  leiParaU(cat).slots.conhecer === 'realizado' &&
  leiParaU(cat).slots.realizar === 'N/A' &&
  leiParaU(cat).slots.cena === 'N/A' &&
  leiParaU(cat).filhos.length === 5 &&
  leiParaU(cat).filhos[0].id === ID_FASE_AURA &&
  uParaLei(leiParaU(cat)).nLeis === 5 &&
  uParaLei(leiParaU(cat)).nEvidencias === 12 + 92 + 21 + 16 + 16 &&
  uParaLei(leiParaU(cat)).ciclo === 'conhecer')

{
  const by = Object.fromEntries(cat.map((l) => [l.id, l]))
  ok('§K2 arqueologia: 92 cards → 5 leis (≥12 testemunhas); n=k nao e lei',
    LIMIAR_TESTEMUNHAS === 12 &&
    LEIS_CANON.length === 5 &&
    cat.length === 5 &&
    by[ID_FASE_AURA].nEvidencias === 12 && by[ID_FASE_AURA].instrumento === 'aura' &&
    by[ID_FP_1].nEvidencias === 92 && by[ID_FP_1].expressao === 'FP=1' &&
    by[ID_REVELA].nEvidencias === 21 && by[ID_REVELA].expressao === 'a: 0 → 1' &&
    by[ID_TRIADE_FECHA].nEvidencias === 16 && by[ID_TRIADE_FECHA].instrumento === 'textura' &&
    by[ID_TRIADE_FECHA].mesmoInstrumento === true &&
    by[ID_DUAL_SMIN].nEvidencias === 16 && by[ID_DUAL_SMIN].instrumento === 'respira' &&
    !cat.some((l) => /^n=/.test(l.id) || l.id === 'pbr-6' || l.expressao === 'n=2') &&
    reconheceLei(gkMan, pecas, { id: 'n=2', expressao: 'n=2', instrumento: '' }).ok === false)
}

{
  const V = varreduraLegendas(gkMan)
  const idsL = V.conhecidas.map((g) => g.id).sort().join(' ')
  const idsO = V.observadas.map((g) => g.id).sort().join(' ')
  ok('§K2 varredura: mesma expressao => mesma candidata; >=12 => conhecida',
    CAMPOS_VARREDURA.join('>') === 'regua>fp>revela>pbr>dual>formula' &&
    V.nCards === 92 && V.nOps === 63 &&
    V.nConhecidas === 5 &&
    idsL === [ID_DUAL_SMIN, ID_FASE_AURA, ID_FP_1, ID_REVELA, ID_TRIADE_FECHA].sort().join(' ') &&
    V.parametros.some((g) => g.expr === 'n' && g.n >= 30) &&
    V.bracos.some((g) => g.id === ID_TRIADE_FECHA) &&
    idsO.includes('phi2') && idsO.includes('sigma-dual') && idsO.includes('julia-c') &&
    V.observadas.every((g) => g.n < LIMIAR_TESTEMUNHAS) &&
    V.realiza === false && V.cena === false)
}

{
  const by = Object.fromEntries(cat.map((l) => [l.id, l]))
  const fis = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  const catTx = readFileSync(join(RAIZ, 'catalogo.tex'), 'utf8')
  function temLabel (src, lab) {
    return src.includes('\\label{' + lab + '}')
  }
  const labsF = LEIS_CANON.flatMap((l) => l.refs.fisica).concat(OBSERVACOES.flatMap((o) => o.refs.fisica))
  const labsC = LEIS_CANON.flatMap((l) => l.refs.catalogo).concat(OBSERVACOES.flatMap((o) => o.refs.catalogo))
  ok('§K7 refs existem em fisica.tex e catalogo.tex',
    labsF.every((lab) => temLabel(fis, lab)) &&
    labsC.every((lab) => temLabel(catTx, lab)) &&
    by[ID_TRIADE_FECHA].refs.cruz === 'realizado' &&
    by[ID_FASE_AURA].refs.cruz === 'relacionado' &&
    by[ID_FP_1].refs.fisica.includes('fis:em-pergunta') &&
    OBSERVACOES.every((o) => o.refs.cruz === 'realizado') &&
    /cruz=realizado/.test(leiParaU(cat).filhos.find((f) => f.id === ID_TRIADE_FECHA).evidencia))
}

{
  const by = Object.fromEntries(cat.map((l) => [l.id, l]))
  const cruzadas = cat.filter((l) => l.refs.cruz === 'realizado')
  const phi = admiteLei({ expressao: EXPR_PHI2, n: 1, cruz: 'realizado' })
  const nova = admiteLei({ expressao: 'x', nEvidencias: LIMIAR_TESTEMUNHAS, cruz: 'relacionado' })
  const eTriade = eixosDaLei(by[ID_TRIADE_FECHA])
  ok('§K8 contrato congelado: eixos independentes; cruzamento nao vaza',
    CONGELADO === true &&
    FUNIL.join('>') === 'expressao>testemunhas>limiar>cruzamento' &&
    EIXOS.join('>') === 'interno>cruzamento' &&
    STATUS_INTERNO.join('>') === 'conhecida>observada' &&
    CRUZ.join('>') === 'realizado>relacionado' &&
    /CONGELADO/.test(inst.nota) &&
    /CONGELADO/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    /F8 ainda nao/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.every((l) => l.statusInterno === 'conhecida' && l.realiza === false && l.cena === false) &&
    cruzadas.length === 1 && cruzadas[0].id === ID_TRIADE_FECHA &&
    by[ID_DUAL_SMIN].refs.cruz === 'relacionado' && by[ID_DUAL_SMIN].expressao === 'smooth-min' &&
    by[ID_FP_1].nEvidencias === 92 && by[ID_FP_1].refs.cruz === 'relacionado' &&
    !cat.some((l) => l.id === 'phi2' || l.id === 'sigma-dual' || l.id === 'julia-c') &&
    OBSERVACOES.every((o) => o.refs.cruz === 'realizado') &&
    eTriade.interno.status === 'conhecida' &&
    eTriade.cruzamento.cruz === 'realizado' &&
    eTriade.realiza === false && eTriade.cena === false &&
    leiParaU(cat).slots.conhecer === 'realizado' &&
    leiParaU(cat).slots.realizar === 'N/A' &&
    leiParaU(cat).slots.cena === 'N/A' &&
    phi.ok === false && phi.statusInterno === 'observada' && phi.realiza === false && phi.f8 === false &&
    nova.ok === true && nova.realiza === false && nova.cena === false && nova.f8 === false &&
    /funil=expressao>testemunhas>limiar>cruzamento/.test(leiParaU(cat).nota))
}

{
  const univ = readFileSync(join(RAIZ, 'corpo_universal.tex'), 'utf8')
  const fisTx = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  function temLab (src, lab) {
    return src.includes('\\label{' + lab + '}')
  }
  const afim = admiteLei({ expressao: EXPR_AFIM, n: 3, cruz: 'relacionado' })
  const slot = ondeEntraAfim()
  const labsU = ESPACO.refs.filter((l) => l.startsWith('univ:'))
  const labsF = ESPACO.refs.filter((l) => l.startsWith('fis:')).concat(EIXO_AFIM.refs.fisica.filter((l) => l.startsWith('fis:')))
  const labsUnivAfim = EIXO_AFIM.refs.fisica.filter((l) => l.startsWith('univ:'))
  ok('§K9 espaco X dim=8; 3 dobras -> 8 leis; afim na 3a dobra',
    DIM_X === 8 && DIM_X === 2 ** DOBRAS_GERADORAS.length &&
    LEIS_BASE.join('') === '01234567' && ek(8) === null &&
    LEIS_BASE.every((k) => ek(k) === (1 << k) && coord(ek(k), k) === 1) &&
    gramOrtonormal() === true &&
    coords(255).join('') === '11111111' &&
    coords(0).join('') === '00000000' &&
    EIXOS.length === 2 && DOBRAS_GERADORAS.length === 3 &&
    EIXO_AFIM.dobra === 3 && EIXO_AFIM.formula === EXPR_AFIM &&
    EIXO_AFIM.cruz === 'relacionado' && EIXO_AFIM.papel === 'realizacao' &&
    EIXO_AFIM.nao.includes('Pontryagin') && EIXO_AFIM.nao.includes('associador') &&
    EIXO_AFIM.nao.includes('lei_gk') && EIXO_AFIM.nao.includes('coord2-terceira') &&
    EXPR_TRIADE.includes('Pontryagin') && EXPR_TRIADE !== EXPR_AFIM &&
    slot.conhecida === false && slot.realiza === false && slot.f8 === false &&
    afim.ok === false && afim.statusInterno === 'observada' &&
    cat.length === 5 && !cat.some((l) => l.expressao === EXPR_AFIM) &&
    leiParaU(cat).slots.espaco === 'realizado' &&
    leiParaU(cat).slots.afim === 'relacionado' &&
    /dim=8/.test(leiParaU(cat).nota) && /dobras=3/.test(leiParaU(cat).nota) &&
    labsU.every((lab) => temLab(univ, lab)) &&
    labsUnivAfim.every((lab) => temLab(univ, lab)) &&
    labsF.every((lab) => temLab(fisTx, lab)))
}

{
  const univ = readFileSync(join(RAIZ, 'corpo_universal.tex'), 'utf8')
  const fisTx = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  function temLab (src, lab) {
    return src.includes('\\label{' + lab + '}')
  }
  const v = admiteLei({ expressao: EXPR_VINCO, n: 8, cruz: 'relacionado' })
  const slot = ondeEntraVinco()
  const cLei = classificaObjecto({ papel: 'lei', nEvidencias: 16, cruz: 'realizado' })
  const cGer = classificaObjecto(GERADORES[0])
  const cVin = classificaObjecto(VINCO_AFIM)
  const pontoFixo = JSON.stringify(gkMan).split('ponto fixo').length - 1
  ok('§K10 lei != gerador != realizacao; vinco relacionado; sigma_i geram 8',
    PAPEIS.join('>') === 'lei>gerador>realizacao' &&
    GERADORES.length === 1 && GERADOR_VIZDOBRA.elementos.length === 3 &&
    REALIZACOES.length === 2 &&
    grupoGeradores8() === true &&
    sigmaI(1, 0) === 1 && sigmaI(2, 0) === 2 && sigmaI(3, 0) === 4 &&
    sigmaI(4, 0) === null &&
    GERADORES.every((g) => g.papel === 'gerador') &&
    VINCO_AFIM.formula === EXPR_VINCO && VINCO_AFIM.papel === 'realizacao' &&
    VINCO_AFIM.cruz === 'relacionado' && VINCO_AFIM.depende === 'eixo-afim' &&
    VINCO_AFIM.nao.includes('unidade') && VINCO_AFIM.nao.includes('estrela-sigma') &&
    VINCO_AFIM.nao.includes('sigma-dual') && VINCO_AFIM.nao.includes('lei_gk') &&
    EXPR_VINCO !== EXPR_AFIM && EXPR_VINCO !== EXPR_SIGMA &&
    EXPR_SIGMA !== 'σ₁' &&
    cLei.papel === 'lei' && cLei.conhecida === true && cLei.realiza === false &&
    cGer.papel === 'gerador' && cGer.conhecida === false &&
    cVin.papel === 'realizacao' && cVin.conhecida === false && cVin.colapsa === false &&
    v.ok === false && v.statusInterno === 'observada' &&
    slot.conhecida === false && slot.f8 === false &&
    pontoFixo < LIMIAR_TESTEMUNHAS &&
    cat.length === 5 && !cat.some((l) => l.expressao === EXPR_VINCO) &&
    leiParaU(cat).slots.geradores === 'realizado' &&
    leiParaU(cat).slots.vinco === 'relacionado' &&
    /papeis=lei>gerador>realizacao/.test(leiParaU(cat).nota) &&
    temLab(fisTx, 'fis:thm:vinco') && temLab(fisTx, 'fis:thm:vizdobra') &&
    temLab(univ, 'univ:def:unidade') &&
    /lei != gerador != realizacao/.test(man.corpos?.motor?.nucleo?.lei_gk || ''))
}

{
  const reg = registroVinco()
  const infla = admiteLei({ expressao: EXPR_VINCO, nEvidencias: 99, cruz: 'relacionado' })
  const u = leiParaU(cat)
  ok('§K11 realizacao:vinco; lei_gk != vinco; formula nao implica lei',
    CAMPOS_LEI.join('>') === 'expressao>testemunhas>instrumento' &&
    CAMPOS_GERADOR.join('>') === 'objeto>estados>origem' &&
    CAMPOS_REALIZACAO.join('>') === 'objeto>origem>endereco' &&
    FUNIL_OBJECTO.join('>') === 'expressao>classificacao>papel>cruzamento' &&
    temCampos(VINCO, 'realizacao') &&
    GERADORES.every((g) => temCampos(g, 'gerador') && g.estados === 8) &&
    temCampos(montaLei(LEI_FASE_AURA, prova.orig), 'lei') &&
    expressaoNaoELei(EXPR_VINCO) && expressaoNaoELei(EXPR_AFIM) &&
    !expressaoNaoELei(FASE_AURA) &&
    reg.id === 'vinco' && reg.papel === 'realizacao' &&
    reg.origem === 'dobra-3' && reg.endereco === 'Fix(∂)' &&
    reg.expressao === EXPR_VINCO && reg.cruz === 'relacionado' &&
    reg.lei_gk === 'N/A' && reg.ciclo === 'conhecer' &&
    VINCO.lei_gk === 'N/A' &&
    !LEIS_CANON.some((l) => l.id === 'vinco') &&
    !cat.some((l) => l.id === 'vinco') &&
    u.filhos.every((f) => f.id !== 'vinco') &&
    infla.ok === false &&
    cat.filter((l) => l.refs.cruz === 'realizado').length === 1 &&
    cat.filter((l) => l.refs.cruz === 'realizado')[0].id === ID_TRIADE_FECHA &&
    /formula != lei/.test(u.proibicao) &&
    /realizacao:vinco/.test(man.corpos?.motor?.nucleo?.lei_gk || ''))
}

{
  const g = registroVizdobra()
  const fisTx = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  const um = admiteLei({ expressao: 'σ₁', nEvidencias: 99 })
  const grp = admiteLei({ expressao: EXPR_GRUPO, nEvidencias: 99 })
  const dual = admiteLei({ expressao: EXPR_SIGMA, n: 1 })
  ok('§K12 gerador:vizdobra e um grupo; sigma_i != sigma-dual; nao 3 leis',
    GERADORES.length === 1 && SIGMAS.length === 3 &&
    GERADOR_VIZDOBRA.id === 'vizdobra' &&
    GERADOR_VIZDOBRA.objeto === EXPR_GRUPO &&
    GERADOR_VIZDOBRA.estados === 8 && GERADOR_VIZDOBRA.origem === 'dobra' &&
    GERADOR_VIZDOBRA.lei_gk === 'N/A' &&
    GERADOR_VIZDOBRA.nao.includes('sigma-dual') &&
    EXPR_GRUPO !== EXPR_SIGMA && SIGMAS.every((s) => s.objeto !== EXPR_SIGMA) &&
    !SIGMAS.some((s) => s.papel === 'gerador' || s.papel === 'lei') &&
    temCampos(GERADOR_VIZDOBRA, 'gerador') &&
    g.papel === 'gerador' && g.elementos === 3 && g.lei_gk === 'N/A' && g.f8 === false &&
    um.ok === false && grp.ok === false &&
    dual.ok === false && OBSERVACOES.some((o) => o.id === 'sigma-dual') &&
    grupoGeradores8() &&
    sigmaI(1, sigmaI(1, 5)) === 5 &&
    sigmaI(1, sigmaI(2, 5)) === sigmaI(2, sigmaI(1, 5)) &&
    !cat.some((l) => l.id === 'vizdobra' || l.id === 'sigma1') &&
    cat.length === 5 &&
    fisTx.includes('\\label{fis:thm:vizdobra}') &&
    /gerador:vizdobra/.test(man.corpos?.motor?.nucleo?.lei_gk || ''))
}

{
  const G = grafoDependencia()
  const baseLei = admiteLei({ expressao: EXPR_BASE, nEvidencias: 99 })
  const dimLei = admiteLei({ expressao: 'dim=8', nEvidencias: 99 })
  const u = leiParaU(cat)
  ok('§K13 grafo vizdobra>X>base>afim>vinco; estrutura != lei; 5 leis',
    CADEIA.join('>') === 'vizdobra>X>base>afim>vinco' &&
    RELS.join('>') === 'gera>realiza>fixa' &&
    ARESTAS.length === 3 &&
    ARESTAS[0].de === 'vizdobra' && ARESTAS[0].rel === 'gera' && ARESTAS[0].para === 'X' &&
    ARESTAS[1].de === 'X' && ARESTAS[1].rel === 'realiza' && ARESTAS[1].para === 'afim' &&
    ARESTAS[2].de === 'afim' && ARESTAS[2].rel === 'fixa' && ARESTAS[2].para === 'vinco' &&
    G.cadeia === CADEIA && G.promove === false && G.nLeis === 5 && G.f8 === false &&
    ESPACO.id === 'X' && ESPACO.lei_gk === 'N/A' && ESPACO.gerador === 'N/A' &&
    ESPACO.base === EXPR_BASE && ESPACO.gram === 'Id' && gramOrtonormal() &&
    expressaoNaoELei(EXPR_BASE) && expressaoNaoELei('dim=8') &&
    baseLei.ok === false && dimLei.ok === false &&
    !LEIS_CANON.some((l) => l.id === 'X' || l.id === 'e_k' || l.id === 'base') &&
    !cat.some((l) => l.id === 'X' || l.id === 'e_k') &&
    u.filhos.length === 5 &&
    /cadeia=vizdobra>X>base>afim>vinco/.test(u.nota) &&
    u.slots.grafo === 'realizado' &&
    /estrutura != lei/.test(u.proibicao) &&
    cat.length === 5)
}

{
  const G = grafoDependencia()
  const L = registroLeitura()
  const cL = classificaObjecto(LEITURA_COORD)
  const cX = classificaObjecto(ESPACO)
  const infla = admiteLei({ expressao: EXPR_LEITURA, nEvidencias: 99 })
  const u = leiParaU(cat)
  ok('§K14 leitura fora da cadeia; relacao nao promove; ontologia 5',
    ONTOLOGIA.join('>') === 'lei>gerador>espaco>realizacao>leitura' &&
    PAPEIS.every((p) => ONTOLOGIA.includes(p)) &&
    temCampos(ESPACO, 'espaco') && temCampos(LEITURA_COORD, 'leitura') &&
    LEITURA_COORD.cadeia === false && !CADEIA.includes('coord-byte') &&
    ARESTAS.every((a) => a.rel !== 'le') &&
    ARESTAS_LEITURA.length === 1 && ARESTAS_LEITURA[0].rel === 'le' &&
    G.leituras.length === 1 && G.promove === false &&
    L.papel === 'leitura' && L.origem === 'X' && L.observado === 'byte' &&
    L.lei_gk === 'N/A' && L.cadeia === false &&
    cL.papel === 'leitura' && cL.conhecida === false &&
    cX.papel === 'espaco' && cX.conhecida === false &&
    expressaoNaoELei(EXPR_LEITURA) && infla.ok === false &&
    coord(ek(3), 3) === 1 &&
    !cat.some((l) => l.id === 'coord-byte') &&
    u.slots.leitura === 'realizado' &&
    /leitura != cadeia/.test(u.proibicao) &&
    /relacao nao promove/.test(u.proibicao) &&
    /ontologia=lei>gerador>espaco>realizacao>leitura/.test(u.nota) &&
    cat.length === 5)
}

{
  const X = estruturaX()
  const ekLei = admiteLei('e_k=2^k')
  const dimLei = admiteLei('dim X = 8')
  const gramLei = admiteLei('Gram = Id')
  const byteLei = admiteLei('coord-byte')
  const u = leiParaU(cat)
  ok('§K15 vizdobra gera X; base binaria; Gram=Id; nao leis',
    GERADOR_VIZDOBRA.objeto === EXPR_GRUPO &&
    GERADOR_VIZDOBRA.estados === 8 &&
    X.objeto === EXPR_GRUPO && X.estados === 8 && X.elementos === 3 &&
    X.gera === true && X.dim === 8 && X.base === EXPR_BASE &&
    X.gram === 'Id' && X.gramId === true && X.oitoDeTres === true && X.bits === true &&
    X.lei_gk === 'N/A' && X.f8 === false &&
    ESPACO.dim === 8 && ESPACO.base === 'e_k=2^k' && ESPACO.gram === 'Id' &&
    temCampos(ESPACO, 'espaco') && gramOrtonormal() && grupoGeradores8() &&
    SIGMAS.every((s) => sigmaI(s.i, 0) === ek(s.bit)) &&
    expressaoNaoELei('e_k=2^k') && expressaoNaoELei('dim X = 8') &&
    expressaoNaoELei('Gram = Id') && expressaoNaoELei('coord-byte') &&
    ekLei.ok === false && dimLei.ok === false &&
    gramLei.ok === false && byteLei.ok === false &&
    admiteLei({ id: 'coord-byte', nEvidencias: 99 }).ok === false &&
    !LEIS_CANON.some((l) => l.id === 'e_k' || l.id === 'X' || l.id === 'coord-byte') &&
    !cat.some((l) => l.id === 'e_k' || l.id === 'X' || l.id === 'coord-byte') &&
    u.filhos.length === 5 &&
    /estrutura=vizdobra>X/.test(u.nota) &&
    /construcao != lei/.test(u.proibicao) &&
    /Gram != lei/.test(u.proibicao) &&
    cat.length === 5)
}

{
  const ml = medirELer()
  const base2 = alojarTeorema({ id: 'fis:thm:base', papel: 'leitura' })
  const gram = alojarTeorema({ papel: 'espaco' })
  const gera = alojarTeorema({ rel: 'gera' })
  const le = alojarTeorema({ rel: 'le' })
  const metrica = alojarTeorema({ papel: 'metrica' })
  const reua = alojarTeorema({ papel: 'regua' })
  const rotor = alojarTeorema({ papel: 'rotor' })
  const tipo = alojarTeorema({ papel: 'teorema' })
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K16 teorema != tipo; fis:thm:base(2) aterra em leitura',
    ONTOLOGIA.length === 5 &&
    FUNIL_TEOREMA.join('>') === 'papel>relacao' &&
    ESPACO.coord === EXPR_LEITURA && LEITURA_COORD.objeto === EXPR_LEITURA &&
    ml.identico === true && ml.papel === 'leitura' && ml.tipoNovo === false && ml.f8 === false &&
    base2.alojado === true && base2.papel === 'leitura' && base2.tipoNovo === false &&
    gram.papel === 'espaco' && gram.tipoNovo === false &&
    gera.relacao === 'gera' && gera.papel === '' && gera.tipoNovo === false &&
    le.relacao === 'le' && le.alojado === true &&
    metrica.alojado === false && metrica.tipoNovo === false &&
    reua.alojado === false && rotor.alojado === false && tipo.alojado === false &&
    tipo.tipoNovo === false && tipo.f8 === false &&
    !kinds.includes('teorema') && !kinds.includes('metrica') &&
    !kinds.includes('lei') && !kinds.includes('espaco') && !kinds.includes('leitura') &&
    GERADORES.length === 1 && cat.length === 5 &&
    admiteLei(EXPR_LEITURA).ok === false &&
    /funilTeorema=papel>relacao/.test(u.nota) &&
    /teorema != tipo/.test(u.proibicao) &&
    /teorema != tipo/.test(man.corpos?.motor?.nucleo?.lei_gk || ''))
}

{
  const fisTx = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  const catTx = readFileSync(join(RAIZ, 'catalogo.tex'), 'utf8')
  const rotorA = alojarTeorema({ id: 'rotor', origem: 'fis:thm:fecho' })
  const dtcA = alojarTeorema({ id: 'dtc' })
  const lei8 = alojarTeorema({ id: 'lei-8', ausente: true })
  const rReg = registroCandidato('rotor')
  const dReg = registroCandidato('dtc')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K17 desconhecido != ausente != novo tipo; dtc != rotor != corpo-em',
    ESTATUTO_ALOJAMENTO.join('>') === 'alojado>desconhecido>ausente' &&
    ESTATUTO_ALOJAMENTO[0] !== ESTATUTO_ALOJAMENTO[1] &&
    ESTATUTO_ALOJAMENTO[1] !== ESTATUTO_ALOJAMENTO[2] &&
    CANDIDATOS.length === 2 &&
    CANDIDATO_ROTOR.estatuto === 'desconhecido' && CANDIDATO_ROTOR.tipoNovo === false &&
    CANDIDATO_DTC.estatuto === 'desconhecido' && CANDIDATO_DTC.observa === 'relogio' &&
    CANDIDATO_DTC.em === 'N/A' &&
    CANDIDATO_ROTOR.nao.includes('dtc') && CANDIDATO_DTC.nao.includes('rotor') &&
    CANDIDATO_DTC.nao.includes('entrada-catalogo') && CANDIDATO_DTC.nao.includes('corpo-em') &&
    CANDIDATO_DTC.nao.includes('fis:thm:fecho') &&
    rotorA.estatuto === 'desconhecido' && rotorA.alojado === false && rotorA.tipoNovo === false &&
    dtcA.estatuto === 'desconhecido' && dtcA.alojado === false && dtcA.tipoNovo === false &&
    lei8.estatuto === 'ausente' && lei8.tipoNovo === false && lei8.estatuto !== rotorA.estatuto &&
    rReg.estatuto === 'desconhecido' && dReg.observa === 'relogio' && dReg.em === 'N/A' &&
    admiteLei('rotor').ok === false && admiteLei('dtc').ok === false &&
    !REALIZACOES.some((r) => r.id === 'dtc' || r.id === 'rotor') &&
    !LEIS_CANON.some((l) => l.id === 'dtc' || l.id === 'rotor') &&
    !kinds.includes('rotor') && !kinds.includes('dtc') && !kinds.includes('em') &&
    !/banco_relogio_u/.test(ponte) &&
    fisTx.includes('\\label{fis:thm:fecho}') &&
    fisTx.includes('\\label{fis:def:em-forma}') &&
    catTx.includes('\\label{cat:bloco4}') &&
    /inversor n[aã]o [eé] uma entrada/i.test(catTx) &&
    /DTC multinivel n[aã]o guarda nada/i.test(catTx.replace(/í/g, 'i')) &&
    /forma ainda n[aã]o corpo/i.test(catTx) &&
    GERADORES.length === 1 && REALIZACOES.length === 2 && cat.length === 5 &&
    /desconhecido != ausente != novo tipo/.test(u.proibicao) &&
    /dtc != rotor/.test(u.proibicao) &&
    /desconhecido != ausente != novo tipo/.test(man.corpos?.motor?.nucleo?.lei_gk || ''))
}

{
  const fisTx = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  const F = buscaFecho()
  const rReg = registroCandidato('rotor')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K18 fis:thm:fecho: endereco em X; nenhuma relacao; permanece desconhecido',
    FUNIL_BUSCA.join('>') === 'objecto>relacao' &&
    HOSPEDES.join('>') === 'vizdobra>X>base>afim>vinco>coord-byte' &&
    F.funil === FUNIL_BUSCA &&
    F.id === 'rotor-fecho' && F.expressao === EXPR_ROTOR && F.origem === 'fis:thm:fecho' &&
    F.hospede === 'X' && F.endereco === '(e_i,e_j)' && HOSPEDES.includes(F.hospede) &&
    F.papel === '' && F.relacao === '' &&
    F.nPlanos === 3 && F.ordemPaper === 4 && F.ordemEmB === 2 && F.ordemEmB !== F.ordemPaper &&
    F.rels.length === RELS.length * HOSPEDES.length + RELS_LEITURA.length * HOSPEDES.length &&
    F.rels.every((a) => a.ok === false && a.de === 'rotor-fecho' && HOSPEDES.includes(a.para)) &&
    RELS.concat(RELS_LEITURA).every((rel) => F.rels.some((a) => a.rel === rel)) &&
    F.alojado === false && F.estatuto === 'desconhecido' && F.tipoNovo === false &&
    F.lei_gk === 'N/A' && F.gerador === 'N/A' && F.kind === '' && F.f8 === false &&
    CANDIDATO_ROTOR.expressao === EXPR_ROTOR && CANDIDATO_ROTOR.hospede === 'X' &&
    CANDIDATO_ROTOR.relacao === '' && CANDIDATO_ROTOR.estatuto === 'desconhecido' &&
    CANDIDATO_ROTOR.nao.includes('gerador') &&
    rReg.endereco === '(e_i,e_j)' && rReg.estatuto === 'desconhecido' &&
    ARESTAS.every((a) => a.de !== 'rotor-fecho' && a.para !== 'rotor-fecho') &&
    GERADORES.length === 1 && GERADOR_VIZDOBRA.nao.includes('rotor') &&
    !LEIS_CANON.some((l) => l.id === 'rotor' || l.expressao === EXPR_ROTOR) &&
    admiteLei(EXPR_ROTOR).ok === false && admiteLei('rotor-fecho').ok === false &&
    expressaoNaoELei(EXPR_ROTOR) &&
    !kinds.includes('rotor') &&
    CANDIDATO_DTC.observa === 'relogio' && CANDIDATO_DTC.em === 'N/A' &&
    fisTx.includes('\\label{fis:thm:fecho}') &&
    fisTx.includes('\\label{fis:thm:quarteto}') &&
    /funilBusca=objecto>relacao/.test(u.nota) &&
    /R_ij != gera/.test(u.proibicao) &&
    /fecho != gerador/.test(u.proibicao) &&
    cat.length === 5)
}

{
  const F = buscaFecho()
  const cruzF = cruzarIdentidade(F)
  const cruzG = cruzarIdentidade({
    hospede: 'X',
    relacao: 'gera',
    ordemPaper: 2,
    ordemEmB: 2,
  })
  const ausente = alojarTeorema({ id: 'lei-8', ausente: true })
  const u = leiParaU(cat)
  ok('§K19 localizado e incompativel => desconhecido; identidade expressao>suporte>realizacao',
    CAMPOS_IDENTIDADE.join('>') === 'expressao>suporte>realizacao' &&
    F.hospede === 'X' && F.ordemPaper === 4 && F.ordemEmB === 2 &&
    cruzF.localizado === true && cruzF.incompativel === true &&
    cruzF.compativel === false && cruzF.estatuto === 'desconhecido' &&
    cruzF.tipoNovo === false && cruzF.estatuto !== 'ausente' &&
    cruzF.estatuto !== ausente.estatuto &&
    cruzG.localizado === true && cruzG.incompativel === false &&
    cruzG.compativel === true && cruzG.estatuto === 'alojado' &&
    cruzG.tipoNovo === false &&
    sigmaI(1, sigmaI(1, 5)) === 5 &&
    ARESTAS.some((a) => a.de === 'vizdobra' && a.rel === 'gera' && a.para === 'X') &&
    GERADORES.length === 1 && cat.length === 5 &&
    /identidade=expressao>suporte>realizacao/.test(u.nota) &&
    /localizado e incompativel => desconhecido/.test(u.proibicao) &&
    /localizado e incompativel => desconhecido/.test(man.corpos?.motor?.nucleo?.lei_gk || ''))
}

{
  const b3 = baseClausula3()
  const reua = alojarTeorema({ papel: 'regua' })
  const pesoLei = admiteLei(EXPR_PESO)
  const kinds = schema.properties.kind.enum
  const fisTx = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  const u = leiParaU(cat)
  ok('§K20 fis:thm:base(3) peso aterra em leitura; somam != composem; reua != peso',
    b3.origem === 'fis:thm:base' && b3.clausula === 3 &&
    b3.expressao === EXPR_PESO && b3.suporte === 'X' && b3.realizacao === 'B' &&
    b3.somaId === true && b3.leitura === true && b3.cega === true && b3.naoCompoe === true &&
    b3.identico === true &&
    pesoK(7, 0) === 1 && pesoK(7, 1) === 2 && pesoK(7, 2) === 4 && pesoK(7, 3) === 0 &&
    pesoK(7, 0) === coord(7, 0) * ek(0) &&
    b3.papel === 'leitura' && b3.relacao === 'le' &&
    b3.cruz.estatuto === 'alojado' && b3.estatuto === 'alojado' &&
    b3.tipoNovo === false && b3.lei_gk === 'N/A' &&
    LEITURAS.length === 1 && LEITURAS[0].id === 'coord-byte' &&
    reua.alojado === false && reua.tipoNovo === false &&
    CAMPOS_DECL.includes('regua') && !ONTOLOGIA.includes('regua') &&
    pesoLei.ok === false && expressaoNaoELei(EXPR_PESO) &&
    !kinds.includes('regua') && !kinds.includes('peso') &&
    !LEIS_CANON.some((l) => l.id === 'peso' || l.expressao === EXPR_PESO) &&
    fisTx.includes('\\label{fis:thm:base}') &&
    /peso != reua do card/.test(u.proibicao) &&
    cat.length === 5)
}

{
  const D = dualParidade()
  const kinds = schema.properties.kind.enum
  const fisTx = readFileSync(join(RAIZ, 'fisica.tex'), 'utf8')
  const u = leiParaU(cat)
  ok('§K21 fis:def:dual paridade aterra em leitura; chi pede Zc; != dual-smin',
    D.origem === 'fis:def:dual' && D.tipoNovo === false && D.f8 === false &&
    D.par.expressao === EXPR_PAR && D.par.suporte === 'X' && D.par.realizacao === 'B' &&
    D.par.gram === true && D.par.linear === true && D.par.identico === true &&
    D.par.papel === 'leitura' && D.par.relacao === 'le' && D.par.estatuto === 'alojado' &&
    D.chi.expressao === EXPR_CHI && D.chi.realizacao === 'Zc' &&
    D.chi.cruz.localizado === true && D.chi.cruz.incompativel === true &&
    D.chi.estatuto === 'desconhecido' && D.chi.cruz.tipoNovo === false &&
    paridadeKJ(ek(0), ek(0)) === 1 && paridadeKJ(ek(0), ek(1)) === 0 &&
    paridadeKJ(7, 1) === 1 &&
    gramOrtonormal() === true &&
    LEITURAS.length === 1 && !HOSPEDES.includes('V') &&
    D.nao.includes('dual-smin') && D.nao.includes('Duo') && D.nao.includes('V') &&
    ID_DUAL_SMIN === 'dual-smin' &&
    !LEIS_CANON.some((l) => l.expressao === EXPR_CHI || l.expressao === EXPR_PAR) &&
    admiteLei(EXPR_CHI).ok === false && admiteLei(EXPR_PAR).ok === false &&
    !kinds.includes('dual') && !kinds.includes('caractere') && !kinds.includes('V') &&
    !/banco_transf_u/.test(ponte) &&
    fisTx.includes('\\label{fis:def:dual}') &&
    fisTx.includes('\\label{fis:lem:orto}') &&
    /paridade != chi != dual-smin/.test(u.proibicao) &&
    cat.length === 5)
}

{
  const base = destinosDoTeorema('fis:thm:base')
  const dual = destinosDoTeorema('fis:def:dual')
  const fecho = destinosDoTeorema('fis:thm:fecho')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K22 teorema != objecto; destino por objecto; dual ramifica',
    FUNIL_DESTINO.join('>') === 'teorema>objecto>identidade>destino' &&
    base.length === 3 && base.every((d) => d.estatuto === 'alojado' && d.tipoNovo === false) &&
    base.filter((d) => d.papel === 'leitura').length === 2 &&
    base.some((d) => d.papel === 'espaco') &&
    dual.length === 2 &&
    dual[0].objecto === 'paridade' && dual[0].estatuto === 'alojado' && dual[0].papel === 'leitura' &&
    dual[1].objecto === 'chi' && dual[1].estatuto === 'desconhecido' && dual[1].papel === '' &&
    dual[0].estatuto !== dual[1].estatuto &&
    fecho.length === 1 && fecho[0].objecto === 'R_ij' && fecho[0].estatuto === 'desconhecido' &&
    dual.every((d) => d.tipoNovo === false) && fecho.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && REALIZACOES.length === 2 &&
    destinosDoTeorema('fis:thm:duasinvol').length === 0 &&
    !kinds.includes('destino') && !kinds.includes('objecto') &&
    /funilDestino=teorema>objecto>identidade>destino/.test(u.nota) &&
    /teorema != objecto/.test(u.proibicao) &&
    /teorema != objecto/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const F = cruzarIdentidade(buscaFecho())
  const chi = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'Zc', realizacaoBanco: 'B',
  })
  const par = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const dual = destinosDoTeorema('fis:def:dual')
  const fecho = destinosDoTeorema('fis:thm:fecho')
  const base = destinosDoTeorema('fis:thm:base')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K23 mesma expr != mesmo objecto; suporte != realizacao; dois desconhecidos',
    CAMPOS_ALOJAR.join('>') === 'objecto>expressao>suporte>realizacao>relacao' &&
    MOTIVO_DESCONHECIDO.join('>') === 'estrutural>algebrica' &&
    F.motivo === 'estrutural' && F.estatuto === 'desconhecido' &&
    chi.motivo === 'algebrica' && chi.estatuto === 'desconhecido' &&
    F.motivo !== chi.motivo &&
    par.estatuto === 'alojado' && par.motivo === '' &&
    par.localizado === chi.localizado && par.compativel !== chi.compativel &&
    fecho[0].motivo === 'estrutural' && dual[1].motivo === 'algebrica' &&
    dual[0].papel === 'leitura' &&
    base.filter((d) => d.papel === 'leitura').length === 2 &&
    LEITURAS.length === 1 && LEITURAS[0].id === 'coord-byte' &&
    !kinds.includes('coordenada') &&
    /alojar=objecto>expressao>suporte>realizacao>relacao/.test(u.nota) &&
    /mesma expressao notRightarrow mesmo objecto/.test(u.proibicao) &&
    /mesmo suporte notRightarrow mesma realizacao/.test(u.proibicao) &&
    /desconhecido estrutural != algebrica/.test(u.proibicao) &&
    /desconhecido estrutural != algebrica/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const Q = quartetoN2()
  const dest = destinosDoTeorema('fis:thm:quarteto')
  const F = buscaFecho()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K24 quarteto aterra em vizdobra; reconhecimento != criacao; != rotor',
    ALOJAR_E === 'reconhecimento' &&
    Q.origem === 'fis:thm:quarteto' && Q.objecto === 'Q_ab' &&
    Q.expressao === EXPR_QUARTETO && Q.n === 4 && Q.ordem === 2 &&
    Q.suporte === 'X' && Q.realizacao === 'B' &&
    Q.identico === true && Q.distintos && Q.involutivos && Q.composicao && Q.comutam &&
    Q.papel === 'gerador' && Q.relacao === 'gera' &&
    Q.casa === 'vizdobra' && Q.casa === GERADOR_VIZDOBRA.id &&
    Q.cria === false && Q.tipoNovo === false && Q.estatuto === 'alojado' &&
    Q.lei_gk === 'N/A' && Q.f8 === false &&
    Q.nao.includes('rotor') && Q.nao.includes('Duo') &&
    qAb(1, 0, 0) === ek(0) && qAb(0, 1, 0) === ek(1) &&
    qAb(1, 1, 0) === (ek(0) ^ ek(1)) &&
    dest.length === 1 && dest[0].objecto === 'Q_ab' &&
    dest[0].papel === 'gerador' && dest[0].estatuto === 'alojado' &&
    dest[0].relacao === 'gera' && dest[0].tipoNovo === false &&
    GERADORES.length === 1 && REALIZACOES.length === 2 && LEITURAS.length === 1 &&
    F.estatuto === 'desconhecido' && F.ordemPaper === 4 && F.ordemEmB === 2 &&
    Q.ordem !== F.ordemPaper && Q.estatuto !== F.estatuto &&
    admiteLei(EXPR_QUARTETO).ok === false && expressaoNaoELei(EXPR_QUARTETO) &&
    !kinds.includes('quarteto') && !kinds.includes('Q_ab') &&
    /alojarE=reconhecimento/.test(u.nota) &&
    /alojamento = reconhecimento/.test(u.proibicao) &&
    /Q_ab != R_ij/.test(u.proibicao) &&
    /quarteto != rotor/.test(u.proibicao) &&
    /alojamento = reconhecimento/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const Q = quartetoN2()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K25 subestrutura reconhecida != novo objecto; corte n=2; 4 estados != 4 geradores',
    REGRA_SUB === 'subestrutura reconhecida != novo objecto' &&
    Q.corte === 2 && Q.estados === 4 && Q.estados === 2 ** Q.corte &&
    Q.construcao === EXPR_QAB && Q.rotulos === ROTULOS_QAB &&
    Q.rotulos.join('>') === '00>01>10>11' &&
    qAb(0, 0, 0) === 0 &&
    qAb(1, 0, 0) === ek(0) &&
    qAb(0, 1, 0) === ek(1) &&
    qAb(1, 1, 0) === (ek(0) ^ ek(1)) &&
    GERADOR_VIZDOBRA.estados === 8 && GERADOR_VIZDOBRA.n === 3 &&
    Q.estados !== GERADOR_VIZDOBRA.estados &&
    Q.casa === GERADOR_VIZDOBRA.id &&
    GERADORES.length === 1 && Q.cria === false && Q.tipoNovo === false &&
    CAMPOS_GERADOR.join('>') === 'objeto>estados>origem' &&
    !CAMPOS_GERADOR.includes('corte') &&
    ONTOLOGIA.length === 5 && !ONTOLOGIA.includes('corte') &&
    !kinds.includes('subestrutura') && !kinds.includes('corte') &&
    admiteLei(EXPR_QAB).ok === false && expressaoNaoELei(EXPR_QAB) &&
    /regraSub=subestrutura reconhecida != novo objecto/.test(u.nota) &&
    /subestrutura reconhecida != novo objecto/.test(u.proibicao) &&
    /quatro estados != quatro geradores/.test(u.proibicao) &&
    /subestrutura reconhecida != novo objecto/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const el = grauNaConstrucao(SIGMAS[0].objeto)
  const sub = grauNaConstrucao('Q_ab')
  const cons = grauNaConstrucao('vizdobra')
  const rotor = grauNaConstrucao(EXPR_ROTOR)
  const F = buscaFecho()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K26 elemento != subestrutura subset construcao; schema so vizdobra; R_ij fora',
    GRAU_NA.join('>') === 'elemento>subestrutura>construcao' &&
    REGRA_GRAU === 'elemento/subestrutura subset construcao != novo objecto' &&
    el.grau === 'elemento' && el.casa === 'vizdobra' && el.tipoNovo === false &&
    sub.grau === 'subestrutura' && sub.casa === 'vizdobra' && sub.corte === 2 &&
    cons.grau === 'construcao' && cons.casa === 'vizdobra' &&
    el.grau !== sub.grau && sub.grau !== cons.grau &&
    SIGMAS.every((s) => grauNaConstrucao(s.objeto).grau === 'elemento') &&
    GERADORES.length === 1 && GERADORES[0].id === 'vizdobra' &&
    !GERADORES.some((g) => g.id === 'Q' || g.id === 'n2' || g.id === 'Q_ab') &&
    rotor.grau === '' && rotor.casa === '' &&
    F.estatuto === 'desconhecido' && F.ordemPaper !== quartetoN2().ordem &&
    ONTOLOGIA.length === 5 && GRAU_NA.every((g) => !ONTOLOGIA.includes(g)) &&
    !kinds.includes('elemento') && !kinds.includes('subestrutura') &&
    !kinds.includes('construcao') && !kinds.includes('corte') && !kinds.includes('n2') &&
    /regraGrau=elemento\/subestrutura subset construcao != novo objecto/.test(u.nota) &&
    /elemento\/subestrutura subset construcao != novo objecto/.test(u.proibicao) &&
    /elemento\/subestrutura subset construcao != novo objecto/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const v = pertenceA('vizdobra')
  const s = pertenceA(SIGMAS[0].objeto)
  const q = pertenceA('Q_ab')
  const n2 = pertenceA('n=2')
  const r = pertenceA(EXPR_ROTOR)
  const F = buscaFecho()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K27 propriedade interna != ontologia; localizado em X != pertence a vizdobra',
    PROPS_INTERNAS.join('>') === 'grau>corte>cardinalidade' &&
    REGRA_PROP === 'propriedade interna notRightarrow entrada em ONTOLOGIA' &&
    PROPS_INTERNAS.every((p) => !ONTOLOGIA.includes(p)) &&
    !CAMPOS_GERADOR.includes('grau') && !CAMPOS_GERADOR.includes('corte') &&
    !CAMPOS_GERADOR.includes('cardinalidade') &&
    v.onde === 'ontologia' && v.papel === 'gerador' && v.casa === 'vizdobra' &&
    s.onde === 'construcao' && s.papel === '' && s.casa === 'vizdobra' &&
    q.onde === 'construcao' && q.casa === 'vizdobra' &&
    n2.onde === 'parametro' && n2.casa === 'vizdobra' &&
    v.onde !== s.onde && s.onde === q.onde && n2.onde !== v.onde &&
    r.onde === '' && r.casa === '' &&
    F.hospede === 'X' && F.estatuto === 'desconhecido' &&
    HOSPEDES.includes('X') && r.casa !== 'vizdobra' &&
    ONTOLOGIA.join('>') === 'lei>gerador>espaco>realizacao>leitura' &&
    GERADORES.length === 1 && GERADORES[0].id === 'vizdobra' &&
    !kinds.includes('grau') && !kinds.includes('corte') &&
    !kinds.includes('cardinalidade') && !kinds.includes('parametro') &&
    /regraProp=propriedade interna notRightarrow entrada em ONTOLOGIA/.test(u.nota) &&
    /propriedade interna notRightarrow entrada em ONTOLOGIA/.test(u.proibicao) &&
    /localizado em X notRightarrow pertence a vizdobra/.test(u.proibicao) &&
    /propriedade interna notRightarrow entrada em ONTOLOGIA/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const F = buscaFecho()
  const u = leiParaU(cat)
  ok('§K28 suporte != pertencimento; vive em X != gerado por vizdobra; schema intacto',
    REGRA_SUPORTE === 'suporte != pertencimento a construcao' &&
    REGRA_VIVE === 'vive em X notRightarrow gerado por vizdobra' &&
    viveEmX(EXPR_ROTOR) && !geradoPorVizdobra(EXPR_ROTOR) &&
    viveEmX('chi') && !geradoPorVizdobra('chi') &&
    F.hospede === 'X' && !geradoPorVizdobra(F.expressao) &&
    geradoPorVizdobra(ESPACO.id) &&
    SIGMAS.every((s) => geradoPorVizdobra(s.objeto) && viveEmX(s.objeto)) &&
    geradoPorVizdobra('Q_ab') && viveEmX('Q_ab') &&
    viveEmX('coord-byte') && !geradoPorVizdobra('coord-byte') &&
    viveEmX('afim') && !geradoPorVizdobra('afim') &&
    viveEmX('vinco') && !geradoPorVizdobra('vinco') &&
    !geradoPorVizdobra('vizdobra') &&
    ONTOLOGIA.length === 5 && GERADORES.length === 1 && GERADORES[0].id === 'vizdobra' &&
    LEIS_CANON.length === 5 && REALIZACOES.length === 2 && LEITURAS.length === 1 &&
    /regraSuporte=suporte != pertencimento a construcao/.test(u.nota) &&
    /regraVive=vive em X notRightarrow gerado por vizdobra/.test(u.nota) &&
    /suporte != pertencimento a construcao/.test(u.proibicao) &&
    /vive em X notRightarrow gerado por vizdobra/.test(u.proibicao) &&
    /vive em X notRightarrow gerado por vizdobra/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K29 ontologia fechada por contencao; funil papel existente ou desconhecido',
    ONTOLOGIA_FECHADA === true &&
    REGRA_ONTO === 'ontologia fechada por contencao' &&
    FUNIL_CASA.join('>') === 'localizar>suporte>expressao>realizacao>relacao>papel' &&
    ONTOLOGIA.join('>') === 'lei>gerador>espaco>realizacao>leitura' &&
    ONTOLOGIA.length === 5 &&
    CASAS.leis === LEIS_CANON.length &&
    CASAS.geradores === GERADORES.length &&
    CASAS.espaco === 1 && ESPACO.id === 'X' &&
    CASAS.realizacoes === REALIZACOES.length &&
    CASAS.leituras === LEITURAS.length &&
    !kinds.includes('V') && !kinds.includes('modulo') && !kinds.includes('andar') &&
    /funilCasa=localizar>suporte>expressao>realizacao>relacao>papel/.test(u.nota) &&
    /ontologia fechada por contencao/.test(u.proibicao) &&
    /descoberta notRightarrow expande ONTOLOGIA/.test(u.proibicao) &&
    /observado != construcao/.test(u.proibicao) &&
    /ontologia fechada por contencao/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const D = doisAndares()
  const dest = destinosDoTeorema('fis:def:doisandares')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K30 fis:def:doisandares: X aterra; V=Zc^X desconhecido; != ausente; != X',
    D.origem === 'fis:def:doisandares' && D.funil === FUNIL_CASA &&
    D.baixo.objecto === 'X' && D.baixo.papel === 'espaco' &&
    D.baixo.papelExiste === true && D.baixo.estatuto === 'alojado' &&
    D.baixo.casa === ESPACO.id && D.baixo.realizacao === 'B' &&
    D.cima.objecto === 'V' && D.cima.expressao === EXPR_V &&
    D.cima.realizacao === 'Zc' && D.cima.papel === '' &&
    D.cima.papelExiste === false && D.cima.estatuto === 'desconhecido' &&
    D.cima.motivo === 'algebrica' && D.cima.estatuto !== 'ausente' &&
    D.cima.tipoNovo === false && D.tipoNovo === false && D.f8 === false &&
    D.cima.nao.includes('X') && D.baixo.estatuto !== D.cima.estatuto &&
    dest.length === 2 && dest[0].objecto === 'X' && dest[0].estatuto === 'alojado' &&
    dest[1].objecto === 'V' && dest[1].estatuto === 'desconhecido' && dest[1].motivo === 'algebrica' &&
    !HOSPEDES.includes('V') && ESPACO.id !== 'V' &&
    admiteLei(EXPR_V).ok === false && expressaoNaoELei(EXPR_V) &&
    GERADORES.length === 1 && REALIZACOES.length === 2 && LEITURAS.length === 1 &&
    !kinds.includes('V') &&
    /V != X/.test(u.proibicao) &&
    /V != X/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const C = convRamos()
  const dest = destinosDoTeorema('fis:def:conv')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K31 fis:def:conv: xor aterra em X; * pede V; C_f/hat/H pendentes; nao cria espaco',
    ESPACO_UNICO === true && REGRA_V === 'objecto em V pede casa de V' &&
    PENDENTES_V.join('>') === 'conv>C_f>hat>H' &&
    C.origem === 'fis:def:conv' && C.funil === FUNIL_CASA &&
    C.xor.objecto === 'xor' && C.xor.suporte === 'X' && C.xor.realizacao === 'B' &&
    C.xor.estatuto === 'alojado' && C.xor.casa === 'X' && C.xor.cria === false &&
    C.xor.grupo && C.xor.bits && C.xor.tipoNovo === false &&
    xorX(ek(0), ek(1)) === (ek(0) ^ ek(1)) &&
    C.conv.objecto === '*' && C.conv.suporte === 'V' && C.conv.realizacao === 'Zc' &&
    C.conv.estatuto === 'desconhecido' && C.conv.motivo === 'algebrica' &&
    C.conv.cria === false && C.conv.tipoNovo === false &&
    doisAndares().cima.estatuto !== 'alojado' &&
    C.pendentes === PENDENTES_V &&
    dest.length === 2 && dest[0].objecto === 'xor' && dest[0].estatuto === 'alojado' &&
    dest[1].objecto === '*' && dest[1].estatuto === 'desconhecido' &&
    GERADORES.length === 1 && CASAS.espaco === 1 && !HOSPEDES.includes('V') &&
    admiteLei(EXPR_XOR).ok === false && admiteLei(EXPR_CONV).ok === false &&
    expressaoNaoELei(EXPR_XOR) && expressaoNaoELei(EXPR_CONV) &&
    !kinds.includes('conv') && !kinds.includes('xor') && !kinds.includes('anel') &&
    /regraV=objecto em V pede casa de V/.test(u.nota) &&
    /objecto em V pede casa de V/.test(u.proibicao) &&
    /xor != conv/.test(u.proibicao) &&
    /nao cria espaco/.test(u.proibicao) &&
    /objecto em V pede casa de V/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const R = ramosDoAndarV()
  const cf = destinosDoTeorema('fis:lem:comuta')
  const pr = destinosDoTeorema('fis:thm:proprios')
  const tr = destinosDoTeorema('fis:def:transf')
  const h = destinosDoTeorema('fis:thm:H')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K32 C_f != hat != H; cada um pede V; mesmo enunciado != mesmo objecto',
    REGRA_ENUN === 'mesmo enunciado notRightarrow mesmo objecto' &&
    R.distintos && R.cria === false && R.tipoNovo === false && R.f8 === false &&
    R.cf.objecto === 'C_f' && R.cf.expressao === EXPR_CF &&
    R.cf.suporte === 'V' && R.cf.realizacao === 'Zc' &&
    R.cf.estatuto === 'desconhecido' && R.cf.motivo === 'algebrica' &&
    R.cf.nao.includes('*') && R.cf.nao.includes('xor') &&
    R.hat.objecto === 'hat' && R.hat.expressao === EXPR_HAT &&
    R.hat.suporte === 'V' && R.hat.nao.includes('paridade') && R.hat.nao.includes('C_f') &&
    R.H.objecto === 'H' && R.H.expressao === EXPR_H &&
    R.H.fatorPaper === 256 && R.H.fatorEmB === 0 && R.H.incompFator === true &&
    R.H.nao.includes('hat') && R.H.estatuto === 'desconhecido' &&
    operadorCf().objecto !== hatF().objecto && hatF().objecto !== matrizH().objecto &&
    cf.length === 1 && cf[0].objecto === 'C_f' &&
    pr.length === 2 && pr[0].objecto === 'C_f' && pr[1].objecto === 'hat' &&
    tr.length === 2 && tr[0].objecto === 'hat' && tr[1].objecto === 'H' &&
    h.length === 1 && h[0].objecto === 'H' &&
    [cf, pr, tr, h].every((ds) => ds.every((d) => d.estatuto === 'desconhecido' && d.tipoNovo === false)) &&
    GERADORES.length === 1 && CASAS.espaco === 1 && !HOSPEDES.includes('V') &&
    admiteLei(EXPR_CF).ok === false && admiteLei(EXPR_HAT).ok === false && admiteLei(EXPR_H).ok === false &&
    !kinds.includes('C_f') && !kinds.includes('hat') && !kinds.includes('H') &&
    /regraEnun=mesmo enunciado notRightarrow mesmo objecto/.test(u.nota) &&
    /mesmo enunciado notRightarrow mesmo objecto/.test(u.proibicao) &&
    /C_f != hat != H/.test(u.proibicao) &&
    /mesmo enunciado notRightarrow mesmo objecto/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const D = dobraNorma()
  const dest = destinosDoTeorema('fis:thm:dobranorma')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K33 fis:thm:dobranorma: norma-V != paridade; 2P nao traduz para B; F(G) pede H',
    REGRA_FATOR === 'fator nao traduz para B' &&
    D.origem === 'fis:thm:dobranorma' && D.funil === FUNIL_CASA && D.distintos &&
    D.norma.objecto === 'norma-V' && D.norma.expressao === EXPR_NORMA_V &&
    D.norma.suporte === 'V' && D.norma.realizacao === 'Zc' &&
    D.norma.estatuto === 'desconhecido' && D.norma.nao.includes('paridade') &&
    D.doisP.objecto === '2P' && D.doisP.fatorPaper === 2 && D.doisP.fatorEmB === 0 &&
    D.doisP.incompFator === true && D.doisP.nao.includes('vinco') &&
    D.FG.objecto === 'F(G)' && D.FG.depende === 'H' && D.FG.estatuto === 'desconhecido' &&
    D.cria === false && D.tipoNovo === false && D.f8 === false &&
    dest.length === 3 && dest.every((d) => d.estatuto === 'desconhecido' && d.tipoNovo === false) &&
    dest[0].objecto === 'norma-V' && dest[1].objecto === '2P' && dest[2].objecto === 'F(G)' &&
    operadorCf().nao.includes('*') && hatF().nao.includes('paridade') &&
    matrizH().objecto !== hatF().objecto &&
    GERADORES.length === 1 && CASAS.espaco === 1 && !HOSPEDES.includes('V') &&
    admiteLei(EXPR_NORMA_V).ok === false && admiteLei(EXPR_2P).ok === false &&
    !kinds.includes('norma') && !kinds.includes('transformacao') &&
    !/banco_transf_u/.test(ponte) &&
    /regraFator=fator nao traduz para B/.test(u.nota) &&
    /C_f != \*/.test(u.proibicao) &&
    /hat != chi != paridade/.test(u.proibicao) &&
    /fator nao traduz para B/.test(u.proibicao) &&
    /fator nao traduz para B/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const D = dobraNorma()
  const H = matrizH()
  const u = leiParaU(cat)
  ok('§K34 2 != 0 em V/Zc nao autoriza 2mapsto0 em B; X em B nao contamina V',
    REGRA_DOIS === '2 != 0 em V/Zc nao autoriza 2mapsto0 em B' &&
    D.doisP.fatorPaper === 2 && D.doisP.fatorEmB === 0 && D.doisP.incompFator &&
    H.fatorPaper === 256 && H.fatorEmB === 0 && H.incompFator &&
    D.doisP.fatorEmB === H.fatorEmB && D.doisP.fatorPaper !== H.fatorPaper &&
    D.norma.estatuto === 'desconhecido' && D.doisP.estatuto === 'desconhecido' &&
    D.FG.estatuto === 'desconhecido' &&
    ESPACO.id === 'X' && !HOSPEDES.includes('V') && CASAS.espaco === 1 &&
    GERADORES.length === 1 && LEIS_CANON.length === 5 &&
    /regraDois=2 != 0 em V\/Zc nao autoriza 2mapsto0 em B/.test(u.nota) &&
    /2 != 0 em V\/Zc nao autoriza 2mapsto0 em B/.test(u.proibicao) &&
    /2 != 0 em V\/Zc nao autoriza 2mapsto0 em B/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const M = modosRamos()
  const dest = destinosDoTeorema('fis:cor:modos')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K35 fis:cor:modos: hamming aterra; alpha/A pedem V; 2_V != 2_B; simbolo != operacao',
    REGRA_SIM === 'simbolo igual notRightarrow operacao igual' &&
    REGRA_2VB === '2_V != 2_B' &&
    FUNIL_CASA.join('>') === 'localizar>suporte>expressao>realizacao>relacao>papel' &&
    pesoHamming(0) === 0 && pesoHamming(255) === 8 &&
    pesoHamming(ek(0) ^ ek(1)) === 2 &&
    M.origem === 'fis:cor:modos' &&
    M.hamming.objecto === 'hamming' && M.hamming.papel === 'leitura' &&
    M.hamming.casa === 'coord-byte' && M.hamming.estatuto === 'alojado' &&
    M.hamming.cria === false && M.hamming.tipoNovo === false &&
    M.alpha.objecto === 'alpha' && M.alpha.suporte === 'V' && M.alpha.estatuto === 'desconhecido' &&
    M.A.objecto === 'A' && M.A.nao.includes('C_f') && M.A.estatuto === 'desconhecido' &&
    M.auto.fatorPaper === 2 && M.auto.fatorEmB === 0 && M.auto.incompFator &&
    M.auto.nao.includes('2_B') &&
    dest.length === 3 && dest[0].objecto === 'hamming' && dest[0].estatuto === 'alojado' &&
    dest[0].papel === 'leitura' && dest[1].objecto === 'alpha' && dest[2].objecto === 'A' &&
    dest[1].estatuto === 'desconhecido' && dest[2].estatuto === 'desconhecido' &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    admiteLei(EXPR_HAM).ok === false && admiteLei(EXPR_A).ok === false &&
    !kinds.includes('hamming') && !kinds.includes('adjacencia') &&
    /regraSim=simbolo igual notRightarrow operacao igual/.test(u.nota) &&
    /regra2VB=2_V != 2_B/.test(u.nota) &&
    /simbolo igual notRightarrow operacao igual/.test(u.proibicao) &&
    /2_V != 2_B/.test(u.proibicao) &&
    /2_V != 2_B/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const C = convTeorema()
  const dest = destinosDoTeorema('fis:thm:conv')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K36 fis:thm:conv: produto/inj/inv pedem V; injetivo != invertivel; fase aterra',
    REGRA_INJ === 'injetivo != invertivel' &&
    REGRA_CASA_LE === 'outra leitura do mesmo suporte nao cria casa' &&
    C.origem === 'fis:thm:conv' && C.funil === FUNIL_CASA && C.distintos &&
    C.prod.objecto === 'produto' && C.prod.expressao === EXPR_FG &&
    C.prod.suporte === 'V' && C.prod.realizacao === 'Zc' &&
    C.prod.estatuto === 'desconhecido' && C.prod.nao.includes('xor') && C.prod.nao.includes('*') &&
    C.inj.objecto === 'C_f' && C.inj.expressao === EXPR_INJ &&
    C.inj.pergunta === 'injetivo' && C.inj.nao.includes('invertivel') &&
    C.inv.objecto === 'C_f' && C.inv.expressao === EXPR_INV &&
    C.inv.pergunta === 'invertivel' && C.inv.nao.includes('injetivo') &&
    C.inv.testemunha === 'C_d0+d0' &&
    C.inj.objecto === C.inv.objecto && C.inj.expressao !== C.inv.expressao &&
    C.fase.objecto === 'paridade' && C.fase.casa === 'coord-byte' &&
    C.fase.estatuto === 'alojado' && C.fase.cria === false && C.fase.tipoNovo === false &&
    C.cria === false && C.tipoNovo === false && C.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'produto' && dest[1].objecto === 'C_f' &&
    dest.every((d) => d.estatuto === 'desconhecido' && d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    !HOSPEDES.includes('V') &&
    admiteLei(EXPR_FG).ok === false && admiteLei(EXPR_INJ).ok === false &&
    admiteLei(EXPR_INV).ok === false &&
    !kinds.includes('produto') && !kinds.includes('injetivo') && !kinds.includes('invertivel') &&
    !/banco_transf_u/.test(ponte) &&
    /regraInj=injetivo != invertivel/.test(u.nota) &&
    /regraCasaLe=outra leitura do mesmo suporte nao cria casa/.test(u.nota) &&
    /injetivo != invertivel/.test(u.proibicao) &&
    /outra leitura do mesmo suporte nao cria casa/.test(u.proibicao) &&
    /injetivo != invertivel/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const T = topoTese()
  const dest = destinosDoTeorema('fis:topo-tese')
  const destV = destinosDoTeorema('fis:def:viztopo')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K37 fis:topo-tese: oficio=leitura; V_E aterra; esquecer E nao cria; sem 6a categoria',
    REGRA_OFICIO === 'oficio != categoria' &&
    REGRA_PARTE === 'parte nao acrescenta objectos' &&
    REGRA_FG === 'xor != * != F(f)F(g)' &&
    ONTOLOGIA.length === 5 && !ONTOLOGIA.includes('oficio') && !ONTOLOGIA.includes('grafo') &&
    T.origem === 'fis:topo-tese' && T.funil === FUNIL_CASA && T.distintos &&
    T.oficio.objecto === 'enderecar' && T.oficio.papel === 'leitura' &&
    T.oficio.papelExiste === true && T.oficio.casa === 'coord-byte' &&
    T.oficio.estatuto === 'alojado' && T.oficio.cria === false &&
    T.oficio.nao.includes('categoria') && T.oficio.nao.includes('realizacao') &&
    T.VE.objecto === 'V_E' && T.VE.casa === 'vizdobra' && T.VE.estatuto === 'alojado' &&
    T.VE.nao.includes('V_topo') && T.VE.cria === false &&
    T.Vtopo.objecto === 'V_topo' && T.Vtopo.papel === 'leitura' &&
    T.Vtopo.casa === 'coord-byte' && T.Vtopo.estatuto === 'alojado' &&
    T.Vtopo.cria === false && T.Vtopo.nao.includes('grafo') && T.Vtopo.nao.includes('V_E') &&
    T.cega === 'medida' && T.cria === false && T.tipoNovo === false && T.f8 === false &&
    dest.length === 3 && dest[0].objecto === 'enderecar' && dest[0].papel === 'leitura' &&
    dest[0].estatuto === 'alojado' && dest[1].objecto === 'V_E' && dest[1].estatuto === 'alojado' &&
    dest[2].objecto === 'V_topo' && dest[2].estatuto === 'alojado' &&
    dest.every((d) => d.tipoNovo === false) &&
    destV.length === dest.length && destV[0].objecto === dest[0].objecto &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 &&
    admiteLei(EXPR_END).ok === false && admiteLei(EXPR_VE).ok === false &&
    admiteLei(EXPR_VIZ).ok === false &&
    !kinds.includes('oficio') && !kinds.includes('grafo') &&
    !kinds.includes('vizinhanca') && !kinds.includes('topologia') &&
    /regraOficio=oficio != categoria/.test(u.nota) &&
    /regraParte=parte nao acrescenta objectos/.test(u.nota) &&
    /regraFg=xor != \* != F\(f\)F\(g\)/.test(u.nota) &&
    /oficio != categoria/.test(u.proibicao) &&
    /parte nao acrescenta objectos/.test(u.proibicao) &&
    /xor != \* != F\(f\)F\(g\)/.test(u.proibicao) &&
    /oficio != categoria/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const A = arvoreRamos()
  const dest = destinosDoTeorema('fis:def:arvore')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const V = topoTese()
  ok('§K38 fis:def:arvore: tres geometrias != tres objectos; Qm/T/I aterraram; d_ultra nao traduz',
    REGRA_VE === 'V_E != V' &&
    REGRA_TRESGEO === 'tres geometrias != tres objectos' &&
    REGRA_D === 'd_ultra != d_Hamming != d_percurso' &&
    V.VE.objecto !== V.Vtopo.objecto &&
    A.origem === 'fis:def:arvore' && A.funil === FUNIL_CASA && A.distintos &&
    A.conjunto.objecto === '{0,1}^p' && A.conjunto.papel === 'espaco' &&
    A.conjunto.casa === 'X' && A.conjunto.estatuto === 'alojado' && A.conjunto.cria === false &&
    A.Qm.objecto === 'Qm' && A.Qm.papel === 'leitura' && A.Qm.casa === 'coord-byte' &&
    A.Qm.estatuto === 'alojado' && A.Qm.nao.includes('T') &&
    A.T.objecto === 'T' && A.T.expressao === EXPR_PROF && A.T.casa === 'coord-byte' &&
    A.T.estatuto === 'alojado' && A.T.nao.includes('Qm') && A.T.cria === false &&
    A.I.objecto === 'I' && A.I.papel === 'leitura' && A.I.casa === 'coord-byte' &&
    A.I.estatuto === 'alojado' && A.I.nao.includes('espaco-novo') &&
    A.dUltra.objecto === 'd_ultra' && A.dUltra.incompFator && A.dUltra.estatuto === 'desconhecido' &&
    A.dUltra.nao.includes('Hamming') && A.dUltra.nao.includes('d_percurso') &&
    A.testemunha.ham === 1 && A.testemunha.prof === 3 && A.testemunha.d === 0.125 &&
    A.testemunha.distHamUltra === true &&
    pesoHamming(xorX(240, 224)) === 1 && prof(240, 224) === 3 && dUltra(240, 224) === 0.125 &&
    dUltra(7, 7) === 0 &&
    A.Qm.casa === A.T.casa && A.T.casa === A.I.casa &&
    A.cria === false && A.tipoNovo === false && A.f8 === false &&
    dest.length === 3 && dest.every((d) => d.papel === 'leitura' && d.estatuto === 'alojado' && d.tipoNovo === false) &&
    dest[0].objecto === 'Qm' && dest[1].objecto === 'T' && dest[2].objecto === 'I' &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 &&
    admiteLei(EXPR_QM).ok === false && admiteLei(EXPR_PROF).ok === false &&
    admiteLei(EXPR_DULTRA).ok === false && admiteLei(EXPR_PERC).ok === false &&
    !kinds.includes('arvore') && !kinds.includes('ultrametrica') &&
    !kinds.includes('Q_m') && !kinds.includes('geometria') &&
    /regraVe=V_E != V/.test(u.nota) &&
    /regraTresGeo=tres geometrias != tres objectos/.test(u.nota) &&
    /regraD=d_ultra != d_Hamming != d_percurso/.test(u.nota) &&
    /V_E != V/.test(u.proibicao) &&
    /tres geometrias != tres objectos/.test(u.proibicao) &&
    /d_ultra != d_Hamming != d_percurso/.test(u.proibicao) &&
    /V_E != V/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const U = ultraLema()
  const dest = destinosDoTeorema('fis:lem:ultra')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const A = arvoreRamos()
  ok('§K39 fis:lem:ultra: simetria/desigualdade sao propriedades de T; sem metrica nova',
    REGRA_PROP === 'propriedade interna notRightarrow entrada em ONTOLOGIA' &&
    REGRA_SUP_EX === 'mesmo suporte notRightarrow mesma expressao' &&
    REGRA_EX_OBJ === 'expressoes distintas notRightarrow objectos ontologicos distintos' &&
    U.origem === 'fis:lem:ultra' && U.funil === FUNIL_CASA && U.ok === true &&
    U.mesmoObjecto && U.simetria.objecto === 'T' && U.desigualdade.objecto === 'T' &&
    U.simetria.objecto === U.desigualdade.objecto &&
    U.simetria.expressao === EXPR_SIMETRIA && U.desigualdade.expressao === EXPR_ULTRA &&
    U.simetria.propriedade === true && U.desigualdade.propriedade === true &&
    U.simetria.papel === 'leitura' && U.simetria.casa === 'coord-byte' &&
    U.simetria.estatuto === 'alojado' && U.desigualdade.estatuto === 'alojado' &&
    U.simetria.cria === false && U.desigualdade.tipoNovo === false &&
    U.simetria.nao.includes('metrica') && U.simetria.nao.includes('ultrametrica') &&
    U.simetria.nao.includes('grafo') &&
    dUltra(240, 224) === dUltra(224, 240) && prof(240, 224) === prof(224, 240) &&
    A.dUltra.estatuto === 'desconhecido' &&
    U.cria === false && U.tipoNovo === false && U.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'T' && dest[0].papel === 'leitura' &&
    dest[0].estatuto === 'alojado' && dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 &&
    admiteLei(EXPR_SIMETRIA).ok === false && admiteLei(EXPR_ULTRA).ok === false &&
    !kinds.includes('metrica') && !kinds.includes('ultrametrica') && !kinds.includes('grafo') &&
    /regraSupEx=mesmo suporte notRightarrow mesma expressao/.test(u.nota) &&
    /regraExObj=expressoes distintas notRightarrow objectos ontologicos distintos/.test(u.nota) &&
    /mesmo suporte notRightarrow mesma expressao/.test(u.proibicao) &&
    /expressoes distintas notRightarrow objectos ontologicos distintos/.test(u.proibicao) &&
    /mesmo suporte notRightarrow mesma expressao/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const B = bolasRamos()
  const dest = destinosDoTeorema('fis:thm:bolas')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K40 fis:thm:bolas: B_cal != B; familia/prefixo aterraram; escala desconhecida; pi_r != afim',
    REGRA_PROP_EX === 'propriedade da leitura != realizacao da expressao' &&
    REGRA_BB === 'B_cal != B' &&
    B.origem === 'fis:thm:bolas' && B.funil === FUNIL_CASA && B.distintos &&
    B.familia.objecto === 'B_cal' && B.familia.papel === 'leitura' &&
    B.familia.casa === 'coord-byte' && B.familia.estatuto === 'alojado' &&
    B.familia.nao.includes('B') && B.familia.cria === false &&
    B.condPref.objecto === 'condicao' && B.condPref.expressao === EXPR_PREF &&
    B.condPref.estatuto === 'alojado' && B.condPref.casa === 'coord-byte' &&
    B.condD.objecto === 'condicao' && B.condD.expressao === EXPR_COND_D &&
    B.condD.estatuto === 'desconhecido' && B.condPref.expressao !== B.condD.expressao &&
    B.escala.objecto === 'escala' && B.escala.incompFator && B.escala.estatuto === 'desconhecido' &&
    B.pir.objecto === 'pi_r' && B.pir.nao.includes('afim') && B.pir.nao.includes('vinco') &&
    B.pir.estatuto === 'desconhecido' && B.pir.cria === false && B.pir.tipoNovo === false &&
    prefixoIgual(240, 224, 3) === true && prefixoIgual(240, 224, 4) === false &&
    B.testemunha.pref === true &&
    B.cria === false && B.tipoNovo === false && B.f8 === false &&
    dest.length === 3 && dest[0].objecto === 'B_cal' && dest[0].estatuto === 'alojado' &&
    dest[1].objecto === 'condicao' && dest[1].estatuto === 'alojado' &&
    dest[2].objecto === 'escala' && dest[2].estatuto === 'desconhecido' &&
    dest.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    REALIZACOES.length === 2 &&
    admiteLei(EXPR_BOLA).ok === false && admiteLei(EXPR_PREF).ok === false &&
    admiteLei(EXPR_COND_D).ok === false && admiteLei(EXPR_PIR).ok === false &&
    !kinds.includes('bola') && !kinds.includes('particao') && !kinds.includes('raio') &&
    /regraPropEx=propriedade da leitura != realizacao da expressao/.test(u.nota) &&
    /regraBB=B_cal != B/.test(u.nota) &&
    /propriedade da leitura != realizacao da expressao/.test(u.proibicao) &&
    /B_cal != B/.test(u.proibicao) &&
    /B_cal != B/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const V = vizArvoreRamos()
  const dest = destinosDoTeorema('fis:thm:vizarvore')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K41 fis:thm:vizarvore: N_T aterra; |N_T|=1 propriedade; != 2n != grafo',
    REGRA_NT === '|N_T(x)|=1 != 2n' &&
    PROPS_INTERNAS.includes('cardinalidade') &&
    V.origem === 'fis:thm:vizarvore' && V.funil === FUNIL_CASA && V.distintos &&
    V.NT.objecto === 'N_T' && V.NT.expressao === EXPR_NT &&
    V.NT.suporte === 'X' && V.NT.realizacao === 'B' && V.NT.relacao === 'le' &&
    V.NT.papel === 'leitura' && V.NT.casa === 'coord-byte' &&
    V.NT.estatuto === 'alojado' && V.NT.cria === false &&
    V.NT.nao.includes('V_E') && V.NT.nao.includes('V_topo') && V.NT.nao.includes('grafo') &&
    V.cardinalidade.objecto === 'cardinalidade' && V.cardinalidade.expressao === EXPR_CARD_T &&
    V.cardinalidade.propriedade === true && V.cardinalidade.papel === 'leitura' &&
    V.cardinalidade.casa === 'coord-byte' && V.cardinalidade.estatuto === 'alojado' &&
    V.cardinalidade.nao.includes('grau') && V.cardinalidade.nao.includes('2n') &&
    V.contraste.um === 1 && V.contraste.doisN === 6 && V.contraste.distinto && V.contraste.cria === false &&
    irmaoArvore(240, 3) === 208 && irmaoArvore(208, 3) === 240 &&
    irmaoArvore(240, 0) === null && irmaoArvore(240, 9) === null &&
    V.NT.casa === V.cardinalidade.casa &&
    V.cria === false && V.tipoNovo === false && V.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'N_T' && dest[1].objecto === 'cardinalidade' &&
    dest.every((d) => d.papel === 'leitura' && d.estatuto === 'alojado' && d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_NT).ok === false && admiteLei(EXPR_CARD_T).ok === false &&
    !kinds.includes('grafo') && !kinds.includes('vizinhanca') && !kinds.includes('cardinalidade') &&
    /regraNt=\|N_T\(x\)\|=1 != 2n/.test(u.nota) &&
    /\|N_T\(x\)\|=1 != 2n/.test(u.proibicao) &&
    /\|N_T\(x\)\|=1 != 2n/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const P = percursoRamos()
  const dest = destinosDoTeorema('fis:def:percurso')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const A = arvoreRamos()
  ok('§K42 fis:def:percurso: I aterra; percurso propriedade; I != T; != grafo',
    REGRA_IT === 'I != T' &&
    REGRA_PG === 'percurso != grafo' &&
    P.origem === 'fis:def:percurso' && P.funil === FUNIL_CASA && P.distintos &&
    P.I.objecto === 'I' && P.I.papel === 'leitura' && P.I.casa === 'coord-byte' &&
    P.I.estatuto === 'alojado' && P.I.cria === false && P.I.nao.includes('T') &&
    P.percurso.objecto === 'percurso' && P.percurso.expressao === EXPR_CAMINHO &&
    P.percurso.propriedade === true && P.percurso.papel === 'leitura' &&
    P.percurso.casa === 'coord-byte' && P.percurso.estatuto === 'alojado' &&
    P.percurso.nao.includes('grafo') && P.percurso.nao.includes('afim') &&
    P.I.objecto !== A.T.objecto && P.I.casa === A.T.casa &&
    P.contraste.distinto && P.contraste.cria === false &&
    irmaoArvore(240, 3) === 208 && P.testemunha.passo === true &&
    P.I.casa === P.percurso.casa &&
    P.cria === false && P.tipoNovo === false && P.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'I' && dest[1].objecto === 'percurso' &&
    dest.every((d) => d.papel === 'leitura' && d.estatuto === 'alojado' && d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_CAMINHO).ok === false &&
    !kinds.includes('percurso') && !kinds.includes('grafo') && !kinds.includes('ordem') &&
    /regraIt=I != T/.test(u.nota) &&
    /regraPg=percurso != grafo/.test(u.nota) &&
    /I != T/.test(u.proibicao) &&
    /percurso != grafo/.test(u.proibicao) &&
    /I != T/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const E = eulerRamos()
  const dest = destinosDoTeorema('fis:thm:euler')
  const dual = destinosDoTeorema('fis:def:dual')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K43 fis:thm:euler: chi_e aterra; ciclo em vizdobra; != chi_k; G_ar anexa',
    REGRA_EULER === 'Euler != gerador' &&
    REGRA_CICLO === 'ciclo != lei' &&
    E.origem === 'fis:thm:euler' && E.funil === FUNIL_CASA && E.distintos &&
    E.chiE.objecto === 'chi_e' && E.chiE.expressao === EXPR_CHI_E &&
    E.chiE.papel === 'leitura' && E.chiE.casa === 'coord-byte' &&
    E.chiE.estatuto === 'alojado' && E.chiE.nao.includes('chi') && E.chiE.nao.includes('lei_gk') &&
    E.ciclo.objecto === 'ciclo' && E.ciclo.casa === 'vizdobra' &&
    E.ciclo.propriedade === true && E.ciclo.estatuto === 'alojado' &&
    E.ciclo.nao.includes('gerador-novo') && E.ciclo.nao.includes('lei_gk') &&
    E.Gar.anexa === 'percurso' && E.Gar.casa === '' && E.Gar.cria === false &&
    chiEuler(2, 1) === 1 && E.testemunha.arvore === true &&
    dual.some((d) => d.objecto === 'chi' && d.estatuto === 'desconhecido') &&
    E.chiE.objecto !== 'chi' &&
    E.cria === false && E.tipoNovo === false && E.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'chi_e' && dest[0].papel === 'leitura' &&
    dest[1].objecto === 'ciclo' && dest[1].estatuto === 'alojado' &&
    dest.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_CHI_E).ok === false && admiteLei(EXPR_CICLO).ok === false &&
    !kinds.includes('euler') && !kinds.includes('ciclo') && !kinds.includes('chi_e') &&
    /regraEuler=Euler != gerador/.test(u.nota) &&
    /regraCiclo=ciclo != lei/.test(u.nota) &&
    /Euler != gerador/.test(u.proibicao) &&
    /ciclo != lei/.test(u.proibicao) &&
    /Euler != gerador/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const H = handshakeRamos()
  const dest = destinosDoTeorema('fis:thm:handshake')
  const E = eulerRamos()
  const dual = destinosDoTeorema('fis:def:dual')
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K44 fis:thm:handshake: conta local aterra; incidencia propriedade; != Euler; G_ar anexa',
    REGRA_HS === 'handshake != Euler' &&
    REGRA_INC === 'incidencia != grafo' &&
    H.origem === 'fis:thm:handshake' && H.funil === FUNIL_CASA && H.distintos &&
    H.conta.objecto === 'conta' && H.conta.expressao === EXPR_HS &&
    H.conta.papel === 'leitura' && H.conta.casa === 'coord-byte' &&
    H.conta.estatuto === 'alojado' && H.conta.nao.includes('Euler') &&
    H.conta.nao.includes('2_B') && H.conta.nao.includes('gerador') &&
    H.incidencia.objecto === 'incidencia' && H.incidencia.expressao === EXPR_INC &&
    H.incidencia.propriedade === true && H.incidencia.casa === '' &&
    H.incidencia.estatuto === 'alojado' && H.incidencia.nao.includes('grafo') &&
    H.Gar.anexa === 'percurso' && H.Gar.casa === '' && H.Gar.cria === false &&
    handshakeLocal(1, true, false) === 1 && handshakeLocal(1, false, true) === 1 &&
    H.testemunha.fecha === true &&
    EXPR_HS !== EXPR_CHI_E && E.chiE.expressao !== H.conta.expressao &&
    dual.some((d) => d.objecto === 'chi' && d.estatuto === 'desconhecido') &&
    H.conta.nao.includes('chi') &&
    H.cria === false && H.tipoNovo === false && H.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'conta' && dest[0].papel === 'leitura' &&
    dest[1].objecto === 'incidencia' && dest[1].estatuto === 'alojado' &&
    dest.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_HS).ok === false && admiteLei(EXPR_INC).ok === false &&
    !kinds.includes('handshake') && !kinds.includes('incidencia') && !kinds.includes('G_ar') &&
    /regraHs=handshake != Euler/.test(u.nota) &&
    /regraInc=incidencia != grafo/.test(u.nota) &&
    /handshake != Euler/.test(u.proibicao) &&
    /incidencia != grafo/.test(u.proibicao) &&
    /handshake != Euler/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const G = gradeRamos()
  const destG = destinosDoTeorema('fis:def:grade')
  const destD = destinosDoTeorema('fis:thm:dim')
  const NT = vizArvoreRamos()
  const A = arvoreRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K45 fis:def:grade / fis:thm:dim: grade pede Zc; != hipercubo; 2n != m != |N_T|',
    REGRA_GH === 'grade != hipercubo' &&
    REGRA_PM === '2n != m' &&
    REGRA_NT === '|N_T(x)|=1 != 2n' &&
    G.origem === 'fis:def:grade' && G.funil === FUNIL_CASA && G.distintos &&
    G.grade.objecto === 'grade' && G.grade.expressao === EXPR_GRADE &&
    G.grade.realizacao === 'Zc' && G.grade.incompFator && G.grade.estatuto === 'desconhecido' &&
    G.grade.nao.includes('hipercubo') && G.grade.nao.includes('espaco-novo') &&
    G.viz.objecto === 'V_grade' && G.viz.expressao === EXPR_VIZ_G &&
    G.viz.estatuto === 'desconhecido' && G.viz.nao.includes('V_E') && G.viz.nao.includes('N_T') &&
    G.card.objecto === 'card_grade' && G.card.expressao === EXPR_DIM &&
    G.card.incompFator && G.card.estatuto === 'desconhecido' &&
    G.card.nao.includes('N_T') && G.card.nao.includes('2_B') &&
    G.nSitio.anexa === 'clausula-4' && G.nSitio.casa === '' && G.nSitio.cria === false &&
    grauM(3) === 3 && grau2n(3) === 6 && G.testemunha.m === 3 && G.testemunha.doisN === 6 &&
    G.testemunha.distinto === true && G.testemunha.colapsa === true &&
    vizB(0, 0) === ek(0) && xorX(ek(0), ek(0)) === 0 &&
    NT.contraste.um === 1 && NT.contraste.doisN === 6 &&
    A.Qm.estatuto === 'alojado' && G.grade.estatuto !== A.Qm.estatuto &&
    G.cria === false && G.tipoNovo === false && G.f8 === false &&
    destG.length === 2 && destG[0].objecto === 'grade' && destG[0].estatuto === 'desconhecido' &&
    destG[1].objecto === 'V_grade' && destG[1].estatuto === 'desconhecido' &&
    destD.length === 1 && destD[0].objecto === 'card_grade' && destD[0].estatuto === 'desconhecido' &&
    destG.every((d) => d.tipoNovo === false) && destD.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_GRADE).ok === false && admiteLei(EXPR_VIZ_G).ok === false &&
    admiteLei(EXPR_DIM).ok === false &&
    !kinds.includes('grade') && !kinds.includes('hipercubo') && !kinds.includes('posto') &&
    /regraGh=grade != hipercubo/.test(u.nota) &&
    /regraPm=2n != m/.test(u.nota) &&
    /grade != hipercubo/.test(u.proibicao) &&
    /2n != m/.test(u.proibicao) &&
    /grade != hipercubo/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const V = vizdobraTopo()
  const dest = destinosDoTeorema('fis:thm:vizdobra')
  const Q = quartetoN2()
  const G = gradeRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K46 fis:thm:vizdobra: grupo ja alojado; n pares propriedade; sigma_grade != sigma_B',
    REGRA_SG === 'sigma_grade != sigma_B' &&
    REGRA_PARES === 'n pares != 2n soltos' &&
    REGRA_ENUN === 'mesmo enunciado notRightarrow mesmo objecto' &&
    REGRA_SIM === 'simbolo igual notRightarrow operacao igual' &&
    V.origem === 'fis:thm:vizdobra' && V.funil === FUNIL_CASA && V.distintos &&
    V.gerador.objecto === 'vizdobra' && V.gerador.papel === 'gerador' &&
    V.gerador.casa === 'vizdobra' && V.gerador.estatuto === 'alojado' &&
    V.gerador.cria === false && V.gerador.nao.includes('gerador-novo') &&
    V.pares.objecto === 'pares' && V.pares.expressao === EXPR_PARES &&
    V.pares.propriedade === true && V.pares.casa === 'vizdobra' &&
    V.pares.estatuto === 'alojado' && V.pares.nao.includes('2n') &&
    V.sigmaG.objecto === 'sigma_grade' && V.sigmaG.expressao === EXPR_SIG_G &&
    V.sigmaG.realizacao === 'Zc' && V.sigmaG.incompFator &&
    V.sigmaG.estatuto === 'desconhecido' && V.sigmaG.nao.includes('sigma_B') &&
    V.sigmaG.nao.includes('xor') && V.sigmaG.nao.includes('vinco') &&
    grupoGeradores8() === true &&
    sigmaI(1, 0) === 1 && sigmaBFixa(1, 0) === false && V.testemunha.naoFixa === true &&
    V.testemunha.n === 3 && V.testemunha.pares === 3 && V.testemunha.doisN === 6 &&
    V.testemunha.distinto === true &&
    GERADORES.length === 1 && GERADORES[0].id === 'vizdobra' &&
    Q.estatuto === 'alojado' && Q.nao.includes('rotor') &&
    G.grade.estatuto === 'desconhecido' &&
    V.cria === false && V.tipoNovo === false && V.f8 === false &&
    dest.length === 3 && dest[0].objecto === 'vizdobra' && dest[0].papel === 'gerador' &&
    dest[0].estatuto === 'alojado' && dest[1].objecto === 'pares' && dest[1].estatuto === 'alojado' &&
    dest[2].objecto === 'sigma_grade' && dest[2].estatuto === 'desconhecido' &&
    dest.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && CASAS.espaco === 1 && CASAS.leituras === 1 &&
    REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_SIG_G).ok === false && admiteLei(EXPR_PARES).ok === false &&
    !kinds.includes('pares') && !kinds.includes('sigma_grade') && !kinds.includes('dobra') &&
    /regraSg=sigma_grade != sigma_B/.test(u.nota) &&
    /regraPares=n pares != 2n soltos/.test(u.nota) &&
    /sigma_grade != sigma_B/.test(u.proibicao) &&
    /n pares != 2n soltos/.test(u.proibicao) &&
    /sigma_grade != sigma_B/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const F = fechoTopo()
  const dest = destinosDoTeorema('fis:thm:fecho')
  const B = buscaFecho()
  const Q = quartetoN2()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K47 fis:thm:fecho: R_ij desconhecido; R^2=-id pede Zc; binom != 2n; ordem 4 != 2',
    REGRA_ORD === 'ordem 4 != ordem 2' &&
    REGRA_PLANOS === 'binom(n,2) != 2n' &&
    F.origem === 'fis:thm:fecho' && F.funil === FUNIL_CASA && F.distintos &&
    F.Rij.objecto === 'R_ij' && F.Rij.expressao === EXPR_R4 &&
    F.Rij.estatuto === 'desconhecido' && F.Rij.casa === '' &&
    F.Rij.nao.includes('gerador') && F.Rij.nao.includes('quarteto') &&
    F.Rij.ordemPaper === 4 && F.Rij.ordemEmB === 2 &&
    F.menos.objecto === 'R2' && F.menos.expressao === EXPR_R2 &&
    F.menos.realizacao === 'Zc' && F.menos.incompFator &&
    F.menos.estatuto === 'desconhecido' && F.menos.nao.includes('2_B') &&
    F.planos.objecto === 'planos' && F.planos.expressao === EXPR_PLANOS &&
    F.planos.propriedade === true && F.planos.casa === '' &&
    F.planos.estatuto === 'alojado' && F.planos.nao.includes('2n') &&
    nPlanos(1) === 0 && nPlanos(2) === 1 && nPlanos(3) === 3 && nPlanos(4) === 6 &&
    F.testemunha.planos === 3 && F.testemunha.doisN === 6 && F.testemunha.distinto === true &&
    B.estatuto === 'desconhecido' && B.ordemPaper === 4 && B.ordemEmB === 2 && B.nPlanos === 3 &&
    Q.ordem === 2 && Q.estatuto === 'alojado' && Q.nao.includes('rotor') &&
    dest.length === 1 && dest[0].objecto === 'R_ij' && dest[0].estatuto === 'desconhecido' &&
    dest[0].motivo === 'estrutural' && dest[0].tipoNovo === false &&
    F.cria === false && F.tipoNovo === false && F.f8 === false &&
    GERADORES.length === 1 && LEITURAS.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_R4).ok === false && admiteLei(EXPR_R2).ok === false &&
    admiteLei(EXPR_PLANOS).ok === false &&
    !kinds.includes('rotor') && !kinds.includes('plano') && !kinds.includes('binom') &&
    /regraOrd=ordem 4 != ordem 2/.test(u.nota) &&
    /regraPlanos=binom\(n,2\) != 2n/.test(u.nota) &&
    /ordem 4 != ordem 2/.test(u.proibicao) &&
    /binom\(n,2\) != 2n/.test(u.proibicao) &&
    /ordem 4 != ordem 2/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const V = viznisoRamos()
  const dest = destinosDoTeorema('fis:thm:vizniso')
  const P = percursoRamos()
  const NT = vizArvoreRamos()
  const G = gradeRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K48 fis:thm:vizniso: bloqueio; codificar em I; linearizar n>=2 incompatível',
    REGRA_CL === 'codificar != linearizar' &&
    REGRA_PS === 'Peano != serpentina' &&
    REGRA_IVG === 'I != V_grade' &&
    V.origem === 'fis:thm:vizniso' && V.funil === FUNIL_CASA && V.distintos &&
    V.I.objecto === 'I' && V.I.papel === 'leitura' && V.I.casa === 'coord-byte' &&
    V.I.estatuto === 'alojado' && V.I.cria === false && V.I.nao.includes('V_grade') &&
    V.linearizar.objecto === 'linearizar' && V.linearizar.expressao === EXPR_LIN &&
    V.linearizar.estatuto === 'desconhecido' && V.linearizar.casa === '' &&
    V.linearizar.nao.includes('codificar') && V.linearizar.nao.includes('isomorfismo') &&
    V.bloqueio.objecto === 'bloqueio' && V.bloqueio.expressao === EXPR_BLOQ &&
    V.bloqueio.propriedade === true && V.bloqueio.casa === '' &&
    V.bloqueio.estatuto === 'alojado' && V.bloqueio.nao.includes('dinamica') &&
    viznisoBloqueia(3, 1) === true && viznisoBloqueia(3, 2) === true &&
    viznisoBloqueia(1, 2) === false &&
    V.testemunha.um === 1 && V.testemunha.dois === 2 && V.testemunha.doisN === 6 &&
    V.testemunha.contraArvore === true && V.testemunha.contraRegua === true &&
    V.testemunha.n1 === true && V.testemunha.distinto === true &&
    NT.contraste.um === 1 && NT.contraste.doisN === 6 &&
    P.I.casa === V.I.casa && G.viz.estatuto === 'desconhecido' &&
    V.cria === false && V.tipoNovo === false && V.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'I' && dest[0].papel === 'leitura' &&
    dest[0].estatuto === 'alojado' && dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_COD).ok === false && admiteLei(EXPR_LIN).ok === false &&
    admiteLei(EXPR_BLOQ).ok === false &&
    !kinds.includes('isomorfismo') && !kinds.includes('dinamica') && !kinds.includes('Peano') &&
    /regraCl=codificar != linearizar/.test(u.nota) &&
    /regraPs=Peano != serpentina/.test(u.nota) &&
    /regraIvg=I != V_grade/.test(u.nota) &&
    /codificar != linearizar/.test(u.proibicao) &&
    /Peano != serpentina/.test(u.proibicao) &&
    /I != V_grade/.test(u.proibicao) &&
    /codificar != linearizar/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const M = matrizRamos()
  const dest = destinosDoTeorema('fis:cor:matriz')
  const V = viznisoRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K49 fis:cor:matriz: IxI nao achata; indice != conteudo; 1x1=escalar',
    REGRA_IC === 'indice != conteudo' &&
    REGRA_IXI === 'IxI != I' &&
    M.origem === 'fis:cor:matriz' && M.funil === FUNIL_CASA && M.distintos &&
    M.I.objecto === 'I' && M.I.papel === 'leitura' && M.I.casa === 'coord-byte' &&
    M.I.estatuto === 'alojado' && M.I.nao.includes('IxI') &&
    M.indice.objecto === 'IxI' && M.indice.expressao === EXPR_IXI &&
    M.indice.estatuto === 'desconhecido' && M.indice.casa === '' &&
    M.indice.nao.includes('I') && M.indice.nao.includes('categoria') &&
    M.conteudo.objecto === 'conteudo' && M.conteudo.expressao === EXPR_CONT &&
    M.conteudo.propriedade === true && M.conteudo.estatuto === 'alojado' &&
    M.conteudo.nao.includes('indice') && M.conteudo.nao.includes('categoria') &&
    grau2n(2) === 4 && viznisoBloqueia(2, 2) === true && viznisoBloqueia(1, 2) === false &&
    M.testemunha.quatro === 4 && M.testemunha.achata === true &&
    M.testemunha.escalar === true && M.testemunha.distinto === true &&
    V.I.casa === M.I.casa && V.linearizar.estatuto === 'desconhecido' &&
    M.cria === false && M.tipoNovo === false && M.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'I' && dest[0].papel === 'leitura' &&
    dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_IXI).ok === false && admiteLei(EXPR_CONT).ok === false &&
    admiteLei(EXPR_ESC).ok === false &&
    !kinds.includes('matriz') && !kinds.includes('indice') && !kinds.includes('produto') &&
    /regraIc=indice != conteudo/.test(u.nota) &&
    /regraIxi=IxI != I/.test(u.nota) &&
    /indice != conteudo/.test(u.proibicao) &&
    /IxI != I/.test(u.proibicao) &&
    /indice != conteudo/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const P = palavraRamos()
  const dest = destinosDoTeorema('fis:def:palavra')
  const perc = percursoRamos()
  const A = arvoreRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K50 fis:def:palavra: palavra aterra em I; T_letra != T; alfabeto != A; 2n != m',
    REGRA_PAL === 'palavra != espaco' &&
    REGRA_TT === 'T_letra != T' &&
    REGRA_ALF === 'alfabeto != A' &&
    REGRA_CASA_LE === 'outra leitura do mesmo suporte nao cria casa' &&
    P.origem === 'fis:def:palavra' && P.funil === FUNIL_CASA && P.distintos &&
    P.palavra.objecto === 'palavra' && P.palavra.expressao === EXPR_PAL &&
    P.palavra.papel === 'leitura' && P.palavra.casa === 'coord-byte' &&
    P.palavra.estatuto === 'alojado' && P.palavra.nao.includes('espaco') &&
    P.alfabeto.objecto === 'alfabeto' && P.alfabeto.expressao === EXPR_ALF &&
    P.alfabeto.propriedade === true && P.alfabeto.casa === '' &&
    P.alfabeto.estatuto === 'alojado' && P.alfabeto.nao.includes('A') &&
    P.Tletra.objecto === 'T_letra' && P.Tletra.expressao === EXPR_TLETRA &&
    P.Tletra.estatuto === 'desconhecido' && P.Tletra.casa === '' &&
    P.Tletra.nao.includes('T') && P.Tletra.nao.includes('afim') &&
    P.Tletra.nao.includes('realizacao-nova') &&
    grauM(3) === 3 && grau2n(3) === 6 && P.testemunha.m === 3 &&
    P.testemunha.doisN === 6 && P.testemunha.distinto === true &&
    perc.I.casa === P.palavra.casa && A.T.objecto === 'T' &&
    P.Tletra.objecto !== A.T.objecto && P.alfabeto.expressao !== EXPR_A &&
    P.cria === false && P.tipoNovo === false && P.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'palavra' && dest[0].papel === 'leitura' &&
    dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_PAL).ok === false && admiteLei(EXPR_ALF).ok === false &&
    admiteLei(EXPR_TLETRA).ok === false &&
    !kinds.includes('palavra') && !kinds.includes('alfabeto') && !kinds.includes('letra') &&
    /regraPal=palavra != espaco/.test(u.nota) &&
    /regraTt=T_letra != T/.test(u.nota) &&
    /regraAlf=alfabeto != A/.test(u.nota) &&
    /palavra != espaco/.test(u.proibicao) &&
    /T_letra != T/.test(u.proibicao) &&
    /alfabeto != A/.test(u.proibicao) &&
    /palavra != espaco/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const P = palavraTeorema()
  const dest = destinosDoTeorema('fis:thm:palavra')
  const W = palavraRamos()
  const E = eulerRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K51 fis:thm:palavra: reconstrucao aterra; G>1 e ker w propriedades; != vizdobra',
    REGRA_CC === 'comuta != cancela' &&
    REGRA_G1 === 'G>1 != gerador' &&
    REGRA_KER === 'ker w != nucleo' &&
    P.origem === 'fis:thm:palavra' && P.funil === FUNIL_CASA && P.distintos &&
    P.recon.objecto === 'reconstrucao' && P.recon.expressao === EXPR_REC &&
    P.recon.papel === 'leitura' && P.recon.casa === 'coord-byte' &&
    P.recon.estatuto === 'alojado' && P.recon.nao.includes('espaco-novo') &&
    P.g1.objecto === 'G1' && P.g1.expressao === EXPR_G1 &&
    P.g1.propriedade === true && P.g1.estatuto === 'alojado' &&
    P.g1.nao.includes('gerador') && P.g1.nao.includes('vizdobra') &&
    P.ker.objecto === 'ker_w' && P.ker.expressao === EXPR_KER &&
    P.ker.propriedade === true && P.ker.casa === '' &&
    P.ker.estatuto === 'alojado' && P.ker.nao.includes('nucleo') &&
    xorPalavra([ek(0), ek(0)]) === 0 && prefixosColidem([ek(0), ek(0)]) === true &&
    prefixosColidem([ek(0)]) === false &&
    P.testemunha.fecha === true && P.testemunha.soma === 0 &&
    W.palavra.casa === P.recon.casa && E.ciclo.casa === 'vizdobra' &&
    P.g1.nao.includes('vizdobra') &&
    P.cria === false && P.tipoNovo === false && P.f8 === false &&
    dest.length === 3 && dest[0].objecto === 'reconstrucao' && dest[0].papel === 'leitura' &&
    dest[1].objecto === 'G1' && dest[1].estatuto === 'alojado' &&
    dest[2].objecto === 'ker_w' && dest[2].estatuto === 'alojado' &&
    dest.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_REC).ok === false && admiteLei(EXPR_G1).ok === false &&
    admiteLei(EXPR_KER).ok === false &&
    !kinds.includes('kernel') && !kinds.includes('nucleo') && !kinds.includes('fator') &&
    /regraCc=comuta != cancela/.test(u.nota) &&
    /regraG1=G>1 != gerador/.test(u.nota) &&
    /regraKer=ker w != nucleo/.test(u.nota) &&
    /comuta != cancela/.test(u.proibicao) &&
    /G>1 != gerador/.test(u.proibicao) &&
    /ker w != nucleo/.test(u.proibicao) &&
    /comuta != cancela/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const C = contrariaRamos()
  const dest = destinosDoTeorema('fis:thm:contraria')
  const P = palavraTeorema()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K52 fis:thm:contraria: rho aterra; buraco != dobra; injetivo != invertivel',
    REGRA_PR === 'pi != rho' &&
    REGRA_BUR === 'buraco != dobra' &&
    REGRA_INJ === 'injetivo != invertivel' &&
    C.origem === 'fis:thm:contraria' && C.funil === FUNIL_CASA && C.distintos &&
    C.rho.objecto === 'rho' && C.rho.expressao === EXPR_RHO &&
    C.rho.papel === 'leitura' && C.rho.casa === 'coord-byte' &&
    C.rho.estatuto === 'alojado' && C.rho.nao.includes('inversa') &&
    C.rho.nao.includes('2_B') && C.rho.nao.includes('Duo') &&
    C.buraco.objecto === 'buraco' && C.buraco.expressao === EXPR_BUR &&
    C.buraco.propriedade === true && C.buraco.casa === '' &&
    C.buraco.estatuto === 'alojado' && C.buraco.nao.includes('dobra') &&
    rho2t(0) === 0 && rho2t(1) === 2 && rho2t(2) === 4 &&
    rhoBuracos(3) === 3 &&
    C.testemunha.inj === true && C.testemunha.surj === false &&
    C.testemunha.buracos.length === 3 && C.testemunha.distinto === true &&
    P.g1.estatuto === 'alojado' && P.g1.objecto !== C.buraco.objecto &&
    C.cria === false && C.tipoNovo === false && C.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'rho' && dest[0].papel === 'leitura' &&
    dest[1].objecto === 'buraco' && dest[1].estatuto === 'alojado' &&
    dest.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_RHO).ok === false && admiteLei(EXPR_BUR).ok === false &&
    !kinds.includes('buraco') && !kinds.includes('rho') && !kinds.includes('retratacao') &&
    /regraPr=pi != rho/.test(u.nota) &&
    /regraBur=buraco != dobra/.test(u.nota) &&
    /pi != rho/.test(u.proibicao) &&
    /buraco != dobra/.test(u.proibicao) &&
    /pi != rho/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const F = folgaRamos()
  const dest = destinosDoTeorema('fis:cor:folga')
  const C = contrariaRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K53 fis:cor:folga: mesma folga; duas expr != dois objectos; != Duo',
    REGRA_FOL === 'folga != Duo' &&
    REGRA_MESMA === 'mesma folga != dois objectos' &&
    REGRA_EX_OBJ === 'expressoes distintas notRightarrow objectos ontologicos distintos' &&
    F.origem === 'fis:cor:folga' && F.funil === FUNIL_CASA && F.mesmoObjecto && F.distintos &&
    F.cola.objecto === 'folga' && F.cola.expressao === EXPR_G1SUM &&
    F.cola.propriedade === true && F.cola.casa === '' && F.cola.estatuto === 'alojado' &&
    F.cola.nao.includes('Duo') &&
    F.conta.objecto === 'folga' && F.conta.expressao === EXPR_FOL &&
    F.conta.propriedade === true && F.conta.estatuto === 'alojado' &&
    F.conta.nao.includes('Duo') && F.cola.objecto === F.conta.objecto &&
    folga(6, 3) === 3 && rhoBuracos(3) === 3 && F.testemunha.mesma === true &&
    C.testemunha.buracos.length === 3 &&
    F.cria === false && F.tipoNovo === false && F.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'folga' && dest[0].estatuto === 'alojado' &&
    dest[0].papel === '' && dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_FOL).ok === false && admiteLei(EXPR_G1SUM).ok === false &&
    !kinds.includes('folga') && !kinds.includes('Duo') &&
    /regraFol=folga != Duo/.test(u.nota) &&
    /regraMesma=mesma folga != dois objectos/.test(u.nota) &&
    /folga != Duo/.test(u.proibicao) &&
    /mesma folga != dois objectos/.test(u.proibicao) &&
    /folga != Duo/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const B = bijeccaoRamos()
  const dest = destinosDoTeorema('fis:thm:bijeccao')
  const F = folgaRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const c0 = serpentina(0, 2)
  const c1 = serpentina(1, 2)
  const c2 = serpentina(2, 2)
  const c3 = serpentina(3, 2)
  ok('§K54 fis:thm:bijeccao: s bijetiva; G=1; folga 0; != Iso != vizinhanca',
    REGRA_BIJ === 'bijeccao != Iso' &&
    REGRA_BV === 'bijeccao != vizinhanca' &&
    REGRA_CL === 'codificar != linearizar' &&
    REGRA_PS === 'Peano != serpentina' &&
    B.origem === 'fis:thm:bijeccao' && B.funil === FUNIL_CASA && B.distintos &&
    B.s.objecto === 's' && B.s.expressao === EXPR_S &&
    B.s.papel === 'leitura' && B.s.relacao === 'le' &&
    B.s.casa === LEITURA_COORD.id && B.s.estatuto === 'alojado' &&
    B.s.nao.includes('Iso') && B.s.nao.includes('vizinhanca') && B.s.nao.includes('Peano') &&
    B.gum.expressao === EXPR_GUM && B.gum.propriedade === true &&
    B.gum.casa === '' && B.gum.estatuto === 'alojado' &&
    B.gum.nao.includes('gerador') && EXPR_GUM !== EXPR_G1 &&
    c0[0] === 0 && c0[1] === 0 &&
    c1[0] === 1 && c1[1] === 0 &&
    c2[0] === 1 && c2[1] === 1 &&
    c3[0] === 0 && c3[1] === 1 &&
    serpentinaInv(c0, 2) === 0 && serpentinaInv(c1, 2) === 1 &&
    serpentinaInv(c2, 2) === 2 && serpentinaInv(c3, 2) === 3 &&
    B.testemunha.bij === true && B.testemunha.n === 4 &&
    B.testemunha.folgaZero === true && folga(4, 4) === 0 &&
    F.testemunha.mesma === true &&
    B.cria === false && B.tipoNovo === false && B.f8 === false &&
    dest.length === 1 && dest[0].objecto === 's' && dest[0].papel === 'leitura' &&
    dest[0].relacao === 'le' && dest[0].estatuto === 'alojado' && dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_S).ok === false && admiteLei(EXPR_GUM).ok === false &&
    !kinds.includes('bijeccao') && !kinds.includes('serpentina') && !kinds.includes('Iso') &&
    /regraBij=bijeccao != Iso/.test(u.nota) &&
    /regraBv=bijeccao != vizinhanca/.test(u.nota) &&
    /bijeccao != Iso/.test(u.proibicao) &&
    /bijeccao != vizinhanca/.test(u.proibicao) &&
    /bijeccao != Iso/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const E = enumeraRamos()
  const dest = destinosDoTeorema('fis:thm:enumera')
  const S = bijeccaoRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const p0 = phiEnumera(0, 2, 2)
  const p1 = phiEnumera(1, 2, 2)
  const p2 = phiEnumera(2, 2, 2)
  const p3 = phiEnumera(3, 2, 2)
  const s2 = serpentina(2, 2)
  ok('§K55 fis:thm:enumera: varphi posicional; != s; != vizinhanca; != cadeia',
    REGRA_POS === 'posicional != vizinhanca' &&
    REGRA_VS === 'varphi != s' &&
    REGRA_CL === 'codificar != linearizar' &&
    REGRA_BV === 'bijeccao != vizinhanca' &&
    E.origem === 'fis:thm:enumera' && E.funil === FUNIL_CASA &&
    E.mesmoObjecto && E.distintos &&
    E.phi.objecto === 'varphi' && E.phi.expressao === EXPR_VARPHI &&
    E.phi.papel === 'leitura' && E.phi.relacao === 'le' &&
    E.phi.casa === LEITURA_COORD.id && E.phi.estatuto === 'alojado' &&
    E.phi.nao.includes('cadeia') && E.phi.nao.includes('vizinhanca') && E.phi.nao.includes('s') &&
    E.pos.objecto === 'varphi' && E.pos.expressao === EXPR_POSL &&
    E.pos.propriedade === true && E.pos.casa === '' && E.pos.estatuto === 'alojado' &&
    p0[0] === 0 && p0[1] === 0 &&
    p1[0] === 1 && p1[1] === 0 &&
    p2[0] === 0 && p2[1] === 1 &&
    p3[0] === 1 && p3[1] === 1 &&
    phiInv(p0, 2) === 0 && phiInv(p1, 2) === 1 &&
    phiInv(p2, 2) === 2 && phiInv(p3, 2) === 3 &&
    coord(2, 0) === p2[0] && coord(2, 1) === p2[1] &&
    s2[0] === 1 && s2[1] === 1 &&
    !(s2[0] === p2[0] && s2[1] === p2[1]) &&
    E.testemunha.bij === true && E.testemunha.N === 4 &&
    E.testemunha.bit === true && E.testemunha.distintoS === true &&
    E.testemunha.guardaViz === false &&
    S.testemunha.bij === true &&
    E.cria === false && E.tipoNovo === false && E.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'varphi' && dest[0].papel === 'leitura' &&
    dest[0].relacao === 'le' && dest[0].estatuto === 'alojado' && dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    CADEIA.join('>') === 'vizdobra>X>base>afim>vinco' &&
    admiteLei(EXPR_VARPHI).ok === false && admiteLei(EXPR_POSL).ok === false &&
    !kinds.includes('enumeracao') && !kinds.includes('posicional') &&
    !kinds.includes('tupla') && !kinds.includes('digito') &&
    /regraPos=posicional != vizinhanca/.test(u.nota) &&
    /regraVs=varphi != s/.test(u.nota) &&
    /posicional != vizinhanca/.test(u.proibicao) &&
    /varphi != s/.test(u.proibicao) &&
    /posicional != vizinhanca/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const N = navegaRamos()
  const dest = destinosDoTeorema('fis:thm:navega')
  const U = ultraLema()
  const E = enumeraRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K56 fis:thm:navega: D=sup d_x em T; compor != acumular; d_x != D',
    REGRA_ACC === 'compor != acumular' &&
    REGRA_DX === 'd_x != D' &&
    REGRA_D === 'd_ultra != d_Hamming != d_percurso' &&
    REGRA_IT === 'I != T' &&
    N.origem === 'fis:thm:navega' && N.funil === FUNIL_CASA &&
    N.mesmoObjecto && N.distintos &&
    N.dx.objecto === 'T' && N.dx.expressao === EXPR_DX &&
    N.dx.propriedade === true && N.dx.papel === 'leitura' &&
    N.dx.casa === LEITURA_COORD.id && N.dx.estatuto === 'alojado' &&
    N.dx.nao.includes('estado') && N.dx.nao.includes('D') &&
    N.D.objecto === 'T' && N.D.expressao === EXPR_DMAX &&
    N.D.propriedade === true && N.D.estatuto === 'alojado' &&
    N.D.nao.includes('metrica') && N.D.nao.includes('acumulo') &&
    N.testemunha.zero === 0 && N.testemunha.pior > 0 &&
    N.testemunha.sim === true && N.testemunha.naoExcede === true &&
    N.testemunha.naoIgual === true &&
    supDx(N.testemunha.addrPhi, N.testemunha.addrPhi, 2) === 0 &&
    supDx(N.testemunha.addrPhi, N.testemunha.addrS, 2) === N.testemunha.pior &&
    dUltra(2, 3, 2) === dUltra(3, 2, 2) &&
    U.ok === true && U.simetria.objecto === 'T' &&
    E.testemunha.distintoS === true &&
    N.cria === false && N.tipoNovo === false && N.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'T' && dest[0].papel === 'leitura' &&
    dest[0].relacao === 'le' && dest[0].estatuto === 'alojado' && dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_DX).ok === false && admiteLei(EXPR_DMAX).ok === false &&
    !kinds.includes('navegacao') && !kinds.includes('travessia') &&
    !kinds.includes('D') && !kinds.includes('acumulo') && !kinds.includes('representacao') &&
    /regraAcc=compor != acumular/.test(u.nota) &&
    /regraDx=d_x != D/.test(u.nota) &&
    /compor != acumular/.test(u.proibicao) &&
    /d_x != D/.test(u.proibicao) &&
    /compor != acumular/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const P = travessiaRamos()
  const dest = destinosDoTeorema('fis:prop:travessia')
  const N = navegaRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K57 fis:prop:travessia: T=S o R^{-1} permuta digitos; preco em pi; != T',
    REGRA_TR === 'T_trav != T' &&
    REGRA_PX === 'preco em pi != X' &&
    REGRA_ACC === 'compor != acumular' &&
    REGRA_TT === 'T_letra != T' &&
    P.origem === 'fis:prop:travessia' && P.funil === FUNIL_CASA &&
    P.mesmoObjecto && P.distintos &&
    P.trav.objecto === 'Trav' && P.trav.expressao === EXPR_TRAV &&
    P.trav.propriedade === true && P.trav.casa === '' && P.trav.estatuto === 'alojado' &&
    P.trav.nao.includes('T') && P.trav.nao.includes('dinamica') &&
    P.pip.objecto === 'Trav' && P.pip.expressao === EXPR_PIP &&
    P.pip.propriedade === true && P.pip.estatuto === 'alojado' &&
    P.pip.nao.includes('X') && P.pip.nao.includes('pi_r') &&
    EXPR_PIP !== EXPR_PIR && EXPR_TRAV !== EXPR_DMAX &&
    P.trav.objecto !== 'T' &&
    permPi([0, 1], [0, 1])[0] === 0 && permPi([1, 0], [0, 1])[0] === 1 &&
    qPi([0, 1]) === -1 && qPi([1, 0]) === 0 &&
    precoPi([0, 1]) === 0 && precoPi([1, 0]) === 1 &&
    aplicaTrav(1, [1, 0], 2, 2) === 2 && aplicaTrav(2, [1, 0], 2, 2) === 1 &&
    leituraR([0, 1], [0, 1], 2) === 2 && leituraR([0, 1], [1, 0], 2) === 1 &&
    P.testemunha.escreve === true && P.testemunha.bij === true &&
    P.testemunha.mu === true && P.testemunha.precoOk === true &&
    P.testemunha.Did === 0 && P.testemunha.Dswap === 1 &&
    P.testemunha.qSwap === 0 && P.testemunha.precoSwap === 1 &&
    N.testemunha.naoExcede === true &&
    P.cria === false && P.tipoNovo === false && P.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'Trav' && dest[0].papel === '' &&
    dest[0].estatuto === 'alojado' && dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_TRAV).ok === false && admiteLei(EXPR_PIP).ok === false &&
    !kinds.includes('travessia') && !kinds.includes('permutacao') &&
    !kinds.includes('medida') && !kinds.includes('pi') && !kinds.includes('Trav') &&
    /regraTr=T_trav != T/.test(u.nota) &&
    /regraPx=preco em pi != X/.test(u.nota) &&
    /T_trav != T/.test(u.proibicao) &&
    /preco em pi != X/.test(u.proibicao) &&
    /T_trav != T/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const D = duomorfRamos()
  const dest = destinosDoTeorema('fis:def:duomorf')
  const P = travessiaRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const uJson = JSON.parse(readFileSync(join(RAIZ, 'conecthus', 'schema', 'u.json'), 'utf8'))
  ok('§K58 fis:def:duomorf: unica != duomorfa; Iso != Duo; D_op != D; Mor intacto',
    REGRA_UD === 'unica != duomorfa' &&
    REGRA_BID === 'Iso != Duo' &&
    REGRA_DOP === 'D_op != D' &&
    REGRA_BIJ === 'bijeccao != Iso' &&
    REGRA_FOL === 'folga != Duo' &&
    D.origem === 'fis:def:duomorf' && D.funil === FUNIL_CASA &&
    D.mesmoObjecto === false && D.distintos &&
    D.unica.objecto === 'Trav' && D.unica.expressao === EXPR_UNI &&
    D.unica.propriedade === true && D.unica.estatuto === 'alojado' &&
    D.unica.nao.includes('Duo') && D.unica.nao.includes('Iso') &&
    D.duo.objecto === 'Duo' && D.duo.expressao === EXPR_DOP &&
    D.duo.estatuto === 'desconhecido' && D.duo.motivo === 'estrutural' &&
    D.duo.nao.includes('Trav') && D.duo.nao.includes('estrela') &&
    D.duo.tipoNovo === false && D.duo.cria === false &&
    eixosF(true, null).bij === true &&
    eixosF(true, null).iso === false && eixosF(true, null).duo === false &&
    eixosF(true, 0).iso === true && eixosF(true, 0).duo === false &&
    eixosF(true, 1).duo === true && eixosF(true, 1).iso === false &&
    eixosF(false, 1).duo === false &&
    D.testemunha.bij === true && D.testemunha.iso === false &&
    D.testemunha.duo === false && D.testemunha.unica === true &&
    D.testemunha.promovido === false &&
    P.testemunha.bij === true && EXPR_DOP !== EXPR_DMAX &&
    uJson.Mor.includes('Duo') && uJson.star === 'D' &&
    D.cria === false && D.tipoNovo === false && D.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'Trav' && dest[0].estatuto === 'alojado' &&
    dest[1].objecto === 'Duo' && dest[1].estatuto === 'desconhecido' &&
    dest[1].motivo === 'estrutural' && dest[1].tipoNovo === false &&
    dest.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_UNI).ok === false && admiteLei(EXPR_DOP).ok === false &&
    !kinds.includes('Duo') && !kinds.includes('duomorfismo') &&
    !kinds.includes('dualidade') && !kinds.includes('estrela') &&
    /regraUd=unica != duomorfa/.test(u.nota) &&
    /regraBid=Iso != Duo/.test(u.nota) &&
    /regraDop=D_op != D/.test(u.nota) &&
    /unica != duomorfa/.test(u.proibicao) &&
    /Iso != Duo/.test(u.proibicao) &&
    /D_op != D/.test(u.proibicao) &&
    /unica != duomorfa/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const O = octoniaoObs ()
  const dest = destinosDoTeorema('fis:obs:octoniao-interface')
  const D = duomorfRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K59 fis:obs:octoniao-interface: L7 duas faces; interface != categoria; hexal != Duo',
    REGRA_IF === 'interface != categoria' &&
    REGRA_HX === 'hexal != Duo' &&
    REGRA_LF === 'ligar != fundir' &&
    REGRA_OFICIO === 'oficio != categoria' &&
    REGRA_FOL === 'folga != Duo' &&
    O.origem === 'fis:obs:octoniao-interface' && O.funil === FUNIL_CASA &&
    O.mesmoObjecto === false && O.distintos &&
    O.faces.objecto === 'faces' && O.faces.expressao === EXPR_FACES &&
    O.faces.propriedade === true && O.faces.estatuto === 'alojado' &&
    O.faces.nao.includes('categoria') && O.faces.nao.includes('fusao') &&
    O.iface.objecto === 'interface' && O.iface.expressao === EXPR_IF &&
    O.iface.estatuto === 'desconhecido' && O.iface.motivo === 'estrutural' &&
    O.iface.nao.includes('categoria') && O.iface.nao.includes('hexal') &&
    O.iface.nao.includes('relogio') && O.iface.tipoNovo === false &&
    O.testemunha.oito === 8 && O.testemunha.l7 === true &&
    O.testemunha.lei8 === false && O.testemunha.hexal === 6 &&
    O.testemunha.dim === 8 && O.testemunha.dobras === 3 &&
    LEIS_BASE.join('') === '01234567' && DIM_X === 8 &&
    O.testemunha.cadeiaObs !== O.testemunha.cadeiaGer &&
    D.duo.estatuto === 'desconhecido' && D.f8 === false &&
    O.cria === false && O.tipoNovo === false && O.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'faces' && dest[0].estatuto === 'alojado' &&
    dest[1].objecto === 'interface' && dest[1].estatuto === 'desconhecido' &&
    dest[1].motivo === 'estrutural' && dest[1].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_FACES).ok === false && admiteLei(EXPR_IF).ok === false &&
    !kinds.includes('interface') && !kinds.includes('octoniao') &&
    !kinds.includes('hexal') && !kinds.includes('categoria') && !kinds.includes('Lei8') &&
    !/from ['"]\.\/banco_relogio/.test(ponte) &&
    /regraIf=interface != categoria/.test(u.nota) &&
    /regraHx=hexal != Duo/.test(u.nota) &&
    /regraLf=ligar != fundir/.test(u.nota) &&
    /interface != categoria/.test(u.proibicao) &&
    /hexal != Duo/.test(u.proibicao) &&
    /ligar != fundir/.test(u.proibicao) &&
    /interface != categoria/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const M = dominioInterface()
  const dest = destinosDoTeorema('fis:def:dominio-interface')
  const Q = quartetoN2()
  const O = octoniaoObs()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K60 fis:def:dominio-interface: M=O sqcup T; interface != transporte; T_fio != T',
    REGRA_ITR === 'interface != transporte' &&
    REGRA_TF === 'T_fio != T' &&
    REGRA_QM === 'Q_ab != operadores de M' &&
    REGRA_IF === 'interface != categoria' &&
    REGRA_TR === 'T_trav != T' &&
    REGRA_IT === 'I != T' &&
    M.origem === 'fis:def:dominio-interface' && M.funil === FUNIL_CASA &&
    M.mesmoObjecto === false && M.distintos &&
    M.part.objecto === 'M' && M.part.expressao === EXPR_M &&
    M.part.propriedade === true && M.part.estatuto === 'desconhecido' &&
    M.part.nao.includes('espaco') && M.part.nao.includes('MOVE') &&
    M.tfio.objecto === 'T_fio' && M.tfio.expressao === EXPR_TFIO &&
    M.tfio.estatuto === 'desconhecido' && M.tfio.na === true &&
    M.tfio.nao.includes('T') && M.tfio.nao.includes('Duo') &&
    M.iface.objecto === 'interface' && M.iface.estatuto === 'desconhecido' &&
    M.iface.nao.includes('transporte') && M.iface.nao.includes('categoria') &&
    M.qab.objecto === 'Q_ab' && M.qab.casa === GERADOR_VIZDOBRA.id &&
    M.qab.estatuto === 'alojado' && M.qab.n === 4 &&
    M.qab.viveEmO === true && M.qab.operadoresDeM === false &&
    Q.estatuto === 'alojado' && Q.n === 4 &&
    M.testemunha.quatro === true && M.testemunha.vazio === true &&
    M.testemunha.duoEmT === false && M.testemunha.move === false &&
    O.iface.estatuto === 'desconhecido' &&
    M.cria === false && M.tipoNovo === false && M.f8 === false &&
    dest.length === 3 && dest[0].objecto === 'Q_ab' && dest[0].papel === 'gerador' &&
    dest[0].estatuto === 'alojado' && dest[0].relacao === 'gera' &&
    dest[1].objecto === 'T_fio' && dest[1].estatuto === 'desconhecido' &&
    dest[2].objecto === 'interface' && dest[2].estatuto === 'desconhecido' &&
    dest.every((d) => d.tipoNovo === false) &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_M).ok === false && admiteLei(EXPR_TFIO).ok === false &&
    !kinds.includes('M') && !kinds.includes('O') && !kinds.includes('T_fio') &&
    !kinds.includes('transporte') && !kinds.includes('dominio') &&
    /regraItr=interface != transporte/.test(u.nota) &&
    /regraTf=T_fio != T/.test(u.nota) &&
    /regraQm=Q_ab != operadores de M/.test(u.nota) &&
    /interface != transporte/.test(u.proibicao) &&
    /T_fio != T/.test(u.proibicao) &&
    /Q_ab != operadores de M/.test(u.proibicao) &&
    /interface != transporte/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const E = eixosRamos()
  const dest = destinosDoTeorema('fis:thm:eixos')
  const B = bijeccaoRamos()
  const C = contrariaRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K61 fis:thm:eixos: G mede bijeccao; G=1 != Iso; duas especies != tres',
    REGRA_GI === 'G=1 != Iso' &&
    REGRA_AX === 'eixo a != eixo G' &&
    REGRA_2E === 'duas especies != tres' &&
    REGRA_BIJ === 'bijeccao != Iso' &&
    REGRA_BID === 'Iso != Duo' &&
    E.origem === 'fis:thm:eixos' && E.funil === FUNIL_CASA &&
    E.mesmoObjecto === false && E.distintos &&
    E.eixoA.objecto === 'a' && E.eixoA.expressao === EXPR_AA &&
    E.eixoA.propriedade === true && E.eixoA.estatuto === 'alojado' &&
    E.eixoA.nao.includes('G') && E.eixoA.nao.includes('bijeccao') &&
    E.eixoG.objecto === 'G' && E.eixoG.expressao === EXPR_GMEDE &&
    E.eixoG.propriedade === true && E.eixoG.estatuto === 'alojado' &&
    E.eixoG.nao.includes('Iso') && E.eixoG.nao.includes('Duo') &&
    compoeA(0, 0) === 0 && compoeA(1, 1) === 0 &&
    compoeA(0, 1) === 1 && compoeA(1, 0) === 1 &&
    eixosF(true, null).iso === false && eixosF(true, null).duo === false &&
    eixosF(true, 0).iso === true && eixosF(true, 1).duo === true &&
    E.testemunha.dobraA === true && E.testemunha.gum === true &&
    E.testemunha.bijDeG1 === true && E.testemunha.isoDeG1 === false &&
    E.testemunha.duoDeG1 === false &&
    E.testemunha.rhoInj === true && E.testemunha.rhoSurj === false &&
    B.testemunha.bij === true && B.testemunha.folgaZero === true &&
    C.testemunha.inj === true && C.testemunha.surj === false &&
    E.cria === false && E.tipoNovo === false && E.f8 === false &&
    dest.length === 2 && dest[0].objecto === 'G' && dest[0].estatuto === 'alojado' &&
    dest[1].objecto === 'Iso' && dest[1].estatuto === 'desconhecido' &&
    dest[1].motivo === 'estrutural' && dest[1].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_AA).ok === false && admiteLei(EXPR_GMEDE).ok === false &&
    !kinds.includes('Iso') && !kinds.includes('eixo') && !kinds.includes('especie') &&
    !kinds.includes('plana') && !kinds.includes('G') &&
    /regraGi=G=1 != Iso/.test(u.nota) &&
    /regraAx=eixo a != eixo G/.test(u.nota) &&
    /regra2e=duas especies != tres/.test(u.nota) &&
    /G=1 != Iso/.test(u.proibicao) &&
    /eixo a != eixo G/.test(u.proibicao) &&
    /duas especies != tres/.test(u.proibicao) &&
    /G=1 != Iso/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const I = dualInvolutiva()
  const dest = destinosDoTeorema('fis:thm:dual-involutiva')
  const E = eixosRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const uJson = JSON.parse(readFileSync(join(RAIZ, 'conecthus', 'schema', 'u.json'), 'utf8'))
  const ops = Object.freeze(['oplus', 'otimes'])
  ok('§K62 fis:thm:dual-involutiva: D^2=id estrutural; != Iso; G intacto; sem terceiro',
    REGRA_EQ === 'igualdade estrutural != Iso' &&
    REGRA_3S === 'duas dobras != terceiro estado' &&
    REGRA_DOP === 'D_op != D' &&
    REGRA_AX === 'eixo a != eixo G' &&
    REGRA_2E === 'duas especies != tres' &&
    I.origem === 'fis:thm:dual-involutiva' && I.funil === FUNIL_CASA &&
    I.mesmoObjecto && I.distintos &&
    I.d2.objecto === 'D_op' && I.d2.expressao === EXPR_D2 &&
    I.d2.propriedade === true && I.d2.estatuto === 'alojado' &&
    I.d2.nao.includes('Iso') && I.d2.nao.includes('G') && I.d2.nao.includes('terceiro') &&
    I.eq.objecto === 'D_op' && I.eq.expressao === EXPR_EQA &&
    I.eq.propriedade === true && I.eq.estatuto === 'alojado' &&
    I.eq.nao.includes('copia') &&
    dualOps(ops)[0] === 'otimes' && dualOps(dualOps(ops))[0] === 'oplus' &&
    dualOps(dualOps(ops))[1] === 'otimes' &&
    I.testemunha.troca === true && I.testemunha.mesma === true &&
    I.testemunha.a2 === 0 && I.testemunha.gIntacto === true &&
    I.testemunha.isoDeG1 === false &&
    E.testemunha.gum === true && E.testemunha.isoDeG1 === false &&
    uJson.star === 'D' &&
    I.cria === false && I.tipoNovo === false && I.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'D_op' && dest[0].estatuto === 'alojado' &&
    dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_D2).ok === false && admiteLei(EXPR_EQA).ok === false &&
    !kinds.includes('involucao') && !kinds.includes('D_op') && !kinds.includes('copia') &&
    /regraEq=igualdade estrutural != Iso/.test(u.nota) &&
    /regra3s=duas dobras != terceiro estado/.test(u.nota) &&
    /igualdade estrutural != Iso/.test(u.proibicao) &&
    /duas dobras != terceiro estado/.test(u.proibicao) &&
    /igualdade estrutural != Iso/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const C = duoComposicao()
  const dest = destinosDoTeorema('fis:thm:duo-composicao')
  const I = dualInvolutiva()
  const E = eixosRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K63 fis:thm:duo-composicao: tabua = paridade de a; != objecto; G intacto',
    REGRA_TAB === 'tabua != objecto' &&
    REGRA_SOMA === 'compor = somar a' &&
    REGRA_2E === 'duas especies != tres' &&
    REGRA_3S === 'duas dobras != terceiro estado' &&
    C.origem === 'fis:thm:duo-composicao' && C.funil === FUNIL_CASA &&
    C.mesmoObjecto && C.distintos &&
    C.tabua.objecto === 'a' && C.tabua.expressao === EXPR_TAB &&
    C.tabua.propriedade === true && C.tabua.estatuto === 'alojado' &&
    C.tabua.nao.includes('objecto') && C.tabua.nao.includes('G') && C.tabua.nao.includes('b') &&
    C.soma.objecto === 'a' && C.soma.expressao === EXPR_SOMA &&
    C.soma.propriedade === true && C.soma.estatuto === 'alojado' &&
    C.tabua.objecto === E.eixoA.objecto &&
    paridadeD(0) === 0 && paridadeD(1) === 1 && paridadeD(2) === 0 && paridadeD(3) === 1 &&
    compoeA(1, 1) === 0 && compoeA(0, 1) === 1 &&
    C.testemunha.fecha === true && C.testemunha.par === true &&
    C.testemunha.d2 === true && C.testemunha.gIntacto === true &&
    I.testemunha.mesma === true && E.testemunha.isoDeG1 === false &&
    C.cria === false && C.tipoNovo === false && C.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'a' && dest[0].estatuto === 'alojado' &&
    dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_TAB).ok === false && admiteLei(EXPR_SOMA).ok === false &&
    !kinds.includes('tabua') && !kinds.includes('paridade') && !kinds.includes('tabela') &&
    /regraTab=tabua != objecto/.test(u.nota) &&
    /regraSoma=compor = somar a/.test(u.nota) &&
    /tabua != objecto/.test(u.proibicao) &&
    /compor = somar a/.test(u.proibicao) &&
    /tabua != objecto/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const D = quartetoDuo()
  const S = quartetoN2()
  const dest = destinosDoTeorema('fis:thm:quarteto')
  const C = duoComposicao()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K64 fis:thm:quarteto: Q_sigma != Q_dt; quatro quadrantes != quatro objectos',
    REGRA_QAB2 === 'Q_sigma != Q_dt' &&
    REGRA_4Q === 'quatro quadrantes != quatro objectos' &&
    REGRA_TAB === 'tabua != objecto' &&
    REGRA_SUB === 'subestrutura reconhecida != novo objecto' &&
    D.origem === 'fis:thm:quarteto' && D.funil === FUNIL_CASA &&
    D.mesmoObjecto === false && D.distintos &&
    D.sigma.objecto === 'Q_ab' && D.sigma.expressao === EXPR_QAB &&
    D.sigma.casa === GERADOR_VIZDOBRA.id && D.sigma.estatuto === 'alojado' &&
    D.sigma.nao.includes('Q_dt') && D.sigma.n === 4 &&
    D.dt.objecto === 'a' && D.dt.expressao === EXPR_QDT &&
    D.dt.propriedade === true && D.dt.casa === '' && D.dt.estatuto === 'alojado' &&
    D.dt.nao.includes('Q_ab') && D.dt.nao.includes('quinta') && D.dt.n === 4 &&
    D.quad.objecto === 'a' && D.quad.expressao === EXPR_QUAD &&
    D.quad.propriedade === true && D.quad.estatuto === 'alojado' &&
    D.quad.nao.includes('objectos') &&
    S.objecto === 'Q_ab' && S.construcao === EXPR_QAB && S.casa === 'vizdobra' &&
    S.construcao !== EXPR_QDT && D.sigma.objecto !== D.dt.objecto &&
    D.testemunha.quatro === true && D.testemunha.fecha === true &&
    D.testemunha.invol === true && D.testemunha.rotulosIguais === true &&
    D.testemunha.constrDistintas === true && D.testemunha.objectosDistintos === true &&
    D.testemunha.qAbAge === true && D.testemunha.gIntacto === true &&
    parAB(1, 1).a === 1 && parAB(1, 1).b === 1 &&
    rotuloPar(compoePar(parAB(1, 0), parAB(1, 1))) === '01' &&
    rotuloPar(compoePar(parAB(1, 1), parAB(1, 1))) === '00' &&
    qAb(1, 0, 0) === ek(0) &&
    C.tabua.objecto === D.dt.objecto &&
    D.cria === false && D.tipoNovo === false && D.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'Q_ab' && dest[0].estatuto === 'alojado' &&
    dest[0].papel === 'gerador' && dest[0].tipoNovo === false &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_QDT).ok === false && admiteLei(EXPR_QUAD).ok === false &&
    !kinds.includes('quarteto') && !kinds.includes('Q_dt') && !kinds.includes('quadrante') &&
    /regraQab2=Q_sigma != Q_dt/.test(u.nota) &&
    /regra4q=quatro quadrantes != quatro objectos/.test(u.nota) &&
    /Q_sigma != Q_dt/.test(u.proibicao) &&
    /quatro quadrantes != quatro objectos/.test(u.proibicao) &&
    /Q_sigma != Q_dt/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const T = trialRamos()
  const dest = destinosDoTeorema('fis:thm:trial')
  const destQ = destinosDoTeorema('fis:thm:quarteto')
  const E = eixosRamos()
  const C = duoComposicao()
  const Qd = quartetoDuo()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K65 fis:thm:trial: centro != objecto; dois lados != duas casas; a=0,1 ja existem',
    REGRA_CTR === 'centro != objecto' &&
    REGRA_LD === 'dois lados != duas casas' &&
    REGRA_2E === 'duas especies != tres' &&
    REGRA_3S === 'duas dobras != terceiro estado' &&
    T.origem === 'fis:thm:trial' && T.funil === FUNIL_CASA &&
    T.mesmoObjecto && T.distintos &&
    T.centro.objecto === 'a' && T.centro.expressao === EXPR_CTR &&
    T.centro.propriedade === true && T.centro.papel === 'leitura' &&
    T.centro.casa === LEITURA_COORD.id && T.centro.estatuto === 'alojado' &&
    T.centro.nao.includes('objecto') && T.centro.nao.includes('absorcao') &&
    T.centro.nao.includes('vinco') &&
    T.lado0.objecto === 'a' && T.lado0.expressao === EXPR_L0 &&
    T.lado0.propriedade === true && T.lado0.casa === '' &&
    T.lado0.estatuto === 'alojado' && T.lado0.nao.includes('casa') &&
    T.lado1.objecto === 'a' && T.lado1.expressao === EXPR_L1 &&
    T.lado1.propriedade === true && T.lado1.casa === '' &&
    T.lado1.estatuto === 'alojado' && T.lado1.nao.includes('casa') &&
    T.centro.objecto === T.lado0.objecto && T.lado0.objecto === T.lado1.objecto &&
    T.centro.objecto === E.eixoA.objecto && T.centro.objecto === C.tabua.objecto &&
    paridadeD(0) === 0 && paridadeD(1) === 1 &&
    eixosF(true, 0).iso === true && eixosF(true, 0).duo === false &&
    eixosF(true, 1).duo === true && eixosF(true, 1).iso === false &&
    T.testemunha.iso0 === true && T.testemunha.duo0 === false &&
    T.testemunha.iso1 === false && T.testemunha.duo1 === true &&
    T.testemunha.d0 === true && T.testemunha.d1 === true &&
    T.testemunha.soma === true && T.testemunha.d2 === true &&
    T.testemunha.duasEspecies === true &&
    T.testemunha.qAbSigma === true && T.testemunha.qAbDistintos === true &&
    Qd.sigma.objecto !== Qd.dt.objecto &&
    T.cria === false && T.tipoNovo === false && T.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'a' && dest[0].estatuto === 'alojado' &&
    dest[0].tipoNovo === false &&
    destQ.length === 1 && destQ[0].objecto === 'Q_ab' &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_CTR).ok === false && admiteLei(EXPR_L0).ok === false &&
    admiteLei(EXPR_L1).ok === false &&
    !kinds.includes('trial') && !kinds.includes('centro') &&
    !kinds.includes('lado') && !kinds.includes('ramo') &&
    /regraCtr=centro != objecto/.test(u.nota) &&
    /regraLd=dois lados != duas casas/.test(u.nota) &&
    /centro != objecto/.test(u.proibicao) &&
    /dois lados != duas casas/.test(u.proibicao) &&
    /centro != objecto/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const F = duasFacesRamos()
  const dest = destinosDoTeorema('fis:thm:duasfaces')
  const destT = destinosDoTeorema('fis:thm:trial')
  const destQ = destinosDoTeorema('fis:thm:quarteto')
  const destD = destinosDoTeorema('fis:def:duomorf')
  const Tr = trialRamos()
  const O = octoniaoObs()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  ok('§K66 fis:thm:duasfaces: duas faces != dois objectos; equacao != nova lei',
    REGRA_2F === 'duas faces != dois objectos' &&
    REGRA_EQN === 'equacao != nova lei' &&
    REGRA_2E === 'duas especies != tres' &&
    REGRA_CTR === 'centro != objecto' &&
    F.origem === 'fis:thm:duasfaces' && F.funil === FUNIL_CASA &&
    F.mesmoObjecto && F.distintos &&
    F.face0.objecto === 'a' && F.face0.expressao === EXPR_F0 &&
    F.face0.propriedade === true && F.face0.casa === '' &&
    F.face0.estatuto === 'alojado' && F.face0.nao.includes('objecto') &&
    F.face1.objecto === 'a' && F.face1.expressao === EXPR_F1 &&
    F.face1.propriedade === true && F.face1.casa === '' &&
    F.face1.estatuto === 'alojado' &&
    F.eq.objecto === 'a' && F.eq.expressao === EXPR_EQF &&
    F.eq.propriedade === true && F.eq.estatuto === 'alojado' &&
    F.eq.nao.includes('lei') && F.eq.nao.includes('estrela') &&
    F.eq.nao.includes('rotor') && F.eq.nao.includes('terceira') &&
    F.face0.objecto === F.face1.objecto && F.face1.objecto === F.eq.objecto &&
    F.face0.objecto === Tr.centro.objecto &&
    F.face0.objecto !== O.faces.objecto && EXPR_F0 !== EXPR_FACES &&
    paridadeD(0) === 0 && paridadeD(1) === 1 &&
    eixosF(true, 0).iso === true && eixosF(true, 1).duo === true &&
    F.testemunha.iso0 === true && F.testemunha.duo1 === true &&
    F.testemunha.trialA === true && F.testemunha.umaEquacao === true &&
    F.testemunha.octFaces === 'faces' &&
    F.cria === false && F.tipoNovo === false && F.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'a' && dest[0].estatuto === 'alojado' &&
    dest[0].tipoNovo === false &&
    destT.length === 1 && destT[0].objecto === 'a' &&
    destQ.length === 1 && destQ[0].objecto === 'Q_ab' &&
    destD.length === 2 && destD[1].objecto === 'Duo' && destD[1].estatuto === 'desconhecido' &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_F0).ok === false && admiteLei(EXPR_F1).ok === false &&
    admiteLei(EXPR_EQF).ok === false &&
    !kinds.includes('faces') && !kinds.includes('equacao') &&
    !kinds.includes('interface') && !kinds.includes('trial') &&
    /regra2f=duas faces != dois objectos/.test(u.nota) &&
    /regraEqn=equacao != nova lei/.test(u.nota) &&
    /duas faces != dois objectos/.test(u.proibicao) &&
    /equacao != nova lei/.test(u.proibicao) &&
    /duas faces != dois objectos/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const S = estrelaRamos()
  const dest = destinosDoTeorema('fis:thm:estrela')
  const destQ = destinosDoTeorema('fis:thm:quarteto')
  const destF = destinosDoTeorema('fis:thm:duasfaces')
  const F = duasFacesRamos()
  const E = eixosRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const uJson = JSON.parse(readFileSync(join(RAIZ, 'conecthus', 'schema', 'u.json'), 'utf8'))
  ok('§K67 fis:thm:estrela: Star(U)=D != x^2=x+1; condicao desconhecida em B',
    REGRA_STU === 'Star(U) != x^2=x+1' &&
    REGRA_STQ === 'Star != Q_ab' &&
    REGRA_STF8 === 'Star != F8' &&
    REGRA_STG === 'Star != novo gerador' &&
    REGRA_AX === 'eixo a != eixo G' &&
    S.origem === 'fis:thm:estrela' && S.funil === FUNIL_CASA &&
    S.mesmoObjecto && S.distintos &&
    S.face0.objecto === 'a' && S.face0.expressao === EXPR_F0 &&
    S.face0.estatuto === 'alojado' && S.face0.casa === '' &&
    S.face1.objecto === 'a' && S.face1.expressao === EXPR_F1 &&
    S.face1.estatuto === 'alojado' &&
    S.st.objecto === 'a' && S.st.expressao === EXPR_ST &&
    S.st.propriedade === true && S.st.estatuto === 'desconhecido' &&
    S.st.motivo === 'estrutural' && S.st.casa === '' &&
    S.st.nao.includes('Q_ab') && S.st.nao.includes('F8') &&
    S.st.nao.includes('gerador') && S.st.nao.includes('D') &&
    S.face0.objecto === S.face1.objecto && S.face1.objecto === S.st.objecto &&
    S.face0.objecto === F.face0.objecto && S.face0.objecto === E.eixoA.objecto &&
    EXPR_ST !== EXPR_EQF && EXPR_ST !== EXPR_FACES &&
    estrelaEmB() === false && S.testemunha.cabem === false &&
    S.testemunha.starU === 'D' && uJson.star === 'D' &&
    S.testemunha.iso0 === true && S.testemunha.duo1 === true &&
    S.testemunha.eixoA === 'a' && S.testemunha.qAb === 'Q_ab' &&
    S.testemunha.gIntacto === true && E.testemunha.isoDeG1 === false &&
    S.cria === false && S.tipoNovo === false && S.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'a' && dest[0].estatuto === 'alojado' &&
    dest[0].tipoNovo === false &&
    destF.length === 1 && destF[0].objecto === 'a' &&
    destQ.length === 1 && destQ[0].objecto === 'Q_ab' &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_ST).ok === false &&
    !kinds.includes('estrela') && !kinds.includes('Star') && !kinds.includes('phi') &&
    /regraStu=Star\(U\) != x\^2=x\+1/.test(u.nota) &&
    /regraStq=Star != Q_ab/.test(u.nota) &&
    /Star\(U\) != x\^2=x\+1/.test(u.proibicao) &&
    /Star != Q_ab/.test(u.proibicao) &&
    /Star != F8/.test(u.proibicao) &&
    /Star != novo gerador/.test(u.proibicao) &&
    /Star\(U\) != x\^2=x\+1/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const E = estrelaEncaixe()
  const dest = destinosDoTeorema('fis:thm:estrelaencaixe')
  const destS = destinosDoTeorema('fis:thm:estrela')
  const S = estrelaRamos()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const uJson = JSON.parse(readFileSync(join(RAIZ, 'conecthus', 'schema', 'u.json'), 'utf8'))
  ok('§K68 fis:thm:estrelaencaixe: Zc realiza; != promover em B; sem kind',
    REGRA_ZCB === 'realizar em Zc != promover em B' &&
    REGRA_STU === 'Star(U) != x^2=x+1' &&
    REGRA_DOIS === '2 != 0 em V/Zc nao autoriza 2mapsto0 em B' &&
    E.origem === 'fis:thm:estrelaencaixe' && E.funil === FUNIL_CASA &&
    E.mesmoObjecto && E.distintos &&
    E.enc.objecto === 'a' && E.enc.expressao === EXPR_ENC &&
    E.enc.realizacao === 'Zc' && E.enc.propriedade === true &&
    E.enc.estatuto === 'alojado' && E.enc.casa === '' &&
    E.enc.nao.includes('categoria') && E.enc.nao.includes('B') &&
    E.emB.objecto === 'a' && E.emB.expressao === EXPR_ST &&
    E.emB.realizacao === 'B' && E.emB.estatuto === 'desconhecido' &&
    E.emB.motivo === 'estrutural' && E.emB.nao.includes('promocao') &&
    E.enc.objecto === E.emB.objecto && E.enc.realizacao !== E.emB.realizacao &&
    E.enc.objecto === S.st.objecto && S.st.estatuto === 'desconhecido' &&
    estrelaEmB() === false && estrelaEmZc() === true &&
    E.testemunha.zc === true && E.testemunha.b === false &&
    E.testemunha.cassini[0] === 1 && E.testemunha.cassini[1] === -1 &&
    E.testemunha.cassini[2] === 1 && fibU(0) === 0 && fibU(1) === 1 &&
    fibU(5) === 5 && cassini(0) === 1 &&
    E.testemunha.starU === 'D' && uJson.star === 'D' &&
    E.cria === false && E.tipoNovo === false && E.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'a' && dest[0].estatuto === 'alojado' &&
    dest[0].tipoNovo === false &&
    destS.length === 1 && destS[0].objecto === 'a' &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_ENC).ok === false &&
    !kinds.includes('encaixe') && !kinds.includes('fibonacci') &&
    !kinds.includes('Zc') && !kinds.includes('estrela') &&
    /regraZcb=realizar em Zc != promover em B/.test(u.nota) &&
    /realizar em Zc != promover em B/.test(u.proibicao) &&
    /realizar em Zc != promover em B/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

{
  const D = duoEstrelaRamos()
  const dest = destinosDoTeorema('fis:thm:duoestrela')
  const destDef = destinosDoTeorema('fis:def:duoestrela')
  const destT = destinosDoTeorema('fis:def:dominio-interface')
  const destDuo = destinosDoTeorema('fis:def:duomorf')
  const destI = destinosDoTeorema('fis:obs:octoniao-interface')
  const S = estrelaRamos()
  const Enc = estrelaEncaixe()
  const kinds = schema.properties.kind.enum
  const u = leiParaU(cat)
  const uJson = JSON.parse(readFileSync(join(RAIZ, 'conecthus', 'schema', 'u.json'), 'utf8'))
  ok('§K69 fis:thm:duoestrela: reversivel != realizacao; Duo != Star != Iso',
    REGRA_REV === 'reversivel != nova realizacao' &&
    REGRA_DS === 'Duo != Star' &&
    REGRA_RISO === 'Duo reversivel != Iso' &&
    REGRA_ITR === 'interface != transporte' &&
    REGRA_BID === 'Iso != Duo' &&
    D.origem === 'fis:thm:duoestrela' && D.def === 'fis:def:duoestrela' &&
    D.funil === FUNIL_CASA && D.mesmoObjecto && D.distintos &&
    D.face0.objecto === 'a' && D.face0.expressao === EXPR_F0 &&
    D.face1.objecto === 'a' && D.face1.expressao === EXPR_F1 &&
    D.rev.objecto === 'a' && D.rev.expressao === EXPR_REV &&
    D.rev.propriedade === true && D.rev.estatuto === 'alojado' &&
    D.rev.casa === '' && D.rev.nao.includes('Iso') && D.rev.nao.includes('T_fio') &&
    D.rev.nao.includes('realizacao') &&
    D.face0.objecto === D.face1.objecto && D.face1.objecto === D.rev.objecto &&
    compoeA(1, 1) === 0 && eixosF(true, 1).duo === true &&
    eixosF(true, 1).iso === false && eixosF(true, 0).iso === true &&
    D.testemunha.volta === true && D.testemunha.d2 === true &&
    D.testemunha.duo === true && D.testemunha.isoDuo === false &&
    D.testemunha.starU === 'D' && uJson.star === 'D' &&
    D.testemunha.stB === 'desconhecido' && S.st.estatuto === 'desconhecido' &&
    D.testemunha.encZc === 'alojado' && Enc.enc.estatuto === 'alojado' &&
    D.testemunha.iface === 'desconhecido' && D.testemunha.tfio === 'desconhecido' &&
    D.cria === false && D.tipoNovo === false && D.f8 === false &&
    dest.length === 1 && dest[0].objecto === 'a' && dest[0].estatuto === 'alojado' &&
    destDef.length === 1 && destDef[0].objecto === 'a' &&
    destDuo[1].objecto === 'Duo' && destDuo[1].estatuto === 'desconhecido' &&
    destI[1].objecto === 'interface' && destI[1].estatuto === 'desconhecido' &&
    destT[1].objecto === 'T_fio' && destT[1].estatuto === 'desconhecido' &&
    LEITURAS.length === 1 && GERADORES.length === 1 && CASAS.espaco === 1 &&
    CASAS.leituras === 1 && REALIZACOES.length === 2 && ONTOLOGIA.length === 5 &&
    admiteLei(EXPR_REV).ok === false &&
    !kinds.includes('duoestrela') && !kinds.includes('reversivel') &&
    !kinds.includes('interface') && !kinds.includes('Star') &&
    /regraRev=reversivel != nova realizacao/.test(u.nota) &&
    /regraDs=Duo != Star/.test(u.nota) &&
    /reversivel != nova realizacao/.test(u.proibicao) &&
    /Duo != Star/.test(u.proibicao) &&
    /Duo reversivel != Iso/.test(u.proibicao) &&
    /reversivel != nova realizacao/.test(man.corpos?.motor?.nucleo?.lei_gk || '') &&
    cat.length === 5)
}

ok('§K2 descricao(foo) = descricao(bar)',
  igualCatalogoLeis(cat, catalogoLeis(gkMan, pecas)) &&
  igualLei(montaLei(LEI_FASE_AURA, prova.orig), lei))

{
  const recusa = disparaLei(null, ID_FASE_AURA, { realizar: true, cena: true, executar: true })
  ok('§K3 conhecer recusa realizar e cena',
    recusa.conheceu === true &&
    recusa.realizou === false &&
    recusa.cena === false &&
    recusa.recusou === true &&
    recusa.ciclo === 'conhecer')
}

{
  const ls = memoriaLS()
  const foo = discoIsolado(ls, 'foo')
  const bar = discoIsolado(ls, 'bar')
  disparaLei(foo, ID_FASE_AURA)
  disparaLei(bar, 'outra')
  const e = estadoVazio()
  gravaEstado(e, ls)
  ok('§K4 conhecimento(foo) perp (bar); S_ESTADO intocado',
    leLeiSelecionada(foo) === ID_FASE_AURA &&
    leLeiSelecionada(bar) === 'outra' &&
    ls.getItem(CHAVE_LEI) === null &&
    chaveIsolada(CHAVE_LEI) &&
    chaveIsolada(CHAVE_CADEIA + 'x') &&
    !chaveIsolada(CHAVE_ESTADO) &&
    JSON.parse(ls.getItem(CHAVE_ESTADO)).magia === MAGIA &&
    !JSON.stringify(leEstado(ls)).includes('fase-aura'))
}

const idFoo = await ligaIdentidade(discoIsolado(memoriaLS(), 'foo'), { chave: pub })
const idBar = await ligaIdentidade(discoIsolado(memoriaLS(), 'bar'), { chave: pub })
const idHex = await idEstavelDaChave(pub)
ok('§K4 mesma chave → mesmo id(K) nos dois tenants',
  idFoo.id === idHex && idBar.id === idHex && idFoo.id !== 'foo')

ok('§K5 front nao importa lei; ponte nao pinta nem executa',
  !/banco_lei_gk_u/.test(front) &&
  !/from ['"]\.\/banco_disco\.js['"]/.test(ponte) &&
  !/gravaEstado/.test(ponte) &&
  !/CHAVE_ESTADO/.test(ponte) &&
  /GKBANCO/.test(discoSrc) &&
  !/from ['"]\.\/banco_front\.js['"]/.test(ponte) &&
  !/from ['"]\.\/motor_campo\.js['"]/.test(ponte) &&
  !/from ['"]\.\/cards_kernel\.js['"]/.test(ponte) &&
  !/WebGL/.test(ponte) &&
  !/banco_lei_gk/.test(cardsSrc) &&
  existsSync(INV))

{
  const extra = []
  function walk (n) {
    for (const k of Object.keys(n || {})) {
      if (!(k in schema.properties)) extra.push((n.kind || '?') + '.' + k)
    }
    if (n?.faces?.menos) walk(n.faces.menos)
    if (n?.faces?.mais) walk(n.faces.mais)
    for (const f of n?.filhos || []) walk(f)
  }
  walk(leiParaU(cat))
  ok('§K6 U da lei cabe no schema', extra.length === 0)
}

ok('§K5 inventario nao importa lei_gk (tomografia != ingestao)',
  !/banco_lei_gk_u/.test(readFileSync(INV, 'utf8')))

console.log('')
if (!falhas) {
  console.log('  lei_gk: CONGELADO; 92 cards → 5 leis; so triade cruz=realizado; sem realizar.')
}
console.log(`#TOTAL ${feitas} ${falhas}`)
process.exit(falhas ? 1 : 0)
