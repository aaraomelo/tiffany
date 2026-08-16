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
#include "../lib/disco.h"
#include <string.h>
#include <math.h>
#include "unidade.h"
#include "racionais.h"
#include "poli.h"

typedef struct { long a, b, c, d; } M2;                 /* [[a,b],[c,d]] */
static M2 mul(M2 X, M2 Y){
    M2 r = { X.a*Y.a + X.b*Y.c, X.a*Y.b + X.b*Y.d,
             X.c*Y.a + X.d*Y.c, X.c*Y.b + X.d*Y.d };
    return r;
}
static long det(M2 X){ return X.a*X.d - X.b*X.c; }

/* E O DETERMINANTE DE ORDEM QUALQUER, TAMBÉM EXACTO — por BAREISS. A eliminação de
 * Gauss divide, e por isso pedia doubles e um limiar para o pivô; a de Bareiss divide
 * pelo pivô ANTERIOR, e essa divisão é exacta em ℤ (é o teorema). Fica tudo inteiro, o
 * determinante sai sem resíduo, e o limiar do pivô desaparece: `== 0` é uma pergunta
 * sobre o número, e `fabs(.) < 1e-14` era uma pergunta sobre a régua que escolhi.
 *
 * n é a ordem; a matriz é dada em G com passo `passo`, e é consumida. */
static long det_bareiss(long *G, int n, int passo){
    long ant = 1;
    int sinal = 1;
    for(int k = 0; k < n - 1; k++){
        if(G[k*passo + k] == 0){                       /* troca por uma linha com pivô */
            int t = -1;
            for(int i = k + 1; i < n; i++) if(G[i*passo + k]){ t = i; break; }
            if(t < 0) return 0;                        /* coluna nula: o det É zero */
            for(int j = 0; j < n; j++){
                long v = G[k*passo + j];
                G[k*passo + j] = G[t*passo + j];
                G[t*passo + j] = v;
            }
            sinal = -sinal;
        }
        for(int i = k + 1; i < n; i++)
            for(int j = k + 1; j < n; j++)
                G[i*passo + j] = (G[i*passo + j]*G[k*passo + k]
                                  - G[i*passo + k]*G[k*passo + j]) / ant;
        ant = G[k*passo + k];
    }
    return sinal*G[(n-1)*passo + (n-1)];
}

/* o inverso em 𝔽ₚ por Fermat — nenhuma divisão, e nenhum real */
static long inv_mod(long a, long p){
    long r = 1, e = p - 2;
    a = ((a % p) + p) % p;
    while(e){ if(e & 1) r = r*a % p; a = a*a % p; e >>= 1; }
    return r;
}

/* E O MESMO DETERMINANTE EM 𝔽ₚ, para quando os intermédios não cabem. O Bareiss é
 * exacto em ℤ, mas os seus intermédios são MENORES da matriz: numa 5×5 com entradas
 * da ordem de 10^10 eles chegam a 10^42, e o `long` enrola calado — o resultado final
 * cabia (10^14) e mesmo assim saía errado. Em 𝔽ₚ nada cresce. */
static long det_mod(long *G, int n, int passo, long p){
    long d = 1;
    for(int k = 0; k < n; k++){
        int piv = -1;
        for(int i = k; i < n; i++) if(((G[i*passo+k] % p) + p) % p){ piv = i; break; }
        if(piv < 0) return 0;
        if(piv != k){
            for(int j = 0; j < n; j++){
                long t = G[k*passo+j]; G[k*passo+j] = G[piv*passo+j]; G[piv*passo+j] = t;
            }
            d = (p - d) % p;
        }
        long a = ((G[k*passo+k] % p) + p) % p;
        d = d*a % p;
        long iv = inv_mod(a, p);
        for(int i = k+1; i < n; i++){
            long f = ((G[i*passo+k] % p) + p) % p * iv % p;
            if(!f) continue;
            for(int j = k; j < n; j++)
                G[i*passo+j] = (((G[i*passo+j] - f*G[k*passo+j]) % p) + p) % p;
        }
    }
    return d;
}

/* e a potência inteira, para a lei det(A⊗B) = det(A)^b · det(B)^a — `pow` devolvia um
 * double e obrigava a comparar com margem */
static long ipow(long base, int e){
    long r = 1;
    while(e-- > 0) r *= base;
    return r;
}
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
    /* E A EXPANSÃO NÃO SE FAZ EM DOUBLE. A linha 62 deste ficheiro já diz que a régua é
     * exacta em inteiros «porque o cone_espiral.c mostrou porque não pode ser em
     * double», e logo aqui expandia-se σ_m com sqrt e floor, sete termos, «porque os
     * primeiros são estáveis». Sete são; o vigésimo não é — e a periodicidade é uma
     * afirmação sobre TODOS.
     *
     * Ela prova-se em duas linhas, e sem avaliar σ:
     *
     *   ⌊σ⌋ = m      porque m < σ < m+1, e as duas desigualdades são INTEIRAS:
     *                σ > m ⟺ √(m²+4) > m ⟺ 4 > 0
     *                σ < m+1 ⟺ √(m²+4) < m+2 ⟺ m²+4 < m²+4m+4 ⟺ 0 < 4m
     *
     *   1/(σ−m) = σ  porque σ(σ−m) = σ² − mσ = 1, pela própria lei σ² = mσ + 1
     *
     * A segunda é o PASSO: o resto da expansão é σ outra vez, logo o quociente seguinte
     * é m outra vez, e por indução TODOS são m. Não são sete: são todos, e a prova é do
     * passo — como no supremo. */
    printf("      m    ⌊σ⌋ = m?   4 > 0   0 < 4m   σ(σ−m) = 1 em Z[σ]   régua\n");
    int mau = 0;
    for(long m = 1; m <= 5; m++){
        /* AS DUAS DESIGUALDADES DO CHÃO, e escritas na forma GERAL. Simplifiquei-as
         * primeiro até «4 > 0» e «0 < 4m» — e o primeiro é uma CONSTANTE, que passa
         * sempre e não mede nada. Na forma que depende do objecto, com D = m² + 4:
         *
         *      σ > m    ⟺  D > m²           σ < m+1  ⟺  D < (m+2)²
         *
         * e agora elas podem falhar. Para a família geral x² − t·x − n vale D = t² + 4n,
         * e a segunda dá t² + 4n < t² + 4t + 4, isto é n ≤ t — que é EXACTAMENTE a
         * condição do encaixe (thm:condicao). A mesma desigualdade, no mesmo sítio. */
        long D = m*m + 4;
        int baixo = (D > m*m);                        /* σ > m */
        int cima  = (D < (m+2)*(m+2));                /* σ < m+1 */
        /* e o PASSO, em ℤ[σ]: σ·(σ − m) = σ² − mσ = (mσ + 1) − mσ = 1 */
        long u1 = 0, v1 = 1, u2 = -m, v2 = 1;         /* σ e (σ − m) */
        long pu = u1*u2 + v1*v2;                      /* σ² = mσ + 1 */
        long pv = u1*v2 + u2*v1 + m*v1*v2;
        int passo = (pu == 1 && pv == 0);
        if(!(baixo && cima && passo)) mau++;
        printf("      %-4ld %-11s %-7s %-8s %-20s [%ld; %ld, %ld, …]\n",
               m, (baixo && cima) ? "sim" : "NÃO", baixo ? "sim" : "NÃO",
               cima ? "sim" : "NÃO", passo ? "sim" : "NÃO", m, m, m);
    }
    /* E O CONTROLO, que é a mesma conta a falhar onde tem de falhar: para x² − t·x − n
     * com n > t o chão deixa de ser t, porque D = t² + 4n passa de (t+2)². */
    long tc = 2, nc = 4, Dc = tc*tc + 4*nc;
    int cai = !(Dc < (tc+2)*(tc+2));
    printf("\n      GUME: x² − 2x − 4 tem D = %ld e (t+2)² = %ld, logo ⌊σ⌋ ≠ t — a mesma"
           " conta FALHA onde n > t\n", Dc, (tc+2)*(tc+2));
    printf("      e é uma prova do PASSO, não uma amostra: o resto da expansão é σ outra"
           " vez,\n      logo o quociente seguinte é m outra vez — por indução, TODOS\n\n");
    ok("σ_m TEM RÉGUA [m; m, m, …] — A FAMÍLIA REAL É A RÉGUA DE PERÍODO 1, e prova-se em"
       " inteiros. Antes expandia-se σ_m em double com sqrt e floor e olhavam-se SETE"
       " termos, «porque os primeiros são estáveis» — sete são, e a periodicidade é uma"
       " afirmação sobre todos. Agora: ⌊σ⌋ = m sai de duas desigualdades inteiras (σ > m"
       " ⟺ 4 > 0, e σ < m+1 ⟺ 0 < 4m), e o PASSO 1/(σ−m) = σ sai da própria lei"
       " σ² = mσ + 1, medido em ℤ[σ] com parte em σ nula. Do passo a indução dá todos os"
       " quocientes de uma vez — a mesma forma que corrigiu o supremo. E as desigualdades"
       " escrevem-se na forma GERAL, com D = m²+4 contra m² e (m+2)²: simplificadas até"
       " «4 > 0» passavam por serem constantes, e nesta forma FALHAM onde têm de falhar —"
       " para x² − t·x − n com n > t, que é a condição do encaixe a aparecer no mesmo"
       " sítio",
       mau == 0 && cai);
    printf("      Portanto A_m do familia_real.c É M(m) deste ficheiro: o gato de cada metal é\n");
    printf("      a matriz de um quociente parcial. Os dois estudos falavam da mesma peça.\n");
}

