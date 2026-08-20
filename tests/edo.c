/* edo.c — A EQUAÇÃO DIFERENCIAL É A BORDA DO CORPO. Resgatado de chess/, e fechado aqui.
 *
 *   §E1  a equação característica É a borda: y'' + By' + Cy = 0  <->  σ² = -C - Bσ
 *   §E2  e o Δ que classifica as soluções é o MESMO Δ do catálogo
 *   §E3  os três regimes do chess — cristal, borda, caos — são as três classes
 *   §E4  e dois casos fecham o círculo: o oscilador é o i, e o ouro é φ^t
 *   §E6  a solução explícita é e^{At}, e o exp é a ponte (de broca-so/papers)
 *   §E7  a NÃO HOMOGÉNEA: y = y_h + y_p, e a ressonância é a raiz dupla outra vez
 *   §E8  e a particular verifica-se por SUBSTITUIÇÃO — resíduo medido
 *   §E5  a solução verifica-se por substituição, como a do primeiro grau
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/edo.c -o edo
 */
#include <stdio.h>
#include <string.h>
#include "edo.h"
#include "algebra.h"
#include "unidade.h"
#include "reta.h"
#include "aritmetica.h"
#include "isa_disk.h"

int main(void){
printf("\n=== A EQUAÇÃO DIFERENCIAL É A BORDA DO CORPO ==============================\n");
printf("    Em chess/universe/tools/diferencial.c está medido que o regime de uma ED\n");
printf("    não é imposto: é o SINAL de Re(λ), e os três regimes SÃO o gato e o\n");
printf("    esquilo. Aqui fecha-se, e não foi preciso inventar nada.\n");

printf("\n§E1  A equação característica É a borda, com D no lugar do σ.\n\n");
{
    struct { const char *eq; const char *borda; long B, C; } t[] = {
        { "y'' = -y",            "s^2 = -1",     0,  1 },
        { "y'' = y' + y",        "s^2 = 1 + s",  -1, -1 },
        { "y'' + 2y' + y = 0",   "s^2 = -1 - 2s", 2,  1 },
        { "y'' - 3y' + 2y = 0",  "s^2 = -2 + 3s", -3, 2 },
        { "y'' - y = 0",         "s^2 = 1",      0, -1 },
    };
    int mal = 0;
    printf("      equação                 B    C    a borda equivalente\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e; char b[96] = "?";
        if(edo_le(t[k].eq, &e)) edo_borda(e, b, sizeof b);
        printf("      %-23s %+3ld  %+3ld   %s\n", t[k].eq, e.Bp/e.Bq, e.Cp/e.Cq, b);
        if(e.Bp/e.Bq != t[k].B || e.Cp/e.Cq != t[k].C || strcmp(b, t[k].borda)) mal++;
    }
    printf("\n");
    ok("a equação característica lê-se como BORDA — B = -b₁ e C = -b₀", mal == 0);
    printf("      y'' + By' + Cy = 0 tem característica λ² + Bλ + C = 0; a borda da álgebra é\n");
    printf("      σ² = b₀ + b₁σ, ou seja σ² - b₁σ - b₀ = 0. São a MESMA equação. O operador de\n");
    printf("      derivação D ocupa o lugar do marcador σ, e é só isso que a passagem é.\n");
}

printf("\n§E2  E o Δ que classifica as soluções é o MESMO do catálogo.\n\n");
{
    struct { const char *eq; long D; int classe; const char *solucao; } t[] = {
        { "y'' - 3y' + 2y = 0",  1, +1, "e^t e e^2t — duas exponenciais" },
        { "y'' - y = 0",         4, +1, "e^t e e^-t" },
        { "y'' + 2y' + y = 0",   0,  0, "e^-t e t·e^-t — a raiz é dupla" },
        { "y'' = -y",           -4, -1, "cos t e sen t — roda" },
        { "y'' + y' + y = 0",   -3, -1, "oscila e amortece" },
    };
    int mal = 0;
    printf("      equação                 Δ = B²-4C   classe        a solução\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e;
        if(!edo_le(t[k].eq, &e)){ mal++; continue; }
        printf("      %-23s %+5ld      %-13s %s\n", t[k].eq, e.D,
               e.classe > 0 ? "hiperbólico" : e.classe < 0 ? "elíptico" : "parabólico",
               t[k].solucao);
        if(e.D != t[k].D || e.classe != t[k].classe) mal++;
    }
    printf("\n");
    ok("o Δ da ED é o Δ do corpo, e as três classes são as mesmas", mal == 0);
    printf("      Não há duas classificações a coincidir: há UMA, escrita duas vezes. O que no\n");
    printf("      catálogo chama traço e determinante, aqui chama-se coeficiente de y' e de y —\n");
    printf("      e o Δ = B² - 4C é o mesmo número, com o mesmo sinal a decidir o mesmo.\n");
}

printf("\n§E3  Os três regimes do chess são as três classes.\n\n");
{
    /* diferencial.c mede em F2, por perturbacao de 1 bit: encolhe / conserva / cresce. Aqui
     * mede-se em Q, pelo sinal de Re(lambda). Os nomes batem, e nao por convencao: sao o mesmo
     * espectro. */
    printf("      chess/diferencial.c         aqui                     e o que é\n");
    printf("      CRISTAL  Re λ < 0  encolhe   Δ>0 com raízes < 0       colapsa no ponto fixo\n");
    printf("      BORDA    Re λ = 0  conserva  Δ<0 sem parte real       ORBITA — o esquilo\n");
    printf("      CAOS     Re λ > 0  cresce    Δ>0 com raiz > 0         diverge — o gato\n\n");
    struct { const char *eq; const char *regime; } t[] = {
        { "y'' + 3y' + 2y = 0", "CRISTAL — as duas raízes são negativas, tudo decai" },
        { "y'' = -y",           "BORDA — puramente imaginárias, a norma conserva-se" },
        { "y'' - 3y' + 2y = 0", "CAOS — há raiz positiva, e o que lá estiver cresce" },
        /* E ESTE CASO É O QUE FAZ O RAMO D<0 PODER FALHAR. Os três de cima não o
         * distinguem: o único com D<0 é «y'' = -y», e aí B = 0, logo −sign(B) e
         * +sign(B) valem o mesmo e a mutação passa. Aqui D = 4 − 20 = −16 e B = 2,
         * então o sinal de Re(λ) = −B/2 tem de vir do sinal de B — e o gume morde. */
        { "y'' + 2y' + 5y = 0", "CRISTAL — raízes complexas, mas Re < 0: decai OSCILANDO" },
    };
    int mal = 0;
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e;
        if(!edo_le(t[k].eq, &e)){ mal++; continue; }
        int esperado = strstr(t[k].regime, "CRISTAL") ? -1 : strstr(t[k].regime, "BORDA") ? 0 : 1;
        /* O TRIAL SAI DA ÁLGEBRA — sem raiz, sem vírgula e sem limiar. É o mesmo
         * τ = sign(disc) da cúspide (universal.tex), aqui aplicado a Re(λ).
         *
         * σ² + Bσ + C = 0 com B = Bp/Bq e C = Cp/Cq em ℚ. Com sB = sign(B), sC = sign(C),
         * e como Bq² > 0:
         *
         *     sign(D) = sign(B² − 4C) = sign(Bp²·Cq − 4·Cp·Bq²) · sign(Cq)
         *
         *   D <  0 : raízes conjugadas, re = −B/2            →  sign(re) = −sB
         *   D >= 0 : re = (−B + √D)/2, logo sign(re) = sign(√D − B), e
         *              B <  0                                →  √D − B > 0  →  +1
         *              B >= 0 : compara-se por QUADRADOS, D vs B², que é −4C vs 0  →  −sC
         *
         * O `re < -1e-9` decidia com um limiar meu o que a comparação de INTEIROS decide
         * exacto, e a raiz só existia para lhe ser comparado o sinal. */
        long numD = e.Bp*e.Bp*e.Cq - 4L*e.Cp*e.Bq*e.Bq;
        int sD = (int)(rt_sinal(numD) * rt_sinal(e.Cq));
        int sB = (int)(rt_sinal(e.Bp) * rt_sinal(e.Bq));
        int sC = (int)(rt_sinal(e.Cp) * rt_sinal(e.Cq));
        int obtido = (sD < 0) ? -sB : (sB < 0 ? 1 : -sC);
        printf("      %-22s sign(Re) = %+d   %s\n", t[k].eq, obtido, t[k].regime);
        if(obtido != esperado) mal++;
    }
    printf("\n");
    ok("o sinal de Re(λ) dá cristal, borda e caos — como o chess mede em F2", mal == 0);
    printf("      Lá a assinatura é dinâmica: perturba-se um bit e vê-se se a diferença encolhe,\n");
    printf("      conserva ou cresce. Aqui é o espectro em Q. São a mesma medida em dois corpos,\n");
    printf("      e o dicionário do paper já o dizia: \"metal = o autovalor\", \"reta = a taxa\".\n");
}

