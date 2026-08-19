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
 *   §S3  a órbita: Cayley–Hamilton e A^{a+b}=A^a A^b, em ℤ
 *   §S4  o produto de sistemas: o gerador SOMA (tr A⊕B), a solução MULTIPLICA (tr A⊗B)
 *   §S5  e o AMORTECIMENTO move o espectro: os três nomes da física são os do catálogo
 *
 * LEI vs TRANSPORTE. e^{At} em forma fechada contra RK4, a série de exp(A⊕B) e max Re(λ)
 * com 1e-9 eram o método. A lei é a companion em ℚ (edo_le), os dois Δ iguais em ℤ,
 * p(A)=0 e A²A³=A⁵, traço da soma/produto de Kronecker, e o sinal de Δ = B²−4C.
 *
 *   cc -O2 -std=c99 -I lib tests/sistema.c -o sistema && ./sistema
 */
#include <stdio.h>
#include <string.h>
#include "edo.h"
#include "reta.h"
#include "unidade.h"

typedef struct { long a, b, c, d; } M2;      /* [[a,b],[c,d]] */
static long tr(M2 M){ return M.a + M.d; }
static long de(M2 M){ return M.a*M.d - M.b*M.c; }
static long dis(M2 M){ return tr(M)*tr(M) - 4*de(M); }

static M2 mmul(M2 X, M2 Y){
    M2 R = { X.a*Y.a + X.b*Y.c, X.a*Y.b + X.b*Y.d,
             X.c*Y.a + X.d*Y.c, X.c*Y.b + X.d*Y.d };
    return R;
}
static M2 mpot(M2 A, int k){
    M2 R = { 1, 0, 0, 1 };
    for(int t = 0; t < k; t++) R = mmul(R, A);
    return R;
}
static int ch2(M2 A){
    M2 A2 = mmul(A, A);
    long T = tr(A), D = de(A);
    return A2.a - T*A.a + D == 0
        && A2.b - T*A.b     == 0
        && A2.c - T*A.c     == 0
        && A2.d - T*A.d + D == 0;
}
static int migual(M2 X, M2 Y){
    return X.a == Y.a && X.b == Y.b && X.c == Y.c && X.d == Y.d;
}