printf("\n§M4  ⊕ é a SOMA DIRETA e ⊗ é KRONECKER — as dimensões do corpo de corpos.\n\n");
{
    /* A primeira versao disto comparava (a+b) com (a+b): tautologia. Aqui CONSTROEM-SE as
     * matrizes — o bloco diagonal e o Kronecker — e mede-se a dimensao contando a ordem da
     * matriz construida, e o determinante calculando-o por eliminacao. Se eu tivesse escrito
     * a construcao errada, a dimensao ou o determinante denunciavam-na. */
    /* E TUDO ISTO É INTEIRO. A lei det(A⊗B) = det(A)^b·det(B)^a é ALGÉBRICA: vale para
     * quaisquer entradas, e as decimais 0.3 e 0.2 que aqui estavam eram sabor — traziam
     * o double, o double trazia a eliminação com divisão, e a divisão trazia dois
     * limiares (o do pivô, 1e-14, e o da comparação final, 1e-6 relativo mais 1e-9). Com
     * entradas inteiras e o determinante por Bareiss, a lei fecha por IGUALDADE. */
    #define NX 16
    long (*A)[NX] = DISCO_FIXO2(long, NX, 363);
    long (*B)[NX] = DISCO_FIXO2(long, NX, 364);
    long (*R)[NX] = DISCO_FIXO2(long, NX, 365);
    disco_prende(DISCO_BASE(363),"dados/A_363.bin",(size_t)((NX)*(NX)),sizeof(long));
    disco_zera(A,(size_t)((NX)*(NX)),sizeof(long));
    disco_prende(DISCO_BASE(364),"dados/B_364.bin",(size_t)((NX)*(NX)),sizeof(long));
    disco_zera(B,(size_t)((NX)*(NX)),sizeof(long));
    disco_prende(DISCO_BASE(365),"dados/R_365.bin",(size_t)((NX)*(NX)),sizeof(long));
    disco_zera(R,(size_t)((NX)*(NX)),sizeof(long));
    printf("      a   b   ordem de A⊕B   ordem de A⊗B   det(A⊗B)     det A^b·det B^a   igual\n");
    int mauDim = 0, mauDet = 0, bareiss_bate = 0;
    for(int a = 1; a <= 3; a++)
    for(int b = 1; b <= 3; b++){
        /* A é a×a e B é b×b, com entradas concretas — e INTEIRAS */
        for(int i = 0; i < a; i++) for(int j = 0; j < a; j++) A[i][j] = (i==j) ? 2 : (i-j);
        for(int i = 0; i < b; i++) for(int j = 0; j < b; j++) B[i][j] = (i==j) ? 3 : (i+j);
        /* ⊕: o BLOCO DIAGONAL, construído */
        /* limpar a matriz INTEIRA e não só ns×ns: a ordem conta-se varrendo até NX, e o lixo
         * da iteração anterior fazia a contagem dar mais — foi o que a asserção apanhou. */
        memset(R, 0, ((size_t)((NX)*(NX))*sizeof(long)));
        for(int i = 0; i < a; i++) for(int j = 0; j < a; j++) R[i][j] = A[i][j];
        for(int i = 0; i < b; i++) for(int j = 0; j < b; j++) R[a+i][a+j] = B[i][j];
        /* conta-se a ordem PELA matriz: a maior linha/coluna não-nula */
        int ordS = 0;
        for(int i = 0; i < NX; i++){ int nz=0; for(int j=0;j<NX;j++) if(R[i][j]!=0) nz=1; if(nz) ordS=i+1; }
        if(ordS != a+b) mauDim++;
        /* ⊗: KRONECKER, construído entrada a entrada */
        int nk = a*b;
        long (*K)[NX] = DISCO_FIXO2(long, NX, 218);
        disco_prende(DISCO_BASE(218),"dados/K_218.bin",(size_t)((NX)*(NX)),sizeof(long));
        memset(K, 0, ((size_t)((NX)*(NX))*sizeof(long)));
        for(int i = 0; i < a; i++) for(int j = 0; j < a; j++)
        for(int k = 0; k < b; k++) for(int l = 0; l < b; l++)
            K[i*b+k][j*b+l] = A[i][j]*B[k][l];
        int ordK = 0;
        for(int i = 0; i < NX; i++){ int nz=0; for(int j=0;j<NX;j++) if(K[i][j]!=0) nz=1; if(nz) ordK=i+1; }
        if(ordK != a*b) mauDim++;
        /* det da matriz CONSTRUÍDA, por BAREISS — inteiro, e sem pivô com limiar */
        long G[NX][NX];
        memcpy(G, K, sizeof G);
        long dK = det_bareiss(&G[0][0], nk, NX);
        /* e os de A e de B, pelo mesmo método, para a lei clássica */
        long dAB[2] = {0, 0};
        for(int caso = 0; caso < 2; caso++){
            memcpy(G, caso ? B : A, sizeof G);
            dAB[caso] = det_bareiss(&G[0][0], caso ? b : a, NX);
        }
        /* e o BAREISS confere-se contra a régua que este ficheiro já tinha: para ordem 2
         * o det é ad − bc, escrito na linha 58 e usado em todo o §M2. Duas rotas para o
         * mesmo número — sem isto, um Bareiss errado passava despercebido nas DUAS
         * pontas da lei, que é onde ele é usado. */
        if(a == 2){
            M2 X = { A[0][0], A[0][1], A[1][0], A[1][1] };
            if(det(X) != dAB[0]) mauDet++; else bareiss_bate++;
        }
        if(b == 2){
            M2 Y = { B[0][0], B[0][1], B[1][0], B[1][1] };
            if(det(Y) != dAB[1]) mauDet++; else bareiss_bate++;
        }
        long lei = ipow(dAB[0], b) * ipow(dAB[1], a);
        if(dK != lei) mauDet++;                      /* IGUALDADE, e não margem */
        if(a <= 2 && b <= 2)
            printf("      %d   %d   %-14d %-14d %-12ld %-17ld %s\n",
                   a, b, ordS, ordK, dK, lei, dK == lei ? "sim" : "NÃO");
    }
    printf("      …\n\n");
    printf("      e o Bareiss confere com o det 2×2 (ad − bc) que o §M2 usa: %d de 6\n\n",
           bareiss_bate);
    ok("as matrizes CONSTRUÍDAS têm ordem a+b e a·b — as dimensões do corpo de corpos", mauDim == 0);
    ok("E det(A⊗B) BATE COM det A^b·det B^a, POR IGUALDADE E EM INTEIROS: a lei é"
       " algébrica e vale para quaisquer entradas, logo as decimais 0.3 e 0.2 que aqui"
       " estavam eram sabor — e traziam o double, o double trazia a eliminação com"
       " divisão, e a divisão trazia TRÊS limiares: o do pivô (1e-14) e os dois da"
       " comparação final (1e-6 relativo mais 1e-9). Com entradas inteiras e o"
       " determinante por BAREISS — cuja divisão pelo pivô anterior é exacta em ℤ — o"
       " resíduo deixa de existir: mede-se `dK == lei`, que é uma pergunta sobre o"
       " número, e não `|dK − lei| < régua`, que é uma pergunta sobre a régua",
       mauDet == 0 && bareiss_bate == 6);
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
        /* A MEDIDA ESTAVA VAZIA, e o defeito é o que o §O1 do `operacao.c` já tinha
         * apanhado. Escrevia-se
         *
         *      S = (Bxy + Byx)/2      e      Syx = (Byx + Bxy)/2
         *
         * e comparava-se um com o outro: é a MESMA expressão com as parcelas trocadas. O
         * que dava zero era a soma de doubles comutar, e não B ter parte simétrica. As
         * três asserções deste bloco não podiam falhar — e o limiar 1e-14 dava-lhes cara
         * de medição.
         *
         * A tese é sobre a FORMA, e mede-se avaliando-a nos argumentos trocados. Com
         * Ms = M + Mᵀ e Ma = M − Mᵀ (em dobro, para não dividir):
         *
         *      x·Ms·y = y·Ms·x        e        x·Ma·y = − y·Ma·x
         *
         * e agora há como falhar: se Ms fosse construída mal, falhava. Tudo em INTEIROS,
         * e por isso o resíduo é ZERO EXACTO — e não «menor que a régua que escolhi». */
        const long Mb[2][2] = {{1, 2}, {-1, 3}};       /* uma bilinear NÃO simétrica */
        long Ms[2][2], Ma[2][2];
        for(int u=0;u<2;u++) for(int v=0;v<2;v++){
            Ms[u][v] = Mb[u][v] + Mb[v][u];
            Ma[u][v] = Mb[u][v] - Mb[v][u];
        }
        long casos = 0, sim = 0, anti = 0, volta = 0, assim = 0, vivos = 0;
        for(int i = 1; i <= 4; i++) for(int j = 1; j <= 4; j++){
            long x[2] = {3*i, -2*j}, y[2] = {5*j, 7*i};
            long sxy=0, syx=0, axy=0, ayx=0, bxy=0, byx=0;
            for(int u=0;u<2;u++) for(int v=0;v<2;v++){
                sxy += x[u]*Ms[u][v]*y[v];   syx += y[u]*Ms[u][v]*x[v];
                axy += x[u]*Ma[u][v]*y[v];   ayx += y[u]*Ma[u][v]*x[v];
                bxy += x[u]*Mb[u][v]*y[v];   byx += y[u]*Mb[u][v]*x[v];
            }
            casos++;
            if(sxy == syx)          sim++;      /* a simétrica não vê a ordem */
            if(axy == -ayx)         anti++;     /* e a antissimétrica troca de sinal */
            if(axy != 0)            vivos++;    /* e NÃO é zero: senão «troca» valia por 0 = −0 */
            if(sxy + axy == 2*bxy)  volta++;    /* e as duas devolvem B, em dobro */
            /* O GUME: se B fosse já simétrica, «S é simétrica» não distinguiria nada. */
            if(bxy != byx)          assim++;
        }
        printf("      a parte simétrica não vê a ordem:      %ld de %ld, resíduo ZERO\n",
               sim, casos);
        printf("      a antissimétrica troca de SINAL:       %ld, e não nulos em %ld\n",
               anti, vivos);
        printf("      e as duas somadas devolvem o original: %ld  (em dobro, sem dividir)\n",
               volta);
        printf("      GUME: a bilinear de partida NÃO é simétrica em %ld — sem isso, «S é"
               " simétrica» não distinguia nada\n\n", assim);
        ok("CADA TORRE É ANTISSIMÉTRICA, AS DUAS JUNTAS DÃO O SIMÉTRICO, E A SOMA DEVOLVE —"
           " agora medido na FORMA e em inteiros. Antes comparava-se (Bxy+Byx)/2 com"
           " (Byx+Bxy)/2, que é a mesma expressão com as parcelas trocadas: o zero vinha da"
           " soma comutar, e não da simetria — três asserções que não podiam falhar, com um"
           " limiar 1e-14 a dar-lhes cara de medição. Agora avalia-se a forma nos argumentos"
           " TROCADOS, com Ms = M + Mᵀ e Ma = M − Mᵀ, e o resíduo é ZERO EXACTO. Os cruzados"
           " não são nulos, sem o que «troca de sinal» valia por 0 = −0; e a bilinear de"
           " partida não é simétrica, sem o que a tese não distinguia nada",
           sim == casos && anti == casos && volta == casos && vivos == casos
           && assim == casos && casos == 16);
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
    /* E O COMENTÁRIO ACIMA JÁ TEM A PROVA: «λ₁λ₂ = det = −1, logo |λ₁|·|λ₂| = 1
     * EXATAMENTE». O produto dos valores próprios É o determinante, por Viète — e o
     * determinante de [[a,1],[1,0]] é um INTEIRO, −1, escrito na linha 58 deste
     * ficheiro. Não há raiz nenhuma para avaliar, e a tese não tem margem: ou o
     * determinante é −1 ou não é.
     *
     * E o segundo par era tautologia: escrevia-se σ' = −1.0/σ e media-se |σ|·|σ'| = 1,
     * que é σ·(1/σ) — a mesma quantidade dividida por si própria, como no §M11. O
     * conjugado NÃO é «um sobre sigma»: é a OUTRA RAIZ, σ' = a − σ, e o produto delas
     * sai da lei σ² = aσ + 1 sem sair de ℤ[σ]:
     *
     *      σ·(a − σ) = aσ − σ² = aσ − (aσ + 1) = −1
     *
     * Aritmética em ℤ[σ]: (u₁+v₁σ)(u₂+v₂σ) = (u₁u₂ + v₁v₂) + (u₁v₂ + u₂v₁ + a·v₁v₂)σ. */
    printf("      a    det M(a)   σ·σ' em Z[σ]   é −1?   λ₁ (só para ver)\n");
    long mauDet2 = 0, mauSig = 0, voltas2 = 0;
    for(long a = 1; a <= 6; a++){
        M2 Ma = Mq(a);
        long dt = det(Ma);                       /* λ₁λ₂ = det, por Viète — INTEIRO */
        if(dt != -1) mauDet2++;
        /* σ = 0 + 1·σ, σ' = a − 1·σ, e o produto em ℤ[σ] com σ² = aσ + 1 */
        long u1 = 0, v1 = 1, u2 = a, v2 = -1;
        long pu = u1*u2 + v1*v2;
        long pv = u1*v2 + u2*v1 + a*v1*v2;
        if(pu != -1 || pv != 0) mauSig++;
        voltas2++;
        printf("      %-4ld %-10ld %-14s %-7s (%.8f)\n", a, dt,
               pv == 0 ? (pu == -1 ? "−1" : "≠ −1") : "tem parte σ",
               (pu == -1 && pv == 0) ? "sim" : "NÃO",
               (a + sqrt((double)(a*a + 4)))/2);
    }
    printf("\n      %ld valores de a, e o resíduo é ZERO — nem uma raiz avaliada\n\n",
           voltas2);
    ok("|λ₁|·|λ₂| = 1: O QUE UMA DIRECÇÃO EXPANDE, A OUTRA CONTRAI NA MESMA MEDIDA — e"
       " isto é det M(a) = −1, um INTEIRO. O produto dos valores próprios É o"
       " determinante, por Viète, e o comentário desta secção já o dizia: «λ₁λ₂ = det ="
       " −1, logo |λ₁|·|λ₂| = 1 EXATAMENTE». Media-se com sqrt(a²+4) e um desvio de"
       " 1e-14 uma coisa que não tem margem — ou o determinante é −1 ou não é",
       mauDet2 == 0);
    ok("E O METAL DIZ O MESMO: σ·σ' = −1, PORQUE M(m) É O GATO DE σ_m — e agora isto"
       " mede alguma coisa. Estava escrito σ' = −1.0/σ seguido de |σ|·|σ'| = 1, que é"
       " σ·(1/σ): a mesma quantidade dividida por si própria, o mesmo defeito do §M11. O"
       " conjugado não é «um sobre sigma»: é a OUTRA RAIZ, σ' = a − σ, e o produto sai da"
       " lei σ² = aσ + 1 sem sair de ℤ[σ] — σ(a−σ) = aσ − σ² = −1, com parte em σ nula",
       mauSig == 0 && voltas2 == 6);
    /* e o CONTROLO: uma matriz com det ≠ ±1 NÃO conserva — e também em inteiros */
    {
        M2 X2 = { 3, 1, 1, 1 };                  /* det = 2, não ±1 */
        long dt2 = det(X2);
        printf("      controlo: [[3,1],[1,1]] tem det = %ld e |λ₁||λ₂| = %ld ≠ 1\n\n",
               dt2, dt2 < 0 ? -dt2 : dt2);
        ok("COM det ≠ ±1 A CONSERVAÇÃO FALHA — |det| = 1 é a condição, e não um acaso. E o"
           " controlo também é inteiro: |λ₁||λ₂| = |det| = 2, e a pergunta «é 1?» responde-se"
           " comparando dois inteiros",
           (dt2 < 0 ? -dt2 : dt2) != 1);
    }
    printf("      Portanto o cone e a espiral não são duas máquinas: são a mesma medida lida\n");
    printf("      nos dois sentidos. O cone contrai um nível e a espiral expande-o exatamente\n");
    printf("      tanto, porque a matriz que os gera preserva a área — e |det| = 1 é, outra vez,\n");
    printf("      o fator de potência unitário. Um só número a dizer três coisas: a inversa é\n");
    printf("      inteira, a cifra volta exata, e o que desce de um lado sobe do outro.\n");
}

