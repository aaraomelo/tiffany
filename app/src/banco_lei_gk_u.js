// banco_lei_gk_u.js — ingerir leis que os cards declaram, não cards nem shaders.
// card → declaração → lei → instrumento. Cards são evidências, não donos.
// CONGELADO: funil expressão → testemunhas → limiar → cruzamento. Sem F8.
// lei ≠ gerador ≠ realização. Vinco 2m=a+b ≠ unidade ≠ estrela. σ_i ≠ σ·σ′.
// Grafo: vizdobra gera X; X realiza afim; afim fixa vinco. Leitura fora da cadeia.
// Conteúdo de X: ⟨σ₁,σ₂,σ₃⟩ gera dim=8, e_k=2^k, Gram=Id. Construção ≠ lei.
// Teorema do paper ⇏ tipo no banco. Funil: papel, senão relação. Sem 6º papel.
// desconhecido ≠ ausente ≠ novo tipo. rotor (fecho) e DTC (relógio/inversor) sem casa.
// Busca do fecho: R_ij no plano de X; nenhuma de gera/realiza/fixa/le; permanece desconhecido.
// Identidade = expressão + suporte + realização. localizado ∧ incompatível ⇒ desconhecido.
// fis:thm:base(3) peso por direcção = leitura sobre B; régua do card ≠ g^{(k)}.
// fis:def:dual: paridade aterra em leitura; χ pede Zc e permanece desconhecido.
// Teorema ≠ objecto. Destino por objecto; o mesmo enunciado pode ramificar.
// Mesma expressão ⇏ mesmo objecto. Mesmo suporte ⇏ mesma realização.
// Mesmo enunciado ⇏ mesmo objecto. ⊕ ∈ X; * ∈ V. C_f ≠ hat ≠ H.
// Alojamento é reconhecimento, não criação. fis:thm:quarteto Q_ab(σ) aterra em vizdobra n=2.
// Q_ab(σ)=σ₁^a σ₂^b ≠ Q_ab(D,†)=D^a †^b. Mesmo rótulo ⇏ fusão. Quatro quadrantes ≠ quatro objectos.
// Subestrutura reconhecida ≠ novo objecto ontológico. Corte n=2 ≠ segundo gerador.
// Elemento/subestrutura ⊂ construção ≠ novo objecto. σᵢ ≠ Q_ab ≠ vizdobra como kind.
// Propriedade interna ⇏ entrada em ONTOLOGIA. Localizado em X ⇏ pertence a vizdobra.
// Suporte ≠ pertencimento. Vive em X ⇏ gerado por vizdobra.
// Ontologia fechada por contenção. Funil: localizar>suporte>expressao>realizacao>relacao>papel.
// fis:def:doisandares: X aterra em espaço; V=Zc^X permanece desconhecido.
// Objecto em V pede casa de V. Convolução/C_f/hat/H não alojam. xor em X ≠ * em V.
// C_f ≠ * ; hat ≠ χ ≠ paridade ; H ≠ hat. Factor 2^m não traduz para B.
// 2≠0 em V/Zc não autoriza 2↦0 em B. X em B não contamina objectos de V.
// Símbolo igual ⇏ operação igual. 2_V ≠ 2_B.
// Espaço fixo: X dim=8, e_k=2^k, 3 dobras geradoras. Afim = realização da 3ª
// dobra (∂x=a+b-x), não substitui Pontryagin, não é associador, não é 6ª lei.
// Eixos independentes: status interno (conhecida) ≠ cruzamento (realizado|relacionado).
// Cruzamento não vaza para realizar/cena. Reino ⇏ herdar fisica.tex; paper ⇏ declarar
// realizado o que o Reino só relaciona. conhecer ≠ realizar ≠ cena.
// op ≠ régua ≠ tag ≠ fórmula. Limiar = 12 testemunhas da mesma expressão.
// n=k é parâmetro, não lei. Não toca banco_front / disco / S_ESTADO. Não pinta. Não GLSL.

import { completa } from './banco_schema.js'
import { catalogoCards } from './banco_cards_u.js'
import { FASE_AURA, ROT_REGUA, ROT_FP, ROT_REVELA, faseNaDesc } from './banco_inventario_u.js'

export const ID_LEI_GK = 'gk'
export const CHAVE_LEI = 'gk:reino:lei'
/** Objecto da ingestão: a declaração, não o cartão. */
export const PIPE = Object.freeze(['card', 'declaracao', 'lei', 'instrumento'])
/** Só o primeiro ciclo corre agora. */
export const CICLOS = Object.freeze(['conhecer', 'realizar', 'cena'])
export const CICLO = 'conhecer'
export const CAMPOS_DECL = Object.freeze(['op', 'regua', 'tag', 'formula'])
/** Segunda varredura: estes campos, depois normalizar. */
export const CAMPOS_VARREDURA = Object.freeze(['regua', 'fp', 'revela', 'pbr', 'dual', 'formula'])
export const LIMIAR_TESTEMUNHAS = 12
/** Contrato congelado: lei nova entra só por este funil. Não abre F8. */
export const CONGELADO = true
export const FUNIL = Object.freeze(['expressao', 'testemunhas', 'limiar', 'cruzamento'])
export const EIXOS = Object.freeze(['interno', 'cruzamento'])
export const STATUS_INTERNO = Object.freeze(['conhecida', 'observada'])
export const CRUZ = Object.freeze(['realizado', 'relacionado'])
/** Lei do catálogo ≠ gerador das dobras ≠ realização no suporte. Não colapsar. */
export const PAPEIS = Object.freeze(['lei', 'gerador', 'realizacao'])
/** Ontologia completa. PAPEIS ⊂ isto. Espaço e leitura não sobem a lei. */
export const ONTOLOGIA = Object.freeze(['lei', 'gerador', 'espaco', 'realizacao', 'leitura'])
/** Campos congelados por papel. Fórmula ≠ lei. */
export const CAMPOS_LEI = Object.freeze(['expressao', 'testemunhas', 'instrumento'])
export const CAMPOS_GERADOR = Object.freeze(['objeto', 'estados', 'origem'])
export const CAMPOS_REALIZACAO = Object.freeze(['objeto', 'origem', 'endereco'])
export const CAMPOS_ESPACO = Object.freeze(['dim', 'base', 'gram'])
export const CAMPOS_LEITURA = Object.freeze(['objeto', 'origem', 'observado'])
export const FUNIL_OBJECTO = Object.freeze(['expressao', 'classificacao', 'papel', 'cruzamento'])
/** Teorema do paper ⇏ tipo no banco. Primeiro o papel; senão a relação. */
export const FUNIL_TEOREMA = Object.freeze(['papel', 'relacao'])
/** Teorema → objecto existente → relação → alojar. Sem tipo novo. */
export const FUNIL_BUSCA = Object.freeze(['objecto', 'relacao'])
/** Identidade cruzada: expressão ≠ suporte ≠ realização. Símbolo não basta. */
export const CAMPOS_IDENTIDADE = Object.freeze(['expressao', 'suporte', 'realizacao'])
/** Um teorema pode ter vários objectos; o destino é por objecto. Sem tipo novo. */
export const FUNIL_DESTINO = Object.freeze(['teorema', 'objecto', 'identidade', 'destino'])
/** Alojamento fecha só com os cinco. Expressão ou suporte sozinhos não bastam. */
export const CAMPOS_ALOJAR = Object.freeze(['objecto', 'expressao', 'suporte', 'realizacao', 'relacao'])
/** Dois desconhecidos distintos. Não são a mesma recusa. */
export const MOTIVO_DESCONHECIDO = Object.freeze(['estrutural', 'algebrica'])
/** Alojamento: desconhecido ≠ ausente ≠ novo tipo. Novo tipo nunca. */
export const ESTATUTO_ALOJAMENTO = Object.freeze(['alojado', 'desconhecido', 'ausente'])
/** Alojamento reconhece casa existente. Não cria tipo, gerador nem lei. */
export const ALOJAR_E = 'reconhecimento'
/** Corte de gerador existente ≠ objecto ontológico novo. Quatro estados ≠ quatro geradores. */
export const REGRA_SUB = 'subestrutura reconhecida != novo objecto'
/** Grau na construção existente. Não entra na ontologia nem em kind. */
export const GRAU_NA = Object.freeze(['elemento', 'subestrutura', 'construcao'])
export const REGRA_GRAU = 'elemento/subestrutura subset construcao != novo objecto'
/** Grau, corte e cardinalidade: propriedades da construção, não papéis do banco. */
export const PROPS_INTERNAS = Object.freeze(['grau', 'corte', 'cardinalidade'])
export const REGRA_PROP = 'propriedade interna notRightarrow entrada em ONTOLOGIA'
/** Estar em X não é ser elemento da construção que gera X. */
export const REGRA_SUPORTE = 'suporte != pertencimento a construcao'
export const REGRA_VIVE = 'vive em X notRightarrow gerado por vizdobra'
/** Ontologia fechada por contenção. Papel já existente, ou desconhecido. Sem 6º papel. */
export const ONTOLOGIA_FECHADA = true
export const FUNIL_CASA = Object.freeze(['localizar', 'suporte', 'expressao', 'realizacao', 'relacao', 'papel'])
export const REGRA_ONTO = 'ontologia fechada por contencao'
export const CASAS = Object.freeze({ leis: 5, geradores: 1, espaco: 1, realizacoes: 2, leituras: 1 })
/** Papel espaço: um ocupante. V e todo espaço citado não herdam X. */
export const ESPACO_UNICO = true
export const REGRA_V = 'objecto em V pede casa de V'
export const REGRA_ENUN = 'mesmo enunciado notRightarrow mesmo objecto'
export const REGRA_FATOR = 'fator nao traduz para B'
export const REGRA_DOIS = '2 != 0 em V/Zc nao autoriza 2mapsto0 em B'
export const REGRA_SIM = 'simbolo igual notRightarrow operacao igual'
export const REGRA_2VB = '2_V != 2_B'
export const REGRA_CASA_LE = 'outra leitura do mesmo suporte nao cria casa'
export const REGRA_INJ = 'injetivo != invertivel'
export const REGRA_FG = 'xor != * != F(f)F(g)'
export const REGRA_OFICIO = 'oficio != categoria'
export const REGRA_PARTE = 'parte nao acrescenta objectos'
export const REGRA_VE = 'V_E != V'
export const REGRA_TRESGEO = 'tres geometrias != tres objectos'
export const REGRA_D = 'd_ultra != d_Hamming != d_percurso'
export const REGRA_SUP_EX = 'mesmo suporte notRightarrow mesma expressao'
export const REGRA_EX_OBJ = 'expressoes distintas notRightarrow objectos ontologicos distintos'
export const REGRA_PROP_EX = 'propriedade da leitura != realizacao da expressao'
export const REGRA_BB = 'B_cal != B'
export const REGRA_NT = '|N_T(x)|=1 != 2n'
export const REGRA_IT = 'I != T'
export const REGRA_PG = 'percurso != grafo'
export const REGRA_EULER = 'Euler != gerador'
export const REGRA_CICLO = 'ciclo != lei'
export const REGRA_HS = 'handshake != Euler'
export const REGRA_INC = 'incidencia != grafo'
export const REGRA_GH = 'grade != hipercubo'
export const REGRA_PM = '2n != m'
export const REGRA_SG = 'sigma_grade != sigma_B'
export const REGRA_PARES = 'n pares != 2n soltos'
export const REGRA_ORD = 'ordem 4 != ordem 2'
export const REGRA_PLANOS = 'binom(n,2) != 2n'
export const REGRA_CL = 'codificar != linearizar'
export const REGRA_PS = 'Peano != serpentina'
export const REGRA_IVG = 'I != V_grade'
export const REGRA_IC = 'indice != conteudo'
export const REGRA_IXI = 'IxI != I'
export const REGRA_PAL = 'palavra != espaco'
export const REGRA_TT = 'T_letra != T'
export const REGRA_ALF = 'alfabeto != A'
export const REGRA_CC = 'comuta != cancela'
export const REGRA_G1 = 'G>1 != gerador'
export const REGRA_KER = 'ker w != nucleo'
export const REGRA_PR = 'pi != rho'
export const REGRA_BUR = 'buraco != dobra'
export const REGRA_FOL = 'folga != Duo'
export const REGRA_MESMA = 'mesma folga != dois objectos'
export const REGRA_BIJ = 'bijeccao != Iso'
export const REGRA_BV = 'bijeccao != vizinhanca'
export const REGRA_POS = 'posicional != vizinhanca'
export const REGRA_VS = 'varphi != s'
export const REGRA_ACC = 'compor != acumular'
export const REGRA_DX = 'd_x != D'
export const REGRA_TR = 'T_trav != T'
export const REGRA_PX = 'preco em pi != X'
export const REGRA_UD = 'unica != duomorfa'
export const REGRA_BID = 'Iso != Duo'
export const REGRA_DOP = 'D_op != D'
export const REGRA_IF = 'interface != categoria'
export const REGRA_HX = 'hexal != Duo'
export const REGRA_LF = 'ligar != fundir'
export const REGRA_ITR = 'interface != transporte'
export const REGRA_TF = 'T_fio != T'
export const REGRA_QM = 'Q_ab != operadores de M'
export const REGRA_GI = 'G=1 != Iso'
export const REGRA_AX = 'eixo a != eixo G'
export const REGRA_2E = 'duas especies != tres'
export const REGRA_EQ = 'igualdade estrutural != Iso'
export const REGRA_3S = 'duas dobras != terceiro estado'
export const REGRA_TAB = 'tabua != objecto'
export const REGRA_SOMA = 'compor = somar a'
export const REGRA_QAB2 = 'Q_sigma != Q_dt'
export const REGRA_4Q = 'quatro quadrantes != quatro objectos'
export const REGRA_CTR = 'centro != objecto'
export const REGRA_LD = 'dois lados != duas casas'
export const REGRA_2F = 'duas faces != dois objectos'
export const REGRA_EQN = 'equacao != nova lei'
export const REGRA_STU = 'Star(U) != x^2=x+1'
export const REGRA_STQ = 'Star != Q_ab'
export const REGRA_STF8 = 'Star != F8'
export const REGRA_STG = 'Star != novo gerador'
export const REGRA_ZCB = 'realizar em Zc != promover em B'
export const REGRA_REV = 'reversivel != nova realizacao'
export const REGRA_DS = 'Duo != Star'
export const REGRA_RISO = 'Duo reversivel != Iso'
export const PENDENTES_V = Object.freeze(['conv', 'C_f', 'hat', 'H'])
/** Espaço de U: 3 dobras geradoras → 8 leis. Não são 8 axiomas. Não Lei 8. */
export const DIM_X = 8
export const DOBRAS_GERADORAS = Object.freeze([1, 2, 3])
export const LEIS_BASE = Object.freeze([0, 1, 2, 3, 4, 5, 6, 7])
export const EXPR_AFIM = '∂x=a+b-x'
export const EXPR_VINCO = '2m=a+b'
export const ESPACO = Object.freeze({
  id: 'X',
  papel: 'espaco',
  dim: DIM_X,
  base: 'e_k=2^k',
  dobras: 3,
  leis: 8,
  gram: 'Id',
  coord: '<b,e_k>=(b>>k) AND 1',
  endereco: '(x,k) horizontal/vertical',
  lei_gk: 'N/A',
  gerador: 'N/A',
  refs: Object.freeze([
    'univ:def:espaco', 'univ:thm:base', 'univ:thm:oito-de-tres',
    'fis:thm:base', 'fis:def:coord2',
  ]),
})
export const EXPR_BASE = 'e_k=2^k'
export const EXPR_LEITURA = '<b,e_k>=(b>>k) AND 1'
export const EXPR_PESO = 'g^(k)(b)=b AND 2^k'
export const EXPR_PAR = 'paridade(k AND j)'
export const EXPR_CHI = 'chi_k(j)=(-1)^<k,j>'
export const EXPR_V = 'V=Zc^X'
export const EXPR_XOR = 'a XOR b'
export const EXPR_CONV = 'delta_a * delta_b = delta_{a XOR b}'
export const EXPR_CF = 'C_f(g)=f*g'
export const EXPR_HAT = 'hat(f)(k)=sum f(j) chi_k(j)'
export const EXPR_H = 'H^2=2^m Id'
export const EXPR_NORMA_V = '||f||^2=sum f(x)f(x)'
export const EXPR_2P = '|I|+2P'
export const EXPR_HAM = '|k|=sum coord'
export const EXPR_ALPHA = 'alpha=sum delta_e_i'
export const EXPR_A = 'A=C_alpha'
export const EXPR_AUTO = 'm-2|k|'
export const EXPR_FG = 'F(f*g)=F(f)F(g)'
export const EXPR_INJ = 'hat f != 0 => C_f injetivo'
export const EXPR_INV = 'C_f invertivel iff hat f invertivel em Zc'
export const EXPR_END = 'enderecar: o que esta ao lado'
export const EXPR_VE = 'V_E(x)=x XOR E'
export const EXPR_VIZ = 'V(x) subset X, simetrica'
export const EXPR_QM = '|a XOR b|'
export const EXPR_PROF = 'prof=primeira divergencia MSB'
export const EXPR_DULTRA = 'd=2^{-prof}'
export const EXPR_PERC = 'd_percurso'
export const EXPR_SIMETRIA = 'd(a,b)=d(b,a)'
export const EXPR_ULTRA = 'd(a,c)<=max'
export const EXPR_BOLA = 'B_cal(a,r)={x: d<=r}'
export const EXPR_PREF = 'prefixo MSB'
export const EXPR_COND_D = 'd(a,x)<=r'
export const EXPR_PIR = 'pi_r(i)=bola que contem i'
export const EXPR_NT = 'N_T(x)=irmao prefixo'
export const EXPR_CARD_T = '|N_T(x)|=1'
export const EXPR_CAMINHO = 'pi(t+1) in N_T(pi(t))'
export const EXPR_CHI_E = 'chi=v-a'
export const EXPR_CICLO = 'cada dobra fecha um ciclo'
export const EXPR_HS = 'sum G_ar=2G-pontas'
export const EXPR_INC = 'vertice <-> aresta'
export const EXPR_GRADE = 'X gerado por e_i em Zc'
export const EXPR_VIZ_G = 'V(x)={x±e_i}'
export const EXPR_DIM = '|V(x)|=2n'
export const EXPR_SIG_G = 'sigma_i(y)=x-c_i e_i'
export const EXPR_PARES = 'n pares'
export const EXPR_R4 = 'R_ij^4=id'
export const EXPR_R2 = 'R_ij^2=-id'
export const EXPR_PLANOS = 'binom(n,2)'
export const EXPR_COD = 'codificar'
export const EXPR_LIN = 'linearizar'
export const EXPR_BLOQ = 'n<=1'
export const EXPR_IXI = 'I x I'
export const EXPR_CONT = 'A:IxI->X'
export const EXPR_ESC = '1x1=escalar'
export const EXPR_PAL = 's0...sN-1'
export const EXPR_ALF = '|A|=|V(x)|'
export const EXPR_TLETRA = 'pi(t+1)=pi(t)+T(e_s)'
export const EXPR_REC = 'pi de pi(0) e palavra'
export const EXPR_G1 = 'G(x)>1'
export const EXPR_KER = 'fator em ker w'
export const EXPR_RHO = 'rho(q_t)=2t'
export const EXPR_BUR = 'impares por usar'
export const EXPR_FOL = '|I|-|X|'
export const EXPR_G1SUM = 'sum(G-1)'
export const EXPR_S = 'serpentina s(t)'
export const EXPR_GUM = 'G=1'
export const EXPR_VARPHI = 'varphi(t) base M'
export const EXPR_POSL = 'leitura posicional da tupla'
export const EXPR_DX = 'd_x=d(sigma(x),tau(x))'
export const EXPR_DMAX = 'D=sup d_x'
export const EXPR_TRAV = 'T=S o R^{-1}'
export const EXPR_PIP = 'pi=s o r^{-1}'
export const EXPR_UNI = 'mapa unico'
export const EXPR_DOP = 'D(A)=A^vee'
export const EXPR_FACES = 'observador != observado'
export const EXPR_IF = 'interface operacional'
export const EXPR_M = 'M = O sqcup T'
export const EXPR_TFIO = 'T transporte'
export const EXPR_AA = 'iso o duo = dobra'
export const EXPR_GMEDE = 'inj <=> G<=1'
export const EXPR_D2 = 'D^2=id'
export const EXPR_EQA = 'D^2(A)=A'
export const EXPR_TAB = 'tabua iso/duo'
export const EXPR_SOMA = 'compor = somar a'
export const EXPR_QDT = 'D^a dagger^b'
export const EXPR_QUAD = 'quatro quadrantes'
export const EXPR_CTR = 'centro = leitura'
export const EXPR_L0 = 'D^0=id'
export const EXPR_L1 = 'D^1=D'
export const EXPR_F0 = 'face 0 = a=0'
export const EXPR_F1 = 'face 1 = a=1'
export const EXPR_EQF = 'uma equacao'
export const EXPR_ST = 'faces iguais'
export const EXPR_ENC = 'encaixe em inteiros'
export const EXPR_REV = 'passagem reversivel'
/** Lê X; não constrói X. Fora da cadeia geradora. */
export const LEITURA_COORD = Object.freeze({
  id: 'coord-byte',
  papel: 'leitura',
  objeto: EXPR_LEITURA,
  origem: 'X',
  observado: 'byte',
  lei_gk: 'N/A',
  ciclo: CICLO,
  cadeia: false,
})
export const LEITURAS = Object.freeze([LEITURA_COORD])
export const RELS = Object.freeze(['gera', 'realiza', 'fixa'])
export const RELS_LEITURA = Object.freeze(['le'])
/** gerador → espaço → realização. Relações, não leis. Leitura não entra. */
export const CADEIA = Object.freeze(['vizdobra', 'X', 'base', 'afim', 'vinco'])
/** Objectos já alojados. A busca do fecho só pode apontar para aqui. */
export const HOSPEDES = Object.freeze(CADEIA.concat(['coord-byte']))
export const ARESTAS = Object.freeze([
  Object.freeze({ de: 'vizdobra', rel: 'gera', para: 'X' }),
  Object.freeze({ de: 'X', rel: 'realiza', para: 'afim' }),
  Object.freeze({ de: 'afim', rel: 'fixa', para: 'vinco' }),
])
export const ARESTAS_LEITURA = Object.freeze([
  Object.freeze({ de: 'X', rel: 'le', para: 'coord-byte' }),
])
export const INSTRUMENTO_AURA = 'aura'
export const ID_FASE_AURA = 'fase-aura'
export const ID_FP_1 = 'fp-1'
export const ID_REVELA = 'revela'
export const ID_TRIADE_FECHA = 'triade-fecha'
export const ID_DUAL_SMIN = 'dual-smin'
export const EXPR_TRIADE = '⊕ Clifford ⋈ ⊗ La Hire ⋈ ∏ Pontryagin'
export const EXPR_PHI2 = 'φ² = φ + 1'
export const EXPR_SIGMA = 'σ·σ′ = −1'
export const EXPR_JULIA = 'z ↦ z² + c'
/** Parâmetros da lei, como o original os escreve. θ é coordenada do domínio. */
export const PARAMS_FASE_AURA = Object.freeze(['f', 'r', 'a', 'χ', 'n'])
export const DOMINIO_FASE_AURA = Object.freeze(['r', 'θ'])

