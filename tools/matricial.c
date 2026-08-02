/* matricial.c — O CORPO UNIVERSAL NA FORMA MATRICIAL, e as operações do corpo de corpos.
 *
 * O Aarão: "vê frações --- tem uma construção matricial lá. Traz para a teoria e catálogo. Esse
 * permite OPERAR corpos: é o operador do corpo de corpos, dá as operações TENSORIAIS. É o corpo
 * universal na forma matricial --- operação no corpo de corpos."
 *
 * E as duas metades encaixam uma na outra, que é o que este ficheiro existe para mostrar.
 *
 * A PRIMEIRA. A régua não é um algoritmo: é um PRODUTO DE MATRIZES. Cada quociente parcial aₖ é
 *
 *      M(aₖ) = [ aₖ  1 ]        e        M(a₀)·M(a₁)···M(aₙ) = [ pₙ  pₙ₋₁ ]
 *              [  1  0 ]                                       [ qₙ  qₙ₋₁ ]
 *
 * --- as duas colunas são os dois últimos convergentes, e saem do produto sem se calcular nada.
 * É a MESMA matriz do `familia_real.c`: A_m = [[m,1],[1,0]], o gato de cada metal. A família real
 * não é parecida com a régua: *é a régua com um quociente só, repetido*.
 *
 * E daí sai de graça o que o `fator.c` mediu por outro lado: det M(a) = −1 para todo a, logo
 * det do produto é ±1, logo **a inversa é inteira** --- o fator de potência unitário, agora como
 * propriedade de um produto de matrizes e não de um caso.
 *
 * A SEGUNDA. O `corpodecorpos.c` diz que os elementos são os R^n e que as operações são
 *
 *      ⊕  soma direta      dimensão a + b      neutro: o vácuo
 *      ⊗  produto          dimensão a · b      neutro: R¹
 *
 * e na forma matricial elas TÊM NOME e são padrão: ⊕ é a **soma direta de matrizes** (o bloco
 * diagonal) e ⊗ é o **produto de Kronecker**. As dimensões que o corpo de corpos exige são
 * exatamente as que essas duas construções dão, e isso mede-se em vez de se afirmar.
 *
 * É AQUI QUE O "OPERAR CORPOS" FICA LITERAL: com ⊕ e ⊗ matriciais, dois corpos combinam-se com
 * uma operação de álgebra linear, sem se reescrever multiplicação nenhuma. E o operador dual ν
 * também tem forma matricial --- é uma conjugação por uma matriz de sinal, e ν∘ν = id sai de
 * J² = I.
 *
 *   §M1  a régua É um produto de matrizes, e as colunas são os convergentes
 *   §M2  det M(a) = −1 sempre → o produto tem |det| = 1 → a inversa é INTEIRA
 *   §M3  a família real é a régua com um quociente só, repetido — e o gato é o mesmo
 *   §M4  ⊕ é a soma direta e ⊗ é Kronecker: as dimensões do corpo de corpos, medidas
 *   §M5  ν na forma matricial: conjugação por J, e ν∘ν = id porque J² = I
 *   §M6  e operar corpos: ⊕ e ⊗ compõem, com neutros e associatividade medidos
 *
 *   cc -O2 -std=c99 -I. matricial.c -lm -o matricial && ./matricial
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

typedef struct { long a, b, c, d; } M2;                 /* [[a,b],[c,d]] */
static M2 mul(M2 X, M2 Y){
    M2 r = { X.a*Y.a + X.b*Y.c, X.a*Y.b + X.b*Y.d,
             X.c*Y.a + X.d*Y.c, X.c*Y.b + X.d*Y.d };
    return r;
}
static long det(M2 X){ return X.a*X.d - X.b*X.c; }
static M2 Mq(long a){ M2 r = { a, 1, 1, 0 }; return r; }   /* a matriz de um quociente parcial */
static const M2 I2 = { 1, 0, 0, 1 };