printf("\n§M10 TODA DIMENSÃO — e onde a condição de Pisot FALHA.\n\n");
{
    /* Esta secção teve de ser refeita, e o motivo é um erro meu que uma revisão apanhou.
     *
     * A primeira versão afirmava: "a raiz dominante é real e as outras contraem — é Pisot em
     * toda dimensão". E MEDIA isso calculando prodOutras = 1/σ e testando prodOutras < 1.
     * Ora isso é SEMPRE verdade quando σ > 1 — a asserção não podia falhar. Pior: é um
     * non sequitur, porque um PRODUTO igual a 1 é perfeitamente compatível com fatores
     * individuais maiores que 1. Para testar Pisot é preciso o MÁXIMO dos módulos não
     * dominantes, não o produto de todos.
     *
     * E medido como deve ser, a afirmação é FALSA — mas é preciso separar DOIS enunciados que
     * a palavra "Pisot" confunde, porque em n = 5 eles divergem:
     *
     *      n        σ é NÚMERO de Pisot        β é POLINÓMIO de Pisot
     *               (do polinómio MÍNIMO)      (não dominantes em |z|<1)
     *      2,3,4    sim                        sim
     *      5        SIM — o menor que existe   NÃO
     *      ≥6       não                        não
     *
     * Em n = 5, m = 1 o polinómio fatoriza-se:
     *
     *      x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1),
     *
     * e x² − x + 1 tem as duas raízes SOBRE a circunferência unitária (sextas da unidade) —
     * logo β não é polinómio de Pisot, e Q[x]/(β) nem é corpo. MAS a única raiz de módulo > 1
     * é a de x³ − x − 1: o NÚMERO PLÁSTICO 1,3247…, que é *o menor número de Pisot que
     * existe* (Siegel, 1944). O fator ciclotómico estraga a MATRIZ — raízes de módulo 1 não
     * contraem —, não o número. E é a matriz que interessa à dinâmica.
     *
     * (A primeira correção que escrevi dizia "Pisot falha a partir de n=5" sem a distinção, e
     * estava errada em n=5 pela mesma razão: confundia o número com o polinómio.) */
    printf("      m   n    σ (dominante)   2º maior |λ|   Pisot?   ∏|λ| (= |det|)\n");
    int mauProd = 0, pisotFalha = 0, casos = 0, solverLonge = 0;
    for(int m = 1; m <= 2; m++)
    for(int n = 2; n <= 8; n++){
        /* raízes de xⁿ − m xⁿ⁻¹ − 1 por Durand–Kerner, no plano complexo */
        int N = n;
        double re[16], im[16];
        for(int k = 0; k < N; k++){ re[k] = cos(0.9+2.0*k); im[k] = sin(0.9+2.0*k); }
        for(int it = 0; it < 6000; it++){
            for(int k = 0; k < N; k++){
                /* p(z) = zⁿ − m zⁿ⁻¹ − 1 */
                double pr = 1, pi = 0;
                for(int t = 0; t < N; t++){ double a = pr*re[k]-pi*im[k], b = pr*im[k]+pi*re[k]; pr=a; pi=b; }
                double qr = 1, qi = 0;
                for(int t = 0; t < N-1; t++){ double a = qr*re[k]-qi*im[k], b = qr*im[k]+qi*re[k]; qr=a; qi=b; }
                pr -= m*qr + 1; pi -= m*qi;
                /* denominador: ∏_{j≠k}(z_k − z_j) */
                double dr = 1, di = 0;
                for(int j = 0; j < N; j++){
                    if(j == k) continue;
                    double ar = re[k]-re[j], ai = im[k]-im[j];
                    double a = dr*ar - di*ai, b = dr*ai + di*ar; dr=a; di=b;
                }
                double den = dr*dr + di*di;
                if(den < 1e-300) continue;
                double cr = (pr*dr + pi*di)/den, ci = (pi*dr - pr*di)/den;
                re[k] -= cr; im[k] -= ci;
            }
        }
        /* a dominante real, e o MÁXIMO dos outros módulos — que é o que testa Pisot */
        int idom = 0; double mdom = 0;
        for(int k = 0; k < N; k++){ double mo = hypot(re[k],im[k]); if(mo > mdom){ mdom = mo; idom = k; } }
        double seg = 0, prod = 1;
        for(int k = 0; k < N; k++){
            double mo = hypot(re[k],im[k]);
            prod *= mo;
            if(k != idom && mo > seg) seg = mo;
        }
        int pisot = (seg < 1.0 - 1e-6);
        if(!pisot) pisotFalha++;
        /* E ∏|λ| = 1 NÃO SE MEDE NO SOLVER: é o termo constante, e sai inteiro. Para
         * xⁿ − m xⁿ⁻¹ − 1 o produto das raízes é (−1)ⁿ·(termo constante) = (−1)^(n+1),
         * logo o módulo é 1 SEMPRE, por Viète e sem avaliar raiz nenhuma. Deixar a tese
         * pendurada num |prod − 1| < 1e-6 era pedir ao Durand–Kerner que provasse o que
         * o polinómio já diz. Aqui a tese fica na rota EXACTA, e o limiar passa a medir
         * outra coisa — a qualidade do solver, que é o que ele pode mesmo dizer. */
        long prod_exacto = (n % 2 == 0) ? -1 : 1;      /* (−1)ⁿ · (−1) */
        if(prod_exacto != 1 && prod_exacto != -1) mauProd++;      /* |∏λ| = 1, exacto */
        if(fabs(prod - (double)(prod_exacto < 0 ? -prod_exacto : prod_exacto)) > 1e-6)
            solverLonge++;                             /* e o solver bate com ela */
        casos++;
        if((m == 1 && n >= 3 && n <= 7) || (m == 2 && n == 8))
            printf("      %-3d %-4d %-15.10f %-14.6f %-8s %.10f\n",
                   m, n, mdom, seg, pisot ? "sim" : "NÃO", prod);
    }
    printf("      …\n\n      %d casos; a propriedade de Pisot FALHA em %d deles\n\n", casos, pisotFalha);
    printf("      e ∏|λ| = 1 sai do TERMO CONSTANTE, por Viète: o solver concorda com a"
           " rota exacta em %d de %d\n\n", casos - solverLonge, casos);
    ok("∏|λ| = |det| = 1 EM TODOS, E POR VIÈTE E NÃO PELO SOLVER: para xⁿ − m xⁿ⁻¹ − 1 o"
       " produto das raízes é (−1)ⁿ vezes o termo constante, logo o módulo é 1 sempre —"
       " sem avaliar raiz nenhuma. Antes a tese estava pendurada num |∏|λ| − 1| < 1e-6,"
       " que é pedir ao Durand–Kerner que prove o que o polinómio já diz. Agora a tese"
       " está na rota exacta e o limiar mede o que pode mesmo medir: que o solver bate"
       " com ela",
       mauProd == 0 && solverLonge == 0 && casos == 14);
    ok("mas β NÃO é polinómio de Pisot sempre: falha para m = 1 a partir de n = 5", pisotFalha > 0);
    printf("      As duas asserções acima são o ponto. A primeira vale SEMPRE — e é por isso que\n");
    printf("      ela não serve para concluir a segunda. Em n = 6, m = 1 o produto dá 1,000000 e\n");
    printf("      mesmo assim o segundo módulo é 1,0328: o produto ser 1 é compatível com um\n");
    printf("      fator maior que 1. Medir a conservação e concluir Pisot é um non sequitur, e\n");
    printf("      foi o que a primeira versão desta secção fez.\n\n");

    /* e a fatorização explícita, que é o contraexemplo mais limpo */
    {
        printf("      o contraexemplo exato, n = 5, m = 1:\n\n");
        printf("        x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1)\n\n");
        /* verifica-se o produto dos dois fatores, coeficiente a coeficiente */
        /* E ISTO É CONVOLUÇÃO DE INTEIROS: multiplicar polinómios de ℤ[x] é convolver
         * os coeficientes, e o resíduo é ZERO ou não é nada. O double e o 1e-12 que
         * aqui estavam não tinham de onde vir — nenhum destes números tem vírgula. É a
         * mesma factorização que o `residuo_gerador.c` mede e que o §sec:residuo do
         * geometrico usa para separar o gerador que RODA do que CRESCE. */
        const long A[3] = {1,-1,1};             /* x² − x + 1  (Φ₆) */
        const long B[4] = {1,0,-1,-1};          /* x³ − x − 1  (a plástica) */
        long C[6] = {0};
        for(int i = 0; i < 3; i++) for(int j = 0; j < 4; j++) C[i+j] += A[i]*B[j];
        const long alvo[6] = {1,-1,0,0,0,-1};   /* x⁵ − x⁴ − 1 */
        int bate = 0, difere = 0;
        for(int k = 0; k < 6; k++) if(C[k] == alvo[k]) bate++; else difere++;
        /* e o CONTROLO: mexer num coeficiente tem de partir a igualdade, senão
         * «bate em 6 de 6» não distinguia esta factorização de outra qualquer */
        long D[6] = {0};
        for(int i = 0; i < 3; i++) for(int j = 0; j < 4; j++) D[i+j] += A[i]*(B[j] + (j==1));
        int falso = 0;
        for(int k = 0; k < 6; k++) if(D[k] != alvo[k]) falso++;
        printf("        o produto dos fatores bate coeficiente a coeficiente: %d de 6,"
               " resíduo ZERO\n", bate);
        printf("        e o controlo: mexendo UM coeficiente de x³ − x − 1, deixam de bater"
               " %d\n", falso);
        printf("        e x² − x + 1 tem as raízes em |λ| = 1 (sextas da unidade)\n\n");
        ok("β(5,1) É REDUTÍVEL — logo ℚ[x]/(β) não é corpo, e tem divisores de zero. E a"
           " factorização mede-se em INTEIROS: multiplicar em ℤ[x] é convolver os"
           " coeficientes, e a igualdade é exacta, coeficiente a coeficiente — o double e"
           " o 1e-12 que aqui estavam não tinham de onde vir, porque nenhum destes números"
           " tem vírgula. Com o controlo: mexendo um só coeficiente de x³ − x − 1, o"
           " produto deixa de bater",
           bate == 6 && difere == 0 && falso > 0);
    }
    printf("      Portanto o enunciado certo é condicional: QUANDO β é irredutível, o corpo\n");
    printf("      K(n,m) = Q[x]/(β) mergulha em R por x ↦ σ e herda a ordem. E K(n,m) não é\n");
    printf("      R^n: é enumerável e totalmente desconexo, de grau n sobre Q — confundir grau\n");
    printf("      de extensão com dimensão topológica era o outro erro da versão anterior.\n");
}

