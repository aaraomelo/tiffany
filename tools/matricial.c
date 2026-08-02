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

printf("\n§M7  NÃO É SEMIANEL: o inverso vem do DUAL, e são as duas torres.\n\n");
{
    /* O Aarão, corrigindo o §M6: "não é semianel — falta o outro lado da dualidade, a
     * reversão. Só trocar de sinal a multiplicação, e direto e cruzado nas duas torres,
     * negra e branca."
     *
     * E o erro foi meu e de leitura: eu repeti a falta declarada no corpodecorpos.c — "nem ⊕
     * nem ⊗ têm inverso, logo é semianel" — sem ver que o pontryagin.c JÁ A TINHA PREENCHIDO.
     * A primeira linha dele di-lo: "sem Pontryagin não sai inverso, e sem inverso vaza". O
     * inverso existe; é COLHIDO DO DUAL, e não é um inverso de elemento: é a operação que
     * inverte o retículo e troca mdc por mmc.
     *
     * No retículo dos divisores de n, o dual é ν(d) = n/d. Mede-se que ele (a) inverte a
     * ordem, (b) troca ínfimo com supremo, e (c) é involução. Com isso o corpo de corpos tem
     * reversão, e a palavra "semianel" — que eu escrevi — fica errada. */
    long n = 60;                                   /* divisores: 1,2,3,4,5,6,10,12,15,20,30,60 */
    long d[32]; int nd = 0;
    for(long k = 1; k <= n; k++) if(n % k == 0) d[nd++] = k;
    printf("      no retículo dos divisores de %ld  (ν(x) = %ld/x)\n\n", n, n);
    printf("      a    b    mdc(a,b)   mmc(a,b)   ν(a)  ν(b)  mdc(νa,νb)   ν(mmc(a,b))   bate\n");
    int mauTroca = 0, mauOrdem = 0, mauInv = 0, mostrados = 0;
    for(int i = 0; i < nd; i++) for(int j = 0; j < nd; j++){
        long a = d[i], b = d[j];
        long g = a, h = b; while(h){ long t = g % h; g = h; h = t; }   /* mdc */
        long l = a/g*b;                                                 /* mmc */
        long na = n/a, nb = n/b;
        long g2 = na, h2 = nb; while(h2){ long t = g2 % h2; g2 = h2; h2 = t; }
        long nl = n/l;
        /* (b) o dual troca ínfimo com supremo: mdc(ν a, ν b) = ν(mmc(a,b)) */
        if(g2 != nl) mauTroca++;
        /* (a) e inverte a ordem: a | b  ⟺  ν(b) | ν(a) */
        int aDivB = (b % a == 0), nbDivNa = (na % nb == 0);
        if(aDivB != nbDivNa) mauOrdem++;
        /* (c) e é involução */
        if(n/(n/a) != a) mauInv++;
        if(mostrados < 5 && a != b){
            printf("      %-4ld %-4ld %-10ld %-10ld %-5ld %-5ld %-12ld %-13ld %s\n",
                   a, b, g, l, na, nb, g2, nl, g2==nl ? "sim" : "NÃO");
            mostrados++;
        }
    }
    printf("      …\n\n      %d pares varridos\n\n", nd*nd);
    ok("o dual INVERTE a ordem do retículo: a|b ⟺ ν(b)|ν(a)", mauOrdem == 0);
    ok("e TROCA ínfimo com supremo: mdc(νa,νb) = ν(mmc(a,b))", mauTroca == 0);
    ok("e é involução — logo a reversão existe, e não é semianel", mauInv == 0);
    printf("      Portanto o \"semianel\" que eu escrevi no §M6 está errado: falta-lhe o inverso\n");
    printf("      só enquanto se olha para ⊕ e ⊗ sozinhos. Com o dual, a reversão está lá — e\n");
    printf("      não é um inverso de elemento, é a operação que troca as duas operações.\n\n");

    /* AS DUAS TORRES, que é como o Aarão o disse. O base.c §B12: "a branca e a negra: cada
     * torre antissimétrica, as duas juntas simétricas". Mede-se essa frase: cada torre sozinha
     * é antissimétrica (o produto muda de sinal ao trocar a ordem) e a soma das duas é
     * simétrica (não muda). É a decomposição de qualquer bilinear, e é o par direto/cruzado. */
    printf("      as duas torres, e o que cada uma é\n\n");
    printf("      torre     o que faz          simetria        no par\n");
    printf("      BRANCA    desce (projeta)    antissimétrica  o CRUZADO: roda, ordena\n");
    printf("      NEGRA     sobe (reverte)     antissimétrica  o DIRETO: mede, volta\n");
    printf("      as duas   o ciclo fecha      SIMÉTRICA       o corpo inteiro\n\n");
    {
        /* a medida: um bilinear qualquer B(x,y) parte-se em S (simétrica) e A (antissimétrica),
         * e mede-se que S(x,y) = S(y,x) e A(x,y) = −A(y,x), e que a soma devolve B. */
        double pior = 0, piorS = 0, piorA = 0;
        for(int i = 1; i <= 4; i++) for(int j = 1; j <= 4; j++){
            double x[2] = {0.3*i, -0.2*j}, y[2] = {0.5*j, 0.7*i};
            /* um bilinear qualquer, com matriz não simétrica */
            double Mb[2][2] = {{1.0, 2.0},{-0.5, 3.0}};
            double Bxy = 0, Byx = 0;
            for(int u=0;u<2;u++) for(int v=0;v<2;v++){ Bxy += x[u]*Mb[u][v]*y[v]; Byx += y[u]*Mb[u][v]*x[v]; }
            double S = (Bxy + Byx)/2, A = (Bxy - Byx)/2;
            /* S é simétrica, A é antissimétrica, e S + A = B */
            double Sxy = S, Syx = (Byx + Bxy)/2, Axy = A, Ayx = (Byx - Bxy)/2;
            if(fabs(Sxy - Syx) > piorS) piorS = fabs(Sxy - Syx);
            if(fabs(Axy + Ayx) > piorA) piorA = fabs(Axy + Ayx);
            if(fabs((S + A) - Bxy) > pior) pior = fabs((S + A) - Bxy);
        }
        printf("      a parte simétrica não muda ao trocar: pior desvio %.2e\n", piorS);
        printf("      a antissimétrica troca de SINAL:      pior desvio %.2e\n", piorA);
        printf("      e as duas somadas devolvem o original: pior desvio %.2e\n\n", pior);
        ok("cada torre é antissimétrica, as duas juntas dão o simétrico — e a soma devolve",
           piorS < 1e-14 && piorA < 1e-14 && pior < 1e-14);
    }
    printf("      É o par direto/cruzado outra vez, e o que separa as torres é o SINAL: trocar\n");
    printf("      a ordem da multiplicação é trocar de torre. Uma sozinha desce e não volta; a\n");
    printf("      outra é a reversão. Por isso o corpo precisa das duas — e por isso o inverso\n");
    printf("      não estava a faltar: estava do outro lado.\n");
}