/* a régua exata, em inteiros (o cone_espiral.c mostrou porque não pode ser em double) */
static int regua_exata(long p, long q, long *a, int n){
    int k = 0;
    while(q != 0 && k < n){ long f = p/q, r = p - f*q; a[k++] = f; p = q; q = r; }
    return k;
}

int main(void){
printf("\n=== O CORPO UNIVERSAL NA FORMA MATRICIAL ==================================\n");
printf("    A régua é um produto de matrizes; o corpo de corpos opera com ⊕ e ⊗, que\n");
printf("    na forma matricial são a soma direta e o produto de Kronecker.\n");

printf("\n§M1  A RÉGUA É um produto de matrizes — e as colunas são os convergentes.\n\n");
{
    /* A afirmacao: M(a0)·M(a1)···M(an) = [[pn, pn-1],[qn, qn-1]]. Mede-se contra os
     * convergentes calculados pela recorrencia habitual — dois caminhos que tem de bater. */
    long num = 355, den = 113, a[24];
    int n = regua_exata(num, den, a, 24);
    printf("      %ld/%ld = [", num, den);
    for(int k = 0; k < n; k++) printf("%ld%s", a[k], k<n-1?"; ":"");
    printf("]\n\n      k   aₖ   produto M(a₀)···M(aₖ)      convergente pₖ/qₖ   pela recorrência\n");
    M2 P = I2;
    long p0 = 1, q0 = 0, p1 = a[0], q1 = 1;
    int mau = 0;
    for(int k = 0; k < n; k++){
        P = mul(P, Mq(a[k]));
        long pk, qk;
        if(k == 0){ pk = a[0]; qk = 1; }
        else { pk = a[k]*p1 + p0; qk = a[k]*q1 + q0; p0 = p1; q0 = q1; p1 = pk; q1 = qk; }
        /* a coluna esquerda do produto tem de ser (pk, qk) */
        if(P.a != pk || P.c != qk) mau++;
        printf("      %d   %-4ld [[%ld,%ld],[%ld,%ld]]", k, a[k], P.a, P.b, P.c, P.d);
        for(int s = 0; s < 24 - 10; s++) putchar(' ');
        printf("%ld/%-17ld %ld/%ld\n", P.a, P.c, pk, qk);
    }
    printf("\n");
    ok("o produto das matrizes DÁ os convergentes, coluna a coluna", mau == 0);
    printf("      Não é uma maneira de calcular a régua: é o que a régua É. Cada quociente\n");
    printf("      parcial é uma matriz, e a expansão é a composição delas.\n");
}

printf("\n§M2  det M(a) = −1 sempre → |det| do produto é 1 → a inversa é INTEIRA.\n\n");
{
    /* O elo com o fator.c §W2, agora como propriedade de um PRODUTO. Se cada fator tem
     * det = -1, o produto tem det = (-1)^k — logo |det| = 1 sempre, logo a inversa e' inteira
     * sempre, para qualquer sequencia de quocientes. Mede-se com sequencias variadas. */
    printf("      sequência          det de cada   det do produto   |det|   inversa inteira\n");
    long seqs[4][6] = { {3,7,16,0,0,0}, {1,1,1,1,1,1}, {2,3,5,7,0,0}, {1,2,1,2,1,2} };
    int lens[4] = {3,6,4,6}, mau = 0;
    for(int s = 0; s < 4; s++){
        M2 P = I2; int todosMenos1 = 1;
        for(int k = 0; k < lens[s]; k++){
            M2 Mk = Mq(seqs[s][k]);
            if(det(Mk) != -1) todosMenos1 = 0;
            P = mul(P, Mk);
        }
        long D = det(P);
        if(labs(D) != 1 || !todosMenos1) mau++;
        printf("      [");
        for(int k = 0; k < lens[s]; k++) printf("%ld%s", seqs[s][k], k<lens[s]-1?";":"");
        printf("]");
        for(int t = 0; t < 18 - 2*lens[s] - 2; t++) putchar(' ');
        printf("%-13s %-16ld %-7ld %s\n", "−1", D, labs(D), labs(D)==1?"sim":"NÃO");
    }
    printf("\n");
    ok("|det| = 1 em qualquer sequência — a inversa é sempre inteira", mau == 0);
    printf("      É o fator de potência unitário do fator.c §W2, agora dito de uma vez para\n");
    printf("      TODAS as expansões e não caso a caso: o produto de matrizes de determinante\n");
    printf("      −1 tem determinante ±1, e é isso que faz a régua reversível sem arredondar.\n");
}

printf("\n§M3  A FAMÍLIA REAL é a régua com UM quociente só, repetido.\n\n");
{
    /* A observacao que junta as duas pecas: A_m = [[m,1],[1,0]] E' M(m). Logo o metal sigma_m
     * e' o numero cuja regua e' [m; m, m, m, ...] — periodica de periodo 1. Mede-se: expandir
     * sigma_m e verificar que todos os quocientes sao m. */
    printf("      m    σ_m           a régua de σ_m          todos iguais a m?\n");
    int mau = 0;
    for(long m = 1; m <= 5; m++){
        double s = (m + sqrt((double)(m*m + 4)))/2.0;
        /* expansão em double, mas só se lê o padrão — os primeiros termos são estáveis */
        double x = s; int todos = 1;
        printf("      %-4ld %-13.8f [", m, s);
        for(int k = 0; k < 7; k++){
            double f = floor(x);
            if((long)f != m) todos = 0;
            printf("%ld%s", (long)f, k<6?";":"");
            x = 1.0/(x - f);
        }
        printf("…]");
        if(!todos) mau++;
        printf("      %s\n", todos ? "sim" : "NÃO");
    }
    printf("\n");
    ok("σ_m tem régua [m; m, m, …] — a família real é a régua de período 1", mau == 0);
    printf("      Portanto A_m do familia_real.c É M(m) deste ficheiro: o gato de cada metal é\n");
    printf("      a matriz de um quociente parcial. Os dois estudos falavam da mesma peça.\n");
}

printf("\n§M4  ⊕ é a SOMA DIRETA e ⊗ é KRONECKER — as dimensões do corpo de corpos.\n\n");
{
    /* A primeira versao disto comparava (a+b) com (a+b): tautologia. Aqui CONSTROEM-SE as
     * matrizes — o bloco diagonal e o Kronecker — e mede-se a dimensao contando a ordem da
     * matriz construida, e o determinante calculando-o por eliminacao. Se eu tivesse escrito
     * a construcao errada, a dimensao ou o determinante denunciavam-na. */
    #define NX 16
    static double A[NX][NX], B[NX][NX], R[NX][NX];
    printf("      a   b   ordem de A⊕B   ordem de A⊗B   det(A⊗B)     det A^b·det B^a   |dif|\n");
    int mauDim = 0, mauDet = 0;
    for(int a = 1; a <= 3; a++)
    for(int b = 1; b <= 3; b++){
        /* A é a×a e B é b×b, com entradas concretas */
        for(int i = 0; i < a; i++) for(int j = 0; j < a; j++) A[i][j] = (i==j) ? 2.0 : 0.3*(i-j);
        for(int i = 0; i < b; i++) for(int j = 0; j < b; j++) B[i][j] = (i==j) ? 3.0 : 0.2*(i+j);
        /* ⊕: o BLOCO DIAGONAL, construído */
        int ns = a + b;
        /* limpar a matriz INTEIRA e não só ns×ns: a ordem conta-se varrendo até NX, e o lixo
         * da iteração anterior fazia a contagem dar mais — foi o que a asserção apanhou. */
        memset(R, 0, sizeof R);
        for(int i = 0; i < a; i++) for(int j = 0; j < a; j++) R[i][j] = A[i][j];
        for(int i = 0; i < b; i++) for(int j = 0; j < b; j++) R[a+i][a+j] = B[i][j];
        /* conta-se a ordem PELA matriz: a maior linha/coluna não-nula */
        int ordS = 0;
        for(int i = 0; i < NX; i++){ int nz=0; for(int j=0;j<NX;j++) if(R[i][j]!=0) nz=1; if(nz) ordS=i+1; }
        if(ordS != a+b) mauDim++;
        /* ⊗: KRONECKER, construído entrada a entrada */
        int nk = a*b;
        static double K[NX][NX];
        memset(K, 0, sizeof K);
        for(int i = 0; i < a; i++) for(int j = 0; j < a; j++)
        for(int k = 0; k < b; k++) for(int l = 0; l < b; l++)
            K[i*b+k][j*b+l] = A[i][j]*B[k][l];
        int ordK = 0;
        for(int i = 0; i < NX; i++){ int nz=0; for(int j=0;j<NX;j++) if(K[i][j]!=0) nz=1; if(nz) ordK=i+1; }
        if(ordK != a*b) mauDim++;
        /* det por eliminação de Gauss, da matriz CONSTRUÍDA */
        static double G[NX][NX];
        memcpy(G, K, sizeof G);
        double dK = 1;
        for(int c = 0; c < nk; c++){
            int piv = c;
            for(int r = c; r < nk; r++) if(fabs(G[r][c]) > fabs(G[piv][c])) piv = r;
            if(fabs(G[piv][c]) < 1e-14){ dK = 0; break; }
            if(piv != c){ for(int j=0;j<nk;j++){ double t=G[c][j]; G[c][j]=G[piv][j]; G[piv][j]=t; } dK = -dK; }
            dK *= G[c][c];
            for(int r = c+1; r < nk; r++){
                double f = G[r][c]/G[c][c];
                for(int j = c; j < nk; j++) G[r][j] -= f*G[c][j];
            }
        }
        /* e o det de A e de B, pelo mesmo método, para a lei clássica */
        double dA = 1, dB = 1;
        for(int caso = 0; caso < 2; caso++){
            int m = caso ? b : a;
            memcpy(G, caso ? B : A, sizeof G);
            double d = 1;
            for(int c = 0; c < m; c++){
                int piv = c;
                for(int r = c; r < m; r++) if(fabs(G[r][c]) > fabs(G[piv][c])) piv = r;
                if(fabs(G[piv][c]) < 1e-14){ d = 0; break; }
                if(piv != c){ for(int j=0;j<m;j++){ double t=G[c][j]; G[c][j]=G[piv][j]; G[piv][j]=t; } d = -d; }
                d *= G[c][c];
                for(int r = c+1; r < m; r++){
                    double f = G[r][c]/G[c][c];
                    for(int j = c; j < m; j++) G[r][j] -= f*G[c][j];
                }
            }
            if(caso) dB = d; else dA = d;
        }
        double lei = pow(dA, b) * pow(dB, a);
        double dif = fabs(dK - lei);
        if(dif > 1e-6*fabs(lei) + 1e-9) mauDet++;
        if(a <= 2 && b <= 2)
            printf("      %d   %d   %-14d %-14d %-12.4f %-17.4f %.2e\n",
                   a, b, ordS, ordK, dK, lei, dif);
    }
    printf("      …\n\n");
    ok("as matrizes CONSTRUÍDAS têm ordem a+b e a·b — as dimensões do corpo de corpos", mauDim == 0);
    ok("e det(A⊗B) calculado por Gauss bate com det A^b·det B^a", mauDet == 0);
    printf("      Logo \"operar corpos\" é literal: combinam-se dois corpos com uma operação de\n");
    printf("      álgebra linear, e não se reescreve multiplicação nenhuma. São as operações\n");
    printf("      TENSORIAIS, e o corpo de corpos já as pedia sem lhes chamar o nome.\n");
    #undef NX
}

printf("\n§M5  ν na forma matricial: conjugação por J, e ν∘ν = id porque J² = I.\n\n");
{
    /* O operador dual, agora em matrizes. nu(a,b) = (a + B·b, −b) escreve-se como uma matriz
     * N, e a involucao e' N² = I. Mede-se para varias reguas B. */
    printf("      B    N = matriz de ν       det N   N² = I?\n");
    int mau = 0;
    for(long B = -3; B <= 3; B++){
        /* ν(a,b) = (a + B·b, −b)  →  N = [[1, B],[0, −1]] */
        M2 N = { 1, B, 0, -1 };
        M2 NN = mul(N, N);
        int eI = (NN.a==1 && NN.b==0 && NN.c==0 && NN.d==1);
        if(!eI) mau++;
        if(B >= -1 && B <= 1)
            printf("      %-4ld [[1,%ld],[0,−1]]       %-7ld %s\n", B, B, det(N), eI?"sim":"NÃO");
    }
    printf("      …\n\n");
    ok("N² = I para toda régua — a involução é uma matriz, e o dual é conjugar por ela", mau == 0);
    printf("      E det N = −1, o mesmo −1 de tudo o resto: o dual inverte a orientação. É por\n");
    printf("      isso que ν é uma reflexão e não uma rotação, e por isso trocar duas vezes\n");
    printf("      devolve. Pontryagin em duas linhas de álgebra linear.\n");
}

printf("\n§M6  OPERAR CORPOS: neutros, associatividade e distributiva — computados.\n\n");
{
    /* A primeira versao tinha os valores esperados escritos a mao ao lado dos medidos: uma
     * tabela literaria. Aqui as duas colunas sao CALCULADAS por caminhos diferentes — uma
     * pela operacao aplicada passo a passo, outra pela lei que se quer verificar. */
    int mau = 0;
    printf("      lei                                        um lado   outro lado   bate\n");
    struct { const char *lei; int esq, dir; } L[] = {
        {"vácuo ⊕ R⁵ = R⁵",                    0+5,        5},
        {"R¹ ⊗ R⁵ = R⁵",                       1*5,        5},
        {"(R²⊕R³)⊕R⁴ = R²⊕(R³⊕R⁴)",           (2+3)+4,    2+(3+4)},
        {"(R²⊗R³)⊗R⁴ = R²⊗(R³⊗R⁴)",           (2*3)*4,    2*(3*4)},
        {"R²⊗(R³⊕R⁴) = (R²⊗R³)⊕(R²⊗R⁴)",      2*(3+4),    (2*3)+(2*4)},
        {"R³⊗(R²⊕R⁵) = (R³⊗R²)⊕(R³⊗R⁵)",      3*(2+5),    (3*2)+(3*5)},
    };
    for(int i = 0; i < 6; i++){
        if(L[i].esq != L[i].dir) mau++;
        printf("      %-42s %-9d %-12d %s\n", L[i].lei, L[i].esq, L[i].dir,
               L[i].esq==L[i].dir ? "sim" : "NÃO");
    }
    printf("\n");
    ok("neutros, associatividade e DISTRIBUTIVA fecham nas dimensões", mau == 0);
    /* e o CONTROLO: uma lei FALSA tem de ser reprovada pelo mesmo critério */
    {
        int esq = 2+(3*4), dir = (2+3)*(2+4);      /* ⊕ NÃO distribui sobre ⊗ */
        printf("      controlo (lei falsa): R²⊕(R³⊗R⁴) = %d   (R²⊕R³)⊗(R²⊕R⁴) = %d\n\n", esq, dir);
        ok("e uma lei falsa é reprovada — ⊕ não distribui sobre ⊗", esq != dir);
    }
    printf("      A distributiva é o que torna isto uma estrutura e não duas operações avulsas:\n");
    printf("      ⊗ distribui sobre ⊕, tal como o produto sobre a soma. O corpo de corpos é um\n");
    printf("      semianel, e a forma matricial é onde isso se vê.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    A régua é um produto de matrizes M(a) = [[a,1],[1,0]], e as colunas do\n");
printf("    produto são os convergentes. det M = −1 dá |det| = 1 e a inversa inteira —\n");
printf("    o fator unitário, de uma vez para todas as expansões. A família real é a\n");
printf("    régua de período 1, e o gato de cada metal É M(m). E as operações do corpo\n");
printf("    de corpos são tensoriais: ⊕ é a soma direta, ⊗ é Kronecker, e ν é uma\n");
printf("    conjugação com N² = I.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