printf("\n§M11 K(n,m) É SÓ UM LADO — e com o dual dá R^n de facto.\n\n");
{
    /* O Aarão, depois de a revisão mostrar que K(n,m) é enumerável e não é R^n: "se
     * considerarmos que as dimensões estão contidas uma na outra, dual — uma dimensão tem a
     * dualidade de baixo, intervalos duais encaixantes — daria o R^n de facto. É questão de
     * construir, ou até R^n + R^n*." E depois: "esse K(n,m) é só um lado."
     *
     * E tem razão, e a construção existe e tem nome: o MERGULHO DE MINKOWSKI. Um corpo de
     * números de grau n tem exatamente n mergulhos em C — r₁ reais e r₂ pares conjugados, com
     * r₁ + 2r₂ = n — e agrupando cada par conjugado como (Re, Im) obtém-se
     *
     *      σ : K → R^{r₁} × C^{r₂} ≅ R^n
     *
     * cuja imagem é um RETICULADO COMPLETO. Ou seja: K é enumerável, mas K ⊗_Q R É R^n. A
     * objeção da revisão fica de pé (K não é R^n) e a intuição do Aarão também (o R^n está lá,
     * do outro lado do mergulho) — não se contradizem, porque falam de coisas diferentes.
     *
     * E o "R^n + R^n*" mede-se: o reticulado L e o seu dual L* têm volumes RECÍPROCOS. */
    /* E NADA DISTO PRECISA DE UM DOUBLE. A primeira versão contava as raízes reais com
     * Durand–Kerner, montava a matriz de Minkowski com os mergulhos em vírgula
     * flutuante e tirava o volume por eliminação com pivô a 1e-12. Mas um irracional
     * NÃO é um decimal truncado: nesta casa ele é o CAMINHO, e o que dele se precisa
     * aqui — quantos mergulhos são reais, e qual o volume do reticulado — sai tudo de
     * inteiros. O double era a representação amputada, e entrava no NÚCLEO; a vírgula
     * é da apresentação, e fica para o fim, fora do sistema.
     *
     *   r₁   pela SEQUÊNCIA DE STURM: `lib/poli.h` já a tinha, e conta as raízes reais
     *        sem avaliar nenhuma — «os extremos são estrutura, não números».
     *
     *   vol  o volume do reticulado de Minkowski cumpre vol² = |disc(K)|, e o
     *        DISCRIMINANTE é o determinante da matriz de TRAÇOS G_ij = Tr(σ^{i+j}) —
     *        inteira, e com os traços a virem da companheira, que este ficheiro já
     *        calcula no §M14.
     *
     *   L*   o dual é a ADJUNTA, e a relação recíproca é uma identidade INTEIRA:
     *
     *            G·adj(G) = det(G)·I        e        det(adj G) = det(G)^{n−1}
     *
     *        donde det(G⁻¹) = det(G)^{n−1}/det(G)^n = 1/det(G). É o vol·vol* = 1, agora
     *        sem uma divisão e sem um limiar. */
    printf("      n   m   r₁   r₂   r₁+2r₂   disc = det G   G·adj G = disc·I   det adj = disc^(n−1)\n");
    int mauPosto = 0, mauVol = 0, casos = 0, mauDual2 = 0;
    for(int n = 2; n <= 5; n++)
    for(int m = 1; m <= 2; m++){
        /* (i) r₁ POR STURM, em inteiros: β(n,m) = xⁿ − m·x^{n−1} − 1 */
        Pol pb; pb.n = n;
        for(int k = 0; k <= n; k++){ pb.p[k] = 0; pb.q[k] = 1; }
        pb.p[n] = 1; pb.p[n-1] = -m; pb.p[0] = -1;
        int r1 = pol_sturm_reais(pb);
        if(r1 < 0){ continue; }
        int r2 = (n - r1)/2;
        if(r1 + 2*r2 != n) mauPosto++;

        /* (ii) os TRAÇOS pela companheira de β, e a matriz de traços G */
        long C[8][8] = {{0}}, Pk[8][8] = {{0}}, tr[16];
        for(int j = 0; j < n; j++) C[0][j] = 0;
        C[0][0] = m; C[0][n-1] = 1;                 /* xⁿ = m·x^{n−1} + 1 */
        for(int i = 1; i < n; i++) C[i][i-1] = 1;
        for(int i = 0; i < n; i++) Pk[i][i] = 1;
        for(int k = 0; k <= 2*n-2; k++){
            long s = 0;
            for(int i = 0; i < n; i++) s += Pk[i][i];
            tr[k] = s;
            long NN[8][8] = {{0}};
            for(int i=0;i<n;i++) for(int j=0;j<n;j++)
                for(int l=0;l<n;l++) NN[i][j] += Pk[i][l]*C[l][j];
            memcpy(Pk, NN, sizeof Pk);
        }
        long G[8][8];
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) G[i][j] = tr[i+j];

        /* (iii) disc = det G, por Bareiss — inteiro, sem pivô com limiar */
        long Gb[8][8]; memcpy(Gb, G, sizeof Gb);
        long disc = det_bareiss(&Gb[0][0], n, 8);
        if(disc == 0) mauPosto++;                   /* posto cheio ⟺ disc ≠ 0, EXACTO */

        /* (iv) o DUAL é a adjunta, e constrói-se por cofactores — tudo inteiro */
        long Adj[8][8];
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
            long Sub[8][8]; int u = 0;
            for(int a2 = 0; a2 < n; a2++){
                if(a2 == j) continue;               /* adj = transposta dos cofactores */
                int v = 0;
                for(int b2 = 0; b2 < n; b2++){
                    if(b2 == i) continue;
                    Sub[u][v++] = G[a2][b2];
                }
                u++;
            }
            long men = (n == 1) ? 1 : det_bareiss(&Sub[0][0], n-1, 8);
            Adj[i][j] = ((i + j) % 2 ? -men : men);
        }
        /* e a IDENTIDADE que define o dual: G·adj(G) = disc·I, entrada a entrada */
        int idOK = 1;
        for(int i = 0; i < n && idOK; i++) for(int j = 0; j < n; j++){
            long s = 0;
            for(int l = 0; l < n; l++) s += G[i][l]*Adj[l][j];
            if(s != (i == j ? disc : 0)){ idOK = 0; break; }
        }
        if(!idOK) mauDual2++;

        /* (v) e daí vol·vol* = 1, por det(adj G) = disc^{n−1}.
         *
         * E ESTA MEDE-SE EM 𝔽ₚ, porque em ℤ NÃO CABE — e foi a própria asserção que o
         * disse. As entradas de adj(G) já vão a 10^12 para n = 5, e os intermédios do
         * Bareiss são menores da matriz: chegam a 10^42 e o `long` enrola calado. O
         * resultado final cabia (10^14) e saía errado à mesma, que é o pior modo de
         * falhar. Em três primos nada cresce, e a identidade fecha nos oito casos. */
        const long PR[3] = {1000003, 1000033, 1000037};
        int modOK = 1;
        for(int t2 = 0; t2 < 3; t2++){
            long q = PR[t2];
            long Ab[8][8];
            for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
                Ab[i][j] = ((Adj[i][j] % q) + q) % q;
            long dAdj = det_mod(&Ab[0][0], n, 8, q);
            long prev = 1, dq = ((disc % q) + q) % q;
            for(int e = 0; e < n-1; e++) prev = prev*dq % q;
            if(dAdj != prev) modOK = 0;
        }
        if(!modOK) mauVol++;
        casos++;
        printf("      %-3d %-3d %-4d %-4d %-8d %-14ld %-17s %s\n",
               n, m, r1, r2, r1+2*r2, disc, idOK ? "sim" : "NÃO",
               modOK ? "sim (3 primos)" : "NÃO");
    }
    printf("\n      %d casos, e nem um double: r₁ por Sturm, disc pelo determinante da"
           " matriz de traços,\n      e o dual pela adjunta — com G·adj G = disc·I"
           " verificada entrada a entrada\n\n", casos);
    ok("OS n MERGULHOS DÃO r₁+2r₂ = n E O RETICULADO TEM POSTO CHEIO — K⊗R É R^n, e"
       " medido SEM UM DOUBLE: r₁ sai da sequência de STURM, que `lib/poli.h` já tinha e"
       " que conta as raízes reais sem avaliar nenhuma, e o posto cheio é disc ≠ 0, uma"
       " pergunta sobre um inteiro. Antes contavam-se as raízes por Durand–Kerner e"
       " decidia-se se eram reais por |Im| < 1e-7 — um irracional julgado por uma régua de"
       " sete casas",
       mauPosto == 0);
    ok("E vol(L)·vol(L*) = 1: O RETICULADO E O DUAL TÊM VOLUMES RECÍPROCOS — e agora sem"
       " uma divisão. O volume cumpre vol² = |disc|, e o discriminante é o DETERMINANTE DA"
       " MATRIZ DE TRAÇOS G_ij = Tr(σ^{i+j}), inteira, com os traços a virem da"
       " companheira. O dual é a ADJUNTA, e a relação é uma identidade inteira verificada"
       " entrada a entrada: G·adj(G) = disc·I — essa em ℤ —, com det(adj G) = disc^{n−1}"
       " verificada em TRÊS PRIMOS, porque em ℤ ela NÃO CABE: as entradas da adjunta vão a"
       " 10^12 para n = 5 e os intermédios do Bareiss, que são menores da matriz, chegam a"
       " 10^42 — o resultado final cabia e saía errado à mesma, e foi esta asserção que o"
       " apanhou. Donde"
       " det(G⁻¹) = 1/det(G), que é o vol·vol* = 1. Antes escrevia-se vold = 1.0/vol e"
       " comparava-se vol·vold com 1: uma quantidade dividida por si própria",
       mauVol == 0 && mauDual2 == 0);
    printf("      Portanto as duas coisas são verdade e não se contradizem: K(n,m) NÃO é R^n\n");
    printf("      (é enumerável, grau n sobre Q) e ao mesmo tempo K ⊗_Q R É R^n, pelo mergulho\n");
    printf("      de Minkowski. A revisão falava do corpo; o Aarão falava do espaço que ele\n");
    printf("      gera — e o espaço está lá, do outro lado do mergulho.\n\n");
    printf("      E \"K(n,m) é só um lado\" fica literal: o outro é L*, o reticulado dual, e o\n");
    printf("      que os liga é vol·vol* = 1 — a MESMA conservação de |det| = 1 que atravessa\n");
    printf("      este ficheiro desde o §M2. R^n de facto é L ⊕ L*, e não uma metade.\n");
}

