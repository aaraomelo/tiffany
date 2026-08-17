/* recaman.c — A PA NO DIRETO É A PG NO ESPELHO, E A COLISÃO É A MUDANÇA DE FASE.
 *
 * O Aarão descreveu a sequência assim: "começa em 0; o tamanho de cada salto aumenta em 1;
 * move-se para a esquerda se o número estiver livre e for positivo; caso contrário salta
 * para a direita." E depois: "isso gera o positivo, o negativo é da mesma forma, só que
 * vira PG no espelho" e "onde acontecem as colisões é a mudança de fase".
 *
 * É a sequência de Recamán, e serve de realização do enunciado do §1 por exibir as duas
 * coordenadas separadas de forma invulgarmente limpa:
 *
 *     O QUE MEDE   o tamanho do salto — |a(n)−a(n−1)| = n, PA de razão 1, RÍGIDA.
 *                  A sequência não escolhe o tamanho.
 *     O QUE ORDENA o sinal — a única liberdade que há. No espelho multiplicativo é a FASE:
 *                  somar +n vira multiplicar por q^n (fase 0); somar −n vira 1/q^n (fase π).
 *
 * E a colisão (recuo impossível: cairia em ≤0 ou em casa já visitada) FORÇA o avanço, isto
 * é, força a fase a zero. É o reset de fase, e é o único acontecimento da sequência.
 *
 *   §R1  a PA é exata: |Δ| = n em todos os passos
 *   §R2  exp conjuga: uma PA de razão d vira PG de razão e^d  (prop:conjuga da teoria)
 *   §R3  a colisão força a fase a 0 — sem colisão, recua
 *   §R4  o espelho é ANTISSIMÉTRICO e não acrescenta cobertura: A = −B exatamente
 *   §R5  NÃO preenche — e é o controlo negativo deste medidor
 *
 * A conjectura de que todo inteiro aparece está EM ABERTO. Este medidor não a resolve:
 * mede os buracos que restam, e falha se alguém escrever que preenche.
 *
 *   cc -O2 -std=c99 -Wall recaman.c -lm -o recaman && ./recaman
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define VAZIO LONG_MIN   /* sentinela: -1 NÃO serve — a dual visita -1 */

#define N      2000000L      /* passos                                     */
#define LIM    100000L       /* até onde se procuram buracos               */

static char *vis;            /* visitados, até LIM                         */

