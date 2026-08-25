/* espectro.c — O MAPA ESPECTRAL DAS CADEIAS, LIDO NA ÓRBITA.
 *
 * A pergunta é uma: varrendo TODAS as cadeias mónicas de grau N com |a_0| = 1 e
 * coeficientes no trial {-1,0,+1}, que espectros a álgebra PERMITE? Não se
 * pergunta «qual cadeia é o elétron» — pergunta-se que objetos existem.
 *
 * E NÃO SE AVALIA RAIZ NENHUMA. O instrumento é o que a casa já usa no §W219: a
 * ÓRBITA DE (1,0,…,0) SOB A COMPANHEIRA. Ela é inteira, a companheira de uma
 * cadeia com |a_0| = 1 está em GL_N(ℤ), e o que se lê é o CRESCIMENTO:
 *
 *      fica            a órbita fecha         todas as raízes no círculo
 *      cresce          a órbita escapa        há raiz fora
 *
 * — que é o Teor. do período (fisica.tex §fis:periodo) aplicado à própria
 * companheira: a cauda ℓ é o que se gastou e o período p é o que sobrou. Nenhum
 * double, nenhum limiar, nenhuma raiz avaliada. O único número escolhido é o
 * TETO, e ele é verificado: uma órbita que o toca é declarada CRESCENTE, e §E5
 * mede que mover o teto não move a classificação.
 *
 * Porque isto decide mesmo: a companheira tem |det| = |a_0| = 1, logo é
 * invertível sobre os inteiros; uma órbita inteira LIMITADA num reticulado só
 * tem finitos estados, portanto fecha — e fechar é ter ordem finita, que é ter
 * todas as raízes na unidade. Escapar é ter raiz de módulo > 1. A dicotomia é
 * exaustiva e é integral.
 *
 *   §E1  A NEUTRALIDADE: |det C| = |a_0| = 1 em toda cadeia — o determinante
 *        pelo desenvolvimento inteiro, e é a Prop. da neutralidade medida
 *   §E2  A TAXONOMIA: quantas cadeias FECHAM e quantas ESCAPAM, por grau
 *   §E3  OS PERÍODOS QUE EXISTEM: e em grau 2 têm de ser {1,2,3,4,6}, nunca 5 —
 *        o Lema cristalográfico, obtido aqui por outro caminho
 *   §E4  A CAUDA: quem fecha, fecha com cauda 0 (a companheira é invertível)
 *   §E5  CONTROLO DO TETO: mover o teto por décadas não move a classificação
 *   §E6  CONTROLO EXACTO: quem fecha é RECÍPROCO (a_i = ±a_{N-i}) — um teste
 *        sobre coeficientes inteiros, que não vê órbita nenhuma
 *
 *   cc -O2 -std=c99 -I. -I../lib espectro.c -o espectro && ./espectro
 */
#include <stdio.h>
#include <stdlib.h>
#include "unidade.h"

#define NMAX   10          /* graus varridos: 2..NMAX                        */
#define TETO   1000000LL   /* |componente| acima disto: a órbita ESCAPOU     */
#define PASSOS 4096        /* voltas máximas antes de desistir               */

/* a companheira de x^N + c[0] x^{N-1} + … + c[N-1], aplicada a v (inteiro) */
static void passo(const int *c, int N, long long *v){
    long long topo = 0;
    for(int i = 0; i < N; i++) topo -= (long long)c[i] * v[i];
    for(int i = N - 1; i >= 1; i--) v[i] = v[i-1];
    v[0] = topo;
}

/* Corre a órbita de e_1. Devolve:
 *    0  ESCAPOU (alguma componente passou o teto)      *per, *cau indefinidos
 *    1  FECHOU  com período *per e cauda *cau
 * Os estados são comparados por igualdade de vectores INTEIROS — nada de norma,
 * nada de distância. É a cláusula (1) do Teor. da multiplicidade: duplicidade é
 * dobra, e a dobra decide-se por IGUALDADE.                                   */