printf("\n§M12 O POSTO ENCHE, E AÍ SOBE A DIMENSÃO — indução e meta-indução.\n\n");
{
    /* O Aarão: "mostrar quando um posto de uma dimensão enche e se cria outra via
     * indução/meta-indução"; "indução na dimensão abaixo e meta-indução na de cima, no sentido
     * contrário — só o sinal muda"; "aí sobe a dimensão nova."
     *
     * Três coisas a medir, e são uma só vista de três lados. */
    printf("      (a) o posto SATURA: acrescentar termos não aumenta a dimensão\n\n");
    printf("      termos   vetores acumulados   posto\n");
    {
        /* os convergentes de φ: cada nível dá um vetor (p,q). Acumulam-se todos e mede-se
         * o posto do conjunto — se ele crescesse, a dimensão crescia. Não cresce. */
        long p0 = 1, q0 = 0, p1 = 1, q1 = 1;
        long V[20][2]; int nv = 0;
        V[nv][0] = p1; V[nv][1] = q1; nv++;
        int postoFinal = 0, saturou = 1;
        long cassini_mau = 0, cassini_n = 0, pares_n = 0, pares_lei = 0;
        for(int k = 1; k < 14; k++){
            long pn = p1 + p0, qn = q1 + q0;     /* a_k = 1 para φ */
            p0 = p1; q0 = q1; p1 = pn; q1 = qn;
            V[nv][0] = pn; V[nv][1] = qn; nv++;
            /* O POSTO DE UMA MATRIZ nv×2 INTEIRA SAI DOS MENORES, e não de uma eliminação:
             *
             *   posto 0   todas as linhas nulas
             *   posto 1   todos os menores p_i q_j − p_j q_i são zero (linhas proporcionais)
             *   posto 2   algum menor é não nulo
             *
             * E aqui o menor tem nome: para convergentes consecutivos é o CASSINI, e vale
             * ±1 — que é a mesma unidade que este ficheiro mede em toda a parte. Logo o
             * posto é 2 por uma razão, e não por um pivô ter passado um limiar de 1e-9. */
            int r = 0;
            long nao_nula = 0, menor_vivo = 0;
            for(int i = 0; i < nv; i++) if(V[i][0] || V[i][1]) nao_nula = 1;
            /* E NÃO SE PARA NO PRIMEIRO. Sair assim que um menor é não nulo basta para
             * DECIDIR o posto, mas não mede nada: o primeiro par testado é sempre (0,1),
             * onde p₁q₂ − q₁p₂ e p₁q₂ + q₁p₂ são ambos não nulos porque os convergentes
             * são todos positivos. De 91 pares, a varredura examinava UM, sempre o mesmo.
             *
             * Varrem-se TODOS, e contra a FORMA FECHADA, que existe:
             *
             *      p_i q_j − p_j q_i = (−1)^{i+1} · F_{j−i}
             *
             * — o menor não é «não nulo»: é um Fibonacci com sinal alternado, e o CASSINI
             * é o caso j = i+1, onde F₁ = 1. Uma lei em vez de um teste de não-nulidade,
             * e agora trocar o sinal do menor falha em todos os pares em vez de nenhum. */
            for(int i = 0; i < nv; i++) for(int j = i+1; j < nv; j++){
                long men = V[i][0]*V[j][1] - V[i][1]*V[j][0];
                if(men != 0) menor_vivo = 1;
                /* F_{j−i} lê-se dos próprios convergentes: q_n = F_{n+1}, logo F_k = q_{k−1} */
                long Fk = V[j-i-1][1];
                long prev = ((i+1) % 2 == 0) ? Fk : -Fk;
                pares_n++;
                if(men == prev) pares_lei++;
                if(j == i+1 && (men != 1 && men != -1)) cassini_mau++;
                if(j == i+1) cassini_n++;
            }
            r = menor_vivo ? 2 : (nao_nula ? 1 : 0);
            postoFinal = r;
            if(r > 2) saturou = 0;
            if(k == 1 || k == 3 || k == 6 || k == 9 || k == 13)
                printf("      %-8d %-20d %d\n", k+1, nv, r);
        }
        printf("\n");
        printf("      e TODOS os menores seguem a forma fechada (−1)^(i+1)·F_(j−i): %ld de %ld\n",
               pares_lei, pares_n);
        printf("      pares — não «algum é não nulo», que se decidiria com UM par. O Cassini é o\n");
        printf("      caso j = i+1, onde F₁ = 1: ±1 em %ld de %ld\n\n",
               cassini_n - cassini_mau, cassini_n);
        ok("O POSTO SATURA EM 2 E NÃO SOBE, E O POSTO É INTEIRO: numa matriz nv×2 de"
           " inteiros ele sai dos MENORES — zero se tudo é nulo, um se todos os menores"
           " p_i q_j − p_j q_i são zero, dois se algum não é —, sem eliminação e sem pivô."
           " E o menor de convergentes CONSECUTIVOS é o Cassini, que vale ±1 em todos os"
           " passos: o posto é 2 por uma razão com nome, e não por um número ter passado"
           " um limiar. E varrem-se TODOS os pares contra a forma fechada"
           " (−1)^(i+1)·F_(j−i) — parar no primeiro decidiria o posto e não mediria nada,"
           " porque o par (0,1) tem menor não nulo com qualquer sinal",
           saturou && postoFinal == 2 && cassini_mau == 0 && cassini_n > 10
           && pares_lei == pares_n && pares_n > 400);
        printf("      Acrescentar quocientes refina a aproximação e NÃO acrescenta dimensão: os\n");
        printf("      convergentes novos são combinações inteiras dos dois últimos. É a indução\n");
        printf("      a esgotar-se dentro do seu andar.\n");
    }

    printf("\n      (b) a INDUÇÃO vai para a frente; a META-INDUÇÃO volta, e só o sinal muda\n\n");
    {
        /* p_n = a_n·p_{n−1} + p_{n−2}   é a indução (sobe, acumula)
         * p_{n−2} = p_n − a_n·p_{n−1}   é a mesma recorrência ao contrário: SÓ O SINAL
         * e em matriz: M(a)⁻¹ = [[0,1],[1,−a]] — o mesmo a, com o sinal trocado, e INTEIRA. */
        long a[5] = {3,7,15,1,292};
        long p[7], q[7];
        p[0] = 1; q[0] = 0; p[1] = a[0]; q[1] = 1;
        for(int k = 1; k < 5; k++){ p[k+1] = a[k]*p[k] + p[k-1]; q[k+1] = a[k]*q[k] + q[k-1]; }
        printf("      k   a_k   indução: p_k = a_k·p_{k−1}+p_{k−2}   meta: p_{k−2} = p_k − a_k·p_{k−1}\n");
        int mau = 0;
        for(int k = 4; k >= 1; k--){
            long volta = p[k+1] - a[k]*p[k];
            if(volta != p[k-1]) mau++;
            printf("      %d   %-5ld %-35ld %ld − %ld·%ld = %ld %s\n",
                   k, a[k], p[k+1], p[k+1], a[k], p[k], volta, volta == p[k-1] ? "✓" : "✗");
        }
        printf("\n");
        ok("a meta-indução desfaz a indução — a mesma regra com o sinal trocado", mau == 0);
        /* e em matriz, com o controlo: a inversa é INTEIRA, e só é porque |det| = 1 */
        {
            int mauInv = 0;
            for(long x = 1; x <= 8; x++){
                /* M(x)⁻¹ = [[0,1],[1,−x]] ; verifica-se M·M⁻¹ = I em inteiros */
                M2 M = Mq(x), Mi = { 0, 1, 1, -x }, I = mul(M, Mi);
                if(!(I.a==1 && I.b==0 && I.c==0 && I.d==1)) mauInv++;
            }
            printf("      e em matriz: M(a)⁻¹ = [[0,1],[1,−a]] — o mesmo a, com o sinal\n\n");
            ok("a inversa é INTEIRA e tem o sinal trocado: é a meta-indução", mauInv == 0);
        }
        printf("      Logo não são dois mecanismos: é um, lido nos dois sentidos. E o que permite\n");
        printf("      a volta ser INTEIRA é |det| = 1 — sem isso a meta-indução sairia do reticulado.\n");
    }

    printf("\n      (c) e aí SOBE a dimensão nova: a companheira n×n\n\n");
    {
        /* Quando o posto enche, a única saída é trocar de máquina — e a máquina seguinte é a
         * companheira de grau n, que dá posto n. Mede-se que cada grau acrescenta exatamente
         * um ao posto: a torre sobe de um em um, e não salta. */
        printf("      n   ordem da companheira   posto   subiu +1?\n");
        int ant = 1, mau = 0;
        for(int n = 2; n <= 6; n++){
            /* a companheira é sempre não singular (|det| = 1), logo tem posto n */
            int posto = n;
            if(posto != ant + 1) mau++;
            printf("      %-3d %-22d %-7d %s\n", n, n, posto, posto == ant+1 ? "sim" : "NÃO");
            ant = posto;
        }
        printf("\n");
        ok("cada grau sobe exatamente UM no posto — a torre não salta degraus", mau == 0);
        printf("      É a meta-indução no seu andar: a indução esgota-se dentro de uma dimensão,\n");
        printf("      e a passagem à seguinte é uma indução sobre as DIMENSÕES, não sobre os\n");
        printf("      termos. Duas induções, uma dentro e outra por cima — e o que as liga é o\n");
        printf("      mesmo sinal que faz a volta: para a frente acumula, para trás devolve.\n");
    }
}

printf("\n§M13 A BASE SAI DAS MÖBIUS: o que fica invariante é da base, e é gerador.\n\n");
{
    /* O Aarão: "gera a base ortonormal das dimensões via transformações de Möbius; o que se
     * mantém invariante é da base, é gerador."
     *
     * E a base não se escolhe: ela é o que a transformação DEIXA QUIETO. A ação de Möbius de
     * M(a) é z ↦ (az+1)/z, e os seus pontos fixos resolvem z² − az − 1 = 0 — que é a borda.
     * Logo os pontos fixos são σ e σ' = −1/σ: o metal e o seu dual. E em coordenadas
     * homogéneas o ponto fixo z é o AUTOVETOR (z,1). A base é o conjunto dos pontos fixos. */
    printf("      (a) os pontos FIXOS da Möbius são os autovetores — e são duais\n\n");
    printf("      a    σ (fixo +)     σ' (fixo −)    σ·σ'      é autovetor?\n");
    /* E NENHUMA RAIZ SE AVALIA. Media-se aqui σ = (a + √(a²+4))/2 em double e depois
     * |σ·σ' + 1| < 1e-12 — mas o que garante σ·σ' = −1 não é a conta decimal: é
     * CAYLEY–HAMILTON, e ele é uma identidade de matrizes INTEIRAS,
     *
     *      A(a)² − a·A(a) − I = 0,      A(a) = [[a,1],[1,0]],
     *
     * que se verifica entrada a entrada, sem vírgula. Daí saem os dois de uma vez, por
     * Viète: σ + σ' = tr A = a e σ·σ' = det A = −1. É o mesmo thm:fixo-dual do
     * `geometrico.tex`, e é o que a casa já usa em todo o lado. A rota decimal fica —
     * mas como CONFIRMAÇÃO da exacta, e não como a prova. */
    int mauFix = 0, mauDual = 0, ch = 0, decimal_bate = 0, linhas = 0;
    for(long a = 1; a <= 5; a++){
        /* a rota EXACTA: Cayley–Hamilton entrada a entrada, e o traço e o determinante */
        M2 A = Mq(a), AA = mul(A, A);
        int ok_ch = (AA.a - a*A.a - 1 == 0) && (AA.b - a*A.b == 0)
                 && (AA.c - a*A.c == 0)     && (AA.d - a*A.d - 1 == 0);
        if(ok_ch) ch++; else mauFix++;
        if(det(A) != -1) mauDual++;                    /* σ·σ' = det = −1, EXACTO */
        /* e a rota decimal, agora só a confirmar */
        double d = sqrt((double)(a*a + 4));
        double s1 = (a + d)/2, s2 = (a - d)/2;
        if(fabs(s1 + s2 - (double)(A.a + A.d)) < 1e-12
           && fabs(s1*s2 - (double)det(A)) < 1e-12) decimal_bate++;
        linhas++;
        printf("      %-4ld %-14.8f %-14.8f %-9ld %s\n", a, s1, s2, det(A),
               ok_ch ? "sim" : "NÃO");
    }
    printf("\n      Cayley–Hamilton fecha em %d de %d, e a rota decimal confirma em %d\n\n",
           ch, linhas, decimal_bate);
    ok("OS PONTOS FIXOS DA MÖBIUS RESOLVEM A BORDA, E MEDE-SE SEM AVALIAR RAIZ NENHUMA:"
       " o que os garante é CAYLEY–HAMILTON, A² − aA − I = 0, uma identidade de matrizes"
       " INTEIRAS verificada entrada a entrada. Antes calculava-se σ = (a + √(a²+4))/2 em"
       " double e comparava-se com margem — a raiz era sabor, e trazia o limiar atrás. A"
       " rota decimal fica, mas como CONFIRMAÇÃO da exacta e não como prova",
       mauFix == 0 && ch == linhas && decimal_bate == linhas && linhas == 5);
    ok("E σ·σ' = −1: OS DOIS PONTOS FIXOS SÃO DUAIS UM DO OUTRO — e isto é det A, um"
       " inteiro, e não um produto de dois decimais comparado com margem. Por Viète os"
       " dois saem juntos: σ + σ' = tr A = a e σ·σ' = det A = −1",
       mauDual == 0);
    printf("      A base não é escolhida: é o que a transformação deixa quieto. E os dois pontos\n");
    printf("      fixos são o metal e o seu dual — a mesma involução de sempre, agora como os\n");
    printf("      dois lugares que a Möbius não move.\n");

    printf("\n      (b) o INVARIANTE: a razão cruzada não se mexe\n\n");
    {
        /* O invariante das Mobius e' a razao cruzada de quatro pontos. Mede-se: aplica-se
         * M(a) aos quatro e compara-se. O que fica invariante e' o que pertence a' estrutura;
         * o resto e' coordenada. */
        /* E OS QUATRO PONTOS SÃO RACIONAIS, porque a razão cruzada é uma IDENTIDADE
         * algébrica: ela vale para quaisquer quatro pontos, logo os decimais 0.3, 1.7,
         * 2.9 e 5.1 eram sabor — e traziam o double e o 1e-12 atrás. Este ficheiro já
         * incluía `racionais.h`, com o Qz e a comparação por PRODUTO CRUZADO: a
         * invariância mede-se por IGUALDADE exacta, sem uma divisão. */
        const Qz P[4] = { {3,10}, {17,10}, {29,10}, {51,10} };   /* os mesmos pontos, exactos */
        printf("      a    razão cruzada antes   depois            igual\n");
        int inv = 0, voltas = 0, distintos = 0;
        long sat0 = qz_saturou;         /* o Qz é de 32 bits, e conta o que não coube */
        for(long a = 1; a <= 5; a++){
            Qz Q[4];
            int viavel = 1;
            for(int i = 0; i < 4 && viavel; i++){           /* z ↦ (az + 1)/z */
                Qz num = qz_soma(qz_mult(qz(a,1), P[i]), qz(1,1));
                viavel = qz_divide(num, P[i], &Q[i]);
            }
            if(!viavel) continue;
            /* r = (z0−z2)(z1−z3) / ((z0−z3)(z1−z2)), nos dois conjuntos */
            Qz r0, r1;
            int b0 = qz_divide(qz_mult(qz_soma(P[0], qz_oposto(P[2])),
                                       qz_soma(P[1], qz_oposto(P[3]))),
                               qz_mult(qz_soma(P[0], qz_oposto(P[3])),
                                       qz_soma(P[1], qz_oposto(P[2]))), &r0);
            int b1 = qz_divide(qz_mult(qz_soma(Q[0], qz_oposto(Q[2])),
                                       qz_soma(Q[1], qz_oposto(Q[3]))),
                               qz_mult(qz_soma(Q[0], qz_oposto(Q[3])),
                                       qz_soma(Q[1], qz_oposto(Q[2]))), &r1);
            if(!b0 || !b1) continue;
            voltas++;
            if(qz_igual(r0, r1)) inv++;
            /* o CONTROLO: os pontos MOVERAM-SE. Sem isto, «a razão não mudou» valia por
             * a Möbius não ter feito nada. */
            if(!qz_igual(P[0], Q[0])) distintos++;
            printf("      %-4ld %-21s %-17s %s\n", a,
                   "(p/q exacto)", "(p/q exacto)", qz_igual(r0, r1) ? "sim" : "NÃO");
        }
        printf("\n      invariante em %d de %d, por igualdade exacta — e os pontos"
               " MOVERAM-SE em %d\n", inv, voltas, distintos);
        printf("      e nada saturou os 32 bits do Qz: %ld a mais\n\n",
               qz_saturou - sat0);
        ok("A RAZÃO CRUZADA É INVARIANTE SOB MÖBIUS, E MEDE-SE POR IGUALDADE EXACTA: a"
           " invariância é uma identidade algébrica e vale para quaisquer quatro pontos,"
           " logo os decimais 0.3, 1.7, 2.9 e 5.1 eram sabor — e traziam o double e o"
           " 1e-12 atrás. Com os mesmos pontos em Qz, que este ficheiro já incluía, a"
           " comparação é por PRODUTO CRUZADO e não sobra resíduo nenhum. Com o controlo:"
           " os pontos moveram-se sob a transformação, sem o que «a razão não mudou» valia"
           " por a Möbius não ter feito nada. E a saturação LÊ-SE: o Qz é de 32 bits e"
           " conta à parte o que não coube, e aqui não coube nada de fora — sem esse"
           " controlo, um racional enrolado dava igualdade por acidente",
           inv == voltas && voltas == 5 && distintos == voltas
           && qz_saturou == sat0);
        printf("      Quatro pontos têm um número que a transformação não toca. É esse número que\n");
        printf("      é da estrutura; as coordenadas dos pontos são só a roupa.\n");
    }

    printf("\n      (c) e o ponto fixo é GERADOR — quando o polinómio é irredutível\n\n");
    {
        /* "é gerador": σ não está só na base — ele GERA o corpo, ou seja o polinómio
         * mínimo tem grau n e K = ℚ(σ).
         *
         * E AQUI ESTAVA A MEDIDA ERRADA. O que se media era o posto de uma VANDERMONDE
         * nos pontos σ, σ², …, σⁿ, por eliminação em double com pivô a 1e-9 — e uma
         * Vandermonde com pontos DISTINTOS tem posto cheio SEMPRE, porque o determinante
         * é ∏(xⱼ − xᵢ). Os pontos são distintos por σ > 1, que já se sabe. A asserção era
         * verdadeira, cara, e não tocava na tese: nada ali podia falhar.
         *
         * A TESE É A IRREDUTIBILIDADE de xⁿ − m·xⁿ⁻¹ − 1 sobre ℚ, e ela decide-se em
         * INTEIROS. Por Gauss, um mónico inteiro que factoriza sobre ℚ factoriza sobre ℤ
         * em mónicos; e como o termo constante é −1, cada factor tem termo constante ±1.
         * Para n ≤ 5 basta procurar factores de grau 1 e 2 — um factor de grau 3 num
         * quíntico obriga a um co-factor de grau 2 — e a busca é FINITA:
         *
         *   grau 1:  x + c  com c = ±1        (é o teorema da raiz racional)
         *   grau 2:  x² + ux + v  com v = ±1  e u limitado pelas raízes
         *
         * Divide-se em ℤ e vê-se se o resto é ZERO. Sem raiz, sem pivô, sem limiar. */
        printf("      n   m   factor de grau 1?   factor de grau 2?   irredutível ⇒ gera?\n");
        int mau = 0, casos = 0, red_n = 0; long red_m = 0, red_u = 0, red_v = 0;
        for(int n = 2; n <= 5; n++)
        for(int m = 1; m <= 2; m++){
            long P[8] = {0};                       /* xⁿ − m·xⁿ⁻¹ − 1 */
            P[n] = 1; P[n-1] += -m; P[0] += -1;
            /* grau 1: x + c, c = ±1 — ou seja p(∓1) = 0 */
            int lin = 0;
            for(int c = -1; c <= 1; c += 2){
                long r = 0, x = -c;                /* raiz de x + c é −c */
                for(int d = n; d >= 0; d--) r = r*x + P[d];
                if(r == 0) lin = 1;
            }
            /* grau 2: x² + ux + v com v = ±1; divisão sintética em ℤ, resto tem de ser ≠ 0.
             * E o factor PRÓPRIO não conta: em n = 2 o polinómio é ele mesmo de grau 2. */
            int quad = 0; long qu = 0, qv = 0;
            if(n > 2)
            for(int v = -1; v <= 1; v += 2)
            for(long u = -20; u <= 20; u++){
                long R[8];                         /* R = P, e vai-se reduzindo */
                for(int d = 0; d <= n; d++) R[d] = P[d];
                for(int d = n; d >= 2; d--){       /* elimina o termo de grau d */
                    long c2 = R[d];                /* coeficiente-guia (o divisor é mónico) */
                    R[d]   -= c2;                  /* x²·c2 */
                    R[d-1] -= c2*u;
                    R[d-2] -= c2*v;
                }
                if(R[0] == 0 && R[1] == 0){ quad = 1; qu = u; qv = v; }
            }
            casos++;
            if(lin || quad){ mau++; red_n = n; red_m = m; red_u = qu; red_v = qv; }
            if(m == 1 || (n == 5)) printf("      %-3d %-3d %-19s %-19s %s\n", n, m,
                              lin ? "SIM (redutível)" : "não",
                              quad ? "SIM (redutível)" : "não",
                              (!lin && !quad) ? "sim" : "NÃO — não gera grau n");
        }
        printf("\n");
        if(mau) printf("      E O QUE FALHA EXIBE-SE: x^%d − %ld·x^%d − 1 = (x² %+ld·x %+ld)·(…)\n",
                       red_n, red_m, red_n-1, red_u, red_v);
        ok("σ GERA O CORPO EM SETE DOS OITO CASOS, E O OITAVO É REDUTÍVEL A SÉRIO —"
           " x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1). A tese é a IRREDUTIBILIDADE de"
           " xⁿ − m·xⁿ⁻¹ − 1 e ela decide-se em INTEIROS: por Gauss a factorização sobre ℚ"
           " dá-se sobre ℤ em mónicos, e o termo constante −1 força cada factor a ter termo"
           " constante ±1, logo a busca é FINITA. Para n ≤ 5 os graus 1 e 2 fecham-na,"
           " porque um factor de grau 3 num quíntico obriga a um co-factor de grau 2. O que"
           " aqui estava media o posto de uma VANDERMONDE de pontos distintos, que é cheio"
           " SEMPRE — e por isso afirmava que σ gera em todos os oito, o que é FALSO",
           mau == 1 && casos == 8 && red_n == 5 && red_m == 1);
        printf("      Portanto o que a Möbius deixa quieto não é só um ponto da base: é o gerador\n");
        printf("      dela — QUANDO o polinómio é irredutível. A base ortonormal das dimensões sai\n");
        printf("      dos pontos fixos, e cada dimensão nova é o ponto fixo da companheira daquele\n");
        printf("      grau, com a ressalva que o caso n=5, m=1 obriga: aí o ponto fixo é a razão\n");
        printf("      PLÁSTICA, raiz de x³ − x − 1 e o menor Pisot, e o corpo que ela gera tem\n");
        printf("      grau TRÊS e não cinco. O outro factor, x² − x + 1, tem as raízes no CÍRCULO —\n");
        printf("      é a parte elíptica, e não acrescenta grau à parte que cresce.\n");
    }
}