function refsDe (fisica, catalogo, cruz) {
  return Object.freeze({
    fisica: Object.freeze(fisica || []),
    catalogo: Object.freeze(catalogo || []),
    cruz: cruz || 'relacionado',
  })
}

/**
 * Terceiro eixo da tríade geradora: realização afim da 3ª dobra.
 * Não substitui Pontryagin. Não é associador. Não é 6ª lei do Reino.
 */
export const EIXO_AFIM = Object.freeze({
  id: 'afim',
  objeto: 'afim',
  origem: 'dobra-3',
  endereco: '3ª dobra',
  expressao: EXPR_AFIM,
  dobra: 3,
  formula: EXPR_AFIM,
  papel: 'realizacao',
  cruz: 'relacionado',
  lei_gk: 'N/A',
  ciclo: CICLO,
  entra: 'terceira dobra geradora de 8=2^3',
  nao: Object.freeze(['associador', 'Pontryagin', 'trindade-linguas', 'coord2-terceira', 'lei_gk']),
  refs: refsDe(
    ['fis:thm:dobra', 'fis:obs:quantica-associador-eixo', 'univ:def:base-tres'],
    [],
    'relacionado',
  ),
})

/** Base ortonormal: e_k = 2^k. Sem Lei 8. */
export function ek (k) {
  if (k < 0 || k > 7) return null
  return 1 << k
}

/** Coordenada: ⟨b, e_k⟩ = (b ≫ k) ∧ 1. */
export function coord (b, k) {
  if (k < 0 || k > 7) return null
  return (b >>> k) & 1
}

export function coords (b) {
  return LEIS_BASE.map((k) => coord(b, k))
}

/** Peso de Hamming: soma das coordenadas em N. Leitura de X; não é 2_V. */
export function pesoHamming (b) {
  let n = 0
  for (const k of LEIS_BASE) n += coord(b, k)
  return n
}

/** Primeira divergência a partir do MSB. q=0 se o bit p-1 difere. */
export function prof (a, b, p) {
  const n = p == null ? DIM_X : Number(p)
  if (a === b) return n
  for (let q = 0; q < n; q++) {
    if (coord(a, n - 1 - q) !== coord(b, n - 1 - q)) return q
  }
  return n
}

/** d=2^{-prof}. O valor 2^{-k} não vive em B. */
export function dUltra (a, b, p) {
  if (a === b) return 0
  return 2 ** (-prof(a, b, p))
}

/** Prefixo de q bits a partir do MSB. q=0 é o espaço inteiro. */
export function prefixoIgual (a, x, q, p) {
  const n = p == null ? DIM_X : Number(p)
  if (q <= 0) return true
  const k = q > n ? n : q
  for (let i = 0; i < k; i++) {
    if (coord(a, n - 1 - i) !== coord(x, n - 1 - i)) return false
  }
  return true
}

/** Irmão na árvore: troca o q-ésimo bit MSB. 1≤q≤w; raiz/folha → null. */
export function irmaoArvore (x, q, p) {
  const n = p == null ? DIM_X : Number(p)
  if (q < 1 || q > n) return null
  return xorX(x, 1 << (n - q))
}

/** χ=v-a. Quantidade lida; não é o carácter χ_k. */
export function chiEuler (v, a) {
  return v - a
}

/** Contagem local: 2G menos as pontas. O 2 é passagem, não 2_B. */
export function handshakeLocal (G, inicio, fim) {
  return 2 * G - (inicio ? 1 : 0) - (fim ? 1 : 0)
}

/** Em B, +e_i e -e_i são o mesmo XOR. Em Zc seriam dois. */
export function vizB (x, i) {
  const e = ek(i)
  if (e == null) return null
  return xorX(x, e)
}

/** Grau do hipercubo: m, não 2m. */
export function grauM (n) {
  return n
}

/** Grau da grade: 2n. Pede 2e_i ≠ 0. */
export function grau2n (n) {
  return 2 * n
}

/** Número de planos coordenados. Dimensão conta rotores, não só vizinhos. */
export function nPlanos (n) {
  return n * (n - 1) / 2
}

/** 2n vizinhos não cabem em grauAlvo. Bloqueio, não objecto. */
export function viznisoBloqueia (n, grauAlvo) {
  return grau2n(n) > grauAlvo
}

/** Soma XOR das letras. Em B a reconstrução cancela. */
export function xorPalavra (letras) {
  let y = 0
  for (const e of letras) y = xorX(y, e)
  return y
}

/** Dois prefixos com a mesma imagem. Equivale a G>1 quando XOR cancela. */
export function prefixosColidem (letras) {
  const visto = new Set([0])
  let y = 0
  for (const e of letras) {
    y = xorX(y, e)
    if (visto.has(y)) return true
    visto.add(y)
  }
  return false
}

/** ρ(q_t)=2t. O 2 é índice par, não 2_B. */
export function rho2t (t) {
  return 2 * t
}

/** Ímpares por usar em 0..2n-1 quando a imagem são n pares. */
export function rhoBuracos (n) {
  return n
}

/** Folga: |I|-|X|. A mesma conta dos dois lados. */
export function folga (nI, nX) {
  return nI - nX
}

/** Serpentina no quadrado M×M. Codifica; não preserva vizinhança. */
export function serpentina (t, M) {
  const y = Math.floor(t / M)
  const r = t % M
  const x = (y % 2 === 0) ? r : (M - 1 - r)
  return Object.freeze([x, y])
}

/** Inversa: a mesma regra lida ao contrário. */
export function serpentinaInv (xy, M) {
  const x = xy[0]
  const y = xy[1]
  const r = (y % 2 === 0) ? x : (M - 1 - x)
  return y * M + r
}

/** Enumeração posicional: dígitos de t na base M, posto n. Codifica; não guarda vizinhança. */
export function phiEnumera (t, M, n) {
  const xs = []
  let q = t
  for (let k = 0; k < n; k++) {
    xs.push(q % M)
    q = Math.floor(q / M)
  }
  return Object.freeze(xs)
}

/** Inversa posicional: t = sum x_k M^k. */
export function phiInv (xs, M) {
  let t = 0
  let p = 1
  for (let k = 0; k < xs.length; k++) {
    t += xs[k] * p
    p *= M
  }
  return t
}

/** D=sup_x d(σ(x),τ(x)). A mesma ultramétrica de T, no sup. Não é métrica nova. */
export function supDx (addrA, addrB, p) {
  let D = 0
  const n = Math.min(addrA.length, addrB.length)
  for (let i = 0; i < n; i++) {
    const dx = dUltra(addrA[i], addrB[i], p)
    if (dx > D) D = dx
  }
  return D
}

/** Leitura R_r(x)=sum x_k M^{r(k)}. Permuta posições; não é dinâmica de X. */
export function leituraR (xs, r, M) {
  let t = 0
  for (let k = 0; k < xs.length; k++) t += xs[k] * (M ** r[k])
  return t
}

/** π=s∘r^{-1} nas posições. Não é o percurso π nem π_r da bola. */
export function permPi (s, r) {
  const n = r.length
  const rInv = []
  for (let k = 0; k < n; k++) rInv[r[k]] = k
  const pi = []
  for (let i = 0; i < n; i++) pi.push(s[rInv[i]])
  return Object.freeze(pi)
}

/** Primeiro i com π(i)≠i; -1 se id. */
export function qPi (pi) {
  for (let i = 0; i < pi.length; i++) {
    if (pi[i] !== i) return i
  }
  return -1
}

/** D lê-se de π: 0 se id, senão 2^{-q}. Hipótese M=2 (dígito=bit). */
export function precoPi (pi) {
  const q = qPi(pi)
  return q < 0 ? 0 : 2 ** (-q)
}

/** T(a): os dígitos ficam; mudam-lhes os destinos. */
export function aplicaTrav (t, pi, M, n) {
  return leituraR(phiEnumera(t, M, n), pi, M)
}

/**
 * Três eixos independentes. bij = o que f não perde.
 * a=0 Iso (preserva); a=1 Duo (troca). Sem a, nem Iso nem Duo.
 */
export function eixosF (bij, a) {
  const b = !!bij
  const temA = a === 0 || a === 1
  return Object.freeze({
    bij: b,
    iso: b && temA && a === 0,
    duo: b && temA && a === 1,
  })
}

/** Composição do bit a: soma módulo 2. Iso e Duo são as duas espécies. */
export function compoeA (a, ap) {
  return (Number(a) ^ Number(ap)) & 1
}

/** Troca os papéis das duas operações. Não toca em X nem em G. */
export function dualOps (ops) {
  return Object.freeze([ops[1], ops[0]])
}

/** Paridade de D^n: 0 iso, 1 duo. Expoente lido em B. */
export function paridadeD (n) {
  return Number(n) & 1
}

/**
 * x^2=x+1 em B: char 2 dá x+x=0, AND é idempotente, donde 0=1.
 * Incompatível com o contrato declarado. Não é N/A; não promove.
 */
export function estrelaEmB () {
  return false
}

/** Recorrência u_0=0, u_1=1, u_{k+1}=u_k+u_{k-1}. Inteiros, sem raiz. */
export function fibU (n) {
  let a = 0
  let b = 1
  for (let i = 0; i < n; i++) {
    const t = a + b
    a = b
    b = t
  }
  return a
}

/** Cassini: u_{k+1}^2 - u_{k+2} u_k = (-1)^k. Tudo em inteiros. */
export function cassini (k) {
  return fibU(k + 1) * fibU(k + 1) - fibU(k + 2) * fibU(k)
}

/**
 * O encaixe constrói a condição em Zc. Não avalia raiz.
 * Não autoriza alojar a mesma condição em B.
 */
export function estrelaEmZc () {
  return cassini(0) === 1 && cassini(1) === -1 && cassini(2) === 1 &&
    fibU(5) === 5 && fibU(2) === 1
}

/** Par (a,b) em B². Produto de duas paridades, não quinta espécie. */
export function parAB (a, b) {
  return Object.freeze({ a: Number(a) & 1, b: Number(b) & 1 })
}

/** Compôr Q_ab ∘ Q_a'b' = Q_{a⊕a', b⊕b'}. Não age em X. */
export function compoePar (p, q) {
  return Object.freeze({
    a: (Number(p.a) ^ Number(q.a)) & 1,
    b: (Number(p.b) ^ Number(q.b)) & 1,
  })
}

export function rotuloPar (p) {
  return String(p.a) + String(p.b)
}

/** Em B, σ_i(x)=x XOR e_i. Na grade, σ_i(x)=x. */
export function sigmaBFixa (i, x) {
  return sigmaI(i, x) === x
}

export const EXPR_GRUPO = '⟨σ₁,σ₂,σ₃⟩'
export const EXPR_ROTOR = 'R_ij'
export const EXPR_QUARTETO = 'Q_ab^2=id'
export const EXPR_QAB = 'σ₁^a σ₂^b'
export const ROTULOS_QAB = Object.freeze(['00', '01', '10', '11'])
/** Elementos da construção; não são três geradores nem três leis. */
export const SIGMAS = Object.freeze([
  Object.freeze({ i: 1, objeto: 'σ₁', bit: 0 }),
  Object.freeze({ i: 2, objeto: 'σ₂', bit: 1 }),
  Object.freeze({ i: 3, objeto: 'σ₃', bit: 2 }),
])
/**
 * fis:thm:vizdobra: o objecto é o grupo ⟨σ₁,σ₂,σ₃⟩, 2³=8.
 * σᵢ ≠ σ·σ′=−1. Não são três lei_gk.
 */
export const GERADOR_VIZDOBRA = Object.freeze({
  id: 'vizdobra',
  objeto: EXPR_GRUPO,
  estados: 8,
  origem: 'dobra',
  papel: 'gerador',
  elementos: SIGMAS,
  n: 3,
  lei_gk: 'N/A',
  ciclo: CICLO,
  nao: Object.freeze(['sigma-dual', 'rotor', 'lei_gk']),
  refs: refsDe(['fis:thm:vizdobra', 'fis:thm:quarteto'], [], ''),
})
export const GERADORES = Object.freeze([GERADOR_VIZDOBRA])

/**
 * Candidatos conhecidos sem casa. Não mintam kind.
 * rotor = fis:thm:fecho (quarto de volta no plano). ≠ DTC ≠ rotor do motor.
 * dtc = espiral do relógio no inversor (catálogo); não é entrada EM; forma ≠ corpo.
 */
export const CANDIDATO_ROTOR = Object.freeze({
  id: 'rotor',
  expressao: EXPR_ROTOR,
  origem: 'fis:thm:fecho',
  endereco: '(e_i,e_j)',
  hospede: 'X',
  papel: '',
  relacao: '',
  estatuto: 'desconhecido',
  tipoNovo: false,
  lei_gk: 'N/A',
  gerador: 'N/A',
  ciclo: CICLO,
  nao: Object.freeze(['dtc', 'inversor', 'motor', 'fecho-convexo', 'kind', 'lei_gk', 'gerador']),
})
export const CANDIDATO_DTC = Object.freeze({
  id: 'dtc',
  origem: 'cat:bloco4',
  observa: 'relogio',
  estatuto: 'desconhecido',
  tipoNovo: false,
  lei_gk: 'N/A',
  ciclo: CICLO,
  em: 'N/A',
  nao: Object.freeze([
    'entrada-catalogo', 'corpo-em', 'fis:thm:fecho', 'rotor', 'lei_gk', 'X', 'e_k', 'F8',
  ]),
  refs: Object.freeze(['cat:bloco4', 'fis:def:em-forma', 'fis:thm:fecho']),
})
export const CANDIDATOS = Object.freeze([CANDIDATO_ROTOR, CANDIDATO_DTC])

/**
 * Vinco na realização afim: Fix(∂) ⇔ 2m=a+b.
 * Não é unidade 1=vinco(L0,L7). Não é σ_m=m+1/σ_m. Não é 6ª lei.
 */
export const VINCO_AFIM = Object.freeze({
  id: 'vinco',
  objeto: 'vinco',
  origem: 'dobra-3',
  endereco: 'Fix(∂)',
  expressao: EXPR_VINCO,
  formula: EXPR_VINCO,
  papel: 'realizacao',
  cruz: 'relacionado',
  lei_gk: 'N/A',
  ciclo: CICLO,
  depende: 'eixo-afim',
  entra: 'Fix(∂) na realização afim da 3ª dobra',
  nao: Object.freeze(['unidade', 'estrela-sigma', 'combinador-Y', 'lei_gk', 'sigma-dual']),
  refs: refsDe(
    ['fis:thm:vinco', 'fis:thm:dobra', 'fis:thm:vizdobra', 'univ:def:unidade'],
    [],
    'relacionado',
  ),
})
export const VINCO = VINCO_AFIM

export const REALIZACOES = Object.freeze([EIXO_AFIM, VINCO_AFIM])

export function gramOrtonormal () {
  for (let i = 0; i < DIM_X; i++) {
    for (let j = 0; j < DIM_X; j++) {
      if (coord(ek(i), j) !== (i === j ? 1 : 0)) return false
    }
  }
  return true
}

/** σ_i reverte a coordenada i−1. i ∈ {1,2,3}. Não é σ·σ′=−1. */
export function sigmaI (i, b) {
  const s = SIGMAS.find((x) => x.i === i)
  if (!s) return null
  return b ^ ek(s.bit)
}

/** ⟨σ₁,σ₂,σ₃⟩ tem 8 elementos. fis:thm:vizdobra(4) com n=3. */
export function grupoGeradores8 () {
  const els = new Set()
  for (let mask = 0; mask < DIM_X; mask++) {
    let y = 0
    for (const s of SIGMAS) {
      if ((mask >>> s.bit) & 1) y = sigmaI(s.i, y)
    }
    els.add(y)
  }
  return els.size === DIM_X && [...els].every((y) => y >= 0 && y < DIM_X)
}