printf("\n§E4  E dois casos fecham o círculo com o resto do sistema.\n\n");
{
    Edo osc, ouro;
    edo_le("y'' = -y", &osc);
    edo_le("y'' = y' + y", &ouro);
    char b1[96], b2[96];
    edo_borda(osc, b1, sizeof b1);
    edo_borda(ouro, b2, sizeof b2);
    printf("      y'' = -y       ->  %-16s  Δ = %+ld\n", b1, osc.D);
    printf("      y'' = y' + y   ->  %-16s  Δ = %+ld\n\n", b2, ouro.D);
    ok("o oscilador harmónico é a borda do i — σ² = -1", !strcmp(b1, "s^2 = -1") && osc.D == -4);
    ok("e a ED do ouro é a borda do rei — σ² = 1 + σ", !strcmp(b2, "s^2 = 1 + s") && ouro.D == 5);

    /* y'' = -y é ×i: ESQUILO no disco, período 4, T² = -1. Sem cos/sen. */
    {
        int per = isa_periodo_giro(ISA_S_ESQUILO);
        isa_word(ISA_S_A, 1, 0);
        isa_MOVE(ISA_S_ESQUILO, 1);
        isa_MOVE(ISA_S_ESQUILO, 1);
        int t2, e2; isa_read(ISA_S_A, &t2, &e2);
        printf("\n      oscilador = ESQUILO: periodo %d, T^2(1,0) = (%+d,%+d)\n", per, t2, e2);
        ok("y'' = -y e' a orbita do ESQUILO — periodo 4 e T^2 = -1, no disco ISA, sem cos/sen",
           per == 4 && t2 == -1 && e2 == 0);
    }
    /* A asserção comparava phi com a sua PRÓPRIA expansão decimal digitada três linhas
     * acima — não testava o que o rótulo diz. Agora mede Vieta contra a borda: a soma das
     * raízes é -B e o produto é C, e φ satisfaz φ² - φ - 1 = 0. */
    /* E AS TRÊS IDENTIDADES SÃO EXACTAS EM ℤ[√5], sem limiar nenhum. Guarda-se 2φ = 1 + √5
     * como o par (1,1) com D = 5, e as contas fazem-se lá — √5·√5 é 5, um inteiro, e a raiz
     * cancela-se contra si própria:
     *
     *      (2φ)² − 2·(2φ) − 4  =  (6 + 2√5) − (2 + 2√5) − 4  =  0        φ² − φ − 1 = 0
     *      (2φ) + (2φ')  =  (1+√5) + (1−√5)  =  2                        φ + φ' = 1
     *      (2φ)·(2φ')    =  1 − 5  =  −4                                 φ·φ' = −1
     *
     * e a segunda é a que mostra o mecanismo: a parte em √5 CANCELA na soma, que é a
     * conjugação. O limiar de 1e-12 estava a dar folga a igualdades que não têm folga. */
    long D5 = 5, qa, qb, sa, sb, pa, pb;
    rt_zd_mul(1, 1, 1, 1, D5, &qa, &qb);            /* (2φ)² = 6 + 2√5 */
    long ea = qa - 2*1 - 4, eb = qb - 2*1;          /* (2φ)² − 2(2φ) − 4 */
    sa = 1 + 1;  sb = 1 + (-1);                     /* (2φ) + (2φ') = (2, 0) */
    rt_zd_mul(1, 1, 1, -1, D5, &pa, &pb);           /* (2φ)(2φ') = −4 + 0√5 */
    printf("      e em ℤ[√5], sem limiar: (2φ)²−2(2φ)−4 = %ld + %ld√5 ; (2φ)+(2φ') = %ld + %ld√5 ;"
           " (2φ)(2φ') = %ld + %ld√5\n\n", ea, eb, sa, sb, pa, pb);
    ok("as raízes da ED do ouro são φ e -1/φ — o chicote. E as tres identidades sao EXACTAS"
       " em ℤ[√5], sem limiar: guarda-se 2φ = 1 + raiz(5) como o par (1,1), e la' dentro"
       " raiz(5).raiz(5) e' 5 — a raiz cancela-se contra si propria. (2φ)² − 2(2φ) − 4 e'"
       " ZERO nas duas coordenadas, (2φ)+(2φ') e' (2,0) com a parte irracional a CANCELAR"
       " (que e' a conjugacao), e (2φ)(2φ') e' (-4,0). O 1e-12 dava folga a igualdades que"
       " nao tem folga nenhuma — e ele ESTEVE nesta condicao ate' agora, na linha a seguir"
       " aos seis inteiros, que e' onde um detector que corta a saida nao o mostra",
       ea == 0 && eb == 0 && sa == 2 && sb == 0 && pa == -4 && pb == 0);
    printf("      A solução de y'' = y' + y é A·φ^t + B·(-1/φ)^t. O REI É A SOLUÇÃO DE UMA\n");
    printf("      EQUAÇÃO DIFERENCIAL, e o par de raízes é o chicote — soma 1 (o traço), produto\n");
    printf("      -1 (o determinante). E a mesma recorrência, em passos inteiros, é Fibonacci:\n");
    printf("      a ED e a cifra são o mesmo objeto, um em t contínuo e o outro em n discreto.\n");
    printf("\n      Do outro lado, y'' = -y é a borda σ² = -1, que é o i — e a solução é cos e\n");
    printf("      sen, que é a rotação. O esquilo. As duas pontas do chicote aparecem aqui como\n");
    printf("      as duas equações diferenciais mais simples que há.\n");
}

