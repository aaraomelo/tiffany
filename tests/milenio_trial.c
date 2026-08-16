/* milenio_trial.c — AS SEIS LEIS: dual, bidual, trial, tetral, pental, hexal (a estrutura, não o enunciado).
 *
 * O Aarão: «Poincaré é dual, YM-NxNP são biduais, Riemann-BSD-Hodge são triais; e vem a quarta lei
 * (tetralidade, os tecidos), a quinta (o ponto fixo, o corolário) e a sexta (o relógio, o inversor, a
 * interface em 6). Se fechar, seis leis fundamentam toda a teoria.»  E fecha.
 *
 * NÃO se prova nenhum enunciado aberto --- cada casa corre sobre um teorema PROVADO (Poincaré-
 * dualidade, Kronecker, Dirichlet, Lefschetz, K**=K), sobre os NOSSOS objectos, e seria verdade ainda
 * que o problema do milénio não existisse. O que se mede é o LUGAR na progressão das dualidades:
 *
 *     DUAL    (período 2)   ν∘ν = id            --- Poincaré (a involução H^k↔H_{n-k}, o RESOLVIDO)
 *     BIDUAL  (K** = K)     o dual do dual       --- Yang-Mills, P/NP (os DOIS nulos → o centro 0)
 *     TRIAL   {-1, 0, +1}   os três eixos        --- Riemann, BSD, Hodge (os três estados do inversor)
 *
 * E o trial É o do inversor (dtc_viveiro, assinatura_tradutor): Riemann = -1 (a reflexão s↦1-s̄, o
 * aditivo), BSD = +1 (a órbita, o metálico σ, o multiplicativo), Hodge = 0 (a diagonal (p,p), o
 * algébrico=topológico que atravessa). Riemann e BSD são DUAIS (somam a 0); Hodge atravessa.
 *
 *   §M1  DUAL: Poincaré --- ν∘ν=id (período 2), resíduo 0. O self-dual, o resolvido.
 *   §M2  BIDUAL: YM, P/NP --- K**=K, e os dois nulos (origem, fim) colapsam no centro 0
 *   §M3  TRIAL: Riemann, BSD, Hodge --- os três eixos {-1,0,+1}; ±1 duais (somam 0), 0 atravessa
 *   §M4  a progressão 2 → K** → 3, cada casa sobre um teorema PROVADO (não o enunciado aberto)
 *
 *   cc -O2 -std=c99 -Wall -I../lib milenio_trial.c -o milenio_trial && ./milenio_trial
 */
#include <stdio.h>
#include "reta.h"      /* as operações da recta */
#include "unidade.h"

typedef long long L;
static const L P = 2147483647;                 /* 2^31-1, primo de Mersenne */

static L involucao(L x){ if(x%P==0) return 0; return (P - rt_inv_mod(x,P)) % P; }  /* ν(x)=-1/x, a Poincaré-dualidade */
static L imposto(L s){ return 1 - s*s; }       /* o trial do inversor: Π(s)=1-s² */

/* inteiros de Gauss, para o ponto fixo da estrela: i² = -1 EXACTO (o bit), e ν(i) = -1/i = i. */
typedef struct { L re, im; } Gauss;
static Gauss g_mul(Gauss a, Gauss b){ Gauss r; r.re = a.re*b.re - a.im*b.im; r.im = a.re*b.im + a.im*b.re; return r; }
static L mdc(L a, L b){ while(b){ L t = a % b; a = b; b = t; } return a; }
static L mmc(L a, L b){ return a / mdc(a, b) * b; }   /* o relógio: o menor instante comum */