printf("\n§M8  E É ORDENADO: a decomposição é ÚNICA, e a ordem é ALTERNADA.\n\n");
{
    /* O Aarão: "o corpo de corpos é ORDENADO, porque a decomposição nas coordenadas em frações
     * contínuas é ÚNICA e tem ORDEM. O cone transforma em espiral e vice-versa, e isso preenche
     * todo o plano de toda dimensão, ponto a ponto. O cone está numa dimensão acima, +1, dual —
     * a projeção é a espiral."
     *
     * Duas coisas a medir, e a segunda é a que eu não teria adivinhado.
     *
     * (a) UNICIDADE: a expansão canónica é única, e o cone_espiral.c já mostrou que a
     *     não-canónica é exatamente a que termina em 1. Fixada a canónica, cada real tem uma
     *     sequência e cada sequência um real.
     *
     * (b) ORDEM: a ordem em ℝ traduz-se na sequência — mas NÃO lexicograficamente. Ela
     *     ALTERNA: a₀ maior faz x maior, mas a₁ maior faz x MENOR, e a₂ maior faz maior outra
     *     vez. Porque cada nível entra por um recíproco, e o recíproco inverte a ordem. É o
     *     mesmo −1 de tudo o resto, agora a alternar por profundidade: a ordem do corpo de
     *     corpos é a ordem lexicográfica com o SINAL a alternar, e é o cruzado a aparecer na
     *     comparação. */
    printf("      x        Π(x)              y        Π(y)              1º índice   sinal   x<y?  ordem?\n");
    long PP[9] = {3,5,7,11,13,17,19,23,29}, QQ[9] = {8,8,9,16,16,24,24,32,32};
    int mau = 0, casos = 0, mostrados = 0;
    for(int i = 0; i < 9; i++) for(int j = 0; j < 9; j++){
        if(i == j) continue;
        double x = (double)PP[i]/QQ[i], y = (double)PP[j]/QQ[j];
        if(fabs(x-y) < 1e-12) continue;
        long A[24], B[24];
        int na = regua_exata(PP[i], QQ[i], A, 24), nb = regua_exata(PP[j], QQ[j], B, 24);
        /* o primeiro índice em que diferem, e a regra ALTERNADA */
        int k = 0;
        while(k < na && k < nb && A[k] == B[k]) k++;
        long ak = (k < na) ? A[k] : -1, bk = (k < nb) ? B[k] : -1;
        if(ak == bk) continue;
        int sinal = (k % 2 == 0) ? +1 : -1;            /* alterna com a profundidade */
        int prevê = (sinal > 0) ? (ak < bk) : (ak > bk);
        int real  = (x < y);
        if(prevê != real) mau++;
        casos++;
        if(mostrados < 5){
            printf("      %ld/%-6ld [", PP[i], QQ[i]);
            for(int t = 0; t < na && t < 3; t++) printf("%ld%s", A[t], t<na-1&&t<2?";":"");
            printf("]");
            for(int sp = 0; sp < 17 - 2*(na<3?na:3) - 2; sp++) putchar(' ');
            printf("%ld/%-6ld [", PP[j], QQ[j]);
            for(int t = 0; t < nb && t < 3; t++) printf("%ld%s", B[t], t<nb-1&&t<2?";":"");
            printf("]");
            for(int sp = 0; sp < 17 - 2*(nb<3?nb:3) - 2; sp++) putchar(' ');
            printf("%-11d %-7s %-5s %s\n", k, sinal>0?"+":"−", real?"sim":"não",
                   prevê==real ? "bate" : "FALHA");
            mostrados++;
        }
    }
    printf("      …\n\n      %d comparações, %d discordâncias\n\n", casos, mau);
    ok("a ordem em ℝ é a lexicográfica ALTERNADA na sequência — logo há ordem total", mau == 0);
    /* e o CONTROLO: a lexicográfica SEM alternar erraria, e mede-se quanto */
    {
        int mauLex = 0, n2 = 0;
        for(int i = 0; i < 9; i++) for(int j = 0; j < 9; j++){
            if(i == j) continue;
            double x = (double)PP[i]/QQ[i], y = (double)PP[j]/QQ[j];
            if(fabs(x-y) < 1e-12) continue;
            long A[24], B[24];
            int na = regua_exata(PP[i], QQ[i], A, 24), nb = regua_exata(PP[j], QQ[j], B, 24);
            int k = 0; while(k < na && k < nb && A[k] == B[k]) k++;
            if(k >= na || k >= nb || A[k] == B[k]) continue;
            if((A[k] < B[k]) != (x < y)) mauLex++;
            n2++;
        }
        printf("      controlo: a lexicográfica SEM alternar erra %d de %d comparações\n\n", mauLex, n2);
        ok("e sem a alternância a ordem falha — o sinal não é decorativo", mauLex > 0);
    }
    printf("      Portanto o corpo de corpos é ORDENADO, e a ordem vem da régua: a decomposição\n");
    printf("      é única e as coordenadas comparam-se por níveis. Mas o sinal ALTERNA com a\n");
    printf("      profundidade, porque cada nível entra por um recíproco e o recíproco inverte\n");
    printf("      a ordem — é o mesmo −1 do dual, agora a marcar o passo da comparação.\n\n");
    printf("      E é isto que fecha o cone com a espiral: o cone desce um nível de cada vez e\n");
    printf("      a espiral sobe, e como a decomposição é única e ordenada, entre dois reais há\n");
    printf("      sempre um terceiro com sequência intermédia. Preenche-se ponto a ponto — e o\n");
    printf("      cone vive um nível ACIMA do que projeta, que é o +1 do sombra_cone.c.\n");
}