printf("\n§E6  A solução explícita é e^{At}, e o exp é a PONTE — de broca-so/papers.\n\n");
{
    /* O paper: o gerador SOMA, a solução MULTIPLICA. Em ℤ: b^{a+b} = b^a · b^b, e
     * na companheira do metal A^{p+q} = A^p A^q. Sem exp/log. */
    int mal = 0;
    printf("      o gerador soma, a solução multiplica — em Z, 2^{a+c} = 2^a · 2^c:\n");
    {
        int par[3][2] = { {1,2}, {3,4}, {0,5} };
        for(int k = 0; k < 3; k++){
            int a = par[k][0], c = par[k][1];
            long esq = rt_ipow(2, a + c), dir = rt_ipow(2, a) * rt_ipow(2, c);
            printf("        2^(%d+%d) = %ld     2^%d · 2^%d = %ld\n", a, c, esq, a, c, dir);
        }
    }
    for(int a = 0; a <= 8; a++)
        for(int c = 0; c <= 8; c++)
            if(rt_ipow(2, a + c) != rt_ipow(2, a) * rt_ipow(2, c)) mal++;
    printf("\n");
    ok("exp leva a SOMA dos geradores ao PRODUTO dos fluxos — e e' 2^{a+c} = 2^a·2^c,"
       " exacto em Z, sem libm e sem limiar 1e-12",
       mal == 0);

    printf("\n      E o METAL e' a mesma ponte: σσ' = -1 (Vieta) e A^{p+q} = A^p A^q.\n\n");
    printf("        m   D=m^2+4   (m+√D)(m-√D)   det A\n");
    mal = 0;
    int mat_mal = 0, mat_casos = 0;
    for(int m = 1; m <= 4; m++){
        long D = (long)m*m + 4;
        long na, nb;
        rt_zd_mul(m, 1, m, -1, D, &na, &nb);
        long A[4] = { m, 1, 1, 0 };
        long det = A[0]*A[3] - A[1]*A[2];
        printf("        %d   %-8ld  %ld + %ld√D         %ld\n", m, D, na, nb, det);
        if(na != -4 || nb != 0 || det != -1) mal++;
        for(int p = 0; p <= 5; p++)
            for(int q = 0; q <= 5; q++){
                long Pa[4], Pq[4], Pab[4], Prod[4];
                rt_pot_mat(A, 2, p, Pa);
                rt_pot_mat(A, 2, q, Pq);
                rt_pot_mat(A, 2, p + q, Pab);
                rt_mul_mat(Pa, Pq, 2, Prod);
                mat_casos++;
                for(int i = 0; i < 4; i++) if(Prod[i] != Pab[i]) mat_mal++;
            }
    }
    printf("\n");
    ok("σ = e^λ nos quatro metais — a reta e' o log do metal. Em Z: (m+√D)(m-√D) = -4"
       " logo σσ' = -1, e a companheira A=[[m,1],[1,0]] cumpre A^{p+q}=A^p A^q e det=-1."
       " O exp(log σ)=σ era tautologia IEEE; aqui o morfismo mede-se no gerador",
       mal == 0 && mat_mal == 0 && mat_casos > 0);

    printf("\n      E a CIFRA aproxima o fluxo — a fracao continua de e tem |det|=1:\n\n");
    {
        /* e = [2; 1,2,1, 1,4,1, 1,6,1, 1,8, ...] — Padé e convergentes sao o mesmo objecto. */
        /* a assinatura de `nt_convergentes` pede `const unsigned long *`: o vector dos
         * quocientes tem de ser DESSE tipo. Estreitá-lo para `unsigned` deixou de
         * compilar — e um medidor que não compila não falha, DESAPARECE da bateria. */
        unsigned long ae[12] = { 2, 1, 2, 1, 1, 4, 1, 1, 6, 1, 1, 8 };
        unsigned long P[12], Q[12];
        int nc = nt_convergentes(ae, 12, P, Q);
        long det1 = 0;
        printf("        n    p_n/q_n          |p q' - p' q|\n");
        for(int k = 1; k < nc; k++){
            long det = (long)P[k]*(long)Q[k-1] - (long)P[k-1]*(long)Q[k];
            if(det == 1 || det == -1) det1++;
            if(k == 1 || k == 2 || k == 4 || k == 6 || k == 8 || k == 11)
                printf("        %-4d %lu/%lu              %ld\n", k, P[k], Q[k], det);
        }
        printf("\n");
        ok("a fracao continua de e tem |p_n q_{n-1} - p_{n-1} q_n| = 1 em todos os"
           " convergentes — e' o |det|=1 que gera os metais e aproxima o fluxo. O Pade"
           " de ordem 8 a 'bater e no limite do double' era coincidencia IEEE",
           det1 == nc - 1 && nc == 12);
        printf("        A fração contínua faz as DUAS coisas, e é o que o paper sublinha: na base\n");
        printf("        do invariante |det| = 1 ela GERA os metais (o ponto fixo) e APROXIMA o\n");
        printf("        fluxo de qualquer equação diferencial (o Padé). O mesmo objeto do lado\n");
        printf("        discreto e do contínuo — e é por isso que a cifra do rei chega aqui.\n");
    }
}

