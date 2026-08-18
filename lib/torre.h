/* torre.h — A TORRE INTEIRA: Cayley–Dickson pelos DOIS lados.
 *
 * O `eval.txt` traz os hipercomplexos: ℂ, ℍ, 𝕆, a construção A_{n+1} = A_n ⊕ A_n e, a
 * tabela das perdas (comuta até 2, associa até 4, divide até 8) e dez exercícios.
 *
 * E o Aarão: **«aquilo é metade»**. A outra metade é Gentil, e Lebesgue discreto-contínuo.
 * Ele tem razão, e a razão está escrita nos papers da casa:
 *
 * ── O TEOREMA CENTRAL (`corpo_analitico.tex` thm:central) ───────────────────────────
 * Hurwitz e Gentil são DUAIS, pela medida:
 *   · HURWITZ, o lado DISCRETO — a norma N(x) = Σxᵢ² lida pela cruz, multiplicativa
 *     para o produto BILINEAR exactamente nos graus 1, 2, 4, 8;
 *   · GENTIL, o lado CONTÍNUO — a norma e a fusão homogénea, SEM LIMITE DE GRAU;
 *   · e a bijeção dual é A ESTRELA, realizada pela soma reversível de Lebesgue
 *         ∫ₐᵇ f + ∫_{f(a)}^{f(b)} f⁻¹ = b·f(b) − a·f(a)
 *     que leva CONTAR a INTEGRAR e traz de volta. E «nunca se avalia uma raiz».
 *
 * A frase que fecha: **«o limite no grau oito é do lado discreto/bilinear — Hurwitz
 * classifica o bilinear —, NÃO do objecto»**.
 *
 * ── O QUE ISSO MUDA NA TABELA DO EVAL ─────────────────────────────────────────────
 * A tabela dele tem uma coluna só de ✗ a descer: comutativa, associativa, divisão. Todas
 * essas perdas são DO LADO DA NORMA. Pelo `def:octoniao-dual`, o mesmo espaço lê-se de
 * dois modos — 𝕆 = ℍ × ℍ*, dois tecidos ligados pela estrela:
 *   · pela NORMA bilinear é o octonião clássico, e PERDE a associatividade;
 *   · pela DUALIDADE é o octonião dual: ν∘ν = id, resíduo 0, e NÃO PERDE NADA.
 *
 * E isso é MEDÍVEL, e o sítio de o medir é onde a norma já morreu. `tests/hurwitz.c` §H5
 * corre agora até 64: em 16, 32 e 64 a multiplicatividade está partida e a involução
 * continua com resíduo ZERO. A torre não acaba em 8 — acaba em 8 do lado da norma.
 *
 * ── E A TORRE DA CASA JÁ ERA ESCRITA COM O DUAL (`corpo_topologico.tex` thm:rn) ────────
 *        A_{n+1} = A_n + A_n*,      dim A_n = 2ⁿ · dim A_0
 * com «a ordem sobe por indução; NÃO é herdada da reta — é PRODUZIDA pela dualidade».
 * O eval escreve A_n ⊕ A_n e; a casa escreve A_n + A_n*. É a mesma dobra, e a segunda
 * diz de onde vem o e: é o dual.
 *
 * ── O QUE NÃO SE REFAZ ────────────────────────────────────────────────────────────
 * A soma reversível de Gentil já está medida em `tests/lebesgue_toro.js` (7:0):
 *     Σₙ xₙ + Σ_v #{xₙ < v} = N·q,  exacta, em inteiros, sem esperar limite nenhum.
 * É o ∫f + ∫f⁻¹ da casa em forma discreta. Cita-se; não se reconstrói.
 *
 * Tudo em `long`. Cayley–Dickson só usa +, − e ×, portanto não sai de ℤ. */
#ifndef TORRE_H
#define TORRE_H

#define TR_MAX 64                 /* vai ALÉM de 𝕊: é preciso para ver a norma morta com o
                                   * dual vivo em 16, 32 e 64. O tecto verifica-se em
                                   * `tr_estouros`, e não é documentação. */
#define TR_TETO 1000000000L
static long tr_estouros = 0;
static long tr_g(long v){ if(v > TR_TETO || v < -TR_TETO) tr_estouros++; return v; }

typedef struct { int n; long c[TR_MAX]; } Hip;