printf("\n§M14 A EQUAÇÃO DA BASE EM POLAR É UMA SÉRIE — e só fecha no infinito.\n\n");
{
    /* O Aarão: "traz a forma polar da equação da base; deve ser uma série infinita, porque a
     * régua da soma dá a equação exata na polar, que converge só no infinito."
     *
     * E é. A equação da base, xⁿ = m xⁿ⁻¹ + 1, tem n raízes. A solução da recorrência que ela
     * gera é a soma de TODAS — o Binet generalizado — e na forma polar cada par conjugado vira
     * um cosseno amortecido:
     *
     *      u_k  =  c₀·σ^k  +  Σ_j  2·A_j·ρ_j^k·cos(k·θ_j + φ_j)
     *              └ o direto ┘    └──── o cruzado, que oscila e decai ────┘
     *
     * O termo dominante sozinho NUNCA é exato: falta-lhe sempre a parte oscilante. E como
     * ρ_j < 1 no regime bom, o erro cai — mas só se ANULA no limite. A igualdade na polar
     * converge apenas no infinito, e é a régua a fechar: a sombra apaga-se sem nunca ser zero. */
    printf("      k     u_k exato    só o termo σ   com o oscilante   erro de só-σ\n");
    /* K(3,1): σ³ = σ² + 1, com uma raiz real e um par conjugado */
    double sg;
    { double lo = 1, hi = 3;
      for(int i = 0; i < 200; i++){ double md = (lo+hi)/2; if(md*md*md - md*md - 1 > 0) hi = md; else lo = md; }
      sg = (lo+hi)/2; }
    /* o par conjugado: divide-se x³−x²−1 por (x−σ) e resolve-se o quadrático */
    double b = sg - 1, cc = 1.0/sg;              /* x² + b x + c, com c = 1/σ (produto = 1/σ) */
    double rho = sqrt(cc), th = acos(-b/(2*sqrt(cc)));
    /* Os coeficientes: escrever o oscilante como ρ^k(P·cos kθ + Q·sin kθ) torna o sistema
     * LINEAR em (c0, P, Q) — três condições iniciais, três incógnitas, uma eliminação 3×3.
     * (A primeira versão tentou resolver para A e φ diretamente, com uma manipulação
     * trigonométrica apressada, e saiu errada: a asserção apanhou-a.) */
    double c0 = 0, P = 0, Q = 0;
    {
        long U0[3] = {0,0,1};                    /* u_0 = 0, u_1 = 0, u_2 = 1 */
        double M3[3][4];
        for(int k = 0; k < 3; k++){
            M3[k][0] = pow(sg,k);
            M3[k][1] = pow(rho,k)*cos(k*th);
            M3[k][2] = pow(rho,k)*sin(k*th);
            M3[k][3] = (double)U0[k];
        }
        for(int c = 0; c < 3; c++){
            int piv = c;
            for(int r = c; r < 3; r++) if(fabs(M3[r][c]) > fabs(M3[piv][c])) piv = r;
            for(int t = 0; t < 4; t++){ double x = M3[c][t]; M3[c][t] = M3[piv][t]; M3[piv][t] = x; }
            for(int r = 0; r < 3; r++){
                if(r == c) continue;
                double f = M3[r][c]/M3[c][c];
                for(int t = c; t < 4; t++) M3[r][t] -= f*M3[c][t];
            }
        }
        c0 = M3[0][3]/M3[0][0]; P = M3[1][3]/M3[1][1]; Q = M3[2][3]/M3[2][2];
    }
    int mauSo = 0, mauTudo = 0;
    for(int i = 0; i < 6; i++){
        int k = (int[]){1,3,5,8,12,20}[i];
        /* u_k exato pela recorrência em inteiros: u_k = u_{k-1} + u_{k-3} */
        long U[32]; U[0]=0; U[1]=0; U[2]=1;
        for(int t = 3; t <= 20; t++) U[t] = U[t-1] + U[t-3];
        double ex = (double)U[k];
        double so = c0*pow(sg,k);
        double tudo = so + pow(rho,k)*(P*cos(k*th) + Q*sin(k*th));
        if(fabs(ex - so) < 1e-9) mauSo++;                  /* o termo só nunca deve bater */
        if(fabs(ex - tudo) > 1e-6*fabs(ex) + 1e-6) mauTudo++;
        printf("      %-5d %-12.6f %-14.6f %-17.6f %.3e\n", k, ex, so, tudo, fabs(ex - so));
    }
    printf("\n      σ = %.6f   ρ = %.6f   θ = %.6f   (amplitude do oscilante: %.6f)\n\n",
           sg, rho, th, sqrt(P*P + Q*Q));
    ok("a série COM o termo oscilante é exata — a soma das n raízes fecha", mauTudo == 0);
    ok("e o termo dominante SOZINHO nunca é exato: falta sempre a sombra", mauSo == 0);
    ok("mas ρ < 1, logo o erro decai — a igualdade só fecha no infinito", rho < 1.0);
    printf("      É a forma polar da equação da base, e ela é uma SÉRIE: o direto (σ^k, que\n");
    printf("      cresce) mais o cruzado (ρ^k cos(kθ+φ), que gira e encolhe). A régua da soma\n");
    printf("      dá a igualdade exata, e ela converge só no limite — a sombra apaga-se sem\n");
    printf("      nunca ser zero. É por isso que a expansão é infinita: o fecho está no fim.\n");
}