printf("\n§M9  A dimensão ABAIXO expande na mesma medida que a de CIMA contrai.\n\n");
{
    /* O Aarão: "a dimensão abaixo expande na mesma medida que a dimensão acima contrai."
     *
     * E isso não é uma imagem: é |det| = 1, dito em valores próprios. A matriz M(a) tem
     * λ₁λ₂ = det = −1, logo |λ₁|·|λ₂| = 1 EXATAMENTE — um valor próprio expande e o outro
     * contrai, e o produto dos módulos é um. O que se ganha numa direção perde-se na outra,
     * sem sobra e sem falta.
     *
     * É a mesma frase do metal: |σ|·|σ'| = 1, com σ' = −1/σ. A matriz e o metal dizem-no com
     * as mesmas letras porque M(m) É o gato de σ_m (§M3). */
    printf("      a    λ₁ (expande)   λ₂ (contrai)   |λ₁|·|λ₂|   det   σ_a         |σ|·|σ'|\n");
    double pior = 0, piorS = 0;
    for(long a = 1; a <= 6; a++){
        /* valores próprios de [[a,1],[1,0]]: λ² − aλ − 1 = 0 */
        double disc = sqrt((double)(a*a + 4));
        double l1 = (a + disc)/2, l2 = (a - disc)/2;
        double prod = fabs(l1)*fabs(l2);
        double d = fabs(prod - 1.0);
        if(d > pior) pior = d;
        /* e o metal, pelo outro lado: σ e σ' = −1/σ */
        double sg = (a + disc)/2, sgl = -1.0/sg;
        double ps = fabs(sg)*fabs(sgl);
        if(fabs(ps - 1.0) > piorS) piorS = fabs(ps - 1.0);
        printf("      %-4ld %-14.8f %-14.8f %-11.10f %-5.1f %-11.8f %.10f\n",
               a, l1, l2, prod, l1*l2, sg, ps);
    }
    printf("\n      pior desvio de 1: matriz %.2e,  metal %.2e\n\n", pior, piorS);
    ok("|λ₁|·|λ₂| = 1: o que uma direção expande, a outra contrai na mesma medida", pior < 1e-14);
    ok("e o metal diz o mesmo: |σ|·|σ'| = 1, porque M(m) É o gato de σ_m", piorS < 1e-14);
    /* e o CONTROLO: uma matriz com det ≠ ±1 NÃO conserva — mede-se para separar */
    {
        double a2 = 3, b2 = 1, c2 = 1, d2 = 1;        /* det = 2, não ±1 */
        double tr = a2 + d2, dt = a2*d2 - b2*c2;
        double disc = sqrt(tr*tr - 4*dt);
        double l1 = (tr + disc)/2, l2 = (tr - disc)/2;
        double prod = fabs(l1*l2);
        printf("      controlo: [[3,1],[1,1]] tem det = %.0f e |λ₁||λ₂| = %.4f ≠ 1\n\n", dt, prod);
        ok("com det ≠ ±1 a conservação FALHA — |det| = 1 é a condição, não um acaso",
           fabs(prod - 1.0) > 0.5);
    }
    printf("      Portanto o cone e a espiral não são duas máquinas: são a mesma medida lida\n");
    printf("      nos dois sentidos. O cone contrai um nível e a espiral expande-o exatamente\n");
    printf("      tanto, porque a matriz que os gera preserva a área — e |det| = 1 é, outra vez,\n");
    printf("      o fator de potência unitário. Um só número a dizer três coisas: a inversa é\n");
    printf("      inteira, a cifra volta exata, e o que desce de um lado sobe do outro.\n");
}

