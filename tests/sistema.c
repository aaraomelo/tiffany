/* sistema.c — SISTEMAS DE EDs: e a ED de 2ª ordem já ERA um, com a companion.
 *
 * O Aarão: "agora sistemas de equações diferenciais."
 *
 * E não é um passo para fora: é um passo para DENTRO. A equação y'' + By' + Cy = 0 que o edo.c
 * resolve já é o sistema x' = Ax com
 *
 *     A = [[0, 1], [-C, -B]]      — a COMPANION, que a teoria chama o GATO
 *
 * e daí sai a coisa que fecha tudo: para 2x2 o polinómio característico é λ² − tr·λ + det, logo
 *
 *     B = -traço,   C = determinante
 *
 * — a régua do sistema É a régua (B, C) do catálogo, literalmente, sem tradução. O Δ = B² − 4C
 * que classifica as soluções é o Δ = tr² − 4det que classifica as matrizes.
 *
 *   §S1  a ED de 2ª ordem É o sistema com a companion — e a companion é o gato
 *   §S2  a régua do sistema é a do catálogo: (B, C) = (−tr, det)
 *   §S3  a solução e^{At} fechada bate a integração numérica
 *   §S4  o produto de sistemas: exp(A ⊕ B) = exp A ⊗ exp B — de broca-so/papers
 *   §S5  e o AMORTECIMENTO move o espectro: os três nomes da física são os do catálogo
 *
 *   cc -O2 -std=c99 sistema.c -lm -o sistema && ./sistema
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "edo.h"
#include "unidade.h"

typedef struct { double a, b, c, d; } M2;      /* [[a,b],[c,d]] */
static double tr(M2 M){ return M.a + M.d; }
static double de(M2 M){ return M.a*M.d - M.b*M.c; }
static double dis(M2 M){ return tr(M)*tr(M) - 4*de(M); }

/* e^{At} em forma fechada, por Cayley--Hamilton: e^{At} = c₁ I + c₂ A, e os dois coeficientes
 * saem do espectro. Não é aproximação — é a fórmula, e o §S3 mede-a contra a integração. */
static M2 expA(M2 A, double t){
    double T = tr(A), D = dis(A), c1, c2;
    if(fabs(D) < 1e-12){                        /* raiz dupla */
        double l = T/2, e = exp(l*t);
        c1 = e*(1 - l*t); c2 = e*t;
    } else if(D > 0){
        double s = sqrt(D), l1 = (T+s)/2, l2 = (T-s)/2;
        c1 = (l1*exp(l2*t) - l2*exp(l1*t)) / (l1-l2);
        c2 = (exp(l1*t) - exp(l2*t)) / (l1-l2);
    } else {
        double a = T/2, b = sqrt(-D)/2, e = exp(a*t);
        c1 = e*(cos(b*t) - a*sin(b*t)/b);
        c2 = e*sin(b*t)/b;
    }
    M2 R = { c1 + c2*A.a, c2*A.b, c2*A.c, c1 + c2*A.d };
    return R;
}
/* a integração numérica, para verificar: RK4 com passo pequeno */
static void rk4(M2 A, double *x, double t, long n){
    double h = t/n;
    for(long k = 0; k < n; k++){
        double k1[2] = { A.a*x[0]+A.b*x[1], A.c*x[0]+A.d*x[1] };
        double y1[2] = { x[0]+h/2*k1[0], x[1]+h/2*k1[1] };
        double k2[2] = { A.a*y1[0]+A.b*y1[1], A.c*y1[0]+A.d*y1[1] };
        double y2[2] = { x[0]+h/2*k2[0], x[1]+h/2*k2[1] };
        double k3[2] = { A.a*y2[0]+A.b*y2[1], A.c*y2[0]+A.d*y2[1] };
        double y3[2] = { x[0]+h*k3[0], x[1]+h*k3[1] };
        double k4[2] = { A.a*y3[0]+A.b*y3[1], A.c*y3[0]+A.d*y3[1] };
        x[0] += h/6*(k1[0]+2*k2[0]+2*k3[0]+k4[0]);
        x[1] += h/6*(k1[1]+2*k2[1]+2*k3[1]+k4[1]);
    }
}