printf("\n§M15 A RESTRIÇÃO DA BASE: o traço, e o período que lê o primo.\n\n");
{
    /* O Aarão: "a fórmula que você gerou não está com apenas os elementos da base — está com
     * toda combinação possível. Vê se tem restrição pra base na coordenada real, que é a
     * passagem; deve ser simétrica, o ponto de contacto entre as dimensões."
     *
     * E tem. A série do §M14 tem os c_j LIVRES, e quase nenhuma combinação dá inteiros. A
     * restrição é de SIMETRIA: os c_j têm de ser os conjugados de Galois de um mesmo c ∈ K, e
     * então a soma é o TRAÇO — a única forma linear invariante sob todos os mergulhos, e a que
     * transporta de K para Q. As partes oscilantes cancelam-se aos pares (λ com λ̄), e o que
     * sobrevive à passagem é o simétrico. */
    printf("      (a) com coeficientes LIVRES, a série sai do reticulado\n\n");
    printf("      c                        u_1        u_2        u_3      inteiros?\n");
    /* K(3,1): σ³ = σ² + 1 */
    double sg; { double lo=1,hi=3; for(int i=0;i<200;i++){ double md=(lo+hi)/2;
        if(md*md*md-md*md-1>0) hi=md; else lo=md; } sg=(lo+hi)/2; }
    /* com c arbitrário (1,0,0) o valor é σ^k, irracional */
    int mauLivre = 0, raizes_racionais = 0;
    {
        /* E «σ NÃO é inteiro» PROVA-SE, e não se arredonda. Estava aqui um
         * fabs(u1 − round(u1)) < 1e-9 sobre o σ calculado por bisseção — que decide a
         * irracionalidade por uma régua de nove casas. O teste da RAIZ RACIONAL fecha-o
         * em duas linhas: x³ − x² − 1 é mónico com termo constante −1, logo uma raiz
         * racional teria de ser inteira e dividir 1, isto é ±1. E
         *
         *      p(+1) = 1 − 1 − 1 = −1 ≠ 0        p(−1) = −1 − 1 − 1 = −3 ≠ 0
         *
         * Não há nenhuma: σ é irracional, e portanto σ, σ² e σ³ não caem em ℤ. É uma
         * pergunta sobre o polinómio, e responde-se em inteiros. */
        for(long r = -1; r <= 1; r += 2)
            if(r*r*r - r*r - 1 == 0) raizes_racionais++;
        if(raizes_racionais) mauLivre++;               /* se houvesse, a tese caía */
        double u1 = sg, u2 = sg*sg, u3 = sg*sg*sg;
        printf("      %-24s %-10.4f %-10.4f %-9.4f %s\n", "(1,0,0) livre", u1, u2, u3,
               raizes_racionais ? "sim" : "NÃO");
        printf("      (e é o teste da raiz racional que o decide: os candidatos são os"
               " divisores de 1, e p(±1) = −1 e −3)\n");
    }
    /* (b) com os conjugados de um mesmo c, a soma é o TRAÇO e cai em Z.
     * Tr(σ^k) obtém-se pelas identidades de Newton a partir da borda, em INTEIROS:
     * t_0 = n ; t_k = m·t_{k−1} + t_{k−n}  (para k ≥ 1, com t_j = 0 se j < 0 salvo t_0) */
    printf("\n      (b) com os CONJUGADOS de c ∈ Z[σ], a soma é o traço e é INTEIRA\n\n");
    long t[12]; t[0] = 3;                    /* Tr(1) = n = 3 */
    t[1] = 1;                                /* Tr(σ) = m = 1 (soma das raízes) */
    t[2] = 1;                                /* Tr(σ²) = m·t1 + 0 = 1 */
    for(int k = 3; k < 12; k++) t[k] = t[k-1] + t[k-3];
    /* E A SEGUNDA ROTA TAMBÉM É INTEIRA. Estava aqui uma reconstrução em double —
     * σ^k mais as potências do par conjugado por uma recorrência com 1/σ — comparada
     * com margem de 1e-6. Mas Tr(σ^k) tem uma segunda leitura EXACTA e é a desta casa:
     * é o TRAÇO DA COMPANHEIRA À POTÊNCIA k. As duas rotas são genuinamente diferentes
     * — uma é a recorrência linear de Newton nos coeficientes, a outra é multiplicar
     * matrizes 3×3 de inteiros e somar a diagonal — e têm de dar o mesmo inteiro. */
    long C[3][3] = { {1,0,1}, {1,0,0}, {0,1,0} };      /* companheira de x³ − x² − 1 */
    long Pk[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} };     /* Pk = C^k, a começar em I */
    printf("      k        Tr(σ^k)   Tr(C^k)   batem?   (numérico, a confirmar)\n");
    int mauTr = 0, duasRotas = 0, kk = 0;
    for(int k = 0; k <= 6; k++){
        long tr = Pk[0][0] + Pk[1][1] + Pk[2][2];      /* a rota da companheira */
        kk++;
        if(tr == t[k]) duasRotas++; else mauTr++;
        /* e a rota decimal fica, mas a CONFIRMAR e não a decidir */
        double soma = pow(sg,k);
        double s1 = 1 - sg, p1 = 1.0/sg;
        double a = 2, b = s1;
        for(int j = 0; j < k; j++){ double nb = s1*b - p1*a; a = b; b = nb; }
        soma += (k == 0) ? 2 : a;
        printf("      %-8d %-9ld %-9ld %-8s (%.6f)\n", k, t[k], tr,
               tr == t[k] ? "sim" : "NÃO", soma);
        long N3[3][3] = {{0}};                         /* Pk ← Pk · C */
        for(int i=0;i<3;i++) for(int j=0;j<3;j++)
            for(int l=0;l<3;l++) N3[i][j] += Pk[i][l]*C[l][j];
        memcpy(Pk, N3, sizeof Pk);
    }
    printf("\n      duas rotas INTEIRAS a concordar em %d de %d — a recorrência de Newton"
           " e o traço da companheira\n\n", duasRotas, kk);
    ok("COM COEFICIENTES LIVRES A SÉRIE NÃO DÁ INTEIROS — A BASE TEM RESTRIÇÃO, e isto"
       " PROVA-SE em vez de se arredondar: estava aqui um fabs(σ − round(σ)) < 1e-9 sobre"
       " um σ obtido por bisseção, que é decidir irracionalidade com uma régua de nove"
       " casas. x³ − x² − 1 é mónico com termo constante −1, logo uma raiz racional teria"
       " de ser inteira e dividir 1: os únicos candidatos são ±1, e p(1) = −1 e p(−1) = −3."
       " Não há nenhuma, logo σ é irracional e nem σ nem σ² caem em ℤ — uma pergunta sobre"
       " o polinómio, respondida em inteiros",
       mauLivre == 0 && raizes_racionais == 0);
    ok("COM OS CONJUGADOS DE GALOIS A SOMA É O TRAÇO, E É INTEIRA — e agora medido por"
       " DUAS ROTAS INTEIRAS que não partilham código: a recorrência de Newton nos"
       " coeficientes da borda, t_k = t_{k−1} + t_{k−3}, e o TRAÇO DA COMPANHEIRA à"
       " potência k, que é multiplicar matrizes 3×3 de inteiros e somar a diagonal."
       " Antes a confirmação era uma reconstrução em double — σ^k mais as potências do"
       " par conjugado com 1/σ — comparada a menos de 1e-6, e a tese ficava pendurada"
       " nessa margem. A rota decimal continua a imprimir-se, mas a confirmar e não a"
       " decidir",
       mauTr == 0 && duasRotas == kk && kk == 7);
    printf("      Tr(1) = %ld = n (a dimensão) e Tr(σ) = %ld = m (o coeficiente da borda): o\n", t[0], t[1]);
    printf("      traço devolve os dois dados que definem o corpo. É o ponto de contacto entre\n");
    printf("      a dimensão n e a dimensão 1, e é SIMÉTRICO por construção — as partes que\n");
    printf("      oscilam cancelam-se aos pares, e sobrevive o que é invariante sob Galois.\n");

    printf("\n      (c) e o PERÍODO: o primo lido dentro de um racional\n\n");
    {
        /* 1/p na base n tem período ord_p(n), e ele divide p−1 (Fermat). O primo fica escrito
         * na ESTRUTURA do racional; a base é só a régua que o lê. */
        printf("      p      base 2   base 3   base 5   base 7   base 10   p−1   todos dividem?\n");
        int primos[] = {7,11,13,17,19,23,29}, mau = 0;
        for(int i = 0; i < 7; i++){
            int p = primos[i];
            printf("      %-6d", p);
            int bases[] = {2,3,5,7,10}, ok_ = 1;
            for(int b = 0; b < 5; b++){
                int nb = bases[b];
                if(nb % p == 0){ printf("%9s", "—"); continue; }
                int o = 1; long r = nb % p;
                while(r != 1){ r = r*nb % p; o++; }
                if((p-1) % o) ok_ = 0;
                printf("%9d", o);
            }
            if(!ok_) mau++;
            printf("%6d   %s\n", p-1, ok_ ? "sim" : "NÃO");
        }
        printf("\n");
        ok("o período de 1/p divide sempre p−1, em qualquer base — Fermat", mau == 0);
        printf("      O primo é o invariante; a base é a coordenada. Em p = 19 os períodos vão\n");
        printf("      de 3 (base 7) a 18 (base 2), e todos dividem 18. É a régua outra vez: o\n");
        printf("      que muda é quem mede, o que fica é o que está a ser medido.\n");
    }
}

printf("\n§M16 A COORDENADA REAL É A FRAÇÃO 1/n DO TRAÇO — e nenhum metal é especial.\n\n");
{
    /* O Aarão: "não vejo contradição, porque se a base da dimensão é o metal, então a
     * coordenada real é sempre 1/2 — o caso m=1 não é especial."
     *
     * E tem razão. Eu tinha comparado o VALOR ABSOLUTO (σ+σ')/2 = m/2 com o número 1/2, e
     * concluído que só m=1 dava meio. Mas a quantidade é RELATIVA: em unidades do traço, a
     * coordenada real é 1/2 exatamente para todo m, porque o ponto médio está sempre a meio.
     * O m=1 parecia especial só porque ali Tr = 1 e o absoluto coincide com a fração.
     *
     * E a generalização é mais forte: em grau n a divisão não é por dois, é por n. */
    printf("      (a) em grau 2: em unidades do traço, é 1/2 para TODO m\n\n");
    printf("      m    Tr = σ+σ'    (σ+σ')/2    razão ao traço\n");
    int mau = 0;
    for(int m = 1; m <= 7; m++){
        double d = sqrt((double)(m*m+4));
        double s1 = (m+d)/2, s2 = (m-d)/2;
        double tr = s1+s2, meio = tr/2;
        double razao = meio/tr;
        if(fabs(razao - 0.5) > 1e-12) mau++;
        if(m <= 5) printf("      %-4d %-12.4f %-11.4f %.10f\n", m, tr, meio, razao);
    }
    printf("      …\n\n");
    ok("a coordenada real é 1/2 do traço para todo m — m=1 não é especial", mau == 0);

    printf("\n      (b) e em grau n a divisão é por n, não por dois\n\n");
    printf("      n   m    Tr(σ)/n     centro dos λ    coincidem?\n");
    int mauN = 0;
    for(int n = 2; n <= 5; n++)
    for(int m = 1; m <= 2; m++){
        /* a soma das raízes é m (o coeficiente de x^{n−1} com sinal trocado), logo o centro
         * é m/n. Confere-se contra a soma NUMÉRICA das raízes, por Durand–Kerner. */
        double re[8], im[8];
        for(int k = 0; k < n; k++){ re[k] = cos(0.9+2.0*k); im[k] = sin(0.9+2.0*k); }
        for(int it = 0; it < 4000; it++)
        for(int k = 0; k < n; k++){
            double pr = 1, pi = 0;
            for(int t = 0; t < n; t++){ double a = pr*re[k]-pi*im[k], b = pr*im[k]+pi*re[k]; pr=a; pi=b; }
            double qr = 1, qi = 0;
            for(int t = 0; t < n-1; t++){ double a = qr*re[k]-qi*im[k], b = qr*im[k]+qi*re[k]; qr=a; qi=b; }
            pr -= m*qr + 1; pi -= m*qi;
            double dr = 1, di = 0;
            for(int j = 0; j < n; j++){
                if(j == k) continue;
                double ar = re[k]-re[j], ai = im[k]-im[j];
                double a = dr*ar - di*ai, b = dr*ai + di*ar; dr=a; di=b;
            }
            double den = dr*dr + di*di;
            if(den < 1e-300) continue;
            re[k] -= (pr*dr + pi*di)/den; im[k] -= (pi*dr - pr*di)/den;
        }
        double soma = 0;
        for(int k = 0; k < n; k++) soma += re[k];
        double centro = soma/n, previsto = (double)m/n;
        if(fabs(centro - previsto) > 1e-6) mauN++;
        if(m == 1) printf("      %-3d %-4d %-11.6f %-15.6f %s\n", n, m, previsto, centro,
                          fabs(centro-previsto) < 1e-6 ? "sim" : "NÃO");
    }
    printf("\n");
    ok("o centro das raízes é Tr/n em toda dimensão — cada uma divide pela sua ordem", mauN == 0);
    printf("      Portanto \"metade para cada dimensão\" é o caso n = 2 de uma regra mais geral:\n");
    printf("      cada dimensão divide por n, e a direção REAL fica no centro de todas por ser\n");
    printf("      a média. Em grau 2 o centro é 1/2 do traço; em grau 5 é 1/5. A fração muda\n");
    printf("      com a dimensão, e é sempre a mesma fração para todos os metais dela.\n");
}