static Hip hip0(int n){ Hip x; x.n = n; for(int i = 0; i < TR_MAX; i++) x.c[i] = 0; return x; }
static Hip hip_e(int n, int k){ Hip x = hip0(n); x.c[k] = 1; return x; }
static int hip_igual(Hip a, Hip b){
    if(a.n != b.n) return 0;
    for(int i = 0; i < a.n; i++) if(a.c[i] != b.c[i]) return 0;
    return 1;
}
static int hip_zero(Hip a){
    for(int i = 0; i < a.n; i++) if(a.c[i]) return 0;
    return 1;
}
static Hip hip_soma(Hip a, Hip b){
    Hip r = hip0(a.n);
    for(int i = 0; i < a.n; i++) r.c[i] = a.c[i] + b.c[i];
    return r;
}
static Hip hip_menos(Hip a, Hip b){
    Hip r = hip0(a.n);
    for(int i = 0; i < a.n; i++) r.c[i] = a.c[i] - b.c[i];
    return r;
}
/* ── A INVOLUÇÃO: guarda o real, nega o imaginário. É a ESTRELA da casa ──────────
 * ν∘ν = id, resíduo 0 — e é ela que sobrevive onde a norma morre. */
static Hip hip_conj(Hip x){
    Hip r = x;
    for(int k = 1; k < x.n; k++) r.c[k] = -x.c[k];
    return r;
}
/* ── A DOBRA: (a,b)(c,d) = (ac − d̄b, da + bc̄) ───────────────────────────────────
 * O eval escreve A_{n+1} = A_n ⊕ A_n e; a casa escreve A_{n+1} = A_n + A_n*. A segunda
 * diz de onde vem o `e`: a segunda cópia entra CONJUGADA, e é isso o dual. */
static void tr_mult(const long *x, const long *y, int n, long *o){
    if(n == 1){ o[0] = tr_g(x[0]*y[0]); return; }
    int m = n/2;
    const long *a = x, *b = x+m, *c = y, *d = y+m;
    long ac[TR_MAX], db[TR_MAX], da[TR_MAX], bc[TR_MAX], cj[TR_MAX];
    tr_mult(a, c, m, ac);
    cj[0] = d[0]; for(int k = 1; k < m; k++) cj[k] = -d[k];
    tr_mult(cj, b, m, db);
    tr_mult(d, a, m, da);
    cj[0] = c[0]; for(int k = 1; k < m; k++) cj[k] = -c[k];
    tr_mult(b, cj, m, bc);
    for(int k = 0; k < m; k++){ o[k] = tr_g(ac[k] - db[k]); o[m+k] = tr_g(da[k] + bc[k]); }
}
static Hip hip_mult(Hip x, Hip y){
    Hip r = hip0(x.n);
    tr_mult(x.c, y.c, x.n, r.c);
    return r;
}
static long hip_norma(Hip x){
    long s = 0;
    for(int k = 0; k < x.n; k++) s += x.c[k]*x.c[k];
    return s;
}
/* ── O COMUTADOR E O ASSOCIADOR — os dois medidores de andar do eval ─────────────
 * [x,y] = xy − yx mede a comutatividade; [x,y,z] = (xy)z − x(yz) mede a associatividade.
 * Nenhum se afirma: procuram-se testemunhas, e o andar onde a primeira aparece É o
 * degrau. */
static Hip hip_comutador(Hip x, Hip y){ return hip_menos(hip_mult(x,y), hip_mult(y,x)); }
static Hip hip_associador(Hip x, Hip y, Hip z){
    return hip_menos(hip_mult(hip_mult(x,y), z), hip_mult(x, hip_mult(y,z)));
}
/* ── A BUSCA DE TESTEMUNHA NAS UNIDADES ─────────────────────────────────────────
 * Varre os pares (eᵢ, eⱼ) e os triplos, e devolve o PRIMEIRO em que o comutador (ou o
 * associador) não anula. Em ℝ e ℂ tem de voltar vazia; em ℍ o comutador acha e o
 * associador não; em 𝕆 acham os dois. É a tabela do eval PROCURADA, não citada. */