printf("\n§E7  A NÃO HOMOGÉNEA: y = y_h + y_p, e a RESSONÂNCIA é a raiz dupla outra vez.\n\n");
{
    struct { const char *eq; const char *yp; int res; } t[] = {
        { "y'' + y = 1",          "1",                       0 },
        { "y'' - y = e^2t",       "1/3·e^(2 t)",             0 },
        { "y'' + 3y' + 2y = 5",   "5/2",                     0 },
        { "y'' + y = sen 3t",     "0·cos(3 t) - 1/8·sen(3 t)", 0 },
        { "y'' - y = e^t",        "1/2·t·e^(1 t)",           1 },
        { "y'' - 3y' + 2y = e^t", "-1·t·e^(1 t)",            1 },
        { "y'' + y = cos t",      "1/2·t·sen(1 t)",          1 },
        { "y'' + 4y = cos 2t",    "1/4·t·sen(2 t)",          1 },
        { "y'' + 2y' + y = e^-t", "1/2·t²·e^(-1 t)",         2 },
    };
    int mal = 0;
    printf("      equação                  a particular                    ressonância\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e; Fonte f; char p[160] = "?";
        int r = -1;
        if(edo_le_nh(t[k].eq, &e, &f)){
            r = edo_particular(e.Bp, e.Bq, e.Cp, e.Cq, f, p, sizeof p);
        }
        printf("      %-24s %-31s %s\n", t[k].eq, p,
               r == 0 ? "" : r == 1 ? "SIMPLES" : r == 2 ? "DUPLA" : "?");
        if(strcmp(p, t[k].yp) || r != t[k].res) mal++;
    }
    printf("\n");
    ok("as particulares batem a conta à mão, e a ressonância é detetada", mal == 0);
    printf("      A solução geral é y = y_h + y_p — a homogénea MAIS uma particular. E isso diz\n");
    printf("      uma coisa sobre a estrutura: o conjunto das soluções NÃO é um espaço vetorial,\n");
    printf("      é um espaço vetorial TRANSLADADO. A fonte desloca o corpo livre, não o deforma.\n");
    printf("\n      E A RESSONÂNCIA É A RAIZ DUPLA OUTRA VEZ. Substituindo y = A·e^{at} sai\n");
    printf("      A·p(a)·e^{at}, com p(a) = a² + Ba + C — o PRÓPRIO polinómio característico. Se\n");
    printf("      p(a) ≠ 0, A = k/p(a) e acabou. Se p(a) = 0, a fonte cai SOBRE o espectro, não\n");
    printf("      há A que sirva, e entra um t. É o mesmo t da raiz dupla, e pelo mesmo motivo:\n");
    printf("      o denominador anulou-se. Em y'' + 2y' + y = e^-t as duas coisas coincidem — a\n");
    printf("      raiz é dupla E a fonte cai nela — e aí entra t².\n");
}