/**
 * Q_ab := σ₁^a σ₂^b. Quatro involuções, ordem 2: corte n=2 de vizdobra.
 * Subestrutura reconhecida ≠ novo objecto. ≠ rotor (ordem 4). ≠ Duo.
 */
export function qAb (a, b, x) {
  let y = x
  if (a) y = sigmaI(1, y)
  if (b) y = sigmaI(2, y)
  return y
}

export function quartetoN2 () {
  const visto = new Set()
  for (let a = 0; a < 2; a++) {
    for (let b = 0; b < 2; b++) visto.add(qAb(a, b, 0))
  }
  const distintos = visto.size === 4
  let involutivos = true
  let composicao = true
  let comutam = true
  for (let x = 0; x < 256; x++) {
    if (sigmaI(1, sigmaI(2, x)) !== sigmaI(2, sigmaI(1, x))) comutam = false
    for (let a = 0; a < 2; a++) {
      for (let b = 0; b < 2; b++) {
        if (qAb(a, b, qAb(a, b, x)) !== x) involutivos = false
        for (let ap = 0; ap < 2; ap++) {
          for (let bp = 0; bp < 2; bp++) {
            if (qAb(a, b, qAb(ap, bp, x)) !== qAb(a ^ ap, b ^ bp, x)) composicao = false
          }
        }
      }
    }
  }
  const identico = distintos && involutivos && composicao && comutam
  const cruz = cruzarIdentidade({
    hospede: 'X', relacao: 'gera', ordemPaper: 2, ordemEmB: 2,
    realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  return Object.freeze({
    origem: 'fis:thm:quarteto',
    objecto: 'Q_ab',
    expressao: EXPR_QUARTETO,
    construcao: EXPR_QAB,
    n: 4,
    corte: 2,
    estados: 4,
    rotulos: ROTULOS_QAB,
    ordem: 2,
    suporte: 'X',
    realizacao: 'B',
    relacao: identico ? 'gera' : '',
    papel: identico ? 'gerador' : '',
    casa: GERADOR_VIZDOBRA.id,
    cria: false,
    identico,
    distintos,
    involutivos,
    composicao,
    comutam,
    cruz,
    estatuto: identico && cruz.compativel ? 'alojado' : 'desconhecido',
    tipoNovo: false,
    lei_gk: 'N/A',
    nao: Object.freeze(['rotor', 'fecho', 'Duo', 'lei_gk']),
    f8: false,
  })
}

/**
 * Grau dentro da construção vizdobra. Não mintam kind nem gerador.
 * σᵢ = elemento; Q_ab = subestrutura; vizdobra = construção. R_ij não entra.
 */
export function grauNaConstrucao (id) {
  const n = String(id || '')
  if (SIGMAS.some((s) => s.objeto === n)) {
    return Object.freeze({
      grau: 'elemento', casa: GERADOR_VIZDOBRA.id, tipoNovo: false, kind: '',
    })
  }
  if (n === 'Q_ab' || n === EXPR_QAB || n === EXPR_QUARTETO) {
    return Object.freeze({
      grau: 'subestrutura', casa: GERADOR_VIZDOBRA.id, corte: 2, tipoNovo: false, kind: '',
    })
  }
  if (n === GERADOR_VIZDOBRA.id || n === GERADOR_VIZDOBRA.objeto) {
    return Object.freeze({
      grau: 'construcao', casa: GERADOR_VIZDOBRA.id, tipoNovo: false, kind: '',
    })
  }
  return Object.freeze({ grau: '', casa: '', tipoNovo: false, kind: '' })
}

/**
 * Onde o identificador vive. Ontologia ≠ construção ≠ parâmetro.
 * Localizado em X não basta: R_ij não pertence a vizdobra.
 */
export function pertenceA (id) {
  const n = String(id || '')
  if (n === GERADOR_VIZDOBRA.id || n === GERADOR_VIZDOBRA.objeto) {
    return Object.freeze({
      onde: 'ontologia', papel: 'gerador', casa: GERADOR_VIZDOBRA.id, tipoNovo: false,
    })
  }
  if (SIGMAS.some((s) => s.objeto === n) || n === 'Q_ab' || n === EXPR_QAB || n === EXPR_QUARTETO) {
    return Object.freeze({
      onde: 'construcao', papel: '', casa: GERADOR_VIZDOBRA.id, tipoNovo: false,
    })
  }
  if (n === 'n=2' || n === 'corte') {
    return Object.freeze({
      onde: 'parametro', papel: '', casa: GERADOR_VIZDOBRA.id, tipoNovo: false,
    })
  }
  return Object.freeze({ onde: '', papel: '', casa: '', tipoNovo: false })
}

/** σᵢ e Q_ab geram/cortam X. R_ij e χ vivem em X sem serem gerados. */
export function geradoPorVizdobra (id) {
  const n = String(id || '')
  if (n === ESPACO.id) {
    return ARESTAS.some((a) => a.de === 'vizdobra' && a.rel === 'gera' && a.para === 'X')
  }
  if (SIGMAS.some((s) => s.objeto === n)) return true
  if (n === 'Q_ab' || n === EXPR_QAB || n === EXPR_QUARTETO) return true
  return false
}

export function viveEmX (id) {
  const n = String(id || '')
  if (n === ESPACO.id || n === 'base' || n === 'afim' || n === 'vinco' || n === 'coord-byte') return true
  if (n === EXPR_ROTOR || n === 'R_ij' || n === 'chi' || n === EXPR_CHI) return true
  if (SIGMAS.some((s) => s.objeto === n)) return true
  if (n === 'Q_ab' || n === EXPR_QAB || n === EXPR_QUARTETO) return true
  return false
}

/** Afim entra na 3ª dobra. Não promove lei. Não abre F8. */
export function ondeEntraAfim () {
  return Object.freeze({
    espaco: ESPACO,
    eixo: EIXO_AFIM,
    conhecida: false,
    realiza: false,
    cena: false,
    f8: false,
  })
}

/** Vinco entra na realização afim. Não é unidade. Não promove lei. */
export function ondeEntraVinco () {
  return Object.freeze({
    espaco: ESPACO,
    vinco: VINCO_AFIM,
    conhecida: false,
    realiza: false,
    cena: false,
    f8: false,
  })
}

/**
 * Aloja um teorema do paper na ontologia congelada.
 * desconhecido ≠ ausente ≠ novo tipo. tipoNovo é sempre false.
 */
export function alojarTeorema (cand) {
  const papel = String((cand && cand.papel) || '')
  const rel = String((cand && (cand.rel || cand.relacao)) || '')
  const noPapel = ONTOLOGIA.includes(papel)
  const noRel = RELS.includes(rel) || RELS_LEITURA.includes(rel)
  const forcaAusente = !!(cand && (cand.estatuto === 'ausente' || cand.ausente === true))
  const estatuto = forcaAusente
    ? 'ausente'
    : ((noPapel || noRel) ? 'alojado' : 'desconhecido')
  return Object.freeze({
    funil: FUNIL_TEOREMA,
    papel: estatuto === 'alojado' && noPapel ? papel : '',
    relacao: estatuto === 'alojado' && !noPapel && noRel ? rel : '',
    alojado: estatuto === 'alojado',
    estatuto,
    tipoNovo: false,
    promove: false,
    realiza: false,
    f8: false,
  })
}

/** Candidato conhecido sem casa. Não inventa kind. */
export function registroCandidato (id) {
  const c = CANDIDATOS.find((x) => x.id === id)
  if (!c) {
    return Object.freeze({
      id: String(id || ''),
      estatuto: 'desconhecido',
      tipoNovo: false,
      lei_gk: 'N/A',
      f8: false,
    })
  }
  return Object.freeze({
    id: c.id,
    origem: c.origem,
    expressao: c.expressao || '',
    endereco: c.endereco || '',
    hospede: c.hospede || '',
    observa: c.observa || '',
    estatuto: c.estatuto,
    tipoNovo: false,
    lei_gk: 'N/A',
    em: c.em || '',
    f8: false,
  })
}

/**
 * fis:thm:fecho: R_ij vive no plano (e_i,e_j) de X.
 * Nenhuma de gera/realiza/fixa/le aloja. Ordem 4 ≠ ordem 2 em B.
 * Endereço encontrado ≠ casa. Permanece desconhecido.
 */
export function buscaFecho () {
  const rels = RELS.concat(RELS_LEITURA)
  const tentativas = []
  for (const rel of rels) {
    for (const para of HOSPEDES) {
      tentativas.push(Object.freeze({ de: 'rotor-fecho', rel, para, ok: false }))
    }
  }
  let p = [1, 0]
  const visto = new Set(['1,0'])
  let ordemEmB = 0
  for (let n = 0; n < 8; n++) {
    p = [p[1] & 1, p[0] & 1]
    ordemEmB++
    const k = p[0] + ',' + p[1]
    if (visto.has(k)) break
    visto.add(k)
  }
  const nDobras = DOBRAS_GERADORAS.length
  return Object.freeze({
    funil: FUNIL_BUSCA,
    id: 'rotor-fecho',
    expressao: EXPR_ROTOR,
    origem: 'fis:thm:fecho',
    papel: '',
    relacao: '',
    hospede: ESPACO.id,
    endereco: '(e_i,e_j)',
    nPlanos: nPlanos(nDobras),
    ordemPaper: 4,
    ordemEmB,
    rels: Object.freeze(tentativas),
    alojado: false,
    estatuto: 'desconhecido',
    tipoNovo: false,
    lei_gk: 'N/A',
    gerador: 'N/A',
    kind: '',
    f8: false,
  })
}

/**
 * Cruzar identidade: expressão + suporte + realização.
 * localizado ∧ incompatível ⇒ desconhecido. Nunca tipoNovo. Não é ausente.
 */
export function cruzarIdentidade (cand) {
  const hospede = String((cand && (cand.hospede || cand.suporte)) || '')
  const rel = String((cand && (cand.relacao || cand.rel)) || '')
  const localizado = HOSPEDES.includes(hospede)
  const temOrdem = cand && cand.ordemPaper != null && cand.ordemEmB != null
  const incompOrdem = !!(temOrdem && Number(cand.ordemPaper) !== Number(cand.ordemEmB))
  const realP = String((cand && cand.realizacaoPaper) || '')
  const realB = String((cand && cand.realizacaoBanco) || '')
  const incompReal = !!(realP && realB && realP !== realB)
  const incompativel = incompOrdem || incompReal
  const relOk = RELS.includes(rel) || RELS_LEITURA.includes(rel)
  const compativel = localizado && relOk && !incompativel
  const motivo = incompOrdem ? 'estrutural' : (incompReal ? 'algebrica' : '')
  return Object.freeze({
    campos: CAMPOS_IDENTIDADE,
    localizado,
    incompativel,
    compativel,
    motivo,
    estatuto: compativel ? 'alojado' : 'desconhecido',
    tipoNovo: false,
    f8: false,
  })
}

/** fis:thm:base(2): medir o bit é ler a coordenada. Mesma leitura; não é tipo novo. */
export function medirELer () {
  return Object.freeze({
    papel: 'leitura',
    objeto: EXPR_LEITURA,
    identico: ESPACO.coord === EXPR_LEITURA && LEITURA_COORD.objeto === EXPR_LEITURA,
    tipoNovo: false,
    lei_gk: 'N/A',
    f8: false,
  })
}

/** g^{(k)}(b)=b∧2^k. Peso por direcção; não é a régua do card. */
export function pesoK (b, k) {
  const e = ek(k)
  if (e == null) return null
  return b & e
}

/**
 * fis:thm:base(3): as réguas somam-se na identidade e não se compõem.
 * Mesma realização que a leitura, sobre B. Não é papel novo.
 */
export function baseClausula3 () {
  let soma = true
  let le = true
  let cega = true
  let naoCompoe = true
  for (let b = 0; b < 256; b++) {
    let s = 0
    for (const k of LEIS_BASE) {
      const g = pesoK(b, k)
      s ^= g
      if (g !== coord(b, k) * ek(k)) le = false
    }
    if (s !== b) soma = false
  }
  for (let i = 0; i < DIM_X; i++) {
    for (let j = 0; j < DIM_X; j++) {
      if (pesoK(ek(i), j) !== (i === j ? ek(i) : 0)) cega = false
      if (i !== j && pesoK(pesoK(255, i), j) !== 0) naoCompoe = false
    }
  }
  const identico = soma && le && cega && naoCompoe
  const cruz = cruzarIdentidade({ hospede: 'X', relacao: 'le' })
  return Object.freeze({
    origem: 'fis:thm:base',
    clausula: 3,
    expressao: EXPR_PESO,
    suporte: 'X',
    realizacao: 'B',
    papel: identico ? 'leitura' : '',
    relacao: identico ? 'le' : '',
    somaId: soma,
    leitura: le,
    cega,
    naoCompoe,
    identico,
    cruz,
    estatuto: identico && cruz.compativel ? 'alojado' : 'desconhecido',
    tipoNovo: false,
    lei_gk: 'N/A',
    f8: false,
  })
}

/** ⟨k,j⟩=paridade(k∧j). A forma de Gram; não é χ_k∈{±1}. */
export function paridadeKJ (k, j) {
  let p = 0
  for (const t of LEIS_BASE) p ^= coord(k, t) & coord(j, t)
  return p
}

/**
 * fis:def:dual: a paridade aterra em leitura sobre B.
 * χ_k:X→{±1} pede Zc; não aloja. ≠ dual-smin ≠ Duo ≠ V.
 */
export function dualParidade () {
  let gram = true
  for (let i = 0; i < DIM_X; i++) {
    for (let j = 0; j < DIM_X; j++) {
      if (paridadeKJ(ek(i), ek(j)) !== (i === j ? 1 : 0)) gram = false
    }
  }
  let linear = true
  for (const t of LEIS_BASE) {
    const kk = ek(t)
    for (let j = 0; j < 256; j++) {
      for (let jp = 0; jp < 256; jp++) {
        if (paridadeKJ(kk, j ^ jp) !== (paridadeKJ(kk, j) ^ paridadeKJ(kk, jp))) linear = false
      }
    }
  }
  const cruzPar = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const cruzChi = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'Zc', realizacaoBanco: 'B',
  })
  const identico = gram && linear
  return Object.freeze({
    origem: 'fis:def:dual',
    par: Object.freeze({
      expressao: EXPR_PAR,
      suporte: 'X',
      realizacao: 'B',
      papel: identico ? 'leitura' : '',
      relacao: identico ? 'le' : '',
      gram,
      linear,
      identico,
      estatuto: identico && cruzPar.compativel ? 'alojado' : 'desconhecido',
    }),
    chi: Object.freeze({
      expressao: EXPR_CHI,
      suporte: 'X',
      realizacao: 'Zc',
      papel: '',
      relacao: '',
      estatuto: cruzChi.estatuto,
      cruz: cruzChi,
    }),
    tipoNovo: false,
    lei_gk: 'N/A',
    nao: Object.freeze(['dual-smin', 'Duo', 'V', 'X*']),
    f8: false,
  })
}

/**
 * fis:def:doisandares: andar de baixo = X (já espaço). Andar de cima = V=Zc^X.
 * Papel espaço existe e está ocupado por X. V não cabe. Desconhecido ≠ ausente.
 */
export function doisAndares () {
  const cruzV = cruzarIdentidade({
    hospede: 'V', relacao: '', realizacaoPaper: 'Zc', realizacaoBanco: 'B',
  })
  return Object.freeze({
    origem: 'fis:def:doisandares',
    funil: FUNIL_CASA,
    baixo: Object.freeze({
      andar: 'baixo',
      objecto: ESPACO.id,
      expressao: ESPACO.base,
      suporte: 'X',
      realizacao: 'B',
      relacao: '',
      papel: 'espaco',
      papelExiste: ONTOLOGIA.includes('espaco'),
      casa: ESPACO.id,
      estatuto: 'alojado',
      tipoNovo: false,
    }),
    cima: Object.freeze({
      andar: 'cima',
      objecto: 'V',
      expressao: EXPR_V,
      suporte: 'V',
      realizacao: 'Zc',
      relacao: '',
      papel: '',
      papelExiste: false,
      casa: '',
      estatuto: cruzV.estatuto,
      motivo: cruzV.motivo,
      cruz: cruzV,
      tipoNovo: false,
      nao: Object.freeze(['X', 'espaco-novo', 'lei_gk']),
    }),
    tipoNovo: false,
    f8: false,
  })
}

/** Soma de X sobre B. É a acção de σᵢ. Não é a convolução em V. */
export function xorX (a, b) {
  return a ^ b
}

/**
 * fis:def:conv: dois ramos. ⊕ vive em X e aterra. * vive em V e pede casa de V.
 * C_f, hat, H: cada um o seu suporte. Não herdam o objecto *. Não cria espaço.
 */