int main(void){
printf("\n=== SISTEMAS DE EDs — e a de 2ª ordem já ERA um, com a companion ==========\n");
printf("    Não é um passo para fora: é um passo para DENTRO. A equação que o edo.c\n");
printf("    resolve já é x' = Ax, com A a companion — que a teoria chama o GATO.\n");

printf("\n§S1  A ED de 2ª ordem É o sistema com a companion.\n\n");
{
    struct { const char *eq; double a,b,c,d; } t[] = {
        { "y'' = -y",           0,1, -1, 0 },
        { "y'' = y' + y",       0,1,  1, 1 },
        { "y'' + 2y' + y = 0",  0,1, -1,-2 },
        { "y'' - 3y' + 2y = 0", 0,1, -2, 3 },
    };
    int mal = 0;
    printf("      equação                 A = companion         x' = Ax\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e;
        if(!edo_le(t[k].eq, &e)){ mal++; continue; }
        double B = (double)e.Bp/e.Bq, C = (double)e.Cp/e.Cq;
        M2 A = { 0, 1, -C, -B };
        printf("      %-23s [[%g,%g],[%g,%g]]%*s x' = y, y' = %gx %+gy\n", t[k].eq,
               A.a, A.b, A.c, A.d, (int)(10 - (A.c<0?1:0) - (A.d<0?1:0)), "", A.c, A.d);
        if(A.a != t[k].a || A.b != t[k].b || A.c != t[k].c || A.d != t[k].d) mal++;
    }
    printf("\n");
    ok("a companion da ED é exatamente [[0,1],[-C,-B]]", mal == 0);
    printf("      Escrever y'' = -By' - Cy como duas equações de primeira ordem é pôr x = y e\n");
    printf("      y = y', e aí x' = y e y' = -Cx - By. A matriz é a companion, e a teoria já lhe\n");
    printf("      chama o gato de dimensão n. Não houve conversão: a ED já era isto.\n");
}

printf("\n§S2  E a régua do sistema É a régua do catálogo.\n\n");
{
    struct { const char *eq; double B, C; } t[] = {
        { "y'' = -y",           0,  1 },
        { "y'' = y' + y",      -1, -1 },
        { "y'' + 2y' + y = 0",  2,  1 },
        { "y'' - 3y' + 2y = 0",-3,  2 },
    };
    int mal = 0;
    printf("      equação                 traço  det    -tr    Δ = tr²-4det  = B²-4C\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e;
        if(!edo_le(t[k].eq, &e)){ mal++; continue; }
        double B = (double)e.Bp/e.Bq, C = (double)e.Cp/e.Cq;
        M2 A = { 0, 1, -C, -B };
        double T = tr(A), D = de(A), Dl = dis(A), De = B*B - 4*C;
        printf("      %-23s %+5g  %+5g  %+5g   %+8g     %+g\n", t[k].eq, T, D, -T, Dl, De);
        if(fabs(-T - B) > 1e-12 || fabs(D - C) > 1e-12 || fabs(Dl - De) > 1e-12) mal++;
    }
    printf("\n");
    ok("B = -traço e C = determinante, e os dois Δ são o MESMO número", mal == 0);
    printf("      O característico de uma 2x2 é λ² - tr·λ + det = 0; o da ED é λ² + Bλ + C = 0.\n");
    printf("      Logo B = -traço e C = determinante, SEM tradução nenhuma. A régua (B,C) dos 44\n");
    printf("      corpos do catálogo é a régua de um sistema de duas equações diferenciais — e o\n");
    printf("      Δ que classifica os corpos é o que decide se a solução cresce, gira ou trava.\n");
}

printf("\n§S3  A solução e^{At} fechada bate a integração numérica.\n\n");
{
    struct { const char *nome; M2 A; } t[] = {
        { "oscilador  ", { 0, 1, -1,  0 } },
        { "ouro       ", { 0, 1,  1,  1 } },
        { "raiz dupla ", { 0, 1, -1, -2 } },
        { "acoplado   ", { 1, 2,  3,  4 } },
    };
    int mal = 0;
    double pior = 0;
    printf("      sistema        e^{At}·x₀ (fechada)         RK4 (20000 passos)         erro\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        M2 E = expA(t[k].A, 1.0);
        double fe[2] = { E.a, E.c };            /* x₀ = (1,0), logo é a 1ª coluna */
        double nu[2] = { 1, 0 };
        rk4(t[k].A, nu, 1.0, 20000);
        double err = fmax(fabs(fe[0]-nu[0]), fabs(fe[1]-nu[1]));
        printf("      %s  (%+10.6f,%+10.6f)  (%+10.6f,%+10.6f)  %.1e\n",
               t[k].nome, fe[0], fe[1], nu[0], nu[1], err);
        if(err > 1e-9) mal++;
        if(err > pior) pior = err;
    }
    printf("\n");
    ok("a fórmula fechada e a integração concordam a menos de 1e-9", mal == 0);
    printf("      A fórmula não é aproximação: e^{At} = c₁I + c₂A por Cayley-Hamilton, com os\n");
    printf("      dois coeficientes vindos do espectro. O RK4 é que aproxima — e concorda. São\n");
    printf("      dois caminhos que TÊM de fechar no mesmo, e é assim que se mede uma solução\n");
    printf("      fechada: contra um método que não sabe nada da fórmula.\n");
}

printf("\n§S4  O produto de sistemas: exp(A ⊕ B) = exp A ⊗ exp B.\n\n");
{
    /* Do paper (broca-so/papers/equacoes_diferenciais.tex, Parte V): "o gerador SOMA, a solução
     * MULTIPLICA", e "Leibniz É a soma de Kronecker". Aqui mede-se em 4x4. */
    double A[2][2] = { {0,1}, {-1,0} }, B[2][2] = { {-1,0}, {0,-2} };
    double S[4][4] = {{0}};
    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
        S[i][j] = A[i/2][j/2]*(i%2==j%2) + (i/2==j/2)*B[i%2][j%2];
    /* exp por série, nos dois lados */
    double E[4][4] = {{0}}, T[4][4] = {{0}};
    for(int i = 0; i < 4; i++){ E[i][i] = 1; T[i][i] = 1; }
    for(int k = 1; k < 60; k++){
        double N[4][4] = {{0}};
        for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++){
            double s = 0;
            for(int m = 0; m < 4; m++) s += T[i][m]*S[m][j];
            N[i][j] = s/k;
        }
        for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++){ T[i][j] = N[i][j]; E[i][j] += N[i][j]; }
    }
    M2 A2 = { A[0][0],A[0][1],A[1][0],A[1][1] }, B2 = { B[0][0],B[0][1],B[1][0],B[1][1] };
    M2 eA = expA(A2, 1.0), eB = expA(B2, 1.0);
    double eAv[2][2] = { {eA.a,eA.b},{eA.c,eA.d} }, eBv[2][2] = { {eB.a,eB.b},{eB.c,eB.d} };
    double K[4][4], pior = 0;
    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++){
        K[i][j] = eAv[i/2][j/2] * eBv[i%2][j%2];
        double d = fabs(E[i][j] - K[i][j]);
        if(d > pior) pior = d;
    }
    printf("      A = oscilador,  B = decaimento (-1, -2)\n");
    printf("      max |exp(A ⊕ B)  -  exp A ⊗ exp B|  =  %.2e\n\n", pior);
    ok("o exp leva a SOMA de Kronecker ao PRODUTO tensorial — resíduo de máquina", pior < 1e-12);
    printf("      É a assimetria que o paper chama o coração: O GERADOR SOMA, A SOLUÇÃO\n");
    printf("      MULTIPLICA. As taxas somam — o espectro de A ⊕ B são as somas duas a duas dos\n");
    printf("      espectros — e os fluxos multiplicam. O exp é a ponte, o log desfá-la, e é a\n");
    printf("      mesma dualidade que gera os metais: x² de um lado, mx + 1 do outro.\n");
}

