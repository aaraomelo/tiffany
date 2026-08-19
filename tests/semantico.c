/* semantico.c — O ESPAÇO VETORIAL SEMÂNTICO: a transfusão nos dois sentidos.
 *
 * O Aarão: "aprofunda no espaço vetorial semântico, vê a transfusão nos dois sentidos via
 * embeddings puros."
 *
 * A PERGUNTA: **o espaço semântico tem as duas metades?**
 *
 * Toda a prática de embeddings usa o COSSENO — o DIRETO, simétrico, o que MEDE. O §R7
 * mediu que a norma só fecha com as duas peças. Em 768 dimensões não há produto vetorial
 * (Hurwitz: 1, 3, 7), mas há o bivetor a∧b, e Lagrange fecha:
 *
 *      ‖a∧b‖² + ⟨a,b⟩² = ‖a‖²‖b‖²
 *
 *   §S1  o espaço: o interno é simétrico — a metade que MEDE
 *   §S2  A OUTRA METADE: o bivetor a∧b, antissimétrico, em qualquer dimensão
 *   §S3  O FECHO: Lagrange por DUAS rotas, e o interno SOZINHO não fecha
 *   §S4  a TRANSFUSÃO, ida: o vector entra pela Hadamard (a dobra do §F11)
 *   §S5  a TRANSFUSÃO, volta: a Hadamard devolve o vector — resíduo 0 em ℤ
 *   §S6  os DOIS SENTIDOS: Parseval, o interno atravessa intacto
 *
 * LEI vs TRANSPORTE. Os embeddings do ollama em vírgula, 1e-12 no resíduo e o sqrt
 * da Hadamard eram o método. A lei é Lagrange (Frobenius contra aa·bb−ab²) em ℤ,
 * a antissimetria polinomial, a Hadamard HᵀH = n I, Parseval ⟨Ha,Hb⟩ = n⟨a,b⟩ —
 * e inventar os vectores do modelo seria pior do que medi-los onde a identidade vive.
 *
 *   cc -O2 -std=c99 -I lib tests/semantico.c -o semantico && ./semantico
 */
#include <stdio.h>
#include "unidade.h"

#define D768 768
#define N8     8

static long ip(const long *a, const long *b, int n){
    long s = 0;
    for(int i = 0; i < n; i += 1) s += a[i]*b[i];
    return s;
}
static long biv_ij(const long *a, const long *b, int i, int j){
    return a[i]*b[j] - a[j]*b[i];
}
/* Sylvester: H_{2m} pela dobra do hopfield.c §F11. H[i][j] ∈ {+1,−1}. */
static void hadamard(int n, signed char H[N8][N8]){
    for(int i = 0; i < n; i += 1) for(int j = 0; j < n; j += 1) H[i][j] = 0;
    H[0][0] = 1;
    for(int m = 1; m < n; m *= 2)
        for(int i = 0; i < m; i += 1)
            for(int j = 0; j < m; j += 1){
                H[i][j+m] = H[i][j];
                H[i+m][j] = H[i][j];
                H[i+m][j+m] = (signed char)(-H[i][j]);
            }
}