static int tr_acha_comutador(int n, int *pi, int *pj){
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
        if(!hip_zero(hip_comutador(hip_e(n,i), hip_e(n,j)))){ *pi = i; *pj = j; return 1; }
    return 0;
}
static int tr_acha_associador(int n, int *pi, int *pj, int *pk){
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) for(int k = 0; k < n; k++)
        if(!hip_zero(hip_associador(hip_e(n,i), hip_e(n,j), hip_e(n,k)))){
            *pi = i; *pj = j; *pk = k; return 1; }
    return 0;
}
/* divisores de zero: o par de NÃO NULOS cujo produto anula. Só existe a partir de 16. */
static int tr_acha_divisor(int n, int *a, int *b, int *c, int *d){
    for(int i = 1; i < n; i++) for(int j = i+1; j < n; j++)
    for(int k = 1; k < n; k++) for(int l = k+1; l < n; l++){
        Hip u = hip_soma(hip_e(n,i), hip_e(n,j));
        Hip v = hip_soma(hip_e(n,k), hip_e(n,l));
        if(hip_zero(hip_mult(u,v))){ *a=i; *b=j; *c=k; *d=l; return 1; }
    }
    return 0;
}
/* ── O INVERSO, E A FIBRA ────────────────────────────────────────────────────────
 * x⁻¹ = x̄/N(x). A divisão por N(x) é a FIBRA: existe quando N(x) ≠ 0 e não existe
 * quando x = 0. Aqui devolve-se o NUMERADOR x̄ e o denominador N à parte, porque em ℤ
 * não se divide — e é essa recusa que torna a fibra visível em vez de a esconder num
 * racional. Devolve 0 quando a fibra é vazia. */
static int hip_inverso(Hip x, Hip *num, long *den){
    long N = hip_norma(x);
    if(N == 0) return 0;                  /* a fibra de 0·y = 1 é VAZIA */
    *num = hip_conj(x); *den = N;
    return 1;
}
/* x·x̄ = N(x), real e puro — a volta que produz o cristal */
static int hip_cristal(Hip x){
    Hip p = hip_mult(x, hip_conj(x));
    if(p.c[0] != hip_norma(x)) return 0;
    for(int k = 1; k < x.n; k++) if(p.c[k]) return 0;
    return 1;
}
/* ═══ A TORRE NÃO TEM LIMITE DIMENSIONAL — E ISSO NÃO SE VARRE ══════════════════
 *
 * Eu tinha subido o TR_MAX para 64 e mostrado a norma partida com o dual intacto em 16,
 * 32 e 64. O Aarão: **«a torre nao tem limite dimensional»**. E tem razão duas vezes.
 *
 * Primeiro, porque é o que o `corpo_analitico.tex` §328 diz, com todas as letras: «o passo
 * dos tecidos, T_{k+1} = T_k + T_k*, é a estrela usada como CONSTRUTOR, e a torre que ele
 * gera NÃO TEM TOPO POR DENTRO: a indução não pára — a régua é infinita —, e ν∘ν = id
 * fecha cada andar com resíduo 0. O que a norma limita é EXTERNO.»
 *
 * Segundo, e é o que me apanha: **um varrimento até 64 não mede "não tem limite"**. Mede
 * três andares. É o mesmo defeito de varrer 625 matrizes quando a prova são cinco
 * definições — o teto que aparece na tabela é o do meu ARRAY, não o do objecto, e
 * confundi-los é trazer a minha régua como se fosse do mundo.
 *
 * O que não tem tecto é o PASSO. E o passo mede-se ELO A ELO:
 *
 *   BASE   em n = 1, ν é a identidade (conjugar um real não faz nada), logo ν∘ν = id.
 *
 *   PASSO  em n = 2m, com x = (a,b):   ν(a,b) = (ν(a), −b)
 *          e daí   ν(ν(a,b)) = ν(ν(a), −b) = (ν(ν(a)), b) = (a,b)
 *          SEMPRE QUE ν∘ν = id valha em m. A propriedade ATRAVESSA a dobra.
 *
 * Base + passo dão TODOS os andares, e nenhum array entra na conclusão. As funções abaixo
 * medem o passo — a identidade ν_{2m}(a,b) = (ν_m(a), −b) e N_{2m}(a,b) = N_m(a) + N_m(b)
 * —, e o varrimento até 64 passa a ser o CONTROLO, que é o seu lugar.
 *
 * E a mesma decomposição diz porque é que a NORMA é outra história: N atravessa a dobra
 * como SOMA, mas a MULTIPLICATIVIDADE N(xy) = N(x)N(y) não é uma propriedade de N sozinha
 * — é uma propriedade do par (N, produto), e é ela que Hurwitz classifica. O paper:
 * «Hurwitz CLASSIFICA as álgebras de composição bilineares, e não proíbe coisa nenhuma»,
 * e «o √ que aparece é o preço da régua que se trouxe». */