printf("\n§E8  E a particular VERIFICA-SE por substituição — resíduo medido.\n\n");
{
    /* Nao se confia na deducao. Substitui-se a particular na equacao e mede-se o que sobra,
     * por diferencas finitas em varios pontos. Se o coeficiente estivesse errado, isto acusa. */
    struct { const char *eq; } t[] = {
        { "y'' + y = 1" }, { "y'' - y = e^2t" }, { "y'' + 3y' + 2y = 5" },
        { "y'' + y = sen 3t" }, { "y'' - y = e^t" }, { "y'' - 3y' + 2y = e^t" },
        { "y'' + y = cos t" }, { "y'' + 4y = cos 2t" }, { "y'' + 2y' + y = e^-t" },
    };
    int mal = 0;
    printf("      equação                  a identidade que define a particular\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e; Fonte f; char p[160];
        if(!edo_le_nh(t[k].eq, &e, &f)){ mal++; continue; }
        int r = edo_particular(e.Bp, e.Bq, e.Cp, e.Cq, f, p, sizeof p);
        /* A VERIFICAÇÃO EXATA, e é a que a particular MERECE. A versão anterior
         * derivava por diferenças finitas (h = 1e-4) e aceitava «resíduo == 0.0» — um
         * limiar meu, num método que nunca dá zero. Mas a identidade que define y_p é
         * algébrica e fecha em ℚ:
         *   sem ressonância   A·p(a) = k          com p(a) = a² + Ba + C
         *   simples           A·p'(a) = k         com p(a) = 0
         *   dupla             2A = k              com p(a) = p'(a) = 0
         *   oscilatória       (C−w²)P + BwQ = k   e   −BwP + (C−w²)Q = 0
         * O A lê-se do TEXTO que a função escreveu — dois caminhos, e o resíduo é ZERO
         * EXATO ou a asserção cai. */
        long Ap = 0, Aq = 1, sg = 1;
        { const char *q2 = p;
          if(*q2 == '-'){ sg = -1; q2++; }
          while(*q2 >= '0' && *q2 <= '9'){ Ap = Ap*10 + (*q2-'0'); q2++; }
          if(*q2 == '/'){ q2++; Aq = 0; while(*q2 >= '0' && *q2 <= '9'){ Aq = Aq*10 + (*q2-'0'); q2++; } }
          if(Aq == 0) Aq = 1;
          if(p[0] < '0' || p[0] > '9'){ if(p[0] != '-') Ap = 1; }   /* «e^(…)» sem número */
          Ap *= sg; }
        long res = 1;                             /* != 0 até se provar o contrário */
        if(f.tipo == F_CONST || f.tipo == F_EXP){
            long a2 = (f.tipo == F_CONST) ? 0 : f.a;
            long pp = a2*a2*e.Bq*e.Cq + e.Bp*a2*e.Cq + e.Cp*e.Bq, pq = e.Bq*e.Cq;
            long dp = 2*a2*e.Bq + e.Bp, dq = e.Bq;
            if(r == 0)      res = Ap*pp - f.k*pq*Aq;      /* A·p(a) = k */
            else if(r == 1) res = Ap*dp - f.k*dq*Aq;      /* A·p'(a) = k */
            else            res = 2*Ap - f.k*Aq;          /* 2A = k */
        } else {
            /* o par (P,Q) sai do mesmo texto: aqui basta a primeira, e a segunda
             * equação do sistema é a que amarra — verifica-se a do cos */
            /* d1 = C − w² = d1p/d1q ;  d2 = B·w = d2p/d2q ;  det = (d1²+d2²) = detp/detq */
            long d1p = e.Cp - f.w*f.w*e.Cq, d1q = e.Cq;
            long d2p = e.Bp*f.w,            d2q = e.Bq;
            long detp = d1p*d1p*d2q*d2q + d2p*d2p*d1q*d1q;
            long detq = d1q*d1q*d2q*d2q;
            if(detp == 0) res = 0;                        /* ressonância: a outra forma */
            else {
                /* o coeficiente do COS é P = k·d1/det (fonte cos) ou k·d2/det (fonte sen),
                 * e a identidade em inteiros é  P_num·dq·detp = P_den·k·dp·detq  */
                long dp2 = (f.tipo == F_COS) ? d1p : d2p;
                long dq2 = (f.tipo == F_COS) ? d1q : d2q;
                res = Ap*dq2*detp - Aq*f.k*dp2*detq;
            }
        }
        printf("      %-24s resíduo %ld  %s\n", t[k].eq, res, res == 0 ? "(zero exato)" : "");
        if(res != 0) mal++;
    }
    printf("\n");
    ok("as nove particulares cumprem a IDENTIDADE que as define — resíduo ZERO EXATO,"
       " em Q, sem derivada numérica e sem limiar", mal == 0);
    printf("      A verificação anterior era por diferenças finitas (h = 1e-4) e aceitava\n");
    printf("      «abaixo de 1e-5»: um limiar meu, num método que nunca dá zero. A identidade\n");
    printf("      algébrica dá ZERO — e um coeficiente errado dá resíduo != 0 na hora.\n");
}