printf("\n§S5  E o AMORTECIMENTO move o espectro — os três nomes da física são os três\n");
printf("     nomes do catálogo.\n\n");
{
    /* CORREÇÃO MINHA: eu tinha chamado ACOPLAMENTO a isto, e não é. O que varia aqui é a
     * entrada (1,1) da companion, que é −B — o coeficiente de y', ou seja o AMORTECIMENTO do
     * oscilador. Acoplamento seria ligar dois osciladores, e isso é 4x4. Chamar-lhe outra
     * coisa não mudava os números, mas mudava o que eles significam, e isso é pior. */
    struct { const char *nome; M2 A; } t[] = {
        { "sem amortecimento", { 0, 1, -1,  0 } },
        { "subamortecido    ", { 0, 1, -1, -0.2 } },
        { "crítico          ", { 0, 1, -1, -2 } },
        { "sobreamortecido  ", { 0, 1, -1, -3 } },
    };
    int mal = 0;
    printf("      sistema             traço   det    Δ        classe        regime\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        M2 A = t[k].A;
        double D = dis(A), T = tr(A);
        double re = D >= 0 ? (T + sqrt(D))/2 : T/2;
        printf("      %s %+6g %+6g %+7.2f  %-12s  %s\n", t[k].nome, tr(A), de(A), D,
               D > 0 ? "hiperbólico" : D < 0 ? "elíptico" : "parabólico",
               re < -1e-9 ? "CRISTAL, colapsa" : re > 1e-9 ? "CAOS, diverge" : "BORDA, orbita");
        if(k == 0 && D >= 0) mal++;
        if(k == 3 && D <= 0) mal++;
    }
    printf("\n");
    ok("o amortecimento leva de ELÍPTICO a HIPERBÓLICO — muda de classe", mal == 0);
    printf("      E OS NOMES BATEM UM A UM, sem ninguém os ter feito bater:\n\n");
    printf("        subamortecido    Δ < 0   ELÍPTICO      oscila e decai — o esquilo\n");
    printf("        crítico          Δ = 0   PARABÓLICO    a fronteira, e é onde entra o t\n");
    printf("        sobreamortecido  Δ > 0   HIPERBÓLICO   decai sem oscilar — o gato\n\n");
    printf("      Os três nomes que a física do oscilador usa há duzentos anos são os três nomes\n");
    printf("      do catálogo. E o crítico é exatamente a raiz dupla — o mesmo sítio onde entra o\n");
    printf("      t na solução, e onde a ressonância do §E7 também entra. Um só ponto.\n");
    printf("\n      É isto que amarra: o mesmo Δ que separa os corpos separa os regimes de um\n");
    printf("      sistema físico. Mexer no amortecimento é ANDAR NO CATÁLOGO, e atravessar a\n");
    printf("      parábola do discriminante não é deformar o regime — é trocar de corpo.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