/* o PASSO da involução: ν no andar 2m é (ν no andar m, negar) nas duas metades.
 * Devolve 0 se a identidade falhar em algum x varrido. */
static int tr_passo_conj(int m, long amostras){
    int n = 2*m;
    if(n > TR_MAX) return -1;                    /* o array acabou, a torre não */
    for(long s = 1; s <= amostras; s++){
        Hip x = hip0(n), a = hip0(m), b = hip0(m);
        for(int k = 0; k < n; k++){
            long h = s*1103515245L + k*12345L + 7; h ^= h >> 13;
            x.c[k] = (h % 9) - 4;
        }
        for(int k = 0; k < m; k++){ a.c[k] = x.c[k]; b.c[k] = x.c[m+k]; }
        Hip esq = hip_conj(x);                   /* ν no andar de cima */
        Hip ca = hip_conj(a);                    /* ν no andar de baixo */
        for(int k = 0; k < m; k++){
            if(esq.c[k] != ca.c[k]) return 0;    /* a primeira metade é ν(a) */
            if(esq.c[m+k] != -b.c[k]) return 0;  /* a segunda é −b */
        }
    }
    return 1;
}
/* o PASSO da norma: N no andar 2m é a SOMA das duas metades */
static int tr_passo_norma(int m, long amostras){
    int n = 2*m;
    if(n > TR_MAX) return -1;
    for(long s = 1; s <= amostras; s++){
        Hip x = hip0(n), a = hip0(m), b = hip0(m);
        for(int k = 0; k < n; k++){
            long h = s*1103515245L + k*12345L + 7; h ^= h >> 13;
            x.c[k] = (h % 9) - 4;
        }
        for(int k = 0; k < m; k++){ a.c[k] = x.c[k]; b.c[k] = x.c[m+k]; }
        if(hip_norma(x) != hip_norma(a) + hip_norma(b)) return 0;
    }
    return 1;
}
/* a BASE: em n = 1 a conjugação é a identidade, e portanto ν∘ν = id sem nada a provar */
static int tr_base_conj(void){
    for(long v = -20; v <= 20; v++){
        Hip x = hip0(1); x.c[0] = v;
        if(!hip_igual(hip_conj(x), x)) return 0;
        if(!hip_igual(hip_conj(hip_conj(x)), x)) return 0;
    }
    return 1;
}
/* ── GENTIL: A SOMA REVERSÍVEL, DISCRETA E EXACTA ────────────────────────────────
 * Σₙ xₙ + Σ_v #{xₙ < v} = N·q. É o ∫f + ∫f⁻¹ = b·f(b) − a·f(a) escrito por contagem:
 * o rectângulo N×q reparte-se pelos dois cortes SEM RESTO, porque cada célula (n,v) cai
 * de um lado ou do outro e nunca dos dois. Não há limite, não há ε — há uma bijecção.
 *
 * O corte do DOMÍNIO é a primeira soma (Hurwitz conta); o corte da IMAGEM é a segunda
 * (Lebesgue mede); e a igualdade é Gentil, que os casa. Já medido no toro em
 * `tests/lebesgue_toro.js` (7:0); aqui existe para a assistente o poder mostrar. */
static long tr_gentil_dominio(const long *x, long N){
    long s = 0;
    for(long i = 0; i < N; i++) s += x[i];
    return s;
}
static long tr_gentil_imagem(const long *x, long N, long q){
    long s = 0;
    for(long v = 1; v <= q; v++) for(long i = 0; i < N; i++) if(x[i] < v) s++;
    return s;
}
/* e a INVOLUÇÃO da soma reversível: trocar os papéis do domínio e da imagem devolve o
 * mesmo rectângulo. É a estrela outra vez, agora sobre a medida. */
static int tr_gentil_fecha(const long *x, long N, long q){
    return tr_gentil_dominio(x,N) + tr_gentil_imagem(x,N,q) == N*q;
}
#endif