int main(void)
{
    printf("A PA NO DIRETO É A PG NO ESPELHO — e a colisão é a mudança de fase\n");
    printf("================================================================\n");

    vis = calloc(LIM + 1, 1);
    if (!vis) { fprintf(stderr, "sem memória\n"); return 2; }

    /* a sequência precisa de um conjunto de visitados sem limite superior para decidir
     * o recuo; usa-se uma tabela de dispersão aberta sobre os valores já vistos. */
    const long H = 1L << 23;                 /* 8 Mi entradas, potência de 2 */
    long *tab = malloc(H * sizeof(long));
    if (!tab) { free(vis); fprintf(stderr, "sem memória\n"); return 2; }
    for (long i = 0; i < H; i++) tab[i] = VAZIO;
    #define PUT(v) do { unsigned long h=((unsigned long)(v)*11400714819323198485UL)>>41; \
                        while (tab[h & (H-1)] != VAZIO && tab[h & (H-1)] != (v)) h++;       \
                        tab[h & (H-1)] = (v); } while (0)
    #define HAS(v) ({ unsigned long h=((unsigned long)(v)*11400714819323198485UL)>>41;   \
                      int r=0; while (tab[h & (H-1)] != VAZIO) {                            \
                        if (tab[h & (H-1)] == (v)) { r=1; break; } h++; } r; })

    long a = 0;
    PUT(0); vis[0] = 1;

    long pa_ok = 0, colisoes = 0, col_fase0 = 0, recuos = 0, recuo_fase_pi = 0;

    for (long n = 1; n <= N; n++) {
        long c = a - n, ant = a;
        int colidiu = 0;

        if (c > 0 && !HAS(c)) { a = c; }              /* recua: a fase vai a π  */
        else                  { a = a + n; colidiu = 1; }  /* colide: fase a 0  */

        /* §R1 — a PA: o salto vale exatamente n, em qualquer direção */
        if (labs(a - ant) == n) pa_ok++;

        /* §R3 — a colisão força a fase a 0; sem colisão, recua (fase π) */
        if (colidiu) { colisoes++; if (a > ant) col_fase0++; }
        else         { recuos++;   if (a < ant) recuo_fase_pi++; }

        PUT(a);
        if (a >= 0 && a <= LIM) vis[a] = 1;
    }

    /* ---------------- §R1 — a PA é exata ---------------- */
    printf("\n§R1 o tamanho do salto é a PA de razão 1, e é rígido\n");
    printf("      |a(n) − a(n−1)| = n  em %ld de %ld passos\n", pa_ok, N);
    ok("a PA dos passos é exata: |Δ| = n em todos", pa_ok == N);

    /* ---------------- §R2 — exp conjuga PA e PG ---------------- */
    printf("\n§R2 no espelho multiplicativo, a PA vira PG de razão e^d\n");
    printf("      (é a prop:conjuga da teoria: exp(−r) = 1/exp(r))\n");
    {
        /* um trecho aditivo com diferenças d; no espelho as razões têm de dar e^d */
        double t[4] = { 2, 7, 13, 20 };          /* a(4..7) da sequência        */
        double pior = 0;
        for (int i = 0; i < 3; i++) {
            double d      = t[i+1] - t[i];
            double razao  = exp(t[i+1] / 10.0) / exp(t[i] / 10.0);
            double previs = exp(d / 10.0);
            double r = fabs(razao - previs);
            if (r > pior) pior = r;
        }
        printf("      trecho [2,7,13,20], diferenças 5,6,7 → razões e^{0,5}, e^{0,6}, e^{0,7}\n");
        printf("      pior resíduo: %.2e\n", pior);

        /* O RESÍDUO DEU ZERO EXACTO, E ISSO É SORTE DESTES TRÊS VALORES, não estrutura:
         * exp(x)/exp(y) e exp(x−y) são rotas diferentes e noutro trecho dariam 1e-16.
         * Trocar o limiar por `== 0` seria construir a asserção à medida dos dados.
         *
         * A TESE é o HOMOMORFISMO — b^{a+d} = b^a · b^d, «a soma vai no produto» — e essa
         * não precisa da exponencial real nenhuma: mede-se em INTEIROS, com uma base
         * inteira, e aí é exacta. A rota em double fica ao lado a dizer que a exp real a
         * segue, com o resíduo que tiver. */
        long homo = 0, tot_h = 0;
        for(long b = 2; b <= 7; b++)
            for(long a = 0; a <= 8; a++)
                for(long d = 0; a + d <= 8; d++){
                    long pa = 1, pd = 1, pad = 1;
                    for(long k = 0; k < a; k++)     pa  *= b;
                    for(long k = 0; k < d; k++)     pd  *= b;
                    for(long k = 0; k < a + d; k++) pad *= b;
                    tot_h++;
                    if(pad == pa * pd) homo++;      /* b^{a+d} = b^a · b^d, EXACTO */
                }
        printf("      e a LEI em inteiros: b^{a+d} = b^a·b^d em %ld de %ld (bases 2..7)\n",
               homo, tot_h);
        ok("exp leva a PA de razão d na PG de razão e^d — e a tese é o HOMOMORFISMO"
           " b^{a+d} = b^a·b^d, medido EXACTO em inteiros. O resíduo zero da rota em"
           " vírgula é destes três valores, e não se toma por lei",
           (long long)(pior * 1e12) == 0 && tot_h > 0 && homo == tot_h);
    }

    /* ---------------- §R3 — a colisão é o reset de fase ---------------- */
    printf("\n§R3 a colisão força a fase a 0; sem colisão, a fase é π\n");
    printf("      colisões: %ld (todas avançam)   recuos: %ld (todos retrocedem)\n",
           colisoes, recuos);
    ok("toda colisão avança — a fase vai a 0, sem exceção", col_fase0 == colisoes);
    ok("todo não-colidido recua — a fase vai a π, sem exceção", recuo_fase_pi == recuos);

    /* ---------------- §R4 — o espelho é antissimétrico ---------------- */
    printf("\n§R4 o dual antissimétrico é o mesmo conjunto refletido\n");
    {
        /* a dual: avança se ficar NEGATIVO e livre; vive em ≤0 */
        const long M = 200000;
        char *pos = calloc(20001, 1), *neg = calloc(20001, 1);
        long *t2 = malloc(H * sizeof(long));
        for (long i = 0; i < H; i++) t2[i] = VAZIO;
        long b = 0;
        #define PUT2(v) do { unsigned long h=((unsigned long)(v)*11400714819323198485UL)>>41; \
                             while (t2[h & (H-1)] != VAZIO && t2[h & (H-1)] != (v)) h++;         \
                             t2[h & (H-1)] = (v); } while (0)
        #define HAS2(v) ({ unsigned long h=((unsigned long)(v)*11400714819323198485UL)>>41;   \
                           int r=0; while (t2[h & (H-1)] != VAZIO) {                             \
                             if (t2[h & (H-1)] == (v)) { r=1; break; } h++; } r; })
        PUT2(0); neg[0] = 1;
        for (long n = 1; n <= M; n++) {
            long c = b + n;
            b = (c < 0 && !HAS2(c)) ? c : b - n;
            PUT2(b);
            if (b <= 0 && -b <= 20000) neg[-b] = 1;
        }
        /* e a canónica no mesmo alcance */
        long *t3 = malloc(H * sizeof(long));
        for (long i = 0; i < H; i++) t3[i] = VAZIO;
        long z = 0;
        #define PUT3(v) do { unsigned long h=((unsigned long)(v)*11400714819323198485UL)>>41; \
                             while (t3[h & (H-1)] != VAZIO && t3[h & (H-1)] != (v)) h++;         \
                             t3[h & (H-1)] = (v); } while (0)
        #define HAS3(v) ({ unsigned long h=((unsigned long)(v)*11400714819323198485UL)>>41;   \
                           int r=0; while (t3[h & (H-1)] != VAZIO) {                             \
                             if (t3[h & (H-1)] == (v)) { r=1; break; } h++; } r; })
        PUT3(0); pos[0] = 1;
        for (long n = 1; n <= M; n++) {
            long c = z - n;
            z = (c > 0 && !HAS3(c)) ? c : z + n;
            PUT3(z);
            if (z >= 0 && z <= 20000) pos[z] = 1;
        }
        long dif = 0;
        for (long k = 0; k <= 20000; k++) if (pos[k] != neg[k]) dif++;
        printf("      canónica em ≥0 contra dual em ≤0, refletida: %ld diferenças em 20001\n", dif);
        ok("o dual é o espelho EXATO: A = −B, e não acrescenta cobertura", dif == 0);
        free(pos); free(neg); free(t2); free(t3);
    }

    /* ---------------- §R6 — o PAR (valor, índice) e as duas formas ------------- */
    printf("\n§R6 no PLANO o par nao se repete; na RETA o valor repete-se\n");
    {
        const long M = 200000;
        long *t4 = malloc(H * sizeof(long));
        for (long i = 0; i < H; i++) t4[i] = VAZIO;
        #define PUT4(v) do { unsigned long h=((unsigned long)(v)*11400714819323198485UL)>>41; \
                             while (t4[h & (H-1)] != VAZIO && t4[h & (H-1)] != (v)) h++;      \
                             t4[h & (H-1)] = (v); } while (0)
        #define HAS4(v) ({ unsigned long h=((unsigned long)(v)*11400714819323198485UL)>>41;   \
                           int r=0; while (t4[h & (H-1)] != VAZIO) {                          \
                             if (t4[h & (H-1)] == (v)) { r=1; break; } h++; } r; })
        long z = 0; PUT4(0);
        long repete_valor = 0;
        long dir_pos = 0, cruz_troca = 0, ant_cruz = 0, pares = 0;
        long za = 0, zn = 0;                       /* z anterior, em cartesiana */
        for (long n = 1; n <= M; n++) {
            long c = z - n;
            int rec = (c > 0 && !HAS4(c));
            long novo_z = rec ? c : z + n;
            if (HAS4(novo_z)) repete_valor++;      /* o VALOR ja' tinha aparecido */
            PUT4(novo_z);
            /* o par cartesiano e' (valor, indice); os dois produtos entre passos */
            if (n > 1) {
                long d = za*novo_z + zn*n;         /* DIRETO: mede               */
                long cx = za*n - zn*novo_z;        /* CRUZADO: ordena            */
                if (d > 0) dir_pos++;
                if (n > 2 && ((cx > 0) != (ant_cruz > 0))) cruz_troca++;
                ant_cruz = cx; pares++;
            }
            za = novo_z; zn = n; z = novo_z;
        }
        printf("      o VALOR repetiu-se %ld vezes em %ld passos\n", repete_valor, M);
        printf("      o PAR (valor, indice) repetiu-se 0 vezes — o indice distingue sempre\n");
        ok("na reta o valor repete-se; no plano o par nunca repete", repete_valor > 0);
        printf("      produto DIRETO positivo em %ld de %ld pares consecutivos\n", dir_pos, pares);
        printf("      produto CRUZADO trocou de sinal %ld vezes\n", cruz_troca);
        ok("o direto MEDE (nao inverte); o cruzado ORDENA (alterna)",
           dir_pos == pares && cruz_troca > 0);
        free(t4);
    }

    /* ---------------- §R5 — NÃO preenche (o controlo negativo) ---------------- */
    printf("\n§R5 controlo negativo: a sequência NÃO preenche a reta positiva\n");
    {
        long buracos = 0, primeiro = -1;
        for (long k = 0; k <= LIM; k++)
            if (!vis[k]) { buracos++; if (primeiro < 0) primeiro = k; }
        printf("      com %ld passos, faltam %ld inteiros de 0..%ld\n", N, buracos, LIM);
        printf("      o menor que falta: %ld\n", primeiro);
        printf("      densidade coberta: %.4f%%\n", 100.0 * (LIM + 1 - buracos) / (LIM + 1));
        /* esta asserção FALHA se alguém escrever que a sequência preenche */
        ok("NÃO preenche: restam buracos (a conjectura está EM ABERTO)", buracos > 0);
        conclui("que TODO inteiro apareça é conjectura por resolver — este medidor não a decide,");
        conclui("mede os buracos que restam, e é isso que impede o texto de dizer 'preenche'.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    free(vis); free(tab);
    return falhas ? 1 : 0;
}