int main(void){
printf("\n=== SISTEMAS DE EDs — e a de 2ª ordem já ERA um, com a companion ==========\n");
printf("    Não é um passo para fora: é um passo para DENTRO. A equação que o edo.c\n");
printf("    resolve já é x' = Ax, com A a companion — que a teoria chama o GATO.\n");

printf("\n§S1  A ED de 2ª ordem É o sistema com a companion.\n\n");
{
    struct { const char *eq; long a,b,c,d; } t[] = {
        { "y'' = -y",           0, 1, -1,  0 },
        { "y'' = y' + y",       0, 1,  1,  1 },
        { "y'' + 2y' + y = 0",  0, 1, -1, -2 },
        { "y'' - 3y' + 2y = 0", 0, 1, -2,  3 },
    };
    int mal = 0;
    printf("      equação                 A = companion         x' = Ax\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e;
        if(!edo_le(t[k].eq, &e) || e.Bq != 1 || e.Cq != 1){ mal++; continue; }
        long B = e.Bp, C = e.Cp;
        M2 A = { 0, 1, -C, -B };
        printf("      %-23s [[%ld,%ld],[%ld,%ld]]        x' = y, y' = %ld x %+ld y\n",
               t[k].eq, A.a, A.b, A.c, A.d, A.c, A.d);
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
    struct { const char *eq; long B, C; } t[] = {
        { "y'' = -y",           0,  1 },
        { "y'' = y' + y",      -1, -1 },
        { "y'' + 2y' + y = 0",  2,  1 },
        { "y'' - 3y' + 2y = 0",-3,  2 },
    };
    int mal = 0;
    printf("      equação                 traço  det    -tr    Δ = tr²-4det  = B²-4C\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e;
        if(!edo_le(t[k].eq, &e) || e.Bq != 1 || e.Cq != 1){ mal++; continue; }
        long B = e.Bp, C = e.Cp;
        M2 A = { 0, 1, -C, -B };
        long T = tr(A), D = de(A), Dl = dis(A), De = B*B - 4*C;
        printf("      %-23s %+5ld  %+5ld  %+5ld   %+8ld     %+ld\n", t[k].eq, T, D, -T, Dl, De);
        if(-T != B || D != C || Dl != De || B != t[k].B || C != t[k].C) mal++;
    }
    printf("\n");
    ok("B = -traço e C = determinante, e os dois Δ são o MESMO número", mal == 0);
    printf("      O característico de uma 2x2 é λ² - tr·λ + det = 0; o da ED é λ² + Bλ + C = 0.\n");
    printf("      Logo B = -traço e C = determinante, SEM tradução nenhuma. A régua (B,C) dos 44\n");
    printf("      corpos do catálogo é a régua de um sistema de duas equações diferenciais — e o\n");
    printf("      Δ que classifica os corpos é o que decide se a solução cresce, gira ou trava.\n");
}

printf("\n§S3  A órbita da companion: Cayley–Hamilton e A^{a+b}=A^a A^b, em ℤ.\n\n");
{
    struct { const char *nome; M2 A; } t[] = {
        { "oscilador  ", { 0, 1, -1,  0 } },
        { "ouro       ", { 0, 1,  1,  1 } },
        { "raiz dupla ", { 0, 1, -1, -2 } },
        { "acoplado   ", { 1, 2,  3,  4 } },
    };
    int mal_ch = 0, mal_sg = 0;
    printf("      sistema        p(A)=0   A^5[0,0]   A^2 A^3[0,0]\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int ch = ch2(t[k].A);
        if(!ch) mal_ch++;
        M2 P5 = mpot(t[k].A, 5);
        M2 Prod = mmul(mpot(t[k].A, 2), mpot(t[k].A, 3));
        int sg = migual(P5, Prod);
        if(!sg) mal_sg++;
        printf("      %s  %-7s  %+8ld   %+ld\n", t[k].nome, ch ? "sim" : "nao", P5.a, Prod.a);
    }
    printf("\n");
    ok("Cayley-Hamilton: A² − tr·A + det·I = 0 em INTEIROS — a recorrencia que determina"
       " a orbita sem formar e^{At}",
       mal_ch == 0);
    ok("e A^{2+3}=A^2 A^3 em todos: a soma no expoente vira produto, que e' o morfismo"
       " (N,+) -> (matrizes,x), medido em Z e sem uma exponencial",
       mal_sg == 0);
    printf("      A fórmula não é aproximação: p(A)=0 por Cayley-Hamilton, com os coeficientes\n");
    printf("      vindos do característico. O RK4 é que aproximava — e o que resta é a órbita,\n");
    printf("      que não sabe da fórmula fechada e fecha no mesmo semigrupo.\n");
}

printf("\n§S4  O produto de sistemas: o gerador SOMA, a solução MULTIPLICA.\n\n");
{
    /* Do paper (broca-so/papers/equacoes_diferenciais.tex, Parte V): "o gerador SOMA, a solução
     * MULTIPLICA", e "Leibniz É a soma de Kronecker". Aqui mede-se em 4×4, nos TRAÇOS:
     * tr(A⊕B) = n tr B + m tr A  (soma) e  tr(A⊗B) = tr A · tr B  (produto).
     * A série de exp(A⊕B) contra exp A ⊗ exp B era o transporte. */
    long A[2][2] = { {0,1}, {1,1} }, B[2][2] = { {-1,0}, {0,-2} };   /* ouro e decaimento: tr 1 e −3 */
    long S[4][4], K[4][4];
    long trA = A[0][0]+A[1][1], trB = B[0][0]+B[1][1];
    long trS = 0, trK = 0;
    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++){
        S[i][j] = A[i/2][j/2]*(i%2==j%2) + (i/2==j/2)*B[i%2][j%2];
        K[i][j] = A[i/2][j/2] * B[i%2][j%2];
        if(i == j){ trS += S[i][j]; trK += K[i][j]; }
    }
    printf("      A = ouro (tr=%ld),  B = decaimento (tr=%ld)\n", trA, trB);
    printf("      tr(A ⊕ B) = %ld   2·tr A + 2·tr B = %ld\n", trS, 2*trA + 2*trB);
    printf("      tr(A ⊗ B) = %ld   tr A · tr B     = %ld\n\n", trK, trA*trB);
    ok("o gerador SOMA: tr(A ⊕ B) = n tr B + m tr A — a soma de Kronecker, em INTEIROS."
       " A serie de exp(A⊕B) era o transporte; o traco e' a lei, e nao pede uma exponencial",
       trS == 2*trA + 2*trB);
    ok("a solucao MULTIPLICA: tr(A ⊗ B) = tr A · tr B — o produto tensorial, o outro lado"
       " do par. O exp levava a soma ao produto; o traco ja' os separa sem o formar",
       trK == trA*trB);
    printf("      É a assimetria que o paper chama o coração: O GERADOR SOMA, A SOLUÇÃO\n");
    printf("      MULTIPLICA. As taxas somam — o espectro de A ⊕ B são as somas duas a duas dos\n");
    printf("      espectros — e os fluxos multiplicam. O exp é a ponte, o log desfá-la, e é a\n");
    printf("      mesma dualidade que gera os metais: x² de um lado, mx + 1 do outro.\n");
}

printf("\n§S5  E o AMORTECIMENTO move o espectro — os três nomes da física são os três\n");
printf("     nomes do catálogo.\n\n");
{
    /* A entrada (1,1) da companion é −B — o coeficiente de y', o AMORTECIMENTO.
     * Em ℤ: B = 0, 1, 2, 3 com C = 1. O −0,2 em vírgula era o subamortecido; B = 1
     * já é Δ < 0, e B = 3 já é Δ > 0. */
    struct { const char *nome; long B; } t[] = {
        { "sem amortecimento", 0 },
        { "subamortecido    ", 1 },
        { "crítico          ", 2 },
        { "sobreamortecido  ", 3 },
    };
    const long C = 1;
    int mal = 0;
    printf("      sistema             traço   det    Δ        classe\n");
    long Ds[4];
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        long B = t[k].B;
        M2 A = { 0, 1, -C, -B };
        long D = dis(A);
        Ds[k] = D;
        printf("      %s %+6ld %+6ld %+7ld  %s\n", t[k].nome, tr(A), de(A), D,
               D > 0 ? "hiperbólico" : D < 0 ? "elíptico" : "parabólico");
    }
    if(Ds[0] >= 0) mal++;
    if(Ds[1] >= 0) mal++;
    if(Ds[2] != 0) mal++;
    if(Ds[3] <= 0) mal++;
    printf("\n");
    ok("o amortecimento leva de ELÍPTICO a HIPERBÓLICO — muda de classe. Δ = B²−4C em"
       " INTEIROS: 0, 1, 4−4, 9−4. O −0,2 em virgula era o mesmo subamortecido, com mais"
       " uma casa que a classe nao pede",
       mal == 0);
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