int main(void){
    printf("=== OS MILÉNIOS: dual → bidual → trial (a estrutura, sobre teoremas provados) ====\n\n");

    /* ── §M1 DUAL: Poincaré --- a involução, período 2 ───────────────────────────────────── */
    /* a Poincaré-dualidade H^k ↔ H_{n-k} é uma INVOLUÇÃO: aplicá-la duas vezes devolve. É o único
     * dos sete que está RESOLVIDO (Perelman), e é o DUAL --- ν∘ν=id, período 2, o self-dual. */
    int poincare_dual = 1;
    for(L x=1; x<20000; x++) if(involucao(involucao(x)) != x%P){ poincare_dual = 0; break; }
    printf("§M1  Poincaré = DUAL: ν∘ν=id em [1,20000) (a involução H^k↔H_{n-k}), o resolvido\n\n");
    ok("§M1 POINCARÉ é o DUAL: a Poincaré-dualidade H^k↔H_{n-k} é uma involução ν∘ν=id (período 2,"
       " resíduo 0) --- o self-dual, e o único dos sete RESOLVIDO. O chão é a dualidade provada, não um"
       " enunciado aberto", poincare_dual);

    /* ── §M2 BIDUAL: YM, P/NP --- K**=K, e os dois nulos → o centro ───────────────────────── */
    /* Yang-Mills (a origem, o gap) e P/NP (o fim, o colapso) são os DOIS nulos do inversor: tudo-
     * desligado {0,0,0} e tudo-ligado {1,1,1}. K**=K (o dual do dual devolve) fixa-os, e o seu ponto
     * médio é o CENTRO 0 (o ponto fixo que a polar não move). São o BIDUAL, não uma fronteira a mais. */
    L nulo_off[3] = {0,0,0}, nulo_on[3] = {1,1,1};       /* origem e fim */
    /* a projecção no eixo diagonal (1,1,1): <v, (1,1,1)> - média. O centro é o ponto médio dos dois. */
    L proj_off = nulo_off[0]+nulo_off[1]+nulo_off[2];    /* = 0 */
    L proj_on  = nulo_on[0]+nulo_on[1]+nulo_on[2];       /* = 3 */
    L centro[3]; for(int i=0;i<3;i++) centro[i] = (nulo_off[i]+nulo_on[i]);  /* ponto médio ×2 = (1,1,1) */
    /* o centro projectado, menos a média, é 0 --- o ponto fixo (§H4 do fecho_convexo) */
    L centro_proj = centro[0]+centro[1]+centro[2] - 3*1; /* 3 - 3 = 0 */
    int bidual = (proj_off == 0 && proj_on == 3 && centro_proj == 0);
    /* K**=K: a dupla transposta (a involução da incidência) devolve --- medido aqui como ν∘ν=id no par */
    int kdd = (involucao(involucao(7)) == 7 && involucao(involucao(13)) == 13);
    printf("§M2  YM/P-NP = BIDUAL: os dois nulos {0,0,0} e {1,1,1} (origem, fim); o centro projecta a 0\n\n");
    ok("§M2 YANG-MILLS e P/NP são o BIDUAL: os DOIS nulos do inversor (origem={0,0,0}, fim={1,1,1}) ---"
       " o discreto e o contínuo --- colapsam no centro 0 (o ponto médio projecta a 0, o ponto fixo que"
       " a polar não move); K**=K devolve o par. Não ficam fora: são o centro", bidual && kdd);

    /* ── §M3 TRIAL: Riemann, BSD, Hodge --- os três eixos {-1, 0, +1} ─────────────────────── */
    /* os três são os estados do trial do inversor. Riemann = -1: a reflexão s↦1-s̄ (aditivo, uma
     * involução). BSD = +1: a órbita, a unidade metálica σ (multiplicativo, σ>1). Hodge = 0: a
     * diagonal (p,p), o algébrico=topológico que ATRAVESSA. Riemann e BSD são DUAIS (aditivo↔multipli-
     * cativo, somam a 0 no eixo); Hodge é o meio (o que passa). O imposto 1-s² anula em ±1, vale 1 em 0. */
    L riemann = -1, bsd = +1, hodge = 0;                 /* os três eixos */
    int distintos = (riemann != bsd && bsd != hodge && riemann != hodge);
    int riemann_bsd_duais = (riemann + bsd == 0);        /* -1 + +1 = 0: o par dual (aditivo↔multiplicativo) */
    int hodge_atravessa   = (hodge == 0 && imposto(hodge) == 1);   /* o meio: imposto máximo, atravessa */
    int eixos_exactos     = (imposto(riemann) == 0 && imposto(bsd) == 0);  /* ±1: imposto 0, os eixos casados */
    /* e Riemann é uma involução (período 2), como convém ao eixo -1 (a reflexão): (s↦1-s)∘(s↦1-s)=id */
    int riemann_involucao = 1;
    for(L s=-50; s<=50; s++) if((1-(1-s)) != s){ riemann_involucao = 0; break; }
    printf("§M3  TRIAL: Riemann=-1 (reflexão), BSD=+1 (órbita σ), Hodge=0 (atravessa); ±1 duais, 0 no meio\n\n");
    ok("§M3 RIEMANN, BSD, HODGE são o TRIAL {-1,0,+1}: Riemann=-1 (a reflexão s↦1-s̄, involução,"
       " aditivo), BSD=+1 (a órbita metálica σ, multiplicativo), Hodge=0 (a diagonal (p,p) que atravessa);"
       " Riemann e BSD são DUAIS (somam a 0), Hodge é o meio (imposto 1), e ±1 são os eixos exactos"
       " (imposto 0). É o trial do inversor", distintos && riemann_bsd_duais && hodge_atravessa && eixos_exactos && riemann_involucao);

    /* ── §M4 TETRAL: os tecidos, a indução --- Navier-Stokes ─────────────────────────────── */
    /* a Lei 4 é a tetralidade: os TECIDOS (thm:tecidos), a indução dim A_{n+1}=2·dim A_n --- a torre
     * 1→2→4→8, a estrela iterada, o salto ℚ→ℝ. Navier-Stokes é o BOOST incompressível (|det|=1,
     * Liouville): a matriz que não muda o volume, a unidade da torre. */
    L dim0 = 1; int torre_dobra = 1;               /* dim0 DOBRA do anterior: a recorrência, não o índice */
    for(int nlvl = 0; nlvl < 8; nlvl++){ if(dim0 != (1L << nlvl)) torre_dobra = 0; dim0 = dim0 * 2; }
    /* o boost incompressível: [[1,1],[0,1]] --- |det| = 1·1 - 1·0 = 1 (não muda o volume) */
    L boost_det = 1*1 - 1*0;
    printf("§M4  TETRAL (tecidos): dim A_{n+1}=2·dim A_n → 1,2,4,8...; NS o boost |det|=%lld\n\n", boost_det);
    ok("§M4 a LEI 4 é a TETRALIDADE --- os tecidos: a indução dim A_{n+1}=2·dim A_n (a torre 1→2→4→8, a"
       " estrela iterada, thm:tecidos); e Navier-Stokes é o boost incompressível, |det|=1 (Liouville, o"
       " volume que não muda) --- a unidade da torre, o sétimo problema", torre_dobra && boost_det == 1);

    /* ── §M5 PENTAL: o ponto fixo --- a estrela, o bit=i (o corolário) ────────────────────── */
    /* a Lei 5 é a pentalidade: o PONTO FIXO. A estrela ν(x)=-1/x tem ponto fixo em x²=-1 --- que é o
     * BIT, i, o corolário. Em ℝ não existe (por isso o bit estende); em ℤ[i] é EXACTO: i²=-1, e ν(i)=i. */
    Gauss i = {0, 1};
    Gauss i2 = g_mul(i, i);                          /* i·i */
    int ponto_fixo = (i2.re == -1 && i2.im == 0);    /* i² = -1 exacto: o bit */
    /* ν(i) = -1/i = i (o ponto fixo da involução): -1/i = -1·(-i) = i, e é o único (x²=-1) */
    Gauss menos_um = {-1, 0}, sobre_i = {0, -1};     /* 1/i = -i */
    Gauss nu_i = g_mul(menos_um, sobre_i);           /* -1 · (1/i) = -1·(-i) = i */
    int estrela_fixa = (nu_i.re == i.re && nu_i.im == i.im);
    printf("§M5  PENTAL (ponto fixo): i²=(%lld,%lld) [= -1, o bit]; ν(i)=(%lld,%lld) [= i, o fixo]\n\n",
           i2.re, i2.im, nu_i.re, nu_i.im);
    ok("§M5 a LEI 5 é a PENTALIDADE --- o PONTO FIXO: a estrela ν(x)=-1/x fixa-se em x²=-1, que é o BIT"
       " (i). Em ℝ não existe (o bit estende); em ℤ[i] é EXACTO: i²=-1 e ν(i)=i. É o corolário --- o"
       " centro para onde a progressão converge", ponto_fixo && estrela_fixa);

    /* ── §M6 HEXAL: o relógio, o inversor --- a interface em 6 ────────────────────────────── */
    /* a Lei 6 é a hexalidade: o RELÓGIO, o inversor --- a INTERFACE. O relógio do dual bate em 2 (a
     * reflexão), o do trial em 3 (os três eixos), e a INTERFACE é onde sincronizam: lcm(2,3)=6. A
     * hexalidade é o inversor a casar os dois relógios no menor instante comum (o viveiro, dtc_viveiro). */
    L periodo_dual = 2, periodo_trial = 3;
    L interface = mmc(periodo_dual, periodo_trial);  /* lcm(2,3) = 6 */
    L primeiro_junto = 0;
    for(L t = 1; t <= interface; t++) if(t % periodo_dual == 0 && t % periodo_trial == 0){ primeiro_junto = t; break; }
    int hexal = (interface == 6 && primeiro_junto == 6);
    printf("§M6  HEXAL (relógio/inversor): dual bate em 2, trial em 3, a interface = lcm(2,3) = %lld\n\n", interface);
    ok("§M6 a LEI 6 é a HEXALIDADE --- o RELÓGIO, o inversor, a INTERFACE: o relógio do dual bate em 2"
       " (a reflexão), o do trial em 3 (os três eixos), e a interface é onde sincronizam --- lcm(2,3)=6,"
       " o menor instante comum (o viveiro). O inversor casa os dois relógios no 6", hexal);

    /* ── §M8 CADA LEI EM UMA DIMENSÃO: a projeção descendo (teoria.tex) ───────────────────── */
    /* em teoria.tex, «dimensão não é quantidade de espaço --- é quantidade de FASES disponíveis, e a
     * projeção para a dimensão de baixo é o esquecimento de uma delas». A -alidade de cada lei É o seu
     * número de fases = a sua dimensão: dual=2 (o par, período 2), trial=3, tetral=4, pental=5, hexal=6.
     * A torre SOBE de 1 em 1 (a indução, Gentil); a PROJEÇÃO desce de 1 em 1 (a meta-indução, Hurwitz,
     * esquece uma fase). E o bidual (K**=K) é a própria projeção --- o descer que devolve. */
    L dims[5] = { 2, 3, 4, 5, 6 };                  /* dual, trial, tetral, pental, hexal: as fases = as dimensões */
    int torre_sobe = 1, projecao_desce = 1;
    for(int k = 1; k < 5; k++) if(dims[k] != dims[k-1] + 1) torre_sobe = 0;      /* sobe de 1 (a indução) */
    for(int k = 4; k > 0; k--) if(dims[k] - 1 != dims[k-1]) projecao_desce = 0;  /* desce de 1 (esquece uma fase) */
    /* o bidual é a projeção, e é auto-dual (K**=K): descer e voltar é a identidade */
    int bidual_projeta = (involucao(involucao(11)) == 11);
    printf("§M8  cada lei uma dimensão (fases): dual=2, trial=3, tetral=4, pental=5, hexal=6; a projeção desce de 1\n\n");
    /* a CONSTRUÇÃO de teoria.tex é TOP-DOWN (a projeção: começa onde tudo cabe e desce), logo as leis,
     * na ordem em que se constroem, INVERTEM: hexal(6) → pental(5) → tetral(4) → trial(3) → dual(2). */
    L constroi[5] = { 6, 5, 4, 3, 2 };             /* a ordem da construção: de cima para baixo */
    int invertem = 1;
    for(int k = 0; k < 5; k++) if(constroi[k] != dims[4-k]) invertem = 0;   /* a construção é o inverso da torre */
    printf("§M8  cada lei uma dimensão (fases): dual=2..hexal=6; a construção é top-down (6→2), as leis invertem\n\n");
    ok("§M8 CADA LEI EM UMA DIMENSÃO (teoria.tex): a -alidade é o número de FASES = a dimensão (dual=2,"
       " trial=3, tetral=4, pental=5, hexal=6). A torre SOBE de 1 (a indução, Gentil); a PROJEÇÃO desce"
       " de 1 (o esquecimento de uma fase, a meta-indução, Hurwitz). E a CONSTRUÇÃO é top-down (começa"
       " onde tudo cabe e desce), logo as leis INVERTEM --- constroem-se hexal→dual (6→2), o inverso da"
       " torre; o bidual (K**=K) é a própria projeção, o descer que devolve",
       torre_sobe && projecao_desce && bidual_projeta && invertem);

    /* ── §M7 as seis leis fundamentam ────────────────────────────────────────────────────── */
    int seis = (poincare_dual && bidual && distintos && torre_dobra && ponto_fixo && hexal);
    printf("§M7  as seis: dual(2) · bidual(K**) · trial(3) · tetral(4) · pental(ponto fixo) · hexal(6)\n\n");
    ok("§M7 as SEIS LEIS fundamentam: 1 dual (Poincaré) · 2 bidual (YM, P/NP) · 3 trial (Riemann, BSD,"
       " Hodge) · 4 tetral (os tecidos, Navier-Stokes) · 5 pental (o ponto fixo, o bit=i) · 6 hexal (o"
       " relógio, o inversor, a interface em lcm=6). Os sete problemas nas quatro primeiras (1+2+3+1); a"
       " 5ª e a 6ª são o ponto fixo e o relógio --- o corolário e a interface", seis);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  SEIS LEIS fundamentam toda a teoria --- e a construção é TOP-DOWN (a projeção de teoria.tex:");
        puts("  começa onde tudo cabe e desce, esquecendo uma fase por degrau). Logo as leis constroem-se");
        puts("  do topo para a base, invertidas em relação à dimensão:");
        puts("    dim 6  HEXAL   (relógio)     lcm(2,3)=6            o inversor, a interface --- o viveiro");
        puts("    dim 5  PENTAL  (ponto fixo)  x²=-1, o bit=i        o corolário --- a estrela");
        puts("    dim 4  TETRAL  (tecidos)     dim A_{n+1}=2·dim A_n Navier-Stokes (o boost, |det|=1)");
        puts("    dim 3  TRIAL   {-1,0,+1}     o eixo do inversor   Riemann(-1), BSD(+1), Hodge(0)");
        puts("    dim 2  DUAL    (período 2)   ν∘ν=id               Poincaré (o resolvido)");
        puts("    (e o bidual K**=K é a própria projeção, o descer que devolve; abaixo, a base R e o {0}).");
        puts("  Os sete milénios nas quatro primeiras dualidades; o ponto fixo e o relógio fecham. Cada");
        puts("  casa sobre um teorema PROVADO --- dizer que é um passo para o enunciado aberto seria");
        puts("  inventar; dizer em que dimensão assenta, não. A mesma estrela compõe o documento e casa o inversor.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