int main(void){
    puts("semantico.c — O ESPACO VETORIAL SEMANTICO: a transfusao nos dois sentidos\n");

    /* vectores inteiros: a identidade é homogénea de grau 4, e não pede o modelo. */
    long A[8]  = { 1, 2, 3, 4, 5, 6, 7, 8 };
    long B[8]  = { 8, 7, 6, 5, 4, 3, 2, 1 };
    long E1[8] = { 1, 0, 0, 0, 0, 0, 0, 0 };
    long E2[8] = { 0, 1, 0, 0, 0, 0, 0, 0 };
    long P[8]  = { 1, 2, 3, 4, 5, 6, 7, 8 };
    long Q[8]  = { 2, 4, 6, 8, 10, 12, 14, 16 };   /* 2·P, paralelo */

    printf("§S1  O ESPACO: dimensao, norma, e o INTERNO que toda a gente usa\n");
    printf("     O cosseno e o produto interno normalizado — e o interno e o DIRETO.\n\n");
    {
        int pares = 0, simetricos = 0;
        const long *V[] = { A, B, E1, E2, P, Q };
        int NV = 6;
        for(int i = 0; i < NV; i += 1)
            for(int j = 0; j < NV; j += 1){
                pares += 1;
                if(ip(V[i], V[j], N8) == ip(V[j], V[i], N8)) simetricos += 1;
            }
        printf("     -> %d pares em dim %d; simetricos: %d\n", pares, N8, simetricos);
        ok("o INTERNO e simetrico em todos os pares — trocar a ordem nao muda nada, ele so MEDE."
           " Os embeddings do ollama eram transporte; a simetria e' bilinear em Z",
           simetricos == pares && pares > 0);
        printf("        ele para: e a ordem 2 do §F7.\n\n");
    }

    printf("§S2  A OUTRA METADE: o BIVETOR a^b, e ele existe em 768 dimensoes\n");
    printf("     Em 768 dim nao ha produto vetorial (Hurwitz: 1, 3, 7). O bivetor existe.\n\n");
    {
        int anti = 1, testados = 0;
        long anti_ex = 0;
        for(int k = 0; k < N8; k += 1)
            for(int l = 0; l < N8; l += 1){
                long x = biv_ij(A, B, k, l), y = biv_ij(A, B, l, k);
                if(x + y == 0) anti_ex += 1;
                else anti = 0;
                testados += 1;
            }
        printf("     -> soma x+y ZERO EXACTO em %ld de %d componentes\n", anti_ex, testados);
        ok("o BIVETOR e antissimetrico: (a^b)_ij = -(a^b)_ji — e o zero e' EXACTO,"
           " porque a lei e' polinomial",
           anti && anti_ex == testados && testados == N8*N8);

        int troca = 1; long troca_ex = 0, troca_tot = 0;
        for(int k = 0; k < N8; k += 1)
            for(int l = 0; l < N8; l += 1){
                long so = biv_ij(A, B, k, l) + biv_ij(B, A, k, l);
                if(so == 0) troca_ex += 1;
                else troca = 0;
                troca_tot += 1;
            }
        ok("e a^b = -(b^a): a peca que ORDENA, e ela existe no espaco — zero EXACTO",
           troca && troca_tot > 0 && troca_ex == troca_tot);

        long b2 = ip(A,A,N8)*ip(B,B,N8) - ip(A,B,N8)*ip(A,B,N8);
        ok("e ele NAO e nulo: ha mesmo uma segunda metade, nao e uma peca vazia",
           b2 > 0);
        printf("     -> ||a^b||^2 (forma fechada) = %ld. A metade que ORDENA esta la.\n\n", b2);
    }

    printf("§S3  O FECHO: a identidade de Lagrange — e o interno SOZINHO nao fecha\n");
    printf("     ||a^b||^2 + <a,b>^2 = ||a||^2.||b||^2. Duas rotas, sem uma linha em comum.\n\n");
    {
        /* rota 1: Σ_{i<j} (a_i b_j − a_j b_i)²     rota 2: ‖a‖²‖b‖² − ⟨a,b⟩² */
        const long *U[] = { A, B, E1, E2, P, Q };
        int NU = 6;
        long z_pares = 0, z_fecham = 0, z_vivos = 0;
        for(int i = 0; i < NU; i += 1) for(int j = 0; j < NU; j += 1){
            const long *x = U[i], *y = U[j];
            long aa = ip(x,x,N8), bb = ip(y,y,N8), ab = ip(x,y,N8);
            long fechada = aa*bb - ab*ab;
            long soma = 0;
            for(int k = 0; k < N8; k += 1) for(int l = k+1; l < N8; l += 1){
                long c = biv_ij(x, y, k, l);
                soma += c*c;
            }
            z_pares += 1;
            if(soma == fechada) z_fecham += 1;
            if(fechada != 0) z_vivos += 1;
        }
        printf("     -> dim %d: %ld pares, Frobenius bate forma fechada em %ld, vivos %ld\n",
               N8, z_pares, z_fecham, z_vivos);
        ok("LAGRANGE MEDE-SE AGORA, E EM INTEIROS: ||a^b||^2 somado componente a componente"
           " bate a forma fechada com residuo ZERO EXACTO. O que aqui estava comparava a"
           " DEFINICAO consigo propria — `biv2` E' aa.bb - ab^2. A segunda rota e' Frobenius",
           z_fecham == z_pares && z_vivos > 0 && z_pares > 0);

        /* e em 768: UM par, as duas rotas. Entradas pequenas para caber no long. */
        long X[D768], Y[D768];
        for(int d = 0; d < D768; d += 1){
            X[d] = (d % 5) - 2;          /* −2..2 */
            Y[d] = ((d * 3) % 7) - 3;    /* −3..3 */
        }
        long aa = 0, bb = 0, ab = 0, soma = 0;
        for(int d = 0; d < D768; d += 1){
            aa += X[d]*X[d]; bb += Y[d]*Y[d]; ab += X[d]*Y[d];
        }
        for(int k = 0; k < D768; k += 1) for(int l = k+1; l < D768; l += 1){
            long c = X[k]*Y[l] - X[l]*Y[k];
            soma += c*c;
        }
        long fechada = aa*bb - ab*ab;
        printf("     -> e em dim 768 (um par): Frobenius %ld  forma %ld\n", soma, fechada);
        ok("e em 768 dimensoes a mesma identidade fecha — Hurwitz nao obsta o bivetor."
           " Os milesimos dos embeddings transbordavam o long; aqui as entradas cabem",
           soma == fechada && fechada != 0);

        /* interno sozinho: paralelo (falta 0) vs ortogonal (falta tudo) */
        long tot_par = ip(P,P,N8)*ip(Q,Q,N8), dir_par = ip(P,Q,N8)*ip(P,Q,N8);
        long tot_ort = ip(E1,E1,N8)*ip(E2,E2,N8), dir_ort = ip(E1,E2,N8)*ip(E1,E2,N8);
        int falta_ort = (2*(tot_ort - dir_ort) > tot_ort);     /* falta > 1/2 */
        int falta_par = (tot_par - dir_par == 0);              /* paralelo: bivetor nulo */
        printf("     -> paralelo: falta 0 da norma;  ortogonal: falta mais de metade? %s\n\n",
               falta_ort ? "sim" : "nao");
        ok("e com o INTERNO sozinho falta a maior parte — o bivetor nao e enfeite."
           " 2.(aa.bb-ab^2)>aa.bb no par ORTOGONAL, e no PARALELO falta zero: o contraste"
           " e' um numero, nao a media dos embeddings",
           falta_ort && falta_par && tot_ort > 0);
    }

    printf("§S4  A TRANSFUSAO, IDA: o vector entra no corpo pela base ortonormal\n");
    printf("§S5  E VOLTA: o corpo devolve o vector, e o residuo e ZERO em Z\n\n");
    {
        signed char H[N8][N8];
        hadamard(N8, H);
        /* Hᵀ H = n I: as linhas são ortogonais de norma n. */
        int malH = 0;
        for(int i = 0; i < N8; i += 1) for(int j = 0; j < N8; j += 1){
            long s = 0;
            for(int k = 0; k < N8; k += 1) s += (long)H[i][k] * (long)H[j][k];
            long esp = (i == j) ? N8 : 0;
            if(s != esp) malH += 1;
        }
        printf("     a base: Hadamard %dx%d (a dobra do §F11)\n", N8, N8);

        /* IDA: coef_c = Σ_i H[c][i] x[i]     (sem /n)
         * VOLTA: rec_i = Σ_c H[c][i] coef_c  deve igualar n·x_i  */
        long coef[N8], rec[N8];
        for(int c = 0; c < N8; c += 1){
            long s = 0;
            for(int i = 0; i < N8; i += 1) s += (long)H[c][i] * A[i];
            coef[c] = s;
        }
        int malR = 0;
        for(int i = 0; i < N8; i += 1){
            long s = 0;
            for(int c = 0; c < N8; c += 1) s += coef[c] * (long)H[c][i];
            rec[i] = s;
            if(rec[i] != (long)N8 * A[i]) malR += 1;
        }
        printf("     volta: rec = n·x em %d de %d coordenadas (residuo 0, sem 1e-12)\n\n",
               N8 - malR, N8);
        ok("A VOLTA FECHA com a base completa: H^T H = n I, logo rec = n x, exacto em Z."
           " O residuo sqrt(num/den) era IEEE sobre embeddings; a dobra e' a do §F11",
           malH == 0 && malR == 0);
    }

    printf("§S6  OS DOIS SENTIDOS NUM CIRCUITO SO, e onde ele fecha\n\n");
    printf("        IDA    o vector -> coeficientes na base (a torre BRANCA, desce e projeta)\n");
    printf("        VOLTA  os coeficientes -> o vector      (a torre NEGRA, sobe e recompoe)\n\n");
    {
        signed char H[N8][N8];
        hadamard(N8, H);
        long ca[N8], cb[N8];
        for(int c = 0; c < N8; c += 1){
            long sa = 0, sb = 0;
            for(int i = 0; i < N8; i += 1){
                sa += (long)H[c][i] * A[i];
                sb += (long)H[c][i] * B[i];
            }
            ca[c] = sa; cb[c] = sb;
        }
        long ip_orig = ip(A, B, N8);
        long ip_coef = ip(ca, cb, N8);
        /* Parseval: ⟨Ha, Hb⟩ = n ⟨a,b⟩  (linhas de norma n, ortogonais) */
        printf("     -> <a,b> = %ld no espaco e %ld nos coeficientes (quer n·<a,b> = %ld).\n",
               ip_orig, ip_coef, (long)N8 * ip_orig);
        ok("o INTERNO atravessa a transfusao intacto — Parseval <Ha,Hb> = n<a,b>, em Z."
           " 1e-12 sobre sqrt(n) era IEEE",
           ip_coef == (long)N8 * ip_orig);
        printf("        O que se transfunde nao e o vetor: e a MEDIDA dele, e ela conserva-se.\n\n");
    }

    puts("O que isto fecha:");
    puts("  O espaco TEM as duas metades. O interno mede e o bivetor ordena — Lagrange");
    puts("  fecha os dois. O que falta nao e ao espaco: e a PRATICA, que so olha para o cosseno.");
    printf("\n");
    return falhas ? 1 : 0;
}