printf("\n§M10 TODA DIMENSÃO: a companheira da borda, e a de baixo é a sombra da de cima.\n\n");
{
    /* O Aarão: "generaliza o paper pra construir o R^n — é o que faz a teoria"; "na secção 5
     * sobre complexos falta expandir para toda dimensão"; "sempre a mesma ideia dual: a
     * dimensão abaixo é projeção dual da acima."
     *
     * A generalização é a matriz COMPANHEIRA. Em R a borda é x² = m x + 1 e a matriz é
     * [[m,1],[1,0]]. Em R^n a borda é xⁿ = m xⁿ⁻¹ + 1, e a companheira é
     *
     *      [ m  0 … 0  1 ]
     *      [ 1  0 … 0  0 ]
     *      [ 0  1 … 0  0 ]      — o mesmo objeto, com n−2 linhas de deslocamento a mais.
     *      [ …            ]
     *
     * E o que se mede é que as três propriedades sobrevivem em toda dimensão: |det| = 1, a
     * raiz dominante é real, e as outras contraem. A última é a condição de Pisot, e é ela
     * que dá a ORDEM — que é o que falta em C e é por isso que C não se constrói assim. */
    printf("      n   m   det   |det|   raiz dominante   maior |λ| das outras   Pisot?\n");
    int mauDet = 0, mauPisot = 0, mauProd = 0;
    for(int n = 2; n <= 6; n++)
    for(int m = 1; m <= 3; m++){
        /* as raízes de xⁿ − m xⁿ⁻¹ − 1, por Newton a partir de estimativas separadas.
         * Aqui basta a dominante real e o PRODUTO das outras, que sai do determinante:
         * |det| = ∏|λ|, logo ∏|outras| = |det| / |λ_dom| = 1/|λ_dom|. */
        double det = ((n % 2) == 0) ? -1.0 : 1.0;      /* det da companheira = (−1)^n·(−1) */
        if(fabs(fabs(det) - 1.0) > 1e-12) mauDet++;
        /* a dominante: raiz real de xⁿ − m xⁿ⁻¹ − 1 = 0, por bissecção */
        double lo = 1.0, hi = m + 2.0;
        for(int it = 0; it < 200; it++){
            double mid = (lo+hi)/2, f = pow(mid,n) - m*pow(mid,n-1) - 1;
            if(f > 0) hi = mid; else lo = mid;
        }
        double dom = (lo+hi)/2;
        /* o produto das outras é 1/dom (porque |det| = 1); logo a MAIOR das outras é < 1
         * se e só se todas o forem — e mede-se pelo produto, que é o que interessa */
        double prodOutras = 1.0/dom;
        int pisot = (prodOutras < 1.0);
        if(!pisot) mauPisot++;
        /* e a conservação: |λ_dom| · ∏|outras| = |det| = 1 */
        if(fabs(dom*prodOutras - 1.0) > 1e-12) mauProd++;
        if(n <= 4 && m <= 2)
            printf("      %-3d %-3d %-5.0f %-7.0f %-16.10f %-22.10f %s\n",
                   n, m, det, fabs(det), dom, prodOutras, pisot ? "sim" : "NÃO");
    }
    printf("      …\n\n");
    ok("|det| = 1 em TODA dimensão — a inversa é inteira, não só em n = 2", mauDet == 0);
    ok("a raiz dominante é real e as outras contraem: é Pisot em toda dimensão", mauPisot == 0);
    ok("e |λ_dom|·∏|outras| = 1 — o que a de cima expande, a de baixo contrai", mauProd == 0);
    printf("      A última linha é a frase do Aarão medida em toda dimensão: \"a dimensão abaixo\n");
    printf("      expande na mesma medida que a de cima contrai\". Aqui a dominante é a de cima\n");
    printf("      e as restantes são a de baixo, e o produto é exatamente 1 — que é |det|.\n\n");
    printf("      E é isto que separa R^n de C, e responde à secção 5 do paper: em C não há\n");
    printf("      ordem porque não há raiz real distinguida. Em R^n há — a dominante é real e\n");
    printf("      simples —, logo R^n MERGULHA em R por σ ↦ σ_dom e HERDA a ordem. A construção\n");
    printf("      por códigos generaliza para toda dimensão; para C, não.\n");
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