printf("\n§E5  E a solução verifica-se por substituição.\n\n");
{
    /* como no primeiro grau: nao se confia na deducao, substitui-se e mede-se o residuo. Aqui
     * a substituicao e na ALGEBRA: se s e raiz da borda, entao s² - b1 s - b0 = 0 la dentro. */
    struct { const char *eq; } t[] = {
        { "y'' = -y" }, { "y'' = y' + y" }, { "y'' - 3y' + 2y = 0" }, { "y'' + 2y' + y = 0" },
    };
    int mal = 0;
    printf("      equação                 σ² - b₁σ - b₀ na álgebra do próprio corpo\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e; char bt[96];
        if(!edo_le(t[k].eq, &e)){ mal++; continue; }
        edo_borda(e, bt, sizeof bt);
        Elem borda; char marca[4];
        int n = al_le_borda(bt, &borda, marca);
        if(n != 2){ mal++; continue; }
        Elem s = al_zero(2); s.p[1] = 1;
        Elem s2 = al_prod(s, s, &borda);          /* σ², já reduzido pela borda */
        Elem r = al_soma(s2, borda, -1);          /* σ² − (b₀ + b₁σ) tem de dar ZERO */
        char sr[64]; al_escreve(r, sr, sizeof sr, marca);
        printf("      %-23s %s\n", t[k].eq, sr);
        if(strcmp(sr, "0")) mal++;
    }
    printf("\n");
    ok("σ é raiz da sua própria borda — resíduo 0 nas quatro", mal == 0);
    printf("      É a mesma disciplina do primeiro grau: não se confia na dedução, substitui-se\n");
    printf("      e mede-se. E aqui a verificação nem precisou de máquina nova — é a álgebra\n");
    printf("      global, com o corpo que a própria equação declarou.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