export function convRamos () {
  let grupo = true
  for (let a = 0; a < 256; a++) {
    if (xorX(a, 0) !== a || xorX(a, a) !== 0) grupo = false
  }
  const bits = SIGMAS.every((s) => sigmaI(s.i, 0) === xorX(0, ek(s.bit)))
  const cruzXor = cruzarIdentidade({
    hospede: 'X', relacao: 'gera', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const vCasa = doisAndares().cima.estatuto === 'alojado'
  const cruzConv = cruzarIdentidade({
    hospede: 'V', relacao: '', realizacaoPaper: 'Zc', realizacaoBanco: 'B',
  })
  return Object.freeze({
    origem: 'fis:def:conv',
    funil: FUNIL_CASA,
    xor: Object.freeze({
      objecto: 'xor',
      expressao: EXPR_XOR,
      suporte: 'X',
      realizacao: 'B',
      relacao: bits ? 'gera' : '',
      papel: '',
      casa: ESPACO.id,
      grupo,
      bits,
      estatuto: grupo && bits && cruzXor.compativel ? 'alojado' : 'desconhecido',
      tipoNovo: false,
      cria: false,
    }),
    conv: Object.freeze({
      objecto: '*',
      expressao: EXPR_CONV,
      suporte: 'V',
      realizacao: 'Zc',
      relacao: '',
      papel: '',
      casa: '',
      estatuto: vCasa ? 'alojado' : cruzConv.estatuto,
      motivo: vCasa ? '' : cruzConv.motivo,
      tipoNovo: false,
      cria: false,
      nao: Object.freeze(['espaco-novo', 'lei_gk', 'gerador']),
    }),
    pendentes: PENDENTES_V,
    tipoNovo: false,
    f8: false,
  })
}

function recusaAndarV (origem, objecto, expressao, extra) {
  const vCasa = doisAndares().cima.estatuto === 'alojado'
  const cruz = cruzarIdentidade({
    hospede: 'V', relacao: '', realizacaoPaper: 'Zc', realizacaoBanco: 'B',
  })
  return Object.freeze({
    origem,
    objecto,
    expressao,
    suporte: 'V',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: vCasa ? 'alojado' : cruz.estatuto,
    motivo: vCasa ? '' : cruz.motivo,
    tipoNovo: false,
    cria: false,
    f8: false,
    ...(extra || {}),
  })
}

/** C_f(g)=f*g. Operador em V. ≠ * ≠ xor. */
export function operadorCf () {
  return recusaAndarV('fis:lem:comuta', 'C_f', EXPR_CF, {
    nao: Object.freeze(['xor', '*', 'hat', 'H', 'lei_gk']),
  })
}

/** f̂(k)=∑ f(j)χ_k(j). Coordenada em V. Pede χ e Zc. ≠ C_f ≠ H. */
export function hatF () {
  return recusaAndarV('fis:thm:proprios', 'hat', EXPR_HAT, {
    nao: Object.freeze(['C_f', '*', 'xor', 'paridade', 'H', 'lei_gk']),
  })
}

/** H²=2^m Id. Matriz em V. Sobre B, 2^m=0. ≠ hat ≠ C_f. */
export function matrizH () {
  const fatorPaper = 2 ** DIM_X
  const fatorEmB = 0
  return recusaAndarV('fis:thm:H', 'H', EXPR_H, {
    fatorPaper,
    fatorEmB,
    incompFator: fatorPaper !== fatorEmB,
    nao: Object.freeze(['C_f', 'hat', '*', 'xor', 'lei_gk']),
  })
}

/** Três objectos, três identidades. Mesmo bloco ⇏ mesmo objecto. */
export function ramosDoAndarV () {
  const cf = operadorCf()
  const hat = hatF()
  const H = matrizH()
  return Object.freeze({
    funil: FUNIL_CASA,
    cf,
    hat,
    H,
    distintos: cf.objecto !== hat.objecto && hat.objecto !== H.objecto && cf.objecto !== H.objecto,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:dobranorma: três objectos. Norma em V ≠ paridade.
 * 2P: factor 2 não traduz para B. F(G) pede H. Nenhum cria espaço.
 */
export function dobraNorma () {
  const cruz = cruzarIdentidade({
    hospede: 'V', relacao: '', realizacaoPaper: 'Zc', realizacaoBanco: 'B',
  })
  const fatorPaper = 2
  const fatorEmB = 0
  const norma = recusaAndarV('fis:thm:dobranorma', 'norma-V', EXPR_NORMA_V, {
    nao: Object.freeze(['paridade', 'chi', 'Gram', 'lei_gk']),
  })
  const doisP = recusaAndarV('fis:thm:dobranorma', '2P', EXPR_2P, {
    fatorPaper,
    fatorEmB,
    incompFator: fatorPaper !== fatorEmB,
    nao: Object.freeze(['vinco', 'H', 'lei_gk']),
  })
  const FG = recusaAndarV('fis:thm:dobranorma', 'F(G)', EXPR_H, {
    depende: 'H',
    nao: Object.freeze(['hat', 'C_f', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:dobranorma',
    funil: FUNIL_CASA,
    norma,
    doisP,
    FG,
    cruz,
    distintos: norma.objecto !== doisP.objecto && doisP.objecto !== FG.objecto,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:cor:modos: |k| lê-se em X/B. α e A=C_α vivem em V.
 * m−2|k|: o 2 é 2_V, não 2_B. Não traduz. LEITURAS permanece 1.
 */
export function modosRamos () {
  let ham = true
  if (pesoHamming(0) !== 0 || pesoHamming(255) !== DIM_X) ham = false
  for (const k of LEIS_BASE) {
    if (pesoHamming(ek(k)) !== 1) ham = false
  }
  const cruzHam = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const alpha = recusaAndarV('fis:cor:modos', 'alpha', EXPR_ALPHA, {
    nao: Object.freeze(['xor', 'e_k', 'lei_gk']),
  })
  const A = recusaAndarV('fis:cor:modos', 'A', EXPR_A, {
    nao: Object.freeze(['C_f', '*', 'xor', 'lei_gk']),
  })
  const auto = recusaAndarV('fis:cor:modos', 'autovalor', EXPR_AUTO, {
    fatorPaper: 2,
    fatorEmB: 0,
    incompFator: true,
    nao: Object.freeze(['hamming', '2_B', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:cor:modos',
    funil: FUNIL_CASA,
    hamming: Object.freeze({
      objecto: 'hamming',
      expressao: EXPR_HAM,
      suporte: 'X',
      realizacao: 'B',
      relacao: ham ? 'le' : '',
      papel: ham ? 'leitura' : '',
      casa: LEITURA_COORD.id,
      estatuto: ham && cruzHam.compativel ? 'alojado' : 'desconhecido',
      tipoNovo: false,
      cria: false,
    }),
    alpha,
    A,
    auto,
    distintos: true,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:conv: tres identidades em V. Produto ponto-a-ponto ≠ xor ≠ *.
 * Injetivo ≠ invertivel (Zc tem opostos, nao inversos). C_{d0+d0} testemunha.
 * Fase = paridade: outra leitura de X/B, aterra na casa existente. Nao cria.
 */
export function convTeorema () {
  const prod = recusaAndarV('fis:thm:conv', 'produto', EXPR_FG, {
    nao: Object.freeze(['xor', '*', 'C_f', 'lei_gk']),
  })
  const inj = recusaAndarV('fis:thm:conv', 'C_f', EXPR_INJ, {
    pergunta: 'injetivo',
    nao: Object.freeze(['invertivel', 'corpo', 'lei_gk']),
  })
  const inv = recusaAndarV('fis:thm:conv', 'C_f', EXPR_INV, {
    pergunta: 'invertivel',
    testemunha: 'C_d0+d0',
    nao: Object.freeze(['injetivo', 'corpo', 'lei_gk']),
  })
  const cruzFase = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  return Object.freeze({
    origem: 'fis:thm:conv',
    funil: FUNIL_CASA,
    prod,
    inj,
    inv,
    fase: Object.freeze({
      objecto: 'paridade',
      expressao: EXPR_PAR,
      suporte: 'X',
      realizacao: 'B',
      relacao: 'le',
      papel: 'leitura',
      casa: LEITURA_COORD.id,
      estatuto: cruzFase.compativel ? 'alojado' : 'desconhecido',
      cria: false,
      tipoNovo: false,
    }),
    distintos: prod.objecto !== inj.objecto && inj.expressao !== inv.expressao,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:topo-tese + fis:def:viztopo: ofício endereçar, não 6ª categoria.
 * Estatuto = leitura. V_E = xor já alojado. Esquecer E não acrescenta objecto.
 */
export function topoTese () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const E = SIGMAS.map((s) => ek(s.bit))
  let ve = E.length === DOBRAS_GERADORAS.length && E.every((e) => e > 0)
  for (const e of E) {
    if (xorX(0, e) !== e || xorX(e, e) !== 0) ve = false
  }
  for (let x = 0; x < 8; x++) {
    for (const e of E) {
      const y = xorX(x, e)
      if (y === x || xorX(y, e) !== x) ve = false
    }
  }
  const oficio = Object.freeze({
    objecto: 'enderecar',
    expressao: EXPR_END,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    papelExiste: ONTOLOGIA.includes('leitura'),
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['categoria', 'realizacao', 'lei_gk', 'espaco-novo']),
  })
  const VE = Object.freeze({
    objecto: 'V_E',
    expressao: EXPR_VE,
    suporte: 'X',
    realizacao: 'B',
    relacao: ve ? 'gera' : '',
    papel: '',
    casa: GERADOR_VIZDOBRA.id,
    estatuto: ve ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['V_topo', 'leitura-nova', 'lei_gk']),
  })
  const Vtopo = Object.freeze({
    objecto: 'V_topo',
    expressao: EXPR_VIZ,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['V_E', 'vizdobra', 'grafo', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:topo-tese',
    funil: FUNIL_CASA,
    oficio,
    VE,
    Vtopo,
    cega: 'medida',
    distintos: oficio.objecto !== VE.objecto && VE.objecto !== Vtopo.objecto,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:def:arvore: um conjunto {0,1}^p, três geometrias, não três objectos.
 * Q_m = Hamming; T = prof; I = percurso. d=2^{-prof} não traduz para B.
 */
export function arvoreRamos () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const a = 240
  const b = 224
  const ham = pesoHamming(xorX(a, b))
  const p = prof(a, b)
  const d = dUltra(a, b)
  const distHamUltra = ham !== d
  const conjunto = Object.freeze({
    objecto: '{0,1}^p',
    expressao: ESPACO.id,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: 'espaco',
    casa: ESPACO.id,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['espaco-novo', 'Q_m', 'T', 'I', 'lei_gk']),
  })
  const Qm = Object.freeze({
    objecto: 'Qm',
    expressao: EXPR_QM,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['T', 'I', 'd_ultra', 'lei_gk']),
  })
  const Tgeo = Object.freeze({
    objecto: 'T',
    expressao: EXPR_PROF,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel && p === 3 ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Qm', 'I', 'Hamming', 'lei_gk']),
  })
  const dU = Object.freeze({
    objecto: 'd_ultra',
    expressao: EXPR_DULTRA,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    fatorPaper: 2,
    fatorEmB: 0,
    incompFator: true,
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Hamming', 'd_percurso', '2_B', 'lei_gk']),
  })
  const Igeo = Object.freeze({
    objecto: 'I',
    expressao: EXPR_PERC,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Qm', 'T', 'espaco-novo', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:def:arvore',
    funil: FUNIL_CASA,
    conjunto,
    Qm,
    T: Tgeo,
    dUltra: dU,
    I: Igeo,
    testemunha: Object.freeze({ a, b, ham, prof: p, d, distHamUltra }),
    distintos: Qm.objecto !== Tgeo.objecto && Tgeo.objecto !== Igeo.objecto,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:lem:ultra: simetria e desigualdade são propriedades da leitura T.
 * Não mintam métrica, ultramétrica nem grafo. d=2^{-k} continua desconhecido.
 */
export function ultraLema () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const pts = [0, 1, 2, 7, 16, 224, 240, 255]
  let sim = true
  let minIneq = true
  let maxIneq = true
  for (const a of pts) {
    for (const b of pts) {
      if (prof(a, b) !== prof(b, a)) sim = false
      if (dUltra(a, b) !== dUltra(b, a)) sim = false
      for (const c of pts) {
        if (prof(a, c) < Math.min(prof(a, b), prof(b, c))) minIneq = false
        if (dUltra(a, c) > Math.max(dUltra(a, b), dUltra(b, c))) maxIneq = false
      }
    }
  }
  const ok = sim && minIneq && maxIneq
  const prop = (expressao) => Object.freeze({
    objecto: 'T',
    expressao,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    propriedade: true,
    estatuto: cruzLe.compativel && ok ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['metrica', 'ultrametrica', 'grafo', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:lem:ultra',
    funil: FUNIL_CASA,
    simetria: prop(EXPR_SIMETRIA),
    desigualdade: prop(EXPR_ULTRA),
    ok,
    mesmoObjecto: true,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:bolas: família B_cal ≠ B; condição (prefixo vs d≤r); escala 2^{-k}.
 * π_r ≠ afim ≠ vinco. Partição/centro são propriedades, não casas.
 */
export function bolasRamos () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const a = 240
  const x = 224
  const q = 3
  const pref = prefixoIgual(a, x, q) && !prefixoIgual(a, x, q + 1)
  const familia = Object.freeze({
    objecto: 'B_cal',
    expressao: EXPR_BOLA,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel && pref ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['B', 'espaco-novo', 'metrica', 'lei_gk']),
  })
  const condPref = Object.freeze({
    objecto: 'condicao',
    expressao: EXPR_PREF,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel && pref ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['d<=r', 'escala', 'lei_gk']),
  })
  const condD = Object.freeze({
    objecto: 'condicao',
    expressao: EXPR_COND_D,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['prefixo', 'B', 'lei_gk']),
  })
  const escala = Object.freeze({
    objecto: 'escala',
    expressao: EXPR_DULTRA,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    fatorPaper: 2,
    fatorEmB: 0,
    incompFator: true,
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['B_cal', 'prefixo', '2_B', 'lei_gk']),
  })
  const pir = Object.freeze({
    objecto: 'pi_r',
    expressao: EXPR_PIR,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['afim', 'vinco', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:bolas',
    funil: FUNIL_CASA,
    familia,
    condPref,
    condD,
    escala,
    pir,
    testemunha: Object.freeze({ a, x, q, pref }),
    distintos: familia.objecto !== condPref.objecto && condPref.expressao !== condD.expressao,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:vizarvore: N_T lê o irmão MSB. |N_T|=1 é propriedade, ≠ 2n ≠ grau.
 * Não mintam grafo, vizinhança nem cardinalidade. ONTOLOGIA intacta.
 */
export function vizArvoreRamos () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const x = 240
  const q = 3
  const y = irmaoArvore(x, q)
  const volta = y != null && irmaoArvore(y, q) === x
  const um = y != null && irmaoArvore(x, 0) == null && irmaoArvore(x, DIM_X + 1) == null
  const doisN = 2 * DOBRAS_GERADORAS.length
  const card = 1
  const NT = Object.freeze({
    objecto: 'N_T',
    expressao: EXPR_NT,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel && volta && um ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['V_E', 'V_topo', 'grafo', '2n', 'lei_gk']),
  })
  const cardinalidade = Object.freeze({
    objecto: 'cardinalidade',
    expressao: EXPR_CARD_T,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    propriedade: true,
    estatuto: cruzLe.compativel && card === 1 && card !== doisN ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['grau', 'grafo', '2n', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:vizarvore',
    funil: FUNIL_CASA,
    NT,
    cardinalidade,
    contraste: Object.freeze({ um: card, doisN, distinto: card !== doisN, cria: false }),
    testemunha: Object.freeze({ x, q, y, volta, um }),
    distintos: NT.objecto !== cardinalidade.objecto,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:def:percurso: I já aterra; o percurso é a sequência induzida.
 * I ≠ T. Percurso ≠ grafo. Propriedades da ordem anexam-se a I.
 */
export function percursoRamos () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const a = 240
  const b = irmaoArvore(a, 3)
  const passo = b != null && irmaoArvore(b, 3) === a
  const Igeo = Object.freeze({
    objecto: 'I',
    expressao: EXPR_PERC,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['T', 'grafo', 'espaco-novo', 'lei_gk']),
  })
  const perc = Object.freeze({
    objecto: 'percurso',
    expressao: EXPR_CAMINHO,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    propriedade: true,
    estatuto: cruzLe.compativel && passo ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['grafo', 'T', 'afim', 'vinco', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:def:percurso',
    funil: FUNIL_CASA,
    I: Igeo,
    percurso: perc,
    contraste: Object.freeze({ I: 'I', T: 'T', distinto: Igeo.objecto !== 'T', cria: false }),
    testemunha: Object.freeze({ a, b, passo }),
    distintos: Igeo.objecto !== perc.objecto,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:euler: χ=v-a é leitura, ≠ χ_k. Ciclo da dobra é propriedade de vizdobra.
 * G_ar anexa-se ao percurso. Euler ≠ gerador. χ ≠ tipo.
 */
export function eulerRamos () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const v = 2
  const a = 1
  const chi = chiEuler(v, a)
  const arvore = chi === 1
  const chiE = Object.freeze({
    objecto: 'chi_e',
    expressao: EXPR_CHI_E,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel && arvore ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['chi', 'lei_gk', 'tipo', 'lei']),
  })
  const ciclo = Object.freeze({
    objecto: 'ciclo',
    expressao: EXPR_CICLO,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: GERADOR_VIZDOBRA.id,
    propriedade: true,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['gerador-novo', 'lei_gk', 'lei']),
  })
  const Gar = Object.freeze({
    objecto: 'G_ar',
    anexa: 'percurso',
    casa: '',
    papel: '',
    estatuto: '',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['G', 'lei_gk', 'leitura-nova']),
  })
  return Object.freeze({
    origem: 'fis:thm:euler',
    funil: FUNIL_CASA,
    chiE,
    ciclo,
    Gar,
    testemunha: Object.freeze({ v, a, chi, arvore }),
    distintos: chiE.objecto !== ciclo.objecto,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:handshake: conta local é leitura. Incidência é propriedade.
 * G_ar anexa-se ao percurso. handshake ≠ Euler. 2G é passagem, não 2_B.
 */
export function handshakeRamos () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const ini = handshakeLocal(1, true, false)
  const fim = handshakeLocal(1, false, true)
  const fecha = ini === 1 && fim === 1
  const conta = Object.freeze({
    objecto: 'conta',
    expressao: EXPR_HS,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel && fecha ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Euler', '2_B', 'gerador', 'lei_gk', 'chi']),
  })
  const incidencia = Object.freeze({
    objecto: 'incidencia',
    expressao: EXPR_INC,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['grafo', 'gerador', 'lei_gk']),
  })
  const Gar = Object.freeze({
    objecto: 'G_ar',
    anexa: 'percurso',
    casa: '',
    papel: '',
    estatuto: '',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['G', 'lei_gk', 'leitura-nova']),
  })
  return Object.freeze({
    origem: 'fis:thm:handshake',
    funil: FUNIL_CASA,
    conta,
    incidencia,
    Gar,
    testemunha: Object.freeze({ ini, fim, fecha }),
    distintos: conta.objecto !== incidencia.objecto && conta.expressao !== EXPR_CHI_E,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:def:grade + fis:thm:dim: a grade pede Zc (2e_i ≠ 0).
 * Sobre B os sentidos colapsam: hipercubo = Q_m já lido. 2n ≠ m ≠ |N_T|.
 * n entra só na cláusula (4). Sem casa nova.
 */
export function gradeRamos () {
  const n = DOBRAS_GERADORAS.length
  const m = grauM(n)
  const doisN = grau2n(n)
  const e0 = ek(0)
  const mais = vizB(0, 0)
  const menos = vizB(0, 0)
  const colapsa = mais === menos && xorX(e0, e0) === 0
  const anel = Object.freeze({
    objecto: 'grade',
    expressao: EXPR_GRADE,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    fatorPaper: 2,
    fatorEmB: 0,
    incompFator: true,
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['hipercubo', 'B', 'espaco-novo', 'lei_gk']),
  })
  const viz = Object.freeze({
    objecto: 'V_grade',
    expressao: EXPR_VIZ_G,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['V_E', 'N_T', 'grafo', 'lei_gk']),
  })
  const card = Object.freeze({
    objecto: 'card_grade',
    expressao: EXPR_DIM,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    fatorPaper: 2,
    fatorEmB: 0,
    incompFator: true,
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['N_T', '2_B', 'lei', 'lei_gk']),
  })
  const nSitio = Object.freeze({
    objecto: 'n',
    anexa: 'clausula-4',
    casa: '',
    papel: '',
    estatuto: '',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['lei', 'espaco-novo', 'gerador']),
  })
  return Object.freeze({
    origem: 'fis:def:grade',
    funil: FUNIL_CASA,
    grade: anel,
    viz,
    card,
    nSitio,
    testemunha: Object.freeze({ n, m, doisN, colapsa, distinto: m !== doisN }),
    distintos: anel.objecto !== viz.objecto && card.expressao !== EXPR_CARD_T,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:vizdobra (Topologia): o grupo já é o gerador. n pares é propriedade.
 * A fórmula que fixa o vértice e troca ± pede Zc. ≠ x XOR e_i. Sem segundo gerador.
 */
export function vizdobraTopo () {
  const n = GERADOR_VIZDOBRA.n
  const pares = n
  const doisN = grau2n(n)
  const grupo = grupoGeradores8()
  const naoFixa = SIGMAS.every((s) => sigmaBFixa(s.i, 0) === false)
  const gerador = Object.freeze({
    objecto: 'vizdobra',
    expressao: EXPR_GRUPO,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'gera',
    papel: 'gerador',
    casa: GERADOR_VIZDOBRA.id,
    estatuto: grupo && naoFixa ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['gerador-novo', 'lei_gk', 'rotor']),
  })
  const paresRamo = Object.freeze({
    objecto: 'pares',
    expressao: EXPR_PARES,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: GERADOR_VIZDOBRA.id,
    propriedade: true,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['2n', 'grafo', 'lei_gk']),
  })
  const sigmaG = Object.freeze({
    objecto: 'sigma_grade',
    expressao: EXPR_SIG_G,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    fatorPaper: 2,
    fatorEmB: 0,
    incompFator: true,
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['sigma_B', 'xor', 'vinco', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:vizdobra',
    funil: FUNIL_CASA,
    gerador,
    pares: paresRamo,
    sigmaG,
    testemunha: Object.freeze({
      n, pares, doisN, grupo, naoFixa, distinto: pares !== doisN,
    }),
    distintos: gerador.objecto !== sigmaG.objecto && paresRamo.expressao !== EXPR_DIM,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:fecho (Topologia): R_ij continua desconhecido. R^2=-id pede Zc.
 * binom(n,2) conta planos, ≠ 2n. Ordem 4 ≠ ordem 2. Sem gerador novo.
 */
export function fechoTopo () {
  const F = buscaFecho()
  const n = DOBRAS_GERADORAS.length
  const planosN = nPlanos(n)
  const doisN = grau2n(n)
  const Rij = Object.freeze({
    objecto: 'R_ij',
    expressao: EXPR_R4,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    ordemPaper: F.ordemPaper,
    ordemEmB: F.ordemEmB,
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['gerador', 'quarteto', 'dtc', 'lei_gk']),
  })
  const menos = Object.freeze({
    objecto: 'R2',
    expressao: EXPR_R2,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    fatorPaper: 2,
    fatorEmB: 0,
    incompFator: true,
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['2_B', 'dobra', 'lei_gk']),
  })
  const planos = Object.freeze({
    objecto: 'planos',
    expressao: EXPR_PLANOS,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['2n', 'gerador', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:fecho',
    funil: FUNIL_CASA,
    Rij,
    menos,
    planos,
    testemunha: Object.freeze({
      n,
      planos: planosN,
      doisN,
      ordemPaper: F.ordemPaper,
      ordemEmB: F.ordemEmB,
      distinto: planosN !== doisN && F.ordemPaper !== F.ordemEmB,
    }),
    distintos: Rij.objecto !== menos.objecto && planos.expressao !== EXPR_DIM,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:vizniso: bloqueio. Codificar aterra em I. Linearizar n≥2 é incompatível.
 * Peano ≠ serpentina. I ≠ V_grade. Sem objecto novo.
 */
export function viznisoRamos () {
  const n = DOBRAS_GERADORAS.length
  const um = 1
  const dois = 2
  const doisN = grau2n(n)
  const contraArvore = viznisoBloqueia(n, um)
  const contraRegua = viznisoBloqueia(n, dois)
  const n1 = !viznisoBloqueia(1, dois)
  const I = Object.freeze({
    objecto: 'I',
    expressao: EXPR_COD,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['V_grade', 'grafo', 'lei_gk']),
  })
  const linearizar = Object.freeze({
    objecto: 'linearizar',
    expressao: EXPR_LIN,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['codificar', 'isomorfismo', 'lei_gk']),
  })
  const bloqueio = Object.freeze({
    objecto: 'bloqueio',
    expressao: EXPR_BLOQ,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['dinamica', 'isomorfismo', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:vizniso',
    funil: FUNIL_CASA,
    I,
    linearizar,
    bloqueio,
    testemunha: Object.freeze({
      n, um, dois, doisN, contraArvore, contraRegua, n1,
      distinto: doisN > um && doisN > dois,
    }),
    distintos: I.objecto !== linearizar.objecto && EXPR_COD !== EXPR_LIN,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:cor:matriz: I×I é índice, não achata. Conteúdo A ≠ índice.
 * 4>2 bloqueia. 1×1 = escalar. Sem categoria nova.
 */
export function matrizRamos () {
  const n = 2
  const quatro = grau2n(n)
  const achata = viznisoBloqueia(n, 2)
  const escalar = !viznisoBloqueia(1, 2)
  const I = Object.freeze({
    objecto: 'I',
    expressao: EXPR_COD,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['IxI', 'matriz', 'lei_gk']),
  })
  const indice = Object.freeze({
    objecto: 'IxI',
    expressao: EXPR_IXI,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['I', 'categoria', 'espaco-novo', 'lei_gk']),
  })
  const conteudo = Object.freeze({
    objecto: 'conteudo',
    expressao: EXPR_CONT,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['indice', 'categoria', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:cor:matriz',
    funil: FUNIL_CASA,
    I,
    indice,
    conteudo,
    testemunha: Object.freeze({
      n, quatro, achata, escalar, distinto: quatro !== 2,
    }),
    distintos: I.objecto !== indice.objecto && EXPR_IXI !== EXPR_CONT,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:def:palavra: a palavra é codificação em I. Alfabeto |A|=|V|.
 * T realiza a letra, ≠ T da árvore. 2n ≠ m. Sem casa nova.
 */
export function palavraRamos () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const n = DOBRAS_GERADORAS.length
  const m = grauM(n)
  const doisN = grau2n(n)
  const palavra = Object.freeze({
    objecto: 'palavra',
    expressao: EXPR_PAL,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['espaco', 'grafo', 'lei_gk']),
  })
  const alfabeto = Object.freeze({
    objecto: 'alfabeto',
    expressao: EXPR_ALF,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['A', '2n', 'categoria', 'lei_gk']),
  })
  const Tletra = Object.freeze({
    objecto: 'T_letra',
    expressao: EXPR_TLETRA,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['T', 'afim', 'realizacao-nova', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:def:palavra',
    funil: FUNIL_CASA,
    palavra,
    alfabeto,
    Tletra,
    testemunha: Object.freeze({ n, m, doisN, distinto: m !== doisN }),
    distintos: palavra.objecto !== Tletra.objecto && alfabeto.expressao !== EXPR_A,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:palavra: reconstrução aterra em I. G>1 e ker w são propriedades.
 * Comuta ≠ cancela. Dobra da colagem ≠ vizdobra. Sem casa nova.
 */
export function palavraTeorema () {
  const cruzLe = cruzarIdentidade({
    hospede: 'X', relacao: 'le', realizacaoPaper: 'B', realizacaoBanco: 'B',
  })
  const letras = Object.freeze([ek(0), ek(0)])
  const soma = xorPalavra(letras)
  const colidem = prefixosColidem(letras)
  const noKer = soma === 0
  const recon = Object.freeze({
    objecto: 'reconstrucao',
    expressao: EXPR_REC,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: cruzLe.compativel && soma === 0 ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['espaco-novo', 'lei_gk']),
  })
  const g1 = Object.freeze({
    objecto: 'G1',
    expressao: EXPR_G1,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: LEITURA_COORD.id,
    propriedade: true,
    estatuto: colidem ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['gerador', 'vizdobra', 'lei_gk']),
  })
  const ker = Object.freeze({
    objecto: 'ker_w',
    expressao: EXPR_KER,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: noKer && colidem ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['nucleo', 'gerador', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:palavra',
    funil: FUNIL_CASA,
    recon,
    g1,
    ker,
    testemunha: Object.freeze({ letras, soma, colidem, noKer, fecha: colidem && noKer }),
    distintos: recon.objecto !== g1.objecto && g1.objecto !== ker.objecto,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:contraria: π cola (G>1); ρ conta e paga buraco.
 * π ≠ ρ. Buraco ≠ dobra. Injetivo ≠ invertível. Sem realização nova.
 */
export function contrariaRamos () {
  const n = 3
  const im = []
  const visto = new Set()
  let inj = true
  for (let t = 0; t < n; t++) {
    const y = rho2t(t)
    if (visto.has(y)) inj = false
    visto.add(y)
    im.push(y)
  }
  const Imax = 2 * n
  const surj = im.length === Imax
  const buracosN = []
  for (let i = 0; i < Imax; i++) {
    if (!visto.has(i)) buracosN.push(i)
  }
  const rho = Object.freeze({
    objecto: 'rho',
    expressao: EXPR_RHO,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: inj && !surj ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['inversa', 'pi', '2_B', 'realizacao-nova', 'Duo', 'lei_gk']),
  })
  const buraco = Object.freeze({
    objecto: 'buraco',
    expressao: EXPR_BUR,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: buracosN.length === n ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['dobra', 'G1', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:contraria',
    funil: FUNIL_CASA,
    rho,
    buraco,
    testemunha: Object.freeze({
      n, im, Imax, inj, surj, buracos: buracosN,
      distinto: inj && !surj && buracosN.length === n,
    }),
    distintos: rho.objecto !== buraco.objecto && EXPR_RHO !== EXPR_G1,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:cor:folga: a mesma folga nos dois mapas. Duas expressões, um objecto.
 * Folga ≠ Duo. Sem casa nova.
 */
export function folgaRamos () {
  const nX = 3
  const nI = 6
  const g = folga(nI, nX)
  const r = rhoBuracos(nX)
  const mesma = g === r
  const cola = Object.freeze({
    objecto: 'folga',
    expressao: EXPR_G1SUM,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: mesma ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Duo', 'dobra', 'lei_gk']),
  })
  const conta = Object.freeze({
    objecto: 'folga',
    expressao: EXPR_FOL,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: mesma ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Duo', 'buraco-novo', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:cor:folga',
    funil: FUNIL_CASA,
    cola,
    conta,
    testemunha: Object.freeze({ nX, nI, g, r, mesma }),
    mesmoObjecto: cola.objecto === conta.objecto,
    distintos: cola.expressao !== conta.expressao,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:bijeccao: s é bijeção, G=1, folga=0. Codifica, não lineariza.
 * Bijeção ≠ Iso ≠ vizinhança. Peano ≠ serpentina. Sem kind novo.
 */
export function bijeccaoRamos () {
  const M = 2
  const n = M * M
  const visto = new Set()
  const celulas = []
  let ok = true
  for (let t = 0; t < n; t++) {
    const p = serpentina(t, M)
    celulas.push(p)
    if (p[0] < 0 || p[0] >= M || p[1] < 0 || p[1] >= M) ok = false
    if (serpentinaInv(p, M) !== t) ok = false
    const k = p[0] + ',' + p[1]
    if (visto.has(k)) ok = false
    visto.add(k)
  }
  const bij = ok && visto.size === n
  const s = Object.freeze({
    objecto: 's',
    expressao: EXPR_S,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: bij ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Iso', 'vizinhanca', 'Peano', 'lei_gk']),
  })
  const gum = Object.freeze({
    objecto: 'G1um',
    expressao: EXPR_GUM,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: bij ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['G1', 'gerador', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:bijeccao',
    funil: FUNIL_CASA,
    s,
    gum,
    testemunha: Object.freeze({
      M, n, bij, folgaZero: folga(n, n) === 0,
      celulas: Object.freeze(celulas),
    }),
    distintos: s.objecto !== gum.objecto && EXPR_GUM !== EXPR_G1,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:enumera: varphi é bijeção posicional. Cadeia = leitura da tupla.
 * Posicional ≠ vizinhança. varphi ≠ s. Enumeração ≠ cadeia geradora. Sem kind novo.
 */
export function enumeraRamos () {
  const M = 2
  const posto = 2
  const N = M ** posto
  const visto = new Set()
  const celulas = []
  let ok = true
  let bit = true
  for (let t = 0; t < N; t++) {
    const p = phiEnumera(t, M, posto)
    celulas.push(p)
    if (p.length !== posto) ok = false
    if (phiInv(p, M) !== t) ok = false
    if (coord(t, 0) !== p[0] || coord(t, 1) !== p[1]) bit = false
    const k = p.join(',')
    if (visto.has(k)) ok = false
    visto.add(k)
  }
  const bij = ok && visto.size === N
  const s2 = serpentina(2, M)
  const p2 = phiEnumera(2, M, posto)
  const distintoS = !(s2[0] === p2[0] && s2[1] === p2[1])
  const vizGrade = Math.abs(celulas[0][0] - celulas[2][0]) + Math.abs(celulas[0][1] - celulas[2][1]) === 1
  const vizI = Math.abs(2 - 0) === 1
  const guardaViz = !(vizGrade && !vizI)
  const phi = Object.freeze({
    objecto: 'varphi',
    expressao: EXPR_VARPHI,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    estatuto: bij ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['cadeia', 'vizinhanca', 's', 'lei_gk']),
  })
  const pos = Object.freeze({
    objecto: 'varphi',
    expressao: EXPR_POSL,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: bij ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['cadeia', 'vizinhanca', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:enumera',
    funil: FUNIL_CASA,
    phi,
    pos,
    testemunha: Object.freeze({
      M, posto, N, bij, bit, distintoS, guardaViz,
      celulas: Object.freeze(celulas),
    }),
    mesmoObjecto: phi.objecto === pos.objecto,
    distintos: phi.expressao !== pos.expressao && EXPR_VARPHI !== EXPR_S,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:navega: D=sup d_x é a mesma ultramétrica de T. Compor ≠ acumular.
 * d_x ≠ D. Travessia produz endereço, não estado. Sem kind novo.
 */
export function navegaRamos () {
  const M = 2
  const p = 2
  const addrPhi = []
  const addrS = []
  for (let y = 0; y < M; y++) {
    for (let x = 0; x < M; x++) {
      const xy = [x, y]
      addrPhi.push(phiInv(xy, M))
      addrS.push(serpentinaInv(xy, M))
    }
  }
  const zero = supDx(addrPhi, addrPhi, p)
  const pior = supDx(addrPhi, addrS, p)
  const sim = supDx(addrPhi, addrS, p) === supDx(addrS, addrPhi, p)
  const naoExcede = zero <= Math.max(pior, pior)
  const naoIgual = zero !== pior
  const dx = Object.freeze({
    objecto: 'T',
    expressao: EXPR_DX,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    propriedade: true,
    estatuto: sim && naoExcede ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['D', 'estado', 'I', 'lei_gk']),
  })
  const D = Object.freeze({
    objecto: 'T',
    expressao: EXPR_DMAX,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    propriedade: true,
    estatuto: sim && naoExcede ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['metrica', 'estado', 'acumulo', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:navega',
    funil: FUNIL_CASA,
    dx,
    D,
    testemunha: Object.freeze({
      M, p, zero, pior, sim, naoExcede, naoIgual,
      addrPhi: Object.freeze(addrPhi),
      addrS: Object.freeze(addrS),
    }),
    mesmoObjecto: dx.objecto === D.objecto,
    distintos: dx.expressao !== D.expressao,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:prop:travessia: T=S∘R^{-1} permuta dígitos. Preço em π, não em X.
 * T_trav ≠ T. π_perm ≠ π_r ≠ percurso. Sem dinâmica nova.
 */
export function travessiaRamos () {
  const M = 2
  const n = 2
  const rId = Object.freeze([0, 1])
  const rSwap = Object.freeze([1, 0])
  const piId = permPi(rId, rId)
  const piSwap = permPi(rSwap, rId)
  const addrR = []
  const addrS = []
  const visto = new Set()
  let escreve = true
  for (let y = 0; y < M; y++) {
    for (let x = 0; x < M; x++) {
      const xs = [x, y]
      const aR = leituraR(xs, rId, M)
      const aS = leituraR(xs, rSwap, M)
      addrR.push(aR)
      addrS.push(aS)
      if (aplicaTrav(aR, piSwap, M, n) !== aS) escreve = false
      visto.add(aplicaTrav(aR, piSwap, M, n))
    }
  }
  const bij = visto.size === M ** n
  const Did = supDx(addrR, addrR, n)
  const Dswap = supDx(addrR, addrS, n)
  const precoOk = Did === precoPi(piId) && Dswap === precoPi(piSwap)
  const te = new Set([aplicaTrav(1, piSwap, M, n), aplicaTrav(2, piSwap, M, n)])
  const mu = te.size === 2 && bij
  const trav = Object.freeze({
    objecto: 'Trav',
    expressao: EXPR_TRAV,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: escreve && bij ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['T', 'dinamica', 'pi', 'lei_gk']),
  })
  const pip = Object.freeze({
    objecto: 'Trav',
    expressao: EXPR_PIP,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: precoOk ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['X', 'pi_r', 'percurso', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:prop:travessia',
    funil: FUNIL_CASA,
    trav,
    pip,
    testemunha: Object.freeze({
      M, n, escreve, bij, mu, precoOk,
      piId, piSwap, qId: qPi(piId), qSwap: qPi(piSwap),
      Did, Dswap, precoId: precoPi(piId), precoSwap: precoPi(piSwap),
    }),
    mesmoObjecto: trav.objecto === pip.objecto,
    distintos: trav.expressao !== pip.expressao && EXPR_TRAV !== EXPR_DMAX &&
      EXPR_PIP !== EXPR_PIR,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:def:duomorf: unicidade do mapa ≠ duomorfia das operações.
 * Bijeção ≠ Iso ≠ Duo. D_op ≠ D. Star(U)=D ≠ estrela de elemento.
 * Duo já está em Mor; não se promove a partir de Trav.
 */
export function duomorfRamos () {
  const T = travessiaRamos()
  const eixos = eixosF(T.testemunha.bij, null)
  const unica = Object.freeze({
    objecto: 'Trav',
    expressao: EXPR_UNI,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: eixos.bij ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Duo', 'Iso', 'lei_gk']),
  })
  const duo = Object.freeze({
    objecto: 'Duo',
    expressao: EXPR_DOP,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    motivo: 'estrutural',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Trav', 'estrela', 'D', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:def:duomorf',
    funil: FUNIL_CASA,
    unica,
    duo,
    testemunha: Object.freeze({
      bij: eixos.bij,
      iso: eixos.iso,
      duo: eixos.duo,
      unica: T.testemunha.bij,
      promovido: false,
    }),
    mesmoObjecto: unica.objecto === duo.objecto,
    distintos: unica.objecto !== duo.objecto && EXPR_UNI !== EXPR_DOP &&
      EXPR_DOP !== EXPR_DMAX,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:obs:octoniao-interface: Lei 7, duas faces, interface. Sem categoria.
 * Hexal ≠ Duo. Ligar ≠ fundir. Oito ≠ Lei 8. Relógio não escolhe a interface.
 */
export function octoniaoObs () {
  const oito = LEIS_BASE.length
  const l7 = LEIS_BASE[7] === 7
  const lei8 = LEIS_BASE.includes(8)
  const hx = 6
  const faces = Object.freeze({
    objecto: 'faces',
    expressao: EXPR_FACES,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: l7 && oito === 8 && !lei8 ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['categoria', 'fusao', 'Fix', 'lei_gk']),
  })
  const iface = Object.freeze({
    objecto: 'interface',
    expressao: EXPR_IF,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    motivo: 'estrutural',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['categoria', 'hexal', 'relogio', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:obs:octoniao-interface',
    funil: FUNIL_CASA,
    faces,
    iface,
    testemunha: Object.freeze({
      oito, l7, lei8, hexal: hx, dim: DIM_X, dobras: DOBRAS_GERADORAS.length,
      cadeiaObs: 'L7>faces>assimetria>interface>Duo',
      cadeiaGer: CADEIA.join('>'),
    }),
    mesmoObjecto: faces.objecto === iface.objecto,
    distintos: faces.objecto !== iface.objecto && EXPR_FACES !== EXPR_IF &&
      hx !== 7 && CADEIA.join('>') !== 'L7>faces>assimetria>interface>Duo',
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:def:dominio-interface: M=O⊔T delimita o domínio. Interface ≠ transporte.
 * T_fio ≠ T. Q_ab vive em O; não são 4 operadores sobre M. Duo em T é N/A.
 */
export function dominioInterface () {
  const Q = quartetoN2()
  const part = Object.freeze({
    objecto: 'M',
    expressao: EXPR_M,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: 'desconhecido',
    motivo: 'estrutural',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['espaco', 'MOVE', 'categoria', 'lei_gk']),
  })
  const tfio = Object.freeze({
    objecto: 'T_fio',
    expressao: EXPR_TFIO,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    motivo: 'estrutural',
    na: true,
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['T', 'Trav', 'Duo', 'lei_gk']),
  })
  const iface = Object.freeze({
    objecto: 'interface',
    expressao: EXPR_IF,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    estatuto: 'desconhecido',
    motivo: 'estrutural',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['transporte', 'categoria', 'MOVE', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:def:dominio-interface',
    funil: FUNIL_CASA,
    part,
    tfio,
    iface,
    qab: Object.freeze({
      objecto: Q.objecto,
      casa: Q.casa,
      estatuto: Q.estatuto,
      n: Q.n,
      viveEmO: Q.estatuto === 'alojado',
      operadoresDeM: false,
    }),
    testemunha: Object.freeze({
      quatro: Q.n === 4,
      vazio: true,
      duoEmT: false,
      move: false,
    }),
    mesmoObjecto: false,
    distintos: part.objecto !== tfio.objecto && tfio.objecto !== 'T' &&
      EXPR_M !== EXPR_TFIO && EXPR_IF !== EXPR_TFIO,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:eixos: bit a é dobra (duas espécies, não três). G mede bijeção.
 * G=1 ≠ Iso. Eixo a ≠ eixo G. Sem kind novo.
 */
export function eixosRamos () {
  const B = bijeccaoRamos()
  const C = contrariaRamos()
  const gum = B.testemunha.bij && B.testemunha.folgaZero
  const eG1 = eixosF(gum, null)
  const dobraA = compoeA(0, 0) === 0 && compoeA(1, 1) === 0 &&
    compoeA(0, 1) === 1 && compoeA(1, 0) === 1
  const eixoA = Object.freeze({
    objecto: 'a',
    expressao: EXPR_AA,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: dobraA ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['G', 'bijeccao', 'lei_gk']),
  })
  const eixoG = Object.freeze({
    objecto: 'G',
    expressao: EXPR_GMEDE,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: gum && C.testemunha.inj && !C.testemunha.surj ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Iso', 'Duo', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:eixos',
    funil: FUNIL_CASA,
    eixoA,
    eixoG,
    testemunha: Object.freeze({
      dobraA, gum,
      bijDeG1: eG1.bij, isoDeG1: eG1.iso, duoDeG1: eG1.duo,
      rhoInj: C.testemunha.inj, rhoSurj: C.testemunha.surj,
    }),
    mesmoObjecto: eixoA.objecto === eixoG.objecto,
    distintos: eixoA.objecto !== eixoG.objecto && EXPR_AA !== EXPR_GMEDE &&
      EXPR_GMEDE !== EXPR_GUM,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:dual-involutiva: D²=id por igualdade estrutural, não por Iso.
 * Duas dobras ≠ terceiro estado. Não toca em G. Sem kind novo.
 */
export function dualInvolutiva () {
  const ops = Object.freeze(['oplus', 'otimes'])
  const uma = dualOps(ops)
  const duas = dualOps(uma)
  const troca = uma[0] === ops[1] && uma[1] === ops[0]
  const mesma = duas[0] === ops[0] && duas[1] === ops[1]
  const E = eixosRamos()
  const gIntacto = E.testemunha.gum === true && E.testemunha.isoDeG1 === false
  const d2 = Object.freeze({
    objecto: 'D_op',
    expressao: EXPR_D2,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: mesma && troca ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Iso', 'G', 'terceiro', 'lei_gk']),
  })
  const eq = Object.freeze({
    objecto: 'D_op',
    expressao: EXPR_EQA,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: mesma ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['copia', 'Iso', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:dual-involutiva',
    funil: FUNIL_CASA,
    d2,
    eq,
    testemunha: Object.freeze({
      troca, mesma, gIntacto,
      a2: compoeA(1, 1),
      isoDeG1: E.testemunha.isoDeG1,
    }),
    mesmoObjecto: d2.objecto === eq.objecto,
    distintos: d2.expressao !== eq.expressao && EXPR_D2 !== EXPR_DOP &&
      EXPR_D2 !== EXPR_DMAX,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:duo-composicao: a tábua é a paridade de a. Compôr = somar a.
 * Tábua ≠ objecto. Bit b independente, não alojado. Sem kind novo.
 */
export function duoComposicao () {
  const fecha = compoeA(0, 0) === 0 && compoeA(0, 1) === 1 &&
    compoeA(1, 0) === 1 && compoeA(1, 1) === 0
  const par = paridadeD(0) === 0 && paridadeD(1) === 1 &&
    paridadeD(2) === 0 && paridadeD(3) === 1
  const I = dualInvolutiva()
  const tabua = Object.freeze({
    objecto: 'a',
    expressao: EXPR_TAB,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: fecha && par ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['objecto', 'G', 'b', 'lei_gk']),
  })
  const soma = Object.freeze({
    objecto: 'a',
    expressao: EXPR_SOMA,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: fecha ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['tabua-nova', 'B-nova', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:duo-composicao',
    funil: FUNIL_CASA,
    tabua,
    soma,
    testemunha: Object.freeze({
      fecha, par,
      d2: I.testemunha.mesma,
      gIntacto: I.testemunha.gIntacto,
    }),
    mesmoObjecto: tabua.objecto === soma.objecto,
    distintos: tabua.expressao !== soma.expressao && EXPR_TAB !== EXPR_AA,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:quarteto: Q_ab(σ) ≠ Q_ab(D,†). Mesmo rótulo, estruturas diferentes.
 * (a,b)∈B² são quatro quadrantes, não quatro objectos. Sem casa nova. Sem kind.
 */
export function quartetoDuo () {
  const S = quartetoN2()
  const pares = Object.freeze([
    parAB(0, 0), parAB(0, 1), parAB(1, 0), parAB(1, 1),
  ])
  const rotulos = Object.freeze(pares.map(rotuloPar))
  const quatro = rotulos.length === 4 && new Set(rotulos).size === 4
  let fecha = true
  let invol = true
  for (const p of pares) {
    const qq = compoePar(p, p)
    if (qq.a !== 0 || qq.b !== 0) invol = false
    for (const q of pares) {
      const r = compoePar(p, q)
      if (!rotulos.includes(rotuloPar(r))) fecha = false
    }
  }
  const C = duoComposicao()
  const sigma = Object.freeze({
    objecto: 'Q_ab',
    expressao: EXPR_QAB,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'gera',
    papel: 'gerador',
    casa: GERADOR_VIZDOBRA.id,
    propriedade: false,
    estatuto: S.estatuto,
    n: S.n,
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Q_dt', 'fund', 'lei_gk']),
  })
  const dt = Object.freeze({
    objecto: 'a',
    expressao: EXPR_QDT,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: quatro && fecha && invol ? 'alojado' : 'desconhecido',
    n: 4,
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Q_ab', 'objecto', 'quinta', 'lei_gk']),
  })
  const quad = Object.freeze({
    objecto: 'a',
    expressao: EXPR_QUAD,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: quatro ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['objectos', 'kind', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:quarteto',
    funil: FUNIL_CASA,
    sigma,
    dt,
    quad,
    testemunha: Object.freeze({
      quatro, fecha, invol,
      rotulos,
      rotulosIguais: rotulos.every((r, i) => r === ROTULOS_QAB[i]),
      constrDistintas: EXPR_QAB !== EXPR_QDT && S.construcao !== EXPR_QDT,
      objectosDistintos: sigma.objecto !== dt.objecto,
      qAbAge: qAb(1, 0, 0) === ek(0),
      gIntacto: C.testemunha.gIntacto,
    }),
    mesmoObjecto: false,
    distintos: sigma.expressao !== dt.expressao && EXPR_QAB !== EXPR_QDT,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:trial: centro e dois lados são posições de a, não três objectos.
 * a=0 Iso, a=1 Duo. Casa de leitura já existente. Sem kind novo.
 */
export function trialRamos () {
  const E = eixosRamos()
  const C = duoComposicao()
  const I = dualInvolutiva()
  const Q = quartetoN2()
  const Qd = quartetoDuo()
  const l0 = eixosF(true, 0)
  const l1 = eixosF(true, 1)
  const d0 = paridadeD(0) === 0
  const d1 = paridadeD(1) === 1
  const centro = Object.freeze({
    objecto: 'a',
    expressao: EXPR_CTR,
    suporte: 'X',
    realizacao: 'B',
    relacao: 'le',
    papel: 'leitura',
    casa: LEITURA_COORD.id,
    propriedade: true,
    estatuto: 'alojado',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['objecto', 'absorcao', 'vinco', 'lei_gk']),
  })
  const lado0 = Object.freeze({
    objecto: 'a',
    expressao: EXPR_L0,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: l0.iso && d0 ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['casa', 'Iso-novo', 'lei_gk']),
  })
  const lado1 = Object.freeze({
    objecto: 'a',
    expressao: EXPR_L1,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: l1.duo && d1 ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['casa', 'Duo-novo', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:trial',
    funil: FUNIL_CASA,
    centro,
    lado0,
    lado1,
    testemunha: Object.freeze({
      iso0: l0.iso, duo0: l0.duo,
      iso1: l1.iso, duo1: l1.duo,
      d0, d1,
      soma: C.testemunha.fecha,
      d2: I.testemunha.mesma,
      duasEspecies: E.testemunha.dobraA,
      qAbSigma: Q.estatuto === 'alojado',
      qAbDistintos: Qd.testemunha.objectosDistintos,
    }),
    mesmoObjecto: centro.objecto === lado0.objecto &&
      lado0.objecto === lado1.objecto,
    distintos: centro.expressao !== lado0.expressao &&
      lado0.expressao !== lado1.expressao && EXPR_L0 !== EXPR_L1,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:duasfaces: duas faces, uma equação, um único objecto a.
 * Face 0/1 = a=0/1. Equação é propriedade, não lei. Sem kind novo.
 */
export function duasFacesRamos () {
  const Tr = trialRamos()
  const O = octoniaoObs()
  const l0 = eixosF(true, 0)
  const l1 = eixosF(true, 1)
  const d0 = paridadeD(0) === 0
  const d1 = paridadeD(1) === 1
  const face0 = Object.freeze({
    objecto: 'a',
    expressao: EXPR_F0,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: l0.iso && d0 ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['objecto', 'faces', 'lei_gk']),
  })
  const face1 = Object.freeze({
    objecto: 'a',
    expressao: EXPR_F1,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: l1.duo && d1 ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['objecto', 'faces', 'lei_gk']),
  })
  const eq = Object.freeze({
    objecto: 'a',
    expressao: EXPR_EQF,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: face0.estatuto === 'alojado' && face1.estatuto === 'alojado'
      ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['lei', 'estrela', 'rotor', 'terceira', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:duasfaces',
    funil: FUNIL_CASA,
    face0,
    face1,
    eq,
    testemunha: Object.freeze({
      iso0: l0.iso, duo0: l0.duo,
      iso1: l1.iso, duo1: l1.duo,
      d0, d1,
      trialA: Tr.mesmoObjecto,
      octFaces: O.faces.objecto,
      umaEquacao: face0.objecto === face1.objecto &&
        face1.objecto === eq.objecto,
    }),
    mesmoObjecto: face0.objecto === face1.objecto &&
      face1.objecto === eq.objecto,
    distintos: face0.expressao !== face1.expressao &&
      face1.expressao !== eq.expressao && EXPR_F0 !== EXPR_FACES &&
      EXPR_EQF !== EXPR_L0,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:estrela: faces iguais é condição sobre a, não casa nova.
 * Star(U)=D ≠ x^2=x+1. Em B a condição é incompatível → desconhecido.
 */
export function estrelaRamos () {
  const F = duasFacesRamos()
  const E = eixosRamos()
  const Q = quartetoN2()
  const cabem = estrelaEmB()
  const face0 = Object.freeze({
    objecto: 'a',
    expressao: EXPR_F0,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: F.face0.estatuto,
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['objecto', 'G', 'lei_gk']),
  })
  const face1 = Object.freeze({
    objecto: 'a',
    expressao: EXPR_F1,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: F.face1.estatuto,
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['objecto', 'G', 'lei_gk']),
  })
  const st = Object.freeze({
    objecto: 'a',
    expressao: EXPR_ST,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: cabem ? 'alojado' : 'desconhecido',
    motivo: cabem ? '' : 'estrutural',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Q_ab', 'F8', 'gerador', 'D', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:estrela',
    funil: FUNIL_CASA,
    face0,
    face1,
    st,
    testemunha: Object.freeze({
      cabem,
      starU: 'D',
      iso0: F.testemunha.iso0,
      duo1: F.testemunha.duo1,
      eixoA: E.eixoA.objecto,
      qAb: Q.objecto,
      gIntacto: E.testemunha.isoDeG1 === false,
    }),
    mesmoObjecto: face0.objecto === face1.objecto &&
      face1.objecto === st.objecto,
    distintos: face0.expressao !== face1.expressao &&
      EXPR_ST !== EXPR_EQF && EXPR_ST !== EXPR_FACES,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:thm:estrelaencaixe: Zc realiza a condição em inteiros.
 * Realizar em Zc ≠ promover em B. Sem kind novo. Sem terceira realização.
 */
export function estrelaEncaixe () {
  const S = estrelaRamos()
  const zc = estrelaEmZc()
  const enc = Object.freeze({
    objecto: 'a',
    expressao: EXPR_ENC,
    suporte: 'X',
    realizacao: 'Zc',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: zc ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['categoria', 'B', 'kind', 'lei_gk']),
  })
  const emB = Object.freeze({
    objecto: 'a',
    expressao: EXPR_ST,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: S.st.estatuto,
    motivo: S.st.motivo,
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Zc', 'promocao', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:estrelaencaixe',
    funil: FUNIL_CASA,
    enc,
    emB,
    testemunha: Object.freeze({
      zc,
      b: estrelaEmB(),
      cassini: Object.freeze([cassini(0), cassini(1), cassini(2)]),
      starU: S.testemunha.starU,
    }),
    mesmoObjecto: enc.objecto === emB.objecto,
    distintos: enc.realizacao !== emB.realizacao && EXPR_ENC !== EXPR_ST,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

/**
 * fis:def:duoestrela / fis:thm:duoestrela: relação reversível entre faces.
 * Duo ≠ Star. Reversível ≠ Iso ≠ nova realização. Sem reabrir T_fio.
 */
export function duoEstrelaRamos () {
  const F = duasFacesRamos()
  const I = dualInvolutiva()
  const S = estrelaRamos()
  const Enc = estrelaEncaixe()
  const O = octoniaoObs()
  const M = dominioInterface()
  const duo = eixosF(true, 1)
  const iso = eixosF(true, 0)
  const volta = compoeA(1, 1) === 0
  const rev = Object.freeze({
    objecto: 'a',
    expressao: EXPR_REV,
    suporte: 'X',
    realizacao: 'B',
    relacao: '',
    papel: '',
    casa: '',
    propriedade: true,
    estatuto: volta && I.testemunha.mesma ? 'alojado' : 'desconhecido',
    cria: false,
    tipoNovo: false,
    nao: Object.freeze(['Iso', 'T_fio', 'realizacao', 'lei_gk']),
  })
  return Object.freeze({
    origem: 'fis:thm:duoestrela',
    def: 'fis:def:duoestrela',
    funil: FUNIL_CASA,
    face0: F.face0,
    face1: F.face1,
    rev,
    testemunha: Object.freeze({
      volta,
      d2: I.testemunha.mesma,
      duo: duo.duo, isoDuo: duo.iso,
      iso: iso.iso,
      starU: S.testemunha.starU,
      stB: S.st.estatuto,
      encZc: Enc.enc.estatuto,
      iface: O.iface.estatuto,
      tfio: M.tfio.estatuto,
    }),
    mesmoObjecto: F.face0.objecto === F.face1.objecto &&
      F.face1.objecto === rev.objecto,
    distintos: EXPR_REV !== EXPR_ST && EXPR_REV !== EXPR_IF &&
      EXPR_REV !== EXPR_TFIO && EXPR_F0 !== EXPR_F1,
    cria: false,
    tipoNovo: false,
    f8: false,
  })
}

function destino (objecto, papel, estatuto, relacao, motivo) {
  return Object.freeze({
    objecto, papel, estatuto, relacao, motivo: motivo || '', tipoNovo: false,
  })
}

/**
 * Destino por objecto, não por teorema.
 * O mesmo enunciado pode alojar uns e deixar outros desconhecidos.
 */
export function destinosDoTeorema (origem) {
  const o = String(origem || '')
  if (o === 'fis:thm:base') {
    return Object.freeze([
      destino('e_k', 'espaco', 'alojado', ''),
      destino('coord-byte', 'leitura', 'alojado', 'le'),
      destino('g^(k)', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:def:dual') {
    return Object.freeze([
      destino('paridade', 'leitura', 'alojado', 'le'),
      destino('chi', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:fecho') {
    return Object.freeze([
      destino('R_ij', '', 'desconhecido', '', 'estrutural'),
    ])
  }
  if (o === 'fis:thm:quarteto') {
    return Object.freeze([
      destino('Q_ab', 'gerador', 'alojado', 'gera'),
    ])
  }
  if (o === 'fis:def:doisandares') {
    return Object.freeze([
      destino('X', 'espaco', 'alojado', ''),
      destino('V', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:def:conv') {
    return Object.freeze([
      destino('xor', '', 'alojado', 'gera'),
      destino('*', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:lem:comuta') {
    return Object.freeze([
      destino('C_f', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:proprios') {
    return Object.freeze([
      destino('C_f', '', 'desconhecido', '', 'algebrica'),
      destino('hat', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:def:transf') {
    return Object.freeze([
      destino('hat', '', 'desconhecido', '', 'algebrica'),
      destino('H', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:H') {
    return Object.freeze([
      destino('H', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:dobranorma') {
    return Object.freeze([
      destino('norma-V', '', 'desconhecido', '', 'algebrica'),
      destino('2P', '', 'desconhecido', '', 'algebrica'),
      destino('F(G)', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:cor:modos') {
    return Object.freeze([
      destino('hamming', 'leitura', 'alojado', 'le'),
      destino('alpha', '', 'desconhecido', '', 'algebrica'),
      destino('A', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:conv') {
    return Object.freeze([
      destino('produto', '', 'desconhecido', '', 'algebrica'),
      destino('C_f', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:topo-tese' || o === 'fis:def:viztopo') {
    return Object.freeze([
      destino('enderecar', 'leitura', 'alojado', 'le'),
      destino('V_E', '', 'alojado', ''),
      destino('V_topo', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:def:arvore') {
    return Object.freeze([
      destino('Qm', 'leitura', 'alojado', 'le'),
      destino('T', 'leitura', 'alojado', 'le'),
      destino('I', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:lem:ultra') {
    return Object.freeze([
      destino('T', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:thm:bolas') {
    return Object.freeze([
      destino('B_cal', 'leitura', 'alojado', 'le'),
      destino('condicao', 'leitura', 'alojado', 'le'),
      destino('escala', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:vizarvore') {
    return Object.freeze([
      destino('N_T', 'leitura', 'alojado', 'le'),
      destino('cardinalidade', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:def:percurso') {
    return Object.freeze([
      destino('I', 'leitura', 'alojado', 'le'),
      destino('percurso', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:thm:euler') {
    return Object.freeze([
      destino('chi_e', 'leitura', 'alojado', 'le'),
      destino('ciclo', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:thm:handshake') {
    return Object.freeze([
      destino('conta', 'leitura', 'alojado', 'le'),
      destino('incidencia', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:def:grade') {
    return Object.freeze([
      destino('grade', '', 'desconhecido', '', 'algebrica'),
      destino('V_grade', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:dim') {
    return Object.freeze([
      destino('card_grade', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:vizdobra') {
    return Object.freeze([
      destino('vizdobra', 'gerador', 'alojado', 'gera'),
      destino('pares', '', 'alojado', ''),
      destino('sigma_grade', '', 'desconhecido', '', 'algebrica'),
    ])
  }
  if (o === 'fis:thm:vizniso') {
    return Object.freeze([
      destino('I', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:cor:matriz') {
    return Object.freeze([
      destino('I', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:def:palavra') {
    return Object.freeze([
      destino('palavra', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:thm:palavra') {
    return Object.freeze([
      destino('reconstrucao', 'leitura', 'alojado', 'le'),
      destino('G1', '', 'alojado', ''),
      destino('ker_w', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:thm:contraria') {
    return Object.freeze([
      destino('rho', 'leitura', 'alojado', 'le'),
      destino('buraco', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:cor:folga') {
    return Object.freeze([
      destino('folga', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:thm:bijeccao') {
    return Object.freeze([
      destino('s', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:thm:enumera') {
    return Object.freeze([
      destino('varphi', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:thm:navega') {
    return Object.freeze([
      destino('T', 'leitura', 'alojado', 'le'),
    ])
  }
  if (o === 'fis:prop:travessia') {
    return Object.freeze([
      destino('Trav', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:def:duomorf') {
    return Object.freeze([
      destino('Trav', '', 'alojado', ''),
      destino('Duo', '', 'desconhecido', '', 'estrutural'),
    ])
  }
  if (o === 'fis:obs:octoniao-interface') {
    return Object.freeze([
      destino('faces', '', 'alojado', ''),
      destino('interface', '', 'desconhecido', '', 'estrutural'),
    ])
  }
  if (o === 'fis:def:dominio-interface') {
    return Object.freeze([
      destino('Q_ab', 'gerador', 'alojado', 'gera'),
      destino('T_fio', '', 'desconhecido', '', 'estrutural'),
      destino('interface', '', 'desconhecido', '', 'estrutural'),
    ])
  }
  if (o === 'fis:thm:eixos') {
    return Object.freeze([
      destino('G', '', 'alojado', ''),
      destino('Iso', '', 'desconhecido', '', 'estrutural'),
    ])
  }
  if (o === 'fis:thm:dual-involutiva') {
    return Object.freeze([
      destino('D_op', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:thm:duo-composicao') {
    return Object.freeze([
      destino('a', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:thm:trial') {
    return Object.freeze([
      destino('a', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:thm:duasfaces') {
    return Object.freeze([
      destino('a', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:thm:estrela') {
    return Object.freeze([
      destino('a', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:thm:estrelaencaixe') {
    return Object.freeze([
      destino('a', '', 'alojado', ''),
    ])
  }
  if (o === 'fis:def:duoestrela' || o === 'fis:thm:duoestrela') {
    return Object.freeze([
      destino('a', '', 'alojado', ''),
    ])
  }
  return Object.freeze([])
}

/** Papel estrutural. Lei ≠ gerador ≠ realização. Cruzamento não vaza. */
export function classificaObjecto (o) {
  const papel = (o && o.papel) || ''
  const cruz = (o && ((o.refs && o.refs.cruz) || o.cruz)) || ''
  const n = Number((o && (o.nEvidencias != null ? o.nEvidencias : o.n)) || 0)
  return Object.freeze({
    papel: ONTOLOGIA.includes(papel) ? papel : '',
    cruz: CRUZ.includes(cruz) ? cruz : '',
    conhecida: papel === 'lei' && n >= LIMIAR_TESTEMUNHAS,
    colapsa: false,
    realiza: false,
    cena: false,
    f8: false,
  })
}

export function camposDoPapel (papel) {
  if (papel === 'lei') return CAMPOS_LEI
  if (papel === 'gerador') return CAMPOS_GERADOR
  if (papel === 'realizacao') return CAMPOS_REALIZACAO
  if (papel === 'espaco') return CAMPOS_ESPACO
  if (papel === 'leitura') return CAMPOS_LEITURA
  return Object.freeze([])
}

export function temCampos (o, papel) {
  const p = papel || (o && o.papel) || ''
  return camposDoPapel(p).every((k) => {
    if (k === 'testemunhas') return o && (o.testemunhas != null || o.nEvidencias != null)
    return o && Object.prototype.hasOwnProperty.call(o, k)
  })
}

/** Expressão já classificada como estrutura/gerador/realização/leitura não vira lei. */
export function expressaoNaoELei (expr) {
  const n = norma(expr)
  if (!n) return false
  if (REALIZACOES.some((r) => r.expressao === n || r.formula === n || r.objeto === n || r.id === n)) return true
  if (GERADORES.some((g) => g.objeto === n || g.expr === n || g.id === n)) return true
  if (SIGMAS.some((s) => s.objeto === n)) return true
  if (LEITURAS.some((l) => l.objeto === n || l.id === n)) return true
  const t = n.replace(/\s+/g, '')
  if (n === ESPACO.base || n === EXPR_BASE || n === ESPACO.id) return true
  if (t === 'dim=8' || /^dimX=8$/i.test(t)) return true
  if (t === 'Id' || /^Gram=Id$/i.test(t) || n === ESPACO.gram) return true
  if (t === 'coord-byte') return true
  if (CANDIDATOS.some((c) => c.id === n || c.id === t || c.expressao === n)) return true
  if (n === EXPR_ROTOR || t === 'rotor-fecho') return true
  if (n === EXPR_QUARTETO || n === EXPR_QAB || t === 'Q_ab') return true
  if (n === EXPR_PESO || t === 'g^(k)(b)=bAND2^k') return true
  if (n === EXPR_PAR || n === EXPR_CHI || n === EXPR_V || n === EXPR_XOR || n === EXPR_CONV) return true
  if (n === EXPR_CF || n === EXPR_HAT || n === EXPR_H || n === EXPR_NORMA_V || n === EXPR_2P) return true
  if (n === EXPR_HAM || n === EXPR_ALPHA || n === EXPR_A || n === EXPR_AUTO) return true
  if (n === EXPR_FG || n === EXPR_INJ || n === EXPR_INV) return true
  if (n === EXPR_END || n === EXPR_VE || n === EXPR_VIZ) return true
  if (n === EXPR_QM || n === EXPR_PROF || n === EXPR_DULTRA || n === EXPR_PERC) return true
  if (n === EXPR_SIMETRIA || n === EXPR_ULTRA) return true
  if (n === EXPR_BOLA || n === EXPR_PREF || n === EXPR_COND_D || n === EXPR_PIR) return true
  if (n === EXPR_NT || n === EXPR_CARD_T) return true
  if (n === EXPR_CAMINHO) return true
  if (n === EXPR_CHI_E || n === EXPR_CICLO) return true
  if (n === EXPR_HS || n === EXPR_INC) return true
  if (n === EXPR_GRADE || n === EXPR_VIZ_G || n === EXPR_DIM) return true
  if (n === EXPR_SIG_G || n === EXPR_PARES) return true
  if (n === EXPR_R4 || n === EXPR_R2 || n === EXPR_PLANOS) return true
  if (n === EXPR_COD || n === EXPR_LIN || n === EXPR_BLOQ) return true
  if (n === EXPR_IXI || n === EXPR_CONT || n === EXPR_ESC) return true
  if (n === EXPR_PAL || n === EXPR_ALF || n === EXPR_TLETRA) return true
  if (n === EXPR_REC || n === EXPR_G1 || n === EXPR_KER) return true
  if (n === EXPR_RHO || n === EXPR_BUR) return true
  if (n === EXPR_FOL || n === EXPR_G1SUM) return true
  if (n === EXPR_S || n === EXPR_GUM) return true
  if (n === EXPR_VARPHI || n === EXPR_POSL) return true
  if (n === EXPR_DX || n === EXPR_DMAX) return true
  if (n === EXPR_TRAV || n === EXPR_PIP) return true
  if (n === EXPR_UNI || n === EXPR_DOP) return true
  if (n === EXPR_FACES || n === EXPR_IF) return true
  if (n === EXPR_M || n === EXPR_TFIO) return true
  if (n === EXPR_AA || n === EXPR_GMEDE) return true
  if (n === EXPR_D2 || n === EXPR_EQA) return true
  if (n === EXPR_TAB || n === EXPR_SOMA) return true
  if (n === EXPR_QDT || n === EXPR_QUAD) return true
  if (n === EXPR_CTR || n === EXPR_L0 || n === EXPR_L1) return true
  if (n === EXPR_F0 || n === EXPR_F1 || n === EXPR_EQF) return true
  if (n === EXPR_ST || n === EXPR_ENC || n === EXPR_REV) return true
  return false
}

/** Conteúdo de X: ⟨σ₁,σ₂,σ₃⟩ gera dim=8, e_k=2^k, Gram=Id. Estrutura, não lei. */
export function estruturaX () {
  const gera = ARESTAS.find((a) => a.de === 'vizdobra' && a.rel === 'gera')
  const bits = SIGMAS.every((s) => ek(s.bit) === (1 << s.bit) && sigmaI(s.i, 0) === ek(s.bit))
  return Object.freeze({
    objeto: GERADOR_VIZDOBRA.objeto,
    estados: GERADOR_VIZDOBRA.estados,
    elementos: GERADOR_VIZDOBRA.n,
    gera: !!(gera && gera.para === ESPACO.id),
    dim: ESPACO.dim,
    base: ESPACO.base,
    gram: ESPACO.gram,
    gramId: gramOrtonormal(),
    oitoDeTres: ESPACO.dim === 2 ** GERADOR_VIZDOBRA.n,
    bits,
    lei_gk: 'N/A',
    f8: false,
  })
}

/** Grafo gerador. Leitura à parte. Relação não promove. */
export function grafoDependencia () {
  return Object.freeze({
    cadeia: CADEIA,
    arestas: ARESTAS,
    leituras: LEITURAS,
    arestasLeitura: ARESTAS_LEITURA,
    nLeis: LEIS_CANON.length,
    promove: false,
    realiza: false,
    f8: false,
  })
}

/** Leitura do byte. Observa X; não constrói X. */
export function registroLeitura () {
  return Object.freeze({
    id: LEITURA_COORD.id,
    papel: 'leitura',
    objeto: LEITURA_COORD.objeto,
    origem: LEITURA_COORD.origem,
    observado: LEITURA_COORD.observado,
    lei_gk: 'N/A',
    cadeia: false,
    ciclo: CICLO,
    realiza: false,
    f8: false,
  })
}

/** fis:thm:vizdobra → gerador:vizdobra. Um grupo, não três leis. */
export function registroVizdobra () {
  return Object.freeze({
    id: GERADOR_VIZDOBRA.id,
    papel: 'gerador',
    objeto: GERADOR_VIZDOBRA.objeto,
    estados: GERADOR_VIZDOBRA.estados,
    origem: GERADOR_VIZDOBRA.origem,
    elementos: GERADOR_VIZDOBRA.n,
    lei_gk: 'N/A',
    ciclo: CICLO,
    realiza: false,
    cena: false,
    f8: false,
  })
}

/** fis:thm:vinco → realizacao:vinco. Nunca lei_gk. */
export function registroVinco () {
  return Object.freeze({
    id: VINCO.id,
    papel: 'realizacao',
    objeto: VINCO.objeto,
    origem: VINCO.origem,
    endereco: VINCO.endereco,
    expressao: VINCO.expressao,
    cruz: VINCO.cruz,
    lei_gk: 'N/A',
    ciclo: CICLO,
    realiza: false,
    cena: false,
    f8: false,
  })
}

export const LEI_FASE_AURA = Object.freeze({
  id: ID_FASE_AURA,
  nome: 'fase aura',
  expressao: FASE_AURA,
  parametros: PARAMS_FASE_AURA.slice(),
  dominio: DOMINIO_FASE_AURA.slice(),
  instrumento: INSTRUMENTO_AURA,
  refs: refsDe(['fis:thm:largura'], [], 'relacionado'),
})
export const LEI_FP_1 = Object.freeze({
  id: ID_FP_1,
  nome: 'fator de potencia 1',
  expressao: 'FP=1',
  parametros: Object.freeze(['N']),
  dominio: Object.freeze([]),
  instrumento: '',
  refs: refsDe(['fis:em-pergunta'], [], 'relacionado'),
})
export const LEI_REVELA = Object.freeze({
  id: ID_REVELA,
  nome: 'revela',
  expressao: 'a: 0 → 1',
  parametros: Object.freeze(['destino']),
  dominio: Object.freeze([]),
  instrumento: '',
  refs: refsDe(['fis:thm:alonzo-par'], ['cat:audit:alonzo'], 'relacionado'),
})
export const LEI_TRIADE_FECHA = Object.freeze({
  id: ID_TRIADE_FECHA,
  nome: 'triade fecha',
  expressao: EXPR_TRIADE,
  parametros: Object.freeze(['n']),
  dominio: Object.freeze([]),
  instrumento: 'textura',
  refs: refsDe(['fis:cor:ciclo', 'fis:def:duomorf'], [], 'realizado'),
})
export const LEI_DUAL_SMIN = Object.freeze({
  id: ID_DUAL_SMIN,
  nome: 'dual smooth-min',
  expressao: 'smooth-min',
  parametros: Object.freeze([]),
  dominio: Object.freeze([]),
  instrumento: 'respira',
  refs: refsDe(['fis:thm:dual'], [], 'relacionado'),
})
export const LEIS_CANON = Object.freeze([
  LEI_FASE_AURA, LEI_FP_1, LEI_REVELA, LEI_TRIADE_FECHA, LEI_DUAL_SMIN,
])
/** Paper já tem a equação; o Reino não junta 12 testemunhas. Não promover. */
export const OBSERVACOES = Object.freeze([
  Object.freeze({
    id: 'phi2',
    expressao: EXPR_PHI2,
    refs: refsDe(['fis:thm:estrela'], ['cat:audit:alonzo'], 'realizado'),
  }),
  Object.freeze({
    id: 'sigma-dual',
    expressao: EXPR_SIGMA,
    refs: refsDe(['fis:def:duoestrela', 'fis:thm:estrela'], ['cat:audit:alonzo'], 'realizado'),
  }),
  Object.freeze({
    id: 'julia-c',
    expressao: EXPR_JULIA,
    refs: refsDe(['fis:thm:alonzo-par'], ['cat:def:alonzo', 'cat:audit:alonzo'], 'realizado'),
  }),
])

function norma (s) {
  return String(s || '').replace(/\s+/g, ' ').trim()
}

function tagVal (p, rot) {
  const t = ((p && p.tags) || []).find((x) => String(x.rot || '') === rot)
  return t ? String(t.val || '') : ''
}

export function reguaDoPeca (p) {
  return tagVal(p, ROT_REGUA)
}

/** Uma declaração por card: os quatro campos são distintos. */
export function declaracaoDoCard (p, secao) {
  return {
    secao: String(secao || ''),
    nome: String((p && p.nome) || ''),
    op: String((p && p.op) || ''),
    regua: reguaDoPeca(p),
    formula: faseNaDesc(p && p.desc),
    fp: tagVal(p, ROT_FP),
    revela: tagVal(p, ROT_REVELA),
    dual: tagVal(p, 'dual'),
    pbr: tagVal(p, 'PBR'),
    desc: String((p && p.desc) || ''),
    nTags: ((p && p.tags) || []).length,
  }
}

export function declaracoesDoMan (man) {
  const out = []
  for (const s of (man && man.secoes) || []) {
    for (const p of s.pecas || []) out.push(declaracaoDoCard(p, s.id))
  }
  return out
}

/** A declaração testemunha esta lei? n=k não é lei. */
export function declaraLei (decl, lei) {
  const id = (lei && lei.id) || ''
  if (id === ID_FASE_AURA) return norma(decl && decl.formula) === norma(lei.expressao)
  if (id === ID_FP_1) return String((decl && decl.fp) || '').startsWith('FP=1')
  if (id === ID_REVELA) return !!(decl && decl.revela)
  if (id === ID_TRIADE_FECHA) return String((decl && decl.desc) || '').includes('A tríade fecha')
  if (id === ID_DUAL_SMIN) return String((decl && decl.dual) || '').includes('smooth-min')
  return false
}

function blobDecl (d) {
  return [d.op, d.regua, d.fp, d.revela, d.pbr, d.dual, d.formula, d.desc].join(' ')
}

/** Normaliza um valor de legenda. Parâmetro ≠ lei. Braço ≠ lei nova. */
export function normalizaExpr (campo, val) {
  const v = norma(val)
  if (!v) return null
  if (campo === 'fp' || /^FP=1/.test(v)) {
    return { expr: 'FP=1', id: ID_FP_1, papel: 'lei' }
  }
  if (campo === 'regua' && /^n=\d+$/.test(v)) {
    return { expr: 'n', id: '', papel: 'parametro', val: v }
  }
  if (campo === 'revela') {
    return { expr: 'a: 0 → 1', id: ID_REVELA, papel: 'lei', val: v }
  }
  if (campo === 'dual' && /smooth-min/.test(v)) {
    return { expr: 'smooth-min', id: ID_DUAL_SMIN, papel: 'lei' }
  }
  if (campo === 'pbr' && /os 6 efeitos/.test(v)) {
    return { expr: EXPR_TRIADE, id: ID_TRIADE_FECHA, papel: 'braco', val: 'PBR' }
  }
  if (campo === 'formula' && v === FASE_AURA) {
    return { expr: FASE_AURA, id: ID_FASE_AURA, papel: 'lei' }
  }
  if ((campo === 'desc' || campo === 'formula') && v.includes('A tríade fecha')) {
    return { expr: EXPR_TRIADE, id: ID_TRIADE_FECHA, papel: 'lei' }
  }
  if (/û=p\/\|p\|/.test(v)) {
    return { expr: 'û=p/|p|', id: ID_TRIADE_FECHA, papel: 'braco' }
  }
  if (/φ²\s*=\s*φ\s*\+\s*1/.test(v) || /phi\^2\s*=\s*phi\s*\+\s*1/i.test(v)) {
    return { expr: EXPR_PHI2, id: 'phi2', papel: 'observada' }
  }
  if (/σ\s*[·⊗]\s*σ['′]\s*=\s*[−\-]?1/.test(v) || /σ·σ'\s*=\s*−1/.test(v) || /σ·σ′=−1/.test(v)) {
    return { expr: EXPR_SIGMA, id: 'sigma-dual', papel: 'observada' }
  }
  if (/z\s*↦\s*z²\s*\+\s*c/.test(v) || /z↦z²\+c/.test(v)) {
    return { expr: EXPR_JULIA, id: 'julia-c', papel: 'observada' }
  }
  return null
}

export function hitsDaDecl (decl) {
  const d = decl || {}
  const hits = []
  const push = (h) => { if (h) hits.push(h) }
  push(normalizaExpr('fp', d.fp))
  push(normalizaExpr('regua', d.regua))
  push(normalizaExpr('revela', d.revela))
  push(normalizaExpr('pbr', d.pbr))
  push(normalizaExpr('dual', d.dual))
  push(normalizaExpr('formula', d.formula))
  push(normalizaExpr('desc', d.desc))
  const blob = blobDecl(d)
  if (!hits.some((h) => h.papel === 'observada' && h.id === 'phi2')) {
    push(normalizaExpr('op', d.op))
  }
  if (!hits.some((h) => h.id === 'phi2') && /φ²\s*=\s*φ\s*\+\s*1/.test(blob)) {
    hits.push({ expr: EXPR_PHI2, id: 'phi2', papel: 'observada' })
  }
  if (!hits.some((h) => h.id === 'sigma-dual') && /σ·σ['′]\s*=\s*[−\-]?1/.test(blob)) {
    hits.push({ expr: EXPR_SIGMA, id: 'sigma-dual', papel: 'observada' })
  }
  if (!hits.some((h) => h.id === 'julia-c') && /z\s*↦\s*z²/.test(blob)) {
    hits.push({ expr: EXPR_JULIA, id: 'julia-c', papel: 'observada' })
  }
  return hits
}

/**
 * Segunda varredura: declaração → normalização → agrupamento → testemunhas.
 * mesma expressão ⇒ mesma candidata. ≥12 ⇒ lei conhecida. Não infere realizar.
 */
export function varreduraLegendas (man) {
  const decls = declaracoesDoMan(man)
  const grupos = new Map()
  for (const d of decls) {
    const chave = d.secao + '/' + d.nome
    for (const h of hitsDaDecl(d)) {
      const k = h.papel + '\t' + (h.id || h.expr)
      if (!grupos.has(k)) grupos.set(k, { ...h, n: 0, cards: [] })
      const g = grupos.get(k)
      if (!g.cards.includes(chave)) {
        g.n++
        g.cards.push(chave)
      }
    }
  }
  const lista = [...grupos.values()]
  const conhecidas = lista.filter((g) => g.papel === 'lei' && g.n >= LIMIAR_TESTEMUNHAS)
  const observadas = lista.filter((g) => g.papel === 'observada')
  const parametros = lista.filter((g) => g.papel === 'parametro')
  const bracos = lista.filter((g) => g.papel === 'braco')
  const candidatas = lista.filter((g) => g.papel === 'lei' && g.n >= LIMIAR_TESTEMUNHAS)
  return {
    nCards: decls.length,
    nOps: new Set(decls.map((d) => d.op).filter(Boolean)).size,
    conhecidas,
    observadas,
    parametros,
    bracos,
    candidatas,
    nConhecidas: conhecidas.length,
    nObservadas: observadas.length,
    realiza: false,
    cena: false,
  }
}

/** Cards do original cuja legenda testemunha a lei. */
export function originalDeclara (man, pecas, lei) {
  const cards = catalogoCards(man, pecas)
  const decls = declaracoesDoMan(man)
  const by = new Map(decls.map((d) => [d.secao + '/' + d.nome, d]))
  return cards.filter((c) => declaraLei(by.get(c.secao + '/' + c.nome) || {}, lei)).map((c) => {
    const d = by.get(c.secao + '/' + c.nome) || {}
    return {
      id: c.id,
      nome: c.nome,
      secao: c.secao,
      kernel: c.kernel,
      op: c.op,
      regua: d.regua || '',
      formula: d.formula || faseNaDesc(c.desc),
    }
  })
}

export function montaLei (canon, evidencias) {
  const ev = evidencias || []
  const quer = (canon && canon.instrumento) || ''
  const instrumentos = [...new Set(ev.map((e) => e.kernel).filter(Boolean))].sort()
  const inst = quer || (instrumentos.length === 1 ? instrumentos[0] : '')
  return {
    id: (canon && canon.id) || '',
    nome: (canon && canon.nome) || '',
    expressao: (canon && canon.expressao) || '',
    parametros: ((canon && canon.parametros) || []).slice(),
    dominio: ((canon && canon.dominio) || []).slice(),
    instrumento: inst,
    instrumentos,
    evidencias: ev.map((e) => e.id),
    nEvidencias: ev.length,
    testemunhas: ev.length,
    papel: 'lei',
    ciclo: CICLO,
    statusInterno: 'conhecida',
    realiza: false,
    cena: false,
    mesmoInstrumento: quer ? ev.length > 0 && ev.every((e) => e.kernel === quer) : instrumentos.length <= 1,
    refs: (canon && canon.refs) || refsDe([], [], ''),
  }
}

/** original declara a lei ≡ banco reconhece a mesma lei. Sem realizar. */
export function reconheceLei (man, pecas, canon) {
  const lei = canon || LEI_FASE_AURA
  const orig = originalDeclara(man, pecas, lei)
  const banco = montaLei(lei, orig)
  const idsOrig = orig.map((e) => e.id).join(' ')
  const idsBanco = banco.evidencias.join(' ')
  const instOk = !lei.instrumento || (banco.instrumento === lei.instrumento && banco.mesmoInstrumento)
  return {
    ok: idsOrig === idsBanco &&
      banco.expressao === lei.expressao &&
      instOk &&
      banco.nEvidencias >= LIMIAR_TESTEMUNHAS &&
      banco.realiza === false &&
      banco.cena === false &&
      banco.ciclo === 'conhecer',
    orig,
    banco,
  }
}

export function catalogoLeis (man, pecas) {
  return LEIS_CANON.map((lei) => reconheceLei(man, pecas, lei)).filter((r) => r.ok).map((r) => r.banco)
}

/** Eixo interno (conhecida) ⊥ cruzamento externo. Cruzamento não implica realizar. */
export function eixosDaLei (lei) {
  const refs = (lei && lei.refs) || {}
  return Object.freeze({
    interno: Object.freeze({
      expressao: (lei && lei.expressao) || '',
      nEvidencias: (lei && lei.nEvidencias) || 0,
      instrumento: (lei && lei.instrumento) || '',
      status: (lei && lei.statusInterno) || '',
    }),
    cruzamento: Object.freeze({
      cruz: refs.cruz || '',
      fisica: Object.freeze((refs.fisica || []).slice()),
      catalogo: Object.freeze((refs.catalogo || []).slice()),
    }),
    realiza: false,
    cena: false,
  })
}

/**
 * Lei nova: mesma expressão → testemunhas → limiar → cruzamento.
 * Admissão ao catálogo ≠ realização no paper ≠ cena. F8 fechado.
 */
export function admiteLei (cand) {
  if (typeof cand === 'string') cand = { expressao: cand, nEvidencias: 99 }
  const n = Number((cand && (cand.nEvidencias != null ? cand.nEvidencias : cand.n)) || 0)
  const expr = norma((cand && (cand.expressao || cand.expr || cand.formula)) || '')
  const id = String((cand && cand.id) || '')
  const cruz = (cand && ((cand.refs && cand.refs.cruz) || cand.cruz)) || ''
  const papel = (cand && cand.papel) || ''
  const reservada = expressaoNaoELei(expr) || expressaoNaoELei(id) || (papel !== '' && papel !== 'lei')
  const conhecida = !reservada && !!expr && n >= LIMIAR_TESTEMUNHAS
  return Object.freeze({
    funil: FUNIL,
    congelado: CONGELADO,
    statusInterno: conhecida ? 'conhecida' : (expr ? 'observada' : ''),
    cruz: CRUZ.includes(cruz) ? cruz : '',
    ok: conhecida,
    realiza: false,
    cena: false,
    f8: false,
  })
}

export function igualLei (a, b) {
  if (!a || !b) return a === b
  return a.id === b.id && a.expressao === b.expressao && a.instrumento === b.instrumento &&
    a.nEvidencias === b.nEvidencias && a.evidencias.join(' ') === b.evidencias.join(' ')
}

export function igualCatalogoLeis (a, b) {
  if (!a || !b || a.length !== b.length) return false
  return a.every((l, i) => igualLei(l, b[i]))
}

export function selecionaLei (storage, id) {
  const s = String(id || '')
  if (storage && s) {
    try { storage.setItem(CHAVE_LEI, JSON.stringify({ id: s })) } catch { /* quota */ }
  }
  return s
}

export function leLeiSelecionada (storage) {
  if (!storage) return ''
  try {
    const o = JSON.parse(storage.getItem(CHAVE_LEI) || 'null')
    return (o && o.id) || ''
  } catch {
    return ''
  }
}

/** Só conhece. realizar/cena recusados neste ciclo. */
export function disparaLei (storage, gatilho, opts = {}) {
  const id = typeof gatilho === 'string'
    ? gatilho
    : (gatilho && gatilho.id) || ID_FASE_AURA
  if (id) selecionaLei(storage, id)
  return {
    id,
    ciclo: CICLO,
    conheceu: true,
    realizou: false,
    cena: false,
    recusou: !!(opts && (opts.realizar || opts.cena || opts.executar)),
  }
}

export function leiParaU (cat) {
  const lista = Array.isArray(cat) ? cat : []
  const nEv = lista.reduce((a, l) => a + (l.nEvidencias || 0), 0)
  return completa({
    kind: 'pagina',
    id: ID_LEI_GK,
    sentido: 0,
    formato: 'json',
    camada: 'descricao',
    estatuto: 'realizado',
    evidencia: 'legendas do manifesto; limiar 12; fisica.tex+catalogo.tex por ref; cards=evidencias; tests/lei_gk_u.js',
    proibicao: 'CONGELADO; funil expressao>classificacao>papel>cruzamento; formula != lei; estrutura != lei; construcao != lei; teorema != tipo; teorema != objecto; mesmo enunciado notRightarrow mesmo objecto; desconhecido != ausente != novo tipo; desconhecido estrutural != algebrica; localizado e incompativel => desconhecido; identidade=expressao>suporte>realizacao; mesma expressao notRightarrow mesmo objecto; mesmo suporte notRightarrow mesma realizacao; alojamento = reconhecimento; subestrutura reconhecida != novo objecto; elemento/subestrutura subset construcao != novo objecto; propriedade interna notRightarrow entrada em ONTOLOGIA; localizado em X notRightarrow pertence a vizdobra; suporte != pertencimento a construcao; vive em X notRightarrow gerado por vizdobra; ontologia fechada por contencao; descoberta notRightarrow expande ONTOLOGIA; observado != construcao; V != X; objecto em V pede casa de V; xor != conv; C_f != *; C_f != hat != H; hat != chi != paridade; fator nao traduz para B; 2 != 0 em V/Zc nao autoriza 2mapsto0 em B; simbolo igual notRightarrow operacao igual; 2_V != 2_B; injetivo != invertivel; outra leitura do mesmo suporte nao cria casa; xor != * != F(f)F(g); oficio != categoria; parte nao acrescenta objectos; V_E != V; tres geometrias != tres objectos; d_ultra != d_Hamming != d_percurso; mesmo suporte notRightarrow mesma expressao; expressoes distintas notRightarrow objectos ontologicos distintos; propriedade da leitura != realizacao da expressao; B_cal != B; |N_T(x)|=1 != 2n; I != T; percurso != grafo; Euler != gerador; ciclo != lei; handshake != Euler; incidencia != grafo; grade != hipercubo; 2n != m; sigma_grade != sigma_B; n pares != 2n soltos; ordem 4 != ordem 2; binom(n,2) != 2n; codificar != linearizar; Peano != serpentina; I != V_grade; indice != conteudo; IxI != I; palavra != espaco; T_letra != T; alfabeto != A; comuta != cancela; G>1 != gerador; ker w != nucleo; pi != rho; buraco != dobra; folga != Duo; mesma folga != dois objectos; bijeccao != Iso; bijeccao != vizinhanca; posicional != vizinhanca; varphi != s; compor != acumular; d_x != D; T_trav != T; preco em pi != X; unica != duomorfa; Iso != Duo; D_op != D; interface != categoria; hexal != Duo; ligar != fundir; interface != transporte; T_fio != T; Q_ab != operadores de M; G=1 != Iso; eixo a != eixo G; duas especies != tres; igualdade estrutural != Iso; duas dobras != terceiro estado; tabua != objecto; compor = somar a; Q_sigma != Q_dt; quatro quadrantes != quatro objectos; centro != objecto; dois lados != duas casas; duas faces != dois objectos; equacao != nova lei; Star(U) != x^2=x+1; Star != Q_ab; Star != F8; Star != novo gerador; realizar em Zc != promover em B; reversivel != nova realizacao; Duo != Star; Duo reversivel != Iso; nao cria espaco; Q_ab != R_ij; quarteto != rotor; quatro estados != quatro geradores; leitura != cadeia; relacao nao promove; e_k != lei_gk != gerador; dim X != lei; Gram != lei; coord-byte != lei; lei != gerador != realizacao; cruzamento notRightarrow realizar notRightarrow cena; 3 dobras != 8 leis; afim=realizacao 3a dobra; vinco=2m=a+b != unidade != estrela; sigma_i != sigma-dual; realizacao:vinco lei_gk=N/A; dtc != rotor != entrada-catalogo != corpo-em; R_ij != gera != realiza != fixa != le; fecho != gerador; peso != reua do card; paridade != chi != dual-smin; nao substitui Pontryagin, nao associador, nao 6a lei; op != regua != tag != formula; Reino nao herda fisica.tex; paper nao declara realizado o relacionado; n=k nao e lei; conhecer != realizar != cena; nao F8; nao pintar',
    nota: 'nLeis=' + lista.length + '; nEvidencias=' + nEv +
      '; ciclo=' + CICLO + '; pipe=' + PIPE.join('>') +
      '; funil=' + FUNIL.join('>') +
      '; funilObj=' + FUNIL_OBJECTO.join('>') +
      '; funilTeorema=' + FUNIL_TEOREMA.join('>') +
      '; funilBusca=' + FUNIL_BUSCA.join('>') +
      '; identidade=' + CAMPOS_IDENTIDADE.join('>') +
      '; funilDestino=' + FUNIL_DESTINO.join('>') +
      '; alojar=' + CAMPOS_ALOJAR.join('>') +
      '; funilCasa=' + FUNIL_CASA.join('>') +
      '; regraOnto=' + REGRA_ONTO +
      '; regraV=' + REGRA_V +
      '; regraEnun=' + REGRA_ENUN +
      '; regraFator=' + REGRA_FATOR +
      '; regraDois=' + REGRA_DOIS +
      '; regraSim=' + REGRA_SIM +
      '; regra2VB=' + REGRA_2VB +
      '; regraCasaLe=' + REGRA_CASA_LE +
      '; regraInj=' + REGRA_INJ +
      '; regraFg=' + REGRA_FG +
      '; regraOficio=' + REGRA_OFICIO +
      '; regraParte=' + REGRA_PARTE +
      '; regraVe=' + REGRA_VE +
      '; regraTresGeo=' + REGRA_TRESGEO +
      '; regraD=' + REGRA_D +
      '; regraSupEx=' + REGRA_SUP_EX +
      '; regraExObj=' + REGRA_EX_OBJ +
      '; regraPropEx=' + REGRA_PROP_EX +
      '; regraBB=' + REGRA_BB +
      '; regraNt=' + REGRA_NT +
      '; regraIt=' + REGRA_IT +
      '; regraPg=' + REGRA_PG +
      '; regraEuler=' + REGRA_EULER +
      '; regraCiclo=' + REGRA_CICLO +
      '; regraHs=' + REGRA_HS +
      '; regraInc=' + REGRA_INC +
      '; regraGh=' + REGRA_GH +
      '; regraPm=' + REGRA_PM +
      '; regraSg=' + REGRA_SG +
      '; regraPares=' + REGRA_PARES +
      '; regraOrd=' + REGRA_ORD +
      '; regraPlanos=' + REGRA_PLANOS +
      '; regraCl=' + REGRA_CL +
      '; regraPs=' + REGRA_PS +
      '; regraIvg=' + REGRA_IVG +
      '; regraIc=' + REGRA_IC +
      '; regraIxi=' + REGRA_IXI +
      '; regraPal=' + REGRA_PAL +
      '; regraTt=' + REGRA_TT +
      '; regraAlf=' + REGRA_ALF +
      '; regraCc=' + REGRA_CC +
      '; regraG1=' + REGRA_G1 +
      '; regraKer=' + REGRA_KER +
      '; regraPr=' + REGRA_PR +
      '; regraBur=' + REGRA_BUR +
      '; regraFol=' + REGRA_FOL +
      '; regraMesma=' + REGRA_MESMA +
      '; regraBij=' + REGRA_BIJ +
      '; regraBv=' + REGRA_BV +
      '; regraPos=' + REGRA_POS +
      '; regraVs=' + REGRA_VS +
      '; regraAcc=' + REGRA_ACC +
      '; regraDx=' + REGRA_DX +
      '; regraTr=' + REGRA_TR +
      '; regraPx=' + REGRA_PX +
      '; regraUd=' + REGRA_UD +
      '; regraBid=' + REGRA_BID +
      '; regraDop=' + REGRA_DOP +
      '; regraIf=' + REGRA_IF +
      '; regraHx=' + REGRA_HX +
      '; regraLf=' + REGRA_LF +
      '; regraItr=' + REGRA_ITR +
      '; regraTf=' + REGRA_TF +
      '; regraQm=' + REGRA_QM +
      '; regraGi=' + REGRA_GI +
      '; regraAx=' + REGRA_AX +
      '; regra2e=' + REGRA_2E +
      '; regraEq=' + REGRA_EQ +
      '; regra3s=' + REGRA_3S +
      '; regraTab=' + REGRA_TAB +
      '; regraSoma=' + REGRA_SOMA +
      '; regraQab2=' + REGRA_QAB2 +
      '; regra4q=' + REGRA_4Q +
      '; regraCtr=' + REGRA_CTR +
      '; regraLd=' + REGRA_LD +
      '; regra2f=' + REGRA_2F +
      '; regraEqn=' + REGRA_EQN +
      '; regraStu=' + REGRA_STU +
      '; regraStq=' + REGRA_STQ +
      '; regraStf8=' + REGRA_STF8 +
      '; regraStg=' + REGRA_STG +
      '; regraZcb=' + REGRA_ZCB +
      '; regraRev=' + REGRA_REV +
      '; regraDs=' + REGRA_DS +
      '; regraRiso=' + REGRA_RISO +
      '; alojarE=' + ALOJAR_E +
      '; regraSub=' + REGRA_SUB +
      '; regraGrau=' + REGRA_GRAU +
      '; regraProp=' + REGRA_PROP +
      '; regraSuporte=' + REGRA_SUPORTE +
      '; regraVive=' + REGRA_VIVE +
      '; papeis=' + PAPEIS.join('>') +
      '; ontologia=' + ONTOLOGIA.join('>') +
      '; cadeia=' + CADEIA.join('>') +
      '; estrutura=vizdobra>X' +
      '; dim=' + DIM_X + '; dobras=' + DOBRAS_GERADORAS.length +
      '; limiar=' + LIMIAR_TESTEMUNHAS + '; ids=' + lista.map((l) => l.id).join(','),
    slots: {
      conhecer: 'realizado',
      realizar: 'N/A',
      cena: 'N/A',
      glsl: 'N/A',
      execucao: 'N/A',
      espaco: 'realizado',
      geradores: 'realizado',
      grafo: 'realizado',
      leitura: 'realizado',
      afim: 'relacionado',
      vinco: 'relacionado',
    },
    filhos: lista.map((l) => completa({
      kind: 'ficheiro',
      id: l.id || 'lei',
      sentido: 0,
      formato: 'json',
      estatuto: 'realizado',
      fonte: l.instrumento,
      evidencia: 'nEvidencias=' + (l.nEvidencias || 0) + '; expressao=' + (l.expressao || '') +
        '; cruz=' + ((l.refs && l.refs.cruz) || '') +
        '; fisica=' + ((l.refs && l.refs.fisica) || []).join(','),
      proibicao: 'nao realizar; nao cena; card nao e dono da lei',
    })),
  })
}

export function uParaLei (u) {
  const nota = (u && u.nota) || ''
  const nl = nota.match(/nLeis=([0-9]+)/)
  const ne = nota.match(/nEvidencias=([0-9]+)/)
  return {
    nLeis: nl ? Number(nl[1]) : ((u && u.filhos) || []).length,
    nEvidencias: ne ? Number(ne[1]) : 0,
    ciclo: (nota.match(/ciclo=([a-z]+)/) || [])[1] || '',
    filhos: ((u && u.filhos) || []).map((f) => f.id),
  }
}

export { FASE_AURA, ROT_REGUA, faseNaDesc }