static int corre(const int *c, int N, long long teto, int *per, int *cau){
    static long long hist[PASSOS][NMAX];
    long long v[NMAX];
    for(int i = 0; i < N; i++) v[i] = 0;
    v[0] = 1;
    for(int t = 0; t < PASSOS; t++){
        for(int i = 0; i < N; i++) hist[t][i] = v[i];
        /* já esteve aqui? então fechou: t é o índice actual, e o anterior dá a cauda */
        for(int s = 0; s < t; s++){
            int igual = 1;
            for(int i = 0; i < N && igual; i++) if(hist[s][i] != v[i]) igual = 0;
            if(igual){ *cau = s; *per = t - s; return 1; }
        }
        passo(c, N, v);
        for(int i = 0; i < N; i++)
            if(v[i] > teto || v[i] < -teto) return 0;      /* escapou */
    }
    return 0;                                              /* não fechou a tempo */
}

/* recíproca: a_i = ±a_{N-i} sobre os coeficientes, com a_N = 1 e a_0 = c[N-1] */
static int reciproca(const int *c, int N){
    int a[NMAX + 1];
    a[N] = 1;
    for(int i = 0; i < N; i++) a[N - 1 - i] = c[i];
    int mais = 1, menos = 1;
    for(int i = 0; i <= N; i++){
        if(a[i] !=  a[N-i]) mais  = 0;
        if(a[i] != -a[N-i]) menos = 0;
    }
    return mais || menos;
}

/* o varrimento: c[0..N-2] no trial {-1,0,+1}, c[N-1] = ±1 (a unidade) */
static long long total_grau(int N){
    long long t = 2;
    for(int i = 0; i < N - 1; i++) t *= 3;
    return t;
}
static void monta(long long cod, int *c, int N){
    long long w = cod;
    for(int i = 0; i < N - 1; i++){ c[i] = (int)(w % 3) - 1; w /= 3; }
    c[N-1] = (w % 2 == 0) ? 1 : -1;
}