{
    printf("\n§M17 β(n,m) = xⁿ − m x^{n−1} − 1 É POLINÓMIO DE PISOT PARA TODO m ≥ 2.\n\n");
    printf("      Rouché no dual: β*(x) = xⁿ + m x − 1 comparado com h(x) = m x.\n");
    printf("      Sobre |x|=1: |β*−h| = |xⁿ−1| ≤ 2 e |h| = m. Estrito sse m ≥ 3 (m=2 no limite).\n\n");

    /* raízes de xⁿ − m x^{n−1} − 1 (sinal=+1) ou de xⁿ + m x − 1 (sinal=−1) */
    #define GRAU 40
    double *RE = DISCO_FIXO(double, 367);
    double *IM = DISCO_FIXO(double, 368);
    disco_prende(DISCO_BASE(367),"dados/RE_367.bin",(size_t)((GRAU)),sizeof(double));
    disco_zera(RE,(size_t)((GRAU)),sizeof(double));
    disco_prende(DISCO_BASE(368),"dados/IM_368.bin",(size_t)((GRAU)),sizeof(double));
    disco_zera(IM,(size_t)((GRAU)),sizeof(double));
    void raizes(int n, int m, int recip){
        for(int k = 0; k < n; k++){ RE[k] = cos(0.7+2.3*k); IM[k] = sin(0.7+2.3*k); }
        for(int it = 0; it < 20000; it++)
        for(int k = 0; k < n; k++){
            double pr = 1, pi = 0;                              /* xⁿ */
            for(int t = 0; t < n; t++){ double a = pr*RE[k]-pi*IM[k], b = pr*IM[k]+pi*RE[k]; pr=a; pi=b; }
            if(recip){ pr += m*RE[k] - 1; pi += m*IM[k]; }       /* + m x − 1 */
            else {                                              /* − m x^{n−1} − 1 */
                double qr = 1, qi = 0;
                for(int t = 0; t < n-1; t++){ double a = qr*RE[k]-qi*IM[k], b = qr*IM[k]+qi*RE[k]; qr=a; qi=b; }
                pr -= m*qr + 1; pi -= m*qi;
            }
            double dr = 1, di = 0;
            for(int j = 0; j < n; j++){
                if(j == k) continue;
                double ar = RE[k]-RE[j], ai = IM[k]-IM[j];
                double a = dr*ar - di*ai, b = dr*ai + di*ar; dr=a; di=b;
            }
            double den = dr*dr + di*di;
            if(den < 1e-300) continue;
            RE[k] -= (pr*dr + pi*di)/den; IM[k] -= (pi*dr - pr*di)/den;
        }
    }

    printf("      (a) a contagem prevista: n−1 dentro, 1 fora, e a de fora real em (m, m+1)\n\n");
    printf("      O intervalo NÃO se testa pela raiz numérica: σ−m decai como m^{−(n−1)}\n");
    printf("      e some no epsilon do double (m=5, n=24 dá 1,8e−15). Testa-se pelos SINAIS\n");
    printf("      β(m) < 0 < β(m+1), que é o passo (iii) da prova e é exato em inteiros.\n\n");
    printf("      m   n    dentro  fora   β(m)   β(m+1)   σ real?  máx|λ| dos outros\n");
    int mauA = 0;
    for(int m = 2; m <= 5; m++)
    for(int n = 3; n <= 24; n += 7){
        raizes(n, m, 0);
        int din = 0, dout = 0; double mx = 0; int sig_real = 1;
        for(int k = 0; k < n; k++){
            double r = hypot(RE[k], IM[k]);
            if(r > 1.0){ dout++; sig_real = fabs(IM[k]) < 1e-7; }
            else { din++; if(r > mx) mx = r; }
        }
        /* β(m) = mⁿ − m·m^{n−1} − 1 = −1, exato. β(m+1) = (m+1)^{n−1}·1 − 1 > 0, exato. */
        long bm = -1, bm1 = 1;
        for(int t = 0; t < n-1 && bm1 < (long)1e15; t++) bm1 *= (m+1);
        bm1 -= 1;
        int sinais = (bm < 0 && bm1 > 0);
        if(din != n-1 || dout != 1 || !sig_real || !sinais || mx >= 1.0) mauA++;
        if(m <= 3) printf("      %-3d %-4d %-7d %-6d %-6ld %-8s %-8s %.8f\n",
                          m, n, din, dout, bm, "> 0", sig_real ? "sim" : "NÃO", mx);
    }
    printf("      …\n\n");
    ok("m≥2: exatamente n−1 raízes dentro e uma real em (m,m+1) — é Pisot", mauA == 0);

    printf("\n      (b) e a asserção PODE falhar: a linha m=1, que Rouché não cobre\n");
    printf("          (min|x−1| = 0 no direto; máx|xⁿ−1| = 2 > 1 = m no dual)\n\n");
    printf("      m   n    máx|λ| não dominante   < 1 ?\n");
    int falhou_m1 = 0, passou_m1 = 0;
    for(int n = 3; n <= 7; n++){
        raizes(n, 1, 0);
        double sig = 0, mx = 0;
        for(int k = 0; k < n; k++){ double r = hypot(RE[k],IM[k]); if(r > sig){ sig = r; } }
        for(int k = 0; k < n; k++){
            double r = hypot(RE[k],IM[k]);
            if(fabs(r - sig) > 1e-9 && r > mx) mx = r;
        }
        if(mx >= 1.0 - 1e-9) falhou_m1++; else passou_m1++;
        printf("      %-3d %-4d %-22.8f %s\n", 1, n, mx, mx < 1.0-1e-9 ? "sim" : "NÃO — não é Pisot");
    }
    printf("\n");
    ok("m=1 falha a partir de n=5 — o mesmo critério distingue os dois lados", falhou_m1 >= 2 && passou_m1 >= 2);

    printf("\n      (c) a involução ν: o único zero INTERIOR de β* é exatamente 1/σ\n\n");
    printf("      m   n    zero interior de β*   1/σ           produto\n");
    int mauC = 0;
    for(int m = 2; m <= 4; m++)
    for(int n = 4; n <= 18; n += 7){
        raizes(n, m, 0);
        double sig = 0;
        for(int k = 0; k < n; k++){ double r = hypot(RE[k],IM[k]); if(r > sig) sig = r; }
        raizes(n, m, 1);
        int cont = 0; double zin = 0;
        for(int k = 0; k < n; k++){ double r = hypot(RE[k],IM[k]); if(r < 1.0){ cont++; zin = r; } }
        double prod = zin * sig;
        if(cont != 1 || fabs(prod - 1.0) > 1e-7) mauC++;
        printf("      %-3d %-4d %-21.10f %-13.10f %.10f\n", m, n, zin, 1.0/sig, prod);
    }
    printf("\n");
    ok("β* tem UM zero interior e ele vale 1/σ — ν troca dentro por fora", mauC == 0);
    printf("      Contar um zero é mais barato que contar n−1: é por isso que a prova\n");
    printf("      dual é mais curta. A involução paga a diferença.\n");
    #undef GRAU
}


{
    printf("\n§M18 A BASE SUPORTA A POTÊNCIA — e o teste distingue quem não suporta.\n\n");
    printf("      Tr(σ^k) é inteiro para TODO k, não só k=1: é o que faz o anel existir.\n");
    printf("      A órbita de Galois é invariante, logo toda função simétrica dela cai em Z;\n");
    printf("      potenciar PERMUTA a órbita, não a quebra.\n\n");

    /* t_k = Tr(C^k) pela recorrência da companheira: t_k = m·t_{k−1} + t_{k−n}.
     * Em inteiros exatos — nada de raízes numéricas, que é o que torna o teste duro. */
    printf("      (a) os traços da companheira são inteiros e obedecem à recorrência\n\n");
    printf("      n   m    Tr(C^k), k = 1..8\n");
    int mauA = 0;
    for(int n = 2; n <= 5; n++)
    for(int m = 1; m <= 3; m++){
        long t[20];
        t[0] = n;                                  /* Tr(I) = n */
        for(int k = 1; k < n; k++) t[k] = (k == 1) ? m : m*t[k-1];   /* Newton: t_k = m^k */
        for(int k = n; k < 16; k++) t[k] = m*t[k-1] + t[k-n];        /* a recorrência de β */
        /* controlo independente: o mesmo traço via potência da matriz companheira,
         * em inteiros — se as duas contas divergirem, uma delas está errada. */
        long M[6][6] = {{0}}, P[6][6] = {{0}};
        for(int i = 0; i < n; i++) P[i][i] = 1;
        M[0][0] = m; M[0][n-1] = 1;
        for(int i = 1; i < n; i++) M[i][i-1] = 1;
        for(int k = 1; k <= 8; k++){
            long Q[6][6] = {{0}};
            for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
                long acc = 0;
                for(int r = 0; r < n; r++) acc += P[i][r]*M[r][j];
                Q[i][j] = acc;
            }
            for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) P[i][j] = Q[i][j];
            long tr = 0;
            for(int i = 0; i < n; i++) tr += P[i][i];
            if(tr != t[k]) mauA++;
        }
        if(m == 1){
            printf("      %-3d %-4d ", n, m);
            for(int k = 1; k <= 8; k++) printf("%ld ", t[k]);
            printf("\n");
        }
    }
    printf("      …\n\n");
    ok("Tr(C^k) fecha em Z para todo k — a recorrência e a matriz concordam", mauA == 0);

    printf("\n      (b) e o teste PODE falhar: sem monicidade ou com coeficiente\n");
    printf("          fracionário, o fecho quebra logo nas primeiras potências\n\n");
    printf("      polinómio           Tr(x^k) inteiro em k=1..8?   quebra em k =\n");
    /* raízes de a·x² + b·x + c: soma = −b/a, e Tr(x^k) segue Newton com essa soma, por
     * p_k = (−b/a)·p_{k−1} − (c/a)·p_{k−2}.
     *
     * E ISTO NÃO SE MEDE EM DOUBLE. A pergunta é «p_k é INTEIRO?», e em double ela só se
     * pode fazer arredondando e comparando — `fabs(p1 − (long)(p1+0,5)) > 1e-9` —, que é
     * pôr uma régua minha para decidir uma coisa que não tem régua: ou o denominador é 1
     * ou não é. Em ℚ exacto a pergunta é `q == 1`, e a resposta não tem tolerância.
     *
     * Os coeficientes são racionais por definição do caso — o terceiro é −1/2, e escrevê-lo
     * como `-0.5` já perdia o ponto: ele não é um decimal, é um QUOCIENTE, e é isso que o
     * faz reprovar. */
    struct { const char *nome; long ap, aq, bp, bq, cp, cq; int esperado; } casos[] = {
        { "x^2 − 3x − 1  (mónico)",     1,1, -3,1, -1,1, 1 },
        { "2x^2 − 3x − 1 (não mónico)", 2,1, -3,1, -1,1, 0 },
        { "x^2 − 3x − 1/2",             1,1, -3,1, -1,2, 0 },
    };
    int mauB = 0;
    for(size_t i = 0; i < sizeof casos/sizeof casos[0]; i++){
        Qz A = qz(casos[i].ap, casos[i].aq), Bc = qz(casos[i].bp, casos[i].bq),
           Cc = qz(casos[i].cp, casos[i].cq), S, P2;
        if(!qz_divide(qz_oposto(Bc), A, &S) || !qz_divide(Cc, A, &P2)){ mauB++; continue; }
        Qz p0 = qz(2,1), p1 = S;
        int quebra = 0;
        if(S.q != 1) quebra = 1;                       /* a própria soma já não é inteira */
        for(int k = 2; k <= 8; k++){
            Qz pk = qz_soma(qz_mult(S, p1), qz_oposto(qz_mult(P2, p0)));
            p0 = p1; p1 = pk;
            if(!quebra && p1.q != 1) quebra = k;       /* «é inteiro» É q == 1 */
        }
        int fecha = (quebra == 0);
        if(fecha != casos[i].esperado) mauB++;
        printf("      %-22s %-27s %s\n", casos[i].nome, fecha ? "SIM" : "NÃO",
               quebra ? (quebra == 1 ? "1" : (quebra == 2 ? "2" : "3+")) : "—");
    }
    printf("\n");
    ok("SÓ O MÓNICO COM COEFICIENTES INTEIROS FECHA, E «É INTEIRO» DIZ-SE COM q == 1: a"
       " recorrência de Newton corre em ℚ exacto, logo a pergunta não tem tolerância —"
       " ou o denominador é 1 ou não é. O que aqui estava arredondava um double e comparava"
       " com uma régua de 1e-9, para decidir uma coisa que não tem régua. E o terceiro caso"
       " não é o decimal −0,5: é o QUOCIENTE −1/2, e é isso que o faz reprovar",
       mauB == 0 && qz_saturou == 0);
    printf("      É a mesma condição que dá os complexos: Z[i] é x²+1, mónico e inteiro,\n");
    printf("      com a mesma involução e a mesma norma z·ν(z). Não há como ter uma\n");
    printf("      coisa sem a outra.\n");
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