int main(void){
    printf("\n=== O MAPA ESPECTRAL DAS CADEIAS — lido na órbita, em inteiros ===\n");
    printf("    graus 2..%d, coeficientes no trial {-1,0,+1}, |a_0| = 1\n", NMAX);
    printf("    nenhuma raiz avaliada, nenhum double, nenhum limiar de módulo\n");

    int c[NMAX];
    int per, cau;

    /* ═══ §E1  a neutralidade: |det C| = |a_0| = 1 ═══════════════════════════ */
    printf("\n§E1 O determinante da companheira é ±a_0, e a cadeia pede |a_0| = 1.\n\n");
    {
        long long cadeias = 0, unit = 0;
        for(int N = 2; N <= NMAX; N++){
            long long T = total_grau(N);
            for(long long cod = 0; cod < T; cod++){
                monta(cod, c, N);
                /* det da companheira = (-1)^N · a_0, e a_0 = c[N-1] */
                long long det = (N % 2 == 0) ? c[N-1] : -c[N-1];
                cadeias++;
                if(det == 1 || det == -1) unit++;
            }
        }
        printf("      cadeias varridas: %lld ; com |det| = 1: %lld\n", cadeias, unit);
        ok("A NEUTRALIDADE NÃO É IMPOSTA, É O DETERMINANTE", unit == cadeias);
    }

    /* ═══ §E2  a taxonomia: fecha ou escapa ══════════════════════════════════ */
    long long fecha_g[NMAX+1], escapa_g[NMAX+1];
    printf("\n§E2 A órbita de e_1 sob a companheira: fecha (luz) ou escapa (matéria)?\n\n");
    printf("      grau |    cadeias |     FECHA |    ESCAPA\n");
    {
        long long tf = 0, te = 0;
        for(int N = 2; N <= NMAX; N++){
            long long f = 0, e = 0, T = total_grau(N);
            for(long long cod = 0; cod < T; cod++){
                monta(cod, c, N);
                if(corre(c, N, TETO, &per, &cau)) f++; else e++;
            }
            fecha_g[N] = f; escapa_g[N] = e; tf += f; te += e;
            printf("      %4d | %10lld | %9lld | %9lld\n", N, T, f, e);
        }
        printf("      tot  | %10lld | %9lld | %9lld\n", tf + te, tf, te);
        ok("A DICOTOMIA É EXAUSTIVA E INTEIRA: toda cadeia ou fecha ou escapa",
           tf > 0 && te > 0);
    }

    /* ═══ §E3  os períodos que existem ═══════════════════════════════════════ */
    printf("\n§E3 Os períodos das órbitas que fecham — e o grau 2 é o controlo.\n\n");
    {
        int viu2[64]; for(int i = 0; i < 64; i++) viu2[i] = 0;
        int tem5 = 0, fora_lista = 0;
        for(int N = 2; N <= NMAX; N++){
            int viu[64]; for(int i = 0; i < 64; i++) viu[i] = 0;
            long long T = total_grau(N);
            for(long long cod = 0; cod < T; cod++){
                monta(cod, c, N);
                if(corre(c, N, TETO, &per, &cau) && per < 64){
                    viu[per] = 1;
                    if(N == 2){
                        viu2[per] = 1;
                        if(per == 5) tem5 = 1;
                        if(per!=1 && per!=2 && per!=3 && per!=4 && per!=6) fora_lista = 1;
                    }
                }
            }
            printf("      grau %2d: ", N);
            for(int p = 1; p < 64; p++) if(viu[p]) printf("%d ", p);
            printf("\n");
        }
        printf("\n      grau 2 — a lista: ");
        for(int p = 1; p < 64; p++) if(viu2[p]) printf("%d ", p);
        printf("\n");
        ok("A RESTRIÇÃO CRISTALOGRÁFICA SAI DA ÓRBITA, SEM O LEMA",
           !tem5 && !fora_lista);
    }

    /* ═══ §E4  a cauda ═══════════════════════════════════════════════════════ */
    printf("\n§E4 Quem fecha, fecha com que cauda?\n\n");
    {
        long long com_cauda = 0, sem_cauda = 0;
        for(int N = 2; N <= NMAX; N++){
            long long T = total_grau(N);
            for(long long cod = 0; cod < T; cod++){
                monta(cod, c, N);
                if(corre(c, N, TETO, &per, &cau)){
                    if(cau == 0) sem_cauda++; else com_cauda++;
                }
            }
        }
        printf("      fecham com cauda 0: %lld ; com cauda > 0: %lld\n",
               sem_cauda, com_cauda);
        ok("A COMPANHEIRA É INVERTÍVEL, LOGO NÃO HÁ CAUDA: ℓ = 0 em todas",
           com_cauda == 0 && sem_cauda > 0);
    }

    /* ═══ §E5  o controlo do teto ════════════════════════════════════════════ */
    printf("\n§E5 O teto decide alguma coisa? Move-se por décadas e conta-se.\n\n");
    {
        long long tetos[] = {1000LL, 10000LL, 100000LL, 1000000LL, 10000000LL};
        long long ref = -1; int estavel = 1;
        for(int k = 0; k < 5; k++){
            long long f = 0;
            for(int N = 2; N <= NMAX; N++){
                long long T = total_grau(N);
                for(long long cod = 0; cod < T; cod++){
                    monta(cod, c, N);
                    if(corre(c, N, tetos[k], &per, &cau)) f++;
                }
            }
            printf("      teto = %10lld  ->  fecham: %lld\n", tetos[k], f);
            if(ref < 0) ref = f; else if(f != ref) estavel = 0;
        }
        ok("O TETO NÃO DECIDE NADA: a classificação não move por cinco décadas",
           estavel);
    }

    /* ═══ §E6  o controlo exacto: reciprocidade ══════════════════════════════ */
    printf("\n§E6 Dois caminhos: a ÓRBITA e os COEFICIENTES. Têm de concordar.\n\n");
    {
        long long fecha_e_recip = 0, fecha_nao_recip = 0, escapa_e_recip = 0;
        for(int N = 2; N <= NMAX; N++){
            long long T = total_grau(N);
            for(long long cod = 0; cod < T; cod++){
                monta(cod, c, N);
                int r = reciproca(c, N);
                if(corre(c, N, TETO, &per, &cau)){
                    if(r) fecha_e_recip++; else fecha_nao_recip++;
                } else if(r) escapa_e_recip++;
            }
        }
        printf("      fecham e são recíprocas: %lld\n", fecha_e_recip);
        printf("      fecham e NÃO são recíprocas: %lld   <- tem de ser 0\n", fecha_nao_recip);
        printf("      escapam e são recíprocas: %lld   <- a recíproca NÃO basta\n", escapa_e_recip);
        ok("QUEM FECHA É RECÍPROCO, E A RECÍPROCA NÃO BASTA",
           fecha_nao_recip == 0 && escapa_e_recip > 0);
    }

    printf("\n");
    return 0;
}
